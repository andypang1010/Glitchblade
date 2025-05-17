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

/* These are the common file functions. It mostly dispatches on type */
#include <ATK_codec.h>
#include "ATK_Codec_c.h"
#include <ctype.h>

/**
 * Returns the (string) name for a given source type
 *
 * This function is typically used for debugging.
 *
 * @param type    The source type
 *
 * @return the (string) name for the type
 */
const char* ATK_CodecNameForType(ATK_CodecType type) {
    switch(type) {
        case ATK_CODEC_WAV:
            return "WAV";
        case ATK_CODEC_VORBIS:
            return "OGG Vorbis";
        case ATK_CODEC_FLAC:
            return "Xiph FLAC";
        case ATK_CODEC_MP3:
            return "MP3";
    }
    return "unknown";
}

/**
 * Returns nonzero if strings are equal; otherwise 0
 *
 * @param str1  The first string to compare
 * @param str2  The second string to compare
 *
 * @return nonzero if strings are equal; otherwise 0
 */
int ATK_string_equals(const char *str1, const char *str2) {
    while ( *str1 && *str2 ) {
        if ( toupper((unsigned char)*str1) !=
             toupper((unsigned char)*str2) )
            break;
        ++str1;
        ++str2;
    }
    return (!*str1 && !*str2);
}

#pragma mark -
#pragma mark Stream Metadata
/**
 * Returns SDL_true if the code type supports metadata comments
 *
 * This function tests whether the coded supports any metadata comments at all.
 * Some formats, like WAV, may supported comments but have a very limited tag set.
 *
 * @param type  The codec type to query
 *
 * @return SDL_true if the code type supports metadata comments
 */
extern DECLSPEC SDL_bool SDLCALL ATK_SupportsComments(ATK_CodecType type) {
    switch(type) {
    case ATK_CODEC_WAV:
    case ATK_CODEC_VORBIS:
    case ATK_CODEC_FLAC:
    case ATK_CODEC_MP3:
        return SDL_TRUE;
    }

    // Currently everything supports (limited) comments
    ATK_SetError("Unrecognized codec type: %d",type);
    return SDL_FALSE;
}

/** Array representing all comments supported */
static const char* const ATK_ALL_COMMENTS[] = { "" };

/**
 * Returns an array of comment tags supported by this codec type
 *
 * If the type supports all tags (as is the case with Vorbis comment files),
 * it will return an array with the empty string as a single element. If the
 * type does not support comments at all, it will return NULL.
 *
 * @param type  The codec type to query
 *
 * @return an array of comment tags supported by this codec type
 */
const char* const *ATK_GetCommentTags(ATK_CodecType type) {
    switch(type) {
    case ATK_CODEC_WAV:
        return ATK_WAV_GetCommentTags();
    case ATK_CODEC_VORBIS:
    case ATK_CODEC_FLAC:
        return ATK_ALL_COMMENTS;
    case ATK_CODEC_MP3:
        return ATK_MP3_GetCommentTags();
    }

    ATK_SetError("Unrecognized codec type: %d",type);
    return NULL;

}

/**
 * Returns SDL_TRUE if the codec supports the given comment tag.
 *
 * Many codecs, particularly those that implement Vorbis comment, support all
 * tags. However, other codecs may only support a limited number of tags.
 *
 * @param type  The codec type
 * @param tag   The tag to query
 *
 * @return SDL_TRUE if the codec supports the given comment tag.
 */
SDL_bool ATK_SupportsCommentTag(ATK_CodecType type, const char* tag) {
    switch(type) {
    case ATK_CODEC_WAV:
        return ATK_WAV_SupportsCommentTag(tag);
    case ATK_CODEC_VORBIS:
    case ATK_CODEC_FLAC:
        return SDL_TRUE;
    case ATK_CODEC_MP3:
        return ATK_MP3_SupportsCommentTag(tag);
    }

    ATK_SetError("Unrecognized codec type: %d",type);
    return SDL_FALSE;
}


/**
 * Returns a comment array for the given key-value pairs
 *
 * The two string arrays should be of the same length. The audio comments will
 * allocate space for the strings so that they store a local copy. Hence it is
 * safe for these arrays to be volatile.
 *
 * @param tag       The comment tag names
 * @param values    The comment values
 * @param len       The size of the tag/value arrays
 *
 * @return a comment array for the given key-value pairs
 */
ATK_AudioComment* ATK_AllocComments(const char** tags, const char** values, size_t len) {
    ATK_AudioComment* comments = (ATK_AudioComment*)ATK_malloc(sizeof(ATK_AudioComment)*len);
    if (comments == NULL) {
        ATK_OutOfMemory();
        return NULL;
    }

    memset(comments, 0, sizeof(ATK_AudioComment)*len);
    for(size_t ii = 0; ii < len; ii++) {
        size_t slen;
        char*  str;

        slen = strlen(tags[ii]);
        str = (char*)ATK_malloc(sizeof(char)*(slen+1));
        strcpy(str, tags[ii]);
        comments[ii].key = str;

        slen = strlen(values[ii]);
        str = (char*)ATK_malloc(sizeof(char)*(slen+1));
        strcpy(str, values[ii]);
        comments[ii].value = str;
    }

    return comments;
}

/**
 * Frees a previously allocated collection of metadata comments
 *
 * @param comments  The metadata comment list
 */
void ATK_FreeComments(ATK_AudioComment* comments, size_t len) {
    if (comments == NULL) {
        return;
    }

    for(size_t ii = 0; ii < len; ii++) {
        ATK_AudioComment* curr = comments+ii;
        if (curr->key != NULL) {
            ATK_free((void*)(curr->key));
        }
        if (curr->value != NULL) {
            ATK_free((void*)(curr->value));
        }
    }
    ATK_free((void*)comments);
}

/**
 * Returns a copy of the comments array.
 *
 * This method returns a newly allocated array. It is the responsibility of this caller
 * to free these comments (with {@link ATK_FreeComments}) when done.
 *
 * @param comments    The array of comments
 * @param len        The size of the array
 *
 * @return a copy of the comments array.
 */
ATK_AudioComment* ATK_CopyComments(const ATK_AudioComment* comments, size_t len) {
    if (comments == NULL) {
        return NULL;
    }

    ATK_AudioComment* result = (ATK_AudioComment*)ATK_malloc(sizeof(ATK_AudioComment)*len);
    memset(result, 0, sizeof(ATK_AudioComment)*len);
    for(size_t ii = 0; ii < len; ii++) {
        size_t slen;
        char*  str;

        slen = strlen(comments[ii].key);
        str = (char*)ATK_malloc(sizeof(char)*(slen+1));
        strcpy(str, comments[ii].key);
        result[ii].key = str;

        slen = strlen(comments[ii].value);
        str = (char*)ATK_malloc(sizeof(char)*(slen+1));
        strcpy(str, comments[ii].value);
        result[ii].value = str;
    }

    return result;
}

