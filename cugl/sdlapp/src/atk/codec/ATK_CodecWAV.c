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

/*
 * This is a WAV file loading framework.
 *
 * This file is a modification of SDL_wave.c (from SDL) to implement streams.
 * Looking at the code, it may appear that this source code contains several
 * questionable design decisions. That is because our goal in this file is
 * to preserve SDL_wave.c as much as possible so that we can benefit from
 * updates to that code.
 *
 * The major changes from SDL_wave.c are as follows
 *
 * 1. ADPCM_DecoderState is now the decoder data for WaveFile.
 * This allows us to access this state over multiple functions. Previous
 * decoder data (e.g. coefficients) in WaveFile is now moved inside of
 * ADPCM_DecoderState.
 *
 * 2. We have created a new type, WaveBlock, to replace WaveChunk.
 * WaveBlock does not read the entire chunk into memory, but instead
 * pages in a little bit at a time. This is not a global replacement.
 * It only replaces WaveChunk on large chunks like the DATA segment.
 * In particular, we want to ensure that our API still works on
 * non-seekable RWops.
 *
 * 3. Using WaveBlock, all Decode functions are broken into three parts.
 * The BeginDecode functions initialize the WaveBlock for the decoding
 * process. They do not replace the Init functions, which as still
 * separate, as those functions are invoked before the DATA block is
 * found.  The EndDecode functions provide memory clean up. In-between,
 * StepDecode reads in the next page of data.
 *
 * 4. WaveLoad has been similarly split up.
 * This makes that function consistent with the ATK_AudioSource API.
 *
 * There are other numerous changes to the code, but all other changes
 * are in service to the four points listed above. Whenever possible,
 * we have tried to keep the SDL_wave.c code unchanged.
 */
#include <SDL_atk.h>
#include "ATK_Codec_c.h"
#include <assert.h>
#include <string.h>

/**
 * @file ATK_CodecWav.c
 *
 * This component contains the functions for loading and saving WAV files.
 * It is heavily based on an old version of SDL_wave.c by Sam Lantinga.
 * It has been refactored to support WAV streaming.
 */
#ifdef LOAD_WAV
#include "ATK_CodecWav_c.h"
#include <SDL_stdinc.h>
#pragma mark -
#pragma mark WAV Metadata
/**
 * The supported WAV INFO tags
 */
const char* WAV_TAGS[] = {
    "IART", "ICMT", "ICOP", "ICRD", "IENG", "IGNR", "IKEY",
    "INAM", "IPRD", "ISBJ", "ISFT", "ITCH", "ITRK"
};

/**
 * The WAV INFO tag descriptions
 */
const char* WAV_VERBOSE[] = {
    "Artist", "Comment", "Copyright", "Year", "Engineer", "Genre", "Keywords",
    "Title", "Album", "Subject", "Software", "Encoder", "Track"
};

/**
 * Returns the INFO chuck tag equivalent to the given comment
 *
 * WAV files use the INFO specification for their metadata. However, to provide a
 * uniform comment interface, these tags are expanded into proper words matching
 * the Vorbis comment interface. This function returns the Vorbis comment equivalent
 * for an INFO tag. For information on the INFO specification see
 *
 * https://www.robotplanet.dk/audio/wav_meta_data/
 *
 * If there is no INFO tag for the given comment, this function returns NULL.
 *
 * @param tag   The comment tag
 *
 * @return the comment tag equivalent to the given INFO chunk tag
 */
static const char* ATK_GetCommentInfoTag(const char* tag) {
    size_t len = SDL_arraysize(WAV_TAGS);
    for(size_t ii = 0; ii < len; ii++) {
        if (ATK_string_equals(tag, WAV_VERBOSE[ii])) {
            return WAV_TAGS[ii];
        }
    }
    return NULL;
}

/**
 * Returns the comment tag equivalent to the given INFO chunk tag
 *
 * WAV files use the INFO specification for their metadata. However, to provide a
 * uniform comment interface, these tags are expanded into proper words matching
 * the Vorbis comment interface. This function returns the Vorbis comment equivalent
 * for an INFO tag. For information on the INFO specification see
 *
 * https://www.robotplanet.dk/audio/wav_meta_data/
 *
 * If tag is not a supported INFO tag, this function returns NULL.
 *
 * @param tag   The INFO tag
 *
 * @return the comment tag equivalent to the given INFO chunk tag
 */
const char* ATK_GetInfoCommentTag(const char* tag) {
    size_t len = SDL_arraysize(WAV_TAGS);
    Uint32 code1 = *((Uint32*)tag);
    for(size_t ii = 0; ii < len; ii++) {
        Uint32 code2 = *((Uint32*)WAV_TAGS[ii]);
        if (code1 == code2) {
            return WAV_VERBOSE[ii];
        }
    }
    return NULL;
}

/**
 * Returns SDL_TRUE if WAV supports the given comment tag.
 *
 * @param tag   The tag to query
 *
 * @return SDL_TRUE if WAV supports the given comment tag.
 */
SDL_bool ATK_WAV_SupportsCommentTag(const char* tag) {
    size_t len = SDL_arraysize(WAV_VERBOSE);
    for(size_t ii = 0; ii < len; ii++) {
        if (ATK_string_equals(tag, WAV_VERBOSE[ii])) {
            return SDL_TRUE;
        }
    }
    return SDL_FALSE;
}

/**
 * Returns an array of comment tags supported by the WAV codec
 *
 * @return an array of comment tags supported by the WAV codec
 */
const char* const * ATK_WAV_GetCommentTags() {
    return WAV_VERBOSE;
}

/**
 * Returns a newly allocated list of metadata comments.
 *
 * The comments are parsed using the WAV INFO section specification. For details,
 * see https://www.robotplanet.dk/audio/wav_meta_data/
 *
 * The number of comments in the returned array are stored in the len parameter.
 *
 * @param data  The INFO section
 * @param size  The size of the INFO section
 * @param len   Pointer to store the comment length
 *
 * @return a newly allocated list of metadata comments.
 */
static ATK_AudioComment* ATK_WAV_AllocComments(const Uint8* data, size_t size, int* len) {
    Uint32* curr = (Uint32*)data;
    if (SDL_SwapLE32(*curr) != INFO) {
        return NULL;
    }

    // Count the number of valid keys
    size_t num = 0;
    for(size_t pos = 4; pos < size-4; ) {
        Uint32 len = SDL_SwapLE32(*(Uint32*)(data+pos+4));
        if (len) {
            const char* key = ATK_GetCommentInfoTag((char*)(data+pos));
            if (key) {
                num++;
            }
        }
        pos += 8+len;
    }

    if (!num) {
        return NULL;
    }

    // Now make the tags
    ATK_AudioComment* result = (ATK_AudioComment*)ATK_malloc(sizeof(ATK_AudioComment)*num);

    size_t off = 0;
    for(size_t pos = 4; pos < size-4; ) {
        Uint32 len = SDL_SwapLE32(*(Uint32*)(data+pos+4));
        if (len) {
            const char* key = ATK_GetCommentInfoTag((char*)(data+pos));
            if (key) {
                result[off].key = (char*)ATK_malloc(sizeof(char)*(strlen(key)+1));
                strcpy(DECONST(char*,result[off].key),key);
                result[off].value = (char*)ATK_malloc(sizeof(char)*(len+1));
                memset(DECONST(char*,result[off].value),0,len+1);
                memcpy(DECONST(char*,result[off].value),data+pos+8,len);
                off++;
            }
        }
        pos += 8+len;
    }
    if (len != NULL) {
        *len = (int)num;
    }

    return result;
}

#pragma mark -
#pragma mark WAV Decoding
/**
 * Resets a source to the start of a WaveBlock
 *
 * @param source    The Wave source stream
 * @param source    The wave block with the audio data
 *
 * @return the position of the start of the block (-1 on fail)
 */
static Sint64 Block_Reset(SDL_RWops* source, WaveBlock* block) {
    memset(block->data, 0, block->pagesize);
    Sint64 result = SDL_RWseek(source, block->start, RW_SEEK_SET);
    if (result > 0) {
        block->position = block->start;
    }
    return result;
}

/**
 * Reads the next page of data into a WaveBlock
 *
 * @param source    The Wave source stream
 * @param source    The wave block with the audio data
 *
 * @return the amount of bytes read (-1 on fail)
 */
static Sint64 Block_Read(SDL_RWops* source, WaveBlock* block) {
    size_t off = block->position-block->start;
    size_t amt = block->length-off;
    if (amt >= block->pagesize) {
        amt = block->pagesize;
    }
    block->size = (Sint32)SDL_RWread(source, block->data, 1, amt);
    if (block->size > 0) {
        block->position += block->size;
    }
    return block->size;
}

/**
 * Seeks to a position in the WaveBlock
 *
 * @param source    The Wave source stream
 * @param source    The wave block with the audio data
 * @param position  The psoition to seek
 *
 * @return the new position in bytes (-1 on fail)
 */
static Sint64 Block_Seek(SDL_RWops* source, WaveBlock* block, size_t position) {
    Sint64 result = SDL_RWseek(source, position, RW_SEEK_SET);
    if (result >= 0) {
        block->position = result;
    }
    return result;
}

#pragma mark MS ADPCM WAV
/**
 * Checks the given sampleframes against the number in the fact chunk
 *
 * If the fact chunk has more frames than sampleframes, this function will return an
 * error. Otherwise, it returns the minimum of the fact chuck and the sample frames.
 *
 * @param file          The WAV file to check
 * @param sampleFrames  The candidate number of sampleFrames
 *
 * @return the minimum of samplesFrames and the fact chunk
 */
static Sint64 WaveAdjustToFactValue(WaveFile *file, Sint64 sampleframes) {
    if (file->fact.status == 2) {
        if (file->facthint == FactStrict && sampleframes < file->fact.samplelength) {
            return SDL_SetError("Invalid number of sample frames in WAVE fact chunk (too many)");
        } else if (sampleframes > file->fact.samplelength) {
            return file->fact.samplelength;
        }
    }

    return sampleframes;
}

/**
 * Returns the number of sample frames needed for an MS ADPCM file
 *
 * @param file          The WAV file to check
 * @param datalength    The amount of data encoded
 *
 * @return the number of sample frames needed for an MS ADPCM file
 */
static int MS_ADPCM_CalculateSampleFrames(WaveFile *file, size_t datalength) {
    WaveFormat *format = &file->format;
    const size_t blockheadersize = (size_t)file->format.channels * 7;
    const size_t availableblocks = datalength / file->format.blockalign;
    const size_t blockframebitsize = (size_t)file->format.bitspersample * file->format.channels;
    const size_t trailingdata = datalength % file->format.blockalign;

    if (file->trunchint == TruncVeryStrict || file->trunchint == TruncStrict) {
        /* The size of the data chunk must be a multiple of the block size. */
        if (datalength < blockheadersize || trailingdata > 0) {
            return SDL_SetError("Truncated MS ADPCM block");
        }
    }

    /* Calculate number of sample frames that will be decoded. */
    file->sampleframes = (Sint64)availableblocks * format->samplesperblock;
    if (trailingdata > 0) {
        /* The last block is truncated. Check if we can get any samples out of it. */
        if (file->trunchint == TruncDropFrame) {
            /* Drop incomplete sample frame. */
            if (trailingdata >= blockheadersize) {
                size_t trailingsamples = 2 + (trailingdata - blockheadersize) * 8 / blockframebitsize;
                if (trailingsamples > format->samplesperblock) {
                    trailingsamples = format->samplesperblock;
                }
                file->sampleframes += trailingsamples;
            }
        }
    }

    file->sampleframes = WaveAdjustToFactValue(file, file->sampleframes);
    if (file->sampleframes < 0) {
        return -1;
    }

    return 0;
}

/**
 * Returns a single sample interpolated from two values
 *
 * @param state     The decoder state
 * @param sample1   The first sample to interpolate
 * @param sample2   The second sample to interpolate
 * @param nybble    The next byte to process
 *
 * @return a single sample interpolated from two values
 */
static Sint16 MS_ADPCM_ProcessNibble(MS_ADPCM_ChannelState *cstate, Sint32 sample1, Sint32 sample2, Uint8 nybble) {
    const Sint32 max_audioval = 32767;
    const Sint32 min_audioval = -32768;
    const Uint16 max_deltaval = 65535;
    const Uint16 adaptive[] = {
        230, 230, 230, 230, 307, 409, 512, 614,
        768, 614, 512, 409, 307, 230, 230, 230
    };
    Sint32 new_sample;
    Sint32 errordelta;
    Uint32 delta = cstate->delta;

    new_sample = (sample1 * cstate->coeff1 + sample2 * cstate->coeff2) / 256;
    /* The nibble is a signed 4-bit error delta. */
    errordelta = (Sint32)nybble - (nybble >= 0x08 ? 0x10 : 0);
    new_sample += (Sint32)delta * errordelta;
    if (new_sample < min_audioval) {
        new_sample = min_audioval;
    } else if (new_sample > max_audioval) {
        new_sample = max_audioval;
    }
    delta = (delta * adaptive[nybble]) / 256;
    if (delta < 16) {
        delta = 16;
    } else if (delta > max_deltaval) {
        /* This issue is not described in the Standards Update and therefore
         * undefined. It seems sensible to prevent overflows with a limit.
         */
        delta = max_deltaval;
    }

    cstate->delta = (Uint16)delta;
    return (Sint16)new_sample;
}

/**
 * Decodes the header of an MS ADPCM block, updating state
 *
 * @param state the state containing the block to process
 *
 * @return 0 on success, error code on error.
 */
static int MS_ADPCM_DecodeBlockHeader(ADPCM_DecoderState *state) {
    Uint8 coeffindex;
    const Uint32 channels = state->channels;
    Sint32 sample;
    Uint32 c;
    MS_ADPCM_ChannelState *cstate = (MS_ADPCM_ChannelState *)state->cstate;
    MS_ADPCM_CoeffData *ddata = state->mscoeff;

    for (c = 0; c < channels; c++) {
        size_t o = c;

        /* Load the coefficient pair into the channel state. */
        coeffindex = state->block.data[o];
        if (coeffindex > ddata->coeffcount) {
            return SDL_SetError("Invalid MS ADPCM coefficient index in block header");
        }
        cstate[c].coeff1 = ddata->coeff[coeffindex * 2];
        cstate[c].coeff2 = ddata->coeff[coeffindex * 2 + 1];

        /* Initial delta value. */
        o = channels + c * 2;
        cstate[c].delta = state->block.data[o] | ((Uint16)state->block.data[o + 1] << 8);

        /* Load the samples from the header. Interestingly, the sample later in
         * the output stream comes first.
         */
        o = channels * 3 + c * 2;
        sample = state->block.data[o] | ((Sint32)state->block.data[o + 1] << 8);
        if (sample >= 0x8000) {
            sample -= 0x10000;
        }
        state->output.data[state->output.pos + channels] = (Sint16)sample;

        o = channels * 5 + c * 2;
        sample = state->block.data[o] | ((Sint32)state->block.data[o + 1] << 8);
        if (sample >= 0x8000) {
            sample -= 0x10000;
        }
        state->output.data[state->output.pos] = (Sint16)sample;

        state->output.pos++;
    }

    state->block.pos += state->blockheadersize;

    /* Skip second sample frame that came from the header. */
    state->output.pos += state->channels;

    /* Header provided two sample frames. */
    state->framesleft -= 2;

    return 0;
}

