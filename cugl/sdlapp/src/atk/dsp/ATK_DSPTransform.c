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
 * @file ATK_DSPTransform.c
 *
 * This component provides FFT functions built on top of Kiss FFT. Kiss FFT
 * has made some design decisions that make it produce slightly different
 * outputs than scipy's FFT (especially for inverse FFTs). Our interface
 * adapts Kiss FFT to make it match scipy as much as possible.
 */
#pragma mark -
#pragma mark Real Valued

/**
 * The internal state for a real-valued FFT
 *
 * This algorithm is 45% faster than a complex-valued FFT on real-valued
 * signals.
 *
 * A real-valued FFT can either be a normal FFT or an inverse.  Inverse
 * real-value FFTs may only be used with {@link ATK_ApplyRealInvFFT}
 * and {@link ATK_ApplyRealInvFFT_stride}.
 */
typedef struct ATK_RealFFT {
    /** The FFT size */
    size_t size;
    /** The internal state of the FFT algorithm */
    kiss_fftr_cfg state;
    /** Buffered input (for when we cannot do in-place) */
    kiss_fft_scalar* input;
    /** Buffered output (for when we cannot do in-place) */
    kiss_fft_scalar* output;
    /** Whether this is an inverse FFT */
    SDL_bool inverse;
} ATK_RealFFT;

/**
 * Returns the best real-valued FFT size for the given window length
 *
 * The result will be a value >= size.
 *
 * @param size  The desired window length
 *
 * @return the  best real-valued FFT size for the given window length
 */
size_t ATK_GetRealFFTBestSize(size_t size) {
    return kiss_fftr_next_fast_size_real((int)size);
}

/**
 * Returns a newly allocated real-valued FFT
 *
 * The window length is suggestion. The actual length will be computed from
 * {@link ATK_GetRealFFTBestSize}. Use {@link ATK_GetRealFFTSize} to query the
 * actual size. The size of a real-valued FFT must be even.
 *
 * The resulting FFT can either be F or F^-1 (the inverse transform) as
 * specified. Like the scipy implementation, the inverse FFT is not just a
 * phase shift. It also normalizes the results, guaranteeing that it is a
 * true inverse on the input buffer. Note that inverse FFTs may only be
 * used by the functions {@link ATK_ApplyRealInvFFT} and
 * {@link ATK_ApplyRealInvFFT_stride}.
 *
 * @param size      The window length (suggested)
 * @param inverse   Whether to create the inverse transform
 *
 * @return a newly allocated real-valued FFT
 */
ATK_RealFFT* ATK_AllocRealFFT(size_t size, SDL_bool inverse) {
    size = ATK_GetRealFFTBestSize(size);
    kiss_fftr_cfg state = kiss_fftr_alloc((int)size,inverse ? 1 : 0,NULL,NULL);
    if (state == NULL) {
        ATK_OutOfMemory();
        return NULL;
    }

    kiss_fft_scalar* input = ATK_malloc((size+2)*sizeof(kiss_fft_scalar));
    if (input == NULL) {
        ATK_OutOfMemory();
        free(state);
        return NULL;
    }

    kiss_fft_scalar* output = ATK_malloc((size+2)*sizeof(kiss_fft_scalar));;
    if (output == NULL) {
        ATK_OutOfMemory();
        free(state);
        free(input);
        return NULL;
    }

    ATK_RealFFT* result = ATK_malloc(sizeof(ATK_RealFFT));
    if (result == NULL) {
        ATK_OutOfMemory();
        free(state);
        free(input);
        free(output);
        return NULL;
    }

    memset(input,0, (size+2)*sizeof(kiss_fft_scalar));
    memset(output,0,(size+2)*sizeof(kiss_fft_scalar));

    result->state = state;
    result->size  = size;
    result->input = input;
    result->output  = output;
    result->inverse = inverse;
    return result;
}

/**
 * Frees a previously allocated real-valued FFT
 *
 * @param fft   The FFT state
 */
void ATK_FreeRealFFT(ATK_RealFFT* fft) {
    if (fft == NULL || fft->state == NULL) {
        return;
    }
    kiss_fft_free(fft->state);
    ATK_free(fft->input);
    ATK_free(fft->output);
    fft->state = NULL;
    fft->input = NULL;
    fft->output = NULL;
    ATK_free(fft);
}

/**
 * Returns the window length of the real-valued FFT
 *
 * This value is the actual window length, and not (necessarily) the one
 * suggested at the time of creation
 *
 * @param fft   The FFT state
 *
 * @return the window length of the real-valued FFT
 */
