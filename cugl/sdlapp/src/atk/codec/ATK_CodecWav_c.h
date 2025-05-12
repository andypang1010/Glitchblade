/*
 * SDL_atk:  An audio toolkit library for use with SDL
 * Copyright (C) 2022-2023 Walker M. White
 * Based on SDL by (C) 1997-2023 Sam Lantinga <slouken@libsdl.org>
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
#ifndef __ATK_CODEC_WAV_C_H__
#define __ATK_CODEC_WAV_C_H__
/*
 * This is a WAV file loading framework.
 *
 * It is heavily based on an old version of SDL_wave.c by Sam Lantinga. It
 * has been refactored to support WAV streaming.
 */
 
/** Unexported SDL2 values */
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifndef INT_MAX
/* Make a lucky guess. */
#define INT_MAX SDL_MAX_SINT32
#endif
#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

#ifndef SDL_FALLTHROUGH
#if (defined(__cplusplus) && __cplusplus >= 201703L) || \
    (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202000L)
#define SDL_FALLTHROUGH [[fallthrough]]
#else
#if defined(__has_attribute)
#define _HAS_FALLTHROUGH __has_attribute(__fallthrough__)
#else
#define _HAS_FALLTHROUGH 0
#endif /* __has_attribute */
#if _HAS_FALLTHROUGH && \
   ((defined(__GNUC__) && __GNUC__ >= 7) || \
    (defined(__clang_major__) && __clang_major__ >= 10))
#define SDL_FALLTHROUGH __attribute__((__fallthrough__))
#else
#define SDL_FALLTHROUGH do {} while (0) /* fallthrough */
#endif /* _HAS_FALLTHROUGH */
#undef _HAS_FALLTHROUGH
#endif /* C++17 or C2x */
#endif /* SDL_FALLTHROUGH not defined */

#ifndef SDL_PRIu32
#ifdef PRIu32
#define SDL_PRIu32 PRIu32
#else
#define SDL_PRIu32 "u"
#endif
#endif

 
/* RIFF WAVE files are little-endian */

/*******************************************/
/* Define values for Microsoft WAVE format */
/*******************************************/

/* FOURCC */
#define RIFF            0x46464952      /* "RIFF" */
#define WAVE            0x45564157      /* "WAVE" */
#define FACT            0x74636166      /* "fact" */
#define LIST            0x5453494c      /* "LIST" */
#define INFO            0x4f464e49      /* "INFO" */
#define BEXT            0x74786562      /* "bext" */
#define JUNK            0x4B4E554A      /* "JUNK" */
#define FMT             0x20746D66      /* "fmt " */
#define DATA            0x61746164      /* "data" */

/* Format tags */
#define UNKNOWN_CODE    0x0000
#define PCM_CODE        0x0001
#define MS_ADPCM_CODE   0x0002
#define IEEE_FLOAT_CODE 0x0003
#define ALAW_CODE       0x0006
#define MULAW_CODE      0x0007
#define IMA_ADPCM_CODE  0x0011
#define MPEG_CODE       0x0050
#define MPEGLAYER3_CODE 0x0055
#define EXTENSIBLE_CODE 0xFFFE

/** Default streaming size */
#define WAV_PAGE_SIZE 4096

#pragma mark -
#pragma mark WAV Decoding

#ifdef LOAD_WAV

/* Stores the WAVE format information. */
typedef struct WaveFormat {
    Uint16 formattag;       /* Raw value of the first field in the fmt chunk data. */
    Uint16 encoding;        /* Actual encoding, possibly from the extensible header. */
    Uint16 channels;        /* Number of channels. */
    Uint32 frequency;       /* Sampling rate in Hz. */
    Uint32 byterate;        /* Average bytes per second. */
    Uint16 blockalign;      /* Bytes per block. */
    Uint16 bitspersample;   /* Currently supported are 8, 16, 24, 32, and 4 for ADPCM. */

    /* 
     * Extra information size. 
     *
     * Number of extra bytes starting at byte 18 in the fmt chunk data. This is at least 
     * 22 for the extensible header.
     */
    Uint16 extsize;

    /* Extensible WAVE header fields */
    Uint16 validsamplebits;
    Uint32 samplesperblock; /* For compressed formats. Can be zero. Actually 16 bits in the header. */
    Uint32 channelmask;
    Uint8 subformat[16];    /* A format GUID. */
} WaveFormat;

