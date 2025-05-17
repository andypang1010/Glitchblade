/*
 * SDL_atk:  An audio toolkit library for use with SDL
 * Copyright (C) 2022-2023 Walker M. White
 *
 * This is a library to load different types of audio files as PCM data,
 * and process them with basic DSP tools. The goal of this library is to
 * provide an alternative to SDL_sound that supports efficient streaming
 * and file output. In addition, it provides a minimal math library akin
 * to (and inspired by) numpy for audio processing. This enables the
 * developer to add custom audio effects that are not possible in SDL_mixer.
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

/**
 * @file ATK_codec.h
 *
 * Header file for the codec component of SDL_atk
 *
 * This component provides the API for loading and saving to audio files. These
 * files can be processed even when the audio subsystem is not initialized.
 */
#ifndef __ATK_CODEC_H__
#define __ATK_CODEC_H__
#include <SDL.h>
#include <SDL_version.h>
#include <begin_code.h>

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * The file types supported by this library.
 *
 * Future versions of this library may add additional formats to give
 * it parity with SDL_mixer. 
 */
typedef enum {
    ATK_CODEC_WAV    = 0x00000001,         /**< WAV File */
    ATK_CODEC_VORBIS = 0x00000002,         /**< OGG Vorbis Audio */
    ATK_CODEC_FLAC   = 0x00000003,         /**< Xiph FLAC */
    ATK_CODEC_MP3    = 0x00000004,         /**< MP3 Audio */
} ATK_CodecType;

#pragma mark -
#pragma mark Stream Metadata
/**
 * The representation of a metadata comment entry for an audio source
 *
 * All metadata comments consists of key-value pairs. Not all codecs support
 * metadata comments. For maximum compatibilty, we represent all metadata
 * comments as key-value pairs, as per the Vorbis comment specification:
 *
 * https://en.wikipedia.org/wiki/Vorbis_comment
 *
 * However, in some cases that may be implemented on top of other specifications
 * such as ID3 or WAV INFO chunks. This may limit the number of keys/tags that
 * are supported. See {@link ATK_GetCommentTags} to get a list of the tags
 * supported by any given codec.
 */
typedef struct {
    /** The metadata key */
    const char* key;
    /** The metadata value */
    const char* value;
} ATK_AudioComment;

/**
 * The metadata associated with an audio source or encoding.
 *
 * This information is necessary to apply structure to the source or encoding.
 * We pulled it out of {@link ATK_AudioSource} and {#link ATK_AudioEncoding}
 * so that we can have a uniform interface.
 */
typedef struct {
    /** The number of channels in this source (max 32) */
    const Uint8  channels;
    
    /** The sampling rate (frequency) of this source */
    const Uint32 rate;
    
    /** The number of frames in this source */
    const Uint64 frames;
    
    /** The number of metadata comments */
    const Uint16 num_comments;

    /** An array of metadata comments */
    const ATK_AudioComment* comments;
    
} ATK_AudioMetadata;

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
extern DECLSPEC SDL_bool SDLCALL ATK_SupportsComments(ATK_CodecType type);

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
extern DECLSPEC const char* const * SDLCALL ATK_GetCommentTags(ATK_CodecType type);

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
extern DECLSPEC SDL_bool SDLCALL ATK_SupportsCommentTag(ATK_CodecType type, const char* tag);

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
extern DECLSPEC const char* SDLCALL ATK_GetID3CommentTag(const char* tag);

/**
 * Returns the comment tag equivalent to the given INFO chunk tag
 *
 * WAV files use the INFO specification for their metadata. However, to provide a
 * uniform comment interface, these tags are expanded into proper words matching
 * the Vorbis comment interface. This method returns the Vorbis comment equivalent
 * for an INFO tag. For information on the INFO specification see
 *
 * https://www.robotplanet.dk/audio/wav_meta_data/
 *
 * If tag is not a supported INFO tag, this method returns NULL.
 *
 * @param tag   The INFO tag
 *
 * @return the comment tag equivalent to the given INFO chunk tag
 */
extern DECLSPEC const char* SDLCALL ATK_GetInfoCommentTag(const char* tag);

