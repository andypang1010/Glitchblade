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
 * warranty.  In no event will the authors be held liable for any damages
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
#include "ATK_Codec_c.h"
#include <time.h>

/**
 * @file ATK_CodecVorbis.c
 *
 * This component contains the functions for loading and saving OGG vorbis
 * files.
 */
#ifdef LOAD_VORB
#pragma mark -
#pragma mark OGG Decoding
#include <vorbis/vorbisfile.h>

/**
 * The internal structure for decoding
 */
typedef struct {
    /** The file stream for the audio */
    SDL_RWops* stream;
    /** Whether this object owns the underlying stream */
    int ownstream;
    /** The OGG decoder struct */
    OggVorbis_File oggfile;
    /** Reference to the logical bitstream for decoding */
    Sint32 bitstream;

    /** The size of a decoder chunk */
    Uint32 pagesize;
    /** The current page in the stream */
    Uint32 currpage;
    /** The previous page in the stream */
    Uint32 lastpage;

} ATK_Vorbis_Decoder;

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
static ATK_AudioComment* ATK_Vorbis_AllocComments(const vorbis_comment* comment, int* len) {
    int amt = comment->comments;
    ATK_AudioComment* result = (ATK_AudioComment*)ATK_malloc(sizeof(ATK_AudioComment)*amt);
    memset(result, 0, sizeof(ATK_AudioComment)*amt);
    for(size_t ii = 0; ii < amt; ii++) {
        char* entry = (char*)comment->user_comments[ii];
        int length  = comment->comment_lengths[ii];

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
 * Returns the SDL channel for the given OGG channel
 *
 * The channel layout for Ogg data is nonstandard (e.g. channels > 3 are not
 * stereo compatible), so this function standardizes the channel layout to
 * agree with FLAC and other data encodings.
 *
 * @param ch        The OGG channel position
 * @param channels  The total number of channels
 *
 * @return the SDL channel for the given OGG channel
 */
static Uint32 ogg2sdl(Uint32 ch, Uint32 channels) {
    switch (channels) {
        case 3:
        case 5:
        {
            switch (ch) {
                case 1:
                    return 2;
                case 2:
                    return 1;
            }
        }
            break;
        case 6:
        {
            switch (ch) {
                case 1:
                    return 2;
                case 2:
                    return 1;
                case 3:
                    return 4;
                case 4:
                    return 5;
                case 5:
                    return 3;
            }
        }
    }
    return ch;
}

/**
 * Returns a human readable string for an error code in ogg decoding
 *
 * @param error The error code
 *
 * @return a human readable string for an error code
 */
static const char* ogg_read_error(int error) {
    switch (error) {
        case OV_EREAD:
            return "A read from media returned an error";
        case OV_ENOTVORBIS:
            return "Bitstream does not contain any Vorbis data";
        case OV_EVERSION:
            return "Vorbis version mismatch";
        case OV_EBADHEADER:
            return "Invalid Vorbis bitstream header";
        case OV_EFAULT:
            return "Internal logic fault (likely heap/stack corruption)";
        case OV_ENOSEEK:
            return "Bitstream is not seekable";
        case OV_EINVAL:
            return "The OGG headers cannot be read";
        case OV_EBADLINK:
            return "Invalid stream section";
        case OV_HOLE:
            return "Stream experienced an interruption in data";
    }

    return "Unknown OGG Vorbis error";
}

/**
 * Performs a read of the underlying file stream for the OGG decoder
 *
 * This method abstracts the file access to allow us to read the asset on
 * non-standard platforms (e.g. Android).
 *
 * @param ptr           The buffer to start the data read
 * @param size          The number of bytes per element
 * @param nmemb         The number of elements to read
 * @param datasource    The file to read from
 *
 * @return the number of elements actually read.
 */
static size_t ogg_decoder_read(void *ptr, size_t size, size_t nmemb, void *datasource) {
    return SDL_RWread((SDL_RWops*)datasource, ptr, size, nmemb);
}

/**
 * Performs a seek of the underlying file stream for the OGG decoder
 *
 * This method abstracts the file access to allow us to read the asset on
 * non-standard platforms (e.g. Android).  The value whence is one the
 * classic values SEEK_CUR, SEEK_SET, or SEEK_END.
 *
 * @param datasource    The file to seek
 * @param offset        The offset to seek to
 * @param whence        The position to offset from
 *
 * @return the new file position
 */
static int    ogg_decoder_seek(void *datasource, ogg_int64_t offset, int whence) {
    if(datasource==NULL)return(-1);
    return (int)SDL_RWseek((SDL_RWops*)datasource,offset,whence);
}

/**
 * Performs a tell of the underlying file stream for the OGG decoder
 *
 * This method abstracts the file access to allow us to read the asset on
 * non-standard platforms (e.g. Android). This function returns the current
 * offset (from the beginning) of the file position.
 *
 * @param datasource    The file to query
 *
 * @return the current file position
 */
static long   ogg_decoder_tell(void *datasource) {
    return (long)SDL_RWtell((SDL_RWops*)datasource);
}

#pragma mark -
#pragma mark ATK Functions
/**
 * Creates a new ATK_AudioSource from an OGG Vorbis file
 *
 * This function will return NULL if the file cannot be located or is not an
 * proper OGG Vorbis file. The file will not be read into memory, but is instead
 * available for streaming.
 *
 * It is the responsibility of the caller of this function to close the source
 * (with {@link ATK_UnloadSource}) when it is no longer needed.
 *
 * @param filename    The name of the file with audio data
 *
 * @return a new ATK_AudioSource from an OGG Vorbis file
 */
ATK_AudioSource* ATK_LoadVorbis(const char* filename) {
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
    return ATK_LoadVorbis_RW(stream,1);
}

/**
 * Creates a new ATK_AudioSource from an OGG Vorbis readable/seekable RWops
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
 * @return a new ATK_AudioSource from an OGG Vorbis readable/seekable RWops
 */
ATK_AudioSource* ATK_LoadVorbis_RW(SDL_RWops* source, int ownsrc) {
    if (source == NULL) {
        ATK_SetError("NULL source data");
        return NULL;
    }

    ATK_Vorbis_Decoder* decoder = (ATK_Vorbis_Decoder*)ATK_malloc(sizeof(ATK_Vorbis_Decoder));
    if (!decoder) {
        ATK_OutOfMemory();
        if (ownsrc) {
            SDL_RWclose(source);
        }
        return NULL;
    }
    decoder->bitstream = -1;

    ov_callbacks calls;
    calls.read_func = ogg_decoder_read;
    calls.seek_func = ogg_decoder_seek;
    calls.tell_func = ogg_decoder_tell;
    calls.close_func = NULL;

    int error = ov_open_callbacks(source, &(decoder->oggfile), NULL, 0, calls);
    if (error) {
        ATK_SetError("OGG initialization error: %s",ogg_read_error(error));
        if (ownsrc) {
            SDL_RWclose(source);
        }
        ATK_free(decoder);
        return NULL;
    }

    vorbis_info* info = ov_info(&(decoder->oggfile), decoder->bitstream);
    Uint32 frames =  (uint32_t)ov_pcm_total(&(decoder->oggfile), decoder->bitstream);

    decoder->stream = source;
    decoder->pagesize = ATK_CODEC_PAGE_SIZE/(sizeof(float)*info->channels);
    decoder->lastpage = (Uint32)(frames/decoder->pagesize);
    if (frames % decoder->pagesize != 0) {
        decoder->lastpage++;
    }
    decoder->currpage = 0;

    ATK_AudioSource* result = (ATK_AudioSource*)ATK_malloc(sizeof(ATK_AudioSource));
    if (!result) {
        ATK_OutOfMemory();
        if (ownsrc) {
            SDL_RWclose(source);
        }
        ATK_free(decoder);
        return NULL;
    }
    decoder->ownstream = ownsrc;
    *(ATK_CodecType*)(&result->type) = ATK_CODEC_VORBIS;
    DECONST(Uint8,result->metadata.channels) = info->channels;
    DECONST(Uint32,result->metadata.rate)    = (Uint32)info->rate;
    DECONST(Uint64,result->metadata.frames)  = frames;

    int amt = 0;
    DECONST(ATK_AudioComment*,result->metadata.comments) = ATK_Vorbis_AllocComments(ov_comment(&(decoder->oggfile),-1),&amt);
    DECONST(Uint16,result->metadata.num_comments) = amt;
    result->decoder = (void*)decoder;
    return result;
}

/**
 * Detects OGG Vorbis data on a readable/seekable SDL_RWops.
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
 * @returns SDL_true if this is OGG Vorbis data.
 */
SDL_bool ATK_SourceIsVorbis(SDL_RWops* source) {
    if (source == NULL) {
        return 0;
    }

    ATK_Vorbis_Decoder* decoder = (ATK_Vorbis_Decoder*)ATK_malloc(sizeof(ATK_Vorbis_Decoder));
    if (!decoder) {
        return 0;
    }

    Sint64 pos = SDL_RWtell(source);
    decoder->bitstream = -1;
    ov_callbacks calls;
    calls.read_func = ogg_decoder_read;
    calls.seek_func = ogg_decoder_seek;
    calls.tell_func = ogg_decoder_tell;
    calls.close_func = NULL;

    int error = ov_open_callbacks(source, &(decoder->oggfile), NULL, 0, calls);
    SDL_bool result = error == 0 ? SDL_TRUE : SDL_FALSE;
    ATK_free(decoder);
    ATK_ClearError();
    SDL_RWseek(source, pos, RW_SEEK_SET);
    return result;
}

/**
 * The Vorbis specific implementation of {@link ATK_UnloadSource}.
 *
 * @param source    The source to close
 *
 * @return 0 if the source was successfully closed, -1 otherwise.
 */
Sint32 ATK_Vorbis_UnloadSource(ATK_AudioSource* source) {
    CHECK_SOURCE(source,-1)
    if (source->metadata.comments != NULL) {
        ATK_FreeComments(DECONST(ATK_AudioComment*,source->metadata.comments),
                         source->metadata.num_comments);
        DECONST(ATK_AudioComment*,source->metadata.comments) = NULL;
    }

    ATK_Vorbis_Decoder* decoder = (ATK_Vorbis_Decoder*)(source->decoder);
    ov_clear(&(decoder->oggfile));
    if (decoder->stream != NULL) {
        if (decoder->ownstream) {
            SDL_RWclose(decoder->stream);
        }
        decoder->stream = NULL;
    }
    ATK_free(source->decoder);
    source->decoder = NULL;
    ATK_free(source);
    return 0;
}

/**
 * The Vorbis specific implementation of {@link ATK_SeekSourcePage}.
 *
 * @param source    The audio source
 * @param page        The page to seek
 *
 * @return the page acquired, or -1 if there is an error
 */
Sint32 ATK_Vorbis_SeekSourcePage(ATK_AudioSource* source, Uint32 page) {
    CHECK_SOURCE(source,-1)

    ATK_Vorbis_Decoder* decoder = (ATK_Vorbis_Decoder*)(source->decoder);
    if (page >= decoder->lastpage) {
        ATK_SetError("Page %d is out of bounds",page);
        return -1;
    }

    Uint64 frame = (Uint64)page*decoder->pagesize;
    if (frame > source->metadata.frames) {
        frame = source->metadata.frames;
    }

    int error = ov_pcm_seek(&(decoder->oggfile),frame);
    if (error) {
        ATK_SetError("Seek failure: %s",ogg_read_error(error));
        return -1;
    }
    decoder->currpage = (uint32_t)(frame/decoder->pagesize);
    return decoder->currpage;
}

/**
 * The Vorbis specific implementation of {@link ATK_GetSourcePageSize}.
 *
 * @param source    The audio source
 *
 * @return the number of audio frames in a page, or -1 if there is an error
 */
Sint32 ATK_Vorbis_GetSourcePageSize(ATK_AudioSource* source) {
    CHECK_SOURCE(source,-1)
    return ((ATK_Vorbis_Decoder*)(source->decoder))->pagesize;
}

/**
 * The Vorbis specific implementation of {@link ATK_GetSourceFirstPageSize}.
 *
 * @param source    The audio source
 *
 * @return the number of audio frames on the first page, or -1 if there is an error
 */
Sint32 ATK_Vorbis_GetSourceFirstPageSize(ATK_AudioSource* source) {
    return ATK_Vorbis_GetSourcePageSize(source);
}

/**
 * The Vorbis specific implementation of {@link ATK_GetSourceLastPage}.
 *
 * @param source    The audio source
 *
 * @return the index of the last page in the stream, or -1 if there is an error
 */
Sint32 ATK_Vorbis_GetSourceLastPage(ATK_AudioSource* source) {
    CHECK_SOURCE(source,-1)
    return ((ATK_Vorbis_Decoder*)(source->decoder))->lastpage;
}

/**
 * The Vorbis specific implementation of {@link ATK_GetSourceCurrentPage}.
 *
 * @param source    The audio source
 *
 * @return the index of the current page in the stream, or -1 if there is an error
 */
Sint32 ATK_Vorbis_GetSourceCurrentPage(ATK_AudioSource* source) {
    CHECK_SOURCE(source,-1)
    return ((ATK_Vorbis_Decoder*)(source->decoder))->currpage;
}

/**
 * The Vorbis specific implementation of {@link ATK_IsSourceEOF}.
 *
 * @param source    The audio source
 *
 * @return 1 if the audio source is at the end of the stream; 0 otherwise
 */
Uint32 ATK_Vorbis_IsSourceEOF(ATK_AudioSource* source) {
    CHECK_SOURCE(source,0)
    ATK_Vorbis_Decoder* decoder = (ATK_Vorbis_Decoder*)(source->decoder);
    return (decoder->currpage == decoder->lastpage);
}

/**
 * The Vorbis specific implementation of {@link ATK_ReadSourcePage}.
 *
 * @param source    The audio source
 * @param buffer    The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
Sint32 ATK_Vorbis_ReadSourcePage(ATK_AudioSource* source, float* buffer) {
    CHECK_SOURCE(source,-1)

    ATK_Vorbis_Decoder* decoder = (ATK_Vorbis_Decoder*)(source->decoder);
    if (decoder->currpage == decoder->lastpage) {
    	return 0;
    }

    // Now read from the stream
    Sint32 read = 0;
    Uint32 size = decoder->pagesize;

    float** pcmb;
    while (read < (Sint32)size) {
        Sint64 avail = (size - read >= size ? size : size - read);
        avail = ov_read_float(&(decoder->oggfile), &pcmb, (int)avail, &(decoder->bitstream));
        if (avail < 0) {
            ATK_SetError("Read error: %s",ogg_read_error((Sint32)avail));
            return -1;
        } else if (avail == 0) {
            break;
        }

        // Copy everything into its place
        for (Uint32 ch = 0; ch < source->metadata.channels; ++ch) {
            // OGG representation differs from SDL representation
            Uint32 outch = ogg2sdl(ch,source->metadata.channels);

            float* output = buffer+(read*source->metadata.channels)+outch;
            float* input  = pcmb[ch];
            Uint32 temp = (Sint32)avail;
            while (temp--) {
                *output = *input;
                output += source->metadata.channels;
                input++;
            }
        }

        read += (Sint32)avail;
    }

	decoder->currpage++;
    return read;
}

/**
 * The Vorbis specific implementation of {@link ATK_ReadSource}.
 *
 * @param source    The audio source
 * @param buffer    The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
Sint64 ATK_Vorbis_ReadSource(ATK_AudioSource* source, float* buffer) {
    CHECK_SOURCE(source,-1)

    ATK_Vorbis_Decoder* decoder = (ATK_Vorbis_Decoder*)(source->decoder);
    Uint32 currpage = decoder->currpage;
    if (currpage != 0) {
        ATK_Vorbis_SeekSourcePage(source,0);
    }

    // Now read from the stream
    Sint64 read = 0;
    Sint64 size = (Sint64)source->metadata.frames;

    float** pcmb;
    while (read < size) {
        Sint64 avail = (size - read >= size ? size : size - read);
        avail = (int)ov_read_float(&(decoder->oggfile), &pcmb, (int)avail, &(decoder->bitstream));
        if (avail < 0) {
            ATK_SetError("Read error: %s",ogg_read_error((Sint32)avail));
            return -1;
        } else if (avail == 0) {
            break;
        }

        // Copy everything into its place
        for (Uint32 ch = 0; ch < source->metadata.channels; ++ch) {
            // OGG representation differs from SDL representation
            Uint32 outch = ogg2sdl(ch,source->metadata.channels);

            float* output = buffer+(read*source->metadata.channels)+outch;
            float* input  = pcmb[ch];
            Uint32 temp = (Uint32)avail;
            while (temp--) {
                *output = *input;
                output += source->metadata.channels;
                input++;
            }
        }

        read += avail;
    }

    if (currpage != 0) {
        ATK_Vorbis_SeekSourcePage(source,currpage);
    }
    return read;
}

#else
#pragma mark -
#pragma mark Dummy Decoding
/**
 * Creates a new ATK_AudioSource from an OGG Vorbis file
 *
 * This function will return NULL if the file cannot be located or is not an
 * proper OGG Vorbis file. The file will not be read into memory, but is instead
 * available for streaming.
 *
 * It is the responsibility of the caller of this function to close the source
 * (with {@link ATK_UnloadSource}) when it is no longer needed.
 *
 * @param filename    The name of the file with audio data
 *
 * @return a new ATK_AudioSource from an OGG Vorbis file
 */
ATK_AudioSource* ATK_LoadVorbis(const char* filename) {
    ATK_SetError("Codec OGG Vorbis is not supported");
    return NULL;
}


/**
 * Creates a new ATK_AudioSource from an OGG Vorbis readable/seekable RWops
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
 * @return a new ATK_AudioSource from an OGG Vorbis readable/seekable RWops
 */
ATK_AudioSource* ATK_LoadVorbis_RW(SDL_RWops* source, int ownsrc) {
    ATK_SetError("Codec OGG Vorbis is not supported");
    return NULL;
}

/**
 * Creates a new ATK_AudioSource from an OGG Vorbis file
 *
 * This function will return NULL if the file cannot be located or is not an
 * proper OGG Vorbis file. The file will not be read into memory, but is instead
 * available for streaming.
 *
 * It is the responsibility of the caller of this function to close the source
 * (with {@link ATK_UnloadSource}) when it is no longer needed.
 *
 * @param filename    The name of the file with audio data
 *
 * @return a new ATK_AudioSource from an OGG Vorbis file
 */
ATK_AudioSource* ATK_Vorbis_LoadSource(const char* filename) {
    ATK_SetError("Codec OGG Vorbis is not supported");
    return NULL;
}

/**
 * Creates a new ATK_AudioSource from an OGG Vorbis readable/seekable RWops
 *
 * The RWops object must be positioned at the start of the audio metadata. Note
 * that any modification of the RWops (via seeks or reads) can potentially corrupt
 * the internal state of the ATK_AudioSource for subsequent function calls. That
 * is why loading directly from a file is preferable to this function unless you
 * need the flexibility of an in-memory source.
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
 * @return a new ATK_AudioSource from an OGG Vorbis readable/seekable RWops
 */
ATK_AudioSource* ATK_Vorbis_LoadSource_RW(SDL_RWops* source, int ownsrc) {
    ATK_SetError("Codec OGG Vorbis is not supported");
    return NULL;
}

/**
 * Detects OGG Vorbis data on a readable/seekable SDL_RWops.
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
 * @returns SDL_true if this is OGG Vorbis data.
 */
SDL_bool ATK_SourceIsVorbis(SDL_RWops* source) { return SDL_FALSE; }

/**
 * The Vorbis specific implementation of {@link ATK_UnloadSource}.
 *
 * @param source    The source to close
 *
 * @return 0 if the source was successfully closed, -1 otherwise.
 */
Sint32 ATK_Vorbis_UnloadSource(ATK_AudioSource* source) { return -1; }

/**
 * The Vorbis specific implementation of {@link ATK_SeekSourcePage}.
 *
 * @param source    The audio source
 * @param page      The page to seek
 *
 * @return the page acquired, or -1 if there is an error
 */
Sint32 ATK_Vorbis_SeekSourcePage(ATK_AudioSource* source, Uint32 page) { return -1; }

/**
 * The Vorbis specific implementation of {@link ATK_GetSourcePageSize}.
 *
 * @param source    The audio source
 *
 * @return the number of audio frames in a page, or -1 if there is an error
 */
Sint32 ATK_Vorbis_GetSourcePageSize(ATK_AudioSource* source) { return -1; }

/**
 * The Vorbis specific implementation of {@link ATK_GetSourceFirstPageSize}.
 *
 * @param source    The audio source
 *
 * @return the number of audio frames on the first page, or -1 if there is an error
 */
Sint32 ATK_Vorbis_GetSourceFirstPageSize(ATK_AudioSource* source) { return -1; }

/**
 * The Vorbis specific implementation of {@link ATK_GetSourceLastPage}.
 *
 * @param source    The audio source
 *
 * @return the index of the current page in the stream, or -1 if there is an error
 */
Sint32 ATK_Vorbis_GetSourceLastPage(ATK_AudioSource* source) { return -1; }

/**
 * The Vorbis specific implementation of {@link ATK_GetSourceCurrentPage}.
 *
 * @param source    The audio source
 *
 * @return the index of the current page in the stream, or -1 if there is an error
 */
Sint32 ATK_Vorbis_GetSourceCurrentPage(ATK_AudioSource* source) { return -1; }

/**
 * The Vorbis specific implementation of {@link ATK_IsSourceEOF}.
 *
 * @param source    The audio source
 *
 * @return 1 if the audio source is at the end of the stream; 0 otherwise
 */
Uint32 ATK_Vorbis_IsSourceEOF(ATK_AudioSource* source) { return 0; }

/**
 * The Vorbis specific implementation of {@link ATK_ReadSourcePage}.
 *
 * @param source    The audio source
 * @param buffer    The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
Sint32 ATK_Vorbis_ReadSourcePage(ATK_AudioSource* source, float* buffer) { return -1; }

/**
 * The Vorbis specific implementation of {@link ATK_ReadSource}.
 *
 * @param source    The audio source
 * @param buffer    The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
Sint64 ATK_Vorbis_ReadSource(ATK_AudioSource* source, float* buffer) { return -1; }

#endif

#if SAVE_VORB
#pragma mark -
#pragma mark Vorbis Encoding
#include <vorbis/vorbisenc.h>

/**
 * The internal structure for encoding
 */
typedef struct {
    /** The file stream for the audio */
    SDL_RWops* stream;
    /** Whether this object owns the underlying stream */
    int ownstream;
    /** Whether we have initialized the vorbis state */
    int active;
    /** Turns physical pages into a logical stream of packets */
    ogg_stream_state oggstream;
    /* One Ogg bitstream page.  Vorbis packets are inside */
    ogg_page         oggpage;
    /* One raw packet of data for decode */
    ogg_packet       oggpacket;
    /* Stores all the static vorbis bitstream settings */
    vorbis_info      vinfo;
    /* Stores all the user comments */
    vorbis_comment   vcomment;
    /* Central working state for the packet->PCM decoder */
    vorbis_dsp_state vdsp;
    /* Local working space for packet->PCM decode */
    vorbis_block     vblock;
} ATK_Vorbis_Encoder;

/** The quality setting for encoding */
#define VORBIS_QUALITY 1.0
/** The write buffer size */
#define VORBIS_PAGESIZE 1024

/**
 * Returns a human readable string for an error code in ogg encoding
 *
 * @param error The error code
 *
 * @return a human readable string for an error code
 */
static const char* ogg_write_error(int error) {
    switch (error) {
        case OV_EFAULT:
            return "Internal logic fault (likely heap/stack corruption)";
        case OV_EINVAL:
            return "Invalid request";
        case OV_EIMPL:
            return "Mode not implemented";
    }

    return "Unknown OGG Vorbis error";
}

/**
 * Deletes the Vorbis specific encoder struct
 *
 * @param encoding  The codec encoder struct
 */
static void ATK_Vorbis_FreeEncoding(ATK_AudioEncoding* encoding) {
    if (encoding == NULL) {
        return;
    }

    ATK_Vorbis_Encoder* encoder = (ATK_Vorbis_Encoder*)encoding->encoder;
    if (encoder != NULL) {
        if (encoder->active) {
            ogg_stream_clear(&encoder->oggstream);
            vorbis_block_clear(&encoder->vblock);
            vorbis_dsp_clear(&encoder->vdsp);
            vorbis_comment_clear(&encoder->vcomment);
            vorbis_info_clear(&encoder->vinfo);
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
 * Returns a new Vorbis encoding stream to write to the given file
 *
 * The provided metadata will be copied to the encoding object. So it is safe to delete
 * this metadata before the encoding is complete. If the encoding stream cannot be
 * allocated, or this library does not supported saving to Vorbis, this function will
 * return NULL and an error value will be set.
 *
 * The metadata should reflect the properties of the stream to be encoded as much as
 * possible. However, Vorbis allows the number of frames written to differ from that
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
 * @return a new Vorbis encoding stream to write to the given file
 */
ATK_AudioEncoding* ATK_EncodeVorbis(const char* filename,
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
    return ATK_EncodeVorbis_RW(stream,1,metadata);
}

/**
 * Returns a new Vorbis encoding stream to write to the given RWops
 *
 * The provided metadata will be copied to the encoding object. So it is safe to delete
 * this metadata before the encoding is complete. If the encoding stream cannot be
 * allocated, or this library does not supported saving to Vorbis, this function will
 * return NULL and an error value will be set.
 *
 * The metadata should reflect the properties of the stream to be encoded as much as
 * possible. However, Vorbis allows the number of frames written to differ from that
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
 * @return a new Vorbis encoding stream to write to the given RWops
 */
ATK_AudioEncoding* ATK_EncodeVorbis_RW(SDL_RWops* source, int ownsrc,
                                       const ATK_AudioMetadata* metadata) {
    ATK_AudioEncoding *result = (ATK_AudioEncoding*)ATK_malloc(sizeof(ATK_AudioEncoding));
    if (result == NULL) {
        ATK_OutOfMemory();
        return NULL;
    }
    memset(result, 0, sizeof(ATK_AudioEncoding));

    // Initialize with metadata
    DECONST(ATK_CodecType,result->type) = ATK_CODEC_VORBIS;
    DECONST(Uint8,result->metadata.channels) = metadata->channels;
    DECONST(Uint32,result->metadata.rate)    = metadata->rate;
    DECONST(Uint64,result->metadata.frames)  = metadata->frames;
    DECONST(Uint16,result->metadata.num_comments) = metadata->num_comments;
    DECONST(ATK_AudioComment*,result->metadata.comments) = NULL;
    result->encoder = NULL;

    ATK_Vorbis_Encoder* encoder = (ATK_Vorbis_Encoder*)ATK_malloc(sizeof(ATK_Vorbis_Encoder));
    if (encoder == NULL) {
        ATK_OutOfMemory();
        ATK_Vorbis_FreeEncoding(result);
        return NULL;
    }

    result->encoder = encoder;
    encoder->ownstream = ownsrc;
    encoder->stream = source;

    vorbis_info_init(&encoder->vinfo);
    int err = vorbis_encode_init_vbr(&encoder->vinfo,metadata->channels,metadata->rate,VORBIS_QUALITY);
    if (err) {
        ATK_SetError("Cannot initialize encoder: %s",ogg_write_error(err));
        ATK_Vorbis_FreeEncoding(result);
        return NULL;
    }

    if (metadata->num_comments > 0) {
        vorbis_comment_init(&encoder->vcomment);
        for(size_t ii = 0; ii < metadata->num_comments; ii++) {
            vorbis_comment_add_tag(&encoder->vcomment,metadata->comments[ii].key,metadata->comments[ii].value);
        }
    }

    /* Set up the analysis state and auxiliary encoding storage */
    vorbis_analysis_init(&encoder->vdsp,&encoder->vinfo);
    vorbis_block_init(&encoder->vdsp,&encoder->vblock);

    /* Pick a random serial number */
    srand((Uint32)time(NULL));
    ogg_stream_init(&encoder->oggstream,rand());

    // Create the header
    ogg_packet header;
    ogg_packet header_comm;
    ogg_packet header_code;

    vorbis_analysis_headerout(&encoder->vdsp,&encoder->vcomment,&header,&header_comm,&header_code);
    ogg_stream_packetin(&encoder->oggstream,&header);
    ogg_stream_packetin(&encoder->oggstream,&header_comm);
    ogg_stream_packetin(&encoder->oggstream,&header_code);
    encoder->active = 1;

    /** Write header to ensure audio starts on a new page */
    int avail = ogg_stream_flush(&encoder->oggstream,&encoder->oggpage);
    while (avail) {
        if (SDL_RWwrite(source, encoder->oggpage.header, 1, encoder->oggpage.header_len) < 0) {
            ATK_Vorbis_FreeEncoding(result);
            return NULL;
        }
        if (SDL_RWwrite(source, encoder->oggpage.body, 1, encoder->oggpage.body_len) < 0) {
            ATK_Vorbis_FreeEncoding(result);
            return NULL;
        }
        avail = ogg_stream_flush(&encoder->oggstream,&encoder->oggpage);
    }

    return result;
}

/**
 * The Vorbis specific implementation of {@link ATK_WriteEncoding}.
 *
 * @param encoding  The encoding stream
 * @param buffer    The data buffer with the audio samples
 * @param frames    The number of audio frames to write
 *
 * @return the number of frames written, or -1 on error
 */
Sint64 ATK_Vorbis_WriteEncoding(ATK_AudioEncoding* encoding, float* buffer, size_t frames) {
    if (encoding == NULL) {
        return -1;
    }

    ATK_Vorbis_Encoder* encoder = (ATK_Vorbis_Encoder*)encoding->encoder;

    size_t amt = frames;
    size_t off = 0;
    while (amt > 0) {
        size_t remain = amt > VORBIS_PAGESIZE ? VORBIS_PAGESIZE : amt;

        /* Expose the buffer to submit data */
        float **output=vorbis_analysis_buffer(&encoder->vdsp,(int)remain);

        /* Uninterleave samples */
        size_t ch = encoding->metadata.channels;
        for(size_t ii=0; ii < remain; ii++) {
            for(size_t jj = 0; jj < ch; jj++) {
                output[jj][ii] = buffer[ch*(ii+off)+jj];
            }
        }

        /* Tell the library how much we actually submitted */
        int err = vorbis_analysis_wrote(&encoder->vdsp,(int)remain);
        if (err) {
            ATK_SetError("Write error: %s",ogg_write_error(err));
            return off;
        }

        off += remain;
        amt -= remain;
    }

    return frames;
}

/**
 * The Vorbis specific implementation of {@link ATK_FinishEncoding}.
 *
 * @param encoding  The encoding stream
 *
 * @return 0 if the enconding was successfully completed, -1 otherwise.
 */
Sint32 ATK_Vorbis_FinishEncoding(ATK_AudioEncoding* encoding) {
    if (encoding == NULL) {
        return -1;
    }

    // End of stream
    ATK_Vorbis_Encoder* encoder = (ATK_Vorbis_Encoder*)encoding->encoder;
    vorbis_analysis_wrote(&encoder->vdsp,0);

    /* We are just going to do single block for encoding */
    int result = 0;
    while(!result && vorbis_analysis_blockout(&encoder->vdsp,&encoder->vblock)==1) {
        /* Analysis, assume we want to use bitrate management */
        vorbis_analysis(&encoder->vblock,NULL);
        vorbis_bitrate_addblock(&encoder->vblock);

        while(!result && vorbis_bitrate_flushpacket(&encoder->vdsp,&encoder->oggpacket)) {
            /* weld the packet into the bitstream */
            ogg_stream_packetin(&encoder->oggstream,&encoder->oggpacket);

            /* Write out pages */
            int avail = ogg_stream_pageout(&encoder->oggstream,&encoder->oggpage);
            while (!result && avail) {
                if (SDL_RWwrite(encoder->stream,encoder->oggpage.header,1,encoder->oggpage.header_len) < 0) {
                    result = -1;
                }
                if (SDL_RWwrite(encoder->stream,encoder->oggpage.body,1,encoder->oggpage.body_len) < 0) {
                    result = -1;
                }
                if (ogg_page_eos(&encoder->oggpage)) {
                    avail = 0;
                } else {
                    avail = ogg_stream_pageout(&encoder->oggstream,&encoder->oggpage);
                }
            }
        }
    }

    ATK_Vorbis_FreeEncoding(encoding);
    return result;
}

#else

#pragma mark -
#pragma mark Dummy Encoding
/**
 * Returns a new Vorbis encoding stream to write to the given file
 *
 * The provided metadata will be copied to the encoding object. So it is safe to delete
 * this metadata before the encoding is complete. If the encoding stream cannot be
 * allocated, or this library does not supported saving to Vorbis, this function will
 * return NULL and an error value will be set.
 *
 * The metadata should reflect the properties of the stream to be encoded as much as
 * possible. However, Vorbis allows the number of frames written to differ from that
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
 * @return a new Vorbis encoding stream to write to the given file
 */
ATK_AudioEncoding* ATK_EncodeVorbis(const char* filename,
                                  const ATK_AudioMetadata* metadata) {
    ATK_SetError("Codec Vorbis is not supported");
    return NULL;
}

/**
 * Returns a new Vorbis encoding stream to write to the given RWops
 *
 * The provided metadata will be copied to the encoding object. So it is safe to delete
 * this metadata before the encoding is complete. If the encoding stream cannot be
 * allocated, or this library does not supported saving to Vorbis, this function will
 * return NULL and an error value will be set.
 *
 * The metadata should reflect the properties of the stream to be encoded as much as
 * possible. However, Vorbis allows the number of frames written to differ from that
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
 * @return a new Vorbis encoding stream to write to the given RWops
 */
ATK_AudioEncoding* ATK_EncodeVorbis_RW(SDL_RWops* source, int ownsrc,
                                     const ATK_AudioMetadata* metadata) {
    ATK_SetError("Codec Vorbis is not supported");
    return NULL;
}

/**
 * The Vorbis specific implementation of {@link ATK_WriteEncoding}.
 *
 * @param encoding  The encoding stream
 * @param buffer    The data buffer with the audio samples
 * @param frames    The number of audio frames to write
 *
 * @return the number of frames written, or -1 on error
 */
Sint64 ATK_Vorbis_WriteEncoding(ATK_AudioEncoding* encoding, float* buffer, size_t frames) {
    return -1;
}

/**
 * The Vorbis specific implementation of {@link ATK_FinishEncoding}.
 *
 * @param encoding  The encoding stream
 *
 * @return 0 if the enconding was successfully completed, -1 otherwise.
 */
Sint32 ATK_Vorbis_FinishEncoding(ATK_AudioEncoding* encoding) {
    return -1;
}

#endif
