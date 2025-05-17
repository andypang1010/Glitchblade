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

/**
 * @file ATK_math.h
 *
 * Header file for the math component of SDL_atk
 *
 * This component contains the math functions for processing large, interleaved
 * data streams (analogous to the vDSP tools in Apple's Accelerate Framework).
 * It supports vectors of both real and complex values, as well as polynomials.
 * The goal of this component is to provide a paired-down analogue of numpy for
 * audio processing. These functions highly benefit from compiling with full
 * optimization turned on.
 */
#ifndef __ATK_MATH_H__
#define __ATK_MATH_H__
#include <SDL.h>
#include <SDL_version.h>
#include <begin_code.h>
#include <math.h>
#include <string.h>

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

#pragma mark -
#pragma mark Distance Utils
/**
 * Returns the distance squared between the arrays adata and bdata
 *
 * The distance is the Euclidean distance as if adata and bdata are len-size vectors.
 *
 * @param adata The first array
 * @param bdata The second array
 * @param len   The number of elements to process
 *
 * @return the distance squared between the arrays adata and bdata
 */
extern DECLSPEC double SDLCALL ATK_VecDistSq(const float* adata, const float* bdata, size_t len);

/**
 * Returns the distance squared between the arrays adata and bdata, accounting for stride
 *
 * The distance is the Euclidean distance as if adata and bdata are len-size vectors.
 *
 * @param adata     The first array
 * @param astride   The stride of the first array
 * @param bdata     The second array
 * @param bstride   The stride of the second array
 * @param len       The number of elements to process
 *
 * @return the distance squared between the arrays adata and bdata, accounting for stride
 */
extern DECLSPEC double SDLCALL ATK_VecDistSq_stride(const float* adata, size_t astride,
                                                    const float* bdata, size_t bstride, size_t len);

/**
 * Returns the distance between the arrays adata and bdata
 *
 * The distance is the Euclidean distance as if adata and bdata are len-size vectors.
 *
 * @param adata The first array
 * @param bdata The second array
 * @param len   The number of elements to process
 *
 * @return the distance between the arrays adata and bdata
 */
extern DECLSPEC double SDLCALL ATK_VecDist(const float* adata, const float* bdata, size_t len);

/**
 * Returns the distance between the arrays adata and bdata, accounting for stride
 *
 * The distance is the Euclidean distance as if adata and bdata are len-size vectors.
 *
 * @param adata     The first array
 * @param astride   The stride of the first array
 * @param bdata     The second array
 * @param bstride   The stride of the second array
 * @param len       The number of elements to process
 *
 * @return the distance between the arrays adata and bdata, accounting for stride
 */
extern DECLSPEC double SDLCALL ATK_VecDist_stride(const float* adata, size_t astride,
                                                  const float* bdata, size_t bstride, size_t len);

/**
 * Returns the absolute difference of arrays adata and bdata
 *
 * The distance is the Manahattan distance as if adata and bdata are len-size vectors.
 * In other words, we compute the sum of the pointwise absolute value.
 *
 * @param adata The first array
 * @param bdata The second array
 * @param len   The number of elements to process
 *
 * @return the absolute difference of arrays adata and bdata
 */
extern DECLSPEC double SDLCALL ATK_VecDiff(const float* adata, const float* bdata, size_t len);

/**
 * Returns the absolute difference of arrays adata and bdata, accounting for stride
 *
 * The distance is the Manahattan distance as if adata and bdata are len-size vectors.
 * In other words, we compute the sum of the pointwise absolute value.
 *
 * @param adata     The first array
 * @param astride   The stride of the first array
 * @param bdata     The second array
 * @param bstride   The stride of the second array
 * @param len       The number of elements to process
 *
 * @return the absolute difference of arrays adata and bdata, accounting for stride
 */
extern DECLSPEC double SDLCALL ATK_VecDiff_stride(const float* adata, size_t astride,
                                                  const float* bdata, size_t bstride, size_t len);

/**
 * Returns the Hamming distance of arrays adata and bdata
 *
 * The hamming distances is the count of the number of different elements. The
 * value epsilon is used for comparisons. That is, to be different, two elements
 * must differ by more than epsilon.
 *
 * @param adata     The first array
 * @param bdata     The second array
 * @param epsilon   The comparion tolerance
 * @param len       The number of elements to process
 *
 * @return the Hamming distance of arrays adata and bdata
 */
extern DECLSPEC size_t SDLCALL ATK_VecHamm(const float* adata, const float* bdata,
                                           float epsilon, size_t len);

/**
 * Returns the Hamming distance of arrays adata and bdata, accounting for stride
 *
 * The hamming distances is the count of the number of different elements. The
 * value epsilon is used for comparisons. That is, to be different, two elements
 * must differ by more than epsilon.
 *
 * @param adata     The first array
 * @param astride   The stride of the first array
 * @param bdata     The second array
 * @param bstride   The stride of the second array
 * @param epsilon   The comparion tolerance
 * @param len       The number of elements to process
 *
 * @return the Hamming distance of arrays adata and bdata, accounting for stride
 */
extern DECLSPEC size_t SDLCALL ATK_VecHamm_stride(const float* adata, size_t astride,
                                                  const float* bdata, size_t bstride,
                                                  float epsilon, size_t len);

#pragma mark -
#pragma mark Min/Max Values
/**
 * Returns the maximum value in the data buffer
 *
 * If the buffer is empty, this returns NaN.
 *
 * @param data      The data buffer
 * @param len       The number of elements to search
 *
 * @return the maximum value in the data buffer
 */
extern DECLSPEC float SDLCALL ATK_VecMax(float* data, size_t len);

/**
 * Returns the maximum value in the data buffer
 *
 * The value is searched using the given stride. Elements outside the given
 * stride are ignored. If the buffer is empty, this function returns NaN.
 *
 * @param data      The data buffer
 * @param stride    The data stride
 * @param len       The number of elements to search
 *
 * @return the maximum value in the data buffer
 */
extern DECLSPEC float SDLCALL ATK_VecMax_stride(float* data, size_t stride, size_t len);

/**
 * Returns the index of the maximum value in the data buffer
 *
 * The maximum value is stored in the given pointer if it is not NULL. If the buffer is
 * empty, this returns Npos (-1).
 *
 * @param data      The data buffer
 * @param len       The number of elements to search
 * @param max       Optional pointer to store the maximum
 *
 * @return the maximum value in the data buffer
 */
extern DECLSPEC size_t SDLCALL ATK_VecMaxIndex(float* data, size_t len, float* max);

/**
 * Returns the index of the maximum value in the data buffer
 *
 * The value is both searched and returned using the given stride. Elements outside
 * the given stride are ignored. For example if the stride is 2 and the maximum value
 * of the buffer is at (absolute) position 8, with a second greatest value at 3,
 * then this function will return 1 (e.g. absolute position 3).
 *
 * The maximum value is stored in the given pointer if it is not NULL. If the buffer is
 * empty, this returns Npos (-1).
 *
 * @param data      The data buffer
 * @param stride    The data stride
 * @param len       The number of elements to search
 * @param max       Optional pointer to store the maximum
 *
 * @return the maximum value in the data buffer
 */
extern DECLSPEC size_t SDLCALL ATK_VecMaxIndex_stride(float* data, size_t stride, size_t len, float* max);

/**
 * Returns the maximum magnitude in the data buffer
 *
 * The maximum magnitude is the maximum absolute value of the buffer. If the buffer is
 * empty, this returns NaN.
 *
 * @param data      The data buffer
 * @param len       The number of elements to search
 *
 * @return the maximum value in the data buffer
 */
extern DECLSPEC float SDLCALL ATK_VecMaxMag(float* data, size_t len);

/**
 * Returns the maximum magnitude in the data buffer
 *
 * The magnitude is searched using the given stride. Elements outside of the stride
 * are ignored. If the buffer is empty, this returns NaN.
 *
 * @param data      The data buffer
 * @param stride    The data stride
 * @param len       The number of elements to search
 *
 * @return the maximum value in the data buffer
 */
extern DECLSPEC float SDLCALL ATK_VecMaxMag_stride(float* data, size_t stride, size_t len);

/**
 * Returns the index of the maximum magnitude in the data buffer
 *
 * The maximum magnitude is stored in the given pointer if it is not NULL. If the buffer
 * is empty, this returns Npos (-1).
 *
 * @param data      The data buffer
 * @param len       The number of elements to search
 * @param max       Optional pointer to store the maximum
 *
 * @return the maximum value in the data buffer
 */
extern DECLSPEC size_t SDLCALL ATK_VecMaxMagIndex(float* data, size_t len, float* max);

/**
 * Returns the index of the maximum magnitude in the data buffer
 *
 * The magnitude is both searched and returned using the given stride. Elements outside
 * the given stride are ignored. For example if the stride is 2 and the maximum value
 * of the buffer is at (absolute) position 8, with a second greatest value at 3,
 * then this function will return 1 (e.g. absolute position 3).
 *
 * The maximum magnitude is stored in the given pointer if it is not NULL. If the
 * buffer is empty, this function returns Npos (-1).
 *
 *
 * @param data      The data buffer
 * @param stride    The data stride
 * @param len       The number of elements to search
 * @param max       Optional pointer to store the maximum
 *
 * @return the maximum value in the data buffer
 */
extern DECLSPEC size_t SDLCALL ATK_VecMaxMagIndex_stride(float* data, size_t stride, size_t len, float* max);

/**
 * Returns the minimum value in the data buffer
 *
 * If the buffer is empty, this returns NaN.
 *
 * @param data      The data buffer
 * @param len       The number of elements to search
 *
 * @return the minimum value in the data buffer
 */
extern DECLSPEC float SDLCALL ATK_VecMin(float* data, size_t len);

/**
 * Returns the minimum value in the data buffer
 *
 * The value is searched using the given stride. Elements outside the given
 * stride are ignored. If the buffer is empty, this function returns NaN.
 *
 * @param data      The data buffer
 * @param stride    The data stride
 * @param len       The number of elements to search
 *
 * @return the minimum value in the data buffer
 */
extern DECLSPEC float SDLCALL ATK_VecMin_stride(float* data, size_t stride, size_t len);

/**
 * Returns the index of the minimum value in the data buffer
 *
 * The minimum value is stored in the given pointer if it is not NULL. If the buffer is
 * empty, this returns Npos (-1).
 *
 * @param data      The data buffer
 * @param len       The number of elements to search
 * @param min       Optional pointer to store the minimum
 *
 * @return the minimum value in the data buffer
 */
extern DECLSPEC size_t SDLCALL ATK_VecMinIndex(float* data, size_t len, float* min);

/**
 * Returns the index of the minimum value in the data buffer
 *
 * The value is both searched and returned using the given stride. Elements outside
 * the given stride are ignored. For example if the stride is 2 and the minimum value
 * of the buffer is at (absolute) position 8, with a second least value at 3,
 * then this function will return 1 (e.g. absolute position 3).
 *
 * The minimum value is stored in the given pointer if it is not NULL. If the buffer is
 * empty, this returns Npos (-1).
 *
 * @param data      The data buffer
 * @param stride    The data stride
 * @param len       The number of elements to search
 * @param min       Optional pointer to store the maximum
 *
 * @return the minimum value in the data buffer
 */
extern DECLSPEC size_t SDLCALL ATK_VecMinIndex_stride(float* data, size_t stride, size_t len, float* min);

/**
 * Returns the minimum magnitude in the data buffer
 *
 * The minimum magnitude is the maximum absolute value of the buffer. If the buffer is
 * empty, this returns NaN.
 *
 * @param data      The data buffer
 * @param len       The number of elements to search
 *
 * @return the minimum value in the data buffer
 */
