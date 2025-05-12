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
#ifndef __ATK_CODEC_C_H__
#define __ATK_CODEC_C_H__

/* This is an internal header for Codec delegation */
#include <SDL_atk.h>
#include <begin_code.h>

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/** The default page size (in bytes) */
#define ATK_CODEC_PAGE_SIZE   4096

/** Removes the const specifier of a type */
#define DECONST(T,V) *(T*)(&V)

/** Macro for error checking */
#define CHECK_SOURCE(source, retval) \
    if (source == NULL) { \
        ATK_SetError("Attempt to access a NULL codec source"); \
        return retval; \
    } else if (source->decoder == NULL) { \
        ATK_SetError("Codec source has invalid state"); \
        return retval; \
    }

#define CHECK_ENCODING(encoding, retval) \
    if (encoding == NULL) { \
        ATK_SetError("Attempt to access a NULL codec encoding"); \
        return retval; \
    } else if (encoding->encoder == NULL) { \
        ATK_SetError("Codec encoding has invalid state"); \
        return retval; \
    }


/**
 * Returns nonzero if strings are equal; otherwise 0
 *
 * @param str1  The first string to compare
 * @param str2  The second string to compare
 *
 * @return nonzero if strings are equal; otherwise 0
 */
int ATK_string_equals(const char *str1, const char *str2);
    
/**
 * Returns the (string) name for a given codec type
 *
 * This function is typically used for debugging.
 *
 * @param type    The codec type
 *
 * @return the (string) name for the type
 */
extern DECLSPEC const char* SDLCALL ATK_CodecNameForType(ATK_CodecType type);

#pragma mark -
#pragma mark Metadata Comments

/**
 * Returns SDL_TRUE if WAV supports the given comment tag.
 *
 * @param tag   The tag to query
 *
 * @return SDL_TRUE if WAV supports the given comment tag.
 */
extern DECLSPEC SDL_bool SDLCALL ATK_WAV_SupportsCommentTag(const char* tag);

/**
 * Returns SDL_TRUE if MP3 supports the given comment tag.
 *
 * @param tag   The tag to query
 *
 * @return SDL_TRUE if MP3 supports the given comment tag.
 */
extern DECLSPEC SDL_bool SDLCALL ATK_MP3_SupportsCommentTag(const char* tag);

/**
 * Returns an array of comment tags supported by the WAV codec
 *
 * @return an array of comment tags supported by the WAV codec
 */
extern DECLSPEC  const char* const * SDLCALL ATK_WAV_GetCommentTags();

/**
 * Returns an array of comment tags supported by the MP3 codec
 *
 * @return an array of comment tags supported by the MP3 codec
 */
extern DECLSPEC  const char* const * SDLCALL ATK_MP3_GetCommentTags();

#pragma mark -
#pragma mark Stream Decoding

/**
 * The Vorbis specific implementation of {@link ATK_UnloadSource}.
 *
 * @param source    The source to close
 * 
 * @return 0 if the source was successfully closed, -1 otherwise.
 */
extern DECLSPEC Sint32 SDLCALL ATK_Vorbis_UnloadSource(ATK_AudioSource* source);

/**
 * The Vorbis specific implementation of {@link ATK_SeekSourcePage}.
 *
 * @param source    The audio source
 * @param page      The page to seek
 *
 * @return the page acquired, or -1 if there is an error
 */
extern DECLSPEC Sint32 SDLCALL ATK_Vorbis_SeekSourcePage(ATK_AudioSource* source, Uint32 page);

/**
 * The Vorbis specific implementation of {@link ATK_GetSourcePageSize}.
 *
 * @param source    The audio source
 *
 * @return the number of audio frames in a page, or -1 if there is an error
 */
extern DECLSPEC Sint32 SDLCALL ATK_Vorbis_GetSourcePageSize(ATK_AudioSource* source);

/**
 * The Vorbis specific implementation of {@link ATK_GetSourceFirstPageSize}.
 *
 * @param source    The audio source
 *
 * @return the number of audio frames on the first page, or -1 if there is an error
 */
extern DECLSPEC Sint32 SDLCALL ATK_Vorbis_GetSourceFirstPageSize(ATK_AudioSource* source);

