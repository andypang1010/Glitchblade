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
 * @file ATK_DSPConvolve
 *
 * This component provides support for naive (nested-loop) and FFT convolutions.
 * These convolutions are implemented as stateful-filters, allowing them to be
 * applied to audio streams with no latency beyond the time necessary to compute
 * the convolution. It is possible to switch back-and-forth between FFT and
 * naive convolutions on the same audio stream without needing to reset the
 * filter. The only issue is that FFT convolutions appear to have a precision of
 * 1e-5 on signals normalized to [-1,1].
 *
 * On paper, naive is O(nm) while the FFT algorithms are O(N log N), where n is
 * the length of the signal, m is the length of the kernel and N = max(n,m).
 * However, there is non-trivial overhead in a FFT convolution, meaning that
 * it is not a win for small values of either n or m. Textbooks claim that this
 * value is between 20 and 50. Which is correct for this library -- in debug
 * mode.
 *
 * Running the tests on this part of the library can be quite illuminating. We
 * have added simple timing functions to the unit tests to allow us to compare
 * the performance of naive and FFT convolutions. The base test is on 1024
 * samples, which is the minimum device buffer size of an iPhone. On a Macbook
 * M2 Max, the naive implementation runs at ~1 millisec, while the FFT solution
 * runs at ~100 microsec, about an order of magnitude improvement. If you
 * compile the library with -O3, the FFT speeds up to 40 microsec, but the
 * naive version speeds up to 80 microsec! It appears that the cross-over point
 * with optimization is actually 512, which is the mimimum device buffer size
 * of most PCs. Even more interesesting is the fact that stride 2 convolutions
 * are unchanged for FFTs but drop to 200 microsec for the naive version. This
 * justifies our decision (in some cases) to separate adjacent from stride-aware
 * code.
 */
//https://dsp.stackexchange.com/questions/736/how-do-i-implement-cross-correlation-to-prove-two-audio-files-are-similar

#pragma mark Debugging
/**
 * Prints out the contents of the given real-valued buffer.
 *
 * @param buffer    The real-valued buffer
 * @param size      The buffer size
 */
static void print_buffer(float* buffer, size_t size) {
    const float* src = buffer;
    printf("{");
    if (size > 0) {
        printf(" %f",*src++);
        size--;
    }
    while (size--) {
        printf(", %f",*src++);
    }
    printf(" }\n");
}

/**
 * Prints out the contents of the given complex-valued buffer.
 *
 * The complex data is stored interleaved as a buffer of twice the
 * given size.
 *
 * @param buffer    The complex-valued buffer
 * @param size      The buffer size
 */
static void print_complex_stream(float* buffer, size_t size) {
    const float* src = buffer;
    printf("{");
    if (size > 0) {
        printf(" %f",*src++);
        printf("+%fi",*src++);
        size--;
    }
    while (size--) {
        printf(", %f",*src++);
        printf("+%fi",*src++);
    }
    printf(" }\n");
}

#pragma mark -
#pragma mark Window Generation
#define DEFAULT_ERROR   1.0e-21
/**
 * Returns the bessel function value of x for a kaiser-window
 *
 * This function uses the error value to truncate the infinite series.
 *
 * @param x     The function parameter
 * @param err   The error tolerance for the result
 *
 * @return the bessel function value of x for a kaiser-window
 */
static double bessel(double x, double err) {
    double xdiv2 = x / 2.0;
    double i0 = 1.0;
    double f = 1.0;
    double i = 1.0;

    double stem = xdiv2*xdiv2;
    double diff = stem/(f*f);
    while (diff >= err) {
        i0 += diff;
        i += 1;
        f *= i;
        stem *= xdiv2*xdiv2;
        diff = stem/(f*f);
    }

    return i0;
}

/**
 * Returns a newly allocated Blackman window.
 *
 * Blackman windows are implemented as defined here
 *
 *    https://en.wikipedia.org/wiki/Window_function
 *
 * This function allocates an array that must be freed with ATK_free when it
 * is no longer needed.
 *
 * If half is non-zero, this function allocates a window only (size+1)/2 points.
 * These points are the first half of the window if half is negative, and the
 * second half of the window if half is positive. Either half will contain the
 * center point.
 *
 * @param size  The window size
 * @param half  Whether to restrict to a half window
 *
 * @returns a newly allocated Blackman window.
 */
