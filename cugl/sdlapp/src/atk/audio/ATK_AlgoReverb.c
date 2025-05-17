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
#include <ATK_error.h>
#include <ATK_audio.h>
#include <ATK_math.h>
#include <ATK_dsp.h>

/**
 * @file ATK_AlgoReverb
 *
 * This component provides support for algorithmic (non-convolutional) reverb
 * based on the FreeVerb algorithm by Jezar at Dreampoint, June 2000. That code
 * is in the public domain. For more details about this algorithm, see
 *
 *     https://ccrma.stanford.edu/~jos/pasp/Freeverb.html
 */

#pragma mark Tuning Constants
// Provided by Jezar at Dreampoint, June 2000

/** Number of comb filters */
#define NUM_COMBS   8
/** Number of allpass filters */
#define NUM_ALLPS   4
/** The scaling factor on the wet (reverb) signal */
#define SCALE_WET   3
/** The scaling factor on the dry (reverb) signal */
#define SCALE_DRY   2
/** The scaling factor on (user defined) damping */
#define SCALE_DAMP  0.4f
/** The scaling factor on (user defined) room size */
#define SCALE_ROOM  0.28f
/** The zero offset for the (user defined) room size */
#define OFFSET_ROOM 0.7f
/** The delay spread between left and right speakers */
#define STEREO_SPREAD   23
/** The (initial) input gain */
#define INITIAL_GAIN    0.015f
/** The initial (percentage of) the wet signal */
#define INITIAL_WET     (1.0f/SCALE_WET)
/** The initial (percentage of) the dry signal */
#define INITIAL_DRY     0
/** The initial damping value */
#define INITIAL_DAMP    0.5f
/** The initial room size */
#define INITIAL_ROOM    0.5f
/** The initial speaker width (distance from left to right) */
#define INITIAL_WIDTH   1

// These values were obtained by listening tests.
// Note that the values assume 44.1KHz sample rate, they will probably be OK
// for 48KHz sample rate (e.g. iPhone default). However, they would need
// scaling for 96KHz (or other) sample rates.
const int COMB_TUNING[NUM_COMBS] = { 1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617 };
const int ALLP_TUNING[NUM_ALLPS] = { 556, 441, 341, 225 };

#pragma mark -
#pragma mark Reverb Data Structure

/**
 * An algorithmic reverb filter.
 *
 * This is an opaque type defined via {@link ATK_AlgoReverbDef}. It is stateful,
 * in that there is always an unprocessed reverb tail (accessible via the
 * function {@link ATK_DrainAlgoReverb}). Therefore, you should only apply
 * this filter to one audio signal at a time.
 *
 * It is possible to update the reverb settings at any time with a call to
 * {@link ATK_UpdateAlgoReverb}. However, there is no way to extract the
 * current settings for a reverb filter. That is up to the programmer to
 * remember those values.
 */
typedef struct ATK_AlgoReverb {
    /** The number of channels in the output (need not be stereo) */
    Uint32 channels;

    /** Internal gain for producing wet mix */
    float ingain;

    /** Gain scaling for the wet mix (stereo) */
    float wet;
    float wet1;
    float wet2;

    /** Gain scaling for the dry mix */
    float dry;

    /** The amount of feedback for the comb filters (wet tail length)*/
    float roomsize;

    /** The amount that the wet mix is damped */
    float damping;

    /** The distance between left and right channels */
    float width;

    /** The allpass filters */
    ATK_AllpassFilter* allpassesL[NUM_ALLPS];
    ATK_AllpassFilter* allpassesR[NUM_ALLPS];

    /** The comb filters */
    ATK_CombFilter* combsL[NUM_COMBS];
    ATK_CombFilter* combsR[NUM_COMBS];

    /** The lowpass filter */
    ATK_IIRFilter* lowpass;

    /** The input buffer */
    float* inbuffer;
    /** Left output buffer */
    float* outbufferL;
    /** Right output buffer */
    float* outbufferR;
    /** The size of each buffer */
    size_t frames;

} ATK_AlgoReverb;

