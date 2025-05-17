/*
 * SDL_atk:  An audio toolkit library for use with SDL
 * Copyright (C) 2022-2023 Walker M. White
 *
 * This is a library to load different types of audio files as PCM data,
 * and process them with basic DSP tools. The goal of this library is to
 * create an audio equivalent of SDL_image, where we can load and process
 * audio files without having to initialize the audio subsystem. In addition,
 * giving us direct control of the audio allows us to add more custom
 * effects than is possible in SDL_mixer.
 *
 * SDL License:
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */
#include <SDL_atk.h>
#include <string.h>
#include "ATK_Codec_c.h"

#if defined(FLAC__NO_DLL) && defined(DLL_EXPORT)
    // Apparently CMake is setting this somewhere
    #undef DLL_EXPORT
#endif

/**
 * @file ATK_CodecFLAC.c
 *
 * This component contains the functions for loading and saving FLAC files.
 */
#ifdef LOAD_FLAC
#pragma mark -
#pragma mark FLAC Decoding
#include <FLAC/stream_decoder.h>

/**
 * The internal structure for decoding
 */
typedef struct {
    /** The file stream for the audio */
    SDL_RWops* stream;
    /** Whether this object owns the underlying stream */
    int ownstream;
    /** The FLAC decoder struct */
    FLAC__StreamDecoder* flac;

    /** The size of a decoder chunk */
    Uint32 pagesize;
    /** The current page in the stream */
    Uint32 currpage;
    /** The previous page in the stream */
    Uint32 lastpage;

    /** The intermediate buffer for uniformizing FLAC data */
    float* buffer;
    /** The size of the intermediate buffer */
    Uint64  buffsize;
    /** The last element read from the intermediate buffer */
    Uint64  bufflast;
    /** The number of bits used to encode the sample data */
    Uint32  bitdepth;

} ATK_FLAC_Decoder;

/**
 * Returns a newly allocated list of metadata comments.
 *
 * The comments are parsed according to the Vorbis comment specification. The
 * number of comments in the returned array are stored in the len parameter.
 *
 * @param comment   The Vorbis comment
 * @param len       Pointer to store the comment length
 *
 * @return a newly allocated list of metadata comments.
 */
static ATK_AudioComment* ATK_FLAC_AllocComments(const FLAC__StreamMetadata_VorbisComment* comment,
                                                int* len) {
    int amt = comment->num_comments;
    ATK_AudioComment* result = (ATK_AudioComment*)ATK_malloc(sizeof(ATK_AudioComment)*amt);
    memset(result, 0, sizeof(ATK_AudioComment)*amt);
    for(size_t ii = 0; ii < amt; ii++) {
        char* entry = (char*)comment->comments[ii].entry;
        int length  = comment->comments[ii].length;

        int pos = -1;
        for(size_t jj = 0; jj < length && pos == -1; jj++) {
            if (entry[jj] == '=') {
                pos = (int)jj;
            }
        }

        if (pos == -1) {
            char* str = (char*)ATK_malloc(sizeof(char)*(length+1));
            memcpy(str, entry, length);
            str[length] = 0;
            result[ii].key = str;
        } else {
            char* str = (char*)ATK_malloc(sizeof(char)*(pos+1));
            memcpy(str, entry, pos);
            str[pos] = 0;
            result[ii].key = str;

            str = (char*)ATK_malloc(sizeof(char)*(length-pos));
            memcpy(str, entry+pos+1, length-pos-1);
            str[length-pos-1] = 0;
            result[ii].value = str;
        }
    }
    if (len != NULL) {
        *len = amt;
    }
    return result;
}

/**
 * Performs a read of the underlying file stream for the FLAC decoder
 *
 * This method abstracts the file access to allow us to read the asset on
 * non-standard platforms (e.g. Android). If method reads less than the
 * requested number of bytes, the actual value is stored in the provided
 * parameter pointer.
 *
 * @param decoder   The FLAC decoder struct
 * @param buffer    The buffer to start the data read
 * @param bytes     The number of bytes to read
 * @param cdata     The ATK_AudioSource instance
 *
 * @return the callback status (error or continue)
 */