/**
 * Returns a comment array for the given key-value pairs
 *
 * The two string arrays should be of the same length. The audio comments will
 * allocate space for the strings so that they store a local copy. Hence it is
 * safe for these arrays to be volatile.
 *
 * @param tags      The comment tag names
 * @param values    The comment values
 * @param len       The size of the tag/value arrays
 *
 * @return a comment array for the given key-value pairs
 */
extern DECLSPEC ATK_AudioComment* SDLCALL ATK_AllocComments(const char** tags,
                                                            const char** values, size_t len);

/**
 * Frees a previously allocated collection of metadata comments
 *
 * @param comments  The metadata comment list
 */
extern DECLSPEC void SDLCALL ATK_FreeComments(ATK_AudioComment* comments, size_t len);

/**
 * Returns a copy of the comments array.
 *
 * This method returns a newly allocated array. It is the responsibility of this caller
 * to free these comments (with {@link ATK_FreeComments}) when done.
 *
 * @param comments	The array of comments
 * @param len		The size of the array
 *
 * @return a copy of the comments array.
 */
extern DECLSPEC ATK_AudioComment* SDLCALL ATK_CopyComments(const ATK_AudioComment* comments, size_t len);

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
extern DECLSPEC ATK_AudioMetadata* SDLCALL ATK_AllocMetadata(Uint8 channels, Uint32 rate, Uint64 frames,
                                                             const ATK_AudioComment* comments, Uint16 len);

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
extern DECLSPEC void SDLCALL ATK_FreeMetadata(ATK_AudioMetadata* metadata, int deep);

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
extern DECLSPEC ATK_AudioMetadata* SDLCALL ATK_CopyMetadata(ATK_AudioMetadata* metadata, int deep);

#pragma mark -
#pragma mark Stream Decoding
/** 
 * The representation of an audio source (as a stream).
 *
 * The source is stateful in the sense that there is an active page at any
 * time and reading from the source increments the page.
 */
typedef struct {
    /** The type for this source */
    const ATK_CodecType type;

    /** The audio metadata */
    const ATK_AudioMetadata metadata;

    /** An opaque reference to the format specific decoder */
    void* decoder;

} ATK_AudioSource;

/**
 * Creates a new ATK_AudioSource from the given file
 * 
 * This function will return NULL if the file cannot be located or is not an
 * proper audio file. The file will not be read into memory, but is instead
 * available for streaming. If {@link ATK_init} has been called, a managed file
 * will be used in place of a traditional file.
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
extern DECLSPEC ATK_AudioSource* SDLCALL ATK_LoadSource(const char* filename);

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
extern DECLSPEC ATK_AudioSource* SDLCALL ATK_LoadSource_RW(SDL_RWops* source, int ownsrc);

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
 * @param type      A filename extension that represents this data ("WAV", "MP3", etc)
 *
 * @return a new ATK_AudioSource from the given readable/seekable RWops
 */
extern DECLSPEC ATK_AudioSource* SDLCALL ATK_LoadTypedSource_RW(SDL_RWops* source, int ownsrc, const char* type);

/** 
 * Creates a new ATK_AudioSource from an OGG Vorbis file
 * 
 * This function will return NULL if the file cannot be located or is not an
 * proper OGG Vorbis file. The file will not be read into memory, but is instead
 * available for streaming. If {@link ATK_init} has been called, a managed file
 * will be used in place of a traditional file.
 *
 * It is the responsibility of the caller of this function to close the source
 * (with {@link ATK_UnloadSource}) when it is no longer needed.
 *
 * @param filename    The name of the file with audio data
 *
 * @return a new ATK_AudioSource from an OGG Vorbis file
 */
extern DECLSPEC ATK_AudioSource* SDLCALL ATK_LoadVorbis(const char* filename);

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
extern DECLSPEC ATK_AudioSource* ATK_LoadVorbis_RW(SDL_RWops* source, int ownsrc);

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
extern DECLSPEC SDL_bool SDLCALL ATK_SourceIsVorbis(SDL_RWops* source);

/** 
 * Creates a new ATK_AudioSource from an Xiph FLAC file
 * 
 * This function will return NULL if the file cannot be located or is not an
 * proper Xiph FLAC file. The file will not be read into memory, but is instead
 * available for streaming. If {@link ATK_init} has been called, a managed file
 * will be used in place of a traditional file.
 *
 * It is the responsibility of the caller of this function to close the source
 * (with {@link ATK_UnloadSource}) when it is no longer needed.
 *
 * @param filename    The name of the file with audio data
 *
 * @return a new ATK_AudioSource from an Xiph FLAC file
 */
