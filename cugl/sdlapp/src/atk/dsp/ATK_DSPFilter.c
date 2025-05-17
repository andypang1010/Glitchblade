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
#include <ATK_dsp.h>
#include <ATK_math.h>
#include <ATK_error.h>
#include <kiss_fft.h>
#include <kiss_fftr.h>

/**
 * @file ATK_DSPFilter
 *
 * This component provides the IIR and FIR filters for SDL_atk. We provide
 * several special purpose first-order and second-order filters, and optimize
 * them. Note that we do not do any vectorization (SSE, AVX) as that is not
 * portable (especially with the rise of ARM) and our experience is that
 * compiling this module on -O2 or -Os outperforms these optimizations. In
 * particularly, we ran experiments on this algorithm for biquad filters:
 *
 *   https://pdfs.semanticscholar.org/d150/a3f75dc033916f14029cd9101a8ea1d050bb.pdf
 *
 * Vectorization was barely a win on mono signals and was a loser when applied
 * to interleaved channels. Instead, we simply write tight loops so that
 * the optimizing compiler can do its job.
 *
 * You will notice that we have separate stride and adjacent versions of each
 * function. This may seem redundant (just set stride 1!). But our experiments
 * have shown that on some platforms there is a slight, but significant
 * performance difference between the two, possibly due to compiler optimization.
 * Instead of trying to identify which functions best benefit from the separation,
 * we just went YOLO and separated them all.
 */

/**
 * Internal state of an IIR filter
 *
 * This type is used for FIR filters as well. The code will optimize for
 * the filter type. Filters are stateful and should be reset whenever they
 * are applied to a new audio signal.
 */
typedef struct ATK_IIRFilter {
    /** The degree (size-1) of the a coefficients */
    size_t adeg;
    /** The degree (size-1) of the b coefficients */
    size_t bdeg;
    /** The a coefficients */
    float* a;
    /** The b coefficients */
    float* b;
    /** The cached inputs; size bdegree */
    float* ins;
    /** The cached outputs; size adegree */
    float* outs;
} ATK_IIRFilter;

#pragma mark -
#pragma mark Optimized Functions
/**
 * Returns the next value of the IIR (infinite impulse response) filter
 *
 * This is the general fallback function for large IIRs.
 *
 * @param value     The input value
 * @param filter    The IIR filter
 *
 * @return the next value of this IIR (infinite impulse response) filter
 */
inline static float tick_iir(float value, ATK_IIRFilter* filter) {
    // Invariants: adeg, bdeg >= 2
    size_t adeg = filter->adeg;
    size_t bdeg = filter->bdeg;

    float result = 0.0;
    for (size_t ii = bdeg; ii > 0; ii--) {
        result += filter->b[ii] * filter->ins[ii-1];
        filter->ins[ii] = filter->ins[ii-1];
    }
    result += filter->b[0] * value;
    filter->ins[0] = value;

    for (size_t ii = adeg; ii > 0; ii--) {
        result += filter->a[ii] * filter->outs[ii-1];
        filter->outs[ii] = filter->outs[ii-1];
    }
    filter->outs[0] = result;
    return result;
}

/**
 * Applies the IIR filter to an input buffer, storing the result in output
 *
 * Both input and output should have the given size. This is the general
 * fallback function for large IIRs.
 *
 * @param src       The input buffer
 * @param dst       The output buffer
 * @param size      The number of elements to process
 * @param filter    The IIR filter
 */
static void fill_iir(const float* src, float* dst, size_t size,
                     ATK_IIRFilter* filter) {
    // Invariants: adeg, bdeg >= 2
    size_t adeg = filter->adeg;
    size_t bdeg = filter->bdeg;

    const float* input = src;
    float* output = dst;

    // Unfortunately, requiring that we be in place hurts optimizations
    float prev;
    while(size--) {
        prev = *input++;
        float result = 0.0;
        for (size_t jj = bdeg; jj > 0; jj--) {
            result += filter->b[jj] * filter->ins[jj-1];
        }
        result += filter->b[0]*prev;
        for (size_t jj = adeg; jj > 0; jj--) {
            result += filter->a[jj] * filter->outs[jj-1];
        }
        *output = result;
        SDL_memmove(filter->ins+1,  filter->ins, bdeg*sizeof(float));
        SDL_memmove(filter->outs+1, filter->outs, adeg*sizeof(float));
        filter->ins[0] = prev;
        filter->outs[0] = *output++;
    }
}

/**
 * Applies the IIR filter to an input buffer, storing the result in output
 *
 * Both input and output should have the given size. This is the general
 * fallback function for large IIRs.
 *
 * @param src       The input buffer
 * @param sstride   The input stride
 * @param dst       The output buffer
 * @param dstride   The output stride
 * @param size      The number of elements to process
 * @param filter    The IIR filter
 */
static void fill_iir_stride(const float* src, size_t sstride,
                            float* dst, size_t dstride, size_t size,
                            ATK_IIRFilter* filter) {
    // Invariants: adeg, bdeg >= 2
    size_t adeg = filter->adeg;
    size_t bdeg = filter->bdeg;

    const float* input = src;
    float* output = dst;

    // Unfortunately, requiring that we be in place hurts optimizations
    float prev;
    while(size--) {
        prev = *input; input += sstride;
        float result = 0.0;
        for (size_t jj = bdeg; jj > 0; jj--) {
            result += filter->b[jj] * filter->ins[jj-1];
        }
        result += filter->b[0]*prev;
        for (size_t jj = adeg; jj > 0; jj--) {
            result += filter->a[jj] * filter->outs[jj-1];
        }
        *output = result;
        SDL_memmove(filter->ins+1,  filter->ins, bdeg*sizeof(float));
        SDL_memmove(filter->outs+1, filter->outs, adeg*sizeof(float));
        filter->ins[0] = prev;
        filter->outs[0] = *output;
        output += dstride;
    }
}

/**
 * Returns the next value of the FIR (finite impulse response) filter
 *
 * This is the general fallback function for large FIRs.
 *
 * @param value     The input value
 * @param filter    The FIR filter
 *
 * @return the next value of this FIR (finite impulse response) filter
 */
inline static float tick_fir(float value, ATK_IIRFilter* filter) {
    // Invariants: asize = 0, bdeg >= 2
    size_t bdeg = filter->bdeg;

    float result = 0.0;
    for (size_t ii = bdeg; ii > 0; ii--) {
        result += filter->b[ii] * filter->ins[ii-1];
        filter->ins[ii] = filter->ins[ii-1];
    }
    result += filter->b[0] * value;
    filter->ins[0] = value;

    return result;
}

/**
 * Applies the FIR filter to an input buffer, storing the result in output
 *
 * Both input and output should have the given size. This is the general
 * fallback function for large FIRs.
 *
 * @param src       The input buffer
 * @param dst       The output buffer
 * @param size      The number of elements to process
 * @param filter    The FIR filter
 */
static void fill_fir(const float* src, float* dst, size_t size,
                     ATK_IIRFilter* filter) {
    // Invariants: asize = 0, bdeg >= 1
    size_t bdeg = filter->bdeg;

    const float* input = src;
    float* output = dst;

    // Unfortunately, requiring that we be in place hurts optimizations
    float prev;
    while(size--) {
        prev = *input++;
        float result = 0.0;
        for (size_t jj = bdeg; jj > 0; jj--) {
            result += filter->b[jj] * filter->ins[jj-1];
        }
        result += filter->b[0]*prev;
        *output++ = result;
        SDL_memmove(filter->ins+1,  filter->ins, bdeg*sizeof(float));
        filter->ins[0] = prev;
    }
}

/**
 * Applies the FIR filter to an input buffer, storing the result in output
 *
 * Both input and output should have the given size. This is the general
 * fallback function for large FIRs.
 *
 * @param src       The input buffer
 * @param sstride   The input stride
 * @param dst       The output buffer
 * @param dstride   The output stride
 * @param size      The number of elements to process
 * @param filter    The FIR filter
 */
static void fill_fir_stride(const float* src, size_t sstride,
                            float* dst, size_t dstride, size_t size,
                            ATK_IIRFilter* filter) {
    // Invariants: asize = 0, bdeg >= 1
    size_t bdeg = filter->bdeg;

    const float* input = src;
    float* output = dst;

    // Unfortunately, requiring that we be in place hurts optimizations
    float prev;
    while(size--) {
        prev = *input; input += sstride;
        float result = 0.0;
        for (size_t jj = bdeg; jj > 0; jj--) {
            result += filter->b[jj] * filter->ins[jj-1];
        }
        result += filter->b[0]*prev;
        *output = result; output += dstride;
        SDL_memmove(filter->ins+1,  filter->ins, bdeg*sizeof(float));
        filter->ins[0] = prev;
    }
}

/**
 * Returns the next value of this one-zero FIR.
 *
 * This implementation is optimized for b-degree 1, a-degree 0.
 *
 * @param value     The input value
 * @param filter    The FIR filter
 *
 * @return the next value of this one-zero FIR.
 */
inline static float tick_one_zero(float value, ATK_IIRFilter* filter) {
    // Invariants: adeg = 0, bdeg = 1
    float result = filter->b[0]*value+filter->b[1]*filter->ins[0];
    filter->ins[0] = value;
    return result;
}

/**
 * Applies the FIR filter to an input buffer, storing the result in output
 *
 * Both input and output should have the given size. This implementation is
 * optimized for b-degree 1, a-degree 0.
 *
 * @param src       The input buffer
 * @param dst       The output buffer
 * @param size      The number of elements to process
 * @param filter    The FIR filter
 */
static void fill_one_zero(const float* src, float* dst, size_t size,
                           ATK_IIRFilter* filter) {
    // Invariants: adeg = 0, bdeg = 1
    if (size == 0) {
        return;
    }

    float b0 = filter->b[0];
    float b1 = filter->b[1];
    const float* input = src;
    float* output = dst;

    int ii = (int)size;

    // Need to do this to be safe in place
    float prev = *input;
    float next;

    *output++ = b0*prev+b1*filter->ins[0];
    while (--ii) {
        next  = b1*prev;
        prev  = *(++input);
        *output++ = b0*prev+next;
    }

    filter->ins[0] = prev;
}

/**
 * Applies the FIR filter to an input buffer, storing the result in output
 *
 * Both input and output should have the given size. This implementation is
 * optimized for b-degree 1, a-degree 0.
 *
 * @param src       The input buffer
 * @param sstride   The input stride
 * @param dst       The output buffer
 * @param dstride   The output stride
 * @param size      The number of elements to process
 * @param filter    The FIR filter
 */
static void fill_one_zero_stride(const float* src, size_t sstride,
                                 float* dst, size_t dstride, size_t size,
                                 ATK_IIRFilter* filter) {
    // Invariants: adeg = 0, bdeg = 1
    if (size == 0) {
        return;
    }

    float b0 = filter->b[0];
    float b1 = filter->b[1];
    const float* input = src;
    float* output = dst;

    int ii = (int)size;

    // Need to do this to be safe in place
    float prev = *input;
    float next;

    *output = b0*prev+b1*filter->ins[0];
    output += dstride;
    while (--ii) {
        next  = b1*prev;
        input += sstride;
        prev  = *input;
        next += b0*prev;
        *output = next;
        output += dstride;
    }

    filter->ins[0] = prev;
}