FLAC__StreamDecoderReadStatus flac_decoder_read(const FLAC__StreamDecoder *flac,
                                                FLAC__byte buffer[], size_t *bytes,
                                                void *cdata) {
    //SDL_Log("flac read");
    ATK_AudioSource* source = ( ATK_AudioSource*)cdata;
    ATK_FLAC_Decoder* decoder = (ATK_FLAC_Decoder*)source->decoder;
    if (SDL_RWtell(decoder->stream) == SDL_RWsize(decoder->stream)) {
        return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    }
    *bytes = SDL_RWread(decoder->stream, buffer, 1, *bytes);
    if (*bytes == -1) {
        return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
    }
    return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

/**
 * Performs a seek of the underlying file stream for the FLAC decoder
 *
 * This method abstracts the file access to allow us to read the asset on
 * non-standard platforms (e.g. Android). The offset provided is from
 * the file beginning (e.g. SEEK_SET).
 *
 * @param decoder   The FLAC decoder struct
 * @param offset    The number of bytes from the beginning of the file
 * @param cdata     The ATK_AudioSource instance
 *
 * @return the callback status (error or continue)
 */
FLAC__StreamDecoderSeekStatus flac_decoder_seek(const FLAC__StreamDecoder *flac,
                                                FLAC__uint64 offset,
                                                void *cdata) {
    //SDL_Log("flac seek");
    ATK_AudioSource* source = ( ATK_AudioSource*)cdata;
    ATK_FLAC_Decoder* decoder = (ATK_FLAC_Decoder*)source->decoder;
    if (SDL_RWseek(decoder->stream,offset, RW_SEEK_SET) == -1) {
        return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
    }
    return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}

/**
 * Performs a tell of the underlying file stream for the FLAC decoder
 *
 * This method abstracts the file access to allow us to read the asset on
 * non-standard platforms (e.g. Android). The value computed is the
 * file offset relative to the beginning of the file. The value read is
 * stored in the provided parameter pointer.
 *
 * @param decoder   The FLAC decoder struct
 * @param offset    The pointer to store the offset from the beginning
 * @param cdata     The  ATK_AudioSource reference
 *
 * @return the callback status (error or continue)
 */
FLAC__StreamDecoderTellStatus flac_decoder_tell(const FLAC__StreamDecoder *flac,
                                                FLAC__uint64 *offset,
                                                void *cdata) {
    //SDL_Log("flac tell");
    ATK_AudioSource* source = ( ATK_AudioSource*)cdata;
    ATK_FLAC_Decoder* decoder = (ATK_FLAC_Decoder*)source->decoder;
    *offset = SDL_RWtell(decoder->stream);
    return (*offset == -1 ? FLAC__STREAM_DECODER_TELL_STATUS_ERROR : FLAC__STREAM_DECODER_TELL_STATUS_OK);
}

/**
 * Performs a length computation of the underlying file for the FLAC decoder
 *
 * This method abstracts the file access to allow us to read the asset on
 * non-standard platforms (e.g. Android). The value computed is the
 * length in bytes. The value read is stored in the provided parameter
 * pointer.
 *
 * @param decoder   The FLAC decoder struct
 * @param length    The pointer to store the file length
 * @param cdata     The AUFLACDecoder instance
 *
 * @return the callback status (error or continue)
 */
FLAC__StreamDecoderLengthStatus flac_decoder_size(const FLAC__StreamDecoder *flac,
                                                  FLAC__uint64 *length,
                                                  void *cdata) {
    //SDL_Log("Flac size");
    ATK_AudioSource* source = ( ATK_AudioSource*)cdata;
    ATK_FLAC_Decoder* decoder = (ATK_FLAC_Decoder*)source->decoder;
    *length = SDL_RWsize(decoder->stream);
    return (*length == -1 ? FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR : FLAC__STREAM_DECODER_LENGTH_STATUS_OK);
}

/**
 * Performs an eof computation of the underlying file for the FLAC decoder
 *
 * This method abstracts the file access to allow us to read the asset on
 * non-standard platforms (e.g. Android).
 *
 * @param decoder   The FLAC decoder struct
 * @param cdata     The AUFLACDecoder instance
 *
 * @return true if the stream is at the end of the file
 */
FLAC__bool flac_decoder_eof(const FLAC__StreamDecoder *flac, void *cdata) {
    //SDL_Log("Flac eof");
    ATK_AudioSource* source = ( ATK_AudioSource*)cdata;
    ATK_FLAC_Decoder* decoder = (ATK_FLAC_Decoder*)source->decoder;
    return SDL_RWtell(decoder->stream) == SDL_RWsize(decoder->stream);
}

/**
 * Performs a write of decoded sample data
 *
 * This method is the primary write method for decoded sample data. The
 * data is converted to a float format and stored in the backing buffer
 * for later access.
 *
 * @param decoder   The FLAC decoder struct
 * @param frame     The frame header for the current data block
 * @param buffer    The decoded samples for this block
 * @param cdata     The AUFLACDecoder instance
 *
 * @return the callback status (error or continue)
 */
FLAC__StreamDecoderWriteStatus flac_decoder_write(const FLAC__StreamDecoder *flac,
                                                  const FLAC__Frame *frame,
                                                  const FLAC__int32 * const buffer[],
                                                  void *cdata) {
    ATK_AudioSource* source = ( ATK_AudioSource*)cdata;
    ATK_FLAC_Decoder* decoder = (ATK_FLAC_Decoder*)source->decoder;
    if(frame->header.channels != source->metadata.channels) {
        ATK_SetError("FLAC has changed number of channels from %d to %d",
                     source->metadata.channels, frame->header.channels);
        decoder->buffsize = 0;
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }

    decoder->buffsize = frame->header.blocksize;
    double factor = 1.0/((1L << (decoder->bitdepth-1))-1);
    for(Uint32 ch = 0; ch < source->metadata.channels; ch++) {
        if (buffer[ch] == NULL) {
            ATK_SetError("FLAC channel %d is NULL", ch);
            decoder->buffsize = 0;
            return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
        }

        //Sint32* output = decoder->buffer+ch;
        float* output = decoder->buffer+ch;
        const FLAC__int32* input = buffer[ch];
        for (Uint32 ii = 0; ii < decoder->buffsize; ii++) {
            *output = (float)(*input*factor);
            output += source->metadata.channels;
            input++;
        }
    }

    decoder->bufflast = 0;
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

/**
 * Performs a write of the file metadata
 *
 * This method is called when the decoder is initialized to query the
 * stream info data. This is how the decoder gathers the important
 * decoding information like sample rate and channel layout.
 *
 * @param decoder   The FLAC decoder struct
 * @param metadata  The file metadata
 * @param cdata     The AUFLACDecoder instance
 *
 * @return the callback status (error or continue)
 */
void flac_decoder_metadata(const FLAC__StreamDecoder *flac,
                           const FLAC__StreamMetadata *metadata,
                           void *cdata) {
    ATK_AudioSource* source = ( ATK_AudioSource*)cdata;
    ATK_FLAC_Decoder* decoder = (ATK_FLAC_Decoder*)source->decoder;
    if(metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
        DECONST(Uint8,source->metadata.channels) = metadata->data.stream_info.channels;
        DECONST(Uint64,source->metadata.frames)  = metadata->data.stream_info.total_samples;
        DECONST(Uint32,source->metadata.rate)    = metadata->data.stream_info.sample_rate;
        decoder->pagesize = metadata->data.stream_info.max_blocksize;
        decoder->bitdepth = metadata->data.stream_info.bits_per_sample;
        decoder->lastpage = (Uint32)(source->metadata.frames/decoder->pagesize);
        if (source->metadata.frames % decoder->pagesize != 0) {
        	decoder->lastpage++;
        }
    } else if (metadata->type == FLAC__METADATA_TYPE_VORBIS_COMMENT) {
        int amt = 0;
        DECONST(ATK_AudioComment*,source->metadata.comments) = ATK_FLAC_AllocComments(&metadata->data.vorbis_comment, &amt);
        DECONST(Uint16,source->metadata.num_comments) = amt;
    }
}

/**
 * Records an error in the underlying decoder
 *
 * This method does not abort decoding. Instead, it records the error
 * with ATK_SetError for later retrieval.
 *
 * @param flac      The FLAC decoder struct
 * @param state     The error status
 * @param cdata     The AUFLACDecoder instance
 *
 * @return the callback status (error or continue)
 */
void flac_decoder_error(const FLAC__StreamDecoder *flac,
                        FLAC__StreamDecoderErrorStatus status,
                        void *cdata) {
    ATK_SetError("FLAC error: %s",FLAC__StreamDecoderErrorStatusString[status]);
}


/**
 * Reads a single page of audio data into the buffer
 *
 * This function reads in the current stream page into the buffer. The data written
 * into the buffer is linear PCM data with interleaved channels. If the stream is
 * at the end, nothing will be written.
 *
 * The size of a page is given by {@link CODEC_PageSize}. This buffer should
 * large enough to hold this data. As the page size is in audio frames, that means
 * the buffer should be pagesize * # of channels * sizeof(float) bytes.
 *
 * This function is a helper for {@link CODEC_Read} and {@link CODEC_Fill}.
 *
 * @param source    The audio source
 * @param decoder   The FLAC decoder
 * @param buffer    The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
Sint32 flac_read_page(ATK_AudioSource* source, ATK_FLAC_Decoder* decoder, float* buffer) {
    Sint32 read = 0;
    Uint32 limit = decoder->pagesize;
    while (read < (Sint32)limit) {
        // First see how much is available
        Sint32 avail = (Sint32)(decoder->buffsize - decoder->bufflast);
        avail = ((Sint32)(decoder->pagesize - read) < avail ? decoder->pagesize - read : avail);

        float* output = buffer+(decoder->bufflast*source->metadata.channels);
        memcpy(output,decoder->buffer,avail*source->metadata.channels*sizeof(float));
        read += avail;
        decoder->bufflast += avail;

        // Page in more if we need it
        if (read < (Sint32)limit) {
            if (!FLAC__stream_decoder_process_single(decoder->flac) || decoder->bufflast == decoder->buffsize) {
			    decoder->currpage++;
                return read;
            }
        }
    }
    decoder->currpage++;
    return read;
}


#pragma mark ATK Functions
/**
 * Creates a new ATK_AudioSource from an Xiph FLAC file
 *
 * This function will return NULL if the file cannot be located or is not an
 * proper Xiph FLAC file. The file will not be read into memory, but is instead
 * available for streaming.
 *
 * It is the responsibility of the caller of this function to close the source
 * (with {@link ATK_UnloadSource}) when it is no longer needed.
 *
 * @param filename    The name of the file with audio data
 *
 * @return a new ATK_AudioSource from an Xiph FLAC file
 */
ATK_AudioSource* ATK_LoadFLAC(const char* filename) {
    ATK_FilePool* pool = ATK_DefaultFilePool();
    SDL_RWops* stream = NULL;
    if (pool) {
        stream = ATK_RWFromFilePool(filename, "rb", pool);
    } else {
        stream = SDL_RWFromFile(filename, "rb");
    }
    if (stream == NULL) {
        ATK_SetError("Could not open '%s'",filename);
        return NULL;
    }
    return ATK_LoadFLAC_RW(stream,1);
}

/**
 * Creates a new ATK_AudioSource from an Xiph FLAC readable/seekable RWops
 *
 * The RWops object must be positioned at the start of the audio metadata. Note
 * that any modification of the RWops (via seeks or reads) can potentially corrupt
 * the internal state of the ATK_AudioSource for subsequent function calls. That
 * is why loading directly from a file is preferable to this function unless you
 * need the flexibility of an in-memory source. If the RWops is not a proper
 * OGG vorbis source this function returns NULL and an error code will be set.
 *
 * It is the responsibility of the caller of this function to close the source
 * (with {@link ATK_UnloadSource}) when it is no longer needed. If ownsrc is
 * true, freeing the source will also close/free the underlying RWops object.
 * Otherwise it is the responsibility of the caller of this function to free/close
 * the RWops object separately.
 *
 * @param source    A seekable/readable SDL_RWops to provide audio data
 * @param ownsrc    Non-zero to acquire ownership of the SDL_RWops for deletion later
 *
 * @return a new ATK_AudioSource from an Xiph FLAC readable/seekable RWops
 */
ATK_AudioSource* ATK_LoadFLAC_RW(SDL_RWops* source, int ownsrc) {
    if (source == NULL) {
        ATK_SetError("NULL source data");
        return NULL;
    }

    FLAC__StreamDecoder *flac;
    if (!(flac = FLAC__stream_decoder_new())) {
        ATK_SetError("Could not allocate FLAC decoder");
        if (ownsrc) {
            SDL_RWclose(source);
        }
        return NULL;
    }

    (void)FLAC__stream_decoder_set_md5_checking(flac, true);
    (void)FLAC__stream_decoder_set_metadata_respond(flac,FLAC__METADATA_TYPE_VORBIS_COMMENT);
    FLAC__StreamDecoderInitStatus status;

    ATK_FLAC_Decoder* decoder = (ATK_FLAC_Decoder*)ATK_malloc(sizeof(ATK_FLAC_Decoder));
    if (!decoder) {
        ATK_OutOfMemory();
        FLAC__stream_decoder_delete(flac);
        if (ownsrc) {
            SDL_RWclose(source);
        }
        return NULL;
    }

    ATK_AudioSource* result = ( ATK_AudioSource*)malloc(sizeof( ATK_AudioSource));
    if (!result) {
        ATK_OutOfMemory();
        ATK_free(decoder);
        FLAC__stream_decoder_delete(flac);
        if (ownsrc) {
            SDL_RWclose(source);
        }
        return NULL;
    }

    memset(decoder,0,sizeof(ATK_FLAC_Decoder));
    memset(result,0,sizeof(ATK_AudioSource));
    *(ATK_CodecType*)(&result->type) = ATK_CODEC_FLAC;
    result->decoder = decoder;
    decoder->stream = source;
    decoder->ownstream = ownsrc;
    decoder->flac = flac;


    status = FLAC__stream_decoder_init_stream(flac,
                                              flac_decoder_read, flac_decoder_seek,
                                              flac_decoder_tell, flac_decoder_size,
                                              flac_decoder_eof, flac_decoder_write,
                                              flac_decoder_metadata, flac_decoder_error,
                                              result);
    if (status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        ATK_SetError("FLAC initialization error: %s",FLAC__StreamDecoderInitStatusString[status]);
        FLAC__stream_decoder_delete(flac);
        ATK_free(decoder);
        ATK_free(result);
        if (ownsrc) {
            SDL_RWclose(source);
        }
        return NULL;
    }

    int ok = FLAC__stream_decoder_process_until_end_of_metadata(flac);
    if (!ok || decoder->pagesize == 0) {
        ATK_SetError("FLAC source does not have a stream_info header");
        FLAC__stream_decoder_delete(flac);
        ATK_free(decoder);
        ATK_free(result);
        if (ownsrc) {
            SDL_RWclose(source);
        }
        return NULL;
    }

    decoder->buffer = (float*)ATK_malloc(decoder->pagesize*result->metadata.channels*sizeof(float));
    return result;
}

/**
 * Detects Xiph FLAC data on a readable/seekable SDL_RWops.
 *
 * This function attempts to determine if a file is a given filetype, reading
 * the least amount possible from the SDL_RWops (usually a few bytes). There is
 * no distinction made between "not the filetype in question" and basic i/o errors.
 *
 * This function will always attempt to seek the RWops back to where it
 * started when this function was called, but it will not report any errors in
 * doing so, but assuming seeking works, this means you can immediately use
 * this with a different ATK_AudioSourceIsTYPE function, or load the audio
 * without further seeking.
 *
 * You do not need to call this function to load data; SDL_atk can work to
 * determine file type in many cases in its standard load functions.
 *
 * @param source    A seekable/readable SDL_RWops to provide audio data.
 *
 * @returns SDL_true if this is Xiph FLAC data.
 */
SDL_bool ATK_SourceIsFLAC(SDL_RWops* source) {
    if (source == NULL) {
        return SDL_FALSE;
    }

    FLAC__StreamDecoder *flac;
    if (!(flac = FLAC__stream_decoder_new())) {
        ATK_SetError("Could not allocate FLAC decoder");
        return SDL_FALSE;
    }

    ATK_FLAC_Decoder* decoder = (ATK_FLAC_Decoder*)ATK_malloc(sizeof(ATK_FLAC_Decoder));
    if (!decoder) {
        ATK_OutOfMemory();
        FLAC__stream_decoder_delete(flac);
        return SDL_FALSE;
    }

    ATK_AudioSource* wrapper = ( ATK_AudioSource*)malloc(sizeof( ATK_AudioSource));
    if (!wrapper) {
        ATK_free(decoder);
        FLAC__stream_decoder_delete(flac);
        ATK_ClearError();
        return SDL_FALSE;
    }

    memset(decoder,0,sizeof(ATK_FLAC_Decoder));
    memset(wrapper,0,sizeof(ATK_AudioSource));
    *(ATK_CodecType*)(&wrapper->type) = ATK_CODEC_FLAC;
    wrapper->decoder = decoder;
    decoder->stream = source;
    decoder->flac   = flac;


    Sint64 pos = SDL_RWtell(source);
    (void)FLAC__stream_decoder_set_md5_checking(flac, true);
    FLAC__StreamDecoderInitStatus status;
    status = FLAC__stream_decoder_init_stream(flac,
                                              flac_decoder_read, flac_decoder_seek,
                                              flac_decoder_tell, flac_decoder_size,
                                              flac_decoder_eof, flac_decoder_write,
                                              flac_decoder_metadata, flac_decoder_error,
                                              wrapper);
    SDL_bool result = SDL_FALSE;
    if (status == FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        result = FLAC__stream_decoder_process_until_end_of_metadata(flac) && (decoder->pagesize > 0);
    }



    ATK_free(decoder);
    FLAC__stream_decoder_delete(flac);
    ATK_ClearError();
    SDL_RWseek(source, pos, RW_SEEK_SET);
    return result;
}

/**
 * The FLAC specific implementation of {@link ATK_UnloadSource}.
 *
 * @param source    The source to close
 *
 * @return 0 if the source was successfully closed, -1 otherwise.
 */
Sint32 ATK_FLAC_UnloadSource(ATK_AudioSource* source) {
    CHECK_SOURCE(source,-1)
    if (source->metadata.comments != NULL) {
        ATK_FreeComments(DECONST(ATK_AudioComment*,source->metadata.comments),source->metadata.num_comments);
        DECONST(ATK_AudioComment*,source->metadata.comments) = NULL;
    }

    ATK_FLAC_Decoder* decoder = (ATK_FLAC_Decoder*)(source->decoder);
    if (decoder->flac != NULL) {
        FLAC__stream_decoder_delete(decoder->flac);
        decoder->flac = NULL;
    }
    if (decoder->buffer != NULL) {
        free(decoder->buffer);
        decoder->buffer = NULL;
    }
    if (decoder->stream != NULL && decoder->ownstream) {
        SDL_RWclose(decoder->stream);
        decoder->stream = NULL;
    }
    ATK_free(decoder);
    source->decoder = NULL;
    ATK_free(source);
    return 0;
}

/**
 * The FLAC specific implementation of {@link ATK_SeekSourcePage}.
 *
 * @param source    The audio source
 * @param page      The page to seek
 *
 * @return the page acquired, or -1 if there is an error
 */
Sint32 ATK_FLAC_SeekSourcePage(ATK_AudioSource* source, Uint32 page) {
    CHECK_SOURCE(source,-1)

    ATK_FLAC_Decoder* decoder = (ATK_FLAC_Decoder*)(source->decoder);
    Uint64 pos = page*decoder->pagesize;
    if (page > decoder->lastpage) {
    	page = decoder->lastpage;
        pos  = source->metadata.frames;
    }

    if (!FLAC__stream_decoder_seek_absolute(decoder->flac,pos)) {
        ATK_SetError("Seek is not supported");
    }
    decoder->currpage = page;
    return page;
}

/**
 * The FLAC specific implementation of {@link ATK_GetSourcePageSize}.
 *
 * @param source    The audio source
 *
 * @return the number of audio frames in a page, or -1 if there is an error
 */
Sint32 ATK_FLAC_GetSourcePageSize(ATK_AudioSource* source) {
    CHECK_SOURCE(source,-1)
    return ((ATK_FLAC_Decoder*)(source->decoder))->pagesize;
}

/**
 * The FLAC specific implementation of {@link ATK_GetSourceFirstPageSize}.
 *
 * @param source    The audio source
 *
 * @return the number of audio frames on the first page, or -1 if there is an error
 */
Sint32 ATK_FLAC_GetSourceFirstPageSize(ATK_AudioSource* source) {
    return ATK_FLAC_GetSourcePageSize(source);
}


/**
 * The FLAC specific implementation of {@link ATK_GetSourceLastPage}.
 *
 * @param source    The audio source
 *
 * @return the index of the last page in the stream, or -1 if there is an error
 */
Sint32 ATK_FLAC_GetSourceLastPage(ATK_AudioSource* source) {
    CHECK_SOURCE(source,-1)
    return ((ATK_FLAC_Decoder*)(source->decoder))->lastpage;
}

/**
 * The FLAC specific implementation of {@link ATK_GetSourceCurrentPage}.
 *
 * @param source    The audio source
 *
 * @return the index of the current page in the stream, or -1 if there is an error
 */
Sint32 ATK_FLAC_GetSourceCurrentPage(ATK_AudioSource* source) {
    CHECK_SOURCE(source,-1)
    return ((ATK_FLAC_Decoder*)(source->decoder))->currpage;
}

/**
 * The FLAC specific implementation of {@link ATK_IsSourceEOF}.
 *
 * @param source    The audio source
 *
 * @return 1 if the audio source is at the end of the stream; 0 otherwise
 */
Uint32 ATK_FLAC_IsSourceEOF(ATK_AudioSource* source) {
    CHECK_SOURCE(source,0)
    ATK_FLAC_Decoder* decoder = (ATK_FLAC_Decoder*)(source->decoder);
    return (decoder->currpage == decoder->lastpage);
}

/**
 * The FLAC specific implementation of {@link ATK_ReadSourcePage}.
 *
 * @param source    The audio source
 * @param buffer    The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
Sint32 ATK_FLAC_ReadSourcePage(ATK_AudioSource* source, float* buffer) {
    CHECK_SOURCE(source,-1)
    ATK_FLAC_Decoder* decoder = (ATK_FLAC_Decoder*)(source->decoder);
    if (decoder->currpage == decoder->lastpage) {
    	return 0;
    }
    return flac_read_page(source,decoder,buffer);
}

/**
 * The FLAC specific implementation of {@link ATK_ReadSource}.
 *
 * @param source    The audio source
 * @param buffer    The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
Sint64 ATK_FLAC_ReadSource( ATK_AudioSource* source, float* buffer) {
    CHECK_SOURCE(source,-1)
    ATK_FLAC_Decoder* decoder = (ATK_FLAC_Decoder*)(source->decoder);

    Sint64 read = 0;
    Sint64 limit = (Sint64)source->metadata.frames;
    while (read < limit) {
        Sint32 take = flac_read_page(source,decoder,buffer+read*source->metadata.channels);
        if (take == 0) {
            limit = read;
        } else {
            read += take;
        }
    }
    return read;
}

#else
#pragma mark -
#pragma mark Dummy Decoding
/**
 * Creates a new ATK_AudioSource from an Xiph FLAC file
 *
 * This function will return NULL if the file cannot be located or is not an
 * proper Xiph FLAC file. The file will not be read into memory, but is instead
 * available for streaming.
 *
 * It is the responsibility of the caller of this function to close the source
 * (with {@link ATK_UnloadSource}) when it is no longer needed.
 *
 * @param filename    The name of the file with audio data
 *
 * @return a new ATK_AudioSource from an Xiph FLAC file
 */
ATK_AudioSource* ATK_FLAC_LoadSource(const char* filename) {
    ATK_SetError("Codec FLAC is not supported");
    return NULL;
}

/**
 * Creates a new ATK_AudioSource from an Xiph FLAC readable/seekable RWops
 *
 * The RWops object must be positioned at the start of the audio metadata. Note
 * that any modification of the RWops (via seeks or reads) can potentially corrupt
 * the internal state of the ATK_AudioSource for subsequent function calls. That
 * is why loading directly from a file is preferable to this function unless you
 * need the flexibility of an in-memory source. If the RWops is not a proper
 * OGG vorbis source this function returns NULL and an error code will be set.
 *
 * It is the responsibility of the caller of this function to close the source
 * (with {@link ATK_UnloadSource}) when it is no longer needed. If ownsrc is
 * true, freeing the source will also close/free the underlying RWops object.
 * Otherwise it is the responsibility of the caller of this function to free/close
 * the RWops object separately.
 *
 * @param source    A seekable/readable SDL_RWops to provide audio data
 * @param ownsrc    Non-zero to acquire ownership of the SDL_RWops for deletion later
 *
 * @return a new ATK_AudioSource from an Xiph FLAC readable/seekable RWops
 */
ATK_AudioSource* ATK_LoadFLAC_RW(SDL_RWops* source, int ownsrc) {
    ATK_SetError("Codec FLAC is not supported");
    return NULL;
}


/**
 * Detects Xiph FLAC data on a readable/seekable SDL_RWops.
 *
 * This function attempts to determine if a file is a given filetype, reading
 * the least amount possible from the SDL_RWops (usually a few bytes). There is
 * no distinction made between "not the filetype in question" and basic i/o errors.
 *
 * This function will always attempt to seek the RWops back to where it
 * started when this function was called, but it will not report any errors in
 * doing so, but assuming seeking works, this means you can immediately use
 * this with a different ATK_AudioSourceIsTYPE function, or load the audio
 * without further seeking.
 *
 * You do not need to call this function to load data; SDL_atk can work to
 * determine file type in many cases in its standard load functions.
 *
 * @param source    A seekable/readable SDL_RWops to provide audio data.
 *
 * @returns SDL_true if this is Xiph FLAC data.
 */
SDL_bool ATK_SourceIsFLAC(SDL_RWops* source) { return SDL_FALSE; }

/**
 * The FLAC specific implementation of {@link ATK_UnloadSource}.
 *
 * @param source    The source to close
 *
 * @return 0 if the source was successfully closed, -1 otherwise.
 */
Sint32 ATK_FLAC_UnloadSource(ATK_AudioSource* source) { return -1; }

/**
 * The FLAC specific implementation of {@link ATK_SeekSourcePage}.
 *
 * @param source    The audio source
 * @param page      The page to seek
 *
 * @return the page acquired, or -1 if there is an error
 */
Sint32 ATK_FLAC_SeekSourcePage(ATK_AudioSource* source, Uint32 page) { return -1; }

/**
 * The FLAC specific implementation of {@link ATK_GetSourcePageSize}.
 *
 * @param source    The audio source
 *
 * @return the number of audio frames in a page, or -1 if there is an error
 */
Sint32 ATK_FLAC_GetSourcePageSize(ATK_AudioSource* source) { return -1; }

/**
 * The FLAC specific implementation of {@link ATK_GetSourceFirstPageSize}.
 *
 * @param source    The audio source
 *
 * @return the number of audio frames on the first page, or -1 if there is an error
 */
Sint32 ATK_FLAC_GetSourceFirstPageSize(ATK_AudioSource* source) { return -1; }

/**
 * The FLAC specific implementation of {@link ATK_GetSourceLastPage}.
 *
 * @param source    The audio source
 *
 * @return the index of the last page in the stream, or -1 if there is an error
 */
Sint32 ATK_FLAC_GetSourceLastPage(ATK_AudioSource* source) { return -1; }

/**
 * The FLAC specific implementation of {@link ATK_GetSourceCurrentPage}.
 *
 * @param source    The audio source
 *
 * @return the index of the current page in the stream, or -1 if there is an error
 */
Sint32 ATK_FLAC_GetSourceCurrentPage(ATK_AudioSource* source) { return -1; }

/**
 * The FLAC specific implementation of {@link ATK_IsSourceEOF}.
 *
 * @param source    The audio source
 *
 * @return 1 if the audio source is at the end of the stream; 0 otherwise
 */
Uint32 ATK_FLAC_IsEOF(ATK_AudioSource* source) { return 1; }

/**
 * The FLAC specific implementation of {@link ATK_ReadSourcePage}.
 *
 * @param source    The audio source
 * @param buffer    The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
Sint32 ATK_FLAC_ReadSourcePage(ATK_AudioSource* source, float* buffer) { return -1; }

/**
 * The FLAC specific implementation of {@link ATK_ReadSource}.
 *
 * @param source    The audio source
 * @param buffer    The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
Sint64 ATK_FLAC_ReadSource( ATK_AudioSource* source, float* buffer) { return -1; }
#endif // LOAD_FLAC

#ifdef SAVE_FLAC
#pragma mark -
#pragma mark FLAC Encoding
#include <FLAC/stream_encoder.h>
#include <FLAC/metadata.h>

/**
 * The internal structure for encoding
 */
typedef struct {
    /** The file stream for the audio */
    SDL_RWops* stream;
    /** Whether this object owns the underlying stream */
    int ownstream;
    /** The FLAC encoder struct */
    FLAC__StreamEncoder* flac;
    /** The associated metadata */
    FLAC__StreamMetadata* meta;

    /** The intermediate buffer for uniformizing FLAC data */
    FLAC__int32* buffer;
    /** The size of the intermediate buffer */
    Uint64  buffsize;
    /** The last element read from the intermediate buffer */
    Uint64  bufflast;
    /** The number of bits used to encode the sample data */
    Uint32  bitdepth;

} ATK_FLAC_Encoder;

/** The default bit depth */
#define FLAC_BITDEPTH 16
/** The default compression level */
#define FLAC_COMPRESSION 5
/** The default page size */
#define FLAC_PAGESIZE 1024

/**
 * Writes processed FLAC data to the underlying file stream
 *
 * This method abstracts the file access to allow us to write the asset on
 * non-standard platforms (e.g. Android). The data is written at the current
 * seek position of the file stream.
 *
 * @param encoder   The FLAC encoder struct
 * @param cdata     The ATK_FLAC_Encoder instance
 *
 * @return the callback status (error or continue)
 */

FLAC__StreamEncoderWriteStatus flac_encoder_write(const FLAC__StreamEncoder *encoder,
                                                  const FLAC__byte buffer[], size_t bytes,
                                                  uint32_t samples, uint32_t current_frame,
                                                  void *client_data) {
    ATK_FLAC_Encoder* output = (ATK_FLAC_Encoder*)client_data;
    SDL_RWops* stream = output->stream;
    if (SDL_RWwrite(stream,buffer, 1, bytes) != bytes) {
        return FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR;
    }
    return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
}

/**
 * Performs a seek of the underlying file stream for the FLAC encoder
 *
 * This method abstracts the file access to allow us to write the asset on
 * non-standard platforms (e.g. Android). The offset provided is from
 * the file beginning (e.g. SEEK_SET).
 *
 * @param encoder   The FLAC encoder struct
 * @param offset    The number of bytes from the beginning of the file
 * @param cdata     The ATK_FLAC_Encoder instance
 *
 * @return the callback status (error or continue)
 */
FLAC__StreamEncoderSeekStatus flac_encoder_seek(const FLAC__StreamEncoder *encoder,
                                                FLAC__uint64 absolute_byte_offset,
                                                void *client_data) {

    ATK_FLAC_Encoder* output = (ATK_FLAC_Encoder*)client_data;
    SDL_RWops* stream = output->stream;
    if(SDL_RWseek(stream, absolute_byte_offset, RW_SEEK_SET) < 0) {
        return FLAC__STREAM_ENCODER_SEEK_STATUS_ERROR;
    }
    return FLAC__STREAM_ENCODER_SEEK_STATUS_OK;

}

/**
 * Performs a tell of the underlying file stream for the FLAC encoder
 *
 * This method abstracts the file access to allow us to write the asset on
 * non-standard platforms (e.g. Android). The value computed is the
 * file offset relative to the beginning of the file. The value read is
 * stored in the provided parameter pointer.
 *
 * @param encoder   The FLAC encoder struct
 * @param offset    The pointer to store the offset from the beginning
 * @param cdata     The  ATK_FLAC_Encoder reference
 *
 * @return the callback status (error or continue)
 */
FLAC__StreamEncoderTellStatus flac_encoder_tell(const FLAC__StreamEncoder *encoder,
                                                FLAC__uint64 *absolute_byte_offset,
                                                void *client_data) {
    ATK_FLAC_Encoder* output = (ATK_FLAC_Encoder*)client_data;
    SDL_RWops* stream = output->stream;
    Sint64 pos;
    if((pos = SDL_RWtell(stream)) < 0) {
        return FLAC__STREAM_ENCODER_TELL_STATUS_ERROR;
    }

    *absolute_byte_offset = (FLAC__uint64)pos;
    return FLAC__STREAM_ENCODER_TELL_STATUS_OK;
}


#pragma mark ATK Functions
/**
 * Deletes the FLAC specific encoder struct
 *
 * @param encoding  The codec encoder struct
 */
static void ATK_FLAC_FreeEncoding(ATK_AudioEncoding* encoding) {
    if (encoding == NULL) {
        return;
    }

    ATK_FLAC_Encoder* encoder = (ATK_FLAC_Encoder*)encoding->encoder;
    if (encoder != NULL) {
        if (encoder->meta != NULL) {
            FLAC__metadata_object_delete(encoder->meta);
            encoder->meta = NULL;
        }
        if (encoder->flac != NULL) {
            FLAC__stream_encoder_delete(encoder->flac);
            encoder->flac = NULL;
        }
        if (encoder->buffer != NULL) {
            ATK_free(encoder->buffer);
            encoder->buffer = NULL;
        }
        if (encoder->ownstream && encoder->stream) {
            SDL_RWclose(encoder->stream);
            encoder->stream = NULL;
        }
        ATK_free(encoder);
        encoding->encoder = NULL;
    }
    if (encoding->metadata.comments != NULL) {
        ATK_FreeComments(DECONST(ATK_AudioComment*,encoding->metadata.comments),
                         encoding->metadata.num_comments);
        DECONST(ATK_AudioComment*,encoding->metadata.comments) = NULL;
    }
    ATK_free(encoding);
}

/**
 * Returns a new FLAC encoding stream to write to the given file
 *
 * The provided metadata will be copied to the encoding object. So it is safe to delete
 * this metadata before the encoding is complete. If the encoding stream cannot be
 * allocated, or this library does not supported saving to FLAC, this function will
 * return NULL and an error value will be set.
 *
 * The metadata should reflect the properties of the stream to be encoded as much as
 * possible. However, FLAC allows the number of frames written to differ from that
 * in the metadata.
 *
 * This function will encode the audio using the default settings of the codec. We
 * do not currently support fine tune control of bit rates or compression options.
 *
 * It is the responsibility of the caller of this function to complete the encoding
 * (with {@link ATK_FinishEncoding}) when the stream is finished.
 *
 * @param filename  The name of the new audio file
 * @param type      The codec type
 * @param metadata  The stream metadata
 *
 * @return a new FLAC encoding stream to write to the given file
 */
ATK_AudioEncoding* ATK_EncodeFLAC(const char* filename,
                                  const ATK_AudioMetadata* metadata) {
    ATK_FilePool* pool = ATK_DefaultFilePool();
    SDL_RWops* stream = NULL;
    if (pool) {
        stream = ATK_RWFromFilePool(filename, "wb", pool);
    } else {
        stream = SDL_RWFromFile(filename, "wb");
    }
    if (stream == NULL) {
        ATK_SetError("Could not open '%s'",filename);
        return NULL;
    }
    return ATK_EncodeFLAC_RW(stream,1,metadata);
}

/**
 * Returns a new FLAC encoding stream to write to the given RWops
 *
 * The provided metadata will be copied to the encoding object. So it is safe to delete
 * this metadata before the encoding is complete. If the encoding stream cannot be
 * allocated, or this library does not supported saving to FLAC, this function will
 * return NULL and an error value will be set.
 *
 * The metadata should reflect the properties of the stream to be encoded as much as
 * possible. However, FLAC allows the number of frames written to differ from that
 * in the metadata.
 *
 * The RWops object should be positioned at the start of the stream. Note that any
 * modification of the RWops (via seeks or reads) can potentially corrupt the internal
 * state of the ATK_AudioEncoding for subsequent function calls. That is why writing
 * directly to a file is preferable to this function unless you need the flexibility
 * of an in-memory source.
 *
 * This function will encode the audio using the default settings of the codec. We
 * do not currently support fine tune control of bit rates or compression options.
 *
 * It is the responsibility of the caller of this function to complete the encoding
 * (with {@link ATK_FinishEncoding}) when the stream is finished.
 *
 * @param stream    A seekable/writeable SDL_RWops to store the encoding
 * @param ownsrc    Non-zero to acquire ownership of the SDL_RWops for deletion later
 * @param type      The codec type
 * @param metadata  The stream metadata
 *
 * @return a new FLAC encoding stream to write to the given RWops
 */
ATK_AudioEncoding* ATK_EncodeFLAC_RW(SDL_RWops* source, int ownsrc,
                                     const ATK_AudioMetadata* metadata) {
    FLAC__bool ok = true;
    FLAC__StreamEncoder *flac = 0;
    FLAC__StreamEncoderInitStatus init_status;
    FLAC__StreamMetadata *comments;
    FLAC__StreamMetadata_VorbisComment_Entry entry;

    ATK_AudioEncoding *result = (ATK_AudioEncoding*)ATK_malloc(sizeof(ATK_AudioEncoding));
    if (result == NULL) {
        ATK_OutOfMemory();
        return NULL;
    }

    // Initialize with metadata
    DECONST(ATK_CodecType,result->type) = ATK_CODEC_FLAC;
    DECONST(Uint8,result->metadata.channels) = metadata->channels;
    DECONST(Uint32,result->metadata.rate)    = metadata->rate;
    DECONST(Uint64,result->metadata.frames)  = metadata->frames;
    DECONST(Uint16,result->metadata.num_comments) = metadata->num_comments;
    DECONST(ATK_AudioComment*,result->metadata.comments) = NULL;
    result->encoder = NULL;

    ATK_FLAC_Encoder* encoder = (ATK_FLAC_Encoder*)ATK_malloc(sizeof(ATK_FLAC_Encoder));
    if (encoder == NULL) {
        ATK_OutOfMemory();
        ATK_FLAC_FreeEncoding(result);
        return NULL;
    }

    result->encoder = encoder;
    encoder->meta = NULL;
    encoder->flac = NULL;
    encoder->buffer = NULL;
    encoder->stream = source;
    encoder->ownstream = ownsrc;
    encoder->bufflast  = 0;
    encoder->bitdepth  = FLAC_BITDEPTH;
    encoder->buffsize  = FLAC_PAGESIZE*metadata->channels;

    if((flac = FLAC__stream_encoder_new()) == NULL) {
        ATK_SetError("Could not allocate FLAC encoder");
        ATK_FLAC_FreeEncoding(result);
        return NULL;
    }
    encoder->flac = flac;

    ok &= FLAC__stream_encoder_set_verify(flac, true);
    ok &= FLAC__stream_encoder_set_compression_level(flac, FLAC_COMPRESSION);
    ok &= FLAC__stream_encoder_set_blocksize(flac, FLAC_PAGESIZE);
    ok &= FLAC__stream_encoder_set_channels(flac, metadata->channels);
    ok &= FLAC__stream_encoder_set_bits_per_sample(flac, encoder->bitdepth);
    ok &= FLAC__stream_encoder_set_sample_rate(flac, metadata->rate);
    ok &= FLAC__stream_encoder_set_total_samples_estimate(flac, metadata->frames);

    if (!ok) {
        ATK_SetError("Could not set FLAC encoder metadata");
        ATK_FLAC_FreeEncoding(result);
        return NULL;
    }

    if (metadata->comments) {
        ok &= ((comments = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT)) != NULL);
        if (!ok) {
            ATK_OutOfMemory();
            ATK_FLAC_FreeEncoding(result);
            return NULL;
        }

        encoder->meta = comments;
        for(size_t ii = 0; ok && ii < metadata->num_comments; ii++) {
            ok &= FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(
                    &entry,metadata->comments[ii].key,metadata->comments[ii].value);
            ok &= FLAC__metadata_object_vorbiscomment_append_comment(comments, entry, /*copy=*/false);
        }

        if (!ok) {
            ATK_SetError("Metadata tag error");
            ATK_FLAC_FreeEncoding(result);
            return NULL;
        } else {
            ok = FLAC__stream_encoder_set_metadata(flac, &comments, 1);
        }
    }

    encoder->buffer = (FLAC__int32*)ATK_malloc(sizeof(FLAC__int32)*encoder->buffsize);
    if (encoder->buffer == NULL) {
        ATK_OutOfMemory();
        ATK_FLAC_FreeEncoding(result);
        return NULL;
    }

    /* initialize encoder */
    if (ok) {
        init_status = FLAC__stream_encoder_init_stream(flac,
                                                       flac_encoder_write, flac_encoder_seek,
                                                       flac_encoder_tell, NULL, encoder);
        if (init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK) {
            ATK_SetError("Error initializing encoder: %s",
                         FLAC__StreamEncoderInitStatusString[init_status]);
            ATK_FLAC_FreeEncoding(result);
            return NULL;
        }
    }

    // Last thing is to copy the metadata so we have a local copy
    DECONST(ATK_AudioComment*,result->metadata.comments) =
            (ATK_AudioComment*)ATK_malloc(sizeof(ATK_AudioComment)*metadata->num_comments);
    ATK_AudioComment* src = DECONST(ATK_AudioComment*,metadata->comments);
    ATK_AudioComment* dst = DECONST(ATK_AudioComment*,result->metadata.comments);
    for(size_t ii = 0; ii < metadata->num_comments; ii++) {
        char* key = ATK_malloc(sizeof(char)*(strlen(src->key)));
        strcpy(key, src->key);
        char* value = ATK_malloc(sizeof(char)*(strlen(src->value)));
        strcpy(value, src->value);
        DECONST(char*,dst->key) = key;
        DECONST(char*,dst->value) = value;
        src++; dst++;

    }

    return result;
}

/**
 * The FLAC specific implementation of {@link ATK_WriteEncoding}.
 *
 * @param encoding  The encoding stream
 * @param buffer    The data buffer with the audio samples
 * @param frames    The number of audio frames to write
 *
 * @return the number of frames written, or -1 on error
 */
Sint64 ATK_FLAC_WriteEncoding(ATK_AudioEncoding* encoding, float* buffer, size_t frames) {
    FLAC__bool ok = true;
    Sint64 amt = 0;
    Uint32 times = 0;
    ATK_FLAC_Encoder* flac = (ATK_FLAC_Encoder*)encoding->encoder;
    Uint8 channels = encoding->metadata.channels;
    Uint64 factor = (Uint64)(1L << (flac->bitdepth-1));
    float* input = buffer;
    while (frames) {
        size_t diff = flac->buffsize-flac->bufflast;
        diff = diff < frames*channels ? diff : frames*channels;
        FLAC__int32* output = flac->buffer+flac->bufflast;
        for(size_t ii = 0; ii < diff; ii++) {
            *output++ = (FLAC__int32)((*input++)*factor);
        }
        if (diff == flac->buffsize) {
            ok = FLAC__stream_encoder_process_interleaved(flac->flac,flac->buffer,(uint32_t)diff/channels);
            times++;
            flac->bufflast = 0;
            if (ok) {
                frames -= diff/channels;
                amt += diff/channels;
            } else {
                ATK_SetError("FLAC encoder failed to write a page");
                frames = 0;
            }
        } else {
            flac->bufflast = diff;
            amt += diff/channels;
            frames = 0;
        }
    }
    return amt;
}

/**
 * The FLAC specific implementation of {@link ATK_FinishEncoding}.
 *
 * @param encoding  The encoding stream
 *
 * @return 0 if the enconding was successfully completed, -1 otherwise.
 */
Sint32 ATK_FLAC_FinishEncoding(ATK_AudioEncoding* encoding) {
    FLAC__bool ok = true;
    if (encoding == NULL) {
        return -1;
    }

    ATK_FLAC_Encoder* flac = (ATK_FLAC_Encoder*)encoding->encoder;
    if (flac == NULL) {
        return -1;
    }

    // Flush any remaining data
    Sint32 result = 0;
    if (flac->bufflast != 0) {
        Uint32 amt = (Uint32)flac->bufflast/encoding->metadata.channels;
        if (!FLAC__stream_encoder_process_interleaved(flac->flac,flac->buffer,amt)) {
            ATK_SetError("FLAC encoder failed to write a page");
            result = -1;
        }
    }

    if (!FLAC__stream_encoder_finish(flac->flac)) {
        ATK_SetError("FLAC encoder failed at end of stream");
        result = -1;
    }
    ATK_FLAC_FreeEncoding(encoding);
    return result;
}

#else
#pragma mark -
#pragma mark Dummy Encoding
/**
 * Returns a new FLAC encoding stream to write to the given file
 *
 * The provided metadata will be copied to the encoding object. So it is safe to delete
 * this metadata before the encoding is complete. If the encoding stream cannot be
 * allocated, or this library does not supported saving to FLAC, this function will
 * return NULL and an error value will be set.
 *
 * The metadata should reflect the properties of the stream to be encoded as much as
 * possible. However, FLAC allows the number of frames written to differ from that
 * in the metadata.
 *
 * This function will encode the audio using the default settings of the codec. We
 * do not currently support fine tune control of bit rates or compression options.
 *
 * It is the responsibility of the caller of this function to complete the encoding
 * (with {@link ATK_FinishEncoding}) when the stream is finished.
 *
 * @param filename  The name of the new audio file
 * @param type      The codec type
 * @param metadata  The stream metadata
 *
 * @return a new FLAC encoding stream to write to the given file
 */
ATK_AudioEncoding* ATK_EncodeFLAC(const char* filename,
                                  const ATK_AudioMetadata* metadata) {
    ATK_SetError("Codec FLAC is not supported");
    return NULL;
}

/**
 * Returns a new FLAC encoding stream to write to the given RWops
 *
 * The provided metadata will be copied to the encoding object. So it is safe to delete
 * this metadata before the encoding is complete. If the encoding stream cannot be
 * allocated, or this library does not supported saving to FLAC, this function will
 * return NULL and an error value will be set.
 *
 * The metadata should reflect the properties of the stream to be encoded as much as
 * possible. However, FLAC allows the number of frames written to differ from that
 * in the metadata.
 *
 * The RWops object should be positioned at the start of the stream. Note that any
 * modification of the RWops (via seeks or reads) can potentially corrupt the internal
 * state of the ATK_AudioEncoding for subsequent function calls. That is why writing
 * directly to a file is preferable to this function unless you need the flexibility
 * of an in-memory source.
 *
 * This function will encode the audio using the default settings of the codec. We
 * do not currently support fine tune control of bit rates or compression options.
 *
 * It is the responsibility of the caller of this function to complete the encoding
 * (with {@link ATK_FinishEncoding}) when the stream is finished.
 *
 * @param stream    A seekable/writeable SDL_RWops to store the encoding
 * @param ownsrc    Non-zero to acquire ownership of the SDL_RWops for deletion later
 * @param type      The codec type
 * @param metadata  The stream metadata
 *
 * @return a new FLAC encoding stream to write to the given RWops
 */
ATK_AudioEncoding* ATK_EncodeFLAC_RW(SDL_RWops* source, int ownsrc,
                                     const ATK_AudioMetadata* metadata) {
    ATK_SetError("Codec FLAC is not supported");
    return NULL;
}

/**
 * The FLAC specific implementation of {@link ATK_WriteEncoding}.
 *
 * @param encoding  The encoding stream
 * @param buffer    The data buffer with the audio samples
 * @param frames    The number of audio frames to write
 *
 * @return the number of frames written, or -1 on error
 */
Sint64 ATK_FLAC_WriteEncoding(ATK_AudioEncoding* encoding, float* buffer, size_t frames) {
    return -1;
}

/**
 * The FLAC specific implementation of {@link ATK_FinishEncoding}.
 *
 * @param encoding  The encoding stream
 *
 * @return 0 if the enconding was successfully completed, -1 otherwise.
 */
Sint32 ATK_FLAC_FinishEncoding(ATK_AudioEncoding* encoding) {
    return -1;
}

#endif // SAVE_FLAC