/**
 * Initializes the algorithmic reverb settings to their defaults.
 *
 * These defaults are the ones chosen by Jezar at Dreampoint, the original
 * FreeVerb author.
 *
 * @param def   The algorithmic reverb settings to initialize
 */
void ATK_AlgoReverbDefaults(ATK_AlgoReverbDef* def) {
    if (def == NULL) {
        return;
    }
    def->wet = INITIAL_WET;
    def->dry = INITIAL_DRY;
    def->width  = INITIAL_WIDTH;
    def->ingain = INITIAL_GAIN;
    def->roomsize = INITIAL_ROOM;
    def->damping  = INITIAL_DAMP;
}

#pragma mark -
#pragma mark Reverb Algorithm


static void gather_input(ATK_AlgoReverb* filter, const float* input, size_t frames) {
    switch (filter->channels) {
    case 1:
        ATK_VecScale(input, filter->ingain, filter->inbuffer, frames);
        break;
    case 2:
        for(size_t ii = 0; ii < frames; ii++) {
            filter->inbuffer[ii] = (input[2*ii]+input[2*ii+1])*filter->ingain/2;
        }
        break;
    default:
        memset(filter->inbuffer,0,sizeof(float)*frames);
        for(size_t ii = 0; ii < frames; ii++) {
            for(size_t jj = 0; jj < filter->channels; jj++) {
                filter->inbuffer[ii] = input[filter->channels*ii+jj]*filter->ingain/filter->channels;
            }
        }
        break;
    }
}

/**
 * Processes a mono input signal, storing the result in output
 *
 * The input signal should already be collapsed into a mono signal in the
 * input buffer.
 *
 * @param filter    The reverb filter
 * @param input     The input signal
 * @param output    The output buffer
 * @param frames    The number of frames to process
 *
 * @return the number of frames processed
 */
static void apply_reverb(ATK_AlgoReverb* filter, size_t frames) {
    if (filter->channels == 1) {
        // Accumulate comb filters in parallel
        memset(filter->outbufferL, 0, frames*sizeof(float));
        for (size_t ii = 0; ii < NUM_COMBS; ii++) {
            ATK_AddCombFilter(filter->combsL[ii], filter->inbuffer, filter->outbufferL, frames);
        }

        // Feed through allpasses in series
        for (size_t ii = 0; ii < NUM_ALLPS; ii++) {
            ATK_ApplyAllpassFilter(filter->allpassesL[ii], filter->outbufferL, filter->outbufferL, frames);
        }
    } else {
        // Accumulate comb filters in parallel
        memset(filter->outbufferL, 0, frames*sizeof(float));
        memset(filter->outbufferR, 0, frames*sizeof(float));
        for (size_t ii = 0; ii < NUM_COMBS; ii++) {
            ATK_AddCombFilter(filter->combsL[ii], filter->inbuffer, filter->outbufferL, frames);
            ATK_AddCombFilter(filter->combsR[ii], filter->inbuffer, filter->outbufferR, frames);
        }

        // Feed through allpasses in series
        for (size_t ii = 0; ii < NUM_ALLPS; ii++) {
            ATK_ApplyAllpassFilter(filter->allpassesL[ii], filter->outbufferL, filter->outbufferL, frames);
            ATK_ApplyAllpassFilter(filter->allpassesR[ii], filter->outbufferR, filter->outbufferR, frames);
        }
    }
}