/**
 * The Vorbis specific implementation of {@link ATK_GetSourceLastPage}.
 *
 * @param source    The audio source
 *
 * @return the index of the last page in the stream, or -1 if there is an error
 */
extern DECLSPEC Sint32 SDLCALL ATK_Vorbis_GetSourceLastPage(ATK_AudioSource* source);

/**
 * The Vorbis specific implementation of {@link ATK_GetSourceCurrentPage}.
 *
 * @param source    The audio source
 *
 * @return the index of the current page in the stream, or -1 if there is an error
 */
extern DECLSPEC Sint32 SDLCALL ATK_Vorbis_GetSourceCurrentPage(ATK_AudioSource* source);

/**
 * The Vorbis specific implementation of {@link ATK_IsSourceEOF}.
 *
 * @param source    The audio source
 *
 * @return 1 if the audio source is at the end of the stream; 0 otherwise
 */
extern DECLSPEC Uint32 SDLCALL ATK_Vorbis_IsSourceEOF(ATK_AudioSource* source);

/**
 * The Vorbis specific implementation of {@link ATK_ReadSourcePage}.
 *
 * @param source    The audio source
 * @param buffer    The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
extern DECLSPEC Sint32 SDLCALL ATK_Vorbis_ReadSourcePage(ATK_AudioSource* source, float* buffer);

/**
 * The Vorbis specific implementation of {@link ATK_ReadSource}.
 *
 * @param source    The audio source
 * @param buffer    The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
extern DECLSPEC Sint64 SDLCALL ATK_Vorbis_ReadSource(ATK_AudioSource* source, float* buffer);

/**
 * The FLAC specific implementation of {@link ATK_UnloadSource}.
 *
 * @param source    The source to close
 * 
 * @return 0 if the source was successfully closed, -1 otherwise.
 */
extern DECLSPEC Sint32 SDLCALL ATK_FLAC_UnloadSource(ATK_AudioSource* source);

/**
 * The FLAC specific implementation of {@link ATK_SeekSourcePage}.
 *
 * @param source    The audio source
 * @param page      The page to seek
 *
 * @return the page acquired, or -1 if there is an error
 */
extern DECLSPEC Sint32 SDLCALL ATK_FLAC_SeekSourcePage(ATK_AudioSource* source, Uint32 page);

/**
 * The FLAC specific implementation of {@link ATK_GetSourcePageSize}.
 *
 * @param source    The audio source
 *
 * @return the number of audio frames in a page, or -1 if there is an error
 */
extern DECLSPEC Sint32 SDLCALL ATK_FLAC_GetSourcePageSize(ATK_AudioSource* source);

/**
 * The FLAC specific implementation of {@link ATK_GetSourceFirstPageSize}.
 *
 * @param source    The audio source
 *
 * @return the number of audio frames on the first page, or -1 if there is an error
 */
extern DECLSPEC Sint32 SDLCALL ATK_FLAC_GetSourceFirstPageSize(ATK_AudioSource* source);

/**
 * The FLAC specific implementation of {@link ATK_GetSourceLastPage}.
 *
 * @param source    The audio source
 *
 * @return the index of the last page in the stream, or -1 if there is an error
 */
extern DECLSPEC Sint32 SDLCALL ATK_FLAC_GetSourceLastPage(ATK_AudioSource* source);

/**
 * The FLAC specific implementation of {@link ATK_GetSourceCurrentPage}.
 *
 * @param source    The audio source
 *
 * @return the index of the current page in the stream, or -1 if there is an error
 */
extern DECLSPEC Sint32 SDLCALL ATK_FLAC_GetSourceCurrentPage(ATK_AudioSource* source);

/**
 * The FLAC specific implementation of {@link ATK_IsSourceEOF}.
 *
 * @param source    The audio source
 *
 * @return 1 if the audio source is at the end of the stream; 0 otherwise
 */
extern DECLSPEC Uint32 SDLCALL ATK_FLAC_IsSourceEOF(ATK_AudioSource* source);

/**
 * The FLAC specific implementation of {@link ATK_ReadSourcePage}.
 *
 * @param source    The audio source
 * @param buffer    The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
extern DECLSPEC Sint32 SDLCALL ATK_FLAC_ReadSourcePage(ATK_AudioSource* source, float* buffer);

/**
 * The FLAC specific implementation of {@link ATK_ReadSource}.
 *
 * @param source    The audio source
 * @param buffer    The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
extern DECLSPEC Sint64 SDLCALL ATK_FLAC_ReadSource(ATK_AudioSource* source, float* buffer);

/**
 * The MP3 specific implementation of {@link ATK_UnloadSource}.
 *
 * @param source    The source to close
 * 
 * @return 0 if the source was successfully closed, -1 otherwise.
 */