/**
 * Decodes the data of the MS ADPCM block.
 *
 * Decoding will stop if a block is too short, returning with none or partially
 * decoded data. The partial data will always contain full sample frames (same
 * sample count for each channel). Incomplete sample frames are discarded.
 *
 * @param state the state containing the block to process
 *
 * @return 0 on success, error code on error.
 */
static int MS_ADPCM_DecodeBlockData(ADPCM_DecoderState *state) {
    Uint16 nybble = 0;
    Sint16 sample1, sample2;
    const Uint32 channels = state->channels;
    Uint32 c;
    MS_ADPCM_ChannelState *cstate = (MS_ADPCM_ChannelState *)state->cstate;

    size_t blockpos = state->block.pos;
    size_t blocksize = state->block.size;

    size_t outpos = state->output.pos;

    Sint64 blockframesleft = state->samplesperblock - 2;
    if (blockframesleft > state->framesleft) {
        blockframesleft = state->framesleft;
    }

    while (blockframesleft > 0) {
        for (c = 0; c < channels; c++) {
            if (nybble & 0x4000) {
                nybble <<= 4;
            } else if (blockpos < blocksize) {
                nybble = state->block.data[blockpos++] | 0x4000;
            } else {
                /* Out of input data. Drop the incomplete frame and return. */
                state->output.pos = outpos - c;
                return -1;
            }

            /* Load previous samples which may come from the block header. */
            sample1 = state->output.data[outpos - channels];
            sample2 = state->output.data[outpos - channels * 2];

            sample1 = MS_ADPCM_ProcessNibble(cstate + c, sample1, sample2, (nybble >> 4) & 0x0f);
            state->output.data[outpos++] = sample1;
        }

        state->framesleft--;
        blockframesleft--;
    }

    state->output.pos = outpos;

    return 0;
}

/**
 * Initializes a ADPCM_DecoderState and assigns it to file
 *
 * The decoder state will allow future calls to {@link MS_ADPCM_StepDecode}
 * to succeed. Note that this function allocates memory, which will be cleaned
 * up with a call to {@link MS_ADPCM_EndDecode}.
 *
 * @param file          The WAV file struct
 * @param datalength    The amount of data encoded
 *
 * @return 0 on success, error code on error.
 */
static int MS_ADPCM_Init(WaveFile *file, size_t datalength) {
    WaveFormat *format = &file->format;
    WaveChunk *chunk = &file->chunk;
    const size_t blockheadersize = (size_t)format->channels * 7;
    const size_t blockdatasize = (size_t)format->blockalign - blockheadersize;
    const size_t blockframebitsize = (size_t)format->bitspersample * format->channels;
    const size_t blockdatasamples = (blockdatasize * 8) / blockframebitsize;
    const Sint16 presetcoeffs[14] = {256, 0, 512, -256, 0, 0, 192, 64, 240, 0, 460, -208, 392, -232};
    size_t i, coeffcount;
    MS_ADPCM_CoeffData *coeffdata;
    ADPCM_DecoderState* decoder;

    /* Sanity checks. */

    /* While it's clear how IMA ADPCM handles more than two channels, the nibble
     * order of MS ADPCM makes it awkward. The Standards Update does not talk
     * about supporting more than stereo anyway.
     */
    if (format->channels > 2) {
        return SDL_SetError("Invalid number of channels");
    }

    if (format->bitspersample != 4) {
        return SDL_SetError("Invalid MS ADPCM bits per sample of %u", (unsigned int)format->bitspersample);
    }

    /* The block size must be big enough to contain the block header. */
    if (format->blockalign < blockheadersize) {
        return SDL_SetError("Invalid MS ADPCM block size (nBlockAlign)");
    }

    if (format->formattag == EXTENSIBLE_CODE) {
        /* Does have a GUID (like all format tags), but there's no specification
         * for how the data is packed into the extensible header. Making
         * assumptions here could lead to new formats nobody wants to support.
         */
        return SDL_SetError("MS ADPCM with the extensible header is not supported");
    }

    /*
     * There are wSamplesPerBlock, wNumCoef, and at least 7 coefficient pairs in
     * the extended part of the header.
     */
    if (chunk->size < 22) {
        return SDL_SetError("Could not read MS ADPCM format header");
    }

    format->samplesperblock = chunk->data[18] | ((Uint16)chunk->data[19] << 8);
    /* Number of coefficient pairs. A pair has two 16-bit integers. */
    coeffcount = chunk->data[20] | ((size_t)chunk->data[21] << 8);
    /* bPredictor, the integer offset into the coefficients array, is only
     * 8 bits. It can only address the first 256 coefficients. Let's limit
     * the count number here.
     */
    if (coeffcount > 256) {
        coeffcount = 256;
    }

    if (chunk->size < 22 + coeffcount * 4) {
        return SDL_SetError("Could not read custom coefficients in MS ADPCM format header");
    } else if (format->extsize < 4 + coeffcount * 4) {
        return SDL_SetError("Invalid MS ADPCM format header (too small)");
    } else if (coeffcount < 7) {
        return SDL_SetError("Missing required coefficients in MS ADPCM format header");
    }

    coeffdata = (MS_ADPCM_CoeffData *)ATK_malloc(sizeof(MS_ADPCM_CoeffData) + coeffcount * 4);
    decoder  = (ADPCM_DecoderState*)ATK_malloc(sizeof(ADPCM_DecoderState));
    SDL_zero(*decoder);

    decoder->mscoeff = coeffdata;
    file->decoderdata = decoder; /* Freed in cleanup. */

    if (coeffdata == NULL) {
        return SDL_OutOfMemory();
    }
    coeffdata->coeff = &coeffdata->aligndummy;
    coeffdata->coeffcount = (Uint16)coeffcount;

    /* Copy the 16-bit pairs. */
    for (i = 0; i < coeffcount * 2; i++) {
        Sint32 c = chunk->data[22 + i * 2] | ((Sint32)chunk->data[23 + i * 2] << 8);
        if (c >= 0x8000) {
            c -= 0x10000;
        }
        if (i < 14 && c != presetcoeffs[i]) {
            return SDL_SetError("Wrong preset coefficients in MS ADPCM format header");
        }
        coeffdata->coeff[i] = (Sint16)c;
    }

    /* Technically, wSamplesPerBlock is required, but we have all the
     * information in the other fields to calculate it, if it's zero.
     */
    if (format->samplesperblock == 0) {
        /* Let's be nice to the encoders that didn't know how to fill this.
         * The Standards Update calculates it this way:
         *
         *   x = Block size (in bits) minus header size (in bits)
         *   y = Bit depth multiplied by channel count
         *   z = Number of samples per channel in block header
         *   wSamplesPerBlock = x / y + z
         */
        format->samplesperblock = (Uint32)blockdatasamples + 2;
    }

    /* nBlockAlign can be in conflict with wSamplesPerBlock. For example, if
     * the number of samples doesn't fit into the block. The Standards Update
     * also describes wSamplesPerBlock with a formula that makes it necessary to
     * always fill the block with the maximum amount of samples, but this is not
     * enforced here as there are no compatibility issues.
     * A truncated block header with just one sample is not supported.
     */
    if (format->samplesperblock == 1 || blockdatasamples < format->samplesperblock - 2) {
        return SDL_SetError("Invalid number of samples per MS ADPCM block (wSamplesPerBlock)");
    }

    if (MS_ADPCM_CalculateSampleFrames(file, datalength) < 0) {
        return -1;
    }

    return 0;
}

/**
 * Starts the decoding process on the DATA block
 *
 * This function has been factored out out of the original MS_ADPCM_Decode
 * to allow us to stream the results. Instead of working on the active
 * chunk, we have moved the data chunk to a WaveBlock.
 *
 * @param file          The WAV file struct
 *
 * @return 0 on success, error code on error.
 */
static int MS_ADPCM_BeginDecode(WaveFile *file) {
    size_t outputsize;

    WaveBlock *block = &file->data;
    ADPCM_DecoderState* state = (ADPCM_DecoderState*)file->decoderdata;

    MS_ADPCM_ChannelState* cstate = ATK_malloc(sizeof(MS_ADPCM_ChannelState)*2);
    memset(cstate, 0, sizeof(MS_ADPCM_ChannelState)*2);
    state->cstate = cstate;

    /* Nothing to decode, nothing to return. */
    if (file->sampleframes == 0) {
        return 0;
    }

    state->blocksize = file->format.blockalign;
    state->channels = file->format.channels;
    state->blockheadersize = (size_t)state->channels * 7;
    state->samplesperblock = file->format.samplesperblock;
    state->framesize = state->channels * sizeof(Sint16);
    state->framestotal = file->sampleframes;
    state->framesleft = state->framestotal;

    if (block->data != NULL) {
        ATK_free(block->data);
    }
    block->data = (Uint8*)ATK_malloc(state->blocksize);
    if (block->data == NULL) {
        return SDL_OutOfMemory();
    }
    block->pagesize = state->blocksize;

    state->input.data = block->data;
    state->input.size = block->pagesize;
    state->input.pos = 0;

    /* The output size in bytes. We process one page at a time */
    outputsize = (size_t)(state->samplesperblock * state->framesize);

    state->output.pos = 0;
    state->output.size = outputsize / sizeof(Sint16);
    state->output.data = (Sint16 *)SDL_calloc(1, outputsize);
    if (state->output.data == NULL) {
        return SDL_OutOfMemory();
    }

    return (int)Block_Reset(file->source, block);
}

/**
 * Reads a single page of data from the given file.
 *
 * The buffer should be able to store the output of a page. This is
 * defined by the output samplesize, the number of channels and the
 * sizeof(Sint16). If the read fails, this function returns -1.
 *
 * @param file      The WAV file struct
 * @param buffer    The buffer to store the decoded data
 *
 * @return the number of bytes read (or -1 on error)
 */
static int MS_ADPCM_StepDecode(WaveFile *file, Uint8 *buffer) {
    int result;
    size_t outputsize ;

    /* Decode block by block. A truncated block will stop the decoding. */
    ADPCM_DecoderState* state = (ADPCM_DecoderState*)file->decoderdata;

    // Read next page into the input state
    Sint64 amt = Block_Read(file->source, &(file->data));
    if (amt <= 0) {
        return (int)amt;
    }
    state->input.pos = 0;

    // Normal decode
    size_t bytesleft = state->input.size - state->input.pos;
    outputsize = (size_t)(state->samplesperblock * state->framesize);
    state->output.pos = 0;
    while (state->framesleft > 0 && bytesleft >= state->blockheadersize) {
        state->block.data = state->input.data + state->input.pos;
        state->block.size = bytesleft < state->blocksize ? bytesleft : state->blocksize;
        state->block.pos = 0;

        /* Initialize decoder with the values from the block header. */
        result = MS_ADPCM_DecodeBlockHeader(state);
        if (result == -1) {
            return -1;
        }

        /* Decode the block data. It stores the samples directly in the output. */
        result = MS_ADPCM_DecodeBlockData(state);
        if (result == -1) {
            /* Unexpected end. Stop decoding and return partial data if necessary. */
            if (file->trunchint == TruncVeryStrict || file->trunchint == TruncStrict) {
                return SDL_SetError("Truncated data chunk");
            } else if (file->trunchint != TruncDropFrame) {
                state->output.pos = 0;
            }
            outputsize = state->output.pos * sizeof(Sint16); /* Can't overflow, is always smaller. */
        } else {
            state->input.pos += state->block.size;
            bytesleft = state->input.size - state->input.pos;
        }
    }

    // Copy over the output
    memcpy(buffer, state->output.data, outputsize);
    return (Sint32)outputsize;
}

/**
 * Deallocates the audio block buffer and ADPCM_DecoderState
 *
 * @param file          The WAV file struct
 *
 * @return 0 on success, error code on error.
 */
static int MS_ADPCM_EndDecode(WaveFile *file) {
    WaveBlock *block = &file->data;
    if (block->data != NULL) {
        ATK_free(block->data);
        block->data = NULL;
    }

    ADPCM_DecoderState* state = (ADPCM_DecoderState*)file->decoderdata;
    if (state == NULL) {
        return 0;
    }

    Sint16* data = state->output.data;
    if (data != NULL) {
        ATK_free(data);
        state->output.data = NULL;
    }

    MS_ADPCM_CoeffData* mscoeff = state->mscoeff;
    if (mscoeff != NULL) {
        ATK_free(mscoeff);
        state->mscoeff = NULL;
    }

    MS_ADPCM_ChannelState* cstate = state->cstate;
    if (cstate != NULL) {
        ATK_free(cstate);
        state->cstate = NULL;
    }

    ATK_free(state);
    file->decoderdata = NULL;
    return 0;
}

#pragma mark IMA ADPCM WAV
/**
 * Returns the number of sample frames needed for an IMA ADPCM file
 *
 * @param file          The WAV file to check
 * @param datalength    The amount of data encoded
 *
 * @return the number of sample frames needed for an IMA ADPCM file
 */
static int IMA_ADPCM_CalculateSampleFrames(WaveFile *file, size_t datalength) {
    WaveFormat *format = &file->format;
    const size_t blockheadersize = (size_t)format->channels * 4;
    const size_t subblockframesize = (size_t)format->channels * 4;
    const size_t availableblocks = datalength / format->blockalign;
    const size_t trailingdata = datalength % format->blockalign;

    if (file->trunchint == TruncVeryStrict || file->trunchint == TruncStrict) {
        /* The size of the data chunk must be a multiple of the block size. */
        if (datalength < blockheadersize || trailingdata > 0) {
            return SDL_SetError("Truncated IMA ADPCM block");
        }
    }

    /* Calculate number of sample frames that will be decoded. */
    file->sampleframes = (Uint64)availableblocks * format->samplesperblock;
    if (trailingdata > 0) {
        /* The last block is truncated. Check if we can get any samples out of it. */
        if (file->trunchint == TruncDropFrame && trailingdata > blockheadersize - 2) {
            /* The sample frame in the header of the truncated block is present.
             * Drop incomplete sample frames.
             */
            size_t trailingsamples = 1;

            if (trailingdata > blockheadersize) {
                /* More data following after the header. */
                const size_t trailingblockdata = trailingdata - blockheadersize;
                const size_t trailingsubblockdata = trailingblockdata % subblockframesize;
                trailingsamples += (trailingblockdata / subblockframesize) * 8;
                /* Due to the interleaved sub-blocks, the last 4 bytes determine
                 * how many samples of the truncated sub-block are lost.
                 */
                if (trailingsubblockdata > subblockframesize - 4) {
                    trailingsamples += (trailingsubblockdata % 4) * 2;
                }
            }

            if (trailingsamples > format->samplesperblock) {
                trailingsamples = format->samplesperblock;
            }
            file->sampleframes += trailingsamples;
        }
    }

    file->sampleframes = WaveAdjustToFactValue(file, file->sampleframes);
    if (file->sampleframes < 0) {
        return -1;
    }

    return 0;
}