/**
 * Returns the next value of this one-pole IIR.
 *
 * This implementation is optimized for b-degree 0, a-degree 1.
 *
 * @param value     The input value
 * @param filter    The IIR filter
 *
 * @return the next value of this one-pole IIR.
 */
inline static float tick_one_pole(float value, ATK_IIRFilter* filter) {
    // Invariants: adeg = 1, bdeg = 0
    float result = filter->b[0]*value + filter->a[1]*filter->outs[0];
    filter->outs[0] = result;
    return result;
}

/**
 * Applies the IIR filter to an input buffer, storing the result in output
 *
 * Both input and output should have the given size. This implementation is
 * optimized for b-degree 0, a-degree 1.
 *
 * @param src       The input buffer
 * @param dst       The output buffer
 * @param size      The number of elements to process
 * @param filter    The IIR filter
 */
static void fill_one_pole(const float* src, float* dst, size_t size,
                          ATK_IIRFilter* filter) {
    // Invariants: adeg = 1, bdeg = 0
    if (size == 0) {
        return;
    }

    float b0 = filter->b[0];
    float a1 = filter->a[1];
    const float* input = src;
    float* output = dst;

    int ii = (int)size;

    *output = b0*(*input++)+a1*filter->outs[0];
    while (--ii) {
        float result = a1*(*output++);
        *output = b0*(*input++)+result;
    }

    filter->outs[0] = *output;
}

/**
 * Applies the IIR filter to an input buffer, storing the result in output
 *
 * Both input and output should have the given size. This implementation is
 * optimized for b-degree 0, a-degree 1.
 *
 * @param src       The input buffer
 * @param sstride   The input stride
 * @param dst       The output buffer
 * @param dstride   The output stride
 * @param size      The number of elements to process
 * @param filter    The IIR filter
 */
static void fill_one_pole_stride(const float* src, size_t sstride,
                                 float* dst, size_t dstride, size_t size,
                                 ATK_IIRFilter* filter) {
    // Invariants: adeg = 1, bdeg = 0
    if (size == 0) {
        return;
    }

    float b0 = filter->b[0];
    float a1 = filter->a[1];
    const float* input = src;
    float* output = dst;

    int ii = (int)size;
    *output = b0*(*input)+a1*filter->outs[0];
    while (--ii) {
        input += sstride;
        float result = a1*(*output);
        output += dstride;
        *output = b0*(*input)+result;
    }

    filter->outs[0] = *output;
}

/**
 * Returns the next value of this pole-zero IIR.
 *
 * This implementation is optimized for b-degree and a-degree both 1.
 *
 * @param value     The input value
 * @param filter    The IIR filter
 *
 * @return the next value of this pole-zero IIR.
 */
inline static float tick_pole_zero(float value, ATK_IIRFilter* filter) {
    // Invariants: adeg = 1, bdeg = 1
    float result = filter->b[0]*value+filter->b[1]*filter->ins[0]+filter->a[1]*filter->outs[0];
    filter->outs[0] = result;
    filter->ins[0] = value;
    return result;
}

/**
 * Applies the IIR filter to an input buffer, storing the result in output
 *
 * Both input and output should have the given size. This implementation is
 * optimized for b-degree and a-degree both 1.
 *
 * @param src       The input buffer
 * @param dst       The output buffer
 * @param size      The number of elements to process
 * @param filter    The IIR filter
 */
static void fill_pole_zero(const float* src, float* dst, size_t size,
                           ATK_IIRFilter* filter) {
    // Invariants: adeg = 1, bdeg = 1
    if (size == 0) {
        return;
    }

    float b0 = filter->b[0];
    float b1 = filter->b[1];
    float a1 = filter->a[1];
    const float* input = src;
    float* output = dst;

    int ii = (int)size;

    // Need to do this to be safe in place
    float prev = *input;
    float next;

    *output = b0*prev+b1*filter->ins[0]+a1*filter->outs[0];
    while (--ii) {
        next = b1*prev+a1*(*output++);
        prev = *(++input);
        *output = b0*prev+next;
    }

    filter->ins[0]  = prev;
    filter->outs[0] = *output;
}

/**
 * Applies the IIR filter to an input buffer, storing the result in output
 *
 * Both input and output should have the given size. This implementation is
 * optimized for b-degree and a-degree both 1.
 *
 * @param src       The input buffer
 * @param sstride   The input stride
 * @param dst       The output buffer
 * @param dstride   The output stride
 * @param size      The number of elements to process
 * @param filter    The IIR filter
 */
static void fill_pole_zero_stride(const float* src, size_t sstride,
                                  float* dst, size_t dstride, size_t size,
                                  ATK_IIRFilter* filter) {
    // Invariants: adeg = 1, bdeg = 1
    if (size == 0) {
        return;
    }

    float b0 = filter->b[0];
    float b1 = filter->b[1];
    float a1 = filter->a[1];
    const float* input = src;
    float* output = dst;

    size_t ii = size-1;

    // Need to do this to be safe in place
    float prev = *input;
    float next;

    // Need to do this to be safe in place
    *output = b0*(*input)+b1*filter->ins[0]+a1*filter->outs[0];
    while (ii--) {
        next = b1*prev+a1*(*output);
        input  += sstride;
        output += dstride;
        prev = *input;
        *output = b0*prev+next;
    }

    filter->ins[0]  = prev;
    filter->outs[0] = *output;
}

/**
 * Returns the next value of this two-zero FIR.
 *
 * This implementation is optimized for b-degree 2, a-degree 0.
 *
 * @param value     The input value
 * @param filter    The FIR filter
 *
 * @return the next value of this two-zero FIR.
 */
inline static float tick_two_zero(float value, ATK_IIRFilter* filter) {
    // Invariants: adeg = 0, bdeg = 2
    float result = filter->b[0]*value+filter->b[2]*filter->ins[1]+filter->b[1]*filter->ins[0];
    filter->ins[1] = filter->ins[0];
    filter->ins[0] = value;
    return result;
}

/**
 * Applies the FIR filter to an input buffer, storing the result in output
 *
 * Both input and output should have the given size. This implementation is
 * optimized for b-degree 2, a-degree 0.
 *
 * @param src       The input buffer
 * @param dst       The output buffer
 * @param size      The number of elements to process
 * @param filter    The FIR filter
 */
static void fill_two_zero(const float* src, float* dst, size_t size,
                          ATK_IIRFilter* filter) {
    // Invariants: adeg = 0, bdeg = 2
    if (size == 0) {
        return;
    } else if (size == 1) {
        *dst = tick_two_zero(*src, filter);
        return;
    }

    float b0 = filter->b[0];
    float b1 = filter->b[1];
    float b2 = filter->b[2];

    const float* input = src;
    float* output = dst;

    // Need to do this to be safe in place
    float prev2 = *input++;
    float prev1 = *input++;
    float prev0;

    *output++ = b0*prev2+b1*filter->ins[0]+b2*filter->ins[1];
    *output++ = b0*prev1+b1*prev2+b2*filter->ins[0];
    size -= 2;
    while(size--) {
        prev0 = *input++;
        *output++ = b0*prev0+b1*prev1+b2*prev2;
        prev2 = prev1;
        prev1 = prev0;
    }

    filter->ins[0] = prev1;
    filter->ins[1] = prev2;
}

/**
 * Applies the FIR filter to an input buffer, storing the result in output
 *
 * Both input and output should have the given size. This implementation is
 * optimized for b-degree 2, a-degree 0.
 *
 * @param src       The input buffer
 * @param sstride   The input stride
 * @param dst       The output buffer
 * @param dstride   The output stride
 * @param size      The number of elements to process
 * @param filter    The FIR filter
 */
static void fill_two_zero_stride(const float* src, size_t sstride,
                                 float* dst, size_t dstride, size_t size,
                                 ATK_IIRFilter* filter) {
    // Invariants: adeg = 0, bdeg = 2
    if (size == 0) {
        return;
    } else if (size == 1) {
        *dst = tick_two_zero(*src, filter);
        return;
    }

    float b0 = filter->b[0];
    float b1 = filter->b[1];
    float b2 = filter->b[2];

    const float* input = src;
    float* output = dst;

    // Need to do this to be safe in place
    float prev2 = *input; input += sstride;
    float prev1 = *input; input += sstride;
    float prev0;

    *output = b0*prev2+b1*filter->ins[0]+b2*filter->ins[1];
    output += dstride;
    *output = b0*prev1+b1*prev2+b2*filter->ins[0];
    output += dstride;
    size -= 2;
    while(size--) {
        prev0 = *input;
        input += sstride;
        *output = b0*prev0+b1*prev1+b2*prev2;
        output += dstride;
        prev2 = prev1;
        prev1 = prev0;
    }

    filter->ins[0] = prev1;
    filter->ins[1] = prev2;
}

/**
 * Returns the next value of this two-pole IIR.
 *
 * This implementation is optimized for b-degree 0, a-degree 2.
 *
 * @param value     The input value
 * @param filter    The IIR filter
 *
 * @return the next value of this two-pole IIR.
 */
inline static float tick_two_pole(float value, ATK_IIRFilter* filter) {
    // Invariants: adeg = 2, bdeg = 0
    float result = filter->b[0]*value+filter->a[2]*filter->outs[1]+filter->a[1]*filter->outs[0];
    filter->outs[1] = filter->outs[0];
    filter->outs[0] = result;
    return result;
}

/**
 * Applies the IIR filter to an input buffer, storing the result in output
 *
 * Both input and output should have the given size. This implementation is
 * optimized for b-degree 0, a-degree 2.
 *
 * @param src       The input buffer
 * @param dst       The output buffer
 * @param size      The number of elements to process
 * @param filter    The IIR filter
 */
static void fill_two_pole(const float* src, float* dst, size_t size,
                           ATK_IIRFilter* filter) {
    // Invariants: adeg = 2, bdeg = 0
    if (size == 0) {
        return;
    } else if (size == 1) {
        *dst = tick_two_pole(*src, filter);
        return;
    }

    float b0 = filter->b[0];
    float a1 = filter->a[1];
    float a2 = filter->a[2];

    const float* input = src;
    float* output = dst;

    float next2, next1;

    next2 = b0*(*input++)+a1*filter->outs[0]+a2*filter->outs[1];
    *output++ = next2;
    next1 = b0*(*input++)+a1*next2+a2*filter->outs[0];
    *output++ = next1;
    size -= 2;
    while(size--) {
        *output = b0*(*input++)+a1*next1+a2*next2;
        next2 = next1;
        next1 = *(output++);
    }

    filter->outs[0] = next1;
    filter->outs[1] = next2;
}