float* ATK_AllocBlackmanWindow(size_t size, int half) {
    size_t amt = half ? (size+1)/2 : size;
    float* buffer = ATK_malloc(sizeof(float)*amt);
    if (buffer == NULL) {
        ATK_OutOfMemory();
        return NULL;
    }
    ATK_FillBlackmanWindow(buffer, size, half);
    return buffer;
}

/**
 * Fills the buffer with a Blackman window.
 *
 * Blackman windows are implemented as defined here
 *
 *    https://en.wikipedia.org/wiki/Window_function
 *
 * The buffer should be able to store size values if it is a full window. If
 * half is non-zero, this function will only copy (size+1)/2 points into the
 * buffer. These points are the first half of the window if half is negative,
 * and the second half of the window if half is positive. Either half will
 * contain the center point.
 *
 * @param buffer    The buffer to store the window
 * @param size      The window size
 * @param half      Whether to restrict to a half window
 */
void ATK_FillBlackmanWindow(float* buffer, size_t size, int half) {
    if (size == 0) {
        return;
    }

    size_t amt = size;
    double start = 0;

    if (half < 0) {
        amt = (size+1)/2;
    } else if (half > 0) {
        amt = (size+1)/2;
        start = (double)(size-amt);
    }

    double n;
    double m = (double)(size-1);
    for(size_t ii = 0; ii < amt; ii++) {
        n = ii + start;
        buffer[ii] = (float)(0.42 - (0.5 * cos(2*M_PI*n/m)) + (0.08 * cos(4*M_PI*n/m)));
    }
}

/**
 * Returns a newly allocated Hamming window.
 *
 * Hamming windows are implemented as defined here
 *
 *    https://en.wikipedia.org/wiki/Window_function
 *
 * This function allocates an array that must be freed with ATK_free when it
 * is no longer needed.
 *
 * If half is non-zero, this function allocates a window only (size+1)/2 points.
 * These points are the first half of the window if half is negative, and the
 * second half of the window if half is positive. Either half will contain the
 * center point.
 *
 * @param size  The window size
 * @param half  Whether to restrict to a half window
 *
 * @returns a newly allocated Hamming window.
 */
float* ATK_AllocHammingWindow(size_t size, SDL_bool half) {
    size_t amt = half ? (size+1)/2 : size;
    float* buffer = ATK_malloc(sizeof(float)*amt);
    if (buffer == NULL) {
        ATK_OutOfMemory();
        return NULL;
    }
    ATK_FillHammingWindow(buffer, size, half);
    return buffer;
}

/**
 * Fills the buffer with a Hamming window.
 *
 * Hamming windows are implemented as defined here
 *
 *    https://en.wikipedia.org/wiki/Window_function
 *
 * The buffer should be able to store size values if it is a full window. If
 * half is non-zero, this function will only copy (size+1)/2 points into the
 * buffer. These points are the first half of the window if half is negative,
 * and the second half of the window if half is positive. Either half will
 * contain the center point.
 *
 * @param buffer    The buffer to store the window
 * @param size      The window size
 * @param half      Whether to restrict to a half window
 */
void ATK_FillHammingWindow(float* buffer, size_t size, int half) {
    if (size == 0) {
        return;
    }

    size_t amt = size;
    double start = 0;

    if (half < 0) {
        amt = (size+1)/2;
    } else if (half > 0) {
        amt = (size+1)/2;
        start = (double)(size-amt);
    }

    double n;
    double m = (double)(size-1);
    for(size_t ii = 0; ii < amt; ii++) {
        n = ii + start;
        buffer[ii] = (float)(0.54 - (0.46 * cos(2*M_PI*n/m)));
    }
}

/**
 * Returns a newly allocated Hann window.
 *
 * Hann windows are implemented as defined here
 *
 *    https://en.wikipedia.org/wiki/Window_function
 *
 * This function allocates an array that must be freed with ATK_free when it
 * is no longer needed.
 *
 * If half is non-zero, this function allocates a window only (size+1)/2 points.
 * These points are the first half of the window if half is negative, and the
 * second half of the window if half is positive. Either half will contain the
 * center point.
 *
 * @param size  The window size
 * @param half  Whether to restrict to a half window
 *
 * @returns a newly allocated Hann window.
 */
float* ATK_AllocHannWindow(size_t size, SDL_bool half) {
    size_t amt = half ? (size+1)/2 : size;
    float* buffer = ATK_malloc(sizeof(float)*amt);
    if (buffer == NULL) {
        ATK_OutOfMemory();
        return NULL;
    }
    ATK_FillHannWindow(buffer, size, half);
    return buffer;
}