/**
 * Returns a metadata struct for the given attributes
 *
 * As all of the attributes of ATK_AudioMetadata are constant, it is much
 * easier to allocate metadata struct this way.
 *
 * This function does not specify whether the metdata struct obtains ownership
 * of the comments. This decision can be made at the time the struct is freed
 * via {@link ATK_FreeMetadata}.
 *
 * @param channels  The number of channels (max 32)
 * @param rate      The sampling rate (frequency)
 * @param frames    The number of audio rames
 * @param comments  An array of metadata comments
 * @param len       The number of metadata comments
 *
 * @return a metadata struct for the given attributes
 */
ATK_AudioMetadata* ATK_AllocMetadata(Uint8 channels, Uint32 rate, Uint64 frames,
                                     const ATK_AudioComment* comments, Uint16 len) {
    ATK_AudioMetadata* metadata = (ATK_AudioMetadata*)ATK_malloc(sizeof(ATK_AudioMetadata));
    if (metadata == NULL) {
        ATK_OutOfMemory();
        return NULL;
    }

    memset(metadata, 0, sizeof(ATK_AudioMetadata));
    DECONST(Uint16,metadata->channels) = channels;
    DECONST(Uint32,metadata->rate) = rate;
    DECONST(Uint64,metadata->frames) = frames;
    DECONST(Uint16,metadata->num_comments) = len;
    metadata->comments = comments;
    return metadata;
}

/**
 * Frees a previously allocated metadata struct
 *
 * If the struct is freed deeply, then the associated comments will be
 * deleted as well. This value should only be true if the metadata struct
 * has obtained ownership of the comments.
 *
 * @param metadata  The metadata struct
 * @param deep      Whether to free the associated comments
 */
void ATK_FreeMetadata(ATK_AudioMetadata* metadata, int deep) {
    if (metadata == NULL) {
        return;
    }

    if (deep) {
        ATK_FreeComments(DECONST(ATK_AudioComment*,metadata->comments), metadata->num_comments);
    }
    memset(metadata, 0, sizeof(ATK_AudioMetadata));
    ATK_free(metadata);
}

/**
 * Returns a copy of the given medadata struct
 *
 * This method returns a newly allocated struct. It is the responsibility of this caller
 * to free it (with {@link ATK_FreeMetadata}) when done.
 *
 * If the copy is deep, the associated comments will be copied as well.  Otherwise, the
 * copy will reference the same comment array as the original.
 *
 * @param metadata  The metadata struct
 * @param deep      Whether to copy the associated comments
 *
 * @return a copy of the given medadata struct
 */
ATK_AudioMetadata* ATK_CopyMetadata(ATK_AudioMetadata* metadata, int deep) {
    if (metadata == NULL) {
        return NULL;
    }

    ATK_AudioMetadata* result = (ATK_AudioMetadata*)ATK_malloc(sizeof(ATK_AudioMetadata));
    memset(result, 0, sizeof(ATK_AudioMetadata));
    DECONST(Uint16,result->channels) = metadata->channels;
    DECONST(Uint32,result->rate) = metadata->rate;
    DECONST(Uint64,result->frames) = metadata->frames;
    DECONST(Uint16,result->num_comments) = metadata->num_comments;
    if (deep) {
        ATK_AudioComment* comments = ATK_CopyComments(metadata->comments, metadata->num_comments);
        DECONST(ATK_AudioComment*,result->comments) = comments;
    } else {
        result->comments = metadata->comments;
    }
    return result;
}

#pragma mark -
#pragma mark Stream Decoding

/* Table of code detection and loading functions */
static struct {
    const char *type;
    SDL_bool (SDLCALL *is)(SDL_RWops *src);
    ATK_AudioSource *(SDLCALL *load)(SDL_RWops *src, int ownsrc);
} supported[] = {
    /* keep popular formats first */
    { "wav",  ATK_SourceIsWAV,    ATK_LoadWAV_RW },
    { "ogg",  ATK_SourceIsVorbis, ATK_LoadVorbis_RW },
    { "flac", ATK_SourceIsFLAC,   ATK_LoadFLAC_RW },
    { "mp3",  ATK_SourceIsMP3,    ATK_LoadMP3_RW },
};


/**
 * Creates a new ATK_AudioSource from the given file
 *
 * This function will return NULL if the file cannot be located or is not an
 * proper audio file. The file will not be read into memory, but is instead
 * available for streaming.
 *
 * This function will attempt to automatically determine the codec type. This can
 * be tricky  because the audio container and audio codec are not necessarily the
 * same. This is particularly true for .ogg, and to a lesser extent with .wav. For
 * simplicity, we assume that each file container type holds the standard audio
 * codec for its type. In the case of .ogg, that means OGG Vorbis. If the codec
 * cannot be recognized, or otherwise be initialized, this function returns NULL
 * and an error code will be set.
 *
 * It is the responsibility of the caller of this function to close the source
 * (with {@link ATK_UnloadSource}) when it is no longer needed.
 *
 * @param filename    The name of the file with audio data.
 *
 * @return a new ATK_AudioSource from the given file
 */
ATK_AudioSource* ATK_LoadSource(const char* filename) {
    SDL_RWops *src = SDL_RWFromFile(filename, "rb");
    const char *ext = strrchr(filename, '.');
    if (ext) {
        ext++;
    }
    if (!src) {
        /* The error message has been set in SDL_RWFromFile */
        return NULL;
    }
    return ATK_LoadTypedSource_RW(src, 1, ext);
}

/**
 * Creates a new ATK_AudioSource from the given readable/seekable RWops
 *
 * The RWops object must be positioned at the start of the audio metadata. Note
 * that any modification of the RWops (via seeks or reads) can potentially corrupt
 * the internal state of the ATK_AudioSource for subsequent function calls. That
 * is why loading directly from a file is preferable to this function unless you
 * need the flexibility of an in-memory source.
 *
 * This function will attempt to automatically determine the codec type. It will do
 * this by attempting to parse the data with each supported codec until it finds one
 * that does not produce an error. If the codec cannot be recognized, or otherwise
 * be initialized, this function returns NULL and an error code will be set.
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
 * @return a new ATK_AudioSource from the given readable/seekable RWops
 */
extern DECLSPEC ATK_AudioSource* SDLCALL ATK_LoadAudioSource_RW(SDL_RWops* source, int ownsrc) {
    return ATK_LoadTypedSource_RW(source, ownsrc, NULL);
}