/**
 * Applies the IIR filter to an input buffer, storing the result in output
 *
 * Both input and output should have the given size. This implementation is
 * optimized for b-degree 0, a-degree 2.
 *
 * @param src       The input buffer
 * @param sstride   The input stride
 * @param dst       The output buffer
 * @param dstride   The output stride
 * @param size      The number of elements to process
 * @param filter    The IIR filter
 */
static void fill_two_pole_stride(const float* src, size_t sstride,
                                 float* dst, size_t dstride, size_t size,
                                 ATK_IIRFilter* filter) {
    // Invariants: adeg = 2, bdeg = 0
    if (size == 0) {
        return;
    } else if (size == 1) {
        *dst = tick_two_pole(*src, filter);
        return;
    }

    float b0 = filter->b[0];
    float a1 = filter->a[1];
    float a2 = filter->a[2];

    const float* input = src;
    float* output = dst;

    float next2, next1;

    next2 = b0*(*input)+a1*filter->outs[0]+a2*filter->outs[1];
    *output = next2;
    input  += sstride;
    output += dstride;
    next1 = b0*(*input)+a1*next2+a2*filter->outs[0];
    *output = next1;
    input  += sstride;
    output += dstride;
    for(size_t ii = 2; ii < size; ii++) {
        *output = b0*(*input)+a1*next1+a2*next2;
        next2 = next1;
        next1 = *output;
        input  += sstride;
        output += dstride;
    }

    filter->outs[0] = next1;
    filter->outs[1] = next2;
}

/**
 * Returns the next value of this biquad IIR.
 *
 * This implementation is optimized for b-degree and a-degree both 2. This is
 * the most common type of IIR and so we specifically pull it out.
 *
 * @param value     The input value
 * @param filter    The IIR filter
 *
 * @return the next value of this biquad IIR.
 */
inline static float tick_biquad(float value, ATK_IIRFilter* filter) {
    // Invariants: adeg = bdeg = 2
    float result = filter->b[0]*value+filter->b[2]*filter->ins[1]+filter->b[1]*filter->ins[0];
    result += filter->a[2]*filter->outs[1]+filter->a[1]*filter->outs[0];
    filter->ins[1] = filter->ins[0];
    filter->ins[0] = value;
    filter->outs[1] = filter->outs[0];
    filter->outs[0] = result;
    return result;
}

/**
 * Applies the IIR filter to an input buffer, storing the result in output
 *
 * Both input and output should have the given size. This implementation is
 * optimized for b-degree and a-degree both 2. This is the most common type
 * of IIR and so we specifically pull it out.
 *
 * @param src       The input buffer
 * @param dst       The output buffer
 * @param size      The number of elements to process
 * @param filter    The IIR filter
 */
static void fill_biquad(const float* src, float* dst, size_t size,
                        ATK_IIRFilter* filter) {
    // Invariants: adeg = bdeg = 2
    if (size == 0) {
        return;
    } else if (size == 1) {
        *dst = tick_biquad(*src, filter);
        return;
    }

    float b0 = filter->b[0];
    float b1 = filter->b[1];
    float b2 = filter->b[2];
    float a1 = filter->a[1];
    float a2 = filter->a[2];

    const float* input = src;
    float* output = dst;

    // Need to do this to be in place
    float prev2 = *(input++);
    float prev1 = *(input++);
    float prev0;

    float next2, next1;

    float tail;
    tail  = b0*prev2+b1*filter->ins[0]+b2*filter->ins[1];
    next2 = tail+a1*filter->outs[0]+a2*filter->outs[1];
    *output++ = next2;
    tail = b0*prev1+b1*prev2+b2*filter->ins[0];
    next1 = tail+a1*next2+a2*filter->outs[0];
    *output++ = next1;
    size -= 2;
    while(size--) {
        prev0 = *(input++);
        tail = b0*prev0+b1*prev1+b2*prev2;
        *output = tail+a1*next1+a2*next2;
        prev2 = prev1;
        prev1 = prev0;
        next2 = next1;
        next1 = *(output++);
    }

    filter->ins[0] = prev1;
    filter->ins[1] = prev2;
    filter->outs[0] = next1;
    filter->outs[1] = next2;
}

/**
 * Applies the IIR filter to an input buffer, storing the result in output
 *
 * Both input and output should have the given size. This implementation is
 * optimized for b-degree and a-degree both 2. This is the most common type
 * of IIR and so we specifically pull it out.
 *
 * @param src       The input buffer
 * @param sstride   The input stride
 * @param dst       The output buffer
 * @param dstride   The output stride
 * @param size      The number of elements to process
 * @param filter    The IIR filter
 */
static void fill_biquad_stride(const float* src, size_t sstride,
                               float* dst, size_t dstride, size_t size,
                               ATK_IIRFilter* filter) {
    // Invariants: adeg = bdeg = 2
    if (size == 0) {
        return;
    } else if (size == 1) {
        *dst = tick_biquad(*src, filter);
        return;
    }

    float b0 = filter->b[0];
    float b1 = filter->b[1];
    float b2 = filter->b[2];
    float a1 = filter->a[1];
    float a2 = filter->a[2];

    const float* input = src;
    float* output = dst;

    // Need to do this to be in place
    float prev2 = *input; input += sstride;
    float prev1 = *input; input += sstride;
    float prev0;

    float next2, next1;

    float tail;
    tail = b0*prev2+b1*filter->ins[0]+b2*filter->ins[1];
    next2 = tail+a1*filter->outs[0]+a2*filter->outs[1];
    *output = next2; output += dstride;
    tail = b0*prev1+b1*prev2+b2*filter->ins[0];
    next1 = tail+a1*next2+a2*filter->outs[0];
    *output = next1; output += dstride;
    for(size_t ii = 2; ii < size; ii++) {
        prev0 = *input; input += sstride;
        tail = b0*prev0+b1*prev1+b2*prev2;
        *output = tail+a1*next1+a2*next2;
        prev2 = prev1;
        prev1 = prev0;
        next2 = next1;
        next1 = *output; output += dstride;
    }

    filter->ins[0] = prev1;
    filter->ins[1] = prev2;
    filter->outs[0] = next1;
    filter->outs[1] = next2;
}

#pragma mark -
#pragma mark IIR Functions
/**
 * Returns a newly allocated IIR (infinite impulse response) filter
 *
 * The resulting filter implements the standard difference equation:
 *
 *   a[0]*y[n] = b[0]*x[n]+...+b[nb]*x[n-nb]-a[1]*y[n-1]-...-a[na]*y[n-na]
 *
 * If a[0] is not equal to 1, the filter coeffcients are normalized by a[0].
 *
 * The value bsize cannot be 0, as this would make the equation above
 * indeterminate.  However, the value asize can be 0. In that case, the
 * filter is an FIR filter with a[0] = 1.
 *
 * The order of the filter is determined by the size-1. If asize and bsize are
 * both 3, the result is a classic biquad filter. First-order filters have
 * asize, bsize <= 2, while second-order filters have asize, bsize <= 3. Both
 * first-order and second-order filters are optimized for better performance.
 *
 * The filter does NOT acquire ownership of the coefficient arrays a and b.
 * Disposing of the filter will leave these arrays unaffected. A newly allocated
 * filter will zero-pad its inputs for calculation.
 *
 * @param a     The a (feedback) coefficients
 * @param asize The number of a coefficients
 * @param b     The b (feedforward) coefficients
 * @param bsize The number of b coefficients (must be > 0)
 * @param own   Whether to acquire ownership of the coefficients
 *
 * @return a newly allocated IIR (infinite impulse response) filter
 */
ATK_IIRFilter* ATK_AllocIIRFilter(float* a, size_t asize, float* b, size_t bsize) {
    if (bsize == 0) {
        ATK_SetError("Attempt to allocate IIR filter with bsize 0");
        return NULL;
    }

    size_t adeg = asize > 0 ? asize-1 : 0;
    size_t bdeg = bsize > 0 ? bsize-1 : 0;
    if (adeg == 2 && bdeg == 1) {
        bdeg = 2; // Turn it into a biquad
    } else if (bdeg == 2 && adeg == 1) {
        adeg = 2; // Turn it into a biquad
    }


    float a0 = 1.0f;
    float* acoef = NULL;
    float* outs = NULL;
    if (asize > 0) {
        acoef = (float*)ATK_malloc(sizeof(float)*(adeg+1));
        if (acoef == NULL) {
            ATK_OutOfMemory();
            return NULL;
        }
        a0 = a[0];
        for(size_t ii = 0; ii < asize; ii++) {
            acoef[ii] = -a[ii]/a0;
        }
        if (asize < adeg+1) {
            acoef[adeg] = 0.0f;
        }

        if (asize > 1) {
            outs = (float*)ATK_malloc(sizeof(float)*(asize-1));
            if (outs == NULL) {
                ATK_OutOfMemory();
                ATK_free(acoef);
                return NULL;
            }
            memset(outs,0,sizeof(float)*(asize-1));
        }
    } else {
        acoef = (float*)ATK_malloc(sizeof(float));
        *acoef = a0;
    }

    float* bcoef = NULL;
    float* ins = NULL;
    bcoef = (float*)ATK_malloc(sizeof(float)*(bdeg+1));
    if (bcoef == NULL) {
        ATK_OutOfMemory();
        ATK_free(acoef);
        ATK_free(outs);
        return NULL;
    }
    for(size_t ii = 0; ii < bsize; ii++) {
        bcoef[ii] = b[ii]/a0;
    }
    if (bsize < bdeg+1) {
        bcoef[bdeg] = 0.0f;
    }
    if (bsize > 1) {
        ins = (float*)ATK_malloc(sizeof(float)*(bsize-1));
        if (ins == NULL) {
            ATK_OutOfMemory();
            ATK_free(acoef);
            ATK_free(outs);
            ATK_free(bcoef);
            return NULL;
        }
        memset(ins,0,sizeof(float)*(bsize-1));
    }

    ATK_IIRFilter* result = (ATK_IIRFilter*)ATK_malloc(sizeof(ATK_IIRFilter));
    if (result == NULL) {
        ATK_OutOfMemory();
        ATK_free(bcoef);
        ATK_free(acoef);
        ATK_free(outs);
        ATK_free(ins);
        return NULL;
    }

    result->adeg = adeg;
    result->bdeg = bdeg;
    result->a = acoef;
    result->b = bcoef;
    result->ins  = ins;
    result->outs = outs;
    return result;
}


/**
 * Frees a previously allocated IIR (infinite impulse response) filter
 *
 * @param filter    The IIR filter
 */
void ATK_FreeIIRFilter(ATK_IIRFilter* filter) {
    if (filter == NULL) {
        return;
    }

    if (filter->a != NULL) {
        ATK_free(filter->a);
        filter->a = NULL;
    }
    if (filter->b != NULL) {
        ATK_free(filter->b);
        filter->b = NULL;
    }
    if (filter->ins != NULL) {
        ATK_free(filter->ins);
        filter->ins = NULL;
    }
    if (filter->outs != NULL) {
        ATK_free(filter->outs);
        filter->outs = NULL;
    }
    ATK_free(filter);
}