/**
 * Fills the buffer with a Hann window.
 *
 * Hann windows are implemented as defined here
 *
 *    https://en.wikipedia.org/wiki/Window_function
 *
 * The buffer should be able to store size values if it is a full window. If
 * half is non-zero, this function will only copy (size+1)/2 points into the
 * buffer. These points are the first half of the window if half is negative,
 * and the second half of the window if half is positive. Either half will
 * contain the center point.
 *
 * @param buffer    The buffer to store the window
 * @param size      The window size
 * @param half      Whether to restrict to a half window
 */
void ATK_FillHannWindow(float* buffer, size_t size, int half) {
    if (size == 0) {
        return;
    }

    size_t amt = size;
    double start = 0;

    if (half < 0) {
        amt = (size+1)/2;
    } else if (half > 0) {
        amt = (size+1)/2;
        start = (double)(size-amt);
    }

    double n;
    double m = (double)(size-1);
    for(size_t ii = 0; ii < amt; ii++) {
        n = ii + start;
        buffer[ii] = (float)(0.5 - (0.5 * cos(2*M_PI*n/m)));
    }
}

/**
 * Returns a newly allocated Kaiser window.
 *
 * Kaiser windows are implemented as defined here
 *
 *    https://ccrma.stanford.edu/~jos/sasp/Kaiser_Window.html
 *
 * This function allocates an array that must be freed with ATK_free when it
 * is no longer needed.
 *
 * If half is non-zero, this function allocates a window only (size+1)/2 points.
 * These points are the first half of the window if half is negative, and the
 * second half of the window if half is positive. Either half will contain the
 * center point.
 *
 * @param size  The window size
 * @param beta  The window beta parameter
 * @param half  Whether to restrict to a half window
 *
 * @returns a newly allocated Kaiser window.
 */
float* ATK_AllocKaiserWindow(size_t size, float beta, int half) {
    size_t amt = half ? (size+1)/2 : size;
    float* buffer = ATK_malloc(sizeof(float)*amt);
    if (buffer == NULL) {
        ATK_OutOfMemory();
        return NULL;
    }
    ATK_FillKaiserWindow(buffer, size, beta, half);
    return buffer;
}

/**
 * Fills the buffer with a Kaiser window.
 *
 * Kaiser windows are implemented as defined here
 *
 *    https://ccrma.stanford.edu/~jos/sasp/Kaiser_Window.html
 *
 * The buffer should be able to store size values if it is a full window. If
 * half is non-zero, this function will only copy (size+1)/2 points into the
 * buffer. These points are the first half of the window if half is negative,
 * and the second half of the window if half is positive. Either half will
 * contain the center point.
 *
 * @param buffer    The buffer to store the window
 * @param size      The window size
 * @param beta      The window beta parameter
 * @param half      Whether to restrict to a half window
 */
void ATK_FillKaiserWindow(float* buffer, size_t size, float beta, int half) {
    if (size == 0) {
        return;
    }

    size_t amt = size;
    double start = -((size-1)/2.0);

    if (half < 0) {
        amt = (size+1)/2;
    } else if (half > 0) {
        amt = (size+1)/2;
        start = ((size+1) % 2)/2.0;
    }

    double n, factor, kaiser;
    for(size_t ii = 0; ii < amt; ii++) {
        n = start+ii;
        factor = sqrt(1-4*n*n/((size-1)*(size-1)));
        kaiser = bessel(beta * factor,DEFAULT_ERROR) / bessel(beta,DEFAULT_ERROR);
        buffer[ii] = (float)kaiser;
    }
}

#pragma mark -
#pragma mark FFT Blocks
/**
 * The FFT state for a convolution.
 *
 * FFT convolutions are partitioned into blocks, and then combined using
 * overlap-add.
 */
typedef struct ATK_FFTBlock {
    /** The true size of the kernel */
    size_t ksize;
    /** The convolution block size */
    size_t bsize;
    /** The convolution FFT */
    kiss_fftr_cfg fft;
    /** The convolution inverse */
    kiss_fftr_cfg inv;
    /** The left input buffer, of size 2*bsize+2 */
    kiss_fft_scalar* left;
    /** The right input buffer of size 2*bsize+2 */
    kiss_fft_scalar* right;
    /** The output buffer of size bsize+ksize */
    kiss_fft_scalar* outpt;
} ATK_FFTBlock;