/**
 * Creates a new ATK_AudioSource from the given readable/seekable RWops
 *
 * The RWops object must be positioned at the start of the audio metadata. Note
 * that any modification of the RWops (via seeks or reads) can potentially corrupt
 * the internal state of the ATK_AudioSource for subsequent function calls. That
 * is why loading directly from a file is preferable to this function unless you
 * need the flexibility of an in-memory source.
 *
 * Even though this function accepts a file type, SDL_atk may still try other codecs
 * if the file type does not appear to match the data provided. If the type is NULL,
 * SDL_atk will rely solely on its ability to guess the format. If the codec cannot
 * be recognized, or otherwise be initialized, this function returns NULL and an
 * error code will be set.
 *
 * It is the responsibility of the caller of this function to close the source
 * (with {@link ATK_UnloadSource}) when it is no longer needed. If ownsrc is
 * true, freeing the source will also close/free the underlying RWops object.
 * Otherwise it is the responsibility of the caller of this function to free/close
 * the RWops object separately.
 *
 * @param source    A seekable/readable SDL_RWops to provide audio data
 * @param ownsrc    Non-zero to acquire ownership of the SDL_RWops for deletion later
 * @param type      A filename extension that represent this data ("WAV", "MP3", etc)
 *
 * @return a new ATK_AudioSource from the given readable/seekable RWops
 */
ATK_AudioSource* ATK_LoadTypedSource_RW(SDL_RWops* source, int ownsrc, const char* type) {
    /* Make sure there is something to do.. */
    if ( source == NULL ) {
        ATK_SetError("Passed a NULL data source");
        return(NULL);
    }

    /* See whether or not this data source can handle seeking */
    if ( SDL_RWseek(source, 0, RW_SEEK_CUR) < 0 ) {
        ATK_SetError("Can't seek in this data source");
        if (ownsrc) {
            SDL_RWclose(source);
        }
        return(NULL);
    }

    /* Detect the type of audio being loaded */
    Sint32 index = -1;
    Sint32 size = SDL_arraysize(supported);
    // Try preferred codec first
    if (type) {
        for (Sint32 ii = 0; index == -1 && ii < size; ++ii ) {
            if (ATK_string_equals(type, supported[ii].type) && supported[ii].is) {
                if (supported[ii].is(source)) {
                    index = ii;
                }
            }
        }
    }
    // Look at all codecs next
    if (index == -1) {
        for (Sint32 ii = 0; index == -1 && ii < size; ++ii ) {
            if (supported[ii].is) {
                if (supported[ii].is(source)) {
                    index = ii;
                }
            }
        }
    }

    ATK_AudioSource* result = NULL;
    if (index != -1) {
        result = supported[index].load(source,ownsrc);
    } else {
        if (ownsrc) {
            SDL_RWclose(source);
        }
        ATK_SetError("Unsupported audio format");
    }

    return result;
}

/**
 * Closes a ATK_AudioSource, releasing all memory
 *
 * This function will delete the ATK_AudioSource, making it no longer safe to use.
 *
 * If the audio source is loaded directly from a file, then the source "owns"
 * the underlying file. In that case, it will close/free the file when it is done.
 * However, if the audio source was loaded from a RWops then it will leave that
 * object open unless ownership was transfered at the time the source was loaded.
 *
 * @param source    The source to unload
 *
 * @return 0 if the source was successfully closed, -1 otherwise
 */
Sint32 ATK_UnloadSource(ATK_AudioSource* source) {
    if (source == NULL) {
        ATK_SetError("Attempt to access NULL codec source");
        return -1;
    }

    switch(source->type) {
    case ATK_CODEC_WAV:
        return ATK_WAV_UnloadSource(source);
    case ATK_CODEC_VORBIS:
        return ATK_Vorbis_UnloadSource(source);
    case ATK_CODEC_FLAC:
        return ATK_FLAC_UnloadSource(source);
    case ATK_CODEC_MP3:
        return ATK_MP3_UnloadSource(source);
    }

    ATK_SetError("Unrecognized codec type: %d",source->type);
    return -1;
}

/**
 * Seeks to the given page in the audio source.
 *
 * Audio streams are processed in pages. A page is the minimal amount of information
 * that can be read into memory at a time. Our API only allows seeking at the page
 * level, not at the sample level.
 *
 * If the page is out of bounds, this function will seek to the last page.
 *
 * @param source    The audio source
 * @param page      The page to seek
 *
 * @return the page acquired, or -1 if there is an error
 */
Sint32 ATK_SeekSourcePage(ATK_AudioSource* source, Uint32 page) {
    if (source == NULL) {
        ATK_SetError("Attempt to access NULL codec source");
        return -1;
    }

    switch(source->type) {
    case ATK_CODEC_WAV:
        return ATK_WAV_SeekSourcePage(source,page);
    case ATK_CODEC_VORBIS:
        return ATK_Vorbis_SeekSourcePage(source,page);
    case ATK_CODEC_FLAC:
            return ATK_FLAC_SeekSourcePage(source,page);
    case ATK_CODEC_MP3:
        return ATK_MP3_SeekSourcePage(source,page);
    }

    ATK_SetError("Unrecognized codec type: %d",source->type);
    return -1;
}

/**
 * Returns the number of audio frames in an audio source page.
 *
 * Note that this function is only accurate for the pages after the first. To get
 * the accurate value for the first page, see {@link ATK_GetSourceFirstPageSize}.
 *
 * An audio frame is a collection of simultaneous samples for different channels.
 * Multiplying the page size by the number of channels produces the number of samples
 * in a page.
 *
 * @param source    The audio source
 *
 * @return the number of audio frames in a page, or -1 if there is an error
 */
Sint32 ATK_GetSourcePageSize(ATK_AudioSource* source) {
    if (source == NULL) {
        ATK_SetError("Attempt to access NULL codec source");
        return -1;
    }

    switch(source->type) {
    case ATK_CODEC_WAV:
        return ATK_WAV_GetSourcePageSize(source);
    case ATK_CODEC_VORBIS:
        return ATK_Vorbis_GetSourcePageSize(source);
    case ATK_CODEC_FLAC:
        return ATK_FLAC_GetSourcePageSize(source);
    case ATK_CODEC_MP3:
        return ATK_MP3_GetSourcePageSize(source);
    }

    ATK_SetError("Unrecognized codec type: %d",source->type);
    return -1;
}

/**
 * Returns the number of audio frames on the first audio source page.
 *
 * This function is distinct from {@link ATK_GetSourcePageSize} because some
 * codecs (most notably MP3) can have a different number of samples on their
 * first page. That is because they allow metadata information to take up a
 * portion of the first page.
 *
 * An audio frame is a collection of simultaneous samples for different channels.
 * Multiplying the page size by the number of channels produces the number of samples
 * in a page.
 *
 * @param source    The audio source
 *
 * @return the number of audio frames on the first page, or -1 if there is an error
 */