static void gather_output(ATK_AlgoReverb* filter, const float* input, float* output, size_t frames) {
    switch (filter->channels) {
    case 1:
        for(size_t ii = 0; ii < frames; ii++) {
            output[ii] = filter->outbufferL[ii]*filter->wet + input[ii]*filter->dry;
        }
        break;
    case 2:
        for(size_t ii = 0; ii < frames; ii++) {
            output[2*ii  ] = filter->outbufferL[ii]*filter->wet1 + filter->outbufferR[ii]*filter->wet2 + input[2*ii  ]*filter->dry;
            output[2*ii+1] = filter->outbufferL[ii]*filter->wet2 + filter->outbufferR[ii]*filter->wet1 + input[2*ii+1]*filter->dry;
        }
        break;
    default:
        // Calculate output, replacing everything already in the buffer
        for(size_t ii = 0; ii < frames; ii++) {
            for (size_t jj = 0; jj < filter->channels; jj++) {
                size_t idx = ii*filter->channels + jj;
                if (jj != 2 && jj != 3) {
                    if (jj % 2 == 0) { //L
                        output[idx] = filter->outbufferL[ii]*filter->wet1 + filter->outbufferR[ii]*filter->wet2 + input[idx]*filter->dry;
                    } else { //R
                        output[idx] = filter->outbufferL[ii]*filter->wet2 + filter->outbufferR[ii]*filter->wet1 + input[idx]*filter->dry;
                    }
                } else {
                    output[idx] = (filter->outbufferL[ii]+filter->outbufferR[ii])*(filter->wet/2) + input[idx]*filter->dry;
                }
            }
        }

        ATK_ApplyIIRFilter_stride(filter->lowpass, output+3, filter->channels, output+3, filter->channels, frames);
        break;
    }
}

static void gather_tail(ATK_AlgoReverb* filter, float* output, size_t frames) {
    switch (filter->channels) {
    case 1:
        for(size_t ii = 0; ii < frames; ii++) {
            output[ii] = filter->outbufferL[ii]*filter->wet;
        }
        break;
    case 2:
        for(size_t ii = 0; ii < frames; ii++) {
            output[2*ii  ] = filter->outbufferL[ii]*filter->wet1 + filter->outbufferR[ii]*filter->wet2;
            output[2*ii+1] = filter->outbufferL[ii]*filter->wet2 + filter->outbufferR[ii]*filter->wet1;
        }
        break;
    default:
        // Calculate output, replacing everything already in the buffer
        for(size_t ii = 0; ii < frames; ii++) {
            for (size_t jj = 0; jj < filter->channels; jj++) {
                size_t idx = ii*filter->channels + jj;
                if (jj != 2 && jj != 3) {
                    if (jj % 2 == 0) { //L
                        output[idx] = filter->outbufferL[ii]*filter->wet1 + filter->outbufferR[ii]*filter->wet2;
                    } else { //R
                        output[idx] = filter->outbufferL[ii]*filter->wet2 + filter->outbufferR[ii]*filter->wet1;
                    }
                } else {
                    output[idx] = (filter->outbufferL[ii]+filter->outbufferR[ii])*(filter->wet/2);
                }
            }
        }

        ATK_ApplyIIRFilter_stride(filter->lowpass, output+3, filter->channels, output+3, filter->channels, frames);
        break;
    }
}

#pragma mark -
#pragma mark External API

/**
 * Returns a newly allocated algorithmic reverb filter with the given settings
 *
 * The initialized filter will be padded with zeros, so that the tail is all
 * silence. The settings can be updated at any time with a call to the function
 * {@link ATK_UpdateAlgoReverb}.
 *
 * The value frames is used to allocate the size of the internal buffers. The
 * best performance is achieved when this matches the value provided to
 * {@link ATK_ApplyAlgoReverb}. The number of channels supported by the filter,
 * as well as the sample rate, is fixed at the time of creation.
 *
 * @param def       The algorithmic reverb settings
 * @param rate      The sample rate for the reverb
 * @param channels  The number of channels in an audio frame
 * @param frames    The expected number of frames to process at each step
 *
 * @return a newly allocated algorithmic reverb filter with the given settings
 */
