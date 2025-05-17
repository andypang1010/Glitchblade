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
#include <limits.h>
#include <string.h>
#include "ATK_Codec_c.h"

/**
 * @file ATK_CodecMP3.c
 *
 * This component contains the functions for loading (but not saving) MP3 files.
 */
#pragma mark -
#pragma mark MP3 Metadata
#ifdef LOAD_MP3
#define MINIMP3_IMPLEMENTATION
#include <minimp3.h>
#include <minimp3_ex.h>
#include <SDL.h>

/**
 * The supported ID3 tags
 *
 * Currently, we only support a core set of the text information frames
 */
const char* MP3_TAGS[] = {
    "TALB", "TBPM", "TCMP", "TCOM", "TCON", "TCOP", "TENC",
    "TIT1", "TIT2", "TIT3", "TLEN", "TPE1", "TPE2", "TPE3",
    "TPOS", "TPUB", "TRCK", "TDRC"
};

/**
 * The ID3 tag descriptions
 *
 * Currently, we only support a core set of the text information frames
 */
const char* MP3_VERBOSE[] = {
    "Album", "Beats Per Minute", "Compilation", "Composer", "Genre", "Copyright",
    "Encoder", "Grouping", "Title", "Subtitle", "Length", "Artist", "Band",
    "Conductor", "Disk", "Publisher", "Track", "Year"
};

/** The detection size for a tag */
#define ATK_ID3_DETECT_SIZE 10

/** The BOM markers for processing UCS2 */
#define UNICODE_BOM_NATIVE  0xFEFF
#define UNICODE_BOM_SWAPPED 0xFFFE

/**
 * Converts a UCS2 string to a UTF-8 string
 *
 * This code is taken from SDL_ttf
 *
 * @param src   The string to convert
 *
 * @return the UTF-8 equivalent
 */
static void UCS2_to_UTF8(const Uint16 *src, Uint8 *dst) {
    SDL_bool swapped = SDL_SwapBE32(UNICODE_BOM_NATIVE) == UNICODE_BOM_NATIVE;

    while (*src) {
        Uint16 ch = *src++;
        if (ch == UNICODE_BOM_NATIVE) {
            swapped = SDL_FALSE;
            continue;
        }
        if (ch == UNICODE_BOM_SWAPPED) {
            swapped = SDL_TRUE;
            continue;
        }
        if (swapped) {
            ch = SDL_Swap16(ch);
        }
        if (ch <= 0x7F) {
            *dst++ = (Uint8) ch;
        } else if (ch <= 0x7FF) {
            *dst++ = 0xC0 | (Uint8) ((ch >> 6) & 0x1F);
            *dst++ = 0x80 | (Uint8) (ch & 0x3F);
        } else {
            *dst++ = 0xE0 | (Uint8) ((ch >> 12) & 0x0F);
            *dst++ = 0x80 | (Uint8) ((ch >> 6) & 0x3F);
            *dst++ = 0x80 | (Uint8) (ch & 0x3F);
        }
    }
    *dst = '\0';
}

/**
 * Converts a ISO_8859_1 string to a UTF-8 string
 *
 * https://stackoverflow.com/questions/4059775/convert-iso-8859-1-strings-to-utf-8-in-c-c
 *
 * @param src   The string to convert
 *
 * @return the UTF-8 equivalent
 */
static void ISO_8859_1_to_UTF8(const Uint8 *src, Uint8* dst) {
    while (*src) {
        Uint8 ch = *src++;
        if (ch < 128) {
            *dst++ = ch;
        } else {
            *dst++ = 0xc2+(ch>0xbf);
            *dst++ = (ch & 0x3f)+0x80;
        }
    }
    *dst = '\0';
}

/**
 * Returns the ID3 tag equivalent to the given comment tag
 *
 * MP3 files use the ID3 specification for their metadata. However, to provide a
 * uniform comment interface, these tags are expanded into proper words matching
 * the Vorbis comment interface. This method returns the Vorbis comment equivalent
 * for an ID3 tag. For information on the ID3 specification see
 *
 * http://id3.org
 *
 * If tag is not correspond to a supported ID3 tag, this method returns NULL. In
 * particular, only tags for textual values are supported.
 *
 * @param tag   The comment tag
 *
 * @return the ID3 tag equivalent to the given comment tag
 */