Sint32 ATK_GetSourceFirstPageSize(ATK_AudioSource* source) {
    if (source == NULL) {
        ATK_SetError("Attempt to access NULL codec source");
        return -1;
    }

    switch(source->type) {
    case ATK_CODEC_WAV:
        return ATK_WAV_GetSourceFirstPageSize(source);
    case ATK_CODEC_VORBIS:
        return ATK_Vorbis_GetSourceFirstPageSize(source);
    case ATK_CODEC_FLAC:
        return ATK_FLAC_GetSourceFirstPageSize(source);
    case ATK_CODEC_MP3:
        return ATK_MP3_GetSourceFirstPageSize(source);
    }

    ATK_SetError("Unrecognized codec type: %d",source->type);
    return -1;
}

/**
 * Returns the index of the last page in the audio source.
 *
 * Audio sources are processed in pages. A page is the minimal amount of information
 * that can be read into memory at a time. Our API only allows seeking at the page
 * level, not at the sample level.
 *
 * @param source    The audio source
 *
 * @return the index of the last page in the stream, or -1 if there is an error
 */
Sint32 ATK_GetSourceLastPage(ATK_AudioSource* source) {
    if (source == NULL) {
        ATK_SetError("Attempt to access NULL codec source");
        return -1;
    }

    switch(source->type) {
    case ATK_CODEC_WAV:
        return ATK_WAV_GetSourceLastPage(source);
    case ATK_CODEC_VORBIS:
        return ATK_Vorbis_GetSourceLastPage(source);
    case ATK_CODEC_FLAC:
        return ATK_FLAC_GetSourceLastPage(source);
    case ATK_CODEC_MP3:
        return ATK_MP3_GetSourceLastPage(source);
    }

    ATK_SetError("Unrecognized codec type: %d",source->type);
    return -1;
}

/**
 * Returns the index of the current page in the audio source.
 *
 * Audio sources are processed in pages. A page is the minimal amount of information
 * that can be read into memory at a time. Our API only allows seeking at the page
 * level, not at the sample level.
 *
 * @param source    The audio source
 *
 * @return the index of the current page in the stream, or -1 if there is an error
 */
Sint32 ATK_GetSourceCurrentPage(ATK_AudioSource* source) {
    if (source == NULL) {
        ATK_SetError("Attempt to access NULL codec source");
        return -1;
    }

    switch(source->type) {
    case ATK_CODEC_WAV:
        return ATK_WAV_GetSourceCurrentPage(source);
    case ATK_CODEC_VORBIS:
        return ATK_Vorbis_GetSourceCurrentPage(source);
    case ATK_CODEC_FLAC:
        return ATK_FLAC_GetSourceCurrentPage(source);
    case ATK_CODEC_MP3:
        return ATK_MP3_GetSourceCurrentPage(source);
    }

    ATK_SetError("Unrecognized codec type: %d",source->type);
    return -1;
}

/**
 * Returns 1 if the audio source is at the end of the stream; 0 otherwise
 *
 * @param source    The audio source
 *
 * @return 1 if the audio source is at the end of the stream; 0 otherwise
 */
Uint32 ATK_IsSourceEOF(ATK_AudioSource* source) {
    if (source == NULL) {
        ATK_SetError("Attempt to access NULL codec source");
        return 0;
    }

    switch(source->type) {
    case ATK_CODEC_WAV:
        return ATK_WAV_IsSourceEOF(source);
    case ATK_CODEC_VORBIS:
        return ATK_Vorbis_IsSourceEOF(source);
    case ATK_CODEC_FLAC:
        return ATK_FLAC_IsSourceEOF(source);
    case ATK_CODEC_MP3:
        return ATK_MP3_IsSourceEOF(source);
    }

    ATK_SetError("Unrecognized codec type: %d",source->type);
    return 0;
}

/**
 * Reads a single page of audio data into the buffer
 *
 * This function reads in the current source page into the buffer. The data written
 * into the buffer is linear PCM data with interleaved channels. If the source is
 * at the end, nothing will be written.
 *
 * The size of a page is given by {@link ATK_GetSourcePageSize}. This buffer should
 * large enough to hold this data. As the page size is in audio frames, that means
 * the buffer should be pagesize * # of channels * sizeof(float) bytes.
 *
 * @param source    The audio source
 * @param buffer    The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
Sint32 ATK_ReadSourcePage(ATK_AudioSource* source, float* buffer) {
    if (source == NULL) {
        ATK_SetError("Attempt to access NULL codec source");
        return -1;
    }

    switch(source->type) {
    case ATK_CODEC_WAV:
        return ATK_WAV_ReadSourcePage(source,buffer);
    case ATK_CODEC_VORBIS:
        return ATK_Vorbis_ReadSourcePage(source,buffer);
    case ATK_CODEC_FLAC:
        return ATK_FLAC_ReadSourcePage(source,buffer);
    case ATK_CODEC_MP3:
        return ATK_MP3_ReadSourcePage(source,buffer);
    }

    ATK_SetError("Unrecognized codec type: %d",source->type);
    return -1;
}

/**
 * Reads the entire audio source into the buffer.
 *
 * The data written into the buffer is linear PCM data with interleaved channels. If the
 * stream is not at the initial page, it will rewind before writing the data. It will
 * restore the stream to the initial page when done.
 *
 * The buffer needs to be large enough to hold the entire audio source. As the length is
 * measured in audio frames, this means that the buffer should be
 * frames * # of channels * sizeof(float) bytes.
 *
 * @param source    The audio source
 * @param buffer    The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
Sint64 ATK_ReadSource(ATK_AudioSource* source, float* buffer) {
    if (source == NULL) {
        ATK_SetError("Attempt to access NULL codec source");
        return -1;
    }

    switch(source->type) {
    case ATK_CODEC_WAV:
        return ATK_WAV_ReadSource(source,buffer);
    case ATK_CODEC_VORBIS:
        return ATK_Vorbis_ReadSource(source,buffer);
    case ATK_CODEC_FLAC:
        return ATK_FLAC_ReadSource(source,buffer);
    case ATK_CODEC_MP3:
        return ATK_MP3_ReadSource(source,buffer);
    }

    ATK_SetError("Unrecognized codec type: %d",source->type);
    return -1;
}

#pragma mark -
#pragma mark RWops Decoding
#define ATK_DECODE_CHECKSUM 0x89

typedef struct {
    /** The checksum to "verify" this is indeed a managed file */
    Uint8 checksum;
    Uint32 current;
    size_t capacity;
    size_t pagesize;
    size_t firstpage;
    size_t available;
    size_t offset;
    float* buffer;
} ATK_DecoderState;

/**
 * Internal implementation of SDL_RWsize
 *
 * This function returns the total size of the audio source in bytes.
 *
 * @param context a pointer to an SDL_RWops structure
 *
 * @return the total size of the file in bytes or -1 on error
 */