/**
 * Resets the state of a IIR (infinite impulse response) filter
 *
 * IIR filters have to keep state of the inputs they have received so far. This makes
 * it not safe to use a filter on multiple streams simultaneously. Reseting a filter
 * zeroes the state so that it is the same as if the filter were just allocated.
 *
 * @param filter    The IIR filter
 */
void ATK_ResetIIRFilter(ATK_IIRFilter* filter) {
    if (filter == NULL) {
        return;
    }
    if (filter->adeg > 0) {
        memset(filter->outs,0,filter->adeg*sizeof(float));
    }
    if (filter->bdeg > 0) {
        memset(filter->ins,0,filter->bdeg*sizeof(float));
    }
}

/**
 * Returns the next value of the IIR (infinite impulse response) filter
 *
 * IIR filters have to keep state of the inputs they have received so far. This makes
 * it not safe to use a filter on multiple streams simultaneously.
 *
 * @param filter    The IIR filter
 * @param value     The input value
 *
 * @return the next value of this IIR (infinite impulse response) filter
 */
float ATK_StepIIRFilter(ATK_IIRFilter* filter, float value) {
    switch (filter->adeg) {
        case 0:
            switch (filter->bdeg) {
                case 0:
                    return filter->b[0]*value;
                case 1:
                    return tick_one_zero(value, filter);
                case 2:
                    return tick_two_zero(value, filter);
                default:
                    return tick_fir(value, filter);
            }
        case 1:
            switch (filter->bdeg) {
                case 0:
                    return tick_one_pole(value, filter);
                case 1:
                    return tick_pole_zero(value, filter);
                case 2:
                    return tick_biquad(value, filter);
                default:
                    return tick_iir(value, filter);
            }
        case 2:
            switch (filter->bdeg) {
                case 0:
                    return tick_two_pole(value, filter);
                case 1:
                case 2:
                    return tick_biquad(value, filter);
                default:
                    return tick_iir(value, filter);
            }
        default:
            return tick_iir(value, filter);
    }
}

/**
 * Applies the IIR filter to an input buffer, storing the result in output
 *
 * Both input and output should have size len. It is safe for these buffers to be the
 * same. IIR filters have to keep state of the inputs they have received so far. This
 * makes it not safe to use a filter on multiple streams simultaneously.
 *
 * @param filter    The IIR filter
 * @param input     The input buffer
 * @param output    The output buffer
 * @param len       The number of elements in the buffers
 */
void ATK_ApplyIIRFilter(ATK_IIRFilter* filter, const float* input,
                        float* output, size_t len) {
    switch (filter->adeg) {
        case 0:
            switch (filter->bdeg) {
                case 0:
                    ATK_VecScale(input, filter->b[0], output, len);
                    break;
                case 1:
                    fill_one_zero(input, output, len, filter);
                    break;
                case 2:
                    fill_two_zero(input, output, len, filter);
                    break;
                default:
                    fill_fir(input, output, len, filter);
                    break;
            }
            break;
        case 1:
            switch (filter->bdeg) {
                case 0:
                    fill_one_pole(input, output, len, filter);
                    break;
                case 1:
                    fill_pole_zero(input, output, len, filter);
                    break;
                case 2:
                    fill_biquad(input, output, len, filter);
                    break;
                default:
                    fill_iir(input, output, len, filter);
                    break;
            }
            break;
        case 2:
            switch (filter->bdeg) {
                case 0:
                    fill_two_pole(input, output, len, filter);
                    break;
                case 1:
                case 2:
                    fill_biquad(input, output, len, filter);
                    break;
                default:
                    fill_iir(input, output, len, filter);
                    break;
            }
            break;
        default:
            fill_iir(input, output, len, filter);
    }
}

/**
 * Applies the IIR filter to an input buffer, storing the result in output
 *
 * Both input and output should have size len. It is safe for these buffers to be the
 * same assuming that the strides match. IIR filters have to keep state of the inputs
 * they have received so far. This makes it not safe to use a filter on multiple streams \
 * simultaneously.
 *
 * @param filter    The IIR filter
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements in the buffers
 */
void ATK_ApplyIIRFilter_stride(ATK_IIRFilter* filter,
                               const float* input,  size_t istride,
                               float* output, size_t ostride, size_t len) {
    switch (filter->adeg) {
        case 0:
            switch (filter->bdeg) {
                case 0:
                    ATK_VecScale_stride(input, istride, filter->b[0], output, ostride, len);
                    break;
                case 1:
                    fill_one_zero_stride(input, istride, output, ostride, len, filter);
                    break;
                case 2:
                    fill_two_zero_stride(input, istride, output, ostride, len, filter);
                    break;
                default:
                    fill_fir_stride(input, istride, output, ostride, len, filter);
                    break;
            }
            break;
        case 1:
            switch (filter->bdeg) {
                case 0:
                    fill_one_pole_stride(input, istride, output, ostride, len, filter);
                    break;
                case 1:
                    fill_pole_zero_stride(input, istride, output, ostride, len, filter);
                    break;
                case 2:
                    fill_biquad_stride(input, istride, output, ostride, len, filter);
                    break;
                default:
                    fill_iir_stride(input, istride, output, ostride, len, filter);
                    break;
            }
            break;
        case 2:
            switch (filter->bdeg) {
                case 0:
                    fill_two_pole_stride(input, istride, output, ostride, len, filter);
                    break;
                case 1:
                case 2:
                    fill_biquad_stride(input, istride, output, ostride, len, filter);
                    break;
                default:
                    fill_iir_stride(input, istride, output, ostride, len, filter);
                    break;
            }
            break;
        default:
            fill_iir_stride(input, istride, output, ostride, len, filter);
            break;
    }
}

/**
 * Returns a newly allocated first-order filter.
 *
 * First order filters have at most 1 feedback and feedforward coefficient.
 * They typically have a semantic meaning, as defined by {@link ATK_FOFilter}.
 * The parameter value is filter specific.
 *
 * @param type  The filter type
 * @param param The type-specific parameter
 */
ATK_IIRFilter* ATK_AllocFOFilter(ATK_FOFilter type, float param) {
    float a[2];
    float b[2];
    size_t asize = 2;
    size_t bsize = 2;
    a[0] = 1;

    double tmp;
    switch (type) {
        case ATK_FO_LOWPASS:
            tmp = param*M_PI*2.0;
            b[0] = (float)(tmp/(tmp+1));
            a[1] = b[0]-1.0f;
            bsize = 1;
            break;
        case ATK_FO_HIGHPASS:
            tmp = 1.0/(param*M_PI*2+1.0);
            b[0] = (float)tmp;
            b[1] = -b[0];
            a[1] = -b[0];
            break;
        case ATK_FO_ALLPASS:
            if (SDL_fabsf(param) >= 1.0f) {
                ATK_SetError("Allpass parameter %f is out of range",param);
                return NULL;
            }
            b[0] = param;
            b[1] = 1.0;
            a[1] = param;
            break;
        case ATK_DC_BLOCKING:
            if (SDL_fabsf(param) >= 1.0f) {
                ATK_SetError("DC blocking pole %f is out of range",param);
                return NULL;
            }
            b[0] = 1.0f;
            b[1] = -1.0f;
            a[1] = -param;
            break;
        default:
            return NULL;
    }

    return ATK_AllocIIRFilter(a, asize, b, bsize);
}

/**
 * Returns a newly allocated second-order filter.
 *
 * Second order filters have at most 2 feedback and feedforward coefficients
 * each. They are typically represented as biquad filter, where qfactor is
 * the classic biquad quality factor:
 *
 *    https://www.motioncontroltips.com/what-are-biquad-and-other-filter-types-for-servo-tuning
 *
 * For many applications, a Q factor of 1/sqrt(2) (or ATK_Q_VALUE) is sufficient.
 * Specialized filters should compute the Q factor from either {@link ATK_BandwidthQ}
 * or {@link ATK_ShelfSlopeQ}.
 *
 * The gain factor only applies to the parametric equalizer and shelf filters.
 *
 * @param type      The filter type
 * @param freq      The normalized frequency (frequence / sample rate)
 * @param gain      The filter input gain (in decibels)
 * @param qfactor   The biquad quality factor
 *
 * @return a newly allocated second-order filter.
 */
ATK_IIRFilter* ATK_AllocSOFilter(ATK_SOFilter type, float frequency,
                                 float gain, float qfactor) {
    float a[3];
    float b[3];
    size_t asize = 3;
    size_t bsize = 3;
    a[0] = 1;

    double amp = pow(10, SDL_fabs(gain) / 40.0);
    double asq = sqrt(amp);
    double w0  = 2*M_PI*frequency;
    double sinw0 = sin(w0);
    double cosw0 = cos(w0);
    double alpha = sinw0/(2*qfactor);

    // Taken from http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
    switch (type) {
        case ATK_SO_LOWPASS:
            b[0] = (float)((1.0 - cosw0)/2.0);
            b[1] = 2 * b[0];
            b[2] = b[0];
            a[0] = (float)(1 + alpha);
            a[1] = (float)(-2 * cosw0);
            a[2] = (float)(1 - alpha);
            break;

        case ATK_SO_HIGHPASS:
            b[0] = (float)((1.0 + cosw0)/2.0);
            b[1] = -2 * b[0];
            b[2] = b[0];
            a[0] = (float)(1 + alpha);
            a[1] = (float)(-2 * cosw0);
            a[2] = (float)(1 - alpha);
            break;

        case ATK_SO_ALLPASS:
            b[0] = (float)(1 - alpha);
            b[1] = (float)(-2 * cosw0);
            b[2] = (float)(1 + alpha);
            a[0] = (float)(1 + alpha);
            a[1] = (float)(-2 * cosw0);
            a[2] = (float)(1 - alpha);
            break;

        case ATK_SO_BANDPASS:
            b[0] = (float)(alpha);
            b[1] = 0;
            b[2] = -b[0];
            a[0] = (float)(1 + alpha);
            a[1] = (float)(-2 * cosw0);
            a[2] = (float)(1 - alpha);
            break;

        case ATK_SO_NOTCH:
            b[0] = 1.0;
            b[1] = (float)(-2 * cosw0);
            b[2] = b[0];
            a[0] = (float)(1 + alpha);
            a[1] = (float)(-2 * cosw0);
            a[2] = (float)(1 - alpha);
            break;

        case ATK_SO_PEAK:
            b[0] = (float)(1.0 + alpha*amp);
            b[1] = (float)(-2 * cosw0);
            b[2] = (float)(1.0 - alpha*amp);
            a[0] = (float)(1.0 + alpha*amp);
            a[1] = (float)(-2 * cosw0);
            a[2] = (float)(1.0 - alpha*amp);
            break;

        case ATK_SO_LOWSHELF:
            b[0] = (float)(  amp*( (amp+1) - (amp-1)*cosw0 + 2*asq*alpha ));
            b[1] = (float)(2*amp*( (amp-1) - (amp+1)*cosw0               ));
            b[2] = (float)(  amp*( (amp+1) - (amp-1)*cosw0 - 2*asq*alpha ));
            a[0] = (float)(      ( (amp+1) + (amp-1)*cosw0 + 2*asq*alpha ));
            a[1] = (float)( -2.0*( (amp-1) + (amp+1)*cosw0               ));
            a[2] = (float)(      ( (amp+1) + (amp-1)*cosw0 - 2*asq*alpha ));
            break;

        case ATK_SO_HIGHSHELF:
            b[0] = (float)(   amp*( (amp+1) - (amp-1)*cosw0 + 2*asq*alpha ));
            b[1] = (float)(-2*amp*( (amp-1) - (amp+1)*cosw0               ));
            b[2] = (float)(   amp*( (amp+1) - (amp-1)*cosw0 - 2*asq*alpha ));
            a[0] = (float)(       ( (amp+1) + (amp-1)*cosw0 + 2*asq*alpha ));
            a[1] = (float)(   2.0*( (amp-1) + (amp+1)*cosw0               ));
            a[2] = (float)(       ( (amp+1) + (amp-1)*cosw0 - 2*asq*alpha ));
            break;

        case ATK_SO_RESONANCE:
            // Taken from STK
            if (frequency < 0.0 || frequency > 0.5) {
                ATK_SetError("Normalized frequency %f out of range for resonance",frequency);
                return NULL;
            }
            b[0] = (float)(0.5 - 0.5 * qfactor * qfactor);
            b[1] = 0.0;
            b[2] = -b[1];
            a[1] = (float)(-2.0 * qfactor * cosw0);
            a[2] = (float)(qfactor * qfactor);
            break;

        default:
            break;
    }

    return ATK_AllocIIRFilter(a, asize, b, bsize);
}