extern DECLSPEC float SDLCALL ATK_VecMinMag(float* data, size_t len);

/**
 * Returns the minimum magnitude in the data buffer
 *
 * The magnitude is searched using the given stride. Elements outside of the stride
 * are ignored. If the buffer is empty, this returns NaN.
 *
 * @param data      The data buffer
 * @param stride    The data stride
 * @param len       The number of elements to search
 *
 * @return the minimum value in the data buffer
 */
extern DECLSPEC float SDLCALL ATK_VecMinMag_stride(float* data, size_t stride, size_t len);

/**
 * Returns the index of the minimum magnitude in the data buffer
 *
 * The minimum magnitude is stored in the given pointer if it is not NULL. If the buffer
 * is empty, this returns Npos (-1).
 *
 * @param data      The data buffer
 * @param len       The number of elements to search
 * @param min       Optional pointer to store the maximum
 *
 * @return the minimum value in the data buffer
 */
extern DECLSPEC size_t SDLCALL ATK_VecMinMagIndex(float* data, size_t len, float* min);

/**
 * Returns the index of the minimum value in the data buffer
 *
 * The magnitude is both searched and returned using the given stride. Elements outside
 * the given stride are ignored. For example if the stride is 2 and the minimum value
 * of the buffer is at (absolute) position 8, with a second least value at 3,
 * then this function will return 1 (e.g. absolute position 3).
 *
 * The minimum magnitude is stored in the given pointer if it is not NULL. If the buffer
 * is empty, this returns Npos (-1).
 *
 * @param data      The data buffer
 * @param stride    The data stride
 * @param len       The number of elements to search
 * @param min       Optional pointer to store the maximum
 *
 * @return the minimum value in the data buffer
 */
extern DECLSPEC size_t SDLCALL ATK_VecMinMagIndex_stride(float* data, size_t stride, size_t len, float* min);


#pragma mark -
#pragma mark Stream Memcpy Utils
// Ordering of dst/src chosen to be compatible with rest of the module
/**
 * Copies contents of src into dst
 *
 * This function DOES NOT check whether dst and src overlap, and it is generally
 * unsafe to use the function in that case.
 *
 * @param src       The copy source
 * @param dst       The copy destination
 * @param len       The number of elements to copy
 */
static inline void ATK_VecCopy(const float* src, float* dst, size_t len) {
    memcpy(dst,src,len*sizeof(float));
}

/**
 * Copies contents of src into dst obeying the specified strides
 *
 * The function will copy len element from src, starting at the first location
 * and skipping every sstride elements. It will be copied into dst, starting
 * at the first location and skipping every dstride elements.
 *
 * This function DOES NOT check whether dst and src overlap, and it is generally
 * unsafe to use the function in that case.
 *
 * @param src       The copy source
 * @param sstride   The source stride
 * @param dst       The copy destination
 * @param dstride   The destination stride
 * @param len       The number of elements to copy
 */
extern DECLSPEC void SDLCALL ATK_VecCopy_stride(const float* src, size_t sstride,
                                                float* dst, size_t dstride, size_t len);

/**
 * Copies contents of src into dst obeying the specified strides
 *
 * The function will copy len adjacent element from src. It will be copied
 * into dst, starting at the first location and skipping every dstride elements.
 *
 * This function DOES NOT check whether dst and src overlap, and it is generally
 * unsafe to use the function in that case.
 *
 * @param src       The copy source
 * @param dst       The copy destination
 * @param dstride   The destination stride
 * @param len       The number of elements to copy
 */
extern DECLSPEC void SDLCALL ATK_VecCopy_dstride(const float* src,
                                                 float* dst, size_t dstride,
                                                 size_t len);

/**
 * Copies contents of src into dst obeying the specified strides
 *
 * The function will copy len element from src, starting at the first location
 * and skipping every sstride elements. It will be copied into dst as an
 * adjacent array of elements.
 *
 * This function DOES NOT check whether dst and src overlap, and it is generally
 * unsafe to use the function in that case.
 *
 * @param src       The copy source
 * @param sstride   The source stride
 * @param dst       The copy destination
 * @param len       The number of elements to copy
 */
extern DECLSPEC void SDLCALL ATK_VecCopy_sstride(const float* src, size_t sstride,
                                                 float* dst, size_t len);

/**
 * Swaps the first len elements of adata and bdata
 *
 * This function DOES NOT check whether adata and bdata overlap, and it is generally
 * unsafe to use the function in that case (except when they are identical).
 *
 * @param adata The first array to swap
 * @param bdata The second array to swap
 * @param len   The number of elements to swap
 */
extern DECLSPEC void SDLCALL ATK_VecSwap(float* adata, float* bdata, size_t len);

/**
 * Swaps the first len elements of adata and bdata with the given strides.
 *
 * Only elements at the specified strides are swapped. Elements of adata or
 * bdata that are outside the stride will be unaffected.
 *
 * This function DOES NOT check whether dst and src overlap, and it is generally
 * unsafe to use the function in that case.
 *
 * @param adata     The first array to swap
 * @param astride   The stride of the first array
 * @param bdata     The second array to swap
 * @param bstride   The stride of the first array
 * @param len       The number of elements to swap
 */
extern DECLSPEC void SDLCALL ATK_VecSwap_stride(float* adata, size_t astride,
                                                float* bdata, size_t bstride,
                                                size_t len);

/**
 * Reverses the input, storing the result in output.
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param src   The input buffer to reverse
 * @param dst   The buffer to store the result
 * @param len   The number of elements to reverse
 */
extern DECLSPEC void SDLCALL ATK_VecReverse(const float* src, float* dst, size_t len);

/**
 * Reverses the input, storing the result in output.
 *
 * It is safe for output to be the same as one of the two input buffers,
 * provided that the strides match up. However, this function does not
 * check that this is the case.
 *
 * @param src       The input buffer to reverse
 * @param sstride   The data stride of the input buffer
 * @param dst       The buffer to store the result
 * @param dstride   The data stride of the output buffer
 * @param len       The number of elements to reverse
 */
extern DECLSPEC void SDLCALL ATK_VecReverse_stride(const float* src, size_t sstride,
                                                   float* dst, size_t dstride, size_t len);

/**
 * Rotates input left (amt > 0) or right (amt < 0), storing the result in output.
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param src   The input buffer to roll
 * @param amt   The amount to roll left (<0) or right (>0)
 * @param dst   The buffer to store the result
 * @param len   The number of elements to process
 */
extern DECLSPEC void SDLCALL ATK_VecRoll(const float* src, Sint64 amt, float* dst, size_t len);


/**
 * Rotates input left (amt > 0) or right (amt < 0), storing the result in output.
 *
 * It is safe for output to be the same as one of the two input buffers,
 * provided that the strides match up. However, this function does not
 * check that this is the case.
 *
 * @param src       The input buffer to reverse
 * @param sstride   The data stride of the input buffer
 * @param amt       The amount to roll left (<0) or right (>0)
 * @param dst       The buffer to store the result
 * @param dstride   The data stride of the output buffer
 * @param len       The number of elements to process
 */
extern DECLSPEC void SDLCALL ATK_VecRoll_stride(const float* src, size_t sstride, Sint64 amt,
                                                float* dst, size_t dstride, size_t len);


#pragma mark -
#pragma mark Stream Filling
/**
 * Fills the given buffer with 0s
 *
 * This function is effectively a version of memset dedicated to floats.
 *
 * @param data      the buffer to clear
 * @param len       the buffer length
 */
extern DECLSPEC void SDLCALL ATK_VecClear(float* data, size_t len);

/**
 * Fills the given buffer with 0s at the given stride
 *
 * This function is effectively a stride-aware version of memset.
 *
 * @param data      the buffer to clear
 * @param stride    the buffer stride
 * @param len       the buffer length
 */
extern DECLSPEC void SDLCALL ATK_VecClear_stride(float* data, size_t stride, size_t len);

/**
 * Fills the given buffer with the value
 *
 * This function is effectively a version of memset dedicated to floats.
 *
 * @param data      the buffer to fill
 * @param value     the value to fill with
 * @param len       the buffer length
 */
extern DECLSPEC void SDLCALL ATK_VecFill(float* data, float value, size_t len);

/**
 * Fills the given buffer with the value at the given stride
 *
 * This function is effectively a stride-aware version of memset.
 *
 * @param data      the buffer to fill
 * @param stride    the buffer stride
 * @param value     the value to fill with
 * @param len       the buffer length
 */
extern DECLSPEC void SDLCALL ATK_VecFill_stride(float* data, size_t stride, float value, size_t len);


/**
 * Fills the given buffer with a linear ramp from start to stop.
 *
 * The first element of data will be start, while the last will be stop. All elements
 * in-between will be equidistant.
 *
 * @param data      the buffer to fill
 * @param start     the initial value
 * @param stop      the final value
 * @param len       the buffer length
 */
extern DECLSPEC void SDLCALL ATK_VecRamp(float* data, float start, float stop, size_t len);

/**
 * Fills the given buffer with a linear ramp from start to stop.
 *
 * The first element of data will be start, while the last will be stop. All elements
 * in-between (with respect to the given stride) will be equidistant.
 *
 * @param data      the buffer to fill
 * @param stride    the buffer stride
 * @param start     the initial value
 * @param stop      the final value
 * @param len       the buffer length
 */
extern DECLSPEC void SDLCALL ATK_VecRamp_stride(float* data, size_t stride, float start, float stop, size_t len);

#pragma mark -
#pragma mark Stream Absolute Value
/**
 * Outputs the absolute value of the input buffer.
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_VecAbs(const float* input, float* output, size_t len);

/**
 * Outputs the absolute value of the input buffer.
 *
 * It is safe for output to be the same as the input buffer, provided
 * that the strides match up. However, this function does not check
 * that this is the case.
 *
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_VecAbs_stride(const float* input, size_t istride,
                                               float* output, size_t ostride, size_t len);

/**
 * Outputs the negative absolute value of the input buffer.
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_VecAbsNeg(const float* input, float* output, size_t len);

/**
 * Outputs the negative absolute value of the input buffer.
 *
 * It is safe for output to be the same as the input buffer, provided
 * that the strides match up. However, this function does not check
 * that this is the case.
 *
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_VecAbsNeg_stride(const float* input, size_t istride,
                                                  float* output, size_t ostride, size_t len);

#pragma mark -
#pragma mark Stream Arithmetic
/**
 * Outputs the negative value of the input buffer.
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_VecNeg(const float* input, float* output, size_t len);

/**
 * Outputs the negative value of the input buffer.
 *
 * It is safe for output to be the same as the input buffer, provided
 * that the strides match up. However, this function does not check
 * that this is the case.
 *
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_VecNeg_stride(const float* input, size_t istride,
                                               float* output, size_t ostride, size_t len);

/**
 * Outputs the inverse value of the input buffer.
 *
 * For values that are 0, the inverse will also be 0.
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_VecInv(const float* input, float* output, size_t len);

/**
 * Outputs the negative value of the input buffer.
 *
 * For values that are 0, the inverse will also be 0.
 *
 * It is safe for output to be the same as the input buffer, provided
 * that the strides match up. However, this function does not check
 * that this is the case.
 *
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_VecInv_stride(const float* input, size_t istride,
                                               float* output, size_t ostride, size_t len);

/**
 * Adds two input buffers together, storing the result in output
 *
 * It is safe for output to be the same as one of the two input buffers.
 *
 * @param input1    The first input buffer
 * @param input2    The second input buffer
 * @param output    The output buffer
 * @param len       The number of elements to add
 */
