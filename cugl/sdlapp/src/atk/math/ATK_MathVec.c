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
#include <ATK_math.h>

/**
 * @file ATK_MathVec.c
 *
 * This component contains the functions for optimized operations on (real).
 * vectors. Such vectors are represented as float arrays.
 *
 * You will notice that we have separate stride and adjacent versions of each
 * function. This may seem redundant (just set stride 1!). But our experiments
 * have shown that on some platforms there is a slight, but significant
 * performance difference between the two, possibly due to auto-vectorization
 * or other optimizations. Instead of trying to identify which functions best
 * benefit from the separation, we just went YOLO and separated them all.
 */
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
double ATK_VecDistSq(const float* adata, const float* bdata, size_t len) {
    double dist = 0.0;
    const float* asrc = adata;
    const float* bsrc = bdata;
    float temp;
    while (len--) {
        temp = *(asrc++)-*(bsrc++);
        dist += temp*temp;
    }
    return dist;
}

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
double ATK_VecDistSq_stride(const float* adata, size_t astride,
                            const float* bdata, size_t bstride, size_t len) {
    if (!astride) {
        astride = 1;
    }
    if (!bstride) {
        bstride = 1;
    }
    double dist = 0.0;
    const float* asrc = adata;
    const float* bsrc = bdata;
    float temp;
    while (len--) {
        temp = *(asrc)-*(bsrc);
        asrc += astride;
        bsrc += bstride;
        dist += temp*temp;
    }
    return dist;
}

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
double ATK_VecDist(const float* adata, const float* bdata, size_t len) {
    return sqrt(ATK_VecDistSq(adata,bdata,len));
}

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
double ATK_VecDist_stride(const float* adata, size_t astride,
                          const float* bdata, size_t bstride, size_t len) {
    return sqrt(ATK_VecDistSq_stride(adata,astride,bdata,bstride,len));
}

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
double ATK_VecDiff(const float* adata, const float* bdata, size_t len) {
    double dist = 0.0;
    const float* asrc = adata;
    const float* bsrc = bdata;
    float temp;
    while (len--) {
        temp = *(asrc++)-*(bsrc++);
        dist += temp < 0 ? -temp : temp;
    }
    return dist;
}

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
double ATK_VecDiff_stride(const float* adata, size_t astride,
                          const float* bdata, size_t bstride, size_t len) {
    if (!astride) {
        astride = 1;
    }
    if (!bstride) {
        bstride = 1;
    }
    double dist = 0.0;
    const float* asrc = adata;
    const float* bsrc = bdata;
    float temp;
    while (len--) {
        temp = *(asrc)-*(bsrc);
        asrc += astride;
        bsrc += bstride;
        dist += temp < 0 ? -temp : temp;
    }
    return dist;
}

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
size_t ATK_VecHamm(const float* adata, const float* bdata,
                   float epsilon, size_t len) {
    size_t total = 0;
    const float* asrc = adata;
    const float* bsrc = bdata;
    float temp;
    while (len--) {
        //temp = *(asrc++)-*(bsrc++);
        temp = *asrc-*bsrc;
        if (temp < -epsilon || temp > epsilon) {
            total++;
        }
        asrc++; bsrc++;
    }
    return total;
}

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
size_t ATK_VecHamm_stride(const float* adata, size_t astride,
                          const float* bdata, size_t bstride,
                          float epsilon, size_t len) {
    if (!astride) {
        astride = 1;
    }
    if (!bstride) {
        bstride = 1;
    }
    size_t total = 0;
    const float* asrc = adata;
    const float* bsrc = bdata;
    float temp;
    while (len--) {
        temp = *(asrc)-*(bsrc);
        if (temp < -epsilon || temp > epsilon) {
            total++;
        }
        asrc += astride;
        bsrc += bstride;
    }
    return total;
}

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
float ATK_VecMax(float* data, size_t len) {
    if (!len) {
        return NAN;
    }

    float result = *data;
    float* out = data+1;
    float temp;
    while (--len) {
        temp = *out++;
        if (temp > result) {
            result = temp;
        }
    }
    return result;
}

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
float ATK_VecMax_stride(float* data, size_t stride, size_t len) {
    if (!len) {
        return NAN;
    }
    if (!stride) {
        stride = 1;
    }

    float result = *data;
    float* out = data+stride;
    float temp = result;
    while (--len) {
        temp = *out;
        out += stride;
        if (temp > result) {
            result = temp;
        }
    }
    return result;
}


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
 * @return the index of the maximum value in the data buffer
 */
size_t ATK_VecMaxIndex(float* data, size_t len, float* max) {
    if (!len) {
        return -1;
    }

    size_t result = 0;
    float best = *data;
    float* out = data;
    float temp;
    for(size_t ii = 0; ii < len; ii++) {
        temp = *out++;
        if (temp > best) {
            best = temp;
            result = ii;
        }
    }
    if (max) {
        *max = best;
    }
    return result;
}

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
size_t ATK_VecMaxIndex_stride(float* data, size_t stride, size_t len, float* max) {
    if (!len) {
        return -1;
    }
    if (!stride) {
        stride = 1;
    }

    size_t result = 0;
    float best = *data;
    float* out = data;
    float temp;
    for(size_t ii = 0; ii < len; ii++) {
        temp = *out;
        out += stride;
        if (temp > best) {
            best = temp;
            result = ii;
        }
    }
    if (max) {
        *max = best;
    }
    return result;
}

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
float ATK_VecMaxMag(float* data, size_t len) {
    if (!len) {
        return NAN;
    }

    float result = *data;
    if (result < 0) {
        result = -result;
    }
    float* out = data+1;
    float temp;
    while (--len) {
        temp = *out++;
        if (temp < 0) {
            temp = -temp;
        }
        if (temp > result) {
            result = temp;
        }
    }
    return result;
}

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
float ATK_VecMaxMag_stride(float* data, size_t stride, size_t len) {
    if (!len) {
        return NAN;
    }
    if (!stride) {
        stride = 1;
    }

    float result = *data;
    if (result < 0) {
        result = -result;
    }
    float* out = data+1;
    float temp;
    while (--len) {
        temp = *out;
        out += stride;
        if (temp < 0) {
            temp = -temp;
        }
        if (temp > result) {
            result = temp;
        }
    }
    return result;
}

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
 * @return the index of the maximum magnitude in the data buffer
 */