static Sint64 ATK_RWsizeDecode(SDL_RWops* context) {
    if (context == NULL) {
        ATK_SetError("Attempted to query a null context");
        return -1;
    } else if (context->type != SDL_RWOPS_UNKNOWN) {
        ATK_SetError("Attempted to query a damaged stream");
        return -1;
    }

    Uint8* check = (Uint8*)context->hidden.unknown.data1;
    if (*check != ATK_DECODE_CHECKSUM) {
        ATK_SetError("Attempted to query a damaged stream");
        return -1;
    }

    ATK_AudioSource* source = (ATK_AudioSource*)context->hidden.unknown.data2;
    return source->metadata.frames*source->metadata.channels*sizeof(float);
}

/**
 * Internal implementation of SDL_RWseek
 *
 * This function seeks within the source stream
 *
 * @param context   a pointer to an SDL_RWops structure
 * @param offset    an offset in bytes, relative to whence location; can be negative
 * @param whence    any of RW_SEEK_SET, RW_SEEK_CUR, RW_SEEK_END
 *
 * @return the final offset in the file after the seek or -1 on error
 */
static Sint64 ATK_RWseekDecode(SDL_RWops* context, Sint64 offset, int whence) {
    if (context == NULL) {
        ATK_SetError("Attempted to seek a null context");
        return -1;
    } else if (context->type != SDL_RWOPS_UNKNOWN) {
        ATK_SetError("Attempted to seek a damaged stream");
        return -1;
    }

    Uint8* check = (Uint8*)context->hidden.unknown.data1;
    if (*check != ATK_DECODE_CHECKSUM) {
        ATK_SetError("Attempted to seek a damaged stream");
        return -1;
    }
    if (offset % sizeof(float) != 0) {
        ATK_SetError("Attempted an unaligned seek of a float stream");
        return -1;
    }

    ATK_DecoderState* state = (ATK_DecoderState*)context->hidden.unknown.data1;
    ATK_AudioSource* source = (ATK_AudioSource*)context->hidden.unknown.data2;

    // Compute the absolute location
    Sint64 trueoff = 0;
    Sint64 bytes = (Sint64)sizeof(float);
    switch(whence) {
        case RW_SEEK_SET:
            trueoff = offset/sizeof(float);
            break;
        case RW_SEEK_CUR:
            if (ATK_IsSourceEOF(source) && state->available == 0) {
                // Effectively a seek-end
                trueoff = (source->metadata.frames*source->metadata.channels+offset/bytes);
            } else if (state->current > 0) {
                trueoff = offset/bytes+state->firstpage*source->metadata.channels+state->offset;
                trueoff += (state->current-1)*state->pagesize*source->metadata.channels;
            } else {
                trueoff = offset/bytes+state->offset;
            }
            break;
        case RW_SEEK_END:
            trueoff = (source->metadata.frames*source->metadata.channels-offset/bytes);
            break;
    }
    if (trueoff < 0) {
        trueoff = 0;
    } else if (trueoff > (Sint64)(source->metadata.frames*source->metadata.channels)) {
        trueoff = source->metadata.frames*source->metadata.channels;
    }

    Uint32 page = 0;
    Uint32 off = (Uint32)trueoff;
    if (trueoff > (Sint64)(state->firstpage*source->metadata.channels)) {
        page = 1;
        off -= (Uint32)(state->firstpage*source->metadata.channels);
    }
    page += (Uint32)(off/state->capacity);
    off  = off % state->capacity;

    ATK_SeekSourcePage(source, page);
    state->available = ATK_ReadSourcePage(source, state->buffer);
    state->available *= source->metadata.channels;
    state->current = page;
    state->offset  = off;
    return trueoff*sizeof(float);
}

/**
 * Internal implementation of SDL_RWread
 *
 * This function reads from the audio stream (post metadata).
 *
 * @param context   a pointer to an SDL_RWops structure
 * @param ptr       a pointer to a buffer to read data into
 * @param size      the size of each object to read, in bytes
 * @param maxnum    the maximum number of objects to be read
 *
 * @return the number of objects read, or 0 at error or end of file
 */
static size_t ATK_RWreadDecode(SDL_RWops *context, void *ptr, size_t size, size_t maxnum) {
    if (context == NULL) {
        ATK_SetError("Attempted to read a null context");
        return 0;
    } else if (context->type != SDL_RWOPS_UNKNOWN) {
        ATK_SetError("Attempted to read a damaged stream");
        return 0;
    }

    Uint8* check = (Uint8*)context->hidden.unknown.data1;
    if (*check != ATK_DECODE_CHECKSUM) {
        ATK_SetError("Attempted to read a damaged stream");
        return -1;
    }
    if (size % sizeof(float) != 0) {
        ATK_SetError("Attempted an unaligned read of a float stream");
        return -1;
    }
    size_t chunk = size/sizeof(float);

    ATK_DecoderState* state = (ATK_DecoderState*)context->hidden.unknown.data1;
    ATK_AudioSource* source = (ATK_AudioSource*)context->hidden.unknown.data2;

    size_t offset = state->offset;
    if (ATK_IsSourceEOF(source)) {
        offset = source->metadata.frames*source->metadata.channels-(state->available-offset);
    } else if (state->current > 0) {
        offset += state->firstpage*source->metadata.channels;
        offset += (state->current-1)*state->capacity;
    }
    size_t total  = maxnum*chunk; // Number of floats to read
    if (offset+total > source->metadata.frames*source->metadata.channels) {
        total = source->metadata.frames*source->metadata.channels-offset;
        // Only read full chunks
        if (total % chunk != 0) {
            total -= (total % chunk);
        }
    }

    size_t rem = state->available-state->offset;
    if (rem > total) {
        memcpy(ptr,state->buffer+state->offset,total*sizeof(float));
        state->offset += total;
    } else {
        size_t left = total;
        while (rem && left >= rem) {
            size_t off = (total-left)*sizeof(float);
            char* cptr = (char*)ptr;
            memcpy(cptr+off,state->buffer+state->offset,rem*sizeof(float));
            left -= rem;
            state->available = ATK_ReadSourcePage(source, state->buffer);
            state->available *= source->metadata.channels;
            state->current++;
            state->offset = 0;
            rem = state->available;
        }
        if (rem && left) {
            size_t off = (total-left)*sizeof(float);
            char* cptr = (char*)ptr;
            memcpy(cptr+off,state->buffer,left*sizeof(float));
            state->offset = left;
        }
    }
    return total/chunk;
}