ATK_AlgoReverb* ATK_AllocAlgoReverb(ATK_AlgoReverbDef* def, Uint32 rate,
                                    Uint32 channels, size_t frames) {
    ATK_AlgoReverb* result = (ATK_AlgoReverb*)ATK_malloc(sizeof(ATK_AlgoReverb));
    if (result == NULL) {
        ATK_OutOfMemory();
        return NULL;
    }
    memset(result,0,sizeof(ATK_AlgoReverb));

    result->inbuffer = (float*)ATK_malloc(sizeof(float)*frames);
    if (result->inbuffer == NULL) {
        goto out;
    }
    memset(result->inbuffer,0,sizeof(float)*frames);

    result->outbufferL = (float*)ATK_malloc(sizeof(float)*frames);
    if (result->outbufferL == NULL) {
        goto out;
    }
    memset(result->outbufferL,0,sizeof(float)*frames);

    result->outbufferR = (float*)ATK_malloc(sizeof(float)*frames);
    if (result->outbufferR == NULL) {
        goto out;
    }
    memset(result->outbufferR,0,sizeof(float)*frames);

    for(int ii = 0; ii < NUM_ALLPS; ii++) {
        result->allpassesL[ii] = ATK_AllocAllpassFilter(ALLP_TUNING[ii], 0);
        if (result->allpassesL[ii] == NULL) {
            goto out;
        }
        result->allpassesR[ii] = ATK_AllocAllpassFilter(ALLP_TUNING[ii]+STEREO_SPREAD, 0);
        if (result->allpassesR[ii] == NULL) {
            goto out;
        }
    }

    for(int ii = 0; ii < NUM_COMBS; ii++) {
        result->combsL[ii] = ATK_AllocCombFilter(COMB_TUNING[ii], 0, 0);
        if (result->combsL[ii] == NULL) {
            goto out;
        }
        result->combsR[ii] = ATK_AllocCombFilter(COMB_TUNING[ii]+STEREO_SPREAD, 0, 0);
        if (result->combsR[ii] == NULL) {
            goto out;
        }
    }

    result->lowpass = ATK_AllocFOFilter(ATK_FO_LOWPASS, 120.0f / rate);
    if (result->lowpass == NULL) {
        goto out;
    }

    result->channels = channels;
    ATK_UpdateAlgoReverb(result, def);
    result->frames = frames;
    return result;

out:
    ATK_OutOfMemory();
    if (result->inbuffer) {
        ATK_free(result->inbuffer);
    }
    if (result->outbufferL) {
        ATK_free(result->outbufferL);
    }
    if (result->outbufferR) {
        ATK_free(result->outbufferR);
    }
    if (result->lowpass) {
        ATK_FreeIIRFilter(result->lowpass);
    }
    for(int ii = 0; ii < NUM_ALLPS; ii++) {
        if (result->allpassesL[ii]) {
            ATK_FreeAllpassFilter(result->allpassesL[ii]);
        }
        if (result->allpassesR[ii]) {
            ATK_FreeAllpassFilter(result->allpassesR[ii]);
        }
    }
    for(int ii = 0; ii < NUM_COMBS; ii++) {
        if (result->combsL[ii]) {
            ATK_FreeCombFilter(result->combsL[ii]);
        }
        if (result->combsR[ii]) {
            ATK_FreeCombFilter(result->combsR[ii]);
        }
    }

    return NULL;
}

/**
 * Updates the settings of the given algorithmic reverb
 *
 * These settings can be updated at any time.
 *
 * @param filter    The algorithmic reverb filter
 * @param def       The algorithmic reverb settings
 */
void ATK_UpdateAlgoReverb(ATK_AlgoReverb* filter, ATK_AlgoReverbDef* def) {
    filter->wet = def->wet * SCALE_WET;
    filter->dry = def->dry * SCALE_DRY;
    filter->width = def->width;
    filter->ingain = def->ingain;
    filter->roomsize = (def->roomsize*SCALE_ROOM) + OFFSET_ROOM;
    filter->damping  = def->damping*SCALE_DAMP;

    filter->wet1 = filter->wet * (filter->width / 2 + 0.5f);
    filter->wet2 = filter->wet * ((1 - filter->width) / 2);

    for (int i = 0; i < NUM_COMBS; i++) {
        ATK_UpdateCombFilter(filter->combsL[i],filter->roomsize,filter->damping);
        ATK_UpdateCombFilter(filter->combsR[i],filter->roomsize,filter->damping);
    }
}