size_t ATK_VecMaxMagIndex(float* data, size_t len, float* max) {
    if (!len) {
        return -1;
    }

    size_t result = 0;
    float best = *data;
    if (best < 0) {
        best = -best;
    }
    float* out = data;
    float temp;
    for(size_t ii = 0; ii < len; ii++) {
        temp = *out++;
        if (temp < 0) {
            temp = -temp;
        }
        if (temp > best) {
            best = temp;
            result = ii;
        }
    }
    if (max) {
        *max = best;
    }
    return result;
}

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
size_t ATK_VecMaxMagIndex_stride(float* data, size_t stride, size_t len, float* max) {
    if (!len) {
        return -1;
    }
    if (!stride) {
        stride = 1;
    }

    size_t result = 0;
    float best = *data;
    if (best < 0) {
        best = -best;
    }
    float* out = data;
    float temp;
    for(size_t ii = 0; ii < len; ii++) {
        temp = *out;
        out += stride;
        if (temp < 0) {
            temp = -temp;
        }
        if (temp > best) {
            best = temp;
            result = ii;
        }
    }
    if (max) {
        *max = best;
    }
    return result;
}

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
float ATK_VecMin(float* data, size_t len) {
    if (!len) {
        return NAN;
    }

    float result = *data;
    float* out = data+1;
    float temp;
    while (--len) {
        temp = *out++;
        if (temp < result) {
            result = temp;
        }
    }
    return result;
}

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
float ATK_VecMin_stride(float* data, size_t stride, size_t len) {
    if (!len) {
        return NAN;
    }
    if (!stride) {
        stride = 1;
    }

    float result = *data;
    float* out = data+stride;
    float temp = result;
    while (--len) {
        temp = *out;
        out += stride;
        if (temp < result) {
            result = temp;
        }
    }
    return result;
}

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
size_t ATK_VecMinIndex(float* data, size_t len, float* max) {
    if (!len) {
        return -1;
    }

    size_t result = 0;
    float best = *data;
    float* out = data;
    float temp;
    for(size_t ii = 0; ii < len; ii++) {
        temp = *out++;
        if (temp < best) {
            best = temp;
            result = ii;
        }
    }
    if (max) {
        *max = best;
    }
    return result;
}

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
size_t ATK_VecMinIndex_stride(float* data, size_t stride, size_t len, float* min) {
    if (!len) {
        return -1;
    }
    if (!stride) {
        stride = 1;
    }

    size_t result = 0;
    float best = *data;
    float* out = data;
    float temp;
    for(size_t ii = 0; ii < len; ii++) {
        temp = *out;
        out += stride;
        if (temp < best) {
            best = temp;
            result = ii;
        }
    }
    if (min) {
        *min = best;
    }
    return result;
}

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
float ATK_VecMinMag(float* data, size_t len) {
    if (!len) {
        return NAN;
    }

    float result = *data;
    if (result < 0) {
        result = -result;
    }
    float* out = data+1;
    float temp;
    while (--len) {
        temp = *out++;
        if (temp < 0) {
            temp = -temp;
        }
        if (temp < result) {
            result = temp;
        }
    }
    return result;
}

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
float ATK_VecMinMag_stride(float* data, size_t stride, size_t len) {
    if (!len) {
        return NAN;
    }
    if (!stride) {
        stride = 1;
    }

    float result = *data;
    if (result < 0) {
        result = -result;
    }
    float* out = data+1;
    float temp;
    while (--len) {
        temp = *out;
        out += stride;
        if (temp < 0) {
            temp = -temp;
        }
        if (temp < result) {
            result = temp;
        }
    }
    return result;
}

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
size_t ATK_VecMinMagIndex(float* data, size_t len, float* min) {
    if (!len) {
        return -1;
    }

    size_t result = 0;
    float best = *data;
    if (best < 0) {
        best = -best;
    }
    float* out = data;
    float temp;
    for(size_t ii = 0; ii < len; ii++) {
        temp = *out++;
        if (temp < 0) {
            temp = -temp;
        }
        if (temp < best) {
            best = temp;
            result = ii;
        }
    }
    if (min) {
        *min = best;
    }
    return result;
}

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
size_t ATK_VecMinMagIndex_stride(float* data, size_t stride, size_t len, float* min) {
    if (!len) {
        return -1;
    }
    if (!stride) {
        stride = 1;
    }

    size_t result = 0;
    float best = *data;
    if (best < 0) {
        best = -best;
    }
    float* out = data;
    float temp;
    for(size_t ii = 0; ii < len; ii++) {
        temp = *out;
        out += stride;
        if (temp < 0) {
            temp = -temp;
        }
        if (temp < best) {
            best = temp;
            result = ii;
        }
    }
    if (min) {
        *min = best;
    }
    return result;
}


#pragma mark -
#pragma mark Stride Memcpy
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
void ATK_VecCopy_stride(const float* src, size_t sstride,
                        float* dst, size_t dstride, size_t len) {
    if (!dstride) {
        dstride = 1;
    }
    if (!sstride) {
        sstride = 1;
    }

    const float* in = src;
    float* out = dst;
    while(len--) {
        *out = *in;
        out += dstride;
        in  += sstride;
    }
}

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
void ATK_VecCopy_dstride(const float* src, float* dst, size_t dstride,
                         size_t len) {
    if (!dstride) {
        dstride = 1;
    }
    const float* in = src;
    float* out = dst;
    while(len--) {
        *out = *in++;
        out += dstride;
    }

}

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
void ATK_VecCopy_sstride(const float* src, size_t sstride,
                         float* dst, size_t len) {
    if (!sstride) {
        sstride = 1;
    }
    const float* in = src;
    float* out = dst;
    while(len--) {
        *out++ = *in;
        in  += sstride;
    }
}

/**
 * Swaps the first len elements of adata and bdata
 *
 * This function DOES NOT check whether dst and src overlap, and it is generally
 * unsafe to use the function in that case.
 *
 * @param adata The first array to swap
 * @param bdata The second array to swap
 * @param len   The number of elements to swap
 */
void ATK_VecSwap(float* adata, float* bdata, size_t len) {
    float temp;
    float* left  = adata;
    float* right = bdata;
    while(len--) {
        temp = *left;
        *left++ = *right;
        *right++ = temp;
    }
}

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
void ATK_VecSwap_stride(float* adata, size_t astride,
                        float* bdata, size_t bstride, size_t len) {
    if (!astride) {
        astride = 1;
    }
    if (!bstride) {
        bstride = 1;
    }
    float temp;
    float* left  = adata;
    float* right = bdata;
    while(len--) {
        temp = *left;
        *left = *right;
        *right = temp;
        left  += astride;
        right += bstride;
    }
}