static const char* ATK_GetCommentID3Tag(const char* tag) {
    size_t len = SDL_arraysize(MP3_TAGS);
    for(size_t ii = 0; ii < len; ii++) {
        if (ATK_string_equals(tag, MP3_VERBOSE[ii])) {
            return MP3_TAGS[ii];
        }
    }
    return NULL;
}

/**
 * Returns the comment tag equivalent to the given ID3 tag
 *
 * MP3 files use the ID3 specification for their metadata. However, to provide a
 * uniform comment interface, these tags are expanded into proper words matching
 * the Vorbis comment interface. This method returns the Vorbis comment equivalent
 * for an ID3 tag. For information on the ID3 specification see
 *
 * http://id3.org
 *
 * If tag is not a supported ID3 tag, this method returns NULL. In particular, only
 * tags for textual values are supported.
 *
 * @param tag   The ID3 tag
 *
 * @return the comment tag equivalent to the given ID3 tag
 */
const char* ATK_GetID3CommentTag(const char* tag) {
    size_t len = SDL_arraysize(MP3_TAGS);
    Uint32 code1 = *((Uint32*)tag);
    for(size_t ii = 0; ii < len; ii++) {
        Uint32 code2 = *((Uint32*)MP3_TAGS[ii]);
        if (code1 == code2) {
            return MP3_VERBOSE[ii];
        }
    }
    return NULL;
}

/**
 * Returns a UTF-8 string for the data at the given ID3 block
 *
 * @param data  The ID3 block
 * @param len   The block length
 *
 * @return a UTF-8 string for the data at the given ID3 block
 */
static const char* GetID3v2Value(const char* data, size_t len) {
    if (len == 0) {
        return NULL;
    }

    Uint8* src = (Uint8*)ATK_malloc(len*sizeof(Uint8));
    memcpy(src, data+1, len-1);
    src[len-1] = '\0';

    // Now extract as a UTF8 string
    if (*data == 0) {
        Uint8* dst = (Uint8*)ATK_malloc(2*len*sizeof(Uint8));
        ISO_8859_1_to_UTF8(src,dst);
        ATK_free(src);
        size_t amt = strlen((char*)dst);
        src = ATK_realloc(dst, amt+1);
        return (char*)(src == NULL ? dst : src);
    } else if (*data == 1) {
        Uint8* dst = (Uint8*)ATK_malloc(2*len*sizeof(Uint8));
        UCS2_to_UTF8((Uint16*)src,dst);
        ATK_free(src);
        size_t amt = strlen((char*)dst);
        dst = ATK_realloc(dst, amt+1);
        return (char*)(src == NULL ? dst : src);
    } else if (*data == 3) {
        return (char*)src;
    }

    ATK_free(src);
    return NULL;
}

/**
 * Returns the size of the entire ID3 block
 *
 * If the source is not at the start of the ID3 block, this returns 0.
 *
 * @param source    The MP3 source
 *
 * @return the size of the entire ID3 block
 */
static size_t GetID3v2Length(SDL_RWops* source) {
    char buffer[ATK_ID3_DETECT_SIZE];
    size_t amt = SDL_RWread(source, buffer, ATK_ID3_DETECT_SIZE, 1);
    if (!amt) {
        return 0;
    }

    if (memcmp(buffer, "ID3", 3) || ((buffer[5] & 15) || (buffer[6] & 0x80) ||
                                     (buffer[7] & 0x80) || (buffer[8] & 0x80) ||
                                     (buffer[9] & 0x80))) {
        return 0;
    }

    size_t result = (((buffer[6] & 0x7f) << 21) |
                     ((buffer[7] & 0x7f) << 14) |
                     ((buffer[8] & 0x7f) << 7) |
                     (buffer[9] & 0x7f));

    if ((buffer[5] & 16)) {
        result += ATK_ID3_DETECT_SIZE; /* footer */
    }

    return result;
}

/**
 * Returns SDL_TRUE if MP3 supports the given comment tag.
 *
 * @param tag   The tag to query
 *
 * @return SDL_TRUE if MP3 supports the given comment tag.
 */
SDL_bool ATK_MP3_SupportsCommentTag(const char* tag) {
    size_t len = SDL_arraysize(MP3_VERBOSE);
    for(size_t ii = 0; ii < len; ii++) {
        if (ATK_string_equals(tag, MP3_VERBOSE[ii])) {
            return SDL_TRUE;
        }
    }
    return SDL_FALSE;
}

/**
 * Returns an array of comment tags supported by the MP3 codec
 *
 * @return an array of comment tags supported by the MP3 codec
 */