#pragma mark -
#pragma mark Delay Filter
/**
 * A long running integral delay filter
 *
 * This filter requires a buffer the size of the delay. Thsi value represents
 * the maximum delay. However, it is possible to use this filter to apply
 * any delay up to its maximum value.
 */
typedef struct ATK_DelayFilter {
    /** The maximum delay */
    size_t delay;
    /** The current output position */
    size_t tail;
    /** The delay buffer */
    float* buffer;
} ATK_DelayFilter;

/**
 * Returns a newly allocated delay filter.
 *
 * The filter starts off zero padded, so that all results of the filter are
 * zero until the delay is reached.
 *
 * The delay specified is the maxium delay length. It is possible to have smaller
 * delays using {@link ATK_TapOutDelayFilter} or {@link ATK_TapApplyDelayFilter}.
 *
 * @param delay     The maximum delay length
 *
 * @return a newly allocated delay filter.
 */
ATK_DelayFilter* ATK_AllocDelayFilter(size_t delay) {
    float* buffer = ATK_malloc(sizeof(float)*delay);
    if (buffer == NULL) {
        ATK_OutOfMemory();
        return NULL;
    }
    ATK_DelayFilter* result = ATK_malloc(sizeof(ATK_DelayFilter));
    if (result == NULL) {
        ATK_OutOfMemory();
        ATK_free(buffer);
        return NULL;
    }
    memset(buffer,0,sizeof(float)*delay);
    result->buffer = buffer;
    result->delay = delay;
    result->tail = 0;
    return result;
}

/**
 * Frees a previously allocated delay filter.
 *
 * @param filter    The delay filter
 */
void ATK_FreeDelayFilter(ATK_DelayFilter* filter) {
    if (filter == NULL) {
        return;
    }
    ATK_free(filter->buffer);
    filter->buffer = NULL;
    ATK_free(filter);
}

/**
 * Resets a delay filter to its initial state.
 *
 * The filter buffer will be zeroed, so that no data is stored in the filter.
 *
 * @param filter    The delay filter
 */
void ATK_ResetDelayFilter(ATK_DelayFilter* filter) {
    if (filter == NULL) {
        return;
    }
    memset(filter->buffer,0,sizeof(float)*filter->delay);
}

/**
 * Returns the maximum delay supported by this filter.
 *
 * @param filter    The delay filter
 *
 * @return the maximum delay supported by this filter.
 */
size_t ATK_GetDelayFilterMaximum(ATK_DelayFilter* filter) {
    return filter == NULL ? 0 : filter->delay;
}

/**
 * Returns the next value of the delay filter.
 *
 * The value returned will have maximum delay. Delay filters have to keep state of the
 * inputs they have received so far, so this function moves the filter forward. This makes
 * it not safe to use a filter on multiple streams simultaneously.
 *
 * @param filter    The delay filter
 * @param value     The input value
 *
 * @return the next value of the delay filter.
 */
float ATK_StepDelayFilter(ATK_DelayFilter* filter, float value) {
    float* data = filter->buffer+filter->tail;
    float out = *data;
    *data = value;
    filter->tail = ((filter->tail+1) % filter->delay);
    return out;
}

/**
 * Returns the value in this filter with the given tap position.
 *
 * The value tap should be less than the maximum delay. This function does not
 * modify the filter or move it forward (e.g. the state is unchanged).
 *
 * @param filter    The delay filter
 * @param tap       The tap position
 *
 * @return the value in this filter with the given tap position.
 */
float ATK_TapOutDelayFilter(ATK_DelayFilter* filter, size_t tap) {
    if (tap > filter->delay) {
        ATK_SetError("Tap %zu exceeds delay %zu",tap,filter->delay);
        return 0;
    }
    size_t pos = ((filter->tail+tap) % filter->delay);
    return filter->buffer[pos];
}

/**
 * Sets the filter tap position to have the given value.
 *
 * The value tap should be less than the maximum delay. This function does modify
 * the filter at the given position, but does not move it forward (so calls to
 * {@link ATK_StepDelayFilter} are unaffected if this tap is not at the end).
 *
 * @param filter    The delay filter
 * @param tap       The tap position
 * @param value     The value for the tap position
 *
 * @return the value in this filter with the given tap position.
 */
void ATK_TapInDelayFilter(ATK_DelayFilter* filter, size_t tap, float value) {
    if (tap > filter->delay) {
        ATK_SetError("Tap %zu exceeds delay %zu",tap,filter->delay);
        return;
    }
    size_t pos = ((filter->tail+tap) % filter->delay);
    filter->buffer[pos] = value;
}

/**
 * Applies the delay to an input buffer, storing the result in output
 *
 * The values stored in output will have maximum delay. Both input and output should have
 * size len. It is safe for these two buffers to be the same.
 *
 * Delay filters have to keep state of the inputs they have received so far, so this function
 * moves the filter forward by the given length. This makes it not safe to use a filter on
 * multiple streams simultaneously.
 *
 * @param filter    The delay filter
 * @param input     The input buffer
 * @param output    The output buffer
 * @param len       The number of elements in the buffers
 */
void ATK_ApplyDelayFilter(ATK_DelayFilter* filter, const float* input,
                          float* output, size_t len) {
    const float* src = input;
    float* dst = output;
    float* flt = filter->buffer+filter->tail;
    float* end = filter->buffer+filter->delay;
    float curr;
    while (len--) {
        curr = *flt;
        *flt = *src++;
        *dst++ = curr;
        flt++;
        if (flt == end) {
            flt = filter->buffer;
        }
    }
    filter->tail = flt-filter->buffer;
}

/**
 * Applies the delay to an input buffer, storing the result in output
 *
 * The values stored in output will have maximum delay. Both input and output should have
 * size len. It is safe for these two buffers to be the same provided that the strides match.
 *
 * Delay filters have to keep state of the inputs they have received so far, so this function
 * moves the filter forward by the given length. This makes it not safe to use a filter on
 * multiple streams simultaneously.
 *
 * @param filter    The delay filter
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements in the buffers
 */
void ATK_ApplyDelayFilter_stride(ATK_DelayFilter* filter,
                                 const float* input,  size_t istride,
                                 float* output, size_t ostride, size_t len) {
    const float* src = input;
    float* dst = output;
    float* flt = filter->buffer+filter->tail;
    float* end = filter->buffer+filter->delay;
    float curr;
    while (len--) {
        curr = *flt;
        *flt = *src;
        *dst = curr;
        src += istride;
        dst += ostride;
        flt++;
        if (flt == end) {
            flt = filter->buffer;
        }
    }
    filter->tail = flt-filter->buffer;
}

/**
 * Applies a tapped delay to an input buffer, storing the result in output
 *
 * The values stored in output will be delayed by the given tap. Both input and output should
 * have size len. It is safe for these two buffers to be the same.
 *
 * Delay filters have to keep state of the inputs they have received so far, so this function
 * moves the filter forward by the given length. This means that the last len delayed values
 * will be lost if tap is less than the maximum delay. If you want to have a delay less than
 * the maximum delay without losing state, you should use {@link ATK_TapOutDelayFilter}.
 *
 * @param filter    The delay filter
 * @param input     The input buffer
 * @param output    The output buffer
 * @param tap       The delay tap position
 * @param len       The number of elements in the buffers
 */
void ATK_TapApplyDelayFilter(ATK_DelayFilter* filter, const float* input,
                             float* output, size_t tap, size_t len) {
    if (tap > filter->delay) {
        ATK_SetError("Tap %zu exceeds delay %zu",tap,filter->delay);
        return;
    } else if (tap == filter->delay) {
        ATK_ApplyDelayFilter(filter, input, output, len);
        return;
    }
    const float* src = input;
    float* dst = output;
    size_t pos = ((filter->tail+tap) % filter->delay);
    float* fin  = filter->buffer+pos;
    float* fout = filter->buffer+filter->tail;
    float* end = filter->buffer+filter->delay;
    while (len--) {
        *fin++ = *src++; // Does not work if tap == max delay
        *dst++ = *fout++;
        if (fout == end) {
            fout = filter->buffer;
        }
        if (fin == end) {
            fin = filter->buffer;
        }
    }
    filter->tail = fin-filter->buffer;
}

/**
 * Applies a tapped delay to an input buffer, storing the result in output
 *
 * The values stored in output will have maximum delay. Both input and output should have
 * size len. It is safe for these two buffers to be the same provided that the strides match.
 *
 * Delay filters have to keep state of the inputs they have received so far, so this function
 * moves the filter forward by the given length. This means that the last len delayed values
 * will be lost if tap is less than the maximum delay. If you want to have a delay less than
 * the maximum delay without losing state, you should use {@link ATK_TapOutDelayFilter}.
 *
 * @param filter    The delay filter
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param tap       The delay tap position
 * @param len       The number of elements in the buffers
 */