/**
 * Reverses the input, storing the result in output.
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param src   The input buffer to reverse
 * @param dst   The buffer to store the result
 * @param len   The number of elements to reverse
 */
void ATK_VecReverse(const float* src, float* dst, size_t len) {
    if (src == dst) {
        float* in  = dst;
        float* out = dst+len-1;
        float tmp;
        size_t amt = len/2;
        while(amt--) {
            tmp = *in;
            *(in++)  = *out;
            *(out--) = tmp;
        }
    } else {
        const float* in  = src;
        float* out = dst+len-1;
        while(len--) {
            *(out--) = *(in++);
        }
    }
}

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
void ATK_VecReverse_stride(const float* src, size_t sstride,
                           float* dst, size_t dstride, size_t len) {
    if (sstride == 0) { sstride = 1; }
    if (dstride == 0) { dstride = 1; }

    if (src == dst) {
        float* in  = dst;
        float* out = dst+(len-1)*dstride;
        float tmp;
        size_t amt = len/2;
        while(amt--) {
            tmp = *in;
            *in  = *out;
            *out = tmp;
            in  += sstride;
            out -= dstride;
        }
    } else {
        const float* in  = src;
        float* out = dst+(len-1)*dstride;
        while(len--) {
            *out = *in;
            in  += sstride;
            out -= dstride;
        }
    }
}

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
void ATK_VecRoll(const float* src, Sint64 amt, float* dst, size_t len) {
    if (src == dst) {
        float swap;
        amt = amt % len;
        if (amt < 0) {
            float* tap1 = dst+(len-1);
            float* tap2 = dst+(len-1+amt);
            while(len--) {
                swap = *tap1;
                *tap1-- = *tap2;
                *tap2 = swap;
                if (tap2 == dst) {
                    tap2 = dst+len;
                }
                tap2--;
            }
        } else if (amt > 0) {
            float* tap1 = dst;
            float* tap2 = dst+amt;
            while(len--) {
                swap = *tap1;
                *tap1++ = *tap2;
                *tap2++ = swap;
                if (tap2 == dst+len) {
                    tap2 = dst;
                }
            }
        }
    } else {
        if (amt < 0) {
            memcpy(dst-amt, src, (len+amt)*sizeof(float));
            memcpy(dst, src+(len+amt), -amt*sizeof(float));
        } else if (amt > 0) {
            memcpy(dst,src+amt,(len-amt)*sizeof(float));
            memcpy(dst+(len-amt),src,amt*sizeof(float));
        } else {
            memcpy(dst, src, len*sizeof(float));
        }
    }
}


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
void ATK_VecRoll_stride(const float* src, size_t sstride, Sint64 amt,
                        float* dst, size_t dstride, size_t len) {
    if (src == dst && sstride == dstride) {
        float swap;
        amt = amt % len;
        if (amt < 0) {
            float* tap1 = dst+(len-1)*sstride;
            float* tap2 = dst+(len-1+amt)*sstride;
            while(len--) {
                swap = *tap1;
                *tap1 = *tap2;
                *tap2 = swap;
                if (tap2 == dst) {
                    tap2 = dst+len;
                }
                tap1 -= sstride;
                tap2 -= sstride;
            }
        } else if (amt > 0) {
            float* tap1 = dst;
            float* tap2 = dst+amt*sstride;
            while(len--) {
                swap = *tap1;
                *tap1 = *tap2;
                *tap2 = swap;
                tap1 += sstride;
                tap2 += sstride;
                if (tap2 == dst+len*sstride) {
                    tap2 = dst;
                }
            }
        }
    } else {
        if (amt < 0) {
            ATK_VecCopy_stride(src, sstride, dst-amt*dstride, dstride, len+amt);
            ATK_VecCopy_stride(src+(len+amt)*sstride, sstride, dst, dstride, -amt);
        } else if (amt > 0) {
            ATK_VecCopy_stride(src+amt*sstride,sstride,dst,dstride,len-amt);
            ATK_VecCopy_stride(src,sstride,dst+(len-amt)*dstride,dstride,amt);
        } else {
            ATK_VecCopy_stride(src, sstride, dst, dstride, len);
        }
    }
}

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
void ATK_VecClear(float* data, size_t len) {
    memset(data, 0, sizeof(float)*len);
}

/**
 * Fills the given buffer with 0s at the given stride
 *
 * This function is effectively a stride-aware version of memset.
 *
 * @param data      the buffer to clear
 * @param stride    the buffer stride
 * @param len       the buffer length
 */
void ATK_VecClear_stride(float* data, size_t stride, size_t len) {
    if (!stride) {
        stride = 1;
    }
    float* out = data;
    while (len--) {
        *out = 0;
        out += stride;
    }
}

/**
 * Fills the given buffer with the value
 *
 * This function is effectively a version of memset dedicated to floats.
 *
 * @param data      the buffer to fill
 * @param value     the value to fill with
 * @param len       the buffer length
 */
void ATK_VecFill(float* data, float value, size_t len) {
    float* out = data;
    while (len--) {
        *out++ = value;
    }
}

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
void ATK_VecFill_stride(float* data, size_t stride, float value, size_t len) {
    if (!stride) {
        stride = 1;
    }
    float* out = data;
    while (len--) {
        *out = value;
        out += stride;
    }
}

/**
 * Fills the given buffer with a linear ramp from start to stop.
 *
 * The first element of data will be start, while the last will be stop. All elements
 * in-between will be equidistant.
 *
 * @param data      the buffer to fill
 * @param stride    the buffer stride
 * @param start     the initial value
 * @param stop      the final value
 * @param len       the buffer length
 */