extern DECLSPEC void SDLCALL ATK_VecAdd(const float* input1, const float* input2,
                                        float* output, size_t len);

/**
 * Adds two buffers together, storing the result in output
 *
 * It is safe for output to be the same as one of the two input buffers,
 * provided that the strides match up. However, this function does not
 * check that this is the case.
 *
 * @param input1    The first input buffer
 * @param istride1  The data stride of the first input buffer
 * @param input2    The second input buffer
 * @param istride2  The data stride of the second input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to add
 */
extern DECLSPEC void SDLCALL ATK_VecAdd_stride(const float* input1, size_t istride1,
                                               const float* input2, size_t istride2,
                                               float* output, size_t ostride, size_t len);

/**
 * Subtracts the second buffer from the first, storing the result in output
 *
 * It is safe for output to be the same as one of the two input buffers.
 *
 * @param input1    The first input buffer
 * @param input2    The second input buffer
 * @param output    The output buffer
 * @param len       The number of elements to add
 */
extern DECLSPEC void SDLCALL ATK_VecSub(const float* input1, const float* input2,
                                        float* output, size_t len);

/**
 * Subtracts the second buffer from the first, storing the result in output
 *
 * It is safe for output to be the same as one of the two input buffers,
 * provided that the strides match up. However, this function does not
 * check that this is the case.
 *
 * @param input1    The first input buffer
 * @param istride1  The data stride of the first input buffer
 * @param input2    The second input buffer
 * @param istride2  The data stride of the second input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to add
 */
extern DECLSPEC void SDLCALL ATK_VecSub_stride(const float* input1, size_t istride1,
                                               const float* input2, size_t istride2,
                                               float* output, size_t ostride, size_t len);

/**
 * Multiplies two buffers together, storing the result in output
 *
 * It is safe for output to be the same as one of the two input buffers.
 *
 * @param input1    The first input buffer
 * @param input2    The second input buffer
 * @param output    The output buffer
 * @param len       The number of elements to multiply
 */
extern DECLSPEC void SDLCALL ATK_VecMult(const float* input1, const float* input2,
                                         float* output, size_t len);

/**
 * Multiplies two buffers together, storing the result in output
 *
 * It is safe for output to be the same as one of the two input buffers,
 * provided that the strides match up. However, this function does not
 * check that this is the case.
 *
 * @param input1    The first input buffer
 * @param istride1  The data stride of the first input buffer
 * @param input2    The second input buffer
 * @param istride2  The data stride of the second input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to multiply
 */
extern DECLSPEC void SDLCALL ATK_VecMult_stride(const float* input1, size_t istride1,
                                                const float* input2, size_t istride2,
                                                float* output, size_t ostride, size_t len);

/**
 * Divides the first buffer by the second, storing the result in output
 *
 * Division is pointwise. If a dividend entry is 0, the result of that
 * particular division will be zero. This makes division the same as
 * multiplying by {@link ATK_VecInv}.
 *
 * It is safe for output to be the same as one of the two input buffers.
 *
 * @param input1    The first input buffer
 * @param input2    The second input buffer
 * @param output    The output buffer
 * @param len       The number of elements to multiply
 */
extern DECLSPEC void SDLCALL ATK_VecDiv(const float* input1, const float* input2,
                                        float* output, size_t len);

/**
 * Divides the first buffer by the second, storing the result in output
 *
 * Division is pointwise. If a dividend entry is 0, the result of that
 * particular division will be zero. This makes division the same as
 * multiplying by {@link ATK_VecInv}.
 *
 * It is safe for output to be the same as one of the two input buffers,
 * provided that the strides match up. However, this function does not
 * check that this is the case.
 *
 * @param input1    The first input buffer
 * @param istride1  The data stride of the first input buffer
 * @param input2    The second input buffer
 * @param istride2  The data stride of the second input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to multiply
 */
extern DECLSPEC void SDLCALL ATK_VecDiv_stride(const float* input1, size_t istride1,
                                               const float* input2, size_t istride2,
                                               float* output, size_t ostride, size_t len);

/**
 * Scales an input buffer, storing the result in output
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param input     The input buffer
 * @param scalar    The scalar to mutliply by
 * @param output    The output buffer
 * @param len       The number of elements to multiply
 */
extern DECLSPEC void SDLCALL ATK_VecScale(const float* input, float scalar, float* output, size_t len);

/**
 * Scales an input buffer, storing the result in output
 *
 * It is safe for output to be the same as the input buffer, provided
 * that the strides match up. However, this function does not check
 * that this is the case.
 *
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param scalar    The scalar to mutliply by
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to multiply
 */
extern DECLSPEC void SDLCALL ATK_VecScale_stride(const float* input, size_t istride, float scalar,
                                                 float* output, size_t ostride, size_t len);

/**
 * Scales an input buffer and adds it to another, storing the result in output
 *
 * It is safe for output to be the same as one of the two input buffers.
 *
 * @param input1    The first input buffer
 * @param input2    The second input buffer
 * @param scalar    The scalar to mutliply input1 by
 * @param output    The output buffer
 * @param len       The number of elements to process
 */
extern DECLSPEC void SDLCALL ATK_VecScaleAdd(const float* input1, const float* input2, float scalar,
                                             float* output, size_t len);

/**
 * Scales an input buffer and adds it to another, storing the result in output
 *
 * It is safe for output to be the same as one of the two input buffers,
 * provided that the strides match up. However, this function does not
 * check that this is the case.
 *
 * @param input1    The first input buffer
 * @param istride1  The data stride of the first input buffer
 * @param input2    The second input buffer
 * @param istride2  The data stride of the second input buffer
 * @param scalar    The scalar to mutliply input1 by
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to add
 */
extern DECLSPEC void SDLCALL ATK_VecScaleAdd_stride(const float* input1, size_t istride1,
                                                    const float* input2, size_t istride2, float scalar,
                                                    float* output, size_t ostride, size_t len);

#pragma mark -
#pragma mark Stream Clipping
/**
 * Clips the input buffer to the range [min,max]
 *
 * Values above max will be assigned to max and values below min will be
 * assigned to min.
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param input     The input buffer
 * @param min       The minimum allowed value
 * @param max       The maximum allowed value
 * @param output    The output buffer
 * @param len       The number of elements to clip
 */
extern DECLSPEC void SDLCALL ATK_VecClip(const float* input, float min, float max,
                                         float* output, size_t len);

/**
 * Clips the input buffer to the range [min,max]
 *
 * Values above max will be assigned to max and values below min will be
 * assigned to min.
 *
 * It is safe for output to be the same as the input buffer, provided
 * that the strides match up. However, this function does not check
 * that this is the case.
 *
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param min       The minimum allowed value
 * @param max       The maximum allowed value
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to clip
 */
extern DECLSPEC void SDLCALL ATK_VecClip_stride(const float* input, size_t istride,
                                                float min, float max,
                                                float* output, size_t ostride, size_t len);

/**
 * Clips the input buffer to the range [min,max]
 *
 * Values outside of the range will be assigned 0.
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param input     The input buffer
 * @param min       The minimum allowed value
 * @param max       The maximum allowed value
 * @param output    The output buffer
 * @param len       The number of elements to clip
 */
extern DECLSPEC void SDLCALL ATK_VecClipZero(const float* input, float min, float max,
                                             float* output, size_t len);

/**
 * Clips the input buffer to the range [min,max]
 *
 * Values outside of the range will be assigned 0.
 *
 * It is safe for output to be the same as the input buffer, provided
 * that the strides match up. However, this function does not check
 * that this is the case.
 *
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param min       The minimum allowed value
 * @param max       The maximum allowed value
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to clip
 */
extern DECLSPEC void SDLCALL ATK_VecClipZero_stride(const float* input, size_t istride,
                                                    float min, float max,
                                                    float* output, size_t ostride, size_t len);

/**
 * Clips the input buffer to the range [min,max], counting the number clipped
 *
 * Values above max will be assigned to max and values below min will be
 * assigned to min. The parameters mincnt and maxcnt store the number clipped
 * if they are not null.
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param input     The input buffer
 * @param min       The minimum allowed value
 * @param max       The maximum allowed value
 * @param output    The output buffer
 * @param len       The number of elements to clip
 * @param mincnt    Optional pointer to store the number clipped to min
 * @param maxcnt    Optional pointer to store the number clipped to max
 */
extern DECLSPEC void SDLCALL ATK_VecClipCount(const float* input, float min, float max,
                                              float* output, size_t len,
                                              size_t* mincnt, size_t* maxcnt);

/**
 * Clips the input buffer to the range [min,max], counting the number clipped
 *
 * Values above max will be assigned to max and values below min will be
 * assigned to min. The parameters mincnt and maxcnt store the number clipped
 * if they are not null.
 *
 * It is safe for output to be the same as the input buffer, provided
 * that the strides match up. However, this function does not check
 * that this is the case.
 *
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param min       The minimum allowed value
 * @param max       The maximum allowed value
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to clip
 * @param mincnt    Optional pointer to store the number clipped to min
 * @param maxcnt    Optional pPointer to store the number clipped to max
 */
extern DECLSPEC void SDLCALL ATK_VecClipCount_stride(const float* input, size_t istride,
                                                     float min, float max,
                                                     float* output, size_t ostride, size_t len,
                                                     size_t* mincnt, size_t* maxcnt);
/**
 * Soft clips the input buffer to the range [-bound,bound]
 *
 * The clip is a soft knee. Values in the range [-knee, knee] are not
 * affected. Values outside this range are asymptotically clipped to the
 * range [-bound,bound] with the formula
 *
 *     y = (bound*x - bound*knee+knee*knee)/x
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param input     The input buffer
 * @param bound     The asymptotic bound
 * @param knee      The soft knee bound
 * @param output    The output buffer
 * @param len       The number of elements to clip
 */
extern DECLSPEC void SDLCALL ATK_VecClipKnee(const float* input, float bound, float knee,
                                             float* output, size_t len);

/**
 * Soft clips the input buffer to the range [-bound,bound]
 *
 * The clip is a soft knee. Values in the range [-knee, knee] are not
 * affected. Values outside this range are asymptotically clipped to the
 * range [-bound,bound] with the formula
 *
 *     y = (bound*x - bound*knee+knee*knee)/x
 *
 * It is safe for output to be the same as the input buffer, provided
 * that the strides match up. However, this function does not check
 * that this is the case.
 *
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param bound     The asymptotic bound
 * @param knee      The soft knee bound
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to clip
 */
extern DECLSPEC void SDLCALL ATK_VecClipKnee_stride(const float* input, size_t istride,
                                                    float bound, float knee,
                                                    float* output, size_t ostride, size_t len);

/**
 * Clips the input buffer to be outside the range [min,max]
 *
 * Values inside of the range will be assigned min if they are negative and
 * max if they non-negative.
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param input     The input buffer
 * @param min       The minimum excluded value
 * @param max       The maximum excluded value
 * @param output    The output buffer
 * @param len       The number of elements to clip
 */
extern DECLSPEC void SDLCALL ATK_VecExclude(const float* input,
                                            float min, float max,
                                            float* output, size_t len);

/**
 * Clips the input buffer to be outside the range [min,max]
 *
 * Values inside of the range will be assigned min if they are negative and
 * max if they non-negative.
 *
 * It is safe for output to be the same as the input buffer, provided
 * that the strides match up. However, this function does not check
 * that this is the case.
 *
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param min       The minimum excluded value
 * @param max       The maximum excluded value
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to clip
 */
extern DECLSPEC void SDLCALL ATK_VecExclude_stride(const float* input, size_t istride,
                                                   float min, float max,
                                                   float* output, size_t ostride, size_t len);