/**
 * Internal implementation of SDL_RWwrite
 *
 * This function is not implemented for audio sources.
 *
 * @param context   a pointer to an SDL_RWops structure
 * @param ptr       a pointer to a buffer containing data to write
 * @param size      the size of each object to write, in bytes
 * @param num       the number of objects to write
 *
 * @return the number of objects written, which is less than num on error
 */
static size_t ATK_RWwriteDecode(SDL_RWops *context, const void *ptr, size_t size, size_t num) {
    ATK_SetError("Audio sources are read-only");
    return 0;
}

/**
 * Internal implementation of SDL_RWclose
 *
 * This function closes and frees an allocated audio source.
 *
 * @param context   a pointer to an SDL_RWops structure to close
 *
 * @return 0 on success, negative error code on failure
 */
static int ATK_RWcloseDecode(SDL_RWops *context) {
    if (context == NULL) {
        ATK_SetError("Attempted to close a null context");
        return -1;
    } else if (context->type != SDL_RWOPS_UNKNOWN) {
        ATK_SetError("Attempted to close a damaged stream");
        return -1;
    }

    Uint8* check = (Uint8*)context->hidden.unknown.data1;
    if (*check != ATK_DECODE_CHECKSUM) {
        ATK_SetError("Attempted to close a damaged stream");
        return -1;
    }

    ATK_DecoderState* state = (ATK_DecoderState*)context->hidden.unknown.data1;
    ATK_AudioSource* source = (ATK_AudioSource*)context->hidden.unknown.data2;
    if (source != NULL) {
        ATK_UnloadSource(source);
        context->hidden.unknown.data2 = NULL;
    }
    if (state != NULL) {
        if (state->buffer != NULL) {
            ATK_free(state->buffer);
            state->buffer = NULL;
        }
        ATK_free(state);
        context->hidden.unknown.data1 = NULL;
    }
    ATK_free(context);
    return 0;
}

/**
 * Returns an SDL_RWops (wrapper) with the audio frames of the given audio source
 *
 * The SDL_RWops object returned is read-only. Any attempts to write to it will fail.
 * The purpose of this object is to provide an interace for a smooth, buffered stream
 * on top of the paging interface provided by {@link ATK_AudioSource}.
 *
 * The SDL_RWops operations return the interleaved audio samples. The stream metadata
 * is read immediately once the audio source is opened, and a pointer to it is stored
 * in the parameter metadata. However, the metadata is still owned by the returned
 * SDL_RWops object, and the user should never attempt to free this data.
 *
 * If ownsrc is true, this SDL_RWops will close the wrapped SDL_RWops when done.
 *
 * @param stream    The in-memory representation of the audio source
 * @param ownsrc    Whether this SDL_RWops should own the audio source
 * @param type      A filename extension that represents this data ("WAV", "MP3", etc)
 * @param metadata  A pointer to the source metadata
 *
 * @return an SDL_RWops (wrapper) with the audio frames of the given audio source
 */
SDL_RWops* ATK_RWFromAudioSourceRW(SDL_RWops* stream, int ownsrc, const char* type,
                                  ATK_AudioMetadata** metadata) {
    SDL_RWops* result = NULL;
    ATK_AudioSource* source = NULL;
    ATK_DecoderState* state = NULL;
    float* buffer = NULL;

    result = ATK_malloc(sizeof(ATK_RWops));
    if (result == NULL) {
        ATK_OutOfMemory();
        goto fail;
    }
    source = ATK_LoadTypedSource_RW(stream, ownsrc, type);
    if (source == NULL) {
        goto fail;
    }
    state = ATK_malloc(sizeof(ATK_DecoderState));
    if (state == NULL) {
        ATK_OutOfMemory();
        goto fail;
    }

    state->checksum = ATK_DECODE_CHECKSUM;
    state->current  = 0;
    state->offset   = 0;
    state->pagesize  = ATK_GetSourcePageSize(source);
    state->capacity  = state->pagesize*source->metadata.channels;
    buffer = ATK_malloc(state->capacity*sizeof(float));
    if (buffer == NULL) {
        ATK_OutOfMemory();
        goto fail;
    }
    memset(buffer,0,state->capacity*sizeof(float));
    state->buffer = buffer;

    result->size = ATK_RWsizeDecode;
    result->seek = ATK_RWseekDecode;
    result->read = ATK_RWreadDecode;
    result->write = ATK_RWwriteDecode;
    result->close = ATK_RWcloseDecode;
    result->type = SDL_RWOPS_UNKNOWN;
    result->hidden.unknown.data1 = state;
    result->hidden.unknown.data2 = source;
    if (metadata) {
        *metadata = &(DECONST(ATK_AudioMetadata, source->metadata));
    }
    state->available = ATK_ReadSourcePage(source, buffer);
    state->firstpage = state->available;
    state->available *= source->metadata.channels;
    return result;

fail:
    if (result != NULL) {
        ATK_free(result);
    }
    if (source != NULL) {
        ATK_UnloadSource(source);
    }
    if (state != NULL) {
        ATK_free(state);
    }
    if (buffer != NULL) {
        ATK_free(buffer);
    }

    return NULL;
}

/**
 * Returns an SDL_RWops with the audio frames of the given file
 *
 * The SDL_RWops object returned is read-only. Any attempts to write to it will fail.
 * The purpose of this object is to provide an interace for a smooth, buffered stream
 * on top of the paging interface provided by {@link ATK_AudioSource}.
 *
 * The SDL_RWops operations return the interleaved audio samples. The stream metadata
 * is read immediately once the audio source is opened, and a pointer to it is stored
 * in the parameter metadata. However, the metadata is still owned by the returned
 * SDL_RWops object, and the user should never attempt to free this data.
 *
 * @param filename  The filename of the underlying audio source
 * @param type      A filename extension that represents this data ("WAV", "MP3", etc)
 * @param metadata  A pointer to the source metadata
 *
 * @return an SDL_RWops with the audio frames of the given file
 */
SDL_RWops* ATK_RWFromTypedAudioSource(const char* filename, const char* type,
                                      ATK_AudioMetadata** metadata) {
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
    return ATK_RWFromAudioSourceRW(stream, 1, type, metadata);
}

/**
 * Returns an SDL_RWops with the audio frames of the given file
 *
 * The SDL_RWops object returned is read-only. Any attempts to write to it will fail.
 * The purpose of this object is to provide an interace for a smooth, buffered stream
 * on top of the paging interface provided by {@link ATK_AudioSource}.
 *
 * The audio source type will be inferred from the file extension.
 *
 * The SDL_RWops operations return the interleaved audio samples. The stream metadata
 * is read immediately once the audio source is opened, and a pointer to it is stored
 * in the parameter metadata. However, the metadata is still owned by the returned
 * SDL_RWops object, and the user should never attempt to free this data.
 *
 * @param filename  The filename of the underlying audio source
 * @param metadata  A pointer to the source metadata
 *
 * @return an SDL_RWops with the audio frames of the given file
 */