/**
 * Returns a newly allocated FFT block
 *
 * The FFT will have block size bsize, unless bsize is 0. In that case
 * it will use ksize.
 *
 * @param ksize The kernel size
 * @param bsize The maximum block size
 *
 * @return a newly allocated FFT block
 */
static ATK_FFTBlock* alloc_fft_block(size_t ksize, size_t bsize) {
    bsize = bsize ? bsize : ksize;
    bsize = kiss_fftr_next_fast_size_real((int)bsize);

    // Prepare for allocation errors
    kiss_fft_scalar* left  = NULL;
    kiss_fft_scalar* right = NULL;
    kiss_fft_scalar* outpt = NULL;
    kiss_fftr_cfg fft = NULL;
    kiss_fftr_cfg inv = NULL;
    ATK_FFTBlock* result = NULL;

    size_t fsize = (2*bsize+2);
    left = ATK_malloc(sizeof(kiss_fft_scalar)*fsize);
    if (left == NULL) {
        goto out;
    }
    memset(left, 0, sizeof(kiss_fft_scalar)*fsize);

    right = ATK_malloc(sizeof(kiss_fft_scalar)*fsize);
    if (right == NULL) {
        goto out;
    }
    memset(right, 0, sizeof(kiss_fft_scalar)*fsize);

    outpt = ATK_malloc(sizeof(kiss_fft_scalar)*(bsize+ksize));
    if (outpt == NULL) {
        goto out;
    }
    memset(outpt, 0, sizeof(kiss_fft_scalar)*(bsize+ksize));

    fft = kiss_fftr_alloc((int)(2*bsize),0,NULL,NULL);
    inv = kiss_fftr_alloc((int)(2*bsize),1,NULL,NULL);

    result = ATK_malloc(sizeof(ATK_FFTBlock));
    if (result == NULL) {
        goto out;
    }

    result->ksize = ksize;
    result->bsize = bsize;
    result->left  = left;
    result->right = right;
    result->outpt = outpt;
    result->fft = fft;
    result->inv = inv;

    return result;

out:
    ATK_OutOfMemory();
    if (result == NULL) {
        ATK_free(result);
    }
    if (fft == NULL) {
        kiss_fft_free(fft);
    }
    if (inv == NULL) {
        kiss_fft_free(inv);
    }
    if (left == NULL) {
        ATK_free(left);
    }
    if (right == NULL) {
        ATK_free(right);
    }
    if (outpt == NULL) {
        ATK_free(outpt);
    }
    return NULL;
}

/**
 * Frees a previously allocated FFT block
 *
 * @param block The FFT block
 */
static void free_fft_block(ATK_FFTBlock* block) {
    if (block == NULL) {
        return;
    }
    ATK_free(block->left);
    block->left = NULL;
    ATK_free(block->right);
    block->right = NULL;
    ATK_free(block->outpt);
    block->outpt = NULL;
    kiss_fft_free(block->fft);
    block->fft = NULL;
    kiss_fft_free(block->inv);
    block->inv = NULL;
    ATK_free(block);
}

/**
 * Performs a convolution of the block with the kernel.
 *
 * The signal is stored (zero-padded) in the left attribute of block. This
 * function breaks up the kernel into block-sized chunks to convolve with this
 * signal. The results are combined into the outpt attribute using overlap-add.
 *
 * @param block     The FFT block
 * @param kernel    The kernel to convolve over
 */
static void convolve_block(ATK_FFTBlock* block, float* kernel) {
    memset(block->outpt,0,sizeof(float)*(block->bsize+block->ksize));
    kiss_fftr(block->fft, block->left, (kiss_fft_cpx*)block->left);

    size_t kpos = 0;
    while (kpos < block->ksize) {
        memcpy(block->right,kernel+kpos,sizeof(float)*block->bsize);
        memset(block->right+block->bsize,0,sizeof(float)*block->bsize);

        kiss_fftr(block->fft, block->right, (kiss_fft_cpx*)block->right);
        ATK_ComplexMult(block->left, block->right, block->right, block->bsize+1);
        kiss_fftri(block->inv, (const kiss_fft_cpx*)block->right, block->right);
        ATK_VecScale(block->right, (float)(1.0/(2*block->bsize)), block->right, 2*block->bsize);

        if (kpos == 0) {
            memcpy(block->outpt,block->right,2*sizeof(float)*block->bsize);
        } else {
            ATK_VecAdd(block->outpt+kpos, block->right, block->outpt+kpos, block->bsize);
            memcpy(block->outpt+kpos+block->bsize,block->right+block->bsize,sizeof(float)*block->bsize);
        }
        kpos += block->bsize;
    }
}