void ATK_TapApplyDelayFilter_stride(ATK_DelayFilter* filter,
                                    const float* input,  size_t istride,
                                    float* output, size_t ostride,
                                    size_t tap, size_t len)  {
    if (tap > filter->delay) {
        ATK_SetError("Tap %zu exceeds delay %zu",tap,filter->delay);
        return;
    } else if (tap == filter->delay) {
        ATK_ApplyDelayFilter_stride(filter, input, istride, output, ostride, len);
        return;
    }
    const float* src = input;
    float* dst = output;
    size_t pos = ((filter->tail+tap) % filter->delay);
    float* fin  = filter->buffer+pos;
    float* fout = filter->buffer+filter->tail;
    float* end = filter->buffer+filter->delay;
    while (len--) {
        *fin++ = *src; // Does not work if tap == max delay
        *dst = *fout++;
        src += istride;
        dst += ostride;
        if (fout == end) {
            fout = filter->buffer;
        }
        if (fin == end) {
            fin = filter->buffer;
        }
    }
    filter->tail = fin-filter->buffer;
}

#pragma mark -
#pragma mark Fractional Delay Filter
/**
 * A fractional delay filter.
 *
 * Fractional delay filters can be computed using either linear or allpass
 * interpolation. Linear interpolation is efficient but it does introduce
 * high-frequency signal attenuation. Allpass interpolation has unity magnitude
 * gain but variable phase delay properties, making it useful in achieving
 * fractional delays without affecting a signal's frequency magnitude response.
 * It is possible to switch between the two algorithms on the fly. Both are
 * taken from STK by Perry R. Cook and Gary P. Scavone, 1995--2021.
 *
 *     https://github.com/thestk/stk
 *
 * As with a normal {@link ATK_DelayFilter} the filter delay represents the
 * maximum delay. It is possible to use this filter to apply any delay up to
 * its maximum value. However, due to state limitations, any tap uses linear
 * interpolation.
 */
typedef struct ATK_FractionalFilter {
    /** The buffer capacity */
    size_t capacity;
    /** The current input position */
    size_t head;
    /** The current output position */
    size_t tail;
    /** The integral delay */
    float delay;
    /** The delay buffer */
    float* buffer;
    /** Whether to use allpass interpolation */
    int allpass;
    /** The (linear) interpolation alpha */
    double alpha;
    /** Either 1-alpha (linear) or the allpass coefficient */
    double beta;
    /** The last value produced by this filter */
    double last;
} ATK_FractionalFilter;

/**
 * Returns a newly allocated fractional delay filter.
 *
 * The filter starts off zero padded, so that all results of the filter are
 * zero until the delay is reached. The delay cannot be less than 0.5.
 *
 * If allpass is non-zero, this filter will use allpass interpolation. Allpass
 * interpolation has unity magnitude gain but variable phase delay properties,
 * making it useful in achieving fractional delays without affecting a signal's
 * frequency magnitude response. Otherwise, it will use linear interpolation,
 * which is efficient but does introduce high-frequency signal attenuation.
 *
 * The delay specified is the maxium delay length. It is possible to have
 * smaller fractional delays using {@link ATK_TapOutFractionalFilter} or
 * {@link ATK_TapApplyFractionalFilter}. Not that all taps must use linear
 * interpolation, regardless of the filter type.
 *
 * @param delay     The maximum delay length
 * @param allpass   Whether to use allpass interpolation
 *
 * @return a newly allocated fractional delay filter.
 */
ATK_FractionalFilter* ATK_AllocFractionalFilter(float delay, int allpass) {
    if (delay < 0.5) {
        ATK_SetError("Fractional delay is %f which is < 0.5",delay);
        return NULL;
    }

    size_t capacity = (size_t)delay+2;
    float* buffer = ATK_malloc(sizeof(float)*capacity);
    if (buffer == NULL) {
        ATK_OutOfMemory();
        return NULL;
    }

    ATK_FractionalFilter* result = ATK_malloc(sizeof(ATK_FractionalFilter));
    if (result == NULL) {
        ATK_OutOfMemory();
        ATK_free(buffer);
        return NULL;
    }
    memset(buffer,0,sizeof(float)*capacity);

    result->capacity = capacity;
    result->buffer = buffer;
    result->delay = delay;
    result->last = 0.0;
    result->head = 0;
    double overspill = capacity-delay;
    size_t index = (size_t)overspill;
    if (allpass) {
        result->allpass = 1;
        result->head = 0;

        // For linear interpolation
        result->alpha = overspill - index;

        double alpha = 1.0 + index - overspill;
        if (alpha < 0.5) {
            index += 1;
            if (index >= capacity) {
                index -= capacity;
            }
            alpha += 1;
        } else if (index == capacity) {
            index = 0;
        }
        result->tail  = index;
        result->beta  = (1.0 - alpha) / (1.0 + alpha);
    } else {
        result->allpass = 0;
        double alpha = overspill - index;
        if (index == capacity) {
            index = 0;
        }
        result->alpha = alpha;
        result->tail  = index;
        result->beta  = 1.0-alpha;
    }

    return result;
}

/**
 * Frees a previously allocated fractional delay filter.
 *
 * @param filter    The fractional delay filter
 */
void ATK_FreeFractionalFilter(ATK_FractionalFilter* filter) {
    if (filter == NULL) {
        return;
    }
    ATK_free(filter->buffer);
    filter->buffer = NULL;
    ATK_free(filter);
}

/**
 * Resets a fractional delay filter to its initial state.
 *
 * The filter buffer will be zeroed, so that no data is stored in the filter.
 *
 * @param filter    The fractional delay filter
 */
void ATK_ResetFractionalFilter(ATK_FractionalFilter* filter) {
    if (filter == NULL) {
        return;
    }
    memset(filter->buffer,0,sizeof(float)*filter->capacity);
    filter->head = 0;

    // Need to reset the tail
    double overspill = filter->capacity-filter->delay;
    size_t index = (size_t)overspill;
    if (filter->allpass) {
        double alpha = 1.0 + index - overspill;
        if (alpha < 0.5) {
            index += 1;
            if (index >= filter->capacity) {
                index -= filter->capacity;
            }
        } else if (index == filter->capacity) {
            index = 0;
        }
        filter->tail = index;
        filter->last = 0.0;
    } else {
        double alpha = overspill - index;
        if (index == filter->capacity) {
            index = 0;
        }
        filter->tail = index;
    }

}

/**
 * Returns the maximum delay supported by this filter.
 *
 * @param filter    The fractional delay filter
 *
 * @return the maximum delay supported by this filter.
 */
float ATK_GetFractionalFilterDelay(ATK_FractionalFilter* filter) {
    if (filter == NULL) {
        return 0;
    }
    return filter->delay;
}

/**
 * Returns the next value of the delay filter.
 *
 * The value returned will have maximum delay. Fractional delay filters have
 * to keep state of the inputs they have received so far, so this function
 * moves the filter forward. This makes it not safe to use a filter on multiple
 * streams simultaneously.
 *
 * This function will use allpass interpolation if the filter was allocated with
 * that option.
 *
 * @param filter    The fractional delay filter
 * @param value     The input value
 *
 * @return the next value of the delay filter.
 */
float ATK_StepFractionalFilter(ATK_FractionalFilter* filter, float value) {
    float* fin  = filter->buffer+filter->head;
    float* fout = filter->buffer+filter->tail;
    float* end  = filter->buffer+filter->capacity;
    float output;
    if (filter->allpass) {
        *fin++ = value;
        output  = (float)(-filter->beta * filter->last);
        output += *fout++;
        if (fout == end) {
            fout = filter->buffer;
        }
        output += (float)(filter->beta * *fout);
    } else {
        *fin++ = value;
        // First 1/2 of interpolation
        output = (float)(*fout++ * filter->beta);
        if (fout == end) {
            fout = filter->buffer;
        }
        output += (float)(*fout * filter->alpha);
    }
    if (fin == end) {
        fin = filter->buffer;
    }
    filter->last = output;
    filter->head = fin-filter->buffer;
    filter->tail = fout-filter->buffer;
    return output;
}

/**
 * Returns the value in this filter with the given tap position.
 *
 * The value tap should be less than the maximum delay. This function does
 * not modify the filter or move it forward (e.g. the state is unchanged). Note
 * that all tapped outputs must use linear interpolation.
 *
 * @param filter    The fractional delay filter
 * @param tap       The tap position
 *
 * @return the value in this filter with the given tap position.
 */
float ATK_TapOutFractionalFilter(ATK_FractionalFilter* filter, float tap) {
    if (tap < 0 || tap > filter->delay) {
        ATK_SetError("Tap %f exceeds delay %f",tap,filter->delay);
        return 0;
    }
    float offset = filter->head - tap;
    if (offset < 0) {
        offset += filter->capacity;
    }
    size_t index = (size_t)offset;
    double alpha = offset-index;
    if (index == filter->capacity) {
        index = 0;
    }

    float* fout = filter->buffer+index;
    float* end  = filter->buffer+filter->capacity;

    float output = (float)(*fout++ * (1-alpha));
    if (fout == end) {
        fout = filter->buffer;
    }
    output += (float)(*fout * alpha);
    return output;
}

/**
 * Sets the filter tap position to have the given value.
 *
 * The value tap should be less than the maximum delay. This function does
 * modify the filter at the given position, but does not move it forward
 * (so calls to {@link ATK_StepFractionalFilter} are unaffected if this tap
 * is not at the end). Note that even though the delay is fractional, input
 * taps must be integral.
 *
 * @param filter    The fractional delay filter
 * @param tap       The tap position
 * @param value     The value for the tap position
 *
 * @return the value in this filter with the given tap position.
 */
void ATK_TapInFractionalFilter(ATK_FractionalFilter* filter, size_t tap, float value) {
    if (tap > filter->delay) {
        ATK_SetError("Tap %zu exceeds delay %f",tap,filter->delay);
        return;
    }
    size_t pos = (tap > filter->head ? filter->capacity+filter->head-tap : filter->head-tap);
    filter->buffer[pos] = value;
}

/**
 * Applies the delay to an input buffer, storing the result in output
 *
 * The values stored in output will have maximum delay. Both input and output
 * should have size len. It is safe for these two buffers to be the same.
 *
 * Fractional delay filters have to keep state of the inputs they have
 * received so far, so this function moves the filter forward by the given
 * length. This makes it not safe to use a filter on multiple streams
 * simultaneously.
 *
 * This function will use allpass interpolation if the filter was allocated with
 * that option.
 *
 * @param filter    The fractional delay filter
 * @param input     The input buffer
 * @param output    The output buffer
 * @param len       The number of elements in the buffers
 */