/**
 * Frees a previously allocated algorithmic reverb filter
 *
 * @param filter    The algorithmic reverb filter
 */
void ATK_FreeAlgoReverb(ATK_AlgoReverb* filter) {
    if (filter == NULL) {
        return;
    }

    if (filter->inbuffer) {
        ATK_free(filter->inbuffer);
        filter->inbuffer = NULL;
    }
    if (filter->outbufferL) {
        ATK_free(filter->outbufferL);
        filter->outbufferL = NULL;
    }
    if (filter->outbufferR) {
        ATK_free(filter->outbufferR);
        filter->outbufferR = NULL;
    }
    if (filter->lowpass) {
        ATK_FreeIIRFilter(filter->lowpass);
        filter->lowpass = NULL;
    }
    for(int ii = 0; ii < NUM_ALLPS; ii++) {
        if (filter->allpassesL[ii]) {
            ATK_FreeAllpassFilter(filter->allpassesL[ii]);
            filter->allpassesL[ii] = NULL;
        }
        if (filter->allpassesR[ii]) {
            ATK_FreeAllpassFilter(filter->allpassesR[ii]);
            filter->allpassesR[ii] = NULL;
        }
    }
    for(int ii = 0; ii < NUM_COMBS; ii++) {
        if (filter->combsL[ii]) {
            ATK_FreeCombFilter(filter->combsL[ii]);
            filter->combsL[ii] = NULL;
        }
        if (filter->combsR[ii]) {
            ATK_FreeCombFilter(filter->combsR[ii]);
            filter->combsR[ii] = NULL;
        }
    }
    ATK_free(filter);
}

/**
 * Resets an allocated algorithmic reverb filter to its intial state
 *
 * The reverb tail will be zero padded so that it is all silence.
 *
 * @param filter    The algorithmic reverb filter
 */
void ATK_ResetAlgoReverb(ATK_AlgoReverb* filter) {
    if (filter == NULL) {
        return;
    }
    memset(filter->outbufferL,0,sizeof(float)*filter->frames);
    memset(filter->outbufferR,0,sizeof(float)*filter->frames);
    ATK_ResetIIRFilter(filter->lowpass);
    for(int ii = 0; ii < NUM_ALLPS; ii++) {
        ATK_ResetAllpassFilter(filter->allpassesL[ii]);
        ATK_ResetAllpassFilter(filter->allpassesR[ii]);
    }
    for(int ii = 0; ii < NUM_COMBS; ii++) {
        ATK_ResetCombFilter(filter->combsL[ii]);
        ATK_ResetCombFilter(filter->combsR[ii]);
    }

}

/**
 * Applies the algorithmic reverb filter to single audio frame.
 *
 * The buffers input and output should store a single audio frame, and hence
 * they should be the same size as the number of channels supported by this
 * filter. It is safe for input and output to be the same buffer.
 *
 * @param filter    The algorithmic reverb filter
 * @param input     The buffer for the input audio frame
 * @param output    The buffer for the output audio frame
 */