#pragma mark -
#pragma mark Convolutions
/**
 * A one-dimensional (linear) convolution filter.
 *
 * A convolution filter has state. The state consists of the tail of the
 * previously executed convolution. This tail is preserved so that it can be
 * used in the overlap-add portion of the next convolution. This means that it
 * is not safe to use a convolution filter on multiple streams without first
 * reseting it.
 *
 * A convolution filter may be used for either naive or FFT convolutions.
 * Furthermore, it is possible to mix-and-match these algorithms as the tail
 * will be the same in each case. The choice of filter depends on the size
 * of the kernel and/or signal. For optimized code, the break-over point
 * for these buffers can be as high as 512 samples, depending on hardware.
 */
typedef struct ATK_Convolution {
    /** The kernel size */
    size_t ksize;
    /** The block size */
    size_t bsize;
    /** The convolution kernel */
    float* kernel;
    /** The current tail for managing overlap-add */
    float* tail;
    /** Inlining for now */
    ATK_FFTBlock* fft;
} ATK_Convolution;


/**
 * Returns a newly allocated convolution filter for the given kernel.
 *
 * The convolution block size is used to partition the convolution into blocks.
 * If it is zero, the block size will be the same size as the kernel, which
 * means that signal blocks will be padded to match. For best performance in
 * real-time playback, the blocksize should be the same size as the expected
 * signal, which is typically the buffer size of the output device. This can
 * make a significant performance difference on large kernels, such as those
 * used in convolutional reverb.
 *
 * This function will copy the kernel, and not try to acquire ownership of it.
 * Future changes to the kernel will leave this filter unaffected.
 *
 * @param kernel    The convolution kernel
 * @param len       The kernel size
 * @param block     The convolution block size
 *
 * @return a newly allocated convolution filter for the given kernel.
 */
ATK_Convolution* ATK_AllocConvolution(float* kernel, size_t len, size_t block) {
    size_t bsize = block ? block : len;
    bsize = kiss_fftr_next_fast_size_real((int)bsize);

    // Prepare for allocation errors
    float* kern = NULL;
    float* tail = NULL;
    ATK_Convolution* result = NULL;

    kern = ATK_malloc(sizeof(float)*len);
    if (kern == NULL) {
        goto out;
    }
    memcpy(kern, kernel, sizeof(float)*len);

    tail = ATK_malloc(sizeof(float)*len);
    if (tail == NULL) {
        goto out;
    }
    memset(tail, 0, sizeof(float)*len);

    result = ATK_malloc(sizeof(ATK_Convolution));
    if (result == NULL) {
        goto out;
    }

    ATK_FFTBlock* fft = alloc_fft_block(len, bsize);
    if (fft == NULL) {
        goto out;
    }

    result->ksize = len;
    result->bsize = bsize;
    result->tail  = tail;
    result->kernel = kern;
    result->fft = fft;
    return result;

out:
    ATK_OutOfMemory();
    if (result == NULL) {
        ATK_free(result);
    }
    if (kern == NULL) {
        ATK_free(kern);
    }
    if (tail == NULL) {
        ATK_free(tail);
    }
    return NULL;
}

/**
 * Frees a previously allocated convolution filter.
 *
 * @param filter    The convolution filter
 */
void ATK_FreeConvolution(ATK_Convolution* filter) {
    if (filter == NULL) {
        return;
    }
    ATK_free(filter->kernel);
    filter->kernel = NULL;
    ATK_free(filter->tail);
    filter->tail = NULL;
    free_fft_block(filter->fft);
    filter->fft = NULL;
    ATK_free(filter);
}

/**
 * Resets a convolution filter.
 *
 * The internal buffer will be zeroed, reseting the convolution back
 * the beginning.
 *
 * @param filter    The convolution filter
 */
void ATK_ResetConvolution(ATK_Convolution* filter) {
    if (filter == NULL) {
        return;
    }
    memset(filter->tail,0,sizeof(float)*filter->ksize);
}

/**
 * Scales a convolution by the given amount.
 *
 * This scaling factor is applied to the kernel, allowing for normalization
 * before a convolution is applied.
 *
 * @param filter    The convolution filter
 * @param scalar    The amount to scale the convolution
 */
void ATK_ScaleConvolution(ATK_Convolution* filter, float scalar) {
    ATK_VecScale(filter->kernel, scalar, filter->kernel, filter->ksize);
}