void ATK_VecRamp(float* data, float start, float stop, size_t len) {
    if (len == 0) {
        return;
    }
    float* out = data;
    float step = (stop-start)/(len-1);
    float curr = start;
    size_t amt = len-1;
    while (amt--) {
        *out++ = curr;
        curr += step;
    }
    *out = stop;  // Round off
}

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
void ATK_VecRamp_stride(float* data, size_t stride,
                        float start, float stop, size_t len) {
    if (len == 0) {
        return;
    }
    if (!stride) {
        stride = 1;
    }

    float* out = data;
    float step = (stop-start)/(len-1);
    float curr = start;
    size_t amt = len-1;
    while (amt--) {
        *out = curr;
        curr += step;
        out += stride;
    }
    *out = stop;    // Round off
}

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
void ATK_VecAbs(const float* input, float* output, size_t len) {
    const float* src = input;
    float* dst = output;
    float temp;
    while(len--) {
        temp = *src++;
        *dst++ = (temp < 0 ? -temp : temp);
    }
}

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
void ATK_VecAbs_stride(const float* input, size_t istride,
                       float* output, size_t ostride, size_t len) {
    if (!istride) {
        istride = 1;
    }
    if (!ostride) {
        ostride = 1;
    }

    const float* src = input;
    float* dst = output;
    float temp;
    while(len--) {
        temp = *src;
        *dst = (temp < 0 ? -temp : temp);
        src += istride;
        dst += ostride;
    }
}

/**
 * Outputs the negative absolute value of the input buffer.
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param input     The input buffer
 * @param output    The output buffer
 * @param len       The number of elements in the buffers
 */
void ATK_VecAbsNeg(const float* input, float* output, size_t len) {
    const float* src = input;
    float* dst = output;
    float temp;
    while(len--) {
        temp = *src++;
        *dst++ = (temp < 0 ? temp : -temp);
    }
}

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
void ATK_VecAbsNeg_stride(const float* input, size_t istride,
                          float* output, size_t ostride, size_t len) {
    if (!istride) {
        istride = 1;
    }
    if (!ostride) {
        ostride = 1;
    }

    const float* src = input;
    float* dst = output;
    float temp;
    while(len--) {
        temp = *src;
        *dst = (temp < 0 ? temp : -temp);
        src += istride;
        dst += ostride;
    }
}


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
void ATK_VecNeg(const float* input, float* output, size_t len) {
    const float* src = input;
    float* dst = output;
    while(len--) {
        *dst++ = -*src++;
    }
}

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
void ATK_VecNeg_stride(const float* input, size_t istride,
                       float* output, size_t ostride, size_t len) {
    if (!istride) {
        istride = 1;
    }
    if (!ostride) {
        ostride = 1;
    }

    const float* src = input;
    float* dst = output;
    while(len--) {
        *dst = -*src;
        src += istride;
        dst += ostride;
    }
}

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
void ATK_VecInv(const float* input, float* output, size_t len) {
    const float* src = input;
    float* dst = output;
    float temp;
    while(len--) {
        temp = *src++;
        *dst++ = temp ? 1/temp : 0;
    }

}

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
void ATK_VecInv_stride(const float* input, size_t istride,
                       float* output, size_t ostride, size_t len) {
    if (!istride) {
        istride = 1;
    }
    if (!ostride) {
        ostride = 1;
    }

    const float* src = input;
    float* dst = output;
    float temp;
    while(len--) {
        temp = *src;
        *dst = temp ? 1/temp : 0;
        src += istride;
        dst += ostride;
    }
}

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
void ATK_VecAdd(const float* input1, const float* input2,
                float* output, size_t len) {
    const float* src1 = input1;
    const float* src2 = input2;
    float* dst = output;
    while(len--) {
        *dst++ = *(src1++)+*(src2++);
    }
}

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
void ATK_VecAdd_stride(const float* input1, size_t istride1,
                       const float* input2, size_t istride2,
                       float* output, size_t ostride, size_t len) {
    if (!istride1) {
        istride1 = 1;
    }
    if (!istride2) {
        istride2 = 1;
    }
    if (!ostride) {
        ostride = 1;
    }

    const float* src1 = input1;
    const float* src2 = input2;
    float* dst = output;
    while(len--) {
        *dst  = *src1+*src2;
        dst  += ostride;
        src1 += istride1;
        src2 += istride2;
    }
}

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
void ATK_VecSub(const float* input1, const float* input2,
                float* output, size_t len) {
    const float* src1 = input1;
    const float* src2 = input2;
    float* dst = output;
    while(len--) {
        *dst++ = *(src1++)-*(src2++);
    }
}

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
void ATK_VecSub_stride(const float* input1, size_t istride1,
                       const float* input2, size_t istride2,
                       float* output, size_t ostride, size_t len) {
    if (!istride1) {
        istride1 = 1;
    }
    if (!istride2) {
        istride2 = 1;
    }
    if (!ostride) {
        ostride = 1;
    }

    const float* src1 = input1;
    const float* src2 = input2;
    float* dst = output;
    while(len--) {
        *dst  = *src1-*src2;
        dst  += ostride;
        src1 += istride1;
        src2 += istride2;
    }
}

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
void ATK_VecMult(const float* input1, const float* input2,
                 float* output, size_t len) {
    const float* src1 = input1;
    const float* src2 = input2;
    float* dst = output;
    while(len--) {
        *dst++ = *(src1++) * *(src2++);
    }
}

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
void ATK_VecMult_stride(const float* input1, size_t istride1,
                        const float* input2, size_t istride2,
                        float* output, size_t ostride, size_t len) {
    if (!istride1) {
        istride1 = 1;
    }
    if (!istride2) {
        istride2 = 1;
    }
    if (!ostride) {
        ostride = 1;
    }

    const float* src1 = input1;
    const float* src2 = input2;
    float* dst = output;
    while(len--) {
        *dst = *src1 * *src2;
        dst  += ostride;
        src1 += istride1;
        src2 += istride2;
    }
}

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
void ATK_VecDiv(const float* input1, const float* input2,
                float* output, size_t len) {
    const float* src1 = input1;
    const float* src2 = input2;
    float* dst = output;
    float temp1 = 0;
    float temp2 = 0;
    while(len--) {
        temp1 = *src1++;
        temp2 = *src2++;
        *dst++ = temp2 == 0 ? 0 : temp1/temp2;
    }
}

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
void ATK_VecDiv_stride(const float* input1, size_t istride1,
                       const float* input2, size_t istride2,
                       float* output, size_t ostride, size_t len) {
    if (!istride1) {
        istride1 = 1;
    }
    if (!istride2) {
        istride2 = 1;
    }
    if (!ostride) {
        ostride = 1;
    }

    const float* src1 = input1;
    const float* src2 = input2;
    float* dst = output;
    float temp1 = 0;
    float temp2 = 0;
    while(len--) {
        *dst = *src2 == 0 ? 0 : (*src1)/(*src2);
        dst  += ostride;
        src1 += istride1;
        src2 += istride2;
    }
}

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
void ATK_VecScale(const float* input, float scalar, float* output, size_t len) {
    const float* src = input;
    float* dst = output;
    while(len--) {
        *dst++ = *(src++) * scalar;
    }
}

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
void ATK_VecScale_stride(const float* input, size_t istride, float scalar,
                         float* output, size_t ostride, size_t len) {
    if (!istride) {
        istride = 1;
    }
    if (!ostride) {
        ostride = 1;
    }

    const float* src = input;
    float* dst = output;
    while(len--) {
        *dst = *src * scalar;
        dst += ostride;
        src += istride;
    }
}

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
void ATK_VecScaleAdd(const float* input1, const float* input2, float scalar,
                     float* output, size_t len) {
    const float* src1 = input1;
    const float* src2 = input2;
    float* dst  = output;
    while(len--) {
        *dst++ = *(src1++)*scalar+*(src2++);
    }
}

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
void ATK_VecScaleAdd_stride(const float* input1, size_t istride1,
                            const float* input2, size_t istride2, float scalar,
                            float* output, size_t ostride, size_t len) {
    if (!istride1) {
        istride1 = 1;
    }
    if (!istride2) {
        istride2 = 1;
    }
    if (!ostride) {
        ostride = 1;
    }

    const float* src1 = input1;
    const float* src2 = input2;
    float* dst = output;
    while(len--) {
        *dst = *src1 * scalar + *src2;
        dst  += ostride;
        src1 += istride1;
        src2 += istride2;
    }
}


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
void ATK_VecClip(const float* input, float min, float max,
                 float* output, size_t len) {
    const float* left = input;
    float* rght = output;
    float temp;
    while(len--) {
        temp = *left++;
        if (temp < min) {
            *rght++ = min;
        } else if (temp > max) {
            *rght++ = max;
        } else {
            *rght++ = temp;
        }
    }
}

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
void ATK_VecClip_stride(const float* input, size_t istride,
                        float min, float max,
                        float* output, size_t ostride, size_t len) {
    if (!istride) {
        istride = 1;
    }
    if (!ostride) {
        ostride = 1;
    }

    const float* left = input;
    float* rght = output;
    float temp;
    while(len--) {
        temp = *left;
        if (temp < min) {
            *rght = min;
        } else if (temp > max) {
            *rght = max;
        } else {
            *rght = temp;
        }

        left += istride;
        rght += ostride;
    }
}

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
void ATK_VecClipZero(const float* input, float min, float max,
                     float* output, size_t len) {
    const float* left = input;
    float* rght = output;
    float temp;
    while(len--) {
        temp = *left++;
        *rght++ = (temp < min || temp > max) ? 0 : temp;
    }
}

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
void ATK_VecClipZero_stride(const float* input, size_t istride,
                            float min, float max,
                            float* output, size_t ostride, size_t len) {
    if (!istride) {
        istride = 1;
    }
    if (!ostride) {
        ostride = 1;
    }

    const float* left = input;
    float* rght = output;
    float temp;
    while(len--) {
        temp = *left;
        *rght = (temp < min || temp > max) ? 0 : temp;

        left += istride;
        rght += ostride;
    }
}

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
void ATK_VecClipCount(const float* input, float min, float max,
                      float* output, size_t len,
                      size_t* mincnt, size_t* maxcnt) {
    size_t mins = 0;
    size_t maxs = 0;
    const float* left = input;
    float* rght = output;
    float temp;
    while(len--) {
        temp = *left++;
        if (temp < min) {
            *rght++ = min;
            mins++;
        } else if (temp > max) {
            *rght++ = max;
            maxs++;
        } else {
            *rght++ = temp;
        }
    }
    if (mincnt) {
        *mincnt = mins;
    }
    if (maxcnt) {
        *maxcnt = maxs;
    }
}

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
void ATK_VecClipCount_stride(const float* input, size_t istride,
                             float min, float max,
                             float* output, size_t ostride, size_t len,
                             size_t* mincnt, size_t* maxcnt) {
    if (!istride) {
        istride = 1;
    }
    if (!ostride) {
        ostride = 1;
    }

    size_t mins = 0;
    size_t maxs = 0;
    const float* left = input;
    float* rght = output;
    float temp;
    while(len--) {
        temp = *left;
        if (temp < min) {
            *rght = min;
            mins++;
        } else if (temp > max) {
            *rght = max;
            maxs++;
        } else {
            *rght = temp;
        }

        left += istride;
        rght += ostride;
    }
    if (mincnt) {
        *mincnt = mins;
    }
    if (maxcnt) {
        *maxcnt = maxs;
    }
}