const char* const * ATK_MP3_GetCommentTags() {
    return MP3_VERBOSE;
}


/**
 * Returns a newly allocated list of metadata comments.
 *
 * The comments are parsed using the ID3 specification. See https://id3.org
 * The number of comments in the returned array are stored in the len parameter.

 * @param source    The MP3 source
 * @param len       Pointer to store the comment length
 *
 * @return a newly allocated list of metadata comments.
 */
static ATK_AudioComment* ATK_MP3_AllocComments(SDL_RWops* source, int* len) {
    size_t amt = GetID3v2Length(source);
    char* buffer = ATK_malloc(sizeof(char)*amt);
    if (!SDL_RWread(source, buffer, amt, 1)) {
        ATK_free(buffer);
        return NULL;
    }

    // Count the number of valid keys
    size_t num = 0;
    size_t pos = 0;
    for(size_t ii = 0; pos < amt && ii < amt-4; ii++) {
        Uint32 size = SDL_SwapBE32(*(Uint32*)(buffer+pos+4));
        if (size) {
            const char* key = ATK_GetID3CommentTag(buffer+pos);
            if (key) {
                num++;
            }
            pos += size+ATK_ID3_DETECT_SIZE;
        }
    }
    if (!num) {
        return NULL;
    }

    // Now make the tags
    ATK_AudioComment* result = (ATK_AudioComment*)ATK_malloc(sizeof(ATK_AudioComment)*num);

    size_t off = 0;
    pos = 0;
    for(size_t ii = 0; pos < amt && ii < amt-4; ii++) {
        Uint32 size = SDL_SwapBE32(*(Uint32*)(buffer+pos+4));
        if (size) {
            const char* key = ATK_GetID3CommentTag(buffer+pos);
            if (key) {
                result[off].key = (char*)ATK_malloc(sizeof(char)*(strlen(key)+1));
                strcpy(DECONST(char*,result[off].key),key);
                result[off].value = GetID3v2Value(buffer+pos+ATK_ID3_DETECT_SIZE, size);
                off++;
            }
            pos += size+ATK_ID3_DETECT_SIZE;
        }
    }

    SDL_RWseek(source, 0, RW_SEEK_SET);
    *len = (int)num;
    return result;
}

#pragma mark -
#pragma mark MP3 Decoder
/**
 * The stream abstraction for preocessing MP3s
 */
typedef struct {
	SDL_RWops* source;
	mp3dec_ex_t context;
	mp3dec_io_t stream;
} MP3Stream;

/**
 * The internal structure for decoding
 */
typedef struct {
    /** The MPEG decoder struct */
    MP3Stream* converter;
    /** Whether this object owns the underlying stream */
    int ownstream;
    /** A buffer to store the decoding data */
    Sint16* buffer;

    /** The size of a decoder chunk */
    Uint32 pagesize;
    /** The size of the first page*/
    Uint32 firstpage;
    /** The current page in the stream */
    Uint32 currpage;
    /** The previous page in the stream */
    Uint32 lastpage;

} ATK_MP3_Decoder;

/**
 * The wrapper to read from the IO stream
 *
 * @param buf		The buffer to hold the data read
 * @param size		The number of bytes to read
 * @param user_data	The data stream to read from
 *
 * @return the number of bytes read
 */
size_t read_stream(void *buf, size_t size, void *user_data) {
	SDL_RWops *stream = (SDL_RWops*)user_data;
	return SDL_RWread(stream,buf,1,size);
}

/**
 * The wrapper to seek the IO stream
 *
 * @param position	The byte offset to seek
 * @param user_data	The data stream to read from
 *
 * @return 0 if the seek is successful
 */
int seek_stream(uint64_t position, void *user_data) {
	SDL_RWops *stream = (SDL_RWops*)user_data;
    return (SDL_RWseek(stream,position,RW_SEEK_SET) < 0 ? 1 : 0);
}

/**
 * Allocates a new MP3 stream for the given file
 *
 * The file must be in the current working directory, or else this will return NULL.
 * It the responsibility of the user to free this stream when done.
 *
 * @param source	The RWops for the MP3 data
 *
 * @return a new MP3 stream for the given file
 */