/**
 * Returns the size of the convolution kernel.
 *
 * @param filter    The convolution filter
 *
 * @return the size of the convolution kernel.
 */
size_t ATK_GetConvolutionSize(ATK_Convolution* filter) {
    if (filter == NULL) {
        return 0;
    }
    return filter->ksize;
}

/**
 * Returns the next value in this convolution.
 *
 * This steps the convolution ahead by one value. This is a particularly
 * inefficient way to apply a convolution and should be avoided unless
 * completely necessary. Note that this function updates the convolution
 * state, where the state is tail of convolution for use in overlap-add.
 * This means that it is not safe to use a convolution filter on multiple
 * streams without first reseting it.
 *
 * @param filter    The convolution filter
 * @param value     The input value
 *
 * @return the next value in this convolution.
 */
float ATK_StepConvolution(ATK_Convolution* filter, float value) {
    float result = filter->tail[0];
    memmove(filter->tail, filter->tail+1, sizeof(float)*(filter->ksize-1));
    filter->tail[filter->ksize-1] = 0;

    result += value*filter->kernel[0];
    for(size_t ii = 0; ii < filter->ksize-1; ii++) {
        filter->tail[ii] += value*filter->kernel[ii+1];
    }
    return result;
}

/**
 * Completes the convolution, storing the final elements in buffer.
 *
 * At each step, the convolution keeps the tail in its internal state for
 * use in overlap-add. The function finishes the convolution, storing
 * this tail in the provided buffer. The buffer should be able to hold
 * {@link ATK_GetConvolutionSize}-1 elements, which is the size of this
 * tail.
 *
 * Once finished, the convolution filter will be reset and can be safely
 * reused. This function does not deallocate the filter.
 *
 * @param filter    The convolution filter
 * @param buffer    The buffer to store the convolution tail
 *
 * @return the number of elements stored in the buffer.
 */
size_t ATK_FinishConvolution(ATK_Convolution* filter, float* buffer) {
    if (filter == NULL) {
        return 0;
    }
    memcpy(buffer,filter->tail,sizeof(float)*(filter->ksize-1));
    memset(filter->tail,0,sizeof(float)*(filter->ksize));
    return filter->ksize;
}

/**
 * Completes the convolution, storing the final elements in buffer.
 *
 * At each step, the convolution keeps the tail in its internal state for
 * use in overlap-add. The function finishes the convolution, storing
 * this tail in the provided buffer. The buffer should be able to hold
 * {@link ATK_GetConvolutionSize}-1 elements, which is the size of this
 * tail.
 *
 * Once finished, the convolution filter will be reset and can be safely
 * reused. This function does not deallocate the filter.
 *
 * @param filter    The convolution filter
 * @param buffer    The buffer to store the convolution tail
 * @param stride    The buffer stride
 *
 * @return the number of elements stored in the buffer.
 */
size_t ATK_FinishConvolution_stride(ATK_Convolution* filter, float* buffer, size_t stride) {
    if (filter == NULL) {
        return 0;
    }
    ATK_VecCopy_stride(filter->tail,1,buffer,stride,filter->ksize-1);
    memset(filter->tail,0,sizeof(float)*(filter->ksize));
    return filter->ksize;
}

#pragma mark -
#pragma mark Naive Convolutions
/**
 * Applies a naive convolution on the given input, storing it in output.
 *
 * A naive convolution uses an O(nm) nested loop where n is the size of the
 * buffer and m is the size of the convolution. While generally slower, this
 * can be faster than an FFT convolution if either n or m are small.
 *
 * The input and output should both have size len. It is safe for these to be
 * the same buffer.
 *
 * Note that the restriction on size means this function does not place the
 * tail (e.g. the last {@link ATK_GetConvolutionSize}-1 elemetns) of the
 * convolution in output. Instead, it keeps it internally for later use in
 * overlap-add. That way, calling this function twice on two-halves of an
 * array is the same as calling it once on the entire array. This allows us
 * to apply convolutions to streaming data. To access the final tail of the
 * convolution, call {@link ATK_FinishConvolution}.
 *
 * @param filter    The convolution filter
 * @param input     The input values
 * @param output    The buffer to store len output values
 * @param len       The number of elements to process
 */