extern DECLSPEC ATK_AudioSource* SDLCALL ATK_LoadFLAC(const char* filename);

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
extern DECLSPEC ATK_AudioSource* SDLCALL ATK_LoadFLAC_RW(SDL_RWops* source, int ownsrc);

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
extern DECLSPEC SDL_bool SDLCALL ATK_SourceIsFLAC(SDL_RWops* source);

/** 
 * Creates a new ATK_AudioSource from an MP3 file
 * 
 * This function will return NULL if the file cannot be located or is not an
 * supported MP3 file. SDL_atk's MP3 support is minimal, so some advanced encodings
 * may not be supported. The file will not be read into memory, but is instead
 * available for streaming. If {@link ATK_init} has been called, a managed file
 * will be used in place of a traditional file.
 *
 * It is the responsibility of the caller of this function to close the source
 * (with {@link ATK_UnloadSource}) when it is no longer needed.
 *
 * @param filename    The name of the file with audio data
 *
 * @return a new ATK_AudioSource from an MP3 file
 */
extern DECLSPEC ATK_AudioSource* SDLCALL ATK_LoadMP3(const char* filename);

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
extern DECLSPEC ATK_AudioSource* SDLCALL ATK_LoadMP3_RW(SDL_RWops* source, int ownsrc);

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
extern DECLSPEC SDL_bool SDLCALL ATK_SourceIsMP3(SDL_RWops* source);

/** 
 * Creates a new ATK_AudioSource from an WAV file
 * 
 * This function will return NULL if the file cannot be located or is not an
 * supported WAV file. Note that WAV is a container type in addition to a codec,
 * and so not all WAV files are supported. The file will not be read into memory, 
 * but is instead available for streaming. If {@link ATK_init} has been called, a 
 * managed file will be used in place of a traditional file.
 *
 * It is the responsibility of the caller of this function to close the source
 * (with {@link ATK_UnloadSource}) when it is no longer needed.
 *
 * @param filename    The name of the file with audio data
 *
 * @return a new CODEC_Source from an WAV file
 */
extern DECLSPEC ATK_AudioSource* SDLCALL ATK_LoadWAV(const char* filename);

/** 
 * Creates a new ATK_AudioSource from an WAV readable/seekable RWops
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
 * @return a new CODEC_Source from an WAV readable/seekable RWops
 */
extern DECLSPEC ATK_AudioSource* SDLCALL ATK_LoadWAV_RW(SDL_RWops* source, int ownsrc);

/**
 * Detects WAV data on a readable/seekable SDL_RWops.
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
 * @returns SDL_TRUE if this is WAV data.
 */
extern DECLSPEC SDL_bool SDLCALL ATK_SourceIsWAV(SDL_RWops* source);

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
extern DECLSPEC Sint32 SDLCALL ATK_UnloadSource(ATK_AudioSource* source);

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
extern DECLSPEC Sint32 SDLCALL ATK_SeekSourcePage(ATK_AudioSource* source, Uint32 page);

/**
 * Returns the number of audio frames in an audio source page.
 *
 * An audio frame is a collection of simultaneous samples for different channels.
 * Multiplying the page size by the number of channels produces the number of samples
 * in a page.
 *
 * @param source    The audio source
 *
 * @return the number of audio frames in a page, or -1 if there is an error
 */
extern DECLSPEC Sint32 SDLCALL ATK_GetSourcePageSize(ATK_AudioSource* source);

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
extern DECLSPEC Sint32 SDLCALL ATK_GetSourceFirstPageSize(ATK_AudioSource* source);

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
extern DECLSPEC Sint32 SDLCALL ATK_GetSourceLastPage(ATK_AudioSource* source);

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
extern DECLSPEC Sint32 SDLCALL ATK_GetSourceCurrentPage(ATK_AudioSource* source);

/** 
 * Returns 1 if the audio source is at the end of the stream; 0 otherwise
 *
 * @param source    The audio source
 *
 * @return 1 if the audio source is at the end of the stream; 0 otherwise
 */
extern DECLSPEC Uint32 SDLCALL ATK_IsSourceEOF(ATK_AudioSource* source);