/**
 * Clips the input buffer to be above min.
 *
 * Values less than min are assigned min.
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param input     The input buffer
 * @param min       The minimum permitted value
 * @param output    The output buffer
 * @param len       The number of elements to clip
 *
 * @return the number of elements clipped
 */
extern DECLSPEC size_t SDLCALL ATK_VecThreshold(const float* input, float min,
                                                float* output, size_t len);

/**
 * Clips the input buffer to be above min.
 *
 * Values less than min are assigned min.
 *
 * It is safe for output to be the same as the input buffer, provided
 * that the strides match up. However, this function does not check
 * that this is the case.
 *
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param min       The minimum permitted value
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to clip
 *
 * @return the number of elements clipped
 */
extern DECLSPEC size_t SDLCALL ATK_VecThreshold_stride(const float* input, size_t istride, float min,
                                                       float* output, size_t ostride, size_t len);

/**
 * Invert clips the input buffer to be above min.
 *
 * Values less than min are inverted (assigned their negative).
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param input     The input buffer
 * @param min       The minimum permitted value
 * @param output    The output buffer
 * @param len       The number of elements to clip
 *
 * @return the number of elements clipped
 */
extern DECLSPEC size_t SDLCALL ATK_VecThresholdInvert(const float* input, float min,
                                                      float* output, size_t len);

/**
 * Invert clips the input buffer to be above min.
 *
 * Values less than min are inverted (assigned their negative)
 *
 * It is safe for output to be the same as the input buffer, provided
 * that the strides match up. However, this function does not check
 * that this is the case.
 *
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param min       The minimum permitted value
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to clip
 *
 * @return the number of elements clipped
 */
extern DECLSPEC size_t SDLCALL ATK_VecThresholdInvert_stride(const float* input, size_t istride, float min,
                                                             float* output, size_t ostride, size_t len);

/**
 * Clips the input buffer to a signed constant relative to a threshold.
 *
 * Values greater than or equal to min are assigned scalar, while all
 * others are assigned -scalar.
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param input     The input buffer
 * @param min       The minimum permitted value
 * @param scalar    The signed output value
 * @param output    The output buffer
 * @param len       The number of elements to clip
 */
extern DECLSPEC void SDLCALL ATK_VecThresholdSign(const float* input, float min, float scalar,
                                                  float* output, size_t len);

/**
 * Clips the input buffer to a signed constant relative to a threshold.
 *
 * Values greater than or equal to min are assigned scalar, while all
 * others are assigned -scalar.
 *
 * It is safe for output to be the same as the input buffer, provided
 * that the strides match up. However, this function does not check
 * that this is the case.
 *
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param min       The minimum permitted value
 * @param scalar    The signed output value
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to clip
 */
extern DECLSPEC void SDLCALL ATK_VecThresholdSign_stride(const float* input, size_t istride,
                                                         float min, float scalar,
                                                         float* output, size_t ostride, size_t len);

/**
 * Clips the input buffer to be below max.
 *
 * Values greater than max are assigned max.
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param input     The input buffer
 * @param max       The maximum permitted value
 * @param output    The output buffer
 * @param len       The number of elements to clip
 *
 * @return the number of elements clipped
 */
extern DECLSPEC size_t SDLCALL ATK_VecLimit(const float* input, float max,
                                            float* output, size_t len);

/**
 * Clips the input buffer to be below max.
 *
 * Values greater than max are assigned max.
 *
 * It is safe for output to be the same as the input buffer, provided
 * that the strides match up. However, this function does not check
 * that this is the case.
 *
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param max       The maximum permitted value
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to clip
 *
 * @return the number of elements clipped
 */
extern DECLSPEC size_t SDLCALL ATK_VecLimit_stride(const float* input, size_t istride, float max,
                                                   float* output, size_t ostride, size_t len);

/**
 * Invert clips the input buffer to be below max.
 *
 * Values greater than max are inverted (assigned their negative).
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param input     The input buffer
 * @param max       The maximum permitted value
 * @param output    The output buffer
 * @param len       The number of elements to clip
 *
 * @return the number of elements clipped
 */
extern DECLSPEC size_t SDLCALL ATK_VecLimitInvert(const float* input, float max,
                                                  float* output, size_t len);

/**
 * Invert clips the input buffer to be below max.
 *
 * Values greater than max are inverted (assigned their negative).
 *
 * It is safe for output to be the same as the input buffer, provided
 * that the strides match up. However, this function does not check
 * that this is the case.
 *
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param max       The maximum permitted value
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to clip
 *
 * @return the number of elements clipped
 */
extern DECLSPEC size_t SDLCALL ATK_VecLimitInvert_stride(const float* input, size_t istride, float max,
                                                         float* output, size_t ostride, size_t len);

/**
 * Clips the input buffer to a signed constant relative to a limit.
 *
 * Values less than or equal to max are assigned scalar, while all
 * others are assigned -scalar.
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param input     The input buffer
 * @param max       The maximum permitted value
 * @param scalar    The signed output value
 * @param output    The output buffer
 * @param len       The number of elements to clip
 */
extern DECLSPEC void SDLCALL ATK_VecLimitSign(const float* input, float max, float scalar,
                                              float* output, size_t len);

/**
 * Clips the input buffer to a signed constant relative to a limit.
 *
 * Values less than or equal to max are assigned scalar, while all
 * others are assigned -scalar.
 *
 * It is safe for output to be the same as the input buffer, provided
 * that the strides match up. However, this function does not check
 * that this is the case.
 *
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param max       The maximum permitted value
 * @param scalar    The signed output value
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to clip
 */
extern DECLSPEC void SDLCALL ATK_VecLimitSign_stride(const float* input, size_t istride,
                                                     float max, float scalar,
                                                     float* output, size_t ostride, size_t len);

#pragma mark -
#pragma mark Aggregation
/**
 * Returns the sum of the elements in input.
 *
 * @param input     The input buffer
 * @param len       The number of elements to sum
 *
 * @return the sum of the elements in input.
 */
extern DECLSPEC float SDLCALL ATK_VecSum(const float* input, size_t len);

/**
 * Returns the sum of the elements in input, accounting for stride.
 *
 * Elements outside of the stride will not be summed.
 *
 * @param input     The input buffer
 * @param stride    The input stride
 * @param len       The number of elements to sum
 *
 * @return the sum of the elements in input, accounting for stride.
 */
extern DECLSPEC float SDLCALL ATK_VecSum_stride(const float* input, size_t stride, size_t len);

/**
 * Returns the sum of the absolute value of the input.
 *
 * @param input     The input buffer
 * @param len       The number of elements to sum
 *
 * @return the sum of the absolute value of the input.
 */
extern DECLSPEC float SDLCALL ATK_VecSumMag(const float* input, size_t len);

/**
 * Returns the sum of the absolute value of the input, accounting for stride.
 *
 * Elements outside of the stride will not be summed.
 *
 * @param input     The input buffer
 * @param stride    The input stride
 * @param len       The number of elements to sum
 *
 * @return the sum of the absolute value of the input, accounting for stride.
 */
extern DECLSPEC float SDLCALL ATK_VecSumMag_stride(const float* input, size_t stride, size_t len);

/**
 * Returns the sum of squares of the input.
 *
 * @param input     The input buffer
 * @param len       The number of elements to sum
 *
 * @return the sum of squares of the input.
 */
extern DECLSPEC float SDLCALL ATK_VecSumSq(const float* input, size_t len);

/**
 * Returns the sum of squares of the input, accounting for stride.
 *
 * Elements outside of the stride will not be summed.
 *
 * @param input     The input buffer
 * @param stride    The input stride
 * @param len       The number of elements to sum
 *
 * @return the sum of squares of the input, accounting for stride.
 */
extern DECLSPEC float SDLCALL ATK_VecSumSq_stride(const float* input, size_t stride, size_t len);

/**
 * Returns the average of the elements in input.
 *
 * @param input     The input buffer
 * @param len       The number of elements to average
 *
 * @return the average of the elements in input.
 */
extern DECLSPEC float SDLCALL ATK_VecAverage(const float* input, size_t len);

/**
 * Returns the average of the elements in input, accounting for stride.
 *
 * Elements outside of the stride will not be averaged.
 *
 * @param input     The input buffer
 * @param stride    The input stride
 * @param len       The number of elements to average
 *
 * @return the average of the elements in input, accounting for stride.
 */
extern DECLSPEC float SDLCALL ATK_VecAverage_stride(const float* input, size_t stride, size_t len);

/**
 * Returns the (arithmetic) mean square of input.
 *
 * This value is typically used to measure the power of an audio signal.
 *
 * @param input     The input buffer
 * @param len       The number of elements to include
 *
 * @return the (arithmetic) mean square of input.
 */
extern DECLSPEC float SDLCALL ATK_VecMeanSq(const float* input, size_t len);

/**
 * Returns the (arithmetic) mean square of input, accounting for stride.
 *
 * This value is typically used to measure the power of an audio signal.
 * Elements outside of the stride will not contribute to this value.
 *
 * @param input     The input buffer
 * @param stride    The input stride
 * @param len       The number of elements to include
 *
 * @return the (arithmetic) mean square of input, accounting for stride.
 */
extern DECLSPEC float SDLCALL ATK_VecMeanSq_stride(const float* input, size_t stride, size_t len);

/**
 * Return the standard deviation of input.
 *
 * @param input     The input buffer
 * @param len       The number of elements to include
 *
 * @return the standard deviation of input.
 */
extern DECLSPEC float SDLCALL ATK_VecStdDev(const float* input, size_t len);

/**
 * Return the standard deviation of input, accounting for stride.
 *
 * Elements outside of the stride will not be contribute to the value.
 *
 * @param input     The input buffer
 * @param stride    The input stride
 * @param len       The number of elements to include
 *
 * @return the standard deviation of input, accounting for stride.
 */
extern DECLSPEC float SDLCALL ATK_VecStdDev_stride(const float* input, size_t stride, size_t len);

#pragma mark -
#pragma mark Stream Interpolation
/**
 * Computes the linear interpolation of input1 and input2
 *
 * The elements of the output vector will be (1-factor)*a+factor*b where
 * a is from input1 and b is from input2.
 *
 * It is safe for output to be the same as one of the two input buffers.
 *
 * @param input1    The first input buffer
 * @param input2    The second input buffer
 * @param factor    The interpolation factor
 * @param output    The output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_VecInterp(const float* input1, const float* input2,
                                           float factor, float* output, size_t len);

/**
 * Computes the linear interpolation of input1 and input2
 *
 * The elements of the output vector will be (1-t)*a+t*b where a is from
 * input1 and b is from input2.
 *
 * It is safe for output to be the same as one of the two input buffers,
 * provided that the strides match up. However, this function does not
 * check that this is the case.
 *
 * @param input1    The first input buffer
 * @param istride1  The data stride of the first input buffer
 * @param input2    The second input buffer
 * @param istride2  The data stride of the second input buffer
 * @param factor    The interpolation factor
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_VecInterp_stride(const float* input1, size_t istride1,
                                                  const float* input2, size_t istride2, float factor,
                                                  float* output, size_t ostride, size_t len);

/**
 * Computes the pairwise linear interpolation of input, using factors
 *
 * For each element of b of factors, the integer portion t = trunc(b)
 * must refer to a position in input. The output value is the interpolation
 *
 *     input[t]*(1-b+t)+input[t+1]*(b-t)
 *
 * Values at the boundaries will be zero clamped.
 *
 * It is generally NOT safe for the output buffer to be the same as the
 * input buffer
 *
 * @param input     The input buffer
 * @param factors   The factor array
 * @param output    The output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_VecPairInterp(const float* input, const float* factors,
                                               float* output, size_t len);

/**
 * Computes the pairwise linear interpolation of input, using factors
 *
 * For each element of b of factors, the integer portion t = trunc(b)
 * must refer to a position in input relative to the given stride. The
 * output value is the interpolation
 *
 *     input[istride*t]*(1-b+t)+input[istride*(t+1)]*(b-t)
 *
 * Values at the boundaries will be zero clamped.
 *
 * It is safe for output to be the same as one of the two input buffers,
 * provided that the strides match up. However, this function does not
 * check that this is the case.
 *
 * It is generally NOT safe for the output buffer to be the same as the
 * input buffer
 *
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param factors   The factor array
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_VecPairInterp_stride(const float* input, size_t istride, const float* factors,
                                                      float* output, size_t ostride, size_t len);

#pragma mark -
#pragma mark Fader Support
/**
 * Scales an input buffer, storing the result in output
 *
 * The scalar is a sliding factor linearly interpolated between start to end.
 * It will use start for the first element of input and end for the last
 * element. This effect is used to create smooth fade-outs.
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param input     The input buffer
 * @param start     The initial scalar value
 * @param end       The final scalar value
 * @param output    The output buffer
 * @param len       The number of elements to multiply
 */