void ATK_ApplyFractionalFilter(ATK_FractionalFilter* filter, const float* input,
                               float* output, size_t len) {
    float* fin  = filter->buffer+filter->head;
    float* fout = filter->buffer+filter->tail;
    float* end  = filter->buffer+filter->capacity;

    const float* src = input;
    float* dst = output;
    float curr, last;
    if (filter->allpass) {
        last = (float)filter->last;
        float beta = (float)filter->beta;
        while(len--) {
            *fin++ = *src++;
            if (fin == end) {
                fin = filter->buffer;
            }
            curr  = -beta * last;
            curr += *fout++;
            if (fout == end) {
                fout = filter->buffer;
            }
            curr += beta * *fout;
            *dst++ = curr;
            last = curr;
        }
    } else {
        float beta  = (float)filter->beta;
        float alpha = (float)filter->alpha;
        while(len--) {
            *fin++ = *src++;
            if (fin == end) {
                fin = filter->buffer;
            }
            // First 1/2 of interpolation
            curr = *fout++ * beta;
            if (fout == end) {
                fout = filter->buffer;
            }
            curr += *fout * alpha;
            *dst++ = curr;
        }
        last = curr;
    }
    filter->last = last;
    filter->head = fin-filter->buffer;
    filter->tail = fout-filter->buffer;
}

/**
 * Applies the delay to an input buffer, storing the result in output
 *
 * The values stored in output will have maximum delay. Both input and output
 * should have size len. It is safe for these two buffers to be the same
 * provided that the strides match.
 *
 * Fractional delay filters have to keep state of the inputs they have
 * received so far, so this function moves the filter forward by the given
 * length. This makes it not safe to use a filter on multiple streams
 * simultaneously.
 *
 * This function will use allpass interpolation if the filter was allocated with
 * that option.
 *
 * @param filter    The fractional delay filter
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements in the buffers
 */
void ATK_ApplyFractionalFilter_stride(ATK_FractionalFilter* filter,
                                      const float* input,  size_t istride,
                                      float* output, size_t ostride, size_t len) {
    float* fin  = filter->buffer+filter->head;
    float* fout = filter->buffer+filter->tail;
    float* end  = filter->buffer+filter->capacity;

    const float* src = input;
    float* dst = output;
    float curr, last;
    if (filter->allpass) {
        last = (float)filter->last;
        float beta = (float)filter->beta;
        while(len--) {
            *fin++ = *src;
            if (fin == end) {
                fin = filter->buffer;
            }
            curr  = -beta * last;
            curr += *fout++;
            if (fout == end) {
                fout = filter->buffer;
            }
            curr += beta * *fout;
            *dst = curr;
            last = curr;
            src += istride;
            dst += ostride;
        }
    } else {
        float beta  = (float)filter->beta;
        float alpha = (float)filter->alpha;
        while(len--) {
            *fin++ = *src;
            if (fin == end) {
                fin = filter->buffer;
            }
            // First 1/2 of interpolation
            curr = *fout++ * beta;
            if (fout == end) {
                fout = filter->buffer;
            }
            curr += *fout * alpha;
            *dst = curr;
            src += istride;
            dst += ostride;
        }
        last = curr;
    }
    filter->last = last;
    filter->head = fin-filter->buffer;
    filter->tail = fout-filter->buffer;
}

/**
 * Applies a tapped delay to an input buffer, storing the result in output
 *
 * The values stored in output will be delayed by the given tap. Both input
 * and output should have size len. It is safe for these two buffers to be
 * the same.
 *
 * Fractional delay filters have to keep state of the inputs they have received
 * so far, so this function moves the filter forward by the given length. This
 * means that the last len delayed values will be lost if tap is less than the
 * maximum delay. If you want to have a delay less than the maximum delay without
 * losing state, you should use {@link ATK_TapOutDelayFilter}. Also note that
 * taped outputs always use linear interpolation.
 *
 * @param filter    The delay filter
 * @param input     The input buffer
 * @param output    The output buffer
 * @param tap       The delay tap position
 * @param len       The number of elements in the buffers
 */
void ATK_TapApplyFractionalFilter(ATK_FractionalFilter* filter, const float* input,
                                  float* output, float tap, size_t len) {
    if (tap < 0 || tap > filter->delay) {
        ATK_SetError("Tap %f exceeds delay %f",tap,filter->delay);
        return;
    }

    float offset = filter->head - tap;
    if (offset < 0) {
        offset += filter->capacity;
    }
    size_t index = (size_t)offset;
    double alpha = offset-index;
    if (index == filter->capacity) {
        index = 0;
    }

    float* fout = filter->buffer+index;
    float* fin  = filter->buffer+filter->head;
    float* end  = filter->buffer+filter->capacity;

    const float* src = input;
    float* dst = output;
    float curr;
    while (len--) {
        *fin++ = *src++;
        if (fin == end) {
            fin = filter->buffer;
        }
        curr = (float)(*fout++ * (1-alpha));
        if (fout == end) {
            fout = filter->buffer;
        }
        curr += (float)(*fout * alpha);
        *dst++ = curr;
    }
    filter->head = ((filter->head + len) % filter->capacity);
    filter->tail = fout-filter->buffer;
}

/**
 * Applies a tapped delay to an input buffer, storing the result in output
 *
 * The values stored in output will be delayed by the given tap. Both input
 * and output should have size len. It is safe for these two buffers to be
 * the same, provided that the strides match.
 *
 * Fractional delay filters have to keep state of the inputs they have received
 * so far, so this function moves the filter forward by the given length. This
 * means that the last len delayed values will be lost if tap is less than the
 * maximum delay. If you want to have a delay less than the maximum delay without
 * losing state, you should use {@link ATK_TapOutDelayFilter}. Also note that
 * taped outputs always use linear interpolation.
 *
 * @param filter    The delay filter
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param tap       The delay tap position
 * @param len       The number of elements in the buffers
 */
void ATK_TapApplyFractionalFilter_stride(ATK_FractionalFilter* filter,
                                         const float* input,  size_t istride,
                                         float* output, size_t ostride,
                                         float tap, size_t len) {
    if (tap < 0 || tap > filter->delay) {
        ATK_SetError("Tap %f exceeds delay %f",tap,filter->delay);
        return;
    }

    float offset = filter->head - tap;
    if (offset < 0) {
        offset += filter->capacity;
    }
    size_t index = (size_t)offset;
    double alpha = offset-index;
    if (index == filter->capacity) {
        index = 0;
    }

    float* fout = filter->buffer+index;
    float* fin  = filter->buffer+filter->head;
    float* end  = filter->buffer+filter->capacity;

    const float* src = input;
    float* dst = output;
    float curr;
    while (len--) {
        *fin++ = *src;
        if (fin == end) {
            fin = filter->buffer;
        }
        curr = (float)(*fout++ * (1-alpha));
        if (fout == end) {
            fout = filter->buffer;
        }
        curr += (float)(*fout * alpha);
        *dst = curr;
        src += istride;
        dst += ostride;
    }
    filter->head = ((filter->head + len) % filter->capacity);
    filter->tail = fout-filter->buffer;
}

#pragma mark -
#pragma mark Allpass Filter
/**
 * An allpass delay filter, such as the one used by FreeVerb.
 *
 * This filter has an integral delay, like {@link ATK_DelayFilter}. However,
 * it has additional feedback coefficients to introduce interferance in the
 * signal. Because of this interferance, we do not allow allpass filters to
 * be tapped in or out like a normal delay filter.
 */
typedef struct ATK_AllpassFilter {
    /** The maximum delay */
    size_t delay;
    /** The current output position */
    size_t tail;
    /** The delay buffer */
    float* buffer;
    /** The filter feedback */
    float feedback;
} ATK_AllpassFilter;

/**
 * Returns a newly allocated allpass filter.
 *
 * The filter starts off zero padded, so that all results of the filter are
 * zero until the delay is reached.
 *
 * The delay of this filter can never be resized. However, the coefficients can
 * be updated at any time with {@link ATK_UpdateAllpassFilter}.
 *
 * @param delay     The delay length
 * @param feedback  The feedback coefficient
 *
 * @return a newly allocated allpass filter.
 */
ATK_AllpassFilter* ATK_AllocAllpassFilter(size_t delay, float feedback) {
    float* buffer = ATK_malloc(sizeof(float)*delay);
    if (buffer == NULL) {
        ATK_OutOfMemory();
        return NULL;
    }
    ATK_AllpassFilter* result = ATK_malloc(sizeof(ATK_AllpassFilter));
    if (result == NULL) {
        ATK_OutOfMemory();
        ATK_free(buffer);
        return NULL;
    }
    memset(buffer,0,sizeof(float)*delay);
    result->buffer = buffer;
    result->delay = delay;
    result->tail = 0;
    result->feedback = feedback;
    return result;
}

/**
 * Frees a previously allocated allpass filter.
 *
 * @param filter    The allpass filter
 */
void ATK_FreeAllpassFilter(ATK_AllpassFilter* filter) {
    if (filter == NULL) {
        return;
    }
    ATK_free(filter->buffer);
    filter->buffer = NULL;
    ATK_free(filter);
}

/**
 * Resets a allpass filter to its initial state.
 *
 * The filter buffer will be zeroed, so that no data is stored in the filter.
 *
 * @param filter    The allpass filter
 */
void ATK_ResetAllpassFilter(ATK_AllpassFilter* filter) {
    if (filter == NULL) {
        return;
    }
    memset(filter->buffer,0,sizeof(float)*filter->delay);
}

/**
 * Updates the allpass filter feedback.
 *
 * The filter buffer is unaffected by this function. Note that the delay cannot
 * be altered.
 *
 * @param filter    The allpass filter
 * @param feedback  The feedback coefficient
 */
void ATK_UpdateAllpassFilter(ATK_AllpassFilter* filter, float feedback) {
    filter->feedback = feedback;
}

/**
 * Returns the delay supported by this allpass filter.
 *
 * @param filter    The comb filter
 *
 * @return the delay supported by this allpass filter.
 */
size_t ATK_GetAllpassFilterDelay(ATK_AllpassFilter* filter) {
    return filter == NULL ? 0 : filter->delay;
}

/**
 * Returns the next value of the allpass filter.
 *
 * Allpass filters have to keep state of the inputs they have received so far,
 * so this function moves the filter forward. This makes it not safe to use a
 * filter on multiple streams simultaneously.
 *
 * @param filter    The comb filter
 * @param value     The input value
 *
 * @return the next value of the comb filter.
 */
float ATK_StepAllpassFilter(ATK_AllpassFilter* filter, float value) {
    float* data = filter->buffer+filter->tail;
    float out = *data;
    *data = value + out * filter->feedback;
    filter->tail = ((filter->tail+1) % filter->delay);
    return out-value;

}

/**
 * Applies the filter to an input buffer, storing the result in output
 *
 * Allpass filters have to keep state of the inputs they have received so far,
 * so this function moves the filter forward by the given length. This makes it
 * not safe to use a filter on multiple streams simultaneously.
 *
 * @param filter    The comb filter
 * @param input     The input buffer
 * @param output    The output buffer
 * @param len       The number of elements in the buffers
 */
void ATK_ApplyAllpassFilter(ATK_AllpassFilter* filter, const float* input,
                            float* output, size_t len) {
    const float* src = input;
    float* dst = output;
    float* flt = filter->buffer+filter->tail;
    float* end = filter->buffer+filter->delay;
    float out, in;
    while (len--) {
        in = *src++;
        out = *flt;
        *flt++ = in + out * filter->feedback;
        *dst++ = out - in;
        if (flt == end) {
            flt = filter->buffer;
        }
    }
    filter->tail = flt-filter->buffer;
}