size_t ATK_GetRealFFTSize(ATK_RealFFT* fft) {
    if (fft == NULL) {
        return 0;
    }
    return fft->size;
}

/**
 * Applies the FFT to the real input signal, storing the result in output
 *
 * The input must be an array of floats of size N where N is the FFT size
 * given by {@link ATK_GetRealFFTSize}. The output will be an array of complex
 * numbers of size N/2+1 (so the array itself must be size N+2). The even
 * elements of this array are the real components while the odd values are
 * the imaginary components.
 *
 * The output arrary is shorter than N because the output of a real-valued
 * signal is symmetric. In this case the output is the first half of the
 * output values. This function will fail (returning -1 and setting an error)
 * if the fft is an inverse real-valued fft.
 *
 * @param fft       The FFT state
 * @param input     The input signal
 * @param output    The FFT output
 *
 * @return 0 if the FFT is successful, -1 otherwise
 */
int ATK_ApplyRealFFT(ATK_RealFFT* fft, const float* input, float* output) {
    if (fft == NULL || fft->inverse) {
        return -1;
    }

    kiss_fftr(fft->state,(kiss_fft_scalar*)input,(kiss_fft_cpx*)output);
    return 0;
}

/**
 * Applies the FFT to the real input signal, storing the result in output
 *
 * The input must be an array of floats of size N where N is the FFT size
 * given by {@link ATK_GetRealFFTSize}. The output will be an array of complex
 * numbers of size N/2+1 (so the array itself must be size N+2). The stride
 * for output applies to the complex numbers, not the components. So if the
 * output buffer has stride 3, all positions at multiples of 6 are real,
 * followed by an imaginary at the next position.
 *
 * The output arrary is shorter than N because the output of a real-valued
 * signal is symmetric. In this case the output is the first half of the
 * output values. This function will fail (returning -1 and setting an error)
 * if the fft is an inverse real-valued fft.
 *
 * @param fft       The FFT state
 * @param input     The input signal
 * @param istride   The input stride
 * @param output    The FFT output
 * @param ostride   The output stride
 *
 * @return 0 if the FFT is successful, -1 otherwise
 */
int ATK_ApplyRealFFT_stride(ATK_RealFFT* fft,
                            const float* input, size_t istride,
                            float* output, size_t ostride) {
    if (fft == NULL || fft->inverse) {
        return -1;
    }

    ATK_VecCopy_sstride(input, istride, fft->input, fft->size);
    kiss_fftr(fft->state,fft->input,(kiss_fft_cpx*)fft->output);

    const kiss_fft_cpx* src = (kiss_fft_cpx*)fft->output;
    float* dst = output;
    size_t len = fft->size/2+1;
    while (len--) {
        *dst++ = src->r;
        *dst   = src->i;
        dst += 2*ostride-1;
        src++;
    }

    return 0;
}

/**
 * Applies the inverse FFT to the real input signal, storing the result in output
 *
 * The output will be an array of floats of size N where N is the FFT size
 * given by {@link ATK_GetRealFFTSize}. The input must be an array of complex
 * numbers of size N/2+1 (so the array itself must be size N+2). The even
 * elements of this array are the real components while the odd values are
 * the imaginary components.
 *
 * The input array is shorter than N because the output of a real-valued
 * signal is symmetric. In this case the input is the first half of the
 * FFT values. This function will fail (returning -1 and setting an error)
 * if the fft is not an inverse real-valued fft.
 *
 * @param fft       The FFT state
 * @param input     The input signal
 * @param output    The FFT output
 *
 * @return 0 if the FFT is successful, -1 otherwise
 */
int ATK_ApplyRealInvFFT(ATK_RealFFT* fft, const float* input, float* output) {
    if (fft == NULL || !fft->inverse) {
        return -1;
    }

    kiss_fftri(fft->state,(const kiss_fft_cpx*)input,fft->output);
    ATK_VecScale(fft->output, (float)(1.0/fft->size), output, fft->size);
    return 0;
}

/**
 * Applies the inverse FFT to the real input signal, storing the result in output
 *
 * The output will be an array of floats of size N where N is the FFT size
 * given by {@link ATK_GetRealFFTSize}. The input must be an array of complex
 * numbers of size N/2+1 (so the array itself must be size N+2). The stride
 * for input applies to the complex numbers, not the components. So if the
 * input buffer has stride 3, all positions at multiples of 6 are real,
 * followed by an imaginary at the next position.
 *
 * The input array is shorter than N because the output of a real-valued
 * signal is symmetric. In this case the input is the first half of the
 * FFT values. This function will fail (returning -1 and setting an error)
 * if the fft is not an inverse real-valued fft.
 *
 * @param fft       The FFT state
 * @param input     The input signal
 * @param istride   The input stride
 * @param output    The FFT output
 * @param ostride   The output stride
 *
 * @return 0 if the FFT is successful, -1 otherwise
 */