/**
 * Soft clips the input buffer to the range [-bound,bound]
 *
 * The clip is a soft knee. Values in the range [-knee, knee] are not
 * affected. Values outside this range are asymptotically clipped to the
 * range [-bound,bound] with the formula
 *
 *     y = (bound*x - knee+knee*knee)/x
 *
 * It is safe for output to be the same as the input buffer.
 *
 * @param input     The input buffer
 * @param bound     The asymptotic bound
 * @param knee      The soft knee bound
 * @param output    The output buffer
 * @param len       The number of elements to clip
 */
void ATK_VecClipKnee(const float* input, float bound, float knee,
                     float* output, size_t len) {
    float factor = bound*knee-knee*knee;
    const float* left = input;
    float* rght = output;
    float temp;
    while(len--) {
        temp = *left++;
        if (temp > knee) {
            *rght++ = (bound*temp-factor)/temp;
        } else if (temp < -knee) {
            *rght++ = (bound*temp+factor)/temp;
        } else {
            *rght++ = temp;
        }
    }
}

/**
 * Soft clips the input buffer to the range [-bound,bound]
 *
 * The clip is a soft knee. Values in the range [-knee, knee] are not
 * affected. Values outside this range are asymptotically clipped to the
 * range [-bound,bound] with the formula
 *
 *     y = (bound*x - knee+knee*knee)/x
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
void ATK_VecClipKnee_stride(const float* input, size_t istride,
                            float bound, float knee,
                            float* output, size_t ostride, size_t len) {
    if (!istride) {
        istride = 1;
    }
    if (!ostride) {
        ostride = 1;
    }

    float factor = bound*knee-knee*knee;
    const float* left = input;
    float* rght = output;
    float temp;
    while(len--) {
        temp = *left;
        if (temp > knee) {
            *rght = (bound*temp-factor)/temp;
        } else if (temp < -knee) {
            *rght = (bound*temp+factor)/temp;
        } else {
            *rght = *left;
        }

        left += istride;
        rght += ostride;
    }
}

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
void ATK_VecExclude(const float* input,
                    float min, float max,
                    float* output, size_t len) {
    const float* left = input;
    float* rght = output;
    float temp;
    while(len--) {
        temp = *left++;
        if (min < temp && temp < max) {
            *rght++ = temp < 0 ? min : max;
        } else {
            *rght++ = temp;
        }
    }
}


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
void ATK_VecExclude_stride(const float* input, size_t istride,
                           float min, float max,
                           float* output, size_t ostride, size_t len) {
    if (!istride) {
        istride = 1;
    }
    if (!ostride) {
        ostride = 1;
    }

    const float* left = input;
    float* rght = output;
    float temp;
    while(len--) {
        temp = *left;
        if (min < temp && temp < max) {
            *rght = temp < 0 ? min : max;
        } else {
            *rght = temp;
        }
        left += istride;
        rght += ostride;
    }
}

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
size_t ATK_VecThreshold(const float* input, float min,
                        float* output, size_t len) {
    size_t total = 0;
    const float* left = input;
    float* rght = output;
    float temp;
    while(len--) {
        temp = *left++;
        if (temp < min) {
            *rght++ = min;
            total++;
        } else {
            *rght++ = temp;
        }
    }
    return total;
}

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
size_t ATK_VecThreshold_stride(const float* input, size_t istride, float min,
                               float* output, size_t ostride, size_t len) {
    if (!istride) {
        istride = 1;
    }
    if (!ostride) {
        ostride = 1;
    }

    size_t total = 0;
    const float* left = input;
    float* rght = output;
    float temp;
    while(len--) {
        temp = *left;
        if (temp < min) {
            *rght = min;
            total++;
        } else {
            *rght = temp;
        }
        left += istride;
        rght += ostride;
    }
    return total;
}

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
size_t ATK_VecThresholdInvert(const float* input, float min,
                              float* output, size_t len) {
    size_t total = 0;
    const float* left = input;
    float* rght = output;
    float temp;
    while(len--) {
        temp = *left++;
        if (temp < min) {
            *rght++ = -temp;
            total++;
        } else {
            *rght++ = temp;
        }
    }
    return total;
}

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
size_t ATK_VecThresholdInvert_stride(const float* input, size_t istride, float min,
                                     float* output, size_t ostride, size_t len) {
    if (!istride) {
        istride = 1;
    }
    if (!ostride) {
        ostride = 1;
    }

    size_t total = 0;
    const float* left = input;
    float* rght = output;
    float temp;
    while(len--) {
        temp = *left;
        if (temp < min) {
            *rght = -temp;
            total++;
        } else {
            *rght = temp;
        }
        left += istride;
        rght += ostride;
    }
    return total;
}


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
void ATK_VecThresholdSign(const float* input, float min, float scalar,
                          float* output, size_t len) {
    const float* left = input;
    float* rght = output;
    //float temp;
    while(len--) {
        *rght++ = *left++ < min ? -scalar : scalar;
    }
}

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
void ATK_VecThresholdSign_stride(const float* input, size_t istride,
                                 float min, float scalar,
                                 float* output, size_t ostride, size_t len) {
    if (!istride) {
        istride = 1;
    }
    if (!ostride) {
        ostride = 1;
    }

    const float* left = input;
    float* rght = output;
    //float temp;
    while(len--) {
        *rght = *left < min ? -scalar : scalar;
        left += istride;
        rght += ostride;
    }
}

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
size_t ATK_VecLimit(const float* input, float max,
                    float* output, size_t len) {
    size_t total = 0;
    const float* left = input;
    float* rght = output;
    float temp;
    while(len--) {
        temp = *left++;
        if (temp > max) {
            *rght++ = max;
            total++;
        } else {
            *rght++ = temp;
        }
    }
    return total;
}

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
size_t ATK_VecLimit_stride(const float* input, size_t istride, float max,
                           float* output, size_t ostride, size_t len) {
    if (!istride) {
        istride = 1;
    }
    if (!ostride) {
        ostride = 1;
    }

    size_t total = 0;
    const float* left = input;
    float* rght = output;
    float temp;
    while(len--) {
        temp = *left;
        if (temp > max) {
            *rght = max;
            total++;
        } else {
            *rght = temp;
        }
        left += istride;
        rght += ostride;
    }
    return total;
}

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
size_t ATK_VecLimitInvert(const float* input, float max,
                          float* output, size_t len) {
    size_t total = 0;
    const float* left = input;
    float* rght = output;
    float temp;
    while(len--) {
        temp = *left++;
        if (temp > max) {
            *rght++ = -temp;
            total++;
        } else {
            *rght++ = temp;
        }
    }
    return total;
}

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
size_t ATK_VecLimitInvert_stride(const float* input, size_t istride, float max,
                                 float* output, size_t ostride, size_t len)  {
    if (!istride) {
        istride = 1;
    }
    if (!ostride) {
        ostride = 1;
    }

    size_t total = 0;
    const float* left = input;
    float* rght = output;
    float temp;
    while(len--) {
        temp = *left;
        if (temp > max) {
            *rght = -temp;
            total++;
        } else {
            *rght = temp;
        }
        left += istride;
        rght += ostride;
    }
    return total;
}


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
void ATK_VecLimitSign(const float* input, float max, float scalar,
                      float* output, size_t len) {
    const float* left = input;
    float* rght = output;
    //float temp;
    while(len--) {
        *rght++ = *left++ > max ? -scalar : scalar;
    }
}

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
void ATK_VecLimitSign_stride(const float* input, size_t istride,
                             float max, float scalar,
                             float* output, size_t ostride, size_t len)  {
    if (!istride) {
        istride = 1;
    }
    if (!ostride) {
        ostride = 1;
    }

    const float* left = input;
    float* rght = output;
    //float temp;
    while(len--) {
        *rght = *left > max ? -scalar : scalar;
        left += istride;
        rght += ostride;
    }
}

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
float ATK_VecSum(const float* input, size_t len) {
    float result = 0;
    const float* src = input;
    while (len--) {
        result += *src++;
    }
    return result;
}

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
float ATK_VecSum_stride(const float* input, size_t stride, size_t len) {
    float result = 0;
    const float* src = input;
    while (len--) {
        result += *src;
        src += stride;
    }
    return result;
}

/**
 * Returns the sum of the absolute value of the input.
 *
 * @param input     The input buffer
 * @param len       The number of elements to sum
 *
 * @return the sum of the absolute value of the input.
 */