/**
 * Applies the filter to an input buffer, storing the result in output
 *
 * Allpass filters have to keep state of the inputs they have received so far,
 * so this function moves the filter forward by the given length. This makes it
 * not safe to use a filter on multiple streams simultaneously.

 *
 * @param filter    The comb filter
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements in the buffers
 */
void ATK_ApplyAllpassFilter_stride(ATK_AllpassFilter* filter,
                                   const float* input,  size_t istride,
                                   float* output, size_t ostride, size_t len) {
    const float* src = input;
    float* dst = output;
    float* flt = filter->buffer+filter->tail;
    float* end = filter->buffer+filter->delay;
    float out, in;
    while (len--) {
        in = *src;
        out = *flt;
        *flt = in + out * filter->feedback;
        *dst = out - in;
        src += istride;
        dst += ostride;
        flt++;
        if (flt == end) {
            flt = filter->buffer;
        }
    }
    filter->tail = flt-filter->buffer;
}

#pragma mark -
#pragma mark Comb Filter
/**
 * A comb delay filter, such as the one used by FreeVerb.
 *
 * This filter has an integral delay, like {@link ATK_DelayFilter}. However, it has
 * additional feedforward and feedback coefficients to introduce interferance in the
 * signal. Because of this interferance, we do not allow comb filters to be tapped
 * in or out like a normal delay filter.
 */
typedef struct ATK_CombFilter {
    /** The maximum delay */
    size_t delay;
    /** The current output position */
    size_t tail;
    /** The delay buffer */
    float* buffer;
    /** The filter feedback */
    float feedback;
    /** The filter impulse from the previous frame  */
    float impulse;
    /** Weighs the impulse with previous output */
    float damping;
} ATK_CombFilter;

/**
 * Returns a newly allocated comb filter.
 *
 * The filter starts off zero padded, so that all results of the filter are
 * zero until the delay is reached.
 *
 * The delay of this filter can never be resized. However, the coefficients can be
 * updated at any time with {@link ATK_UpdateCombFilter}.
 *
 * @param delay     The delay length
 * @param feedback  The feedback coefficient
 * @param damping   The filter damping factor
 *
 * @return a newly allocated comb filter.
 */
ATK_CombFilter* ATK_AllocCombFilter(size_t delay, float feedback, float damping) {
    float* buffer = ATK_malloc(sizeof(float)*delay);
    if (buffer == NULL) {
        ATK_OutOfMemory();
        return NULL;
    }
    ATK_CombFilter* result = ATK_malloc(sizeof(ATK_CombFilter));
    if (result == NULL) {
        ATK_OutOfMemory();
        ATK_free(buffer);
        return NULL;
    }
    memset(buffer,0,sizeof(float)*delay);
    result->buffer = buffer;
    result->delay = delay;
    result->tail = 0;
    result->feedback = feedback;
    result->damping = damping;
    result->impulse = 0.0;
    return result;
}

/**
 * Frees a previously allocated comb filter.
 *
 * @param filter    The comb filter
 */
void ATK_FreeCombFilter(ATK_CombFilter* filter) {
    if (filter == NULL) {
        return;
    }
    ATK_free(filter->buffer);
    filter->buffer = NULL;
    ATK_free(filter);
}

/**
 * Resets a comb filter to its initial state.
 *
 * The filter buffer will be zeroed, so that no data is stored in the filter.
 *
 * @param filter    The comb filter
 */
void ATK_ResetCombFilter(ATK_CombFilter* filter) {
    if (filter == NULL) {
        return;
    }
    memset(filter->buffer,0,sizeof(float)*filter->delay);
    filter->impulse = 0.0;
}

/**
 * Updates the comb filter coefficients.
 *
 * The filter buffer is unaffected by this function. Note that the delay cannot
 * be altered.
 *
 * @param filter    The comb filter
 * @param feedback  The feedback coefficient
 * @param damping   The filter damping factor
 */
void ATK_UpdateCombFilter(ATK_CombFilter* filter, float feedback, float damping) {
    filter->feedback = feedback;
    filter->damping  = damping;
}

/**
 * Returns the delay supported by this comb filter.
 *
 * @param filter    The comb filter
 *
 * @return the delay supported by this comb filter.
 */
size_t ATK_GetCombFilterDelay(ATK_CombFilter* filter) {
    return filter == NULL ? 0 : filter->delay;
}

/**
 * Returns the next value of the comb filter.
 *
 * Comb filters have to keep state of the inputs they have received so far, so
 * this function moves the filter forward. This makes it not safe to use a filter
 * on multiple streams simultaneously.
 *
 * @param filter    The comb filter
 * @param value     The input value
 *
 * @return the next value of the comb filter.
 */
float ATK_StepCombFilter(ATK_CombFilter* filter, float value) {
    float* data = filter->buffer+filter->tail;
    float out = *data;
    filter->impulse = out * (1 - filter->damping) + filter->impulse * filter->damping;
    *data = value + filter->impulse*filter->feedback;
    filter->tail = ((filter->tail+1) % filter->delay);
    return out;
}

/**
 * Applies the filter to an input buffer, storing the result in output
 *
 * Comb filters have to keep state of the inputs they have received so far, so
 * this function moves the filter forward by the given length. This makes it not
 * safe to use a filter on multiple streams simultaneously.
 *
 * @param filter    The comb filter
 * @param input     The input buffer
 * @param output    The output buffer
 * @param len       The number of elements in the buffers
 */
void ATK_ApplyCombFilter(ATK_CombFilter* filter, const float* input,
                         float* output, size_t len) {
    const float* src = input;
    float* dst = output;
    float* flt = filter->buffer+filter->tail;
    float* end = filter->buffer+filter->delay;
    float curr;
    float impulse = filter->impulse;
    float feedback = filter->feedback;
    float damping = filter->damping;
    while (len--) {
        curr = *flt;
        impulse = curr * (1 - damping) + impulse * damping;
        *flt   = *src++ + impulse*feedback;

        *dst++ = curr;
        flt++;
        if (flt == end) {
            flt = filter->buffer;
        }
    }
    filter->tail = flt-filter->buffer;
    filter->impulse = impulse;
}

/**
 * Applies the filter to an input buffer, storing the result in output
 *
 * Comb filters have to keep state of the inputs they have received so far, so
 * this function moves the filter forward by the given length. This makes it not
 * safe to use a filter on multiple streams simultaneously.
 *
 * @param filter    The comb filter
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements in the buffers
 */
void ATK_ApplyCombFilter_stride(ATK_CombFilter* filter,
                                const float* input,  size_t istride,
                                float* output, size_t ostride, size_t len) {
    const float* src = input;
    float* dst = output;
    float* flt = filter->buffer+filter->tail;
    float* end = filter->buffer+filter->delay;
    float curr;
    float impulse = filter->impulse;
    float feedback = filter->feedback;
    float damping = filter->damping;
    while (len--) {
        curr = *flt;

        impulse = curr * (1 - damping) + impulse * damping;
        *flt = *src + impulse*feedback;
        src += istride;

        *dst = curr;
        dst += ostride;

        flt++;
        if (flt == end) {
            flt = filter->buffer;
        }
    }
    filter->tail = flt-filter->buffer;
    filter->impulse = impulse;
}


/**
 * Applies the filter to an input buffer, adding the result to output
 *
 * Comb filters have to keep state of the inputs they have received so far, so
 * this function moves the filter forward by the given length. This makes it not
 * safe to use a filter on multiple streams simultaneously.
 *
 * @param filter    The comb filter
 * @param input     The input buffer
 * @param output    The output buffer
 * @param len       The number of elements in the buffers
 */
void ATK_AddCombFilter(ATK_CombFilter* filter, const float* input,
                       float* output, size_t len) {
    const float* src = input;
    float* dst = output;
    float* flt = filter->buffer+filter->tail;
    float* end = filter->buffer+filter->delay;
    float curr;
    float impulse = filter->impulse;
    float feedback = filter->feedback;
    float damping = filter->damping;
    while (len--) {
        curr = *flt;
        impulse = curr * (1 - damping) + impulse * damping;
        *flt   = *src++ + impulse*feedback;

        *dst++ += curr;
        flt++;
        if (flt == end) {
            flt = filter->buffer;
        }
    }
    filter->tail = flt-filter->buffer;
    filter->impulse = impulse;
}

/**
 * Applies the filter to an input buffer, adding the result to output
 *
 * Comb filters have to keep state of the inputs they have received so far, so
 * this function moves the filter forward by the given length. This makes it not
 * safe to use a filter on multiple streams simultaneously.
 *
 * @param filter    The comb filter
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements in the buffers
 */
void ATK_AddCombFilter_stride(ATK_CombFilter* filter,
                              const float* input,  size_t istride,
                              float* output, size_t ostride, size_t len)  {
    const float* src = input;
    float* dst = output;
    float* flt = filter->buffer+filter->tail;
    float* end = filter->buffer+filter->delay;
    float curr;
    float impulse = filter->impulse;
    float feedback = filter->feedback;
    float damping = filter->damping;
    while (len--) {
        curr = *flt;

        impulse = curr * (1 - damping) + impulse * damping;
        *flt = *src + impulse*feedback;
        src += istride;

        *dst += curr;
        dst  += ostride;

        flt++;
        if (flt == end) {
            flt = filter->buffer;
        }
    }
    filter->tail = flt-filter->buffer;
    filter->impulse = impulse;
}


#pragma mark -
#pragma mark Debugging
/**
 * Prints out the IIR filter for debugging purposes
 *
 * @param filter    The IIR filter
 */
void ATK_PrintIIRFilter(ATK_IIRFilter* filter) {
    if (filter == NULL) {
        printf("NULL filter\n");
        return;
    }

    if (filter->adeg < 1) {
        if (filter->bdeg == 1) {
            printf("Scalar filter:\n");
        } else {
            printf("%zu-zero filter:\n",filter->bdeg);
        }
    } else {
        if (filter->bdeg == 0) {
            printf("%zu-pole filter:\n",filter->adeg);
        } else {
            printf("%zu-pole/%zu-zero filter:\n",filter->adeg,filter->bdeg);
        }
    }

    for(size_t ii = 0; ii <= filter->adeg; ii++) {
        printf("  a[%zu] = %f\n",ii,filter->a[ii]);
    }
    for(size_t ii = 0; ii <= filter->bdeg; ii++) {
        printf("  b[%zu] = %f\n",ii,filter->b[ii]);
    }

    for(size_t ii = 0; ii < filter->bdeg; ii++) {
        printf("  in[%zu] = %f\n",ii,filter->ins[ii]);
    }
    for(size_t ii = 0; ii < filter->adeg; ii++) {
        printf("  out[%zu] = %f\n",ii,filter->outs[ii]);
    }
}