int ATK_ApplyRealInvFFT_stride(ATK_RealFFT* fft,
                               const float* input, size_t istride,
                               float* output, size_t ostride) {
    if (fft == NULL || !fft->inverse) {
        return -1;
    }

    const float* src = input;
    kiss_fft_cpx* dst = (kiss_fft_cpx*)fft->input;
    size_t len = fft->size/2+1;
    while (len--) {
        dst->r = *src++;
        dst->i = *src;
        dst++;
        src += 2*istride-1;
    }

    kiss_fftri(fft->state,(const kiss_fft_cpx*)fft->input,fft->output);
    ATK_VecScale_stride(fft->output, 1, (float)(1.0/fft->size), output, ostride, fft->size);
    return 0;
}


/**
 * Applies the FFT to the real input signal, storing the magnitudes in output
 *
 * The input must be an array of floats of size N where N is the FFT size
 * given by {@link ATK_GetRealFFTSize}. The output will be an array of size
 * N/2+1, and will store the magnitudes of the FFT result.
 *
 * The output arrary is shorter than N because the output of a real-valued
 * signal is symmetric. In this case the output is the first half of the
 * output values. This function will fail (returning -1 and setting an error)
 * if the fft is an inverse real-valued fft.
 *
 * @param fft       The FFT state
 * @param input     The input signal
 * @param output    The FFT output
 *
 * @return 0 if the FFT is successful, -1 otherwise
 */
int ATK_ApplyRealFFTMag(ATK_RealFFT* fft, const float* input, float* output) {
    if (fft == NULL || fft->inverse) {
        return -1;
    }

    kiss_fftr(fft->state,(const kiss_fft_scalar*)input,(kiss_fft_cpx*)fft->output);
    const kiss_fft_cpx* src = (kiss_fft_cpx*)fft->output;
    float* dst = output;
    size_t len = fft->size/2+1;
    while (len--) {
        *dst++ = sqrtf(src->r*src->r+src->i*src->i);
        src++;
    }

    return 0;
}

/**
 * Applies the FFT to the real input signal, storing the magnitudes in output
 *
 * The input must be an array of floats of size N where N is the FFT size
 * given by {@link ATK_GetRealFFTSize}. The output will be an array of size
 * N/2+1, and will store the magnitudes of the FFT result.
 *
 * The output arrary is shorter than N because the output of a real-valued
 * signal is symmetric. In this case the output is the first half of the
 * output values. This function will fail (returning -1 and setting an error)
 * if the fft is an inverse real-valued fft.
 *
 * @param fft       The FFT state
 * @param input     The input signal
 * @param istride   The input stride
 * @param output    The FFT output
 * @param ostride   The output stride
 *
 * @return 0 if the FFT is successful, -1 otherwise
 */
int ATK_ApplyRealFFTMag_stride(ATK_RealFFT* fft,
                                const float* input, size_t istride,
                                float* output, size_t ostride) {
    if (fft == NULL || fft->inverse) {
        return -1;
    }

    ATK_VecCopy_sstride(input, istride, fft->input, fft->size);
    kiss_fftr(fft->state,fft->input,(kiss_fft_cpx*)fft->output);

    const kiss_fft_cpx* src = (kiss_fft_cpx*)fft->output;
    float* dst = output;
    size_t len = fft->size/2+1;
    while (len--) {
        *dst = sqrtf(src->r*src->r+src->i*src->i);
        dst += ostride;
        src++;
    }

    return 0;
}

#pragma mark -
#pragma mark Complex Valued
/**
 * The internal state for a complex-valued FFT
 *
 * This algorithm is slower than a real-valued FFT on real-valued signals.
 * It should only be used for properly complex input.
 */
typedef struct ATK_ComplexFFT {
    /** The FFT size */
    size_t size;
    /** The internal state of the FFT algorithm */
    kiss_fft_cfg state;
    /** Buffered input (for when we cannot do in-place) */
    kiss_fft_cpx* input;
    /** Buffered output (for when we cannot do in-place) */
    kiss_fft_cpx* output;
    /** Whether this is an inverse FFT */
    SDL_bool inverse;
} ATK_ComplexFFT;

/**
 * Returns the best complex-valued FFT size for the given window length
 *
 * The result will be a value >= size.
 *
 * @param size  The desired window length
 *
 * @return the  best complex-valued FFT size for the given window length
 */