void ATK_ApplyNaiveConvolution(ATK_Convolution* filter, const float* input,
                               float* output, size_t len) {
    size_t slen = len < filter->ksize ? len : filter->ksize;
    size_t klen = filter->ksize;

    // Naive multiplication
    memset(output,0,len*sizeof(float));
    memcpy(output,filter->tail,slen*sizeof(float));
    memmove(filter->tail,filter->tail+slen,(klen-slen)*sizeof(float));

    size_t remain = klen-slen;
    memset(filter->tail+remain,0,slen*sizeof(float));

    size_t suff;
    size_t thresh = len < klen ? 0 : len-klen;
    const float* krn;
    float* dst;

    const float* src = input;
    for(size_t ii = 0; ii < len; ii++) {
        suff = ii >= thresh ? ii-len+klen : 0;
        dst = output+ii;
        krn = filter->kernel;
        for(size_t jj = 0; jj < klen-suff; jj++) {
            *dst++ += *src*(*krn++);
        }
        krn = filter->kernel+klen-suff;
        dst = filter->tail;
        for(size_t jj = 0; jj < suff; jj++) {
            *dst++ += *src*(*krn++);
        }
        src++;
    }
}

/**
 * Applies a naive convolution on the given input, storing it in output
 *
 * A naive convolution uses an O(nm) nested loop where n is the size of the
 * buffer and m is the size of the convolution. While generally slower, this
 * can be faster than an FFT convolution if either n or m are small.
 *
 * The input and output should both have size len. It is safe for these to be
 * the same buffer, provided that the strides align.
 *
 * Note that the restriction on size means this function does not place the
 * tail (e.g. the last {@link ATK_GetConvolutionSize}-1 elemetns) of the
 * convolution in output. Instead, it keeps it internally for later use in
 * overlap-add. That way, calling this function twice on two-halves of an
 * array is the same as calling it once on the entire array. This allows us
 * to apply convolutions to streaming data. To access the final tail of the
 * convolution, call {@link ATK_FinishConvolution}.
 *
 * @param filter    The convolution filter
 * @param input     The input values
 * @param istride   The data stride of the input buffer
 * @param output    The buffer to store len output values
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to process
 */
void ATK_ApplyNaiveConvolution_stride(ATK_Convolution* filter,
                                      const float* input, size_t istride,
                                      float* output, size_t ostride, size_t len) {
    size_t slen = len < filter->ksize ? len : filter->ksize;
    size_t klen = filter->ksize;

    // Naive multiplication
    memset(output,0,len*sizeof(float));
    ATK_VecCopy_stride(filter->tail, 1, output, ostride, slen);
    memmove(filter->tail,filter->tail+slen,(klen-slen)*sizeof(float));

    size_t remain = klen-slen;
    memset(filter->tail+remain,0,slen*sizeof(float));

    size_t suff;
    size_t thresh = len < klen ? 0 : len-klen;
    const float* krn;
    float* dst;

    const float* src = input;
    for(size_t ii = 0; ii < len; ii++) {
        suff = ii >= thresh ? ii-len+klen : 0;
        dst = output+ii*ostride;
        krn = filter->kernel;
        for(size_t jj = 0; jj < klen-suff; jj++) {
            *dst += *src*(*krn++);
            dst += ostride;
        }
        krn = filter->kernel+klen-suff;
        dst = filter->tail;
        for(size_t jj = 0; jj < suff; jj++) {
            *dst++ += *src*(*krn++);
        }
        src += istride;
    }
}

#pragma mark -
#pragma mark FFT Convolutions
/**
 * Applies an FFT convolution on the given input, storing it in output
 *
 * An FFT convolution breaks the convolution down into several O(n log n) size
 * convolutions where n is the minimum of len and the convolution kernel. This
 * is significantly faster on larger convolutions, those it can be worse than
 * a naive convolution if either len or the kernel are small.
 *
 * The input and output should both have size len. It is safe for these
 * to be the same buffer.
 *
 * Note that the restriction on size means this function does not place the
 * tail (e.g. the last {@link ATK_GetConvolutionSize}-1 elemetns) of the
 * convolution in output. Instead, it keeps it internally for later use in
 * overlap-add. That way, calling this function twice on two-halves of an
 * array is the same as calling it once on the entire array. This allows us
 * to apply convolutions to streaming data. To access the final tail of the
 * convolution, call {@link ATK_FinishConvolution}.
 *
 * @param filter    The convolution filter
 * @param input     The input values
 * @param output    The buffer to store len output values
 * @param len       The number of elements to process
 */