MP3Stream* MP3Stream_Alloc(SDL_RWops *source) {
	if (source == NULL) {
		return NULL;
	}

	MP3Stream* result = (MP3Stream*)ATK_malloc(sizeof(MP3Stream));

	memset(result,0,sizeof(MP3Stream));
	result->source = source;
	result->stream.read_data = source;
	result->stream.seek_data = source;
	result->stream.read = read_stream;
	result->stream.seek = seek_stream;

	int error = mp3dec_ex_open_cb(&(result->context), &(result->stream), MP3D_SEEK_TO_SAMPLE);
	if (error) {
		ATK_free(result);
        return NULL;
	}
	return result;
}

/**
 * Frees the given MP3 stream
 *
 * It the responsibility of the user to free any allocated stream when done.
 *
 * @param stream	The MP3 stream to free
 *
 * @return 0 if successful
 */
Sint32 MP3Stream_Free(MP3Stream* stream) {
    if (stream == NULL) {
        ATK_SetError("Attempt to access a NULL MP3 stream");
        return -1;
	} else if (stream->source == NULL) {
        ATK_SetError("MP3 stream has been corrupted");
        return -1;
	}
    stream->source = NULL;
	ATK_free(stream);
	return 0;
}

/**
 * Returns 1 if this MP3 stream is stereo
 *
 * MP3 files can only either be mono or stereo.
 *
 * @param stream	The MP3 stream
 *
 * @return 1 if stereo, 0 if mono
 */
Uint32 MP3Stream_IsStereo(MP3Stream* stream) {
	if (stream == NULL) {
		return 0;
	}
	return stream->context.info.channels == 2;
}

/**
 * Returns the sample rate for this MP3 stream
 *
 * @param stream	The MP3 stream
 *
 * @return the sample rate for this MP3 stream
 */
Uint32 MP3Stream_GetFrequency(MP3Stream* stream) {
	if (stream == NULL) {
		return 0;
	}
	return stream->context.info.hz;
}

/**
 * Returns the last page for this MP3 stream
 *
 * @param stream	The MP3 stream
 *
 * @return the last page for this MP3 stream
 */
Uint32 MP3Stream_GetLastPage(MP3Stream* stream) {
	if (stream == NULL) {
		return 0;
	}
	return (int)(stream->context.samples/MINIMP3_MAX_SAMPLES_PER_FRAME);
}

/**
 * Returns the size of this MP3 stream in audio frames
 *
 * An audio frame (not to be confused with a page) is a collection of simultaneous
 * samples for all of the different channels.
 *
 * @param stream	The MP3 stream
 *
 * @return the size of this MP3 stream in audio frames
 */
Uint32 MP3Stream_GetLength(MP3Stream* stream) {
	if (stream == NULL) {
		return 0;
	}
	return (int)stream->context.samples/stream->context.info.channels;
}

/**
 * Sets the current sample for the MP3 stream
 *
 * @param stream    The MP3 stream
 * @param sample    The sample to seek
 *
 * @return 0 on success; error code on failure
 */
Uint32 MP3Stream_SetSample(MP3Stream* stream, size_t sample) {
    if (stream == NULL) {
        return 0;
    }
    return mp3dec_ex_seek(&(stream->context),sample);
}