void  ATK_StepAlgoReverb(ATK_AlgoReverb* filter, const float* input, float* output) {
    if (filter == NULL) {
        return;
    }

    float inval = 0;
    switch (filter->channels) {
        case 1:
            inval = filter->ingain*input[0];
            break;
        case 2:
            inval = filter->ingain*(input[0]+input[1]);
            break;
        default:
            for(size_t ii = 0; ii < filter->channels; ii++) {
                inval += filter->ingain*input[ii];
            }
            break;
    }

    float outL = 0;
    float outR = 0;
    if (filter->channels == 1) {
        // Accumulate comb filters in parallel
        for (size_t ii = 0; ii < NUM_COMBS; ii++) {
            outL += ATK_StepCombFilter(filter->combsL[ii], inval);
        }

        // Feed through allpasses in series
        for (size_t ii = 0; ii < NUM_ALLPS; ii++) {
            outL = ATK_StepAllpassFilter(filter->allpassesL[ii], outL);
        }

        output[0] = outL*filter->wet + input[0]*filter->dry;
    } else {
        // Accumulate comb filters in parallel
        for (size_t ii = 0; ii < NUM_COMBS; ii++) {
            outL += ATK_StepCombFilter(filter->combsL[ii], inval);
            outR += ATK_StepCombFilter(filter->combsR[ii], inval);
        }

        // Feed through allpasses in series
        for (size_t ii = 0; ii < NUM_ALLPS; ii++) {
            outL = ATK_StepAllpassFilter(filter->allpassesL[ii], outL);
            outR = ATK_StepAllpassFilter(filter->allpassesL[ii], outR);
        }

        if (filter->channels == 2) {
            output[0] = outL*filter->wet1 + outR*filter->wet2 + input[0]*filter->dry;
            output[1] = outL*filter->wet2 + outR*filter->wet1 + input[1]*filter->dry;
        } else {
            for (size_t ii = 0; ii < filter->channels; ii++) {
                if (ii != 2 && ii != 3) {
                    if (ii % 2 == 0) { //L
                        output[ii] = outL*filter->wet1 + outR*filter->wet2 + input[ii]*filter->dry;
                    } else { //R
                        output[ii] = outL*filter->wet2 + outR*filter->wet1 + input[ii]*filter->dry;
                    }
                } else {
                    output[ii] = (outL+outR)*(filter->wet/2) + input[ii]*filter->dry;
                }
            }

            output[3] = ATK_StepIIRFilter(filter->lowpass, output[3]);
        }
    }
}

/**
 * Applies the algorithmic reverb filter to the given input signal.
 *
 * The input (and output) buffer should have size frames*channels, where
 * channels is the number of channels supported by this filter. The samples
 * for each channel should be interleaved. It is safe for input and output
 * to be the same buffer.
 *
 * @param filter    The algorithmic reverb filter
 * @param input     The buffer for the input
 * @param output    The buffer for the output
 * @param frames    The number of audio frames to process
 */
void ATK_ApplyAlgoReverb(ATK_AlgoReverb* filter,
                         const float* input, float* output,
                         size_t frames) {
    if (filter == NULL) {
        return;
    }

    size_t taken = 0;
    while (taken < frames) {
        if (taken > 0) {
            int x = 2;
        }
        size_t amt = frames-taken < filter->frames ? frames-taken : filter->frames;
        gather_input(filter, input+filter->channels*taken, amt);
        apply_reverb(filter, amt);
        gather_output(filter, input+filter->channels*taken, output+filter->channels*taken, amt);
        taken += amt;
    }
}

/**
 * Drains the contents of the algorithmic reverb filter into the buffer.
 *
 * Even when the input has stopped, there is still some echo left to process.
 * In the case of algorithmic reverb (as opposed to convolutional reverb), this
 * tail can be infinite, especially if the damping is inadequate.  In actual
 * audio system, this tail would be set to automatically fade out over time.
 * However, we separate that from the reverb algorithm, meaning that this
 * function is the same as passing an input of all 0s.
 *
 * The buffer should have size frames*channels, where channels is the number of
 * channels supported by this filter.
 *
 * @param filter    The algorithmic reverb filter
 * @param buffer    The buffer to store the output
 * @param frames    The maximum number of frames to drain
 *
 * @return the number of audio frames stored in buffer
 */
size_t ATK_DrainAlgoReverb(ATK_AlgoReverb* filter, float* buffer, size_t frames) {
    if (filter == NULL) {
        return 0;
    }

    size_t taken = 0;
    while (taken < frames) {
        size_t amt = frames-taken < filter->frames ? frames-taken : filter->frames;
        memset(filter->inbuffer,0,sizeof(float)*amt);
        apply_reverb(filter, amt);
        gather_tail(filter, buffer+filter->channels*taken, amt);
        taken += amt;
    }

    return taken;
}