extern DECLSPEC Sint32 SDLCALL ATK_MP3_UnloadSource(ATK_AudioSource* source);

/**
 * The MP3 specific implementation of {@link ATK_SeekSourcePage}.
 *
 * @param source    The audio source
 * @param page      The page to seek
 *
 * @return the page acquired, or -1 if there is an error
 */
extern DECLSPEC Sint32 SDLCALL ATK_MP3_SeekSourcePage(ATK_AudioSource* source, Uint32 page);

/**
 * The MP3 specific implementation of {@link ATK_GetSourcePageSize}.
 *
 * @param source    The audio source
 *
 * @return the number of audio frames in a page, or -1 if there is an error
 */
extern DECLSPEC Sint32 SDLCALL ATK_MP3_GetSourcePageSize(ATK_AudioSource* source);

/**
 * The MP3 specific implementation of {@link ATK_GetSourceFirstPageSize}.
 *
 * @param source    The audio source
 *
 * @return the number of audio frames on the first page, or -1 if there is an error
 */
extern DECLSPEC Sint32 SDLCALL ATK_MP3_GetSourceFirstPageSize(ATK_AudioSource* source);

/**
 * The MP3 specific implementation of {@link ATK_GetSourceLastPage}.
 *
 * @param source    The audio source
 *
 * @return the index of the last page in the stream, or -1 if there is an error
 */
extern DECLSPEC Sint32 SDLCALL ATK_MP3_GetSourceLastPage(ATK_AudioSource* source);

/**
 * The MP3 specific implementation of {@link ATK_GetSourceCurrentPage}.
 *
 * @param source    The audio source
 *
 * @return the index of the current page in the stream, or -1 if there is an error
 */
extern DECLSPEC Sint32 SDLCALL ATK_MP3_GetSourceCurrentPage(ATK_AudioSource* source);

/**
 * The MP3 specific implementation of {@link ATK_IsSourceEOF}.
 *
 * @param source    The audio source
 *
 * @return 1 if the audio source is at the end of the stream; 0 otherwise
 */
extern DECLSPEC Uint32 SDLCALL ATK_MP3_IsSourceEOF(ATK_AudioSource* source);

/**
 * The MP3 specific implementation of {@link ATK_ReadSourcePage}.
 *
 * @param source    The audio source
 * @param buffer    The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
extern DECLSPEC Sint32 SDLCALL ATK_MP3_ReadSourcePage(ATK_AudioSource* source, float* buffer);

/**
 * The MP3 specific implementation of {@link ATK_ReadSource}.
 *
 * @param source    The audio source
 * @param buffer    The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
extern DECLSPEC Sint64 SDLCALL ATK_MP3_ReadSource(ATK_AudioSource* source, float* buffer);

/**
 * The WAV specific implementation of {@link ATK_UnloadSource}.
 *
 * @param source    The source to close
 * 
 * @return 0 if the source was successfully closed, -1 otherwise.
 */
extern DECLSPEC Sint32 SDLCALL ATK_WAV_UnloadSource(ATK_AudioSource* source);

/**
 * The WAV specific implementation of {@link ATK_SeekSourcePage}.
 *
 * @param source    The audio source
 * @param page      The page to seek
 *
 * @return the page acquired, or -1 if there is an error
 */
extern DECLSPEC Sint32 SDLCALL ATK_WAV_SeekSourcePage(ATK_AudioSource* source, Uint32 page);

/**
 * The WAV specific implementation of {@link ATK_GetSourcePageSize}.
 *
 * @param source    The audio source
 *
 * @return the number of audio frames in a page, or -1 if there is an error
 */
extern DECLSPEC Sint32 SDLCALL ATK_WAV_GetSourcePageSize(ATK_AudioSource* source);

/**
 * The WAV specific implementation of {@link ATK_GetSourceFirstPageSize}.
 *
 * @param source    The audio source
 *
 * @return the number of audio frames on the first page, or -1 if there is an error
 */