/**
 * Reads in a page of MP3 data.
 *
 * This function reads in the current stream page into the buffer. The data written
 * into the buffer is linear PCM data with interleaved channels. If the stream is
 * at the end, nothing will be written.
 *
 * The size of a page is given by {@link MP3Stream_GetPageSize}. This buffer should
 * large enough to hold this data. As the page size is in audio frames, that means
 * the buffer should be pagesize * # of channels * sizeof(Sint16) bytes.
 *
 * @param stream	The MP3 stream
 * @param buffer	The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
Sint64 MP3Stream_ReadPage(MP3Stream* stream, Sint16 *buffer) {
	if (stream == NULL) {
		return 0;
	}
    mp3dec_frame_info_t frame_info;
    memset(&frame_info, 0, sizeof(frame_info));
	mp3d_sample_t *buf_frame = NULL;
	size_t read_samples = mp3dec_ex_read_frame(&(stream->context), &buf_frame, &frame_info, stream->context.samples);
	if (!read_samples) {
        ATK_SetError("Unable to read frame");
        return -1;
	}

	memcpy(buffer, buf_frame, read_samples * sizeof(mp3d_sample_t));
	return (Sint64)read_samples;
}

/**
 * Reads in the entire MP3 stream into the buffer.
 *
 * The data written into the buffer is linear PCM data with interleaved channels. If the
 * stream is not at the initial page, it will rewind before writing the data. It will
 * restore the stream to the initial page when done.
 *
 * The buffer needs to be large enough to hold the entire MP3 stream. As the length is
 * measured in audio frames, this means that the buffer should be
 * length * # of channels * sizeof(Sint16) bytes.
 *
 * @param stream	The MP3 stream
 * @param buffer	The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
Sint64 MP3Stream_Read(MP3Stream* stream, Sint16 *buffer) {
	return mp3dec_ex_read(&(stream->context),buffer,stream->context.samples);
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
 * @param decoder   The MP3 decoder
 * @param buffer    The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
Sint32 mpeg_read_page(ATK_AudioSource* source, ATK_MP3_Decoder* decoder, float* buffer) {
	Sint32 amount = (Sint32)MP3Stream_ReadPage(decoder->converter,decoder->buffer);
	if (amount < 0) {
		return -1;
	}
	double factor = 1.0/(1 << 15);

    Uint32 temp = amount;
    float* output = buffer;
    short* input  = decoder->buffer;
    while (temp--) {
        *output = (float)((*input)*factor);
        output++;
        input++;
    }
	decoder->currpage++;
    return amount/source->metadata.channels;
}


#pragma mark ATK Functions
/**
 * Creates a new ATK_AudioSource from an MP3 file
 *
 * This function will return NULL if the file cannot be located or is not an
 * supported MP3 file. SDL_atk's MP3 support is minimal, so some advanced encodings
 * may not be supported. The file will not be read into memory, but is instead
 * available for streaming.
 *
 * It is the responsibility of the caller of this function to close the source
 * (with {@link ATK_UnloadSource}) when it is no longer needed.
 *
 * @param filename    The name of the file with audio data
 *
 * @return a new ATK_AudioSource from an MP3 file
 */
ATK_AudioSource* ATK_LoadMP3(const char* filename) {
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
    return ATK_LoadMP3_RW(stream,1);
}

/**
 * Creates a new ATK_AudioSource from an MP3 readable/seekable RWops
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
 * @return a new ATK_AudioSource from an MP3 readable/seekable RWops
 */
ATK_AudioSource* ATK_LoadMP3_RW(SDL_RWops* source, int ownsrc) {
    if (source == NULL) {
        ATK_SetError("NULL source data");
        return NULL;
    }

    // Read the metadata FIRST
    int num_comments = 0;
    ATK_AudioComment* comments = ATK_MP3_AllocComments(source,&num_comments);

    MP3Stream* converter = MP3Stream_Alloc(source);
    if (converter == NULL) {
        ATK_SetError("MP3 metadata not found");
        if (ownsrc) {
            SDL_RWclose(source);
        }
        if (num_comments) {
            ATK_FreeComments(comments, num_comments);
        }
        return NULL;
    }

    ATK_MP3_Decoder* decoder  = (ATK_MP3_Decoder*)ATK_malloc(sizeof(ATK_MP3_Decoder));
    if (!decoder) {
        ATK_OutOfMemory();
        MP3Stream_Free(converter);
        if (ownsrc) {
            SDL_RWclose(source);
        }
        if (num_comments) {
            ATK_FreeComments(comments, num_comments);
        }
        return NULL;
    }
    ATK_AudioSource* result = (ATK_AudioSource*)ATK_malloc(sizeof(ATK_AudioSource));
    if (!result) {
        ATK_OutOfMemory();
        MP3Stream_Free(converter);
        ATK_free(decoder);
        if (ownsrc) {
            SDL_RWclose(source);
        }
        if (num_comments) {
            ATK_FreeComments(comments, num_comments);
        }
        return NULL;
    }
    memset(decoder,0,sizeof(ATK_MP3_Decoder));
    memset(result,0,sizeof(ATK_AudioSource));

    DECONST(Uint8,result->metadata.channels) = MP3Stream_IsStereo(converter) ? 2 : 1;
    DECONST(Uint32,result->metadata.rate)    = MP3Stream_GetFrequency(converter);
    DECONST(Uint64,result->metadata.frames)  = MP3Stream_GetLength(converter);
    DECONST(ATK_AudioComment*,result->metadata.comments)  = comments;
    DECONST(Uint16,result->metadata.num_comments) = num_comments;
    decoder->pagesize = MINIMP3_MAX_SAMPLES_PER_FRAME/converter->context.info.channels;
    decoder->currpage = 0;

    Sint16* buffer = (Sint16*)ATK_malloc(sizeof(Sint16)*decoder->pagesize*result->metadata.channels);
    if (buffer == NULL) {
        ATK_OutOfMemory();
        MP3Stream_Free(converter);
        ATK_free(decoder);
        ATK_free(result);
        if (ownsrc) {
            SDL_RWclose(source);
        }
        return NULL;
    }
    memset(buffer,0,sizeof(Sint16)*decoder->pagesize*result->metadata.channels);

    decoder->buffer = buffer;
    decoder->converter = converter;
    decoder->ownstream = ownsrc;
    result->decoder = decoder;
    *(ATK_CodecType*)(&result->type) = ATK_CODEC_MP3;

    // Find the size of the first page
    decoder->firstpage = (Uint32) MP3Stream_ReadPage(converter,buffer)/result->metadata.channels;

    // Reset to start of stream
    mp3dec_ex_seek(&(converter->context),0);

    Uint32 total = (Uint32)(result->metadata.frames + decoder->pagesize - decoder->firstpage);
    decoder->lastpage = (Uint32)(total/decoder->pagesize);
    if (total % decoder->pagesize != 0) {
        decoder->lastpage++;
    }

    return result;
}

