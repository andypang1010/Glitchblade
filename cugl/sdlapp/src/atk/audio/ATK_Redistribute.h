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
#include <ATK_audio.h>
/**
 * @file ATK_Redistribute
 *
 * This header provides the redistribution functions for all of the channel
 * combinations supported by SDL. These functions are heavily adapted from SDL
 * 2.26. In particular, we use all of their magic numbers for audio mixing.
 * However, we make one minor change in that, when we have to construct a
 * front channel, we make it the average of the front left and right.
 */

#pragma mark -
#pragma mark Pair Converters
/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be in mono, while output should support a stereo
 * stream with the same number of audio frames. The value size is specified in
 * terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_mono_to_stereo(const float* input, float* output, size_t size) {
    const float *src = (input + size);
    float *dst = (output + (size*2-1));

    for (size_t ii = 0; ii < size; ii++) {
        src--;
        *dst-- = *src;
        *dst-- = *src;
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be in stereo, while output should support a mono
 * stream with the same number of audio frames. The value size is specified in
 * terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_stereo_to_mono(const float* input, float* output, size_t size) {
    const float *src = input;
    float *dst = output;
    for (size_t ii = 0; ii < size; ii++, src += 2) {
        *(dst++) = (src[0] + src[1]) * 0.5f;
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be in stereo, while output should support a
 * 2.1 stream (stereo with subwoofer) for the same number of audio
 * frames. The value size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_stereo_to_21(const float* input, float* output, size_t size) {
    const float *src = (input + (size*2-1));
    float *dst = (output + (size*3-1));

    for (size_t ii = 0; ii < size; ii++) {
        *dst-- = 0.0f;      /* LFE */
        *dst-- = *src--;    /* FR */
        *dst-- = *src--;    /* FL */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be in stereo, while output should support a
 * quadraphonic stream with the same number of audio frames. The value size
 * is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_stereo_to_quad(const float* input, float* output, size_t size) {
    const float *src = (input + (size*2-1));
    float *dst = (output + (size*4-1));

    for (size_t ii = 0; ii < size; ii++) {
        *dst-- = 0.0f;      /* BR */
        *dst-- = 0.0f;      /* BL */
        *dst-- = *src--;    /* FR */
        *dst-- = *src--;    /* FL */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be in stereo, while output should support a
 * 4.1 stream (quadraphonic with subwoofer) with the same number of audio
 * frames. The value size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_stereo_to_41(const float* input, float* output, size_t size) {
    const float *src = (input + (size*2-1));
    float *dst = (output + (size*5-1));

    for (size_t ii = 0; ii < size; ii++) {
        *dst-- = 0.0f;      /* BR */
        *dst-- = 0.0f;      /* BL */
        *dst-- = 0.0f;      /* LFE */
        *dst-- = *src--;    /* FR */
        *dst-- = *src--;    /* FL */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be in stereo, while output should support a 5.1
 * surround stream with the same number of audio frames. The value size is
 * specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_stereo_to_51(const float* input, float* output, size_t size) {
    const float *src = (input + (size*2-1));
    float *dst = (output + (size*6-1));

    for (size_t ii = 0; ii < size; ii++) {
        const float srcFR = *src--;
        const float srcFL = *src--;

        *dst-- = 0.0f;      /* BR */
        *dst-- = 0.0f;      /* BL */
        *dst-- = 0.0f;      /* LFE */
        *dst-- = (srcFL+srcFR)*0.5f;    /* FC */
        *dst-- = srcFR;     /* FR */
        *dst-- = srcFL;     /* FL */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be in stereo, while output should support a 6.1
 * surround stream (collapsed back channel) with the same number of audio
 * frames. The value size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_stereo_to_61(const float* input, float* output, size_t size) {
    const float *src = (input + (size*2-1));
    float *dst = (output + (size*7-1));
    //float lf, rf;

    for (size_t ii = 0; ii < size; ii++) {
        const float srcFR = *src--;
        const float srcFL = *src--;

        *dst-- = 0.0f;      /* SR */
        *dst-- = 0.0f;      /* SL */
        *dst-- = 0.0f;      /* BC */
        *dst-- = 0.0f;      /* LFE */
        *dst-- = (srcFL+srcFR)*0.5f;    /* FC */
        *dst-- = srcFR;     /* FR */
        *dst-- = srcFL;     /* FL */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be 2.1 stream (stereo with subwoofer), while output
 * should support a mono stream with the same number of audio frames. The value
 * size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_21_to_mono(const float* input, float* output, size_t size) {
    const float *src = input;
    float *dst = output;

    /* SDL's 2.1 layout: FL+FR+LFE */
    for (size_t ii = 0; ii < size; ii++, src += 3, dst += 1) {
        dst[0] = (src[0] * 0.333333343f) + (src[1] * 0.333333343f) + (src[2] * 0.333333343f);  /* FC */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be 2.1 stream (stereo with subwoofer), while output
 * should support a stereo stream with the same number of audio frames. The
 * value size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_21_to_stereo(const float* input, float* output, size_t size) {
    const float *src = input;
    float *dst = output;

    /* SDL's 2.1 layout: FL+FR+LFE */
    for (size_t ii = 0; ii < size; ii++, src += 3, dst += 2) {
        const float srcLFE = src[2];
        dst[0] = (src[0] * 0.800000012f) + (srcLFE * 0.200000003f);  /* FL */
        dst[1] = (src[1] * 0.800000012f) + (srcLFE * 0.200000003f);  /* FR */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be 2.1 stream (stereo with subwoofer), while output
 * should support a quadraphonic stream with the same number of audio frames.
 * The value size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_21_to_quad(const float* input, float* output, size_t size) {
    const float *src = (input + (size*3-1));
    float *dst = (output + (size*4-1));

    for (size_t ii = 0; ii < size; ii++) {
        const float srcLFE = *src-- * 0.111111112f;
        *dst-- = srcLFE;                        /* BR */
        *dst-- = srcLFE;                        /* BL */
        *dst-- = (*src--)*0.888888896f+srcLFE;  /* FR */
        *dst-- = (*src--)*0.888888896f+srcLFE;  /* FL */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be 2.1 stream (stereo with subwoofer), while output
 * should support a 4.1 stream (quadraphonic with subwoofer) with the same
 * number of audio frames. The value size is specified in terms of frames, not
 * samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_21_to_41(const float* input, float* output, size_t size) {
    const float *src = (input + (size*3-1));
    float *dst = (output + (size*5-1));

    for (size_t ii = 0; ii < size; ii++) {
        *dst-- = 0.0f;      /* BR */
        *dst-- = 0.0f;      /* BL */
        *dst-- = *src--;    /* LFE */
        *dst-- = *src--;    /* FR */
        *dst-- = *src--;    /* FL */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be 2.1 stream (stereo with subwoofer), while output
 * should support a 5.1 surround stream with the same number of audio frames.
 * The value size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_21_to_51(const float* input, float* output, size_t size) {
    const float *src = (input + (size*3-1));
    float *dst = (output + (size*6-1));

    for (size_t ii = 0; ii < size; ii++) {
        const float srcLFE = *src--;
        const float srcFR = *src--;
        const float srcFL = *src--;

        *dst-- = 0.0f;      /* BR */
        *dst-- = 0.0f;      /* BL */
        *dst-- = srcLFE;    /* LFE */
        *dst-- = (srcFL+srcFR)*0.5f;    /* FC */
        *dst-- = srcFR;     /* FR */
        *dst-- = srcFL;     /* FL */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be 2.1 stream (stereo with subwoofer), while output
 * should support a 6.1 surround stream (collapsed back channel) with the same
 * number of audio frames. The value size is specified in terms of frames, not
 * samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_21_to_61(const float* input, float* output, size_t size) {
    const float *src = (input + (size*3-1));
    float *dst = (output + (size*7-1));

    for (size_t ii = 0; ii < size; ii++) {
        const float srcLFE = *src--;
        const float srcFR = *src--;
        const float srcFL = *src--;

        *dst-- = 0.0f;      /* SR */
        *dst-- = 0.0f;      /* SL */
        *dst-- = 0.0f;      /* BC */
        *dst-- = srcLFE;    /* LFE */
        *dst-- = (srcFL+srcFR)*0.5f;    /* FC */
        *dst-- = srcFR;     /* FR */
        *dst-- = srcFL;     /* FL */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be in quadraphonic format, while output should
 * support a mono stream with the same number of audio frames. The value
 * size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_quad_to_mono(const float* input, float* output, size_t size) {
    const float *src = input;
    float *dst = output;

    for (size_t ii = 0; ii < size; ii++, src += 4, dst += 1) {
        dst[0] = (float)(src[0]*0.25 + src[1]*0.25 + src[2]*0.25 + src[3]*0.25);
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be in quadraphonic format, while output should
 * support a stereo stream with the same number of audio frames. The value
 * size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_quad_to_stereo(const float* input, float* output, size_t size) {
    const float *src = input;
    float *dst = output;

    for (size_t ii = 0; ii < size; ii++, src += 4, dst += 2) {
        const float srcBL = src[2];
        const float srcBR = src[3];
        dst[0] = (src[0] * 0.421000004f) + (srcBL * 0.358999997f) + (srcBR * 0.219999999f); /* FL */
        dst[1] = (src[1] * 0.421000004f) + (srcBL * 0.219999999f) + (srcBR * 0.358999997f); /* FR */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be in quadraphonic format, while output should
 * support a 4.1 stream (quadraphonic with subwoofer) with the same number of
 * audio frames. The value size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_quad_to_41(const float* input, float* output, size_t size) {
    const float *src = (input + (size*4-1));
    float *dst = (output + (size*5-1));

    for (size_t ii = 0; ii < size; ii++) {
        *dst-- = *src--;    /* BR */
        *dst-- = *src--;    /* BL */
        *dst-- = 0.0f;      /* LFE */
        *dst-- = *src--;    /* FR */
        *dst-- = *src--;    /* FL */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be in quadraphonic format, while output should
 * support a 5.1 surround stream with the same number of audio frames. The
 * value size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_quad_to_51(const float* input, float* output, size_t size) {
    const float *src = (input + (size*4-1));
    float *dst = (output + (size*6-1));
    //float lf, rf;

    for (size_t ii = 0; ii < size; ii++) {
        *dst-- = *src--;        /* BR */
        *dst-- = *src--;        /* BL */

        const float srcFR = *src--;
        const float srcFL = *src--;

        *dst-- = 0.0f;          /* LFE */
        *dst-- = (srcFL+srcFR)*0.5f;    /* FC */
        *dst-- = srcFR;         /* FR */
        *dst-- = srcFL;         /* FL */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be in quadraphonic format, while output should
 * support a 6.1 surround stream (collapsed back channel) with the same number
 * of audio frames. The value size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_quad_to_61(const float* input, float* output, size_t size) {
    const float *src = (input + (size*4-1));
    float *dst = (output + (size*7-1));

    for (size_t ii = 0; ii < size; ii++) {
        const float srcBR = *src--;
        const float srcBL = *src--;
        const float srcFR = *src--;
        const float srcFL = *src--;

        *dst-- = (srcBR * 0.796000004f);          /* BR */
        *dst-- = (srcBL * 0.796000004f);          /* BL */
        *dst-- = (srcBR * 0.5f) + (srcBL * 0.5f); /* BC */
        *dst-- = 0.0f;                            /* LFE */
        *dst-- = (srcFL+srcFR)*0.5f;              /* FC */
        *dst-- = srcFR*0.939999998f;              /* FR */
        *dst-- = srcFL*0.939999998f;              /* FL */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 4.1 stream (quadraphonic with subwoofer), while
 * output should support a mono stream with the same number of audio frames.
 * The value size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_41_to_mono(const float* input, float* output, size_t size) {
    const float *src = input;
    float *dst = output;
    const float fact = 0.200000003f;
    for (size_t ii = 0; ii < size; ii++, src += 5, dst += 1) {
        dst[0] = (src[0] * fact) + (src[1] * fact) + (src[2] * fact) + (src[3] * fact) + (src[4] * fact);

    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 4.1 stream (quadraphonic with subwoofer), while
 * output should support a stereo stream with the same number of audio frames.
 * The value size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_41_to_stereo(const float* input, float* output, size_t size) {
    const float *src = input;
    float *dst = output;

    for (size_t ii = 0; ii < size; ii++, src += 5, dst += 2) {
        const float srcLFE = src[2];
        const float srcBL  = src[3];
        const float srcBR  = src[4];
        dst[0] = ((src[0] * 0.374222219f) + (srcLFE * 0.111111112f) +
                  (srcBL * 0.319111109f) + (srcBR * 0.195555553f));     /* FL */
        dst[1] = ((src[1] * 0.374222219f) + (srcLFE * 0.111111112f) +
                  (srcBL * 0.195555553f) + (srcBR * 0.319111109f));     /* FR */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 4.1 stream (quadraphonic with subwoofer), while
 * output should support a 2.1 stream (stereo with subwoofer) with the same
 * number of audio frames. The value size is specified in terms of frames, not
 * samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_41_to_21(const float* input, float* output, size_t size) {
    const float *src = input;
    float *dst = output;

    for (size_t ii = 0; ii < size; ii++, src += 5, dst += 3) {
        const float srcBL = src[3];
        const float srcBR = src[4];
        dst[0] = (src[0] * 0.421000004f) + (srcBL * 0.358999997f) + (srcBR * 0.219999999f); /* FL */
        dst[1] = (src[1] * 0.421000004f) + (srcBL * 0.219999999f) + (srcBR * 0.358999997f); /* FR */
        dst[2] = src[2];    /* LFE */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 4.1 stream (quadraphonic with subwoofer), while
 * output should support a quadraphonic stream with the same number of audio
 * frames. The value size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_41_to_quad(const float* input, float* output, size_t size) {
    const float *src = input;
    float *dst = output;

    for (size_t ii = 0; ii < size; ii++, src += 5, dst += 4) {
        const float srcLFE = src[2];
        dst[0] = (src[0] * 0.941176474f) + (srcLFE * 0.058823530f); /* FL */
        dst[1] = (src[1] * 0.941176474f) + (srcLFE * 0.058823530f); /* FR */
        dst[2] = (srcLFE * 0.058823530f) + (src[3] * 0.941176474f); /* BL */
        dst[3] = (srcLFE * 0.058823530f) + (src[4] * 0.941176474f); /* BR */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 4.1 stream (quadraphonic with subwoofer), while
 * output should support a 5.1 surround stream with the same number of audio
 * frames. The value size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_41_to_51(const float* input, float* output, size_t size) {
    const float *src = (input + (size*5-1));
    float *dst = (output + (size*6-1));
    //float lf, rf;

    for (size_t ii = 0; ii < size; ii++) {
        *dst-- = *src--;            /* BR */
        *dst-- = *src--;            /* BL */
        *dst-- = *src--;            /* LFE */
        const float srcFR = *src--;
        const float srcFL = *src--;

        *dst-- = (srcFL+srcFR)*0.5f;/* FC */
        *dst-- = srcFR;             /* FR */
        *dst-- = srcFL;             /* FL */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 4.1 stream (quadraphonic with subwoofer), while
 * output should support a 6.1 surround stream (collapsed back channel) with
 * the same number of audio frames. The value size is specified in terms of
 * frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_41_to_61(const float* input, float* output, size_t size) {
    const float *src = (input + size*5-1);
    float *dst = (output + size*7-1);

    for (size_t ii = 0; ii < size; ii++) {
        const float srcBR  = *src--;
        const float srcBL  = *src--;
        const float srcLFE = *src--;
        const float srcFR  = *src--;
        const float srcFL  = *src--;

        *dst-- = (srcBR * 0.796000004f);          /* BR */
        *dst-- = (srcBL * 0.796000004f);          /* BL */
        *dst-- = (srcBR * 0.5f) + (srcBL * 0.5f); /* BC */
        *dst-- = srcLFE;                          /* LFE */
        *dst-- = (srcFL+srcFR)*0.5f;              /* FC */
        *dst-- = srcFR;                           /* FR */
        *dst-- = srcFL;                           /* FL */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 5.1 surround stream, while output should
 * support a mono stream with the same number of audio frames. The value size
 * is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_51_to_mono(const float* input, float* output, size_t size) {
    const float *src = input;
    float *dst = output;
    const float fact = 0.166666672f;
    for (size_t ii = 0; ii < size; ii++, src += 6, dst += 1) {
        dst[0] = ((src[0] * fact) + (src[1] * fact) + (src[2] * fact) +
                  (src[3] * fact) + (src[4] * fact) + (src[5] * fact));

    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 5.1 surround stream, while output should
 * support a stereo stream with the same number of audio frames. The value size
 * is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_51_to_stereo(const float* input, float* output, size_t size) {
    const float *src = input;
    float *dst = output;

    for (size_t ii = 0; ii < size; ii++, src += 6, dst += 2) {
        const float srcFC  = src[2] * 0.208181813f;
        const float srcLFE = src[3] * 0.090909094f;
        const float srcBL = src[4];
        const float srcBR = src[5];
        dst[0] = ((src[0] * 0.294545442f) + srcFC + srcLFE +
                  (srcBL * 0.251818180f) + (srcBR * 0.154545456f)); /* FL */
        dst[1] = ((src[1] * 0.294545442f) + srcFC + srcLFE +
                  (srcBL * 0.154545456f) + (srcBR * 0.251818180f)); /* FR */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 5.1 surround stream, while output should
 * support a 2.1 stream (stereo with subwoofer) with the same number of audio
 * frames. The value size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_51_to_21(const float* input, float* output, size_t size) {
    const float *src = input;
    float *dst = output;

    for (size_t ii = 0; ii < size; ii++, src += 6, dst += 3) {
        const float srcFC = src[2];
        const float srcBL = src[4];
        const float srcBR = src[5];
        dst[0] = ((src[0] * 0.324000001f) + (srcFC * 0.229000002f) +
                  (srcBL * 0.277000010f) + (srcBR * 0.170000002f)); /* FL */
        dst[1] = ((src[1] * 0.324000001f) + (srcFC * 0.229000002f) +
                  (srcBL * 0.170000002f) + (srcBR * 0.277000010f)); /* FL */
        dst[2] = src[3];                                            /* LFE */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 5.1 surround stream, while output should
 * support a quadraphonic stream with the same number of audio frames. The
 * value size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_51_to_quad(const float* input, float* output, size_t size) {
    const float *src = input;
    float *dst = output;

    for (size_t ii = 0; ii < size; ii++, src += 6, dst += 4) {
        const float srcFC = src[2] * 0.394285709f;
        const float srcLFE = src[3] * 0.047619049f;
        dst[0] = (src[0] * 0.558095276f) + srcFC + srcLFE;  /* FL */
        dst[1] = (src[1] * 0.558095276f) + srcFC + srcLFE;  /* FR */
        dst[2] = srcLFE + (src[4] * 0.558095276f);          /* BL */
        dst[3] = srcLFE + (src[5] * 0.558095276f);          /* BR */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 5.1 surround stream, while output should
 * support a 4.1 stream (quadraphonic with subwoofer) with the same number of
 * audio frames. The value size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_51_to_41(const float* input, float* output, size_t size) {
    const float *src = input;
    float *dst = output;

    for (size_t ii = 0; ii < size; ii++, src += 6, dst += 5) {
        const float srcFC = src[2];
        dst[0] = (src[0] * 0.586000025f) + (srcFC * 0.414000005f);  /* FL */
        dst[1] = (src[1] * 0.586000025f) + (srcFC * 0.414000005f);  /* FR */
        dst[2] = src[3];    /* LFE */
        dst[3] = (src[4] * 0.586000025f);   /* BL */
        dst[4] = (src[5] * 0.586000025f);   /* BR */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 5.1 surround stream, while output should
 * support a 6.1 surround stream (collapsed back channel) with the same number
 * of audio frames. The value size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_51_to_61(const float* input, float* output, size_t size) {
    const float *src = (input + (size*6-1));
    float *dst = (output + (size*7-1));

    for (size_t ii = 0; ii < size; ii++) {
        const float srcBR = *src--;
        const float srcBL = *src--;

        *dst-- = (srcBR * 0.796000004f);    /* SR */
        *dst-- = (srcBL * 0.796000004f);    /* SL */
        *dst-- = srcBR*0.5f+srcBL*0.5f;     /* BR */
        *dst-- = *src--;                    /* LFE */
        *dst-- = (*src--)*0.939999998f;     /* FC */
        *dst-- = (*src--)*0.939999998f;     /* FR */
        *dst-- = (*src--)*0.939999998f;     /* FL */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 5.1 surround stream, while output should
 * support a 7.1 surround stream with the same number of audio frames. The
 * value size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_51_to_71(const float* input, float* output, size_t size) {
    const float *src = (input + (size*6-1));
    float *dst = (output + (size*8-1));

    for (size_t ii = 0; ii < size; ii++) {
        *dst-- = 0.0f;      /* SR */
        *dst-- = 0.0f;      /* SL */
        *dst-- = *src--;    /* BR */
        *dst-- = *src--;    /* BL */
        *dst-- = *src--;    /* LFE */
        *dst-- = *src--;    /* FC */
        *dst-- = *src--;    /* FR */
        *dst-- = *src--;    /* FL */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 6.1 surround stream (collapsed back channel),
 * while output should support a mono stream with the same number of audio
 * frames. The value size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_61_to_mono(const float* input, float* output, size_t size) {
    const float *src = input;
    float *dst = output;
    const float fact = 0.143142849f;
    for (size_t ii = 0; ii < size; ii++, src += 7, dst += 1) {
        dst[0] = ((src[0] * fact) + (src[1] * fact) + (src[2] * fact) +
                  (src[4] * fact) + (src[5] * fact) + (src[6] * fact));
        dst[0] += src[3]*0.142857149f;
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 6.1 surround stream (collapsed back channel),
 * while output should support a stereo stream with the same number of audio
 * frames. The value size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_61_to_stereo(const float* input, float* output, size_t size) {
    const float *src = input;
    float *dst = output;
    for (size_t ii = 0; ii < size; ii++, src += 7, dst += 2) {
        const float srcFC = src[2];
        const float srcLFE = src[3];
        const float srcBC = src[4];
        const float srcSL = src[5];
        const float srcSR = src[6];
        dst[0] = ((src[0] * 0.247384623f) + (srcFC * 0.174461529f) + (srcLFE * 0.076923080f) +
                  (srcBC * 0.174461529f) + (srcSL * 0.226153851f) + (srcSR * 0.100615382f));    /* FL */
        dst[1] = ((src[1] * 0.247384623f) + (srcFC * 0.174461529f) + (srcLFE * 0.076923080f) +
                  (srcBC * 0.174461529f) + (srcSL * 0.100615382f) + (srcSR * 0.226153851f));    /* FR */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 6.1 surround stream (collapsed back channel),
 * while output should support a 2.1 stream (stereo with subwoofer) with the
 * same number of audio frames. The value size is specified in terms of frames,
 * not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_61_to_21(const float* input, float* output, size_t size) {
    const float *src = input;
    float *dst = output;
    for (size_t ii = 0; ii < size; ii++, src += 7, dst += 3) {
        const float srcFC = src[2];
        const float srcBC = src[4];
        const float srcSL = src[5];
        const float srcSR = src[6];
        dst[0] = ((src[0] * 0.268000007f) + (srcFC * 0.188999996f) + (srcBC * 0.188999996f) +
                  (srcSL * 0.245000005f) + (srcSR * 0.108999997f));     /* FL */
        dst[1] = ((src[1] * 0.268000007f) + (srcFC * 0.188999996f) + (srcBC * 0.188999996f) +
                  (srcSL * 0.108999997f) + (srcSR * 0.245000005f));     /* FR */
        dst[2] = src[3];                                                /* LFE */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 6.1 surround stream (collapsed back channel),
 * while output should support a quadraphonic stream with the same number of
 * audio frames. The value size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_61_to_quad(const float* input, float* output, size_t size) {
    const float *src = input;
    float *dst = output;
    for (size_t ii = 0; ii < size; ii++, src += 7, dst += 4) {
        const float srcFC = src[2];
        const float srcLFE = src[3];
        const float srcBC = src[4];
        const float srcSL = src[5];
        const float srcSR = src[6];
        dst[0] = ((src[0] * 0.463679999f) + (srcFC * 0.327360004f) +
                  (srcLFE * 0.040000003f) + (srcSL * 0.168960005f));                        /* FL */
        dst[1] = ((src[1] * 0.463679999f) + (srcFC * 0.327360004f) +
                  (srcLFE * 0.040000003f) + (srcSR * 0.168960005f));                        /* FR */
        dst[2] = (srcLFE * 0.040000003f) + (srcBC * 0.327360004f) + (srcSL * 0.431039989f); /* BL */
        dst[3] = (srcLFE * 0.040000003f) + (srcBC * 0.327360004f) + (srcSR * 0.431039989f); /* BR */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 6.1 surround stream (collapsed back channel),
 * while output should support a 4.1 stream (quadraphonic with subwoofer) with
 * the same number of audio frames. The value size is specified in terms of
 * frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_61_to_41(const float* input, float* output, size_t size) {
    const float *src = input;
    float *dst = output;
    for (size_t ii = 0; ii < size; ii++, src += 7, dst += 5) {
        const float srcFC = src[2];
        const float srcLFE = src[3];
        const float srcBC = src[4];
        const float srcSL = src[5];
        const float srcSR = src[6];
        dst[0] = (src[0] * 0.483000010f) + (srcFC * 0.340999991f) + (srcSL * 0.175999999f); /* FL */
        dst[1] = (src[1] * 0.483000010f) + (srcFC * 0.340999991f) + (srcSR * 0.175999999f); /* FR */
        dst[2] = src[3];                                                                    /* LFE */
        dst[3] = (srcBC * 0.340999991f) + (srcSL * 0.449000001f);                           /* BL */
        dst[4] = (srcBC * 0.340999991f) + (srcSR * 0.449000001f);                           /* BR */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 6.1 surround stream (collapsed back channel),
 * while output should support a 5.1 surround stream with the same number of
 * audio frames. The value size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_61_to_51(const float* input, float* output, size_t size) {
    const float *src = input;
    float *dst = output;
    for (size_t ii = 0; ii < size; ii++, src += 7, dst += 6) {
        const float srcFC = src[2];
        const float srcLFE = src[3];
        const float srcBC = src[4];
        const float srcSL = src[5];
        const float srcSR = src[6];
        dst[0] = (src[0] * 0.611000001f) + (srcSL * 0.223000005f);  /* FL */
        dst[1] = (src[1] * 0.611000001f) + (srcSR * 0.223000005f);  /* FR */
        dst[2] = (src[2] * 0.611000001f);                           /* FC */
        dst[3] = src[3];                                            /* LFE */
        dst[4] = (srcBC * 0.432000011f) + (srcSL * 0.568000019f);   /* BL */
        dst[5] = (srcBC * 0.432000011f) + (srcSR * 0.568000019f);   /* BR */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 6.1 surround stream (collapsed back channel),
 * while output should support a 7.1 surround stream with the same number of
 * audio frames. The value size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_61_to_71(const float* input, float* output, size_t size) {
    const float *src = (input + (size*7-1));
    float *dst = (output + (size*8-1));

    for (size_t ii = 0; ii < size; ii++) {
        *dst-- = *src--;    /* SR */
        *dst-- = *src--;    /* SL */
        const float srcBC = *src--;
        *dst-- = (srcBC * 0.707000017f);    /* BR */
        *dst-- = (srcBC * 0.707000017f);    /* BL */
        *dst-- = *src--;    /* LFE */
        *dst-- = *src--;    /* FC */
        *dst-- = *src--;    /* FR */
        *dst-- = *src--;    /* FL */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 7.1 surround stream, while output should
 * support a mono stream with the same number of audio frames. The value size
 * is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_71_to_mono(const float* input, float* output, size_t size) {
    const float *src = input;
    float *dst = output;

    for (size_t ii = 0; ii < size; ii++, src += 8, dst += 1) {
        dst[0] = ((src[0] * 0.125125006f) + (src[1] * 0.125125006f) +
                  (src[2] * 0.125125006f) + (src[3] * 0.125000000f) +
                  (src[4] * 0.125125006f) + (src[5] * 0.125125006f) +
                  (src[6] * 0.125125006f) + (src[7] * 0.125125006f));
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 7.1 surround stream, while output should
 * support a stereo stream with the same number of audio frames. The value size
 * is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_71_to_stereo(const float* input, float* output, size_t size) {
    const float *src = input;
    float *dst = output;

    for (size_t ii = 0; ii < size; ii++, src += 8, dst += 2) {
        const float srcFC = src[2];
        const float srcLFE = src[3];
        const float srcBL = src[4];
        const float srcBR = src[5];
        const float srcSL = src[6];
        const float srcSR = src[7];
        dst[0] = ((src[0] * 0.211866662f) + (srcFC * 0.150266662f) + (srcLFE * 0.066666670f) +
                  (srcBL * 0.181066677f) + (srcBR * 0.111066669f) + (srcSL * 0.194133341f) +
                  (srcSR * 0.085866667f));  /* FL */
        dst[1] = ((src[1] * 0.211866662f) + (srcFC * 0.150266662f) + (srcLFE * 0.066666670f) +
                  (srcBL * 0.111066669f) + (srcBR * 0.181066677f) + (srcSL * 0.085866667f) +
                  (srcSR * 0.194133341f));  /* FR */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 7.1 surround stream, while output should
 * support a 2.1 stream (stereo with subwoofer) with the same number of audio
 * frames. The value size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_71_to_21(const float* input, float* output, size_t size) {
    const float *src = input;
    float *dst = output;

    for (size_t ii = 0; ii < size; ii++, src += 8, dst += 3) {
        const float srcFC = src[2];
        const float srcBL = src[4];
        const float srcBR = src[5];
        const float srcSL = src[6];
        const float srcSR = src[7];
        dst[0] = ((src[0] * 0.226999998f) + (srcFC * 0.160999998f) + (srcBL * 0.194000006f) +
                  (srcBR * 0.119000003f) + (srcSL * 0.208000004f) + (srcSR * 0.092000000f));    /* FL */
        dst[1] = ((src[1] * 0.226999998f) + (srcFC * 0.160999998f) + (srcBL * 0.119000003f) +
                  (srcBR * 0.194000006f) + (srcSL * 0.092000000f) + (srcSR * 0.208000004f));    /* FR */
        dst[2] = src[3];    /* LFE */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 7.1 surround stream, while output should
 * support a quadraphonic stream with the same number of audio frames. The
 * value size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_71_to_quad(const float* input, float* output, size_t size) {
    const float *src = input;
    float *dst = output;

    for (size_t ii = 0; ii < size; ii++, src += 8, dst += 4) {
        const float srcFC = src[2];
        const float srcLFE = src[3];
        const float srcSL = src[6];
        const float srcSR = src[7];
        dst[0] = ((src[0] * 0.466344833f) + (srcFC * 0.329241365f) +
                  (srcLFE * 0.034482758f) + (srcSL * 0.169931039f));    /* FL */
        dst[1] = ((src[1] * 0.466344833f) + (srcFC * 0.329241365f) +
                  (srcLFE * 0.034482758f) + (srcSR * 0.169931039f));    /* FR */
        dst[2] = ((srcLFE * 0.034482758f) + (src[4] * 0.466344833f) +
                  (srcSL * 0.433517247f));                              /* BL */
        dst[3] = ((srcLFE * 0.034482758f) + (src[5] * 0.466344833f) +
                  (srcSR * 0.433517247f));                              /* BR */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 7.1 surround stream, while output should
 * support a 4.1 stream (quadraphonic with subwoofer) with the same number of
 * audio frames. The value size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_71_to_41(const float* input, float* output, size_t size) {
    const float *src = input;
    float *dst = output;

    for (size_t ii = 0; ii < size; ii++, src += 8, dst += 5) {
        const float srcFC = src[2];
        const float srcSL = src[6];
        const float srcSR = src[7];
        dst[0] = ((src[0] * 0.483000010f) + (srcFC * 0.340999991f) +
                  (srcSL * 0.175999999f));  /* FL */
        dst[1] = ((src[1] * 0.483000010f) + (srcFC * 0.340999991f) +
                  (srcSR * 0.175999999f));  /* FR */
        dst[2] = src[3];                    /* LFE */
        dst[3] = (src[4] * 0.483000010f) + (srcSL * 0.449000001f);  /* BL */
        dst[4] = (src[5] * 0.483000010f) + (srcSR * 0.449000001f);  /* BR */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 7.1 surround stream, while output should
 * support a 5.1 surround stream with the same number of audio frames. The
 * value size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_71_to_51(const float* input, float* output, size_t size) {
    const float *src = input;
    float *dst = output;

    for (size_t ii = 0; ii < size; ii++, src += 8, dst += 6) {
        const float srcSL = src[6];
        const float srcSR = src[7];
        dst[0] = (src[0] * 0.518000007f) + (srcSL * 0.188999996f);  /* FL */
        dst[1] = (src[1] * 0.518000007f) + (srcSR * 0.188999996f);  /* FR */
        dst[2] = (src[2] * 0.518000007f);   /* FC */
        dst[3] = src[3];                    /* LFE */
        dst[4] = (src[4] * 0.518000007f) + (srcSL * 0.481999993f);  /* BL */
        dst[5] = (src[5] * 0.518000007f) + (srcSR * 0.481999993f);  /* BR */
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 7.1 surround stream, while output should
 * support a 6.1 surround stream (collapsed back channel) with the same number
 * of audio frames. The value size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param size      The number of audio frames to process
 */
static void convert_71_to_61(const float* input, float* output, size_t size) {
    const float *src = input;
    float *dst = output;

    for (size_t ii = 0; ii < size; ii++, src += 8, dst += 7) {
        const float srcBL = src[4];
        const float srcBR = src[5];
        dst[0] = (src[0] * 0.541000009f);   /* FL */
        dst[1] = (src[1] * 0.541000009f);   /* FR */
        dst[2] = (src[2] * 0.541000009f);   /* FC */
        dst[3] = src[3];                    /* LFE */
        dst[4] = (srcBL * 0.287999988f) + (srcBR * 0.287999988f);   /* BC */
        dst[5] = (srcBL * 0.458999991f) + (src[6] * 0.541000009f);  /* SL */
        dst[6] = (srcBR * 0.458999991f) + (src[7] * 0.541000009f);  /* SR */
    }
}

#pragma mark -
#pragma mark Grouped Converters
/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be in mono, while output should be able to hold
 * outchan*size many samples. The value size is specified in terms of frames,
 * not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param outchan   The number of channels in the output buffer
 * @param size      The number of audio frames to process
 */
static void convert_mono(const float* input, float* output, Uint32 outchan, size_t size) {
    switch(outchan) {
    case 1:
        if (output != input) {
            memcpy(output,input,outchan*size*sizeof(float));
        }
        break;
    case 2:
        convert_mono_to_stereo(input,output,size);
        break;
    case 3:
        convert_mono_to_stereo(input,output,size);
        convert_stereo_to_21(output,output,size);
        break;
    case 4:
        convert_mono_to_stereo(input,output,size);
        convert_stereo_to_quad(output,output,size);
        break;
    case 5:
        convert_mono_to_stereo(input,output,size);
        convert_stereo_to_41(output,output,size);
        break;
    case 6:
        convert_mono_to_stereo(input,output,size);
        convert_stereo_to_51(output,output,size);
        break;
    case 7:
        convert_mono_to_stereo(input,output,size);
        convert_stereo_to_61(output,output,size);
        break;
    case 8:
        convert_mono_to_stereo(input,output,size);
        convert_stereo_to_51(output,output,size);
        convert_51_to_71(output,output,size);
        break;
    default:
        ATK_SetError("Nonstandard output width %d requires an explicit matrix.",
                     outchan);
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be in stereo, while output should be able to hold
 * outchan*size many samples. The value size is specified in terms of frames,
 * not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param outchan   The number of channels in the output buffer
 * @param size      The number of audio frames to process
 */
static void convert_stereo(const float* input, float* output, Uint32 outchan, size_t size) {
    switch(outchan) {
    case 1:
        convert_stereo_to_mono(input,output,size);
        break;
    case 2:
        if (output != input) {
            memcpy(output,input,outchan*size*sizeof(float));
        }
        break;
    case 3:
        convert_stereo_to_21(input,output,size);
        break;
    case 4:
        convert_stereo_to_quad(input,output,size);
        break;
    case 5:
        convert_stereo_to_41(input,output,size);
        break;
    case 6:
        convert_stereo_to_51(input,output,size);
        break;
    case 7:
        convert_stereo_to_61(input,output,size);
        break;
    case 8:
        convert_stereo_to_51(input,output,size);
        convert_51_to_71(output,output,size);
        break;
    default:
        ATK_SetError("Nonstandard output width %d requires an explicit matrix.",
                     outchan);
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 2.1 stream (stereo with subwoofer), while
 * output should be able to hold outchan*size many samples. The value size is
 * specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param outchan   The number of channels in the output buffer
 * @param size      The number of audio frames to process
 */
static void convert_21(const float* input, float* output, Uint32 outchan, size_t size) {
    switch(outchan) {
    case 1:
        convert_21_to_mono(input,output,size);
        break;
    case 2:
        convert_21_to_stereo(input,output,size);
        break;
    case 3:
        if (output != input) {
            memcpy(output,input,outchan*size*sizeof(float));
        }
        break;
    case 4:
        convert_21_to_quad(input,output,size);
        break;
    case 5:
        convert_21_to_41(input,output,size);
        break;
    case 6:
        convert_21_to_51(input,output,size);
        break;
    case 7:
        convert_21_to_61(input,output,size);
        break;
    case 8:
        convert_21_to_51(input,output,size);
        convert_51_to_71(output,output,size);
        break;
    default:
        ATK_SetError("Nonstandard output width %d requires an explicit matrix.",
                     outchan);
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a quadraphonic stream, while output should be
 * able to hold outchan*size many samples. The value size is specified in terms
 * of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param outchan   The number of channels in the output buffer
 * @param size      The number of audio frames to process
 */
static void convert_quad(const float* input, float* output, Uint32 outchan, size_t size) {
    switch(outchan) {
    case 1:
        convert_quad_to_mono(input,output,size);
        break;
    case 2:
        convert_quad_to_stereo(input,output,size);
        break;
    case 3:
        convert_quad_to_stereo(input,output,size);
        convert_stereo_to_21(output,output,size);
        break;
    case 4:
        if (output != input) {
            memcpy(output,input,outchan*size*sizeof(float));
        }
        break;
    case 5:
        convert_quad_to_41(input,output,size);
        break;
    case 6:
        convert_quad_to_51(input,output,size);
        break;
    case 7:
        convert_quad_to_61(input,output,size);
        break;
    case 8:
        convert_quad_to_51(input,output,size);
        convert_51_to_71(output,output,size);
        break;
    default:
        ATK_SetError("Nonstandard output width %d requires an explicit matrix.",
                     outchan);
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 4.1 stream (quadraphonic with subwoofer), while
 * output should be able to hold outchan*size many samples. The value size is
 * specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param outchan   The number of channels in the output buffer
 * @param size      The number of audio frames to process
 */
static void convert_41(const float* input, float* output, Uint32 outchan, size_t size) {
    switch(outchan) {
    case 1:
        convert_41_to_mono(input,output,size);
        break;
    case 2:
        convert_41_to_stereo(input,output,size);
        break;
    case 3:
        convert_41_to_21(input,output,size);
        break;
    case 4:
        convert_41_to_quad(input,output,size);
        break;
    case 5:
        if (output != input) {
            memcpy(output,input,outchan*size*sizeof(float));
        }
        break;
    case 6:
        convert_41_to_51(input,output,size);
        break;
    case 7:
        convert_41_to_61(input,output,size);
        break;
    case 8:
        convert_41_to_51(input,output,size);
        convert_51_to_71(output,output,size);
        break;
    default:
        ATK_SetError("Nonstandard output width %d requires an explicit matrix.",
                     outchan);
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 5.1 surround stream, while output should be
 * able to hold outchan*size many samples. The value size is specified in terms
 * of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param outchan   The number of channels in the output buffer
 * @param size      The number of audio frames to process
 */
static void convert_51(const float* input, float* output, Uint32 outchan, size_t size) {
    switch(outchan) {
    case 1:
        convert_51_to_mono(input,output,size);
        break;
    case 2:
        convert_51_to_stereo(input,output,size);
        break;
    case 3:
        convert_51_to_21(input,output,size);
        break;
    case 4:
        convert_51_to_quad(input,output,size);
        break;
    case 5:
        convert_51_to_41(input,output,size);
        break;
    case 6:
        if (output != input) {
            memcpy(output,input,outchan*size*sizeof(float));
        }
        break;
    case 7:
        convert_51_to_61(input,output,size);
        break;
    case 8:
        convert_51_to_71(input,output,size);
        break;
    default:
        ATK_SetError("Nonstandard output width %d requires an explicit matrix.",
                     outchan);
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 6.1 surround stream (collapsed back channel),
 * while output should be able to hold outchan*size many samples. The value
 * size is specified in terms of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param outchan   The number of channels in the output buffer
 * @param size      The number of audio frames to process
 */
static void convert_61(const float* input, float* output, Uint32 outchan, size_t size) {
    switch(outchan) {
    case 1:
        convert_61_to_mono(input,output,size);
        break;
    case 2:
        convert_61_to_stereo(input,output,size);
        break;
    case 3:
        convert_61_to_21(input,output,size);
        break;
    case 4:
        convert_61_to_quad(input,output,size);
        break;
    case 5:
        convert_61_to_41(input,output,size);
        break;
    case 6:
        convert_61_to_51(input,output,size);
        break;
    case 7:
        if (output != input) {
            memcpy(output,input,outchan*size*sizeof(float));
        }
        break;
    case 8:
        convert_61_to_71(input,output,size);
        break;
    default:
        ATK_SetError("Nonstandard output width %d requires an explicit matrix.",
                     outchan);
    }
}

/**
 * Redistribute from buffer input to buffer output
 *
 * The buffer input should be a 7.1 surround stream, while output should be
 * able to hold outchan*size many samples. The value size is specified in terms
 * of frames, not samples.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param outchan   The number of channels in the output buffer
 * @param size      The number of audio frames to process
 */
static void convert_71(const float* input, float* output, Uint32 outchan, size_t size) {
    switch(outchan) {
    case 1:
        convert_71_to_mono(input,output,size);
        break;
    case 2:
        convert_71_to_stereo(input,output,size);
        break;
    case 3:
        convert_71_to_21(input,output,size);
        break;
    case 4:
        convert_71_to_quad(input,output,size);
        break;
    case 5:
        convert_71_to_41(input,output,size);
        break;
    case 6:
        convert_71_to_51(input,output,size);
        break;
    case 7:
        convert_71_to_61(input,output,size);
        break;
    case 8:
        if (output != input) {
            memcpy(output,input,outchan*size*sizeof(float));
        }
        break;
    default:
        ATK_SetError("Nonstandard output width %d requires an explicit matrix.",
                     outchan);
    }
}