extern DECLSPEC void SDLCALL ATK_VecSlide(const float* input, float start, float end,
                                          float* output, size_t len);

/**
 * Scales an input buffer, storing the result in output
 *
 * The scalar is a sliding factor linearly interpolated between start to end.
 * It will use start for the first element of input and end for the last
 * element. This effect is used to create smooth fade-outs.
 *
 * It is safe for output to be the same as the input buffer, provided
 * that the strides match up. However, this function does not check
 * that this is the case.
 *
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param start     The initial scalar value
 * @param end       The final scalar value
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to multiply
 */
extern DECLSPEC void SDLCALL ATK_VecSlide_stride(const float* input, size_t istride,
                                                 float start, float end,
                                                 float* output, size_t ostride, size_t len);

/**
 * Scales an input buffer and adds it to another, storing the result in output
 *
 * The scalar is a sliding factor linearly interpolated between start to end.
 * It will use start for the first element of input1 and end for the last
 * element. This effect is used to create smooth fade-outs.
 *
 * It is safe for output to be the same as one of the two input buffers.
 *
 * @param input1    The first input buffer
 * @param input2    The second input buffer
 * @param start     The initial scalar value
 * @param end       The final scalar value
 * @param output    The output buffer
 * @param len       The number of elements to process
 */
extern DECLSPEC void SDLCALL ATK_VecSlideAdd(const float* input1, const float* input2,
                                             float start, float end, float* output, size_t len);

/**
 * Scales an input signal and adds it to another, storing the result in output
 *
 * The scalar is a sliding factor linearly interpolated between start to end.
 * It will use start for the first element of input1 and end for the last
 * element. This effect is used to create smooth fade-outs.
 *
 * It is safe for output to be the same as one of the two input buffers,
 * provided that the strides match up. However, this function does not
 * check that this is the case.
 *
 * @param input1    The first input buffer
 * @param istride1  The data stride of the first input buffer
 * @param input2    The second input buffer
 * @param istride2  The data stride of the second input buffer
 * @param start     The initial scalar value
 * @param end       The final scalar value
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to process
 */
extern DECLSPEC void SDLCALL ATK_VecSlideAdd_stride(const float* input1, size_t istride1,
                                                    const float* input2, size_t istride2,
                                                    float start, float end,
                                                    float* output, size_t ostride, size_t len);

#pragma mark -
#pragma mark Stream Misc
/**
 * Converts values from amplitude/power to decibels.
 *
 * The value zero is considered the reference value for zero decibels
 * (as decibels are unitless by themselves). As power and amplitude
 * produce slightly different results, this function has a flag to
 * specify the one desired.
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param input     The input buffer
 * @param zero      The zero decibel value
 * @param power     Whether to use power (not amplitude)
 * @param output    The output buffer
 * @param len       The number of elements to convert
 */
extern DECLSPEC void SDLCALL ATK_VecPowAmpToDecib(const float* input, float zero, SDL_bool power,
                                                  float* output, size_t len);

/**
 * Converts values from amplitude (power) to decibels.
 *
 * The value zero is considered the reference value for zero decibels
 * (as decibels are unitless by themselves). As power and amplitude
 * produce slightly different results, this function has a flag to
 * specify the one desired.
 *
 * It is safe for output to be the same as the input buffer, provided
 * that the strides match up. However, this function does not check
 * that this is the case.
 *
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param zero      The zero decibel value
 * @param output    The output buffer
 * @param power     Whether to use power (not amplitude)
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to convert
 */
extern DECLSPEC void SDLCALL ATK_VecPowAmpToDecib_stride(const float* input, size_t istride,
                                                         float zero, SDL_bool power,
                                                         float* output, size_t ostride, size_t len);

/**
 * Converts values from decibels to amplitude/power.
 *
 * The value zero is considered the reference value for zero decibels
 * (as decibels are unitless by themselves). As power and amplitude
 * produce slightly different results, this function has a flag to
 * specify the one desired.
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param input     The input buffer
 * @param zero      The zero decibel value
 * @param power     Whether to use power (not amplitude)
 * @param output    The output buffer
 * @param len       The number of elements to convert
 */
extern DECLSPEC void SDLCALL ATK_VecDecibToPowAmp(const float* input, float zero, SDL_bool power,
                                                  float* output, size_t len);

/**
 * Converts values from decibels to amplitude/power.
 *
 * The value zero is considered the power for zero decibels (as decibels
 * are unitless by themselves). As power and amplitude produce slightly
 * different results, this function has a flag to specify the one desired.
 *
 * It is safe for output to be the same as the input buffer, provided
 * that the strides match up. However, this function does not check
 * that this is the case.
 *
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param zero      The zero decibel value
 * @param power     Whether to use power (not amplitude)
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to convert
 */
extern DECLSPEC void SDLCALL ATK_VecDecibToPowAmp_stride(const float* input, size_t istride,
                                                         float zero, SDL_bool power,
                                                         float* output, size_t ostride, size_t len);

/**
 * Returns the number of zero-crossings found
 *
 * If the pointer last is not NULL, it will store the index of the last
 * zero-crossing (defined as the end of the crossing). If the number of
 * zero-crossings exceeds max, all zero-crossings after the max one will
 * be ignored.
 *
 * If last is not NULL it will store the index of the last zero-crossing
 * found.
 *
 * @param input     The input buffer
 * @param max       The maximum number of zero-crossings to find
 * @param len       The number of elements in input
 * @param last      Optional pointer to store index of last zero-crossing
 *
 * @return the number of zero-crossings found
 */
extern DECLSPEC size_t SDLCALL ATK_VecZeroCross(const float* input, size_t max,
                                                size_t len, size_t* last);

/**
 * Returns the number of zero-crossings found
 *
 * If the pointer last is not NULL, it will store the index of the last
 * zero-crossing (defined as the end of the crossing). If the number of
 * zero-crossings exceeds max, all zero-crossings after the max one will
 * be ignored.
 *
 * If last is not NULL it will store the index of the last zero-crossing
 * found. This value will be relative to the stride, and is not an absolute
 * index into the buffer.
 *
 * @param input     The input buffer
 * @param stride    The data stride of the input buffer
 * @param max       The maximum number of zero-crossings to find
 * @param len       The number of elements in input
 * @param last      Optional pointer to store index of last zero-crossing
 *
 * @return the number of zero-crossings found
 */
extern DECLSPEC size_t SDLCALL ATK_VecZeroCross_stride(const float* input,size_t stride,
                                                       size_t max, size_t len, size_t* last);

#pragma mark -
#pragma mark Stream De/Interleaving
/**
 * Interleaves the inputs into a single flat stream
 *
 * The nested array input is assumed to be stride streams of length len.
 * These are interleaved into output in order. The array order must have
 * length stride*len.
 *
 * @param input     The array of input streams
 * @param stride    The number of input streams (e.g. output stride)
 * @param output    The flat output stream
 * @param len       The number of elements in the streams
 */
extern DECLSPEC void SDLCALL ATK_VecInterleave(const float** input, size_t stride,
                                               float* output, size_t len);

/**
 * Separates the interleaved input into multiple streams
 *
 * The output should be an array of stride arrays, each individual array
 * of length len. The elements will be stored in output according to
 * their order in input.
 *
 * @param input     The input stream
 * @param stride    The input stream stride
 * @param output    The array of output streams
 * @param len       The number of elements in the streams
 */
extern DECLSPEC void SDLCALL ATK_VecDeinterleave(const float* input, size_t stride,
                                                 float** output, size_t len);

/**
 * Flattens interleaved input by summing it together.
 *
 * The input should have length len*stride, as it contains stride streams of
 * equal length. The output should have length len, as it will store the sum
 * of the elements in input.
 *
 * @param input     The input stream
 * @param stride    The input stream stride
 * @param output    The flattened output stream
 * @param len       The number of elements in the output.
 */
extern DECLSPEC void SDLCALL ATK_VecFlatten(const float* input, size_t stride,
                                            float* output, size_t len);


#pragma mark -
#pragma mark Complex Numbers
/**
 * Outputs the norm of the complex numbers in the input buffer.
 *
 * The input buffer is assumed to consist of complex numbers represented
 * by (interleaved) float pairs. So all even positions are reals and all
 * odd positions are imaginary. The len is the number of complex numbers
 * in the buffer, and is hence half the size of the buffer.
 *
 * The output buffer will consist only of reals and should have size len.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param len       The number of complex numbers in the buffer
 */
extern DECLSPEC void SDLCALL ATK_ComplexNorm(const float* input, float* output, size_t len);

/**
 * Outputs the norm of the complex numbers in the input buffer.
 *
 * The input buffer is assumed to consist of complex numbers represented by
 * (interleaved) float pairs. The stride is applied to the complex numbers,
 * not the components. So if a buffer has stride 3, all positions at multiples
 * of 6 are real, followed by an imaginary at the next position. The len is
 * the number of complex numbers in the buffer, and is hence half the number
 * of elements in the buffer.
 *
 * The output buffer will consist only of reals and should have size ostride*len.
 *
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_ComplexNorm_stride(const float* input, size_t istride,
                                                    float* output, size_t ostride, size_t len);

/**
 * Outputs the square of the norm of the complex numbers in the input buffer.
 *
 * The input buffer is assumed to consist of complex numbers represented
 * by (interleaved) float pairs. So all even positions are reals and all
 * odd positions are imaginary. The len is the number of complex numbers
 * in the buffer, and is hence half the size of the buffer.
 *
 * The output buffer will consist only of reals and should have size len.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param len       The number of complex numbers in the buffer
 */
extern DECLSPEC void SDLCALL ATK_ComplexNormSq(const float* input, float* output, size_t len);

/**
 * Outputs the square of the norm of the complex numbers in the input buffer.
 *
 * The input buffer is assumed to consist of complex numbers represented by
 * (interleaved) float pairs. The stride is applied to the complex numbers,
 * not the components. So if a buffer has stride 3, all positions at multiples
 * of 6 are real, followed by an imaginary at the next position. The len is
 * the number of complex numbers in the buffer, and is hence half the number
 * of elements in the buffer.
 *
 * The output buffer will consist only of reals and should have size ostride*len.
 *
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_ComplexNormSq_stride(const float* input, size_t istride,
                                                      float* output, size_t ostride, size_t len);

/**
 * Outputs the conjugates of the complex numbers in the input buffer.
 *
 * The input buffer (and output) is assumed to consist of complex numbers
 * represented by (interleaved) float pairs. So all even positions are
 * reals and all odd positions are imaginary. The len is the number of
 * complex numbers in the buffer, and is hence half the size of the buffer.
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param len       The number of complex numbers in the buffer
 */