/**
 * Returns a single sample interpolated from a previous value
 *
 * @param state         The decoder state
 * @param lastsample    The sample to interpolate
 * @param nybble        The next byte to process
 *
 * @return a single sample interpolated from a previous value
 */
static Sint16 IMA_ADPCM_ProcessNibble(Sint8 *cindex, Sint16 lastsample, Uint8 nybble) {
    const Sint32 max_audioval = 32767;
    const Sint32 min_audioval = -32768;
    const Sint8 index_table_4b[16] = {
        -1, -1, -1, -1,
        2, 4, 6, 8,
        -1, -1, -1, -1,
        2, 4, 6, 8
    };
    const Uint16 step_table[89] = {
        7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31,
        34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130,
        143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408,
        449, 494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282,
        1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327,
        3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630,
        9493, 10442, 11487, 12635, 13899, 15289, 16818, 18500, 20350,
        22385, 24623, 27086, 29794, 32767
    };
    Uint32 step;
    Sint32 sample, delta;
    Sint8 index = *cindex;

    /* Clamp index into valid range. */
    if (index > 88) {
        index = 88;
    } else if (index < 0) {
        index = 0;
    }

    /* explicit cast to avoid gcc warning about using 'char' as array index */
    step = step_table[(size_t)index];

    /* Update index value */
    *cindex = index + index_table_4b[nybble];

    /* This calculation uses shifts and additions because multiplications were
     * much slower back then. Sadly, this can't just be replaced with an actual
     * multiplication now as the old algorithm drops some bits. The closest
     * approximation I could find is something like this:
     * (nybble & 0x8 ? -1 : 1) * ((nybble & 0x7) * step / 4 + step / 8)
     */
    delta = step >> 3;
    if (nybble & 0x04)
        delta += step;
    if (nybble & 0x02)
        delta += step >> 1;
    if (nybble & 0x01)
        delta += step >> 2;
    if (nybble & 0x08)
        delta = -delta;

    sample = lastsample + delta;

    /* Clamp output sample */
    if (sample > max_audioval) {
        sample = max_audioval;
    } else if (sample < min_audioval) {
        sample = min_audioval;
    }

    return (Sint16)sample;
}

/**
 * Decodes the header of an IMA ADPCM block, updating state
 *
 * @param state the state containing the block to process
 *
 * @return 0 on success, error code on error.
 */
static int IMA_ADPCM_DecodeBlockHeader(ADPCM_DecoderState *state) {
    Sint16 step;
    Uint32 c;
    Uint8 *cstate = (Uint8 *) state->cstate;

    for (c = 0; c < state->channels; c++) {
        size_t o = state->block.pos + c * 4;

        /* Extract the sample from the header. */
        Sint32 sample = state->block.data[o] | ((Sint32)state->block.data[o + 1] << 8);
        if (sample >= 0x8000) {
            sample -= 0x10000;
        }
        state->output.data[state->output.pos++] = (Sint16)sample;

        /* Channel step index. */
        step = (Sint16)state->block.data[o + 2];
        cstate[c] = (Sint8)(step > 0x80 ? step - 0x100 : step);

        /* Reserved byte in block header, should be 0. */
        if (state->block.data[o + 3] != 0) {
            /* Uh oh, corrupt data?  Buggy code? */ ;
        }
    }

    state->block.pos += state->blockheadersize;

    /* Header provided one sample frame. */
    state->framesleft--;

    return 0;
}

/**
 * Decodes the data of the IMA ADPCM block.
 *
 * Decoding will stop if a block is too short, returning with none or partially
 * decoded data. The partial data will always contain full sample frames (same
 * sample count for each channel). Incomplete sample frames are discarded.
 *
 * @param state the state containing the block to process
 *
 * @return 0 on success, error code on error.
 */
static int IMA_ADPCM_DecodeBlockData(ADPCM_DecoderState *state) {
    size_t i;
    int retval = 0;
    const Uint32 channels = state->channels;
    const size_t subblockframesize = channels * 4;
    Uint64 bytesrequired;
    Uint32 c;

    size_t blockpos = state->block.pos;
    size_t blocksize = state->block.size;
    size_t blockleft = blocksize - blockpos;

    size_t outpos = state->output.pos;

    Sint64 blockframesleft = state->samplesperblock - 1;
    if (blockframesleft > state->framesleft) {
        blockframesleft = state->framesleft;
    }

    bytesrequired = (blockframesleft + 7) / 8 * subblockframesize;
    if (blockleft < bytesrequired) {
        /* Data truncated. Calculate how many samples we can get out if it. */
        const size_t guaranteedframes = blockleft / subblockframesize;
        const size_t remainingbytes = blockleft % subblockframesize;
        blockframesleft = guaranteedframes;
        if (remainingbytes > subblockframesize - 4) {
            blockframesleft += (remainingbytes % 4) * 2;
        }
        /* Signal the truncation. */
        retval = -1;
    }

    /* Each channel has their nibbles packed into 32-bit blocks. These blocks
     * are interleaved and make up the data part of the ADPCM block. This loop
     * decodes the samples as they come from the input data and puts them at
     * the appropriate places in the output data.
     */
    while (blockframesleft > 0) {
        const size_t subblocksamples = blockframesleft < 8 ? (size_t)blockframesleft : 8;

        for (c = 0; c < channels; c++) {
            Uint8 nybble = 0;
            /* Load previous sample which may come from the block header. */
            Sint16 sample = state->output.data[outpos + c - channels];

            for (i = 0; i < subblocksamples; i++) {
                if (i & 1) {
                    nybble >>= 4;
                } else {
                    nybble = state->block.data[blockpos++];
                }

                sample = IMA_ADPCM_ProcessNibble((Sint8 *)state->cstate + c, sample, nybble & 0x0f);
                state->output.data[outpos + c + i * channels] = sample;
            }
        }

        outpos += channels * subblocksamples;
        state->framesleft -= subblocksamples;
        blockframesleft -= subblocksamples;
    }

    state->block.pos = blockpos;
    state->output.pos = outpos;

    return retval;
}

/**
 * Initializes a ADPCM_DecoderState and assigns it to file
 *
 * The decoder state will allow future calls to {@link IMA_ADPCM_StepDecode}
 * to succeed. Note that this function allocates memory, which will be cleaned
 * up with a call to {@link IMA_ADPCM_EndDecode}.
 *
 * @param file          The WAV file struct
 * @param datalength    The amount of data encoded
 *
 * @return 0 on success, error code on error.
 */
static int IMA_ADPCM_Init(WaveFile *file, size_t datalength) {
    WaveFormat *format = &file->format;
    WaveChunk *chunk = &file->chunk;
    const size_t blockheadersize = (size_t)format->channels * 4;
    const size_t blockdatasize = (size_t)format->blockalign - blockheadersize;
    const size_t blockframebitsize = (size_t)format->bitspersample * format->channels;
    const size_t blockdatasamples = (blockdatasize * 8) / blockframebitsize;

    /* Sanity checks. */

    /* IMA ADPCM can also have 3-bit samples, but it's not supported by SDL at this time. */
    if (format->bitspersample == 3) {
        return SDL_SetError("3-bit IMA ADPCM currently not supported");
    } else if (format->bitspersample != 4) {
        return SDL_SetError("Invalid IMA ADPCM bits per sample of %u", (unsigned int)format->bitspersample);
    }

    /* The block size is required to be a multiple of 4 and it must be able to
     * hold a block header.
     */
    if (format->blockalign < blockheadersize || format->blockalign % 4) {
        return SDL_SetError("Invalid IMA ADPCM block size (nBlockAlign)");
    }

    if (format->formattag == EXTENSIBLE_CODE) {
        /* There's no specification for this, but it's basically the same
         * format because the extensible header has wSampePerBlocks too.
         */
    } else  {
        /* The Standards Update says there 'should' be 2 bytes for wSamplesPerBlock. */
        if (chunk->size >= 20 && format->extsize >= 2) {
            format->samplesperblock = chunk->data[18] | ((Uint16)chunk->data[19] << 8);
        }
    }

    if (format->samplesperblock == 0) {
        /* Field zero? No problem. We just assume the encoder packed the block.
         * The specification calculates it this way:
         *
         *   x = Block size (in bits) minus header size (in bits)
         *   y = Bit depth multiplied by channel count
         *   z = Number of samples per channel in header
         *   wSamplesPerBlock = x / y + z
         */
        format->samplesperblock = (Uint32)blockdatasamples + 1;
    }

    /* nBlockAlign can be in conflict with wSamplesPerBlock. For example, if
     * the number of samples doesn't fit into the block. The Standards Update
     * also describes wSamplesPerBlock with a formula that makes it necessary
     * to always fill the block with the maximum amount of samples, but this is
     * not enforced here as there are no compatibility issues.
     */
    if (blockdatasamples < format->samplesperblock - 1) {
        return SDL_SetError("Invalid number of samples per IMA ADPCM block (wSamplesPerBlock)");
    }

    if (IMA_ADPCM_CalculateSampleFrames(file, datalength) < 0) {
        return -1;
    }

    // Attach the decoder
    ADPCM_DecoderState* decoder  = (ADPCM_DecoderState*)ATK_malloc(sizeof(ADPCM_DecoderState));
    SDL_zero(*decoder);
    file->decoderdata = decoder; /* Freed in cleanup. */

    return 0;
}

/**
 * Starts the decoding process on the DATA block
 *
 * This function has been factored out out of the original MS_ADPCM_Decode
 * to allow us to stream the results. Instead of working on the active
 * chunk, we have moved the data chunk to a WaveBlock.
 *
 * @param file          The WAV file struct
 *
 * @return 0 on success, error code on error.
 */
static int IMA_ADPCM_BeginDecode(WaveFile *file) {
    size_t outputsize;
    WaveBlock *block = &file->data;
    ADPCM_DecoderState* state = (ADPCM_DecoderState*)file->decoderdata;
    Sint8 *cstate;

    /* Nothing to decode, nothing to return. */
    if (file->sampleframes == 0) {
        return 0;
    }

    state->channels = file->format.channels;
    state->blocksize = file->format.blockalign;
    state->blockheadersize = (size_t)state->channels * 4;
    state->samplesperblock = file->format.samplesperblock;
    state->framesize = state->channels * sizeof(Sint16);
    state->framestotal = file->sampleframes;
    state->framesleft = state->framestotal;

    if (block->data != NULL) {
        ATK_free(block->data);
    }
    block->data = (Uint8*)ATK_malloc(state->blocksize);
    if (block->data == NULL) {
        return SDL_OutOfMemory();
    }
    block->pagesize = state->blocksize;

    state->input.data = block->data;
    state->input.size = block->pagesize;
    state->input.pos = 0;

    /* The output size in bytes. We process one page at a time */
    outputsize = (size_t)(state->samplesperblock * state->framesize);

    state->output.pos = 0;
    state->output.size = outputsize / sizeof(Sint16);
    state->output.data = (Sint16 *)SDL_calloc(1, outputsize);
    if (state->output.data == NULL) {
        return SDL_OutOfMemory();
    }

    cstate = (Sint8 *)SDL_calloc(state->channels, sizeof(Sint8));
    if (cstate == NULL) {
        ATK_free(state->output.data);
        state->output.data = NULL;
        return SDL_OutOfMemory();
    }
    state->cstate = cstate;

    return (int)Block_Reset(file->source, block);
}

/**
 * Reads a single page of data from the given file.
 *
 * The buffer should be able to store the output of a page. This is
 * defined by the output samplesize, the number of channels and the
 * sizeof(Sint16). If the read fails, this function returns -1.
 *
 * @param file      The WAV file struct
 * @param buffer    The buffer to store the decoded data
 *
 * @return the number of bytes read (or -1 on error)
 */
static int IMA_ADPCM_StepDecode(WaveFile *file, Uint8 *buffer) {
    int result;
    size_t outputsize ;

    /* Decode block by block. A truncated block will stop the decoding. */
    ADPCM_DecoderState* state = (ADPCM_DecoderState*)file->decoderdata;

    // Read next page into the input state
    Sint64 amt = Block_Read(file->source, &(file->data));
    if (amt <= 0) {
        return (int)amt;
    }
    state->input.pos = 0;

    // Normal decode
    size_t bytesleft = state->input.size - state->input.pos;
    outputsize = (size_t)(state->samplesperblock * state->framesize);
    state->output.pos = 0;
    while (state->framesleft > 0 && bytesleft >= state->blockheadersize) {
       state->block.data = state->input.data + state->input.pos;
       state->block.size = bytesleft < state->blocksize ? bytesleft : state->blocksize;
       state->block.pos = 0;

       /* Initialize decoder with the values from the block header. */
       result = IMA_ADPCM_DecodeBlockHeader(state);
       if (result == 0) {
           /* Decode the block data. It stores the samples directly in the output. */
           result = IMA_ADPCM_DecodeBlockData(state);
       }

       if (result == -1) {
           /* Unexpected end. Stop decoding and return partial data if necessary. */
           if (file->trunchint == TruncVeryStrict || file->trunchint == TruncStrict) {
               return SDL_SetError("Truncated data chunk");
           } else if (file->trunchint != TruncDropFrame) {
               state->output.pos = 0;
           }
           outputsize = state->output.pos * sizeof(Sint16); /* Can't overflow, is always smaller. */
           break;
       }

       state->input.pos += state->block.size;
       bytesleft = state->input.size - state->input.pos;
   }

    // Copy over the output
    memcpy(buffer, state->output.data, outputsize);
    return (Sint32)outputsize;
}

/**
 * Deallocates the audio block buffer and ADPCM_DecoderState
 *
 * @param file          The WAV file struct
 *
 * @return 0 on success, error code on error.
 */
static int IMA_ADPCM_EndDecode(WaveFile *file) {
    WaveBlock *block = &file->data;
    Uint8* buff = block->data;
    if (buff != NULL) {
        ATK_free(buff);
        block->data = NULL;
    }

    ADPCM_DecoderState* state = (ADPCM_DecoderState*)file->decoderdata;
    if (state == NULL) {
        return 0;
    }

    Sint16* data = state->output.data;
    if (data != NULL) {
        ATK_free(data);
        state->output.data = NULL;
    }

    MS_ADPCM_ChannelState* cstate = state->cstate;
    if (cstate != NULL) {
        ATK_free(cstate);
        state->cstate = NULL;
    }

    ATK_free(state);
    file->decoderdata = NULL;
    return 0;
}