float ATK_VecSumMag(const float* input, size_t len) {
    float result = 0;
    const float* src = input;
    float temp;
    while (len--) {
        temp = *src++;
        result += (temp < 0 ? -temp : temp);
    }
    return result;
}

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
float ATK_VecSumMag_stride(const float* input, size_t stride, size_t len) {
    float result = 0;
    const float* src = input;
    float temp;
    while (len--) {
        temp = *src;
        result += (temp < 0 ? -temp : temp);
        src += stride;
    }
    return result;
}

/**
 * Returns the sum of squares of the input.
 *
 * @param input     The input buffer
 * @param len       The number of elements to sum
 *
 * @return the sum of squares of the input.
 */
float ATK_VecSumSq(const float* input, size_t len) {
    float result = 0;
    const float* src = input;
    float temp;
    while (len--) {
        temp = *src++;
        result += temp*temp;
    }
    return result;
}

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
float ATK_VecSumSq_stride(const float* input, size_t stride, size_t len) {
    float result = 0;
    const float* src = input;
    float temp;
    while (len--) {
        temp = *src;
        result += temp*temp;
        src += stride;
    }
    return result;
}

/**
 * Returns the average of the elements in input.
 *
 * @param input     The input buffer
 * @param len       The number of elements to average
 *
 * @return the average of the elements in input.
 */