/**
 * Reads a single page of audio data into the buffer
 *
 * This function reads in the current source page into the buffer. The data written
 * into the buffer is linear PCM data with interleaved channels. If the source is
 * at the end, nothing will be written. This function will advance the current page
 * when complete.
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
extern DECLSPEC Sint32 SDLCALL ATK_ReadSourcePage(ATK_AudioSource* source, float* buffer);

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
extern DECLSPEC Sint64 SDLCALL ATK_ReadSource(ATK_AudioSource* source, float* buffer);

/**
 * Returns an SDL_RWops (wrapper) with the audio frames of the given audio source
 *
 * The SDL_RWops object returned is read-only. Any attempts to write to it will fail. 
 * The purpose of this object is to provide an interface for a smooth, buffered stream
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
extern DECLSPEC SDL_RWops* SDLCALL ATK_RWFromAudioSourceRW(SDL_RWops* stream,
                                                           int ownsrc, const char* type,
                                                           ATK_AudioMetadata** metadata);

/**
 * Returns an SDL_RWops with the audio frames of the given file
 *
 * The SDL_RWops object returned is read-only. Any attempts to write to it will fail. 
 * The purpose of this object is to provide an interface for a smooth, buffered stream
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
extern DECLSPEC SDL_RWops* SDLCALL ATK_RWFromTypedAudioSource(const char* filename, const char* type,
                                                              ATK_AudioMetadata** metadata);

/**
 * Returns an SDL_RWops with the audio frames of the given file
 *
 * The SDL_RWops object returned is read-only. Any attempts to write to it will fail.
 * The purpose of this object is to provide an interface for a smooth, buffered stream
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
extern DECLSPEC SDL_RWops* SDLCALL ATK_RWFromAudioSource(const char* filename,
                                                         ATK_AudioMetadata** metadata);

#pragma mark -
#pragma mark Stream Encoding
/**
 * The representation of a (partially) encoded audio stream.
 *
 * This type is used to write an audio stream to a file or RWops. Like an
 * audio source, it is stateful, in that it tracks how far we have written.
 */
typedef struct {
    /** The type for this encoding */
    const ATK_CodecType type;

    /** The audio metadata */
    const ATK_AudioMetadata metadata;

    /** An opaque reference to the format specific encoder */
    void* encoder;

} ATK_AudioEncoding;

/**
 * Returns a new encoding stream to write to the given file
 *
 * The provided metadata will be copied to the encoding object. So it is safe to delete
 * this metadata before the encoding is complete. If the encoding stream cannot be
 * allocated, or is not supported, this function will return NULL and an error value
 * will be set.  If {@link ATK_init} has been called, a managed file will be used in 
 * place of a traditional file.
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
extern DECLSPEC ATK_AudioEncoding* SDLCALL ATK_EncodeAudio(const char* filename,
                                                           const char* type,
                                                           const ATK_AudioMetadata* metadata);

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
extern DECLSPEC ATK_AudioEncoding* SDLCALL ATK_EncodeAudio_RW(SDL_RWops* stream, int ownsrc,
                                                              const char* type,
                                                              const ATK_AudioMetadata* metadata);

/**
 * Returns a new Vorbis encoding stream to write to the given file
 *
 * The provided metadata will be copied to the encoding object. So it is safe to delete
 * this metadata before the encoding is complete. If the encoding stream cannot be
 * allocated, or this library does not supported saving to Vorbis, this function will
 * return NULL and an error value will be set. If {@link ATK_init} has been called, 
 * a managed file will be used in place of a traditional file.
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
 * @param metadata  The stream metadata
 *
 * @return a new Vorbis encoding stream to write to the given file
 */
extern DECLSPEC ATK_AudioEncoding* SDLCALL ATK_EncodeVorbis(const char* filename,
                                                            const ATK_AudioMetadata* metadata);

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
 * @param metadata  The stream metadata
 *
 * @return a new Vorbis encoding stream to write to the given RWops
 */
extern DECLSPEC ATK_AudioEncoding* SDLCALL ATK_EncodeVorbis_RW(SDL_RWops* stream, int ownsrc,
                                                               const ATK_AudioMetadata* metadata);