#pragma mark A/MU-LAW WAV
/**
 * Initializes the file settings for A/MU-LAW files
 *
 * This function does not allocate any memory. We delay allocating memory
 * until we begin decoding
 *
 * @param file          The WAV file struct
 * @param datalength    The amount of data encoded
 *
 * @return 0 on success, error code on error.
 */
static int LAW_Init(WaveFile *file, size_t datalength) {
   WaveFormat *format = &file->format;

   /* Standards Update requires this to be 8. */
   if (format->bitspersample != 8) {
       return SDL_SetError("Invalid companded bits per sample of %u", (unsigned int)format->bitspersample);
   }

   /* Not going to bother with weird padding. */
   if (format->blockalign != format->channels) {
       return SDL_SetError("Unsupported block alignment");
   }

   if ((file->trunchint == TruncVeryStrict || file->trunchint == TruncStrict)) {
       if (format->blockalign > 1 && datalength % format->blockalign) {
           return SDL_SetError("Truncated data chunk in WAVE file");
       }
   }

   file->sampleframes = WaveAdjustToFactValue(file, datalength / format->blockalign);
   if (file->sampleframes < 0) {
       return -1;
   }

   return 0;
}

/**
 * Starts the decoding process on the DATA block
 *
 * This function has been factored out out of the original LAW_Decode to
 * allow us to stream the results. Instead of working on the active chunk,
 * we have moved the data chunk to a WaveBlock. In addition, the local look-up
 * tables are copied to file->decoderdata. This memory will be deallocated
 * by {@link LAW_EndDecode}.
 *
 * @param file          The WAV file struct
 *
 * @return 0 on success, error code on error.
 */
static int LAW_BeginDecode(WaveFile *file) {
#ifdef SDL_WAVE_LAW_LUT
    const Sint16 alaw_lut[256] = {
        -5504, -5248, -6016, -5760, -4480, -4224, -4992, -4736, -7552, -7296, -8064, -7808, -6528, -6272, -7040, -6784, -2752,
        -2624, -3008, -2880, -2240, -2112, -2496, -2368, -3776, -3648, -4032, -3904, -3264, -3136, -3520, -3392, -22016,
        -20992, -24064, -23040, -17920, -16896, -19968, -18944, -30208, -29184, -32256, -31232, -26112, -25088, -28160, -27136, -11008,
        -10496, -12032, -11520, -8960, -8448, -9984, -9472, -15104, -14592, -16128, -15616, -13056, -12544, -14080, -13568, -344,
        -328, -376, -360, -280, -264, -312, -296, -472, -456, -504, -488, -408, -392, -440, -424, -88,
        -72, -120, -104, -24, -8, -56, -40, -216, -200, -248, -232, -152, -136, -184, -168, -1376,
        -1312, -1504, -1440, -1120, -1056, -1248, -1184, -1888, -1824, -2016, -1952, -1632, -1568, -1760, -1696, -688,
        -656, -752, -720, -560, -528, -624, -592, -944, -912, -1008, -976, -816, -784, -880, -848, 5504,
        5248, 6016, 5760, 4480, 4224, 4992, 4736, 7552, 7296, 8064, 7808, 6528, 6272, 7040, 6784, 2752,
        2624, 3008, 2880, 2240, 2112, 2496, 2368, 3776, 3648, 4032, 3904, 3264, 3136, 3520, 3392, 22016,
        20992, 24064, 23040, 17920, 16896, 19968, 18944, 30208, 29184, 32256, 31232, 26112, 25088, 28160, 27136, 11008,
        10496, 12032, 11520, 8960, 8448, 9984, 9472, 15104, 14592, 16128, 15616, 13056, 12544, 14080, 13568, 344,
        328, 376, 360, 280, 264, 312, 296, 472, 456, 504, 488, 408, 392, 440, 424, 88,
        72, 120, 104, 24, 8, 56, 40, 216, 200, 248, 232, 152, 136, 184, 168, 1376,
        1312, 1504, 1440, 1120, 1056, 1248, 1184, 1888, 1824, 2016, 1952, 1632, 1568, 1760, 1696, 688,
        656, 752, 720, 560, 528, 624, 592, 944, 912, 1008, 976, 816, 784, 880, 848
    };
    const Sint16 mulaw_lut[256] = {
        -32124, -31100, -30076, -29052, -28028, -27004, -25980, -24956, -23932, -22908, -21884, -20860, -19836, -18812, -17788, -16764, -15996,
        -15484, -14972, -14460, -13948, -13436, -12924, -12412, -11900, -11388, -10876, -10364, -9852, -9340, -8828, -8316, -7932,
        -7676, -7420, -7164, -6908, -6652, -6396, -6140, -5884, -5628, -5372, -5116, -4860, -4604, -4348, -4092, -3900,
        -3772, -3644, -3516, -3388, -3260, -3132, -3004, -2876, -2748, -2620, -2492, -2364, -2236, -2108, -1980, -1884,
        -1820, -1756, -1692, -1628, -1564, -1500, -1436, -1372, -1308, -1244, -1180, -1116, -1052, -988, -924, -876,
        -844, -812, -780, -748, -716, -684, -652, -620, -588, -556, -524, -492, -460, -428, -396, -372,
        -356, -340, -324, -308, -292, -276, -260, -244, -228, -212, -196, -180, -164, -148, -132, -120,
        -112, -104, -96, -88, -80, -72, -64, -56, -48, -40, -32, -24, -16, -8, 0, 32124,
        31100, 30076, 29052, 28028, 27004, 25980, 24956, 23932, 22908, 21884, 20860, 19836, 18812, 17788, 16764, 15996,
        15484, 14972, 14460, 13948, 13436, 12924, 12412, 11900, 11388, 10876, 10364, 9852, 9340, 8828, 8316, 7932,
        7676, 7420, 7164, 6908, 6652, 6396, 6140, 5884, 5628, 5372, 5116, 4860, 4604, 4348, 4092, 3900,
        3772, 3644, 3516, 3388, 3260, 3132, 3004, 2876, 2748, 2620, 2492, 2364, 2236, 2108, 1980, 1884,
        1820, 1756, 1692, 1628, 1564, 1500, 1436, 1372, 1308, 1244, 1180, 1116, 1052, 988, 924, 876,
        844, 812, 780, 748, 716, 684, 652, 620, 588, 556, 524, 492, 460, 428, 396, 372,
        356, 340, 324, 308, 292, 276, 260, 244, 228, 212, 196, 180, 164, 148, 132, 120,
        112, 104, 96, 88, 80, 72, 64, 56, 48, 40, 32, 24, 16, 8, 0
    };
#endif

    WaveFormat *format = &file->format;
    WaveBlock *block = &file->data;
    size_t sample_count, expanded_len;
    //Uint8* src;

    /* Nothing to decode, nothing to return. */
    if (file->sampleframes == 0) {
        return 0;
    }

    sample_count = (size_t)WAV_PAGE_SIZE*format->channels;
    expanded_len = sample_count*sizeof(Sint16);
    if (block->data != NULL) {
        ATK_free(block->data);
    }
    block->data = (Uint8*)ATK_malloc(expanded_len);
    if (block->data == NULL) {
        return SDL_OutOfMemory();
    }
    block->pagesize = sample_count;

#ifdef SDL_WAVE_LAW_LUT
    Sint16* lut = ATK_malloc(sizeof(Sint16)*256);
    switch (file->format.encoding) {
    case ALAW_CODE:
        memcpy(lut, alaw_lut, sizeof(Sint16)*256);
        break;
    case MULAW_CODE:
        memcpy(lut, mulaw_lut, sizeof(Sint16)*256);
        break;
    }
    file->decoderdata = lut;
#endif

    return (int)Block_Reset(file->source, block);
}

/**
 * Reads a single page of data from the given file.
 *
 * The buffer should be able to store the output of a page. This is
 * defined by WAV_PAGE_SIZE, the number of channels and the sizeof(Sint16).
 * If the read fails, this function returns -1.
 *
 * @param file      The WAV file struct
 * @param buffer    The buffer to store the decoded data
 *
 * @return the number of bytes read (or -1 on error)
 */
static int LAW_StepDecode(WaveFile *file, Uint8 *buffer) {
    WaveBlock *block = &file->data;
    size_t i, sample_count, expanded_len;
    Uint8 *src;
    Sint16 *dst;
    // Read next page into the input state
    Sint64 amt = Block_Read(file->source, &(file->data));
    if (amt <= 0) {
        return (int)amt;
    }
    src = block->data;
    dst = (Sint16 *)src;

    sample_count = block->size;
    expanded_len = sample_count*sizeof(Sint16);

    /* Work backwards, since we're expanding in-place.
     * SDL_AudioSpec.format will inform the caller about the byte order.
     */
    i = sample_count;
#ifdef SDL_WAVE_LAW_LUT
    Sint16* lut = (Sint16*)file->decoderdata;
    while (i--) {
        dst[i] = lut[src[i]];
    }
#else
    switch (file->format.encoding) {
    case ALAW_CODE:
        while (i--) {
            Uint8 nibble = src[i];
            Uint8 exponent = (nibble & 0x7f) ^ 0x55;
            Sint16 mantissa = exponent & 0xf;

            exponent >>= 4;
            if (exponent > 0) {
                mantissa |= 0x10;
            }
            mantissa = (mantissa << 4) | 0x8;
            if (exponent > 1) {
                mantissa <<= exponent - 1;
            }

            dst[i] = nibble & 0x80 ? mantissa : -mantissa;
        }
        break;
    case MULAW_CODE:
        while (i--) {
            Uint8 nibble = ~src[i];
            Sint16 mantissa = nibble & 0xf;
            Uint8 exponent = (nibble >> 4) & 0x7;
            Sint16 step = 4 << (exponent + 1);

            mantissa = (0x80 << exponent) + step * mantissa + step / 2 - 132;

            dst[i] = nibble & 0x80 ? -mantissa : mantissa;
        }
        break;
#endif
    default:
        return SDL_SetError("Unknown companded encoding");
    }

    // Copy over the output
    memcpy(buffer, dst, expanded_len);
    return (Sint32)expanded_len;
}

/**
 * Deallocates the audio block buffer
 *
 * @param file          The WAV file struct
 *
 * @return 0 on success, error code on error.
 */
static int LAW_EndDecode(WaveFile *file) {
    WaveBlock *block = &file->data;
    if (block->data != NULL) {
        ATK_free(block->data);
        block->data = NULL;
    }

    if (file->decoderdata != NULL) {
        ATK_free(file->decoderdata);
        file->decoderdata = NULL;
    }
    return 0;
}

#pragma mark PCM WAV
/**
 * Initializes the file settings for PCM files
 *
 * This function does not allocate any memory. We delay allocating memory
 * until we begin decoding
 *
 * @param file          The WAV file struct
 * @param datalength    The amount of data encoded
 *
 * @return 0 on success, error code on error.
 */
static int PCM_Init(WaveFile *file, size_t datalength) {
    WaveFormat *format = &file->format;

    if (format->encoding == PCM_CODE) {
        switch (format->bitspersample) {
        case 8:
        case 16:
        case 24:
        case 32:
            /* These are supported. */
            break;
        default:
            return SDL_SetError("%u-bit PCM format not supported", (unsigned int)format->bitspersample);
        }
    } else if (format->encoding == IEEE_FLOAT_CODE) {
        if (format->bitspersample != 32) {
            return SDL_SetError("%u-bit IEEE floating-point format not supported", (unsigned int)format->bitspersample);
        }
    }

    /* Make sure we're a multiple of the blockalign, at least. */
    if ((format->channels * format->bitspersample) % (format->blockalign * 8)) {
        return SDL_SetError("Unsupported block alignment");
    }

    if ((file->trunchint == TruncVeryStrict || file->trunchint == TruncStrict)) {
        if (format->blockalign > 1 && datalength % format->blockalign) {
            return SDL_SetError("Truncated data chunk in WAVE file");
        }
    }

    file->sampleframes = WaveAdjustToFactValue(file, datalength / format->blockalign);
    if (file->sampleframes < 0) {
        return -1;
    }

    return 0;
}

/**
 * Starts the decoding process on the DATA block
 *
 * This function has been factored out out of the original PCM_Decode
 * to allow us to stream the results. Instead of working on the active
 * chunk, we have moved the data chunk to a WaveBlock.
 *
 * @param file          The WAV file struct
 *
 * @return 0 on success, error code on error.
 */
static int PCM_BeginDecode(WaveFile *file) {
    WaveFormat *format = &file->format;
    WaveBlock *block = &file->data;
    size_t samples, outputsize;

    /* Nothing to decode, nothing to return. */
    if (file->sampleframes == 0) {
        return 0;
    }

    if (format->encoding == PCM_CODE && format->bitspersample == 24) {
        samples = (size_t)(WAV_PAGE_SIZE)*format->channels;
        outputsize = samples*sizeof(Sint32);
        if (block->data != NULL) {
            ATK_free(block->data);
        }
        block->data = (Uint8*)ATK_malloc(outputsize);
        if (block->data == NULL) {
            return SDL_OutOfMemory();
        }
        block->pagesize = samples*24;
    } else {
        outputsize = (size_t)(WAV_PAGE_SIZE)*format->blockalign;
        if (block->data != NULL) {
            ATK_free(block->data);
        }
        block->data = (Uint8*)ATK_malloc(outputsize);
        if (block->data == NULL) {
            return SDL_OutOfMemory();
        }
        block->pagesize = outputsize;
    }

    return (int)Block_Reset(file->source, block);
}

/**
 * Reads a single page of data from the given file.
 *
 * The buffer should be able to store the output of a page. This is
 * defined by the output samplesize, the number of channels and the
 * sizeof(Sint16). If the read fails, this function returns -1.
 *
 * @param file      The WAV file struct
 * @param buffer    The buffer to store the decoded data
 *
 * @return the number of bytes read (or -1 on error)
 */
static int PCM_StepDecode(WaveFile *file, Uint8 *buffer) {
    WaveFormat *format = &file->format;
    WaveBlock *block = &file->data;
    size_t i, samples;
    //Uint8 *src;
    //Sint16 *dst;

    // Read next page into the input state
    if (Block_Read(file->source, &(file->data)) < 0) {
        return -1;
    }

    /* 24-bit samples get shifted to 32 bits. */
    if (format->encoding == PCM_CODE && format->bitspersample == 24) {
        samples = (size_t)(block->size/format->blockalign);
        Uint8 *ptr = block->data;

        /* work from end to start, since we're expanding in-place. */
        for (i = samples; i > 0; i--) {
            const size_t o = i - 1;
            uint8_t b[4];

            b[0] = 0;
            b[1] = ptr[o * 3];
            b[2] = ptr[o * 3 + 1];
            b[3] = ptr[o * 3 + 2];

            ptr[o * 4 + 0] = b[0];
            ptr[o * 4 + 1] = b[1];
            ptr[o * 4 + 2] = b[2];
            ptr[o * 4 + 3] = b[3];
        }
    }
    memcpy(buffer, block->data, block->size);
    return block->size;
}