float ATK_VecAverage(const float* input, size_t len) {
    double result = 0;
    const float* src = input;
    size_t amt = len;
    while (amt--) {
        result += *src++;
    }
    return len ? (float)(result/len) : 0;
}

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
float ATK_VecAverage_stride(const float* input, size_t stride, size_t len) {
    double result = 0;
    const float* src = input;
    size_t amt = len;
    while (amt--) {
        result += *src;
        src += stride;
    }
    return len ? (float)(result/len) : 0;
}

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
float ATK_VecMeanSq(const float* input, size_t len) {
    double result = 0;
    const float* src = input;
    size_t amt = len;
    while (amt--) {
        result += (*src) * (*src);
        src++;
    }
    return len ? (float)(result/len) : 0;
}

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
float ATK_VecMeanSq_stride(const float* input, size_t stride, size_t len) {
    double result = 0;
    const float* src = input;
    size_t amt = len;
    while (amt--) {
        result += (*src) * (*src);
        src += stride;
    }
    return len ? (float)(result/len) : 0;
}

/**
 * Return the standard deviation of input.
 *
 * @param input     The input buffer
 * @param len       The number of elements to include
 *
 * @return the standard deviation of input.
 */
float ATK_VecStdDev(const float* input, size_t len) {
    if (len == 0) {
        return 0;
    }

    double mean   = 0;
    double meansq = 0;
    const float* src = input;
    size_t amt = len;
    while (amt--) {
        double val = *src++;
        mean += val;
        meansq += val*val;
    }
    mean = mean/len;
    meansq = meansq/len;
    return (float)(sqrt(meansq-mean*mean));
}

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
float ATK_VecStdDev_stride(const float* input, size_t stride, size_t len) {
    if (len == 0) {
        return 0;
    }

    double mean   = 0;
    double meansq = 0;
    const float* src = input;
    size_t amt = len;
    while (amt--) {
        double val = *src;
        mean += val;
        meansq += val*val;
        src += stride;
    }
    mean = mean/len;
    meansq = meansq/len;
    return (float)(sqrt(meansq-mean*mean));
}


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
void ATK_VecInterp(const float* input1, const float* input2,
                   float factor, float* output, size_t len) {
    const float* src1 = input1;
    const float* src2 = input2;
    float* dst  = output;
    while(len--) {
        *dst++ = *(src1++)*(1-factor)+*(src2++)*factor;
    }

}

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
void ATK_VecInterp_stride(const float* input1, size_t istride1,
                          const float* input2, size_t istride2, float factor,
                          float* output, size_t ostride, size_t len) {
    if (!istride1) {
        istride1 = 1;
    }
    if (!istride2) {
        istride2 = 1;
    }
    if (!ostride) {
        ostride = 1;
    }

    const float* src1 = input1;
    const float* src2 = input2;
    float* dst = output;
    while(len--) {
        *dst = *src1 * (1-factor) + *src2 * factor;
        dst  += ostride;
        src1 += istride1;
        src2 += istride2;
    }
}


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
 * @param factor    The factor array
 * @param output    The output buffer
 * @param len       The number of elements in the buffers
 */
void ATK_VecPairInterp(const float* input, const float* factors,
                       float* output, size_t len) {
    const float* src = factors;
    float* dst = output;
    //float temp;
    float alpha;
    float left, rght;
    int index;
    size_t amt = len;
    while(amt--) {
        if (amt < 96) {
            int z = 2;
        }
        float temp = *src++;
        index = (int)temp;
        alpha = temp - index;
        left = (index < 0 || index >= len) ? 0 : input[index];
        rght = (index < -1 || index >= len-1) ? 0 : input[index+1];

        *dst++ = left*(1-alpha)+rght*alpha;
    }
}

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
 * @param factor    The factor array
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements in the buffers
 */