extern DECLSPEC void SDLCALL ATK_ComplexConj(const float* input, float* output, size_t len);

/**
 * Outputs the conjugates of the complex numbers in the input buffer.
 *
 * The input (and output) buffer is assumed to consist of complex numbers
 * represented by (interleaved) float pairs. The stride is applied to the
 * complex numbers, not the components. So if a buffer has stride 3, all
 * positions at multiples of 6 are real, followed by an imaginary at the
 * next position. The len is the number of complex numbers in the buffer,
 * and is hence half the number of elements in the buffer.
 *
 * It is safe for output to be the same as the input buffer, provided
 * that the strides match up. However, this function does not check
 * that this is the case.
 *
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_ComplexConj_stride(const float* input, size_t istride,
                                                    float* output, size_t ostride, size_t len);

/**
 * Outputs the angles of the complex numbers in the input buffer.
 *
 * Angles are measured in radians clockwise from the x-axis.
 *
 * The input buffer is assumed to consist of complex numbers represented
 * by (interleaved) float pairs. So all even positions are reals and all
 * odd positions are imaginary. The len is the number of complex numbers
 * in the buffer, and is hence half the size of the buffer.
 *
 * The output buffer will consist only of reals and should have size len.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param len       The number of complex numbers in the buffer
 */
extern DECLSPEC void SDLCALL ATK_ComplexAngle(const float* input, float* output, size_t len);

/**
 * Outputs the angles of the complex numbers in the input buffer.
 *
 * Angles are measured in radians clockwise from the x-axis.
 *
 * The input buffer is assumed to consist of complex numbers represented by
 * (interleaved) float pairs. The stride is applied to the complex numbers,
 * not the components. So if a buffer has stride 3, all positions at multiples
 * of 6 are real, followed by an imaginary at the next position. The len is
 * the number of complex numbers in the buffer, and is hence half the number
 * of elements in the buffer.
 *
 * The output buffer will consist only of reals and should have size ostride*len.
 *
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_ComplexAngle_stride(const float* input, size_t istride,
                                                     float* output, size_t ostride, size_t len);

/**
 * Rotates the complex numbers in the input buffer by the given angle.
 *
 * The angle of rotation is measured (in radians) counter clockwise.
 *
 * The input buffer (and output) is assumed to consist of complex numbers
 * represented by (interleaved) float pairs. So all even positions are
 * reals and all odd positions are imaginary. The len is the number of
 * complex numbers in the buffer, and is hence half the size of the buffer.
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param input     The input buffer
 * @param angle     The counterclockwise angle of rotation in radians
 * @param output    The output buffer
 * @param len       The number of complex numbers in the buffer
 */
extern DECLSPEC void SDLCALL ATK_ComplexRot(const float* input, float angle, float* output, size_t len);

/**
 * Rotates the complex numbers in the input buffer by the given angle.
 *
 * The angle of rotation is measured (in radians) counter clockwise.
 *
 * The input (and output) buffer is assumed to consist of complex numbers
 * represented by (interleaved) float pairs. The stride is applied to the
 * complex numbers, not the components. So if a buffer has stride 3, all
 * positions at multiples of 6 are real, followed by an imaginary at the
 * next position. The len is the number of complex numbers in the buffer,
 * and is hence half the number of elements in the buffer.
 *
 * It is safe for output to be the same as the input buffer, provided
 * that the strides match up. However, this function does not check
 * that this is the case.
 *
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param angle     The counterclockwise angle of rotation in radians
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_ComplexRot_stride(const float* input, size_t istride, float angle,
                                                   float* output, size_t ostride, size_t len);

/**
 * Outputs the negative value of the input buffer.
 *
 * The input buffer (and output) is assumed to consist of complex numbers
 * represented by (interleaved) float pairs. So all even positions are
 * reals and all odd positions are imaginary. The len is the number of
 * complex numbers in the buffer, and is hence half the size of the buffer.
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param len       The number of elements in the buffers
 */
static inline void ATK_ComplexNeg(const float* input, float* output, size_t len) {
    ATK_VecNeg(input,output,len*2);
}

/**
 * Outputs the negative value of the input buffer.
 *
 * The input (and output) buffer is assumed to consist of complex numbers
 * represented by (interleaved) float pairs. The stride is applied to the
 * complex numbers, not the components. So if a buffer has stride 3, all
 * positions at multiples of 6 are real, followed by an imaginary at the
 * next position. The len is the number of complex numbers in the buffer,
 * and is hence half the number of elements in the buffer.
 *
 * It is safe for output to be the same as the input buffer, provided
 * that the strides match up. However, this function does not check
 * that this is the case.
 *
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_ComplexNeg_stride(const float* input, size_t istride,
                                                   float* output, size_t ostride, size_t len);

/**
 * Outputs the inverse value of the input buffer.
 *
 * For values that are 0, the inverse will also be 0.
 *
 * The input buffer (and output) is assumed to consist of complex numbers
 * represented by (interleaved) float pairs. So all even positions are
 * reals and all odd positions are imaginary. The len is the number of
 * complex numbers in the buffer, and is hence half the size of the buffer.
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_ComplexInv(const float* input, float* output, size_t len);

/**
 * Outputs the negative value of the input buffer.
 *
 * For values that are 0, the inverse will also be 0.
 *
 * The input (and output) buffer is assumed to consist of complex numbers
 * represented by (interleaved) float pairs. The stride is applied to the
 * complex numbers, not the components. So if a buffer has stride 3, all
 * positions at multiples of 6 are real, followed by an imaginary at the
 * next position. The len is the number of complex numbers in the buffer,
 * and is hence half the number of elements in the buffer.
 *
 * It is safe for output to be the same as the input buffer, provided
 * that the strides match up. However, this function does not check
 * that this is the case.
 *
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_ComplexInv_stride(const float* input, size_t istride,
                                                   float* output, size_t ostride, size_t len);

/**
 * Adds two input buffers together, storing the result in output
 *
 * The input buffer (and output) is assumed to consist of complex numbers
 * represented by (interleaved) float pairs. So all even positions are
 * reals and all odd positions are imaginary. The len is the number of
 * complex numbers in the buffer, and is hence half the size of the buffer.
 *
 * It is safe for output to be the same as one of the two input buffers.
 *
 * @param input1    The first input buffer
 * @param input2    The second input buffer
 * @param output    The output buffer
 * @param len       The number of elements to add
 */
static inline void ATK_ComplexAdd(const float* input1, const float* input2, float* output, size_t len) {
    ATK_VecAdd(input1,input2,output,len*2);
}

/**
 * Adds two buffers together, storing the result in output
 *
 * The input (and output) buffer is assumed to consist of complex numbers
 * represented by (interleaved) float pairs. The stride is applied to the
 * complex numbers, not the components. So if a buffer has stride 3, all
 * positions at multiples of 6 are real, followed by an imaginary at the
 * next position. The len is the number of complex numbers in the buffer,
 * and is hence half the number of elements in the buffer.
 *
 * It is safe for output to be the same as one of the two input buffers,
 * provided that the strides match up. However, this function does not
 * check that this is the case.
 *
 * @param input1    The first input buffer
 * @param istride1  The data stride of the first input buffer
 * @param input2    The second input buffer
 * @param istride2  The data stride of the second input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to add
 */
extern DECLSPEC void SDLCALL ATK_ComplexAdd_stride(const float* input1, size_t istride1,
                                                   const float* input2, size_t istride2,
                                                   float* output, size_t ostride, size_t len);

/**
 * Subtracts the second buffer from the first, storing the result in output
 *
 * The input buffer (and output) is assumed to consist of complex numbers
 * represented by (interleaved) float pairs. So all even positions are
 * reals and all odd positions are imaginary. The len is the number of
 * complex numbers in the buffer, and is hence half the size of the buffer.
 *
 * It is safe for output to be the same as one of the two input buffers.
 *
 * @param input1    The first input buffer
 * @param input2    The second input buffer
 * @param output    The output buffer
 * @param len       The number of elements to add
 */
static inline void ATK_ComplexSub(const float* input1, const float* input2, float* output, size_t len) {
    ATK_VecSub(input1,input2,output,len*2);
}


/**
 * Subtracts the second buffer from the first, storing the result in output
 *
 * The input (and output) buffer is assumed to consist of complex numbers
 * represented by (interleaved) float pairs. The stride is applied to the
 * complex numbers, not the components. So if a buffer has stride 3, all
 * positions at multiples of 6 are real, followed by an imaginary at the
 * next position. The len is the number of complex numbers in the buffer,
 * and is hence half the number of elements in the buffer.
 *
 * It is safe for output to be the same as one of the two input buffers,
 * provided that the strides match up. However, this function does not
 * check that this is the case.
 *
 * @param input1    The first input buffer
 * @param istride1  The data stride of the first input buffer
 * @param input2    The second input buffer
 * @param istride2  The data stride of the second input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to add
 */
extern DECLSPEC void SDLCALL ATK_ComplexSub_stride(const float* input1, size_t istride1,
                                                   const float* input2, size_t istride2,
                                                   float* output, size_t ostride, size_t len);

/**
 * Multiplies two buffers together, storing the result in output
 *
 * The input buffer (and output) is assumed to consist of complex numbers
 * represented by (interleaved) float pairs. So all even positions are
 * reals and all odd positions are imaginary. The len is the number of
 * complex numbers in the buffer, and is hence half the size of the buffer.
 *
 * It is safe for output to be the same as one of the two input buffers.
 *
 * @param input1    The first input buffer
 * @param input2    The second input buffer
 * @param output    The output buffer
 * @param len       The number of elements to multiply
 */
extern DECLSPEC void SDLCALL ATK_ComplexMult(const float* input1, const float* input2,
                                             float* output, size_t len);

/**
 * Multiplies two buffers together, storing the result in output
 *
 * The input (and output) buffer is assumed to consist of complex numbers
 * represented by (interleaved) float pairs. The stride is applied to the
 * complex numbers, not the components. So if a buffer has stride 3, all
 * positions at multiples of 6 are real, followed by an imaginary at the
 * next position. The len is the number of complex numbers in the buffer,
 * and is hence half the number of elements in the buffer.
 *
 * It is safe for output to be the same as one of the two input buffers,
 * provided that the strides match up. However, this function does not
 * check that this is the case.
 *
 * @param input1    The first input buffer
 * @param istride1  The data stride of the first input buffer
 * @param input2    The second input buffer
 * @param istride2  The data stride of the second input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to multiply
 */
extern DECLSPEC void SDLCALL ATK_ComplexMult_stride(const float* input1, size_t istride1,
                                                    const float* input2, size_t istride2,
                                                    float* output, size_t ostride, size_t len);

/**
 * Scales an input buffer, storing the result in output
 *
 * The input buffer (and output) is assumed to consist of complex numbers
 * represented by (interleaved) float pairs. So all even positions are
 * reals and all odd positions are imaginary. The len is the number of
 * complex numbers in the buffer, and is hence half the size of the buffer.
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param input     The input buffer
 * @param real      The real component of the scalar
 * @param imag      The imaginary component of the scalar
 * @param output    The output buffer
 * @param len       The number of elements to multiply
 */
extern DECLSPEC void SDLCALL ATK_ComplexScale(const float* input, float real, float imag,
                                              float* output, size_t len);