/**
 * Deallocates the audio block buffer
 *
 * @param file          The WAV file struct
 *
 * @return 0 on success, error code on error.
 */
static int PCM_EndDecode(WaveFile *file) {
    WaveBlock *block = &file->data;
    if (block->data != NULL) {
        ATK_free(block->data);
        block->data = NULL;
    }

    return 0;
}

#pragma mark WAV Format Processing
/**
 * Returns the RIFF hint size specified by SDL
 *
 * @return the RIFF hint size specified by SDL
 */
static WaveRiffSizeHint WaveGetRiffSizeHint() {
    const char *hint = SDL_GetHint(SDL_HINT_WAVE_RIFF_CHUNK_SIZE);

    if (hint != NULL) {
        if (SDL_strcmp(hint, "force") == 0) {
            return RiffSizeForce;
        } else if (SDL_strcmp(hint, "ignore") == 0) {
            return RiffSizeIgnore;
        } else if (SDL_strcmp(hint, "ignorezero") == 0) {
            return RiffSizeIgnoreZero;
        } else if (SDL_strcmp(hint, "maximum") == 0) {
            return RiffSizeMaximum;
        }
    }

    return RiffSizeNoHint;
}

/**
 * Returns the truncation hint specified by SDL
 *
 * @return the truncation hint specified by SDL
 */
static WaveTruncationHint WaveGetTruncationHint() {
    const char *hint = SDL_GetHint(SDL_HINT_WAVE_TRUNCATION);

    if (hint != NULL) {
        if (SDL_strcmp(hint, "verystrict") == 0) {
            return TruncVeryStrict;
        } else if (SDL_strcmp(hint, "strict") == 0) {
            return TruncStrict;
        } else if (SDL_strcmp(hint, "dropframe") == 0) {
            return TruncDropFrame;
        } else if (SDL_strcmp(hint, "dropblock") == 0) {
            return TruncDropBlock;
        }
    }

    return TruncNoHint;
}

/**
 * Returns the fact chunk hint specified by SDL
 *
 * @return the fact chunk hint specified by SDL
 */
static WaveFactChunkHint WaveGetFactChunkHint() {
    const char *hint = SDL_GetHint(SDL_HINT_WAVE_FACT_CHUNK);

    if (hint != NULL) {
        if (SDL_strcmp(hint, "truncate") == 0) {
            return FactTruncate;
        } else if (SDL_strcmp(hint, "strict") == 0) {
            return FactStrict;
        } else if (SDL_strcmp(hint, "ignorezero") == 0) {
            return FactIgnoreZero;
        } else if (SDL_strcmp(hint, "ignore") == 0) {
            return FactIgnore;
        }
    }

    return FactNoHint;
}

/**
 * Frees the chunk data array
 *
 * @param chunk the Wave chunk
 */
static void WaveFreeChunkData(WaveChunk *chunk) {
    if (chunk->data != NULL) {
        ATK_free(chunk->data);
        chunk->data = NULL;
    }
    chunk->size = 0;
}

/**
 * Sets the position of the next chunk
 *
 * @param src   the audio source stream
 * @param chunk the Wave chunk
 *
 * @return 0 on success, error code on error.
 */
static int WaveNextChunk(SDL_RWops *src, WaveChunk *chunk) {
    Uint32 chunkheader[2];
    Sint64 nextposition = chunk->position + chunk->length;

    /* Data is no longer valid after this function returns. */
    WaveFreeChunkData(chunk);

    /* Error on overflows. */
    if (SDL_MAX_SINT64 - chunk->length < chunk->position || SDL_MAX_SINT64 - 8 < nextposition) {
        return -1;
    }

    /* RIFF chunks have a 2-byte alignment. Skip padding byte. */
    if (chunk->length & 1) {
        nextposition++;
    }

    if (SDL_RWseek(src, nextposition, RW_SEEK_SET) != nextposition) {
        /* Not sure how we ended up here. Just abort. */
        return -2;
    } else if (SDL_RWread(src, chunkheader, 4, 2) != 2) {
        return -1;
    }

    chunk->fourcc = SDL_SwapLE32(chunkheader[0]);
    chunk->length = SDL_SwapLE32(chunkheader[1]);
    chunk->position = nextposition + 8;

    return 0;
}

/**
 * Reads in data in the chunk buffer up to size length
 *
 * @param src       the audio source stream
 * @param chunk     the Wave chunk
 * @param length    the amount to read
 *
 * @return the number of bytes read (-1 on error)
 */
static int WaveReadPartialChunkData(SDL_RWops *src, WaveChunk *chunk, size_t length) {
    WaveFreeChunkData(chunk);

    if (length > chunk->length) {
        length = chunk->length;
    }

    if (length > 0) {
        chunk->data = (Uint8 *) ATK_malloc(length);
        if (chunk->data == NULL) {
            return SDL_OutOfMemory();
        }

        if (SDL_RWseek(src, chunk->position, RW_SEEK_SET) != chunk->position) {
            /* Not sure how we ended up here. Just abort. */
            return -2;
        }

        chunk->size = SDL_RWread(src, chunk->data, 1, length);
        if (chunk->size != length) {
            /* Expected to be handled by the caller. */
        }
    }

    return 0;
}

/**
 * Reads in chunk data into the buffer
 *
 * @param src       the audio source stream
 * @param chunk     the Wave chunk
 *
 * @return the number of bytes read (-1 on error)
 */
static int WaveReadChunkData(SDL_RWops *src, WaveChunk *chunk) {
    return WaveReadPartialChunkData(src, chunk, chunk->length);
}


/* Some of the GUIDs that are used by WAVEFORMATEXTENSIBLE. */
#define WAVE_FORMATTAG_GUID(tag) {(tag) & 0xff, (tag) >> 8, 0, 0, 0, 0, 16, 0, 128, 0, 0, 170, 0, 56, 155, 113}
static WaveExtensibleGUID extensible_guids[] = {
    {PCM_CODE,        WAVE_FORMATTAG_GUID(PCM_CODE)},
    {MS_ADPCM_CODE,   WAVE_FORMATTAG_GUID(MS_ADPCM_CODE)},
    {IEEE_FLOAT_CODE, WAVE_FORMATTAG_GUID(IEEE_FLOAT_CODE)},
    {ALAW_CODE,       WAVE_FORMATTAG_GUID(ALAW_CODE)},
    {MULAW_CODE,      WAVE_FORMATTAG_GUID(MULAW_CODE)},
    {IMA_ADPCM_CODE,  WAVE_FORMATTAG_GUID(IMA_ADPCM_CODE)}
};

/**
 * Returns the GUID for the format
 *
 * @param format    the Wave format
 *
 * @return the GUID for the format
 */
static Uint16 WaveGetFormatGUIDEncoding(WaveFormat *format) {
    size_t i;
    for (i = 0; i < SDL_arraysize(extensible_guids); i++) {
        if (SDL_memcmp(format->subformat, extensible_guids[i].guid, 16) == 0) {
            return extensible_guids[i].encoding;
        }
    }
    return UNKNOWN_CODE;
}

/**
 * Reads the format into the WaveFile struct
 *
 * @param file  The WaveFile struct
 *
 * @return 0 on success, -1 on error
 */
static int WaveReadFormat(WaveFile *file) {
    WaveChunk *chunk = &file->chunk;
    WaveFormat *format = &file->format;
    SDL_RWops *fmtsrc;
    size_t fmtlen = chunk->size;

    if (fmtlen > SDL_MAX_SINT32) {
        /* Limit given by SDL_RWFromConstMem. */
        return SDL_SetError("Data of WAVE fmt chunk too big");
    }
    fmtsrc = SDL_RWFromConstMem(chunk->data, (int)chunk->size);
    if (fmtsrc == NULL) {
        return SDL_OutOfMemory();
    }

    format->formattag = SDL_ReadLE16(fmtsrc);
    format->encoding = format->formattag;
    format->channels = SDL_ReadLE16(fmtsrc);
    format->frequency = SDL_ReadLE32(fmtsrc);
    format->byterate = SDL_ReadLE32(fmtsrc);
    format->blockalign = SDL_ReadLE16(fmtsrc);

    /* This is PCM specific in the first version of the specification. */
    if (fmtlen >= 16) {
        format->bitspersample = SDL_ReadLE16(fmtsrc);
    } else if (format->encoding == PCM_CODE) {
        SDL_RWclose(fmtsrc);
        return SDL_SetError("Missing wBitsPerSample field in WAVE fmt chunk");
    }

    /* The earlier versions also don't have this field. */
    if (fmtlen >= 18) {
        format->extsize = SDL_ReadLE16(fmtsrc);
    }

    if (format->formattag == EXTENSIBLE_CODE) {
        /* note that this ignores channel masks, smaller valid bit counts
         * inside a larger container, and most subtypes. This is just enough
         * to get things that didn't really _need_ WAVE_FORMAT_EXTENSIBLE
         * to be useful working when they use this format flag.
         */

        /* Extensible header must be at least 22 bytes. */
        if (fmtlen < 40 || format->extsize < 22) {
            SDL_RWclose(fmtsrc);
            return SDL_SetError("Extensible WAVE header too small");
        }

        format->validsamplebits = SDL_ReadLE16(fmtsrc);
        format->samplesperblock = format->validsamplebits;
        format->channelmask = SDL_ReadLE32(fmtsrc);
        SDL_RWread(fmtsrc, format->subformat, 1, 16);
        format->encoding = WaveGetFormatGUIDEncoding(format);
    }

    SDL_RWclose(fmtsrc);

    return 0;
}

/**
 * Verifies the format and initializes the specific decoder
 *
 * @param file          the WaveFile struct
 * @param datalength    the length of the audio data in bytes
 *
 * @return 0 on success, -1 on error
 */
static int WaveCheckFormat(WaveFile *file, size_t datalength) {
    WaveFormat *format = &file->format;

    /* Check for some obvious issues. */

    if (format->channels == 0) {
        return SDL_SetError("Invalid number of channels");
    } else if (format->channels > 255) {
        /* Limit given by SDL_AudioSpec.channels. */
        return SDL_SetError("Number of channels exceeds limit of 255");
    }

    if (format->frequency == 0) {
        return SDL_SetError("Invalid sample rate");
    } else if (format->frequency > INT_MAX) {
        /* Limit given by SDL_AudioSpec.freq. */
        return SDL_SetError("Sample rate exceeds limit of %d", INT_MAX);
    }

    /* Reject invalid fact chunks in strict mode. */
    if (file->facthint == FactStrict && file->fact.status == -1) {
        return SDL_SetError("Invalid fact chunk in WAVE file");
    }

    /* Check for issues common to all encodings. Some unsupported formats set
     * the bits per sample to zero. These fall through to the 'unsupported
     * format' error.
     */
    switch (format->encoding) {
    case IEEE_FLOAT_CODE:
    case ALAW_CODE:
    case MULAW_CODE:
    case MS_ADPCM_CODE:
    case IMA_ADPCM_CODE:
        /* These formats require a fact chunk. */
        if (file->facthint == FactStrict && file->fact.status <= 0) {
            return SDL_SetError("Missing fact chunk in WAVE file");
        }
        SDL_FALLTHROUGH;
    case PCM_CODE:
        /* All supported formats require a non-zero bit depth. */
        if (file->chunk.size < 16) {
            return SDL_SetError("Missing wBitsPerSample field in WAVE fmt chunk");
        } else if (format->bitspersample == 0) {
            return SDL_SetError("Invalid bits per sample");
        }

        /* All supported formats must have a proper block size. */
        if (format->blockalign == 0) {
            return SDL_SetError("Invalid block alignment");
        }

        /* If the fact chunk is valid and the appropriate hint is set, the
         * decoders will use the number of sample frames from the fact chunk.
         */
        if (file->fact.status == 1) {
            WaveFactChunkHint hint = file->facthint;
            Uint32 samples = file->fact.samplelength;
            if (hint == FactTruncate || hint == FactStrict || (hint == FactIgnoreZero && samples > 0)) {
                file->fact.status = 2;
            }
        }
    }

    /* Check the format for encoding specific issues and initialize decoders. */
    switch (format->encoding) {
    case PCM_CODE:
    case IEEE_FLOAT_CODE:
        if (PCM_Init(file, datalength) < 0) {
            return -1;
        }
        break;
    case ALAW_CODE:
    case MULAW_CODE:
        if (LAW_Init(file, datalength) < 0) {
            return -1;
        }
        break;
    case MS_ADPCM_CODE:
        if (MS_ADPCM_Init(file, datalength) < 0) {
            return -1;
        }
        break;
    case IMA_ADPCM_CODE:
        if (IMA_ADPCM_Init(file, datalength) < 0) {
            return -1;
        }
        break;
    case MPEG_CODE:
    case MPEGLAYER3_CODE:
        return SDL_SetError("MPEG formats not supported");
    default:
        if (format->formattag == EXTENSIBLE_CODE) {
            const char *errstr = "Unknown WAVE format GUID: %08x-%04x-%04x-%02x%02x%02x%02x%02x%02x%02x%02x";
            const Uint8 *g = format->subformat;
            const Uint32 g1 = g[0] | ((Uint32)g[1] << 8) | ((Uint32)g[2] << 16) | ((Uint32)g[3] << 24);
            const Uint32 g2 = g[4] | ((Uint32)g[5] << 8);
            const Uint32 g3 = g[6] | ((Uint32)g[7] << 8);
            return SDL_SetError(errstr, g1, g2, g3, g[8], g[9], g[10], g[11], g[12], g[13], g[14], g[15]);
        }
        return SDL_SetError("Unknown WAVE format tag: 0x%04x", (unsigned int)format->encoding);
    }

    return 0;
}

/**
 * Attempts to load the file, initializing the WaveFile struct
 *
 * @param src   the audio stream
 * @param file  the WaveFile struct
 *
 * @return 0 on success, -1 on error
 */