size_t ATK_GetComplexFFTBestSize(size_t size) {
    return kiss_fft_next_fast_size((int)size);
}

/**
 * Returns a newly allocated complex-valued FFT
 *
 * The window length is suggestion. The actual length will be computed from
 * {@link ATK_GetComplexFFTBestSize}. Use {@link ATK_GetComplexFFTSize} to
 * query the actual size.
 *
 * The resulting FFT can either be F or F^-1 (the inverse transform) as
 * specified.
 *
 * @param size      The window length (suggested)
 * @param inverse   Whether to create the inverse transform
 *
 * @return a newly allocated complex-valued FFT
 */
ATK_ComplexFFT* ATK_AllocComplexFFT(size_t size, SDL_bool inverse) {
    size = ATK_GetComplexFFTBestSize(size);
    kiss_fft_cfg state = kiss_fft_alloc((int)size,inverse ? 1 : 0,NULL,NULL);
    if (state == NULL) {
        ATK_OutOfMemory();
        return NULL;
    }

    kiss_fft_cpx* input = ATK_malloc(size*sizeof(kiss_fft_cpx));
    if (input == NULL) {
        ATK_OutOfMemory();
        free(state);
        return NULL;
    }

    kiss_fft_cpx* output = ATK_malloc(size*sizeof(kiss_fft_cpx));
    if (output == NULL) {
        ATK_OutOfMemory();
        free(state);
        free(input);
        return NULL;
    }

    ATK_ComplexFFT* result = ATK_malloc(sizeof(ATK_ComplexFFT));
    if (result == NULL) {
        ATK_OutOfMemory();
        free(state);
        free(input);
        free(output);
        return NULL;
    }

    memset(input,0,size*sizeof(kiss_fft_cpx));
    memset(output,0,size*sizeof(kiss_fft_cpx));

    result->state = state;
    result->size  = size;
    result->input = input;
    result->output  = output;
    result->inverse = inverse;
    return result;
}

/**
 * Frees a previously allocated complex-valued FFT
 *
 * @param fft   The FFT state
 */
void ATK_FreeComplexFFT(ATK_ComplexFFT* fft) {
    if (fft == NULL || fft->state == NULL) {
        return;
    }
    kiss_fft_free(fft->state);
    ATK_free(fft->input);
    ATK_free(fft->output);
    fft->state = NULL;
    fft->input = NULL;
    fft->output = NULL;
    ATK_free(fft);
}

/**
 * Returns the window length of the complex-valued FFT
 *
 * This value is the actual window length, and not (necessarily) the one
 * suggested at the time of creation
 *
 * @param fft   The FFT state
 *
 * @return the window length of the complex-valued FFT
 */
size_t ATK_GetComplexFFTSize(ATK_ComplexFFT* fft) {
    if (fft == NULL) {
        return 0;
    }
    return fft->size;
}

/**
 * Applies the FFT to the input signal, storing the result in output
 *
 * Both input and output should be size 2*{@link ATK_GetComplexFFTSize}.
 * The should consist of interleaved real and imaginary values where the
 * even positions are the real components and the odd positions are the
 * imaginary components.
 *
 * @param fft       The FFT state
 * @param input     The input signal
 * @param output    The FFT output
 */
void ATK_ApplyComplexFFT(ATK_ComplexFFT* fft, const float* input, float* output) {

    const float* src1 = input;
    kiss_fft_cpx* dst1 = fft->input;
    size_t len = fft->size;
    while (len--) {
        dst1->r = *src1++;
        dst1->i = *src1++;
        dst1++;
    }
    if (input == output) {
        ATK_VecCopy(input, (float*)fft->input, 2*fft->size);
        kiss_fft(fft->state,fft->input,(kiss_fft_cpx*)output);
    } else {
        kiss_fft(fft->state,(const kiss_fft_cpx*)input,(kiss_fft_cpx*)output);
    }
    if (fft->inverse) {
        ATK_VecScale(output, (float)(1.0/fft->size), output, 2*fft->size);
    }
}

/**
 * Applies the FFT to the complex input signal, storing the result in output
 *
 * Both input and output should be size 2*{@link ATK_GetComplexFFTSize}.
 * The should consist of interleaved real and imaginary values where the
 * even positions are the real components and the odd positions are the
 * imaginary components.
 *
 * The stride is applied to the complex numbers, not the components. So if
 * a buffer has stride 3, all positions at multiples of 6 are real, followed
 * by an imaginary at the next position.
 *
 * @param fft       The FFT state
 * @param input     The input signal
 * @param istride   The input stride
 * @param output    The FFT output
 * @param ostride   The output stride
 */