/**
 * Returns a new FLAC encoding stream to write to the given file
 *
 * The provided metadata will be copied to the encoding object. So it is safe to delete
 * this metadata before the encoding is complete. If the encoding stream cannot be
 * allocated, or this library does not supported saving to FLAC, this function will
 * return NULL and an error value will be set. If {@link ATK_init} has been called, 
 * a managed file will be used in place of a traditional file.
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
 * @param metadata  The stream metadata
 *
 * @return a new FLAC encoding stream to write to the given file
 */
extern DECLSPEC ATK_AudioEncoding* SDLCALL ATK_EncodeFLAC(const char* filename,
                                                          const ATK_AudioMetadata* metadata);

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
 * @param metadata  The stream metadata
 *
 * @return a new FLAC encoding stream to write to the given RWops
 */
extern DECLSPEC ATK_AudioEncoding* SDLCALL ATK_EncodeFLAC_RW(SDL_RWops* source, int ownsrc,
                                                             const ATK_AudioMetadata* metadata);

/**
 * Returns a new WAV encoding stream to write to the given file
 *
 * The provided metadata will be copied to the encoding object. So it is safe to delete
 * this metadata before the encoding is complete. If the encoding stream cannot be
 * allocated, or this library does not supported saving to WAV, this function will
 * return NULL and an error value will be set. If {@link ATK_init} has been called, 
 * a managed file will be used in place of a traditional file.
 *
 * The metadata should reflect the properties of the stream to be encoded as much as
 * possible. Currently, our implementation of WAV does not allow for a greater number
 * of frames to be written that was specified in the initial metadata.
 *
 * This function will encode the audio using the default settings of the codec. We
 * do not currently support fine tune control of bit rates or compression options.
 *
 * It is the responsibility of the caller of this function to complete the encoding
 * (with {@link ATK_FinishEncoding}) when the stream is finished.
 *
 * @param filename  The name of the new audio file
 * @param metadata  The stream metadata
 *
 * @return a new FLAC encoding stream to write to the given file
 */
extern DECLSPEC ATK_AudioEncoding* SDLCALL ATK_EncodeWAV(const char* filename,
                                                         const ATK_AudioMetadata* metadata);

/**
 * Returns a new WAV encoding stream to write to the given RWops
 *
 * The provided metadata will be copied to the encoding object. So it is safe to delete
 * this metadata before the encoding is complete. If the encoding stream cannot be
 * allocated, or this library does not supported saving to FLAC, this function will
 * return NULL and an error value will be set.
 *
 * The metadata should reflect the properties of the stream to be encoded as much as
 * possible. Currently, our implementation of WAV does not allow for a greater number
 * of frames to be written that was specified in the initial metadata.
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
 * @param metadata  The stream metadata
 *
 * @return a new WAV encoding stream to write to the given RWops
 */
extern DECLSPEC ATK_AudioEncoding* SDLCALL ATK_EncodeWAV_RW(SDL_RWops* source, int ownsrc,
                                                            const ATK_AudioMetadata* metadata);

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
extern DECLSPEC Sint64 SDLCALL ATK_WriteEncoding(ATK_AudioEncoding* encoding,
                                                 float* buffer, size_t frames);

/**
 * Completes the encoding stream, releasing all resources.
 *
 * This function will delete the ATK_AudioEncoding, making it no longer safe to use.
 *
 * If the audio encoding is writing directly to a file, then the encoding "owns"
 * the underlying file. In that case, it will close/free the file when it is done.
 * However, if the audio encoding was writing to a RWops then it will leave that
 * object open unless ownership was transfered at the time the encoding was
 * created.
 *
 * @param encoding  The encoding stream
 *
 * @return 0 if the enconding was successfully completed, -1 otherwise.
 */
extern DECLSPEC Sint32 SDLCALL ATK_FinishEncoding(ATK_AudioEncoding* encoding);

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
extern DECLSPEC SDL_RWops* SDLCALL ATK_RWToAudioEncodingRW(SDL_RWops* stream, int ownsrc,
                                                           const char* type,
                                                           const ATK_AudioMetadata* metadata);

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
extern DECLSPEC SDL_RWops* SDLCALL ATK_RWToTypedAudioEncoding(const char* filename, const char* type,
                                                              const ATK_AudioMetadata* metadata);

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
extern DECLSPEC SDL_RWops* SDLCALL ATK_RWToAudioEncoding(const char* filename,
                                                         const ATK_AudioMetadata* metadata);


/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include <close_code.h>

#endif /* __ATK_CODEC_H__ */