static int WaveLoad(SDL_RWops *src, WaveFile *file) {
    int result;
    Uint32 chunkcount = 0;
    Uint32 chunkcountlimit = 10000;
    char *envchunkcountlimit;
    Sint64 RIFFstart, RIFFend, lastchunkpos;
    SDL_bool RIFFlengthknown = SDL_FALSE;
    WaveFormat *format = &file->format;
    WaveChunk *chunk = &file->chunk;
    WaveChunk RIFFchunk;
    WaveChunk fmtchunk;

    /** Search for the blocks */
    SDL_bool founddata = SDL_FALSE;

    SDL_zero(RIFFchunk);
    SDL_zero(fmtchunk);

    file->source = src;
    envchunkcountlimit = SDL_getenv("SDL_WAVE_CHUNK_LIMIT");
    if (envchunkcountlimit != NULL) {
        unsigned int count;
        if (SDL_sscanf(envchunkcountlimit, "%u", &count) == 1) {
            chunkcountlimit = count <= SDL_MAX_UINT32 ? count : SDL_MAX_UINT32;
        }
    }

    RIFFstart = SDL_RWtell(src);
    if (RIFFstart < 0) {
        return ATK_SetError("Could not seek in file");
    }

    RIFFchunk.position = RIFFstart;
    if (WaveNextChunk(src, &RIFFchunk) < 0) {
        return ATK_SetError("Could not read RIFF header");
    }

    /* Check main WAVE file identifiers. */
    if (RIFFchunk.fourcc == RIFF) {
        Uint32 formtype;
        /* Read the form type. "WAVE" expected. */
        if (SDL_RWread(src, &formtype, sizeof(Uint32), 1) != 1) {
            return SDL_SetError("Could not read RIFF form type");
        } else if (SDL_SwapLE32(formtype) != WAVE) {
            return ATK_SetError("RIFF form type is not WAVE (not a Waveform file)");
        }
    } else if (RIFFchunk.fourcc == WAVE) {
        /* RIFF chunk missing or skipped. Length unknown. */
        RIFFchunk.position = 0;
        RIFFchunk.length = 0;
    } else {
        return ATK_SetError("Could not find RIFF or WAVE identifiers (not a Waveform file)");
    }

    /* The 4-byte form type is immediately followed by the first chunk.*/
    chunk->position = RIFFchunk.position + 4;

    /* Use the RIFF chunk size to limit the search for the chunks. This is not
     * always reliable and the hint can be used to tune the behavior. By
     * default, it will never search past 4 GiB.
     */
    switch (file->riffhint) {
        case RiffSizeIgnore:
            RIFFend = RIFFchunk.position + SDL_MAX_UINT32;
            break;
        default:
        case RiffSizeIgnoreZero:
            if (RIFFchunk.length == 0) {
                RIFFend = RIFFchunk.position + SDL_MAX_UINT32;
                break;
            }
            SDL_FALLTHROUGH;
        case RiffSizeForce:
            RIFFend = RIFFchunk.position + RIFFchunk.length;
            RIFFlengthknown = SDL_TRUE;
            break;
        case RiffSizeMaximum:
            RIFFend = SDL_MAX_SINT64;
            break;
    }

    /* Step through all chunks and save information on the fmt, data, and fact
     * chunks. Ignore the chunks we don't know as per specification. This
     * currently also ignores cue, list, and slnt chunks.
     */
    while ((Uint64)RIFFend > (Uint64)chunk->position + chunk->length + (chunk->length & 1)) {
        /* Abort after too many chunks or else corrupt files may waste time. */
        if (chunkcount++ >= chunkcountlimit) {
            return SDL_SetError("Chunk count in WAVE file exceeds limit of %" SDL_PRIu32, chunkcountlimit);
        }

        result = WaveNextChunk(src, chunk);
        if (result == -1) {
            /* Unexpected EOF. Corrupt file or I/O issues. */
            if (file->trunchint == TruncVeryStrict) {
                return SDL_SetError("Unexpected end of WAVE file");
            }
            /* Let the checks after this loop sort this issue out. */
            break;
        } else if (result == -2) {
            return SDL_SetError("Could not seek to WAVE chunk header");
        }

        if (chunk->fourcc == FMT) {
            if (fmtchunk.fourcc == FMT) {
                /* Multiple fmt chunks. Ignore or error? */
            } else {
                /* The fmt chunk must occur before the data chunk. */
                if (founddata) {
                    return SDL_SetError("fmt chunk after data chunk in WAVE file");
                }
                fmtchunk = *chunk;
            }
        } else if (chunk->fourcc == DATA) {
            /* Only use the first data chunk. Handling the wavl list madness
             * may require a different approach.
             */
            if (!founddata) {
                founddata = SDL_TRUE;
                file->data.start = chunk->position;
                file->data.position = chunk->position;
                file->data.length   = chunk->length;
            }
        } else if (chunk->fourcc == FACT) {
            /* The fact chunk data must be at least 4 bytes for the
             * dwSampleLength field. Ignore all fact chunks after the first one.
             */
            if (file->fact.status == 0) {
                if (chunk->length < 4) {
                    file->fact.status = -1;
                } else {
                    /* Let's use src directly, it's just too convenient. */
                    Sint64 position = SDL_RWseek(src, chunk->position, RW_SEEK_SET);
                    Uint32 samplelength;
                    if (position == chunk->position && SDL_RWread(src, &samplelength, sizeof(Uint32), 1) == 1) {
                        file->fact.status = 1;
                        file->fact.samplelength = SDL_SwapLE32(samplelength);
                    } else {
                        file->fact.status = -1;
                    }
                }
            }
        } else if (chunk->fourcc == LIST) {
            // Will actually need to peek ahead for INFO later
            if (file->info.fourcc != LIST) {
                file->info = *chunk;
                WaveReadChunkData(src, &file->info);
            }
        }

        /* Go through all chunks in verystrict mode or stop the search early if
         * all required chunks were found.
         */
        if (file->trunchint == TruncVeryStrict) {
            if ((Uint64)RIFFend < (Uint64)chunk->position + chunk->length) {
                return SDL_SetError("RIFF size truncates chunk");
            }
        } else if (fmtchunk.fourcc == FMT && founddata) {
            if (file->fact.status == 1 || file->facthint == FactIgnore || file->facthint == FactNoHint) {
                break;
            }
        }
    }

    /* Save the position after the last chunk. This position will be used if the
     * RIFF length is unknown.
     */
    lastchunkpos = chunk->position + chunk->length;

    /* The fmt chunk is mandatory. */
    if (fmtchunk.fourcc != FMT) {
        return ATK_SetError("Missing fmt chunk in WAVE file");
    }
    /* A data chunk must be present. */
    if (!founddata) {
        return ATK_SetError("Missing data chunk in WAVE file");
    }
    /* Check if the last chunk has all of its data in verystrict mode. */
    if (file->trunchint == TruncVeryStrict) {
        /* data chunk is handled later. */
        if (chunk->fourcc != DATA && chunk->length > 0) {
            Uint8 tmp;
            Uint64 position = (Uint64)chunk->position + chunk->length - 1;
            if (position > SDL_MAX_SINT64 || SDL_RWseek(src, (Sint64)position, RW_SEEK_SET) != (Sint64)position) {
                return ATK_SetError("Could not seek to WAVE chunk data");
            } else if (SDL_RWread(src, &tmp, 1, 1) != 1) {
                return ATK_SetError("RIFF size truncates chunk");
            }
        }
    }

    /* Process fmt chunk. */
    *chunk = fmtchunk;

    /* No need to read more than 1046 bytes of the fmt chunk data with the
     * formats that are currently supported. (1046 because of MS ADPCM coefficients)
     */
    if (WaveReadPartialChunkData(src, chunk, 1046) < 0) {
        return SDL_SetError("Could not read data of WAVE fmt chunk");
    }

    /* The fmt chunk data must be at least 14 bytes to include all common fields.
     * It usually is 16 and larger depending on the header and encoding.
     */
    if (chunk->length < 14) {
        return SDL_SetError("Invalid WAVE fmt chunk length (too small)");
    } else if (chunk->size < 14) {
        return SDL_SetError("Could not read data of WAVE fmt chunk");
    } else if (WaveReadFormat(file) < 0) {
        return -1;
    } else if (WaveCheckFormat(file, (size_t)file->data.length) < 0) {
        return -1;
    }

#ifdef SDL_WAVE_DEBUG_LOG_FORMAT
    WaveDebugLogFormat(file);
#endif
#ifdef SDL_WAVE_DEBUG_DUMP_FORMAT
    WaveDebugDumpFormat(file, RIFFchunk.length, fmtchunk.length, datachunk.length);
#endif
    WaveFreeChunkData(chunk);

    switch (format->encoding) {
    case MS_ADPCM_CODE:
    case IMA_ADPCM_CODE:
    case ALAW_CODE:
    case MULAW_CODE:
        /* These can be easily stored in the byte order of the system. */
        file->samplefmt = AUDIO_S16SYS;
        break;
    case IEEE_FLOAT_CODE:
        file->samplefmt = AUDIO_F32LSB;
        break;
    case PCM_CODE:
        switch (format->bitspersample) {
        case 8:
            file->samplefmt = AUDIO_U8;
            break;
        case 16:
            file->samplefmt = AUDIO_S16LSB;
            break;
        case 24: /* Has been shifted to 32 bits. */
        case 32:
            file->samplefmt = AUDIO_S32LSB;
            break;
        default:
            /* Just in case something unexpected happened in the checks. */
            return SDL_SetError("Unexpected %u-bit PCM data format",
                                (unsigned int)format->bitspersample);
        }
        break;
    }

    /* Report the end position back to the cleanup code. */
    if (RIFFlengthknown) {
        chunk->position = RIFFend;
    } else {
        chunk->position = lastchunkpos;
    }
    return 0;
}

/**
 * Starts decoding of the Wave file
 *
 * @param file  the WaveFile struct
 *
 * @return 0 on success, error code on error.
 */
static int WaveBegin(WaveFile *file) {
    WaveFormat *format = &file->format;

    /* Initialize the decoders. */
    switch (format->encoding) {
    case PCM_CODE:
    case IEEE_FLOAT_CODE:
        return PCM_BeginDecode(file);
    case ALAW_CODE:
    case MULAW_CODE:
        return LAW_BeginDecode(file);
    case MS_ADPCM_CODE:
        return MS_ADPCM_BeginDecode(file);
    case IMA_ADPCM_CODE:
        return IMA_ADPCM_BeginDecode(file);
    default:
        ATK_SetError("Unrecognized WAV encoding");
        return -1;
    }
}

/**
 * Reads a single page of audio data into the given buffer
 *
 * The array buffer should be large enough to read the data.
 *
 * @param file      the WaveFile struct
 * @param buffer    the buffer to store the data
 *
 * @return the number of bytes read, error code on error.
 */
static int WaveStep(WaveFile *file, Uint8 *buffer) {
    WaveFormat *format = &file->format;
    WaveBlock *block = &file->data;
    //size_t outsize;

    /* Decode or convert the data if necessary. */
    Sint64 amt = 0;
    switch (format->encoding) {
    case PCM_CODE:
    case IEEE_FLOAT_CODE:
        amt = PCM_StepDecode(file, buffer);
        break;
    case ALAW_CODE:
    case MULAW_CODE:
        amt = LAW_StepDecode(file, buffer);
        break;
    case MS_ADPCM_CODE:
        amt = MS_ADPCM_StepDecode(file, buffer);
        break;
    case IMA_ADPCM_CODE:
        amt = IMA_ADPCM_StepDecode(file, buffer);
        break;
    }

    // Process error codes or empty streams
    if (amt <= 0) {
        return (int)amt;
    }

    // Convert the buffer in-place
    Sint32* in32;
    Sint16* in16;
    Uint8*  in8;
    float*  out = (float*)buffer;
    size_t ii, samples = 0;
    double factor = 1;

    SDL_bool swap = SDL_SwapLE16(1) == 1;
    switch (format->encoding) {
        case MS_ADPCM_CODE:
        case IMA_ADPCM_CODE:
            /** These are Sint16 values */
            samples = amt/sizeof(Sint16);
            in16 = (Sint16*)buffer;
            factor = (double)((1 << 15)-1);
            for (ii = samples; ii > 0; ii--) {
                out[ii-1] = (float)(in16[ii-1]/factor);
            }
            break;
        case ALAW_CODE:
        case MULAW_CODE:
            /** These are encoded as Sint16 bytes */
            samples = amt/sizeof(Sint16);
            in16 = (Sint16*)buffer;
            factor = (double)((1 << 15)-1);
            for (ii = samples; ii > 0; ii--) {
                out[ii-1] = (float)(in16[ii-1]/factor);
            }
            break;
        case PCM_CODE:
            switch (format->bitspersample) {
                case 8:
                    samples = amt;
                    in8 = block->data;
                    factor = (double)((1 << 7)-1);
                    for (ii = samples; ii > 0; ii--) {
                        out[ii-1] = (float)(in8[ii-1]/factor);
                    }
                    break;
                case 16:
                    samples = amt/sizeof(Sint16);
                    in16 = (Sint16*)buffer;
                    factor = (double)((1 << 15)-1);
                    if (swap) {
                        for (ii = samples; ii > 0; ii--) {
                            out[ii-1] = (float)(SDL_SwapLE16(in16[ii-1])/factor);
                        }
                    } else {
                        for (ii = samples; ii > 0; ii--) {
                            out[ii-1] = (float)(in16[ii-1]/factor);
                        }
                    }
                    break;
                case 24:
                case 32:
                    samples = amt/sizeof(Sint32);
                    in32 = (Sint32*)buffer;
                    Uint64 base = 1L;
                    factor = (double)((base << 31)-1);
                    if (swap) {
                        for (ii = samples; ii > 0; ii--) {
                            out[ii-1] = (float)(SDL_SwapLE32(in32[ii-1])/factor);
                        }
                    } else {
                        for (ii = samples; ii > 0; ii--) {
                            out[ii-1] = (float)(in32[ii-1]/factor);
                        }
                    }
                    break;
                default:
                    /* Just in case something unexpected happened in the checks. */
                    return ATK_SetError("Unexpected %u-bit PCM data format",
                                        (unsigned int)format->bitspersample);
            }
            break;
        case IEEE_FLOAT_CODE:
            samples = amt;
            break;
    }

    return (int)samples;
}

/**
 * Finishes process the Wave file, releasing all memory
 *
 * @param file  the WaveFile struct
 *
 * @return 0 on success, -1 on error.
 */
static int WaveEnd(WaveFile *file) {
    if (file->ownsource) {
        SDL_RWclose(file->source);
    } else {
        SDL_RWseek(file->source, file->chunk.position, RW_SEEK_SET);
    }

    WaveFormat *format = &file->format;

    /* Initialize the decoders. */
    switch (format->encoding) {
    case PCM_CODE:
    case IEEE_FLOAT_CODE:
        return PCM_EndDecode(file);
    case ALAW_CODE:
    case MULAW_CODE:
        return LAW_EndDecode(file);
    case MS_ADPCM_CODE:
        return MS_ADPCM_EndDecode(file);
    case IMA_ADPCM_CODE:
        return IMA_ADPCM_EndDecode(file);
    default:
        ATK_SetError("Unrecognized WAV encoding");
        return -1;
    }

    if (file->info.fourcc == LIST) {
        WaveFreeChunkData(&file->info);
    }
}