SDL_RWops* ATK_RWFromAudioSource(const char* filename,
                                 ATK_AudioMetadata** metadata) {
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
    const char *ext = strrchr(filename, '.');
    if (ext) {
        ext++;
    }
    return ATK_RWFromAudioSourceRW(stream, 1, ext, metadata);
}

#pragma mark -
#pragma mark Stream Encoding
/**
 * Returns a new encoding stream to write to the given file
 *
 * The provided metadata will be copied to the encoding object. So it is safe to delete
 * this metadata before the encoding is complete. If the encoding stream cannot be
 * allocated, or is not supported, this function will return NULL and an error value
 * will be set.
 *
 * The metadata should reflect the properties of the stream to be encoded as much as
 * possible. In particular, some codecs will not permit writes with more frames than
 * those specified in the initial metadata.
 *
 * This function will encode the audio using the default settings of the codec. We
 * do not currently support fine tune control of bit rates or compression options.
 *
 * It is the responsibility of the caller of this function to complete the encoding
 * (with {@link ATK_FinishEncoding}) when the stream is finished.
 *
 * @param filename  The name of the new audio file
 * @param type      A filename extension that represents this data ("WAV", "MP3", etc)
 * @param metadata  The stream metadata
 *
 * @return a new encoding stream to write to the given file
 */
ATK_AudioEncoding* ATK_EncodeAudio(const char* filename, const char* type,
                                   const ATK_AudioMetadata* metadata) {
    SDL_RWops *dst = SDL_RWFromFile(filename, "wb");
    if (!dst) {
        /* The error message has been set in SDL_RWFromFile */
        return NULL;
    }
    return ATK_EncodeAudio_RW(dst, 1, type, metadata);

}

/**
 * Returns a new encoding stream to write to the given RWops
 *
 * The provided metadata will be copied to the encoding object. So it is safe to delete
 * this metadata before the encoding is complete. If the encoding stream cannot be
 * allocated, or is not supported, this function will return NULL and an error value
 * will be set.
 *
 * The metadata should reflect the properties of the stream to be encoded as much as
 * possible. In particular, some codecs will not permit writes with more frames than
 * those specified in the initial metadata.
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
 * @param type      A filename extension that represents this data ("WAV", "MP3", etc)
 * @param metadata  The stream metadata
 *
 * @return a new encoding stream to write to the given RWops
 */
ATK_AudioEncoding* ATK_EncodeAudio_RW(SDL_RWops* stream, int ownsrc, const char* type,
                                      const ATK_AudioMetadata* metadata) {
    if (stream == NULL) {
        ATK_SetError("Attempt to access NULL RWops");
        return NULL;
    }

    if (ATK_string_equals(type, "wav")) {
        return ATK_EncodeWAV_RW(stream, ownsrc, metadata);
    } else if (ATK_string_equals(type, "ogg")) {
        return ATK_EncodeVorbis_RW(stream, ownsrc, metadata);
    } else if (ATK_string_equals(type, "flac")) {
        return ATK_EncodeFLAC_RW(stream, ownsrc, metadata);
    } else if (ATK_string_equals(type, "mp3")) {
        ATK_SetError("Codec MP3 is not supported");
        return NULL;
    }

    ATK_SetError("Unrecognized codec type: %s",type);
    return NULL;
}

/**
 * Writes the given data to the encoding stream.
 *
 * The data in the buffer is assumed to be interleaved, with the sample rate and
 * number of channels specified by the encoding metadata. As frames represents
 * the number of audio frames, not samples, this means buffer should have at
 * least frames*channels elements.
 *
 * @param encoding  The encoding stream
 * @param buffer    The data buffer with the audio samples
 * @param frames    The number of audio frames to write
 *
 * @return the number of frames written, or -1 on error
 */
Sint64 ATK_WriteEncoding(ATK_AudioEncoding* encoding, float* buffer, size_t frames) {
    if (encoding == NULL) {
        ATK_SetError("Attempt to access NULL codec encoding");
        return -1;
    }

    switch(encoding->type) {
    case ATK_CODEC_WAV:
        return ATK_WAV_WriteEncoding(encoding,buffer,frames);
    case ATK_CODEC_VORBIS:
        return ATK_Vorbis_WriteEncoding(encoding,buffer,frames);
    case ATK_CODEC_FLAC:
        return ATK_FLAC_WriteEncoding(encoding,buffer,frames);
    default:
        return -1;
    }
}

/**
 * Completes the encoding stream, releasing all resources.
 *
 * This function will delete the ATK_AudioEncoding, making it no longer safe to use.
 *
 * If the audio encoding is writing directly to a file, then the enoding "owns"
 * the underlying file. In that case, it will close/free the file when it is done.
 * However, if the audio encoding was writing to a RWops then it will leave that
 * object open unless ownership was transfered at the time the encoding was
 * created.
 *
 * @param encoding  The encoding stream
 *
 * @return 0 if the enconding was successfully completed, -1 otherwise.
 */
Sint32 ATK_FinishEncoding(ATK_AudioEncoding* encoding) {
    if (encoding == NULL) {
        ATK_SetError("Attempt to access NULL codec encoding");
        return -1;
    }

    switch(encoding->type) {
    case ATK_CODEC_WAV:
        return ATK_WAV_FinishEncoding(encoding);
    case ATK_CODEC_VORBIS:
        return ATK_Vorbis_FinishEncoding(encoding);
    case ATK_CODEC_FLAC:
        return ATK_FLAC_FinishEncoding(encoding);
    default:
        return -1;
    }
}

#pragma mark -
#pragma mark RWops Encoding
#define ATK_ENCODE_CHECKSUM 0x87

/**
 * Internal implementation of SDL_RWsize
 *
 * This function is not implemented for encoding.
 *
 * @param context a pointer to an SDL_RWops structure
 *
 * @return the total size of the file in bytes or -1 on error
 */
static Sint64 ATK_RWsizeEncode(SDL_RWops* context) {
    ATK_SetError("Audio encoding is incomplete");
    return -1;
}

/**
 * Internal implementation of SDL_RWseek
 *
 * This function is not implemented for encoding.
 *
 * @param context   a pointer to an SDL_RWops structure
 * @param offset    an offset in bytes, relative to whence location; can be negative
 * @param whence    any of RW_SEEK_SET, RW_SEEK_CUR, RW_SEEK_END
 *
 * @return the final offset in the file after the seek or -1 on error
 */
static Sint64 ATK_RWseekEncode(SDL_RWops* context, Sint64 offset, int whence) {
    ATK_SetError("Audio encoding does not support seeking");
    return -1;
}