/**
 * Detects MP3 data on a readable/seekable SDL_RWops.
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
 * @returns SDL_true if this is MP3 data.
 */
SDL_bool ATK_SourceIsMP3(SDL_RWops* source) {
    if (source == NULL) {
        return SDL_FALSE;
    }
    Sint64 pos = SDL_RWtell(source);
    MP3Stream* converter = MP3Stream_Alloc(source);
    if (converter == NULL) {
        return SDL_FALSE;
    }
    SDL_bool result = converter->context.samples > 0;
    MP3Stream_Free(converter);
    ATK_ClearError();
    SDL_RWseek(source, pos, RW_SEEK_SET);
    return result;
}

/**
 * The MP3 specific implementation of {@link ATK_UnloadSource}.
 *
 * @param source    The source to close
 *
 * @return 0 if the source was successfully closed, -1 otherwise.
 */
Sint32 ATK_MP3_UnloadSource(ATK_AudioSource* source) {
	CHECK_SOURCE(source,-1)
    if (source->metadata.comments != NULL) {
        ATK_FreeComments(DECONST(ATK_AudioComment*,source->metadata.comments),
                         source->metadata.num_comments);
        DECONST(ATK_AudioComment*,source->metadata.comments) = NULL;
    }

    ATK_MP3_Decoder* decoder = (ATK_MP3_Decoder*)(source->decoder);
    if (decoder->converter != NULL) {
	    MP3Stream_Free(decoder->converter);
	    decoder->converter = NULL;
	}
    if (decoder->buffer != NULL) {
        ATK_free(decoder->buffer);
        decoder->buffer = NULL;
    }
    ATK_free(decoder);
	source->decoder = NULL;
    ATK_free(source);
    return 0;
}

/**
 * The MP3 specific implementation of {@link ATK_SeekSourcePage}.
 *
 * @param source    The audio source
 * @param page      The page to seek
 *
 * @return the page acquired, or -1 if there is an error
 */
Sint32 ATK_MP3_SeekSourcePage(ATK_AudioSource* source, Uint32 page) {
	CHECK_SOURCE(source,-1)

    ATK_MP3_Decoder* decoder = (ATK_MP3_Decoder*)(source->decoder);
    if (page*decoder->pagesize > source->metadata.frames) {
        page = MP3Stream_GetLastPage(decoder->converter);
    }

    size_t sample = (page == 0) ? 0 : decoder->firstpage+(page-1)*decoder->pagesize;
    sample *= decoder->converter->context.info.channels;
    if (MP3Stream_SetSample(decoder->converter,sample)) {
        return -1;
    }
    decoder->currpage = page;
    return page;
}

/**
 * The MP3 specific implementation of {@link ATK_GetSourcePageSize}.
 *
 * @param source    The audio source
 *
 * @return the number of audio frames in a page, or -1 if there is an error
 */
Sint32 ATK_MP3_GetSourcePageSize(ATK_AudioSource* source) {
    CHECK_SOURCE(source,-1)
    return ((ATK_MP3_Decoder*)(source->decoder))->pagesize;
}

/**
 * The MP3 specific implementation of {@link ATK_GetSourceFirstPageSize}.
 *
 * @param source    The audio source
 *
 * @return the number of audio frames on the first page, or -1 if there is an error
 */