/**
 * Reads the INFO block into the stream metadata
 *
 * @param file      the WaveFile struct
 * @param metadata  the stream metadata
 *
 * @return 0 on success, -1 on error.
 */
static int WaveComments(WaveFile *file, ATK_AudioMetadata* metadata) {
    WaveChunk *chunk = &file->info;
    Uint32* code = (Uint32*)(chunk->data);
    if (code == NULL || *code != INFO) {
        return 0;
    }

    Uint32 pos = 4;
    char tag[5];

    char* val;
    Uint32 len;
    memset(tag, 0, 5);

    // First count number of valid comments
    Uint32 num_com = 0;
    while (pos < chunk->size) {
        val = (char*)chunk->data+pos;
        memcpy(tag, val, 4);
        len = *((Uint32*)(chunk->data+pos+4));
        len = SDL_SwapLE32(len);
        if (ATK_GetInfoCommentTag((char*)tag)) {
            num_com++;
        }
        pos += len+8;
    }

    if (num_com > 0) {
        ATK_AudioComment* comments = (ATK_AudioComment*)ATK_malloc(sizeof(ATK_AudioComment)*num_com);
        memset(comments, 0, sizeof(ATK_AudioComment)*num_com);
        metadata->comments = comments;

        size_t ii = 0;
        pos = 4;
        while (pos < chunk->size) {
            val = (char*)chunk->data+pos;
            memcpy(tag, val, 4);
            len = *((Uint32*)(chunk->data+pos+4));
            len = SDL_SwapLE32(len);
            const char* key = ATK_GetInfoCommentTag((char*)tag);
            if (key) {
                char* item = ATK_malloc(strlen(key)+1);
                memset(item,0,strlen(key)+1);
                memcpy(item, key, strlen(key)+1);
                DECONST(char*,comments[ii].key) = item;

                key = val+8;
                item = ATK_malloc(len+1);
                memset(item,0,len+1);
                memcpy(item, key, len-1);
                DECONST(char*,comments[ii].value) = item;
            }
            pos += len+8;
            ii++;
        }
    }


    DECONST(Uint16,metadata->num_comments) = (Uint16)num_com;
    return num_com;
}

/**
 * Stores the current decoding state, to be recovered later
 *
 * This is used to remember where the data block was in the read process.
 *
 * @param file  the WaveFile struct
 * @param state the decoding state
 */
static void WavePushState(WaveFile *file, WaveState* state) {
    WaveFormat *format = &file->format;
    WaveBlock* block   = &(file->data);
    SDL_RWops* stream  = file->source;
    ADPCM_DecoderState* adpcm = NULL;
    if (format->encoding == MS_ADPCM_CODE || format->encoding == IMA_ADPCM_CODE) {
        adpcm = (ADPCM_DecoderState*)file->decoderdata;
    }

    state->filepos = block->position;
    if (state->filepos != block->start) {
        Block_Seek(stream, block, block->start);
    }

    if (adpcm) {
        state->framesleft = adpcm->framesleft;
        adpcm->framesleft = adpcm->framestotal;
    }
}

/**
 * Restores the previously stored decoding state
 *
 * This is used to recover a stored data block position.
 *
 * @param file  the WaveFile struct
 * @param state the decoding state
 */
static void WavePopState(WaveFile *file, WaveState* state) {
    WaveFormat *format = &file->format;
    WaveBlock* block   = &(file->data);
    SDL_RWops* stream  = file->source;
    ADPCM_DecoderState* adpcm = NULL;
    if (format->encoding == MS_ADPCM_CODE || format->encoding == IMA_ADPCM_CODE) {
        adpcm = (ADPCM_DecoderState*)file->decoderdata;
    }

    if (state->filepos != block->start) {
        Block_Seek(stream, block, state->filepos);
    }

    if (adpcm) {
        adpcm->framesleft = state->framesleft;
    }
}


#pragma mark ATK Functions
/**
 * Creates a new ATK_AudioSource from an WAV file
 *
 * This function will return NULL if the file cannot be located or is not an
 * supported WAV file. Note that WAV is a container type in addition to a codec,
 * and so not all WAV files are supported. The file will not be read into memory,
 * but is instead available for streaming.
 *
 * It is the responsibility of the caller of this function to close the source
 * (with {@link ATK_UnloadSource}) when it is no longer needed.
 *
 * @param filename    The name of the file with audio data
 *
 * @return a new CODEC_Source from an WAV file
 */
ATK_AudioSource* ATK_LoadWAV(const char* filename) {
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
    return ATK_LoadWAV_RW(stream,1);
}

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
ATK_AudioSource* ATK_LoadWAV_RW(SDL_RWops* source, int ownsrc) {
    if (source == NULL) {
        ATK_SetError("NULL source data");
        return NULL;
    }

    WaveFile* file = (WaveFile*)ATK_malloc(sizeof(WaveFile));
    SDL_zero(*file);

    /* Make sure we are passed a valid data source */
    if (source == NULL) {
        /* Error may come from RWops. */
        return NULL;
    }

    file->riffhint = WaveGetRiffSizeHint();
    file->trunchint = WaveGetTruncationHint();
    file->facthint = WaveGetFactChunkHint();
    file->ownsource = ownsrc;

    if (WaveLoad(source, file) < 0) {
        return NULL;
    }
    if (WaveBegin(file) < 0) {
        return NULL;
    }

    ATK_AudioSource* result = (ATK_AudioSource*)ATK_malloc(sizeof(ATK_AudioSource));
    if (!result) {
        ATK_OutOfMemory();
        WaveEnd(file);
        if (ownsrc) {
            SDL_RWclose(source);
        }
        return NULL;
    }
    SDL_zero(*result);

    result->decoder = file;
    DECONST(ATK_CodecType, result->type) = ATK_CODEC_WAV;

    DECONST(Uint32,result->metadata.rate) = file->format.frequency;
    DECONST(Uint8,result->metadata.channels) = (Uint8)file->format.channels;
    DECONST(Uint64,result->metadata.frames) = file->sampleframes;

    // Read comments
    WaveComments(file,(ATK_AudioMetadata *)&result->metadata);

    return result;
}

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
 * @returns SDL_true if this is WAV data.
 */
SDL_bool ATK_SourceIsWAV(SDL_RWops* source) {
    if (source == NULL) {
        return SDL_FALSE;
    }

    Sint64 pos = SDL_RWtell(source);
    SDL_bool result = SDL_FALSE;

    WaveFile file;
    SDL_zero(file);

    file.riffhint = WaveGetRiffSizeHint();
    file.trunchint = WaveGetTruncationHint();
    file.facthint = WaveGetFactChunkHint();

    if (!WaveLoad(source, &file)) {
        result = SDL_TRUE;
    }

    if (file.info.fourcc == LIST) {
        WaveFreeChunkData(&file.info);
    }
    ATK_ClearError();
    SDL_RWseek(source, pos, RW_SEEK_SET);
    return result;
}

/**
 * The WAV specific implementation of {@link ATK_UnloadSource}.
 *
 * @param source    The source to close
 *
 * @return 0 if the source was successfully closed, -1 otherwise.
 */
Sint32 ATK_WAV_UnloadSource(ATK_AudioSource* source) {
	CHECK_SOURCE(source,-1)
    if (source->metadata.comments != NULL) {
        ATK_FreeComments(DECONST(ATK_AudioComment*,source->metadata.comments),
                         source->metadata.num_comments);
        DECONST(ATK_AudioComment*,source->metadata.comments) = NULL;
    }

    WaveFile* file = (WaveFile*)(source->decoder);
    if (file != NULL) {
        WaveEnd(file);
        ATK_free(file);
    }
	ATK_free(source);
	return 0;
}

/**
 * The WAV specific implementation of {@link ATK_SeekSourcePage}.
 *
 * @param source    The audio source
 * @param page        The page to seek
 *
 * @return the page acquired, or -1 if there is an error
 */
Sint32 ATK_WAV_SeekSourcePage(ATK_AudioSource* source, Uint32 page) {
	CHECK_SOURCE(source,-1)

    WaveFile* file = (WaveFile*)(source->decoder);
    WaveBlock* block  = &(file->data);
    SDL_RWops* stream = file->source;
    Uint64 offset = block->start+block->pagesize*page;
    offset = Block_Seek(stream, block, offset);

    if (offset < 0) {
        return -1;
    }

    WaveFormat *format = &file->format;
    if (format->encoding == MS_ADPCM_CODE || format->encoding == IMA_ADPCM_CODE) {
        ADPCM_DecoderState* state = (ADPCM_DecoderState*)file->decoderdata;
        state->framesleft = state->framestotal-state->samplesperblock*page;
    }

    return (Sint32)((offset-block->start)/WAV_PAGE_SIZE);
}

/**
 * The WAV specific implementation of {@link ATK_GetSourcePageSize}.
 *
 * @param source    The audio source
 *
 * @return the number of audio frames in a page, or -1 if there is an error
 */
Sint32 ATK_WAV_GetSourcePageSize(ATK_AudioSource* source) {
    CHECK_SOURCE(source,-1)
    WaveFile* file = (WaveFile*)(source->decoder);
    WaveBlock* block  = &(file->data);
    switch (file->format.encoding) {
    case PCM_CODE:
    case IEEE_FLOAT_CODE:
        return (Sint32)(block->pagesize/file->format.blockalign);
    case ALAW_CODE:
    case MULAW_CODE:
        return (Sint32)(block->pagesize/file->format.blockalign);
    case MS_ADPCM_CODE:
    case IMA_ADPCM_CODE:
        return file->format.samplesperblock;
    case MPEG_CODE:
    case MPEGLAYER3_CODE:
    default:
        return -1;
    }
}

/**
 * The WAV specific implementation of {@link ATK_GetSourceFirstPageSize}.
 *
 * @param source    The audio source
 *
 * @return the number of audio frames on the first page, or -1 if there is an error
 */
Sint32 ATK_WAV_GetSourceFirstPageSize(ATK_AudioSource* source) {
    return ATK_WAV_GetSourcePageSize(source);
}

/**
 * The WAV specific implementation of {@link ATK_GetSourceLastPage}.
 *
 * @param source    The audio source
 *
 * @return the index of the last page in the stream, or -1 if there is an error
 */
Sint32 ATK_WAV_GetSourceLastPage(ATK_AudioSource* source) {
    CHECK_SOURCE(source,-1)
    WaveFile* file = (WaveFile*)(source->decoder);
    WaveBlock* block  = &(file->data);
    Uint32 result = (Uint32)(block->length/block->pagesize);

    if (block->length % block->pagesize) {
        result += 1;
    }
    return result;
}

/**
 * The WAV specific implementation of {@link ATK_GetSourceCurrentPage}.
 *
 * @param source    The audio source
 *
 * @return the index of the current page in the stream, or -1 if there is an error
 */
Sint32 ATK_WAV_GetSourceCurrentPage(ATK_AudioSource* source) {
    CHECK_SOURCE(source,-1)
    WaveFile* file = (WaveFile*)(source->decoder);
    WaveBlock* block  = &(file->data);
    Sint32 result = (Sint32)((block->position-block->start)/block->pagesize);
    if ((block->position-block->start) % block->pagesize) {
        result += 1;
    }
    return result;
}

/**
 * The WAV specific implementation of {@link ATK_IsSourceEOF}.
 *
 * @param source    The audio source
 *
 * @return 1 if the audio source is at the end of the stream; 0 otherwise
 */
Uint32 ATK_WAV_IsSourceEOF(ATK_AudioSource* source) {
    CHECK_SOURCE(source,0)
    WaveFile* file = (WaveFile*)(source->decoder);
    WaveBlock* block = &(file->data);
    return (block->position-block->start) == block->length;
}

/**
 * The WAV specific implementation of {@link ATK_ReadSourcePage}.
 *
 * @param source    The audio source
 * @param buffer    The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
Sint32 ATK_WAV_ReadSourcePage(ATK_AudioSource* source, float* buffer) {
	CHECK_SOURCE(source,-1)

    WaveFile* file = (WaveFile*)(source->decoder);
    Uint64 samples = WaveStep(file, (Uint8*)buffer);
    return (Sint32)(samples/file->format.channels);
}

/**
 * The WAV specific implementation of {@link ATK_ReadSource}.
 *
 * @param source    The audio source
 * @param buffer    The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
Sint64 ATK_WAV_ReadSource(ATK_AudioSource* source, float* buffer) {
    CHECK_SOURCE(source,-1)

    WaveFile* file = (WaveFile*)(source->decoder);
    WaveState state;
    memset(&state, 0, sizeof(WaveState));

    WavePushState(file, &state);

    Uint8* output = (Uint8*)buffer;
    Sint32 amt  = WaveStep(file, output);
    if (amt < 0) {
        return amt;
    }

    Sint64 read = 0;
    while (amt > 0) {
        output += amt*sizeof(float);
        read += amt;
        amt = WaveStep(file, output);
    }

    read /= file->format.channels;

    WavePopState(file, &state);
    return read;
}

#else
#pragma mark -
#pragma mark Dummy Decoding
/**
 * Creates a new ATK_AudioSource from an WAV file
 *
 * This function will return NULL if the file cannot be located or is not an
 * supported WAV file. Note that WAV is a container type in addition to a codec,
 * and so not all WAV files are supported. The file will not be read into memory,
 * but is instead available for streaming.
 *
 * It is the responsibility of the caller of this function to close the source
 * (with {@link ATK_UnloadSource}) when it is no longer needed.
 *
 * @param filename    The name of the file with audio data
 *
 * @return a new CODEC_Source from an WAV file
 */
ATK_AudioSource* ATK_LoadWAV(const char* filename) {
    ATK_SetError("Codec WAV is not supported");
    return NULL;
}

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
ATK_AudioSource* ATK_LoadWAV_RW(SDL_RWops* source, int ownsrc) {
    ATK_SetError("Codec WAV is not supported");
    return NULL;
}

/**
 * Creates a new ATK_AudioSource from an WAV file
 *
 * This function will return NULL if the file cannot be located or is not an
 * supported WAV file. Note that WAV is a container type in addition to a codec,
 * and so not all WAV files are supported. The file will not be read into memory,
 * but is instead available for streaming.
 *
 * It is the responsibility of the caller of this function to close the source
 * (with {@link ATK_UnloadAudioSource}) when it is no longer needed.
 *
 * @param filename    The name of the file with audio data
 *
 * @return a new CODEC_Source from an WAV file
 */
ATK_AudioSource* ATK_WAV_LoadSource(const char* filename) {
    ATK_SetError("Codec WAV is not supported");
	return NULL;
}