/* Stores information on the fact chunk. */
typedef struct WaveFact {
    /* 
     * Represents the state of the fact chunk in the WAVE file.
     *
     * Set to -1 if the fact chunk is invalid.
     * Set to 0 if the fact chunk is not present
     * Set to 1 if the fact chunk is present and valid.
     * Set to 2 if samplelength is going to be used as the number of sample frames.
     */
    Sint32 status;

    /* 
     * Version 1 of the RIFF specification calls the field in the fact chunk dwFileSize. 
     *
     * The Standards Update then calls it dwSampleLength and specifies that it is 'the 
     * length of the data in samples'. WAVE files from Windows with this chunk have it 
     * set to the samples per channel (sample frames). This is useful to truncate 
     * compressed audio to a specific sample count because a compressed block is usually 
     * decoded to a fixed number of sample frames.
     */
    Uint32 samplelength; /* Raw sample length value from the fact chunk. */
} WaveFact;

/* Generic struct for the chunks in the WAVE file. */
typedef struct WaveChunk {
    Uint32 fourcc;   /* FOURCC of the chunk. */
    Uint32 length;   /* Size of the chunk data. */
    Sint64 position; /* Position of the data in the stream. */
    Uint8 *data;     /* When allocated, this points to the chunk data. length is used for the memory allocation size. */
    size_t size;     /* Number of bytes in data that could be read from the stream. Can be smaller than length. */
} WaveChunk;

/**
 * A reference to a block of data in the wave source
 *
 * This is a replacement for WaveChunk to allow streaming. It tracks the position
 * in the file, but only allocated enough memory for a page at a time
 */
typedef struct WaveBlock {
    size_t start;    /* The start position in the stream */
    size_t length;   /* Number of bytes in data that could be read from the stream. */
    size_t position; /* The current position of the data in the stream. */
    size_t pagesize; /* The size to read per page (data is at least this large) */
    Uint8 *data;     /* Small buffer to hold a single page */
    Sint32 size;     /* Amount of data in the buffer (-1 for error) */
} WaveBlock;


/* Controls how the size of the RIFF chunk affects the loading of a WAVE file. */
typedef enum WaveRiffSizeHint {
    RiffSizeNoHint,
    RiffSizeForce,
    RiffSizeIgnoreZero,
    RiffSizeIgnore,
    RiffSizeMaximum
} WaveRiffSizeHint;

/* Controls how a truncated WAVE file is handled. */
typedef enum WaveTruncationHint {
    TruncNoHint,
    TruncVeryStrict,
    TruncStrict,
    TruncDropFrame,
    TruncDropBlock
} WaveTruncationHint;

/* Controls how the fact chunk affects the loading of a WAVE file. */
typedef enum WaveFactChunkHint {
    FactNoHint,
    FactTruncate,
    FactStrict,
    FactIgnoreZero,
    FactIgnore
} WaveFactChunkHint;

/** High level representation of a WAV file */
typedef struct WaveFile {
    /** The underlying data source */
    SDL_RWops* source;
    
    /** The file format */
    WaveFormat format;

    /** The next chunk to read */
    WaveChunk chunk;

    /** The state of the fact chunk */
    WaveFact fact;
    
    /** The INFO metadata block */
    WaveChunk info;

    /** The sample data block */
    WaveBlock data;

    /* 
     * Number of sample frames that will be decoded.
     *
     * This is calculated either with the size of the data chunk or, if the appropriate
     * hint is enabled, with the sample length value from the fact chunk.
     */
    Sint64 sampleframes;
    
    /* 
     * The file offset for the sample data
     */
    Sint64 samplestart;

    void *decoderdata;   /* Some decoders require extra data for a state. */

    /** Whether we are responsible for freeing the underlying source */
    int ownsource;
    /** The sample format */
    SDL_AudioFormat samplefmt;

    /** The SDL hints for the the RIFF size */
    WaveRiffSizeHint riffhint;
    /** The SDL hints for the truncation */
    WaveTruncationHint trunchint;
    /** The SDL hints for the the chunk */
    WaveFactChunkHint facthint;
} WaveFile;