void ATK_VecPairInterp_stride(const float* input, size_t istride, const float* factors,
                              float* output, size_t ostride, size_t len) {
    if (!istride) {
        istride = 1;
    }
    if (!ostride) {
        ostride = 1;
    }

    const float* src = factors;
    float* dst = output;
    //float temp;
    float alpha;
    float left, rght;
    int index;
    size_t amt = len;
    while(amt--) {
        float temp = *src++;
        index = (int)temp;
        alpha = temp - index;
        left = (index < 0 || index >= len) ? 0 : input[index*istride];
        rght = (index < -1 || index >= len-1) ? 0 : input[(index+1)*istride];

        *dst = left*(1-alpha)+rght*alpha;
        dst += ostride;
    }
}

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
void ATK_VecSlide(const float* input, float start, float end,
                  float* output, size_t len) {
    if (len == 0) {
        return;
    }
    float step = (end-start)/(len-1);
    float curr = start;

    const float* src = input;
    float* dst = output;
    size_t amt = len-1;
    while(amt--) {
        *(dst++) = *(src++) * curr;
        curr += step;
    }
    *dst = *src * end;  // Round off
}

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
void ATK_VecSlide_stride(const float* input, size_t istride,
                         float start, float end,
                         float* output, size_t ostride, size_t len) {
    if (len == 0) {
        return;
    }
    if (!istride) {
        istride = 1;
    }
    if (!ostride) {
        ostride = 1;
    }

    float step = (end-start)/(len-1);
    float curr = start;

    const float* src = input;
    float* dst = output;
    size_t amt = len-1;
    while(amt--) {
        *dst = *src * curr;
        dst  += ostride;
        src  += istride;
        curr += step;
    }
    *dst = *src * end;  // Round off
}

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
void ATK_VecSlideAdd(const float* input1, const float* input2,
                     float start, float end, float* output, size_t len) {
    if (len == 0) {
        return;
    }
    float step = (end-start)/(len-1);
    float curr = start;

    const float* src1 = input1;
    const float* src2 = input2;
    float* dst = output;
    size_t amt = len-1;
    while(amt--) {
        *(dst++) = *(src1++) * curr + *(src2++);
        curr += step;
    }
    *dst = *src1 * end + *src2;    // Round off
}

/**
 * Scales an input signal and adds it to another, storing the result in output
 *
 * The scalar is a sliding factor linearly interpolated between start to end.
 * It will use start for the first element of input1 and end for the size
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
void ATK_VecSlideAdd_stride(const float* input1, size_t istride1,
                            const float* input2, size_t istride2,
                            float start, float end,
                            float* output, size_t ostride, size_t len) {
    if (len == 0) {
        return;
    }
    if (!istride1) {
        istride1 = 1;
    }
    if (!istride2) {
        istride2 = 1;
    }
    if (!ostride) {
        ostride = 1;
    }

    float step = (end-start)/(len-1);
    float curr = start;

    const float* src1 = input1;
    const float* src2 = input2;
    float* dst = output;
    size_t amt = len-1;
    while(amt--) {
        *dst = *src1 * curr + *src2;
        dst  += ostride;
        src1 += istride1;
        src2 += istride2;
        curr += step;
    }
    *dst = *src1 * curr + *src2;    // Round off
}

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
void ATK_VecPowAmpToDecib(const float* input, float zero, SDL_bool power,
                          float* output, size_t len) {
    const float* src = input;
    float* dst = output;
    float factor = (float)(power ? 10 : 20);
    while(len--) {
        *dst++ = (float)(factor*log10(*src++/zero));
    }
}

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
void ATK_VecPowAmpToDecib_stride(const float* input, size_t istride,
                                 float zero, SDL_bool power,
                                 float* output, size_t ostride, size_t len) {
    if (!istride) {
        istride = 1;
    }
    if (!ostride) {
        ostride = 1;
    }

    const float* src = input;
    float* dst = output;
    float factor = (float)(power ? 10 : 20);
    while(len--) {
        *dst = (float)(factor*log10(*src/zero));
        src += istride;
        dst += ostride;
    }
}

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
void ATK_VecDecibToPowAmp(const float* input, float zero, SDL_bool power,
                          float* output, size_t len) {
    const float* src = input;
    float* dst = output;
    float factor = (float)(power ? 10 : 20);
    while(len--) {
        *dst++ = (float)(zero*pow(10,*src++/factor));
    }
}

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
void ATK_VecDecibToPowAmp_stride(const float* input, size_t istride,
                                 float zero, SDL_bool power,
                                 float* output, size_t ostride, size_t len) {
    if (!istride) {
        istride = 1;
    }
    if (!ostride) {
        ostride = 1;
    }

    const float* src = input;
    float* dst = output;
    float factor = (float)(power ? 10 : 20);
    while(len--) {
        *dst = (float)(zero*pow(10,*src/factor));
        src += istride;
        dst += ostride;
    }
}

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
size_t ATK_VecZeroCross(const float* input, size_t max,
                        size_t len, size_t* last) {
    if (!len) {
        return 0;
    }

    size_t total = 0;
    size_t index = 0;
    size_t final = 0;
    int left = *input < 0 ? -1 : 1;
    int rght = 0;
    const float* src = input+1;
    while (++index < len && total < max) {
        rght = *src++ < 0 ? -1 : 1;
        if (left != rght) {
            total++;
            final = index;
        }
        left = rght;
    }
    if (last) {
        *last = final;
    }

    return total;
}

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
size_t ATK_VecZeroCross_stride(const float* input,size_t stride,
                               size_t max, size_t len, size_t* last) {
    if (!len) {
        return 0;
    }
    if (!stride) {
        stride = 1;
    }

    size_t total = 0;
    size_t index = 0;
    size_t final = 0;
    int left = *input < 0 ? -1 : 1;
    int rght = 0;
    const float* src = input+1;
    while (++index < len && total < max) {
        rght = *src < 0 ? -1 : 1;
        if (left != rght) {
            total++;
            final = index;
        }
        src += stride;
        left = rght;
    }
    if (last) {
        *last = final;
    }

    return total;
}

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
void ATK_VecInterleave(const float** input, size_t stride,
                       float* output, size_t len) {
    if (!stride) {
        stride = 1;
    }

    float* dst = output;
    for(size_t ii = 0; ii < len; ii++) {
        for(size_t jj = 0; jj < stride; jj++) {
            *dst++ = input[jj][ii];
        }
    }
}

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
void ATK_VecDeinterleave(const float* input, size_t stride,
                         float** output, size_t len) {
    if (!stride) {
        stride = 1;
    }

    const float* src = input;
    for(size_t ii = 0; ii < len; ii++) {
        for(size_t jj = 0; jj < stride; jj++) {
            output[jj][ii] = *src++;
        }
    }
}

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
void ATK_VecFlatten(const float* input, size_t stride, float* output, size_t len) {
    const float* src = input;
    float* dst = output;
    size_t channels;
    while(len--) {
        *dst = 0;
        channels = stride;
        while (channels--) {
            *dst += *(src++);
        }
        dst++;
    }
}