/**
 * Scales an input buffer, storing the result in output
 *
 * The input (and output) buffer is assumed to consist of complex numbers
 * represented by (interleaved) float pairs. The stride is applied to the
 * complex numbers, not the components. So if a buffer has stride 3, all
 * positions at multiples of 6 are real, followed by an imaginary at the
 * next position. The len is the number of complex numbers in the buffer,
 * and is hence half the number of elements in the buffer.
 *
 * It is safe for output to be the same as the input buffer, provided
 * that the strides match up. However, this function does not check
 * that this is the case.
 *
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param real      The real component of the scalar
 * @param imag      The imaginary component of the scalar
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to multiply
 */
extern DECLSPEC void SDLCALL ATK_ComplexScale_stride(const float* input, size_t istride,
                                                     float real, float imag,
                                                     float* output, size_t ostride, size_t len);

/**
 * Divides the first buffer by the second, storing the result in output
 *
 * Division is (complex) pointwise. If a dividend entry is 0, the result
 * of that particular division will be zero. This makes division the same
 * as multiplying by {@link ATK_ComplexInv}.
 *
 * The input buffer (and output) is assumed to consist of complex numbers
 * represented by (interleaved) float pairs. So all even positions are
 * reals and all odd positions are imaginary. The len is the number of
 * complex numbers in the buffer, and is hence half the size of the buffer.
 *
 * It is safe for output to be the same as one of the two input buffers.
 *
 * @param input1    The first input buffer
 * @param input2    The second input buffer
 * @param output    The output buffer
 * @param len       The number of elements to multiply
 */
extern DECLSPEC void SDLCALL ATK_ComplexDiv(const float* input1, const float* input2,
                                            float* output, size_t len);

/**
 * Divides the first buffer by the second, storing the result in output
 *
 * Division is (complex) pointwise. If a dividend entry is 0, the result of that
 * particular division will be zero. This makes division the same as
 * multiplying by {@link ATK_ComplexInv}.
 *
 * The input (and output) buffer is assumed to consist of complex numbers
 * represented by (interleaved) float pairs. The stride is applied to the
 * complex numbers, not the components. So if a buffer has stride 3, all
 * positions at multiples of 6 are real, followed by an imaginary at the
 * next position. The len is the number of complex numbers in the buffer,
 * and is hence half the number of elements in the buffer.
 *
 * It is safe for output to be the same as one of the two input buffers,
 * provided that the strides match up. However, this function does not
 * check that this is the case.
 *
 * @param input1    The first input buffer
 * @param istride1  The data stride of the first input buffer
 * @param input2    The second input buffer
 * @param istride2  The data stride of the second input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to multiply
 */
extern DECLSPEC void SDLCALL ATK_ComplexDiv_stride(const float* input1, size_t istride1,
                                                   const float* input2, size_t istride2,
                                                   float* output, size_t ostride, size_t len);

/**
 * Scales an input buffer and adds it to another, storing the result in output
 *
 * The input buffer (and output) is assumed to consist of complex numbers
 * represented by (interleaved) float pairs. So all even positions are
 * reals and all odd positions are imaginary. The len is the number of
 * complex numbers in the buffer, and is hence half the size of the buffer.
 *
 * It is safe for output to be the same as one of the two input buffers.
 *
 * @param input1    The first input buffer
 * @param input2    The second input buffer
 * @param real      The real component of the scalar
 * @param imag      The imaginary component of the scalar
 * @param output    The output buffer
 * @param len       The number of elements to process
 */
extern DECLSPEC void SDLCALL ATK_ComplexScaleAdd(const float* input1, const float* input2,
                                                 float real, float imag,
                                                 float* output, size_t len);

/**
 * Scales an input buffer and adds it to another, storing the result in output
 *
 * The input (and output) buffer is assumed to consist of complex numbers
 * represented by (interleaved) float pairs. The stride is applied to the
 * complex numbers, not the components. So if a buffer has stride 3, all
 * positions at multiples of 6 are real, followed by an imaginary at the
 * next position. The len is the number of complex numbers in the buffer,
 * and is hence half the number of elements in the buffer.
 *
 * It is safe for output to be the same as one of the two input buffers,
 * provided that the strides match up. However, this function does not
 * check that this is the case.
 *
 * @param input1    The first input buffer
 * @param istride1  The data stride of the first input buffer
 * @param input2    The second input buffer
 * @param istride2  The data stride of the second input buffer
 * @param real      The real component of the scalar
 * @param imag      The imaginary component of the scalar
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to add
 */
extern DECLSPEC void SDLCALL ATK_ComplexScaleAdd_stride(const float* input1, size_t istride1,
                                                        const float* input2, size_t istride2,
                                                        float real, float imag,
                                                        float* output, size_t ostride, size_t len);

#pragma mark -
#pragma mark Polynomials
/** Default maximum number of iterations for Bairstow's method */
#define ATK_BAIRSTOW_ITERATIONS 50

/** Default maximum number of attempts to find a root */
#define ATK_BAIRSTOW_ATTEMPTS   10

/**
 * The user preferences for Bairstow's method
 *
 * Bairstow's method is an approximate root finding technique. The value
 * epsilon is the error value for all of the roots. A good description
 * of Bairstow's method can be found here:
 *
 *    http://nptel.ac.in/courses/122104019/numerical-analysis/Rathish-kumar/ratish-1/f3node9.html
 *
 * It uses some randomness, but it can be controlled by the user-supplied
 * generator. A good rule of thumb is a maximum of 10 attempts and 50
 * iterations. Though many applications will converge long before this.
 */
typedef struct ATK_BairstowPrefs {
    /** The maximum number of iterations to apply (See ATK_BAIRSTOW_ITERATIONS) */
    Uint32 maxIterations;
    /** The maximum number of attempts to make (See ATK_BAIRSTOW_ATTEMPTS) */
    Uint32 maxAttempts;
    /** A random generator to drive the algorithm */
    struct ATK_RandGen* random;
    /** The error tolerance for the roots found */
    double epsilon;
} ATK_BairstowPrefs;


/**
 * Standardizes the polynomial so that it is non-degenerate.
 *
 * The polynomial is represented as a big-endian vector of coefficients. So
 * the polynomial has length degree+1, and the first element is the degree-th
 * coefficient, while the constant is the last element.
 *
 * A nondegenerate polynomial has a non-zero coefficient for the highest term,
 * unless the polynomial is the zero constant. Note that the operation may
 * reduce the degree of the input polynomial if it has leading zero coefficients.
 * It is safe for input and output to be the same buffer.
 *
 * @param input     The input polynomial
 * @param degree    The input polynomial degree
 * @param output    The output polynomial
 *
 * @return the degree of the output polynomial
 */
extern DECLSPEC size_t SDLCALL ATK_PolyStandardize(const float* input, size_t degree, float* output);

/**
 * Normalize the given polynomial into a mononomial.
 *
 * The polynomial is represented as a big-endian vector of coefficients. So
 * the polynomial has length degree+1, and the first element is the degree-th
 * coefficient, while the constant is the last element.
 *
 * A normalized polynomial has coefficient 1 for the highest term, unless
 * the polynomial is the zero constant. Note that the operation my reduce the
 * degree of the input polynomial if it has leading zero coefficients.
 *
 * @param input     The input polynomial
 * @param degree    The input polynomial degree
 * @param output    The output polynomial
 *
 * @return the degree of the output polynomial
 */
extern DECLSPEC size_t SDLCALL ATK_PolyNormalize(const float* input, size_t degree, float* output);

/**
 * Outputs the negative of the given polynomial
 *
 * The polynomial is represented as a big-endian vector of coefficients. So
 * the polynomial has length degree+1, and the first element is the degree-th
 * coefficient, while the constant is the last element. It is safe for output
 * to be the same as the input buffer.
 *
 * @param poly      The input polynomial
 * @param degree    The input polynomial degree
 * @param output    The output polynomial
 */
static inline void ATK_PolyNeg(const float* poly, size_t degree, float* output) {
    ATK_VecNeg(poly,output,degree+1);
}

/**
 * Adds two polynomials together, storing the result in output
 *
 * Each polynomial is represented as a big-endian vector of coefficients. So
 * a polynomial has length degree+1, and the first element is the degree-th
 * coefficient, while the constant is the last element.
 *
 * The output should have degree max(degree1,degree2) to store the result.
 * It is safe for the output to be the same as the largest input polynomial.
 * However, the result will be standardized, meaning that the leading
 * coefficient will be non-zero (unless it is the non-zero polynomial).
 * The return value is the degree of the standardized result.
 *
 * @param poly1     The first polynomial
 * @param degree1   The degree of the first polynomial
 * @param poly2     The second polynomial
 * @param degree2   The degree of the second polynomial
 * @param output    The output polynomial
 *
 * @return the degree of the result
 */
extern DECLSPEC size_t SDLCALL ATK_PolyAdd(const float* poly1, size_t degree1,
                                           const float* poly2, size_t degree2,
                                           float* output);

/**
 * Subtracts the second polynomial from the first, storing the result in output
 *
 * Each polynomial is represented as a big-endian vector of coefficients. So
 * a polynomial has length degree+1, and the first element is the degree-th
 * coefficient, while the constant is the last element.
 *
 * The output should have degree max(degree1,degree2) to store the result. It
 * is safe for the output to be the size of the largest input polynomial.
 *
 * @param poly1     The first polynomial
 * @param degree1   The degree of the first polynomial
 * @param poly2     The second polynomial
 * @param degree2   The degree of the second polynomial
 * @param output    The output polynomial
 */
extern DECLSPEC size_t SDLCALL ATK_PolySub(const float* poly1, size_t degree1,
                                           const float* poly2, size_t degree2,
                                           float* output);

/**
 * Scales a polynomial, storing the result in output
 *
 * The polynomial is represented as a big-endian vector of coefficients. So
 * the polynomial has length degree+1, and the first element is the degree-th
 * coefficient, while the constant is the last element. It is safe for output
 * to be the same as the input buffer.
 *
 * The output should have the same degree as the input polynomial to store
 * the result.
 *
 * @param poly      The input polynomial
 * @param degree    The input polynomial degree
 * @param scalar    The scalar to mutliply by
 * @param output    The output degree
 */
extern DECLSPEC size_t SDLCALL ATK_PolyScale(const float* poly, size_t degree,
                                             float scalar, float* output);

/**
 * Scales the first polynomial and adds it to another, storing the result in output
 *
 * Each polynomial is represented as a big-endian vector of coefficients. So
 * a polynomial has length degree+1, and the first element is the degree-th
 * coefficient, while the constant is the last element.
 *
 * The output should have degree max(degree1,degree2) to store the result. It
 * is safe for the output to be the size of the largest input polynomial.
 *
 * @param poly1     The first polynomial
 * @param degree1   The degree of the first polynomial
 * @param scalar    The scalar to mutliply poly1 by
 * @param poly2     The second polynomial
 * @param degree2   The degree of the second polynomial
 * @param output    The output result with remainder
 */
extern DECLSPEC size_t SDLCALL ATK_PolyScaleAdd(const float* poly1, size_t degree1, float scalar,
                                                const float* poly2, size_t degree2, float* output);

/**
 * Multiplies two polynomials together, storing the result in output
 *
 * This function uses either iterative or recursive multiplication optimizing
 * for the degree of the polynomial.
 *
 * Each polynomial is represented as a big-endian vector of coefficients. So
 * a polynomial has length degree+1, and the first element is the degree-th
 * coefficient, while the constant is the last element.
 *
 * The output should have degree degree1+degree2 to store the result. It is
 * generally not safe for output to be the same any of the input polynomials.
 *
 * @param poly1     The first polynomial
 * @param degree1   The degree of the first polynomial
 * @param poly2     The second polynomial
 * @param degree2   The degree of the second polynomial
 * @param output    The output polynomial
 */