Sint32 ATK_MP3_GetSourceFirstPageSize(ATK_AudioSource* source) {
    CHECK_SOURCE(source,-1)
    return ((ATK_MP3_Decoder*)(source->decoder))->firstpage;
}

/**
 * The MP3 specific implementation of {@link ATK_GetSourceLastPage}.
 *
 * @param source    The audio source
 *
 * @return the index of the last page in the stream, or -1 if there is an error
 */
Sint32 ATK_MP3_GetSourceLastPage(ATK_AudioSource* source) {
    CHECK_SOURCE(source,-1)
    return ((ATK_MP3_Decoder*)(source->decoder))->lastpage;
}

/**
 * The MP3 specific implementation of {@link ATK_GetSourceCurrentPage}.
 *
 * @param source    The audio source
 *
 * @return the index of the current page in the stream, or -1 if there is an error
 */
Sint32 ATK_MP3_GetSourceCurrentPage(ATK_AudioSource* source) {
    CHECK_SOURCE(source,-1)
    return ((ATK_MP3_Decoder*)(source->decoder))->currpage;
}

/**
 * The MP3 specific implementation of {@link ATK_IsSourceEOF}.
 *
 * @param source    The audio source
 *
 * @return 1 if the audio source is at the end of the stream; 0 otherwise
 */
Uint32 ATK_MP3_IsSourceEOF(ATK_AudioSource* source) {
    CHECK_SOURCE(source,0)
    ATK_MP3_Decoder* decoder = (ATK_MP3_Decoder*)(source->decoder);
    return (decoder->currpage == decoder->lastpage);
}

/**
 * The MP3 specific implementation of {@link ATK_ReadSourcePage}.
 *
 * @param source    The audio source
 * @param buffer    The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
Sint32 ATK_MP3_ReadSourcePage(ATK_AudioSource* source, float* buffer) {
	CHECK_SOURCE(source,-1)

    ATK_MP3_Decoder* decoder = (ATK_MP3_Decoder*)(source->decoder);
	if (decoder->currpage < decoder->lastpage) {
    	return mpeg_read_page(source,decoder,buffer);
    }
    return 0;
}

/**
 * The MP3 specific implementation of {@link ATK_ReadSource}.
 *
 * @param source    The audio source
 * @param buffer    The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
Sint64 ATK_MP3_ReadSource(ATK_AudioSource* source, float* buffer) {
    CHECK_SOURCE(source,-1)

    ATK_MP3_Decoder* decoder = (ATK_MP3_Decoder*)(source->decoder);
    Uint32 currpage = decoder->currpage;
    if (currpage != 0) {
        MP3Stream_SetSample(decoder->converter,0);
    }

    Sint32 amt  = 0;
	Sint64 read = 0;
	Sint64 offset = 0;
	while (decoder->currpage < decoder->lastpage) {
		amt = mpeg_read_page(source,decoder,buffer+offset);
		read += amt;
		offset += amt*source->metadata.channels;
	}

    if (currpage != 0) {
        size_t sample = (currpage == 0) ? 0 : decoder->firstpage+(currpage-1)*decoder->pagesize;
        MP3Stream_SetSample(decoder->converter,sample);
    }
    return read;
}

#else
#pragma mark -
#pragma mark Dummy Functions
/**
 * Creates a new ATK_AudioSource from an MP3 file
 *
 * This function will return NULL if the file cannot be located or is not an
 * supported MP3 file. SDL_atk's MP3 support is minimal, so some advanced encodings
 * may not be supported. The file will not be read into memory, but is instead
 * available for streaming.
 *
 * It is the responsibility of the caller of this function to close the source
 * (with {@link ATK_UnloadSource}) when it is no longer needed.
 *
 * @param filename    The name of the file with audio data
 *
 * @return a new ATK_AudioSource from an MP3 file
 */
ATK_AudioSource* ATK_LoadMP3(const char* filename) {
    ATK_SetError("Codec MP3 is not supported");
    return NULL;
}

/**
 * Creates a new ATK_AudioSource from an MP3 readable/seekable RWops
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
 * @return a new ATK_AudioSource from an MP3 readable/seekable RWops
 */
ATK_AudioSource* ATK_LoadMP3_RW(SDL_RWops* source, int ownsrc) {
    ATK_SetError("Codec MP3 is not supported");
    return NULL;
}