/**
 * Internal implementation of SDL_RWread
 *
 * This function is not implemented for encoding.
 *
 * @param context   a pointer to an SDL_RWops structure
 * @param ptr       a pointer to a buffer to read data into
 * @param size      the size of each object to read, in bytes
 * @param maxnum    the maximum number of objects to be read
 *
 * @return the number of objects read, or 0 at error or end of file
 */
static size_t ATK_RWreadEncode(SDL_RWops *context, void *ptr, size_t size, size_t maxnum) {
    ATK_SetError("Audio encoding does not support reading");
    return 0;
}

/**
 * Internal implementation of SDL_RWwrite
 *
 * This function writes to the encoding stream.
 *
 * @param context   a pointer to an SDL_RWops structure
 * @param ptr       a pointer to a buffer containing data to write
 * @param size      the size of each object to write, in bytes
 * @param num       the number of objects to write
 *
 * @return the number of objects written, which is less than num on error
 */
static size_t ATK_RWwriteEncode(SDL_RWops *context, const void *ptr, size_t size, size_t num) {
    if (context == NULL) {
        ATK_SetError("Attempted write to a null context");
        return 0;
    } else if (context->type != SDL_RWOPS_UNKNOWN) {
        ATK_SetError("Attempted write to a damaged stream");
        return 0;
    }

    uintptr_t check = (uintptr_t)context->hidden.unknown.data2;
    if (check != ATK_ENCODE_CHECKSUM) {
        ATK_SetError("Attempted write to a damaged stream");
        return 0;
    }

    ATK_AudioEncoding* out = (ATK_AudioEncoding*)context->hidden.unknown.data1;

    if (size % (sizeof(float)*out->metadata.channels) != 0) {
        ATK_SetError("Attempted an unaligned write to a float stream");
        return 0;
    }
    size_t chunk = size/(sizeof(float)*out->metadata.channels);

    Sint64 amt = ATK_WriteEncoding(out, (float*)ptr, chunk*num);
    if (amt < 0) {
        return 0;
    }
    return amt/chunk;
}

/**
 * Internal implementation of SDL_RWclose
 *
 * This function closes and frees an allocated audio encoding.
 *
 * @param context   a pointer to an SDL_RWops structure to close
 *
 * @return 0 on success, negative error code on failure
 */
static int ATK_RWcloseEncode(SDL_RWops *context) {
    if (context == NULL) {
        ATK_SetError("Attempted to close a null context");
        return -1;
    } else if (context->type != SDL_RWOPS_UNKNOWN) {
        ATK_SetError("Attempted to close a damaged stream");
        return -1;
    }

    uintptr_t check = (uintptr_t)context->hidden.unknown.data2;
    if (check != ATK_ENCODE_CHECKSUM) {
        ATK_SetError("Attempted to close a damaged stream");
        return -1;
    }

    ATK_AudioEncoding* out = (ATK_AudioEncoding*)context->hidden.unknown.data1;
    if (out != NULL) {
        ATK_FinishEncoding(out);
        context->hidden.unknown.data1 = NULL;
    }
    ATK_free(context);
    return 0;
}

/**
 * Returns an SDL_RWops (wrapper) to write out audio frames in the given codec.
 *
 * The SDL_RWops object returned is write-only. Any attempts to read from it (or even
 * seek while writing) will fail. The purpose of this object is to provide an interface
 * for a smooth, buffered stream on top of the paging interfaces provided by
 * {@link ATK_AudioEncoding}.
 *
 * The SDL_RWops operations should only write the interleaved audio samples. The metadata
 * is written to the output stream upon the creation of this SDL_RWops wrapper.
 *
 * If ownsrc is true, this SDL_RWops will close the wrapped SDL_RWops when done.
 *
 * @param source    The underlying audio source
 * @param type      A filename extension that represents this data ("WAV", "MP3", etc)
 * @param metadata  The audio metadata
 *
 * @return an SDL_RWops (wrapper) to write out audio frames in the given codec.
 */
SDL_RWops* ATK_RWToAudioEncodingRW(SDL_RWops* stream, int ownsrc,
                                   const char* type,
                                   const ATK_AudioMetadata* metadata) {

    SDL_RWops* result = ATK_malloc(sizeof(SDL_RWops));
    if (result == NULL) {
        ATK_OutOfMemory();
        return NULL;
    }
    memset(result, 0, sizeof(SDL_RWops));

    ATK_AudioEncoding* out = ATK_EncodeAudio_RW(stream, ownsrc, type, metadata);
    if (out == NULL) {
        ATK_free(result);
        return NULL;
    }

    result->size = ATK_RWsizeEncode;
    result->seek = ATK_RWseekEncode;
    result->read = ATK_RWreadEncode;
    result->write = ATK_RWwriteEncode;
    result->close = ATK_RWcloseEncode;
    result->type = SDL_RWOPS_UNKNOWN;
    result->hidden.unknown.data1 = out;
    result->hidden.unknown.data2 = (void*)(ATK_ENCODE_CHECKSUM);
    return result;
}

/**
 * Returns an SDL_RWops to write out audio frames to the given file
 *
 * The SDL_RWops object returned is write-only. Any attempts to read from it (or even
 * seek while writing) will fail. The purpose of this object is to provide an interface
 * for a smooth, buffered stream on top of the paging interfaces provided by
 * {@link ATK_AudioEncoding}.
 *
 * The SDL_RWops operations should only write the interleaved audio samples. The metadata
 * is written to the output stream upon the creation of this SDL_RWops object.
 *
 * @param filename  The file to write to
 * @param type      A filename extension that represents this data ("WAV", "MP3", etc)
 * @param metadata  The audio metadata
 *
 * @return an SDL_RWops to write out audio frames to the given file
 */
SDL_RWops* ATK_RWToTypedAudioEncoding(const char* filename, const char* type,
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
    return ATK_RWToAudioEncodingRW(stream, 1, type, metadata);
}

/**
 * Returns an SDL_RWops to write out audio frames to the given file
 *
 * The SDL_RWops object returned is write-only. Any attempts to read from it (or even
 * seek while writing) will fail. The purpose of this object is to provide an interface
 * for a smooth, buffered stream on top of the paging interfaces provided by
 * {@link ATK_AudioEncoding}.
 *
 * The encoding type will be inferred from the file extension.
 *
 * The SDL_RWops operations should only write the interleaved audio samples. The metadata
 * is written to the output stream upon the creation of this SDL_RWops object.
 *
 * @param filename  The file to write to
 * @param metadata  The audio metadata
 *
 * @return an SDL_RWops to write out audio frames to the given file
 */
SDL_RWops* ATK_RWToAudioEncoding(const char* filename,
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
    const char *ext = strrchr(filename, '.');
    if (ext) {
        ext++;
    }
    return ATK_RWToAudioEncodingRW(stream, 1, ext, metadata);
}