/** The WAV GUID */
typedef struct WaveExtensibleGUID {
    Uint16 encoding;
    Uint8 guid[16];
} WaveExtensibleGUID;

/** For saving and restoring */
typedef struct WaveState {
    size_t filepos;
    Sint64 framesleft;
} WaveState;

/** Coefficient data for MS ADPCM file */
typedef struct MS_ADPCM_CoeffData {
    Uint16 coeffcount;
    Sint16 *coeff;
    Sint16 aligndummy; /* Has to be last member. */
} MS_ADPCM_CoeffData;

/** Channel state for MS ADPCM file */
typedef struct MS_ADPCM_ChannelState {
    Uint16 delta;
    Sint16 coeff1;
    Sint16 coeff2;
} MS_ADPCM_ChannelState;

/** Internal decoder state for ADPCM files */
typedef struct ADPCM_DecoderState {
    Uint32 channels;        /* Number of channels. */
    size_t blocksize;       /* Size of an ADPCM block in bytes. */
    size_t blockheadersize; /* Size of an ADPCM block header in bytes. */
    size_t samplesperblock; /* Number of samples per channel in an ADPCM block. */
    size_t framesize;       /* Size of a sample frame (16-bit PCM) in bytes. */
    Sint64 framestotal;     /* Total number of sample frames. */
    Sint64 framesleft;      /* Number of sample frames still to be decoded. */
    void *cstate;           /* Decoding state for each channel. */

    /** Coefficients for MS ADPCM */
    MS_ADPCM_CoeffData* mscoeff;
    
    /* ADPCM data. */
    struct {
        Uint8 *data;
        size_t size;
        size_t pos;
    } input;

    /* Current ADPCM block in the ADPCM data above. */
    struct {
        Uint8 *data;
        size_t size;
        size_t pos;
    } block;

    /* Decoded 16-bit PCM data. */
    struct {
        Sint16 *data;
        size_t size;
        size_t pos;
    } output;
} ADPCM_DecoderState;

#endif // LOAD_WAV

#pragma mark -
#pragma mark WAV Encoding

#ifdef SAVE_WAV

/** WAV audio file header */
typedef struct WaveHead {
    /** The RIFF Four CC */
    Uint32 riffcc;
    /** The file length (minus this and the RIFF) */
    Uint32 package_len;
    /** The WAVE Four CC */
    Uint32 wavecc;
    /** The FMT Four CC */
    Uint32 fmtcc;
    /** The length of all the data above */
    Uint32 format_len;
    /** Type of format (1 is PCM) */
    Uint16 fixed;
    /** The number of channels */
    Uint16 channels;
    /** The sample rate in Hz */
    Uint32 sample_rate;
    /** (Sample Rate * BitsPerSample * Channels) / 8 */
    Uint32 byte_rate;
    /** (BitsPerSample * Channels) / 8 */
    Uint16 byte_per_sample;
    /** The bits per sample */
    Uint16 bits_per_sample;
} WaveHead;

/** The encoder data structure for the Wave file */
typedef struct WaveOut {
    /** The underlying data source */
    SDL_RWops* source;
    /** Whether we are responsible for freeing the underlying source */
    int ownsource;
    /** The initial file position */
    size_t begin;
    /** The number of frames */
    size_t written;
    
    /** The metadata length */
    size_t meta_len;
    /** The data (post FOURCC) offset */
    size_t data_off;
} WaveOut;


#endif

#endif // __ATK_CODEC_WAV_C_H__