/**
 * Creates a new ATK_AudioSource from an MP3 file
 *
 * This function will return NULL if the file cannot be located or is not an
 * supported MP3 file. SDL_atk's MP3 support is minimal, so some advanced encodings
 * may not be supported. The file will not be read into memory, but is instead
 * available for streaming.
 *
 * It is the responsibility of the caller of this function to close the source
 * (with {@link ATK_UnloadSource}) when it is no longer needed.
 *
 * @param filename    The name of the file with audio data
 *
 * @return a new ATK_AudioSource from an MP3 file
 */
ATK_AudioSource* ATK_MP3_LoadSource(const char* filename) {
    ATK_SetError("Codec MP3 is not supported");
    return NULL;
}

/**
 * Creates a new ATK_AudioSource from an MP3 readable/seekable RWops
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
 * @return a new ATK_AudioSource from an MP3 readable/seekable RWops
 */
ATK_AudioSource* ATK_MP3_LoadSource_RW(SDL_RWops* source, int ownsrc) {
    ATK_SetError("Codec MP3 is not supported");
    return NULL;

}

/**
 * Detects MP3 data on a readable/seekable SDL_RWops.
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
 * @returns SDL_true if this is MP3 data.
 */
SDL_bool ATK_SourceIsMP3(SDL_RWops* source) { return SDL_FALSE; }

/**
 * The MP3 specific implementation of {@link ATK_UnloadSource}.
 *
 * @param source    The source to close
 *
 * @return 0 if the source was successfully closed, -1 otherwise.
 */
Sint32 ATK_MP3_UnloadSource(ATK_AudioSource* source) { return -1; }

/**
 * The MP3 specific implementation of {@link ATK_SeekSourcePage}.
 *
 * @param source    The audio source
 * @param page      The page to seek
 *
 * @return the page acquired, or -1 if there is an error
 */
Sint32 ATK_MP3_SeekSourcePage(ATK_AudioSource* source, Uint32 page) { return -1; }

/**
 * The MP3 specific implementation of {@link ATK_GetSourcePageSize}.
 *
 * @param source    The audio source
 *
 * @return the number of audio frames in a page, or -1 if there is an error
 */
Sint32 ATK_MP3_GetSourcePageSize(ATK_AudioSource* source) { return -1; }

/**
 * The MP3 specific implementation of {@link ATK_GetSourceFirstPageSize}.
 *
 * @param source    The audio source
 *
 * @return the number of audio frames on the first page, or -1 if there is an error
 */
Sint32 ATK_MP3_GetSourceFirstPageSize(ATK_AudioSource* source) { return -1; }

/**
 * The MP3 specific implementation of {@link ATK_GetSourceLastPage}.
 *
 * @param source    The audio source
 *
 * @return the index of the last page in the stream, or -1 if there is an error
 */
Sint32 ATK_MP3_GetSourceLastPage(ATK_AudioSource* source) { return -1; }

/**
 * The MP3 specific implementation of {@link ATK_GetSourceCurrentPage}.
 *
 * @param source    The audio source
 *
 * @return the index of the current page in the stream, or -1 if there is an error
 */
Sint32 ATK_MP3_GetSourceCurrentPage(ATK_AudioSource* source) { return -1; }

/**
 * The MP3 specific implementation of {@link ATK_IsSourceEOF}.
 *
 * @param source    The audio source
 *
 * @return 1 if the audio source is at the end of the stream; 0 otherwise
 */
Uint32 ATK_MP3_IsSourceEOF(ATK_AudioSource* source) { return 0; }

/**
 * The MP3 specific implementation of {@link ATK_ReadSourcePage}.
 *
 * @param source    The audio source
 * @param buffer    The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
Sint32 ATK_MP3_ReadSourcePage(ATK_AudioSource* source, float* buffer) { return -1; }

/**
 * The MP3 specific implementation of {@link ATK_ReadSource}.
 *
 * @param source    The audio source
 * @param buffer    The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
Sint64 ATK_MP3_ReadSource(ATK_AudioSource* source, float* buffer) { return -1; }

/**
 * Returns SDL_TRUE if MP3 supports the given comment tag.
 *
 * @param tag   The tag to query
 *
 * @return SDL_TRUE if MP3 supports the given comment tag.
 */
SDL_bool ATK_MP3_SupportsCommentTag(const char* tag) { return SDL_FALSE; }

/**
 * Returns an array of comment tags supported by the MP3 codec
 *
 * @return an array of comment tags supported by the MP3 codec
 */
const char* const* ATK_MP3_GetCommentTags() { return NULL; }

#endif