extern DECLSPEC size_t SDLCALL ATK_PolyMult(const float* poly1, size_t degree1,
                                            const float* poly2, size_t degree2,
                                            float* output);

/**
 * Iteratively multiplies two polynomials together, storing the result in output
 *
 * This method multiplies the two polynomials with a nested for-loop. It
 * is O(degree1*degree2). It is, however, faster on small polynomials.
 *
 * Each polynomial is represented as a big-endian vector of coefficients. So
 * a polynomial has length degree+1, and the first element is the degree-th
 * coefficient, while the constant is the last element.
 *
 * The output should have degree degree1+degree2 to store the result. It is
 * generally not safe for output to be the same any of the input polynomials.
 *
 * @param poly1     The first polynomial
 * @param degree1   The degree of the first polynomial
 * @param poly2     The second polynomial
 * @param degree2   The degree of the second polynomial
 * @param output    The output polynomial
 */
extern DECLSPEC size_t SDLCALL ATK_PolyIterativeMult(const float* poly1, size_t degree1,
                                                     const float* poly2, size_t degree2,
                                                     float* output);

/**
 * Recursively multiplies two polynomials together, storing the result in output
 *
 * This method multiplies the two polynomials with recursively using a
 * divide-and-conquer algorithm. The algorithm is described here:
 *
 *  http://algorithm.cs.nthu.edu.tw/~course/Extra_Info/Divide%20and%20Conquer_supplement.pdf
 *
 * This algorithm is θ(n) where n is the max(degree1,degree2). It is, however,
 * slower on small polynomials.
 *
 * Each polynomial is represented as a big-endian vector of coefficients. So
 * a polynomial has length degree+1, and the first element is the degree-th
 * coefficient, while the constant is the last element.
 *
 * The output should have degree degree1+degree2 to store the result. It is
 * generally not safe for output to be the same any of the input polynomials.
 *
 * @param poly1     The first polynomial
 * @param degree1   The degree of the first polynomial
 * @param poly2     The second polynomial
 * @param degree2   The degree of the second polynomial
 * @param output    The output polynomial
 */
extern DECLSPEC size_t SDLCALL ATK_PolyRecursiveMult(const float* poly1, size_t degree1,
                                                     const float* poly2, size_t degree2,
                                                     float* output);

/**
 * Computes the synthetic division of the first polynomial by the second
 *
 * This method is adopted from the python code provided at
 *
 *   https://en.wikipedia.org/wiki/Synthetic_division
 *
 * Each polynomial is represented as a big-endian vector of coefficients. So
 * a polynomial has length degree+1, and the first element is the degree-th
 * coefficient, while the constant is the last element.
 *
 * The output buffer for synthetic division should have size degree1+1 (e.g
 * it is the same size as poly1). The beginning is the result and the tail
 * is the remainder. If it exists, the degree of the remainder tail is
 * (d-degree1-1) where d is the result degree. This value must be broken
 * up to implement the / and % operators. However, some algorithms (like
 * Bairstow's method) prefer this method just the way it is.
 *
 * If poly1 and poly2 are standardized (i.e. the highest coefficient is
 * nonzero), then the result is guaranteed to be standardized, but the
 * remainder may not be. The function does not standardize the result if
 * either poly1 or poly2 are not standardized. Furthermore, this function
 * will fail if the leading coefficient of poly2 is zero, even if it is not
 * not the zero polynomial. If you want a form of division that works on
 * non-standardized polynomials, use {@link ATK_PolyDiv} and
 * {@link ATK_PolyRem}.
 *
 * The implementation of the algorithm ensures that it is safe for the
 * output and poly1 to be the same (but not output and poly2). If the leading
 * coefficient of poly2 is 0, this function will simply copy poly1 to the
 * output and return the degree of that polynomial.
 *
 * @param poly1     The divisor polynomial
 * @param degree1   The degree of the divisor polynomial
 * @param poly2     The dividend polynomial
 * @param degree2   The degree of the dividend polynomial
 * @param output    The output result with quotient and remainder
 *
 * @return the degree of the quotient polynomial
 */
extern DECLSPEC size_t SDLCALL ATK_PolySyntheticDiv(const float* poly1, size_t degree1,
                                                    const float* poly2, size_t degree2,
                                                    float* output);

/**
 * Computes the division of the first polynomial by the second
 *
 * The result will be a polynomial of degree no higher than that of the
 * divisor. Hence the output buffer should be large enough to support
 * such a polynomial. Each polynomial is represented as a big-endian vector
 * of coefficients. So a polynomial has length degree+1, and the first
 * element is the degree-th coefficient, while the constant is the last
 * element.
 *
 * The result will be standardized, meaning that the leading coefficient
 * will be non-zero unless it is the zero polynomail. If the dividend is
 * too large or the zero polynomial, this function will return 0 the zero
 * polynomial. To access the remainder, use {@link ATK_PolyRem}.
 *
 * The implementation of the algorithm ensures that it is safe for the
 * output and poly1 to be the same (but not output and poly2).
 *
 * @param poly1     The first polynomial
 * @param degree1   The degree of the first polynomial
 * @param poly2     The second polynomial
 * @param degree2   The degree of the second polynomial
 * @param output    Buffer to store the quotient
 *
 * @return the degree of the quotient polynomial
 */
extern DECLSPEC size_t SDLCALL ATK_PolyDiv(const float* poly1, size_t degree1,
                                           const float* poly2, size_t degree2,
                                           float* output);

/**
 * Computes the remainder of dividing the first polynomial by the second
 *
 * The result will be a polynomial of degree no higher than that of the
 * divisor. Hence the output buffer should be large enough to support
 * such a polynomial. Each polynomial is represented as a big-endian vector
 * of coefficients. So a polynomial has length degree+1, and the first
 * element is the degree-th coefficient, while the constant is the last
 * element.
 *
 * The result will be standardized, meaning that the leading coefficient
 * will be non-zero unless it is the zero polynomail. To access the qotient,
 * use {@link ATK_PolyDiv}.
 *
 * The implementation of the algorithm ensures that it is safe for the
 * output and poly1 to be the same (but not output and poly2).
 *
 * @param poly1     The first polynomial
 * @param degree1   The degree of the first polynomial
 * @param poly2     The second polynomial
 * @param degree2   The degree of the second polynomial
 * @param output    Buffer to store the remainder
 *
 * @return the degree of the remainder polynomial
 */
extern DECLSPEC size_t SDLCALL ATK_PolyRem(const float* poly1, size_t degree1,
                                           const float* poly2, size_t degree2,
                                           float* output);

/**
 * Returns the result of polynomial for the given value
 *
 * The polynomial is represented as a big-endian vector of coefficients. So
 * the polynomial has length degree+1, and the first element is the degree-th
 * coefficient, while the constant is the last element. This function will
 * substitute in value for the polynomial variable to get the final result.
 *
 * @param poly      The polynomial
 * @param degree    The polynomial degree
 * @param value     The value to evaluate
 *
 * @return the result of polynomial for the given value
 */
extern DECLSPEC float SDLCALL ATK_PolyEvaluate(const float* poly, size_t degree, float value);

/**
 * Computes the (complex) roots of this polynomial using Bairstow's method
 *
 * The polynomial is represented as a big-endian vector of coefficients. So
 * the polynomial has length degree+1, and the first element is the degree-th
 * coefficient, while the constant is the last element.
 *
 * Bairstow's method is an approximate root finding technique. The value
 * epsilon is the error value for all of the roots. A good description of
 * Bairstow's method can be found here:
 *
 *    http://nptel.ac.in/courses/122104019/numerical-analysis/Rathish-kumar/ratish-1/f3node9.html
 *
 * The roots are stored in the provided buffer. The values are complex numbers
 * represented as alternating real and imaginary values in the buffer. Hence
 * roots will have 2*degree many elements.
 *
 * This function assumes that the input is a standardized polynomial (i.e.
 * the highest coeffecient is nonzero unless it is the zero polynomial).
 * If it is not standardized, the function will append NaN at the end of
 * the root list to represent the missing roots.
 *
 * It is possible for Bairstow's method to fail, which is why this function
 * has a return value. Adjusting the error tolerance or number of attempts can
 * improve the success rate.
 *
 * @param poly      The polynomial
 * @param degree    The polynomial degree
 * @param roots     The buffer to store the root values
 * @param prefs     The algorithm preferences
 *
 * @return SDL_TRUE if Bairstow's method completes successfully
 */
extern DECLSPEC SDL_bool SDLCALL ATK_PolyRoots(const float* poly, size_t degree, float* roots,
                                               const ATK_BairstowPrefs* prefs);

/**
 * Computes the real roots of this polynomial using Bairstow's method
 *
 * The polynomial is represented as a big-endian vector of coefficients. So
 * the polynomial has length degree+1, and the first element is the degree-th
 * coefficient, while the constant is the last element.
 *
 * Bairstow's method is an approximate root finding technique. The value
 * epsilon is the error value for all of the roots. A good description
 * of Bairstow's method can be found here:
 *
 *    http://nptel.ac.in/courses/122104019/numerical-analysis/Rathish-kumar/ratish-1/f3node9.html
 *
 * The roots are stored in the provided buffer. When complete, roots will have
 * degree many elements. If any root is complex, this method will have added NaN
 * in its place.
 *
 * This function assumes that the input is a standardized polynomial (i.e.
 * the highest coeffecient is nonzero unless it is the zero polynomial).
 * If it is not standardized, the function will append NaN at the end of
 * the root list to represent the missing roots.
 *
 * It is possible for Bairstow's method to fail, which is why this function
 * has a return value. Adjusting the error tolerance or number of attempts can
 * improve the success rate.
 *
 * @param poly      The polynomial
 * @param degree    The polynomial degree
 * @param roots     The buffer to store the root values
 * @param prefs     The algorithm preferences
 *
 * @return SDL_TRUE if Bairstow's method completes successfully
 */
extern DECLSPEC SDL_bool SDLCALL ATK_PolyRealRoots(const float* poly, size_t degree, float* roots,
                                                   const ATK_BairstowPrefs* prefs);

/**
 * Computes the derivative of the given polynomial.
 *
 * The polynomial is represented as a big-endian vector of coefficients. So
 * the polynomial has length degree+1, and the first element is the degree-th
 * coefficient, while the constant is the last element. The output will have
 * one less degree.
 *
 * If input and output are the same buffer, the derivative will be computed
 * in place, reducing the degree. The constant factor of input will be
 * untouched, as we cannot assume output has size degree+1.
 *
 * @param input     The input polynomial
 * @param degree    The input polynomial degree
 * @param output    The output polynomial
 */
extern DECLSPEC void SDLCALL ATK_PolyDerive(const float* input, size_t degree, float* output);

/**
 * Computes the integral of the given polynomial.
 *
 * The constant value of the result will be 0.
 *
 * The polynomial is represented as a big-endian vector of coefficients. So
 * the polynomial has length degree+1, and the first element is the degree-th
 * coefficient, while the constant is the last element. The output will have
 * one higher degree.
 *
 * If input and output are the same buffer, the derivative will be computed
 * in place, extending the degree by one (so input must be size degree+2).
 * The first coefficent will now correspond to a factor of one higher power.
 *
 * @param input     The input polynomial
 * @param degree    The input polynomial degree
 * @param output    The output polynomial
 */
extern DECLSPEC void SDLCALL ATK_PolyIntegrate(const float* input, size_t degree, float* output);


/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif /* __ATK_MATH_H__ */