extern DECLSPEC Sint32 SDLCALL ATK_WAV_GetSourceFirstPageSize(ATK_AudioSource* source);

/**
 * The WAV specific implementation of {@link ATK_GetSourceLastPage}.
 *
 * @param source    The audio source
 *
 * @return the index of the last page in the stream, or -1 if there is an error
 */
extern DECLSPEC Sint32 SDLCALL ATK_WAV_GetSourceLastPage(ATK_AudioSource* source);

/**
 * The WAV specific implementation of {@link ATK_GetSourceCurrentPage}.
 *
 * @param source    The audio source
 *
 * @return the index of the current page in the stream, or -1 if there is an error
 */
extern DECLSPEC Sint32 SDLCALL ATK_WAV_GetSourceCurrentPage(ATK_AudioSource* source);

/**
 * The WAV specific implementation of {@link ATK_IsSourceEOF}.
 *
 * @param source    The audio source
 *
 * @return 1 if the audio source is at the end of the stream; 0 otherwise
 */
extern DECLSPEC Uint32 SDLCALL ATK_WAV_IsSourceEOF(ATK_AudioSource* source);

/**
 * The WAV specific implementation of {@link ATK_ReadSourcePage}.
 *
 * @param source    The audio source
 * @param buffer    The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
extern DECLSPEC Sint32 SDLCALL ATK_WAV_ReadSourcePage(ATK_AudioSource* source, float* buffer);

/**
 * The WAV specific implementation of {@link ATK_ReadSource}.
 *
 * @param source    The audio source
 * @param buffer    The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
extern DECLSPEC Sint64 SDLCALL ATK_WAV_ReadSource(ATK_AudioSource* source, float* buffer);

#pragma mark -
#pragma mark Stream Encoding
/**
 * The Vorbis specific implementation of {@link ATK_WriteEncoding}.
 *
 * @param encoding  The encoding stream
 * @param buffer    The data buffer with the audio samples
 * @param frames    The number of audio frames to write
 *
 * @return the number of frames written, or -1 on error
 */
extern DECLSPEC Sint64 SDLCALL ATK_Vorbis_WriteEncoding(ATK_AudioEncoding* encoding,
                                                        float* buffer, size_t frames);

/**
 * The Vorbis specific implementation of {@link ATK_FinishEncoding}.
 *
 * @param encoding  The encoding stream
 *
 * @return 0 if the enconding was successfully completed, -1 otherwise.
 */
extern DECLSPEC Sint32 SDLCALL ATK_Vorbis_FinishEncoding(ATK_AudioEncoding* encoding);

/**
 * The FLAC specific implementation of {@link ATK_WriteEncoding}.
 *
 * @param encoding  The encoding stream
 * @param buffer    The data buffer with the audio samples
 * @param frames    The number of audio frames to write
 *
 * @return the number of frames written, or -1 on error
 */
extern DECLSPEC Sint64 SDLCALL ATK_FLAC_WriteEncoding(ATK_AudioEncoding* encoding,
                                                      float* buffer, size_t frames);

/**
 * The FLAC specific implementation of {@link ATK_FinishEncoding}.
 *
 * @param encoding  The encoding stream
 *
 * @return 0 if the enconding was successfully completed, -1 otherwise.
 */
extern DECLSPEC Sint32 SDLCALL ATK_FLAC_FinishEncoding(ATK_AudioEncoding* encoding);

/**
 * The WAV specific implementation of {@link ATK_WriteEncoding}.
 *
 * @param encoding  The encoding stream
 * @param buffer    The data buffer with the audio samples
 * @param frames    The number of audio frames to write
 *
 * @return the number of frames written, or -1 on error
 */
extern DECLSPEC Sint64 SDLCALL ATK_WAV_WriteEncoding(ATK_AudioEncoding* encoding,
                                                     float* buffer, size_t frames);

/**
 * The WAV specific implementation of {@link ATK_FinishEncoding}.
 *
 * @param encoding  The encoding stream
 *
 * @return 0 if the enconding was successfully completed, -1 otherwise.
 */
extern DECLSPEC Sint32 SDLCALL ATK_WAV_FinishEncoding(ATK_AudioEncoding* encoding);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include <close_code.h>

#endif /* __ATK_CODEC_C_H__ */