/**
 * Creates a new ATK_AudioSource from an WAV readable/seekable RWops
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
 * @return a new CODEC_Source from an WAV readable/seekable RWops
 */
ATK_AudioSource* ATK_WAV_LoadSource_RW(SDL_RWops* source, int ownsrc) {
    ATK_SetError("Codec WAV is not supported");
    return NULL;
}

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
 * @returns SDL_true if this is WAV data.
 */
SDL_bool ATK_SourceIsWAV(SDL_RWops* source) { return SDL_FALSE; }

/**
 * The WAV specific implementation of {@link ATK_UnloadSource}.
 *
 * @param source    The source to close
 *
 * @return 0 if the source was successfully closed, -1 otherwise.
 */
Sint32 ATK_WAV_UnloadSource(ATK_AudioSource* source) { return -1; }

/**
 * The WAV specific implementation of {@link ATK_SeekSourcePage}.
 *
 * @param source    The audio source
 * @param page        The page to seek
 *
 * @return the page acquired, or -1 if there is an error
 */
Sint32 ATK_WAV_SeekSourcePage(ATK_AudioSource* source, Uint32 page) { return -1; }

/**
 * The WAV specific implementation of {@link ATK_GetSourcePageSize}.
 *
 * @param source    The audio source
 *
 * @return the number of audio frames in a page, or -1 if there is an error
 */
Sint32 ATK_WAV_GetSourcePageSize(ATK_AudioSource* source) { return -1; }

/**
 * The WAV specific implementation of {@link ATK_GetSourceFirstPageSize}.
 *
 * @param source    The audio source
 *
 * @return the number of audio frames on the first page, or -1 if there is an error
 */
Sint32 ATK_WAV_GetSourceFirstPageSize(ATK_AudioSource* source) { return -1; }

/**
 * The WAV specific implementation of {@link ATK_GetSourceLastPage}.
 *
 * @param source    The audio source
 *
 * @return the index of the last page in the stream, or -1 if there is an error
 */
Sint32 ATK_WAV_GetSourceLastPage(ATK_AudioSource* source) { return -1; }

/**
 * The WAV specific implementation of {@link ATK_GetSourceCurrentPage}.
 *
 * @param source    The audio source
 *
 * @return the index of the current page in the stream, or -1 if there is an error
 */
Sint32 ATK_WAV_GetSourceCurrentPage(ATK_AudioSource* source) { return -1; }

/**
 * The WAV specific implementation of {@link ATK_IsSourceEOF}.
 *
 * @param source    The audio source
 *
 * @return 1 if the audio source is at the end of the stream; 0 otherwise
 */
Uint32 ATK_WAV_IsSourceEOF(ATK_AudioSource* source) { return 0; }

/**
 * The WAV specific implementation of {@link ATK_ReadSourcePage}.
 *
 * @param source    The audio source
 * @param buffer    The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
Sint32 ATK_WAV_ReadSourcePage(ATK_AudioSource* source, float* buffer) { return -1; }

/**
 * The WAV specific implementation of {@link ATK_ReadSource}.
 *
 * @param source    The audio source
 * @param buffer    The buffer to hold the PCM data
 *
 * @return the number of audio frames (samples/channels) read
 */
Sint64 ATK_WAV_ReadSource(ATK_AudioSource* source, float* buffer) { return -1; }

/**
 * Returns SDL_TRUE if WAV supports the given comment tag.
 *
 * @param tag   The tag to query
 *
 * @return SDL_TRUE if WAV supports the given comment tag.
 */
SDL_bool ATK_WAV_SupportsCommentTag(const char* tag) { return SDL_FALSE; }

/**
 * Returns an array of comment tags supported by the WAV codec
 *
 * @return an array of comment tags supported by the WAV codec
 */
const char* const* ATK_WAV_GetCommentTags() { return NULL; }

#endif // LOAD_WAV

#ifdef SAVE_WAV
#include "ATK_CodecWav_c.h"

#pragma mark -
#pragma mark WAVE Encoding
/**
 * Returns a new WAV encoding stream to write to the given file
 *
 * The provided metadata will be copied to the encoding object. So it is safe to delete
 * this metadata before the encoding is complete. If the encoding stream cannot be
 * allocated, or this library does not supported saving to WAV, this function will
 * return NULL and an error value will be set.
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
 * @param type      The codec type
 * @param metadata  The stream metadata
 *
 * @return a new FLAC encoding stream to write to the given file
 */
ATK_AudioEncoding* ATK_EncodeWAV(const char* filename,
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
    return ATK_EncodeWAV_RW(stream,1,metadata);
}

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
 * @param type      The codec type
 * @param metadata  The stream metadata
 *
 * @return a new WAV encoding stream to write to the given RWops
 */
ATK_AudioEncoding* ATK_EncodeWAV_RW(SDL_RWops* source, int ownsrc,
                                    const ATK_AudioMetadata* metadata) {
    if (!source) {
        ATK_SetError("NULL file target");
        return NULL;
    }

    ATK_AudioEncoding* result = ATK_malloc(sizeof(ATK_AudioEncoding));
    if (!result) {
        ATK_OutOfMemory();
        return NULL;
    }
    memset(result, 0, sizeof(ATK_AudioEncoding));

    WaveOut* output = ATK_malloc(sizeof(WaveOut));
    if (!result) {
        ATK_OutOfMemory();
        ATK_free(result);
        return NULL;
    }
    output->source = source;
    output->ownsource = ownsrc;
    output->begin = SDL_RWtell(source);
    output->written = 0;

    // Copy the metadata
    DECONST(ATK_CodecType,result->type) = ATK_CODEC_WAV;
    DECONST(Uint32,result->metadata.rate) = metadata->rate;
    DECONST(Uint64,result->metadata.frames) = metadata->frames;
    DECONST(Uint8,result->metadata.channels) = metadata->channels;
    DECONST(Uint16,result->metadata.num_comments) = metadata->num_comments;
    DECONST(ATK_AudioComment*,result->metadata.comments) = ATK_CopyComments(metadata->comments,metadata->num_comments);

    short BPS = 16;
    Uint32 data_len = (Uint32)(metadata->frames*metadata->channels*BPS)/8;

    // Count the metadata size
    Uint32 meta_len = 0;
    if (metadata->num_comments) {
        meta_len = 4;
        for(int ii = 0; ii < metadata->num_comments; ii++) {
            if (ATK_GetCommentInfoTag(metadata->comments[ii].key)) {
                meta_len += 8;
                meta_len += (Uint32)strlen(metadata->comments[ii].value)+1;
            }
        }
    }

    // Now write the header and metadata comments
    output->meta_len = meta_len;
    if (meta_len > 0) {
        output->data_off = meta_len + (meta_len % 2 == 1 ? 1 : 0)+36;
    } else {
        output->data_off = 36;

    }

    WaveHead header;
    header.riffcc = RIFF;
    header.wavecc = WAVE;
    header.fmtcc  = FMT;

    header.format_len = SDL_SwapLE32(0x10);
    header.fixed = SDL_SwapLE16(1);
    header.channels = SDL_SwapLE16(metadata->channels);
    header.sample_rate = SDL_SwapLE32(metadata->rate);
    header.bits_per_sample = SDL_SwapLE16(BPS);
    header.byte_per_sample = SDL_SwapLE16((BPS * metadata->channels) / 8);
    header.byte_rate = SDL_SwapLE32((metadata->rate * BPS * metadata->channels)/8);
    header.package_len = SDL_SwapLE32((Uint32)(data_len+output->data_off));

    // Write header
    if (SDL_RWwrite(source, &(header), sizeof(WaveHead), 1) < 1) {
        goto fail;
    }
    // Write metadata
    if (meta_len > 0) {
        Uint32 code = LIST;
        if (SDL_RWwrite(source, &code, sizeof(Uint32), 1) < 1) {
            goto fail;
        }
        code = SDL_SwapLE32(meta_len);
        if (SDL_RWwrite(source, &code, sizeof(Uint32), 1) < 1) {
            goto fail;
        }
        code = INFO;
        if (SDL_RWwrite(source, &code, sizeof(Uint32), 1) < 1) {
            goto fail;
        }
        if (metadata->num_comments) {
            for(int ii = 0; ii < metadata->num_comments; ii++) {
                const char* info = ATK_GetCommentInfoTag(metadata->comments[ii].key);
                if (info) {
                    if (SDL_RWwrite(source, info, sizeof(char), 4) < 1) {
                        goto fail;
                    }
                    const char* val = metadata->comments[ii].value;
                    Uint32 size = SDL_SwapLE32((Uint32)strlen(val)+1);
                    if (SDL_RWwrite(source, &size, sizeof(Uint32), 1) < 1) {
                        goto fail;
                    }
                    if (SDL_RWwrite(source, val, sizeof(char), strlen(val)+1) < 1) {
                        goto fail;
                    }
                }
            }
        }
        // Get the byte alignment correct
        if (meta_len % 2 == 1) {
            char nullc = 0;
            if (SDL_RWwrite(source, &nullc, sizeof(char), 1) < 1) {
                goto fail;
            }
        }
    }

    // Write data header
    Uint32 code = DATA;
    if (SDL_RWwrite(source, &code, sizeof(Uint32), 1) < 1) {
        goto fail;
    }
    code = SDL_SwapLE32(data_len);
    if (SDL_RWwrite(source, &code, sizeof(Uint32), 1) < 1) {
        goto fail;
    }

    result->encoder  = output;
    return result;

fail:
    // Delete everything if cannot write header
    ATK_FreeComments(DECONST(ATK_AudioComment*,result->metadata.comments),
                     result->metadata.num_comments);
    DECONST(ATK_AudioComment*,result->metadata.comments) = NULL;
    ATK_free(result);
    ATK_free(output);
    if (ownsrc) {
        SDL_RWclose(source);
    }
    return NULL;
}

/**
 * The WAV specific implementation of {@link ATK_WriteEncoding}.
 *
 * @param encoding  The encoding stream
 * @param buffer    The data buffer with the audio samples
 * @param frames    The number of audio frames to write
 *
 * @return the number of frames written, or -1 on error
 */
Sint64 ATK_WAV_WriteEncoding(ATK_AudioEncoding* encoding, float* buffer, size_t frames) {
    WaveOut* output = (WaveOut*)encoding->encoder;
    if (!output) {
        ATK_SetError("Missing codec data");
        return -1;
    }

    size_t samples = frames*encoding->metadata.channels;
    Sint16* out = ATK_malloc(sizeof(Sint16)*samples);
    if (!out) {
        ATK_OutOfMemory();
        return -1;
    }

    // Copy and convert
    SDL_bool swap = SDL_SwapLE16(1) == 1;
    Sint16 limit = ((1 << 15)-1); // Else -1 and 1 are same.
    if (swap) {
        for (int ii = 0; ii < samples; ii++) {
            out[ii] = SDL_SwapLE16((Sint16)(limit*buffer[ii]));
        }
    } else {
        for (int ii = 0; ii < samples; ii++) {
            out[ii] = (Sint16)(limit*buffer[ii]);
        }
    }

    size_t amt = SDL_RWwrite(output->source, out, sizeof(Sint16), samples);
    size_t written = amt/encoding->metadata.channels;
    output->written += written;
    ATK_free(out);
    return written;
}

/**
 * The WAV specific implementation of {@link ATK_FinishEncoding}.
 *
 * @param encoding  The encoding stream
 *
 * @return 0 if the enconding was successfully completed, -1 otherwise.
 */
Sint32 ATK_WAV_FinishEncoding(ATK_AudioEncoding* encoding) {
    if (encoding->metadata.comments != NULL) {
        ATK_FreeComments(DECONST(ATK_AudioComment*,encoding->metadata.comments),
                         encoding->metadata.num_comments);
        DECONST(ATK_AudioComment*,encoding->metadata.comments) = NULL;
    }
    int failure = -1;
    if (encoding->encoder != NULL) {
        failure = 0;
        WaveOut* output = (WaveOut*)encoding->encoder;
        if (output->written != encoding->metadata.frames) {
            short BPS = 16;
            Uint32 data = (Uint32)(output->written*encoding->metadata.channels*BPS)/8;
            Uint32 pckg = SDL_SwapLE32((Uint32)(data+output->data_off));
            data = SDL_SwapLE32(data);
            if (!failure && SDL_RWseek(output->source, output->data_off+4, RW_SEEK_SET) < 0) {
                failure = -1;
            }
            if (!failure && SDL_RWwrite(output->source, &data, sizeof(Uint32), 1) < 0) {
                failure = -1;
            }
            if (!failure && SDL_RWseek(output->source, 4, RW_SEEK_SET) < 0) {
                failure = -1;
            }
            if (!failure && SDL_RWwrite(output->source, &pckg, sizeof(Uint32), 1) < 0) {
                failure = -1;
            }
        }

        if (output->ownsource) {
            SDL_RWclose(output->source);
        } else {
            SDL_RWseek(output->source, output->begin, RW_SEEK_SET);
        }
        ATK_free(output);
        encoding->encoder = NULL;
    }

    return failure;
}

#else

#pragma mark -
#pragma mark Dummy Encoding
/**
 * Returns a new WAV encoding stream to write to the given file
 *
 * The provided metadata will be copied to the encoding object. So it is safe to delete
 * this metadata before the encoding is complete. If the encoding stream cannot be
 * allocated, or this library does not supported saving to WAV, this function will
 * return NULL and an error value will be set.
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
 * @param type      The codec type
 * @param metadata  The stream metadata
 *
 * @return a new FLAC encoding stream to write to the given file
 */
ATK_AudioEncoding* ATK_EncodeWAV(const char* filename,
                                 const ATK_AudioMetadata* metadata) {
    ATK_SetError("Codec WAV is not supported");
    return NULL;
}

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
 * @param type      The codec type
 * @param metadata  The stream metadata
 *
 * @return a new WAV encoding stream to write to the given RWops
 */
ATK_AudioEncoding* ATK_EncodeWAV_RW(SDL_RWops* source, int ownsrc,
                                    const ATK_AudioMetadata* metadata) {
    ATK_SetError("Codec WAV is not supported");
    return NULL;
}

/**
 * The WAV specific implementation of {@link ATK_WriteEncoding}.
 *
 * @param encoding  The encoding stream
 * @param buffer    The data buffer with the audio samples
 * @param frames    The number of audio frames to write
 *
 * @return the number of frames written, or -1 on error
 */
Sint64 ATK_WAV_WriteEncoding(ATK_AudioEncoding* encoding, float* buffer, size_t frames) {
    return -1;
}

/**
 * The WAV specific implementation of {@link ATK_FinishEncoding}.
 *
 * @param encoding  The encoding stream
 *
 * @return 0 if the enconding was successfully completed, -1 otherwise.
 */
Sint32 ATK_WAV_FinishEncoding(ATK_AudioEncoding* encoding) {
    return -1;
}

#endif // SAVE_WAV