void ATK_ApplyComplexFFT_stride(ATK_ComplexFFT* fft, const float* input, size_t istride,
                                float* output, size_t ostride) {
    if (istride == 0) {
        istride = 1;
    }
    if (ostride == 0) {
        ostride = 1;
    }

    const float* src1 = input;
    kiss_fft_cpx* dst1 = fft->input;
    size_t len = fft->size;
    while (len--) {
        dst1->r = *src1++;
        dst1->i = *src1;
        src1 += 2*istride-1;
        dst1++;
    }

    kiss_fft(fft->state,fft->input,fft->output);

    const kiss_fft_cpx* src2 = fft->output;
    float* dst2 = output;
    len = fft->size;
    if (fft->inverse) {
        double factor = 1.0/fft->size;
        while (len--) {
            *dst2++ = (float)(src2->r*factor);
            *dst2 = (float)(src2->i*factor);
            dst2 += 2*ostride-1;
            src2++;
        }
    } else {
        while (len--) {
            *dst2++ = src2->r;
            *dst2   = src2->i;
            dst2 += 2*ostride-1;
            src2++;
        }
    }
}

/**
 * Applies the FFT to the input separated into real and complex components
 *
 * Both the input (realin, imagin) and output (realout, imagout) should be
 * arrays of size {@link ATK_GetComplexFFTSize}. They consiste of the real
 * and imaginary components as separate arrays.
 *
 * @param fft       The FFT state
 * @param realin    The real component of the input signal
 * @param imagin    The imaginary component of the input signal
 * @param realout   The real component of the FFT output
 * @param imagout   The imaginary component of the FFT output
 */
void ATK_ApplySplitComplexFFT(ATK_ComplexFFT* fft, const float* realin, const float* imagin,
                              float* realout, float* imagout) {
    const float* isrc1 = realin;
    const float* isrc2 = imagin;
    kiss_fft_cpx* idst = fft->input;
    size_t len = fft->size;
    while(len--) {
        idst->r = *isrc1++;
        idst->i = *isrc2++;
        idst++;
    }

    kiss_fft(fft->state,fft->input,fft->output);

    const kiss_fft_cpx* osrc = fft->output;
    float* odst1 = realout;
    float* odst2 = imagout;
    len = fft->size;
    if (fft->inverse) {
        double factor = 1.0/fft->size;
        while(len--) {
            *odst1++ = (float)(osrc->r*factor);
            *odst2++ = (float)(osrc->i*factor);
            osrc++;
        }
    } else {
        while(len--) {
            *odst1++ = osrc->r;
            *odst2++ = osrc->i;
            osrc++;
        }
    }
}

/**
 * Applies the FFT to the input separated into real and complex components
 *
 * Both the input (realin, imagin) and output (realout, imagout) should be
 * arrays of size {@link ATK_GetComplexFFTSize}. They consiste of the real
 * and imaginary components as separate arrays.
 *
 * @param fft       The FFT state
 * @param realin    The real component of the input signal
 * @param ristride  The stride of the real input component
 * @param imagin    The imaginary component of the input signal
 * @param iistride  The stride of the imaginary input component
 * @param realout   The real component of the FFT output
 * @param rostride  The stride of the real output component
 * @param imagout   The imaginary component of the FFT output
 * @param iostride  The stride of the imaginary output component
 */
void ATK_ApplySplitComplexFFT_stride(ATK_ComplexFFT* fft,
                                     const float* realin, size_t ristride,
                                     const float* imagin, size_t iistride,
                                     float* realout, size_t rostride,
                                     float* imagout, size_t iostride) {

    const float* isrc1 = realin;
    const float* isrc2 = imagin;
    kiss_fft_cpx* idst = fft->input;
    size_t len = fft->size;
    while(len--) {
        idst->r = *isrc1;
        idst->i = *isrc2;
        idst++;
        isrc1 += ristride;
        isrc2 += iistride;
    }

    kiss_fft(fft->state,fft->input,fft->output);

    const kiss_fft_cpx* osrc = fft->output;
    float* odst1 = realout;
    float* odst2 = imagout;
    len = fft->size;
    if (fft->inverse) {
        double factor = 1.0/fft->size;
        while(len--) {
            *odst1 = (float)(osrc->r*factor);
            *odst2 = (float)(osrc->i*factor);
            osrc++;
            odst1 += rostride;
            odst2 += iostride;
        }
    } else {
        while(len--) {
            *odst1 = osrc->r;
            *odst2 = osrc->i;
            osrc++;
            odst1 += rostride;
            odst2 += iostride;
        }
    }
}