void ATK_ApplyFFTConvolution(ATK_Convolution* filter, const float* input,
                             float* output, size_t len) {
    size_t slen = len < filter->ksize ? len : filter->ksize;
    size_t klen = filter->ksize;

    // Naive multiplication
    memset(output,0,len*sizeof(float));
    memcpy(output,filter->tail,slen*sizeof(float));
    memmove(filter->tail,filter->tail+slen,(klen-slen)*sizeof(float));

    size_t remain = klen-slen;
    memset(filter->tail+remain,0,slen*sizeof(float));

    size_t block = filter->bsize;
    ATK_FFTBlock* fft = filter->fft;

    size_t pos = 0;
    size_t reach, extra, stem;
    size_t limit = block + klen;
    while (pos < len) {
        reach = len - pos;
        stem = reach < fft->bsize ? reach : fft->bsize;
        memset(fft->left,0,sizeof(float)*2*fft->bsize);
        memcpy(fft->left,input+pos,sizeof(float)*stem);
        convolve_block(fft, filter->kernel);

        if (pos + limit > len) {
            stem  = klen <= reach ? klen : reach;
            extra = klen <= limit-reach ? klen : limit-reach;
            ATK_VecAdd(output+pos, fft->outpt, output+pos, stem);
            memcpy(output+pos+stem,fft->outpt+stem, (reach-stem)*sizeof(float));
            ATK_VecAdd(filter->tail,fft->outpt+reach,filter->tail,extra);
        } else {
            ATK_VecAdd(output+pos, fft->outpt, output+pos, klen);
            memcpy(output+pos+klen, fft->outpt+klen, block*sizeof(float));
        }
        pos += block;
    }
}

/**
 * Applies an FFT convolution on the given input, storing it in output.
 *
 * An FFT convolution breaks the convolution down into several O(n log n) size
 * convolutions where n is the minimum of len and the convolution kernel. This
 * is significantly faster on larger convolutions, those it can be worse than
 * a naive convolution if either len or the kernel are small.
 *
 * The input and output should both have size len. It is safe for these to be
 * the same buffer, provided that the strides align.
 *
 * Note that the restriction on size means this function does not place the
 * tail (e.g. the last {@link ATK_GetConvolutionSize}-1 elemetns) of the
 * convolution in output. Instead, it keeps it internally for later use in
 * overlap-add. That way, calling this function twice on two-halves of an
 * array is the same as calling it once on the entire array. This allows us
 * to apply convolutions to streaming data. To access the final tail of the
 * convolution, call {@link ATK_FinishConvolution}.
 *
 * @param filter    The convolution filter
 * @param input     The input values
 * @param istride   The data stride of the input buffer
 * @param output    The buffer to store len output values
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to process
 */
void ATK_ApplyFFTConvolution_stride(ATK_Convolution* filter,
                                    const float* input, size_t istride,
                                    float* output, size_t ostride, size_t len) {

    size_t slen = len < filter->ksize ? len : filter->ksize;
    size_t klen = filter->ksize;

    // Naive multiplication
    ATK_VecClear_stride(output, ostride, len);
    ATK_VecCopy_dstride(filter->tail, output, ostride, len);
    memmove(filter->tail,filter->tail+slen,(klen-slen)*sizeof(float));

    size_t remain = klen-slen;
    memset(filter->tail+remain,0,slen*sizeof(float));

    size_t block = filter->bsize;
    ATK_FFTBlock* fft = filter->fft;

    size_t pos = 0;
    size_t reach, extra, stem;
    size_t limit = block + klen;
    while (pos < len) {
        reach = len - pos;
        stem = reach < fft->bsize ? reach : fft->bsize;
        memset(fft->left,0,sizeof(float)*2*fft->bsize);
        ATK_VecCopy_sstride(input+pos*istride, istride, fft->left, stem);
        convolve_block(fft, filter->kernel);

        if (pos + limit > len) {
            stem  = klen <= reach ? klen : reach;
            extra = klen <= limit-reach ? klen : limit-reach;
            ATK_VecAdd_stride(output+pos*ostride, ostride, fft->outpt, 1,
                              output+pos*ostride, ostride, stem);
            ATK_VecCopy_dstride(fft->outpt+stem, output+(pos+stem)*ostride, ostride, reach);
            ATK_VecAdd(filter->tail,fft->outpt+reach,filter->tail,extra);
        } else {
            ATK_VecAdd_stride(output+pos*ostride, ostride, fft->outpt, 1,
                              output+pos*ostride, ostride, klen);
            ATK_VecCopy_dstride(fft->outpt+stem, output+(pos+stem)*ostride, ostride, block);
        }
        pos += block;
    }
}

#pragma mark -
#pragma mark Cross-Correlation
