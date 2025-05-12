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
 * @file ATK_MathComplex.c
 *
 * This component contains the functions for optimized operations on complex
 * vectors.  Such vectors are represented as interleaved float arrays. The
 * real values are at even positions and the imaginary values are at odd.
 */
#pragma mark Complex Norms
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
void ATK_ComplexNorm(const float* input, float* output, size_t len) {
    const float* src = input;
    float* dst = output;
    float real, imag;
    while(len--) {
        real = *src++;
        imag = *src++;
        *dst++ = sqrtf(real*real+imag*imag);
    }
}

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
void ATK_ComplexNorm_stride(const float* input, size_t istride,
                            float* output, size_t ostride, size_t len) {
    if (!istride) {
        istride = 1;
    }
    if (!ostride) {
        ostride = 1;
    }
    istride += istride;

    const float* src = input;
    float* dst = output;
    float real, imag;
    while(len--) {
        real = src[0];
        imag = src[1];
        *dst = sqrtf(real*real+imag*imag);
        src += istride;
        dst += ostride;
    }
}

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
void ATK_ComplexNormSq(const float* input, float* output, size_t len) {
    const float* src = input;
    float* dst = output;
    float real, imag;
    while(len--) {
        real = *src++;
        imag = *src++;
        *dst++ = real*real+imag*imag;
    }
}

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
void ATK_ComplexNormSq_stride(const float* input, size_t istride,
                            float* output, size_t ostride, size_t len) {
    if (!istride) {
        istride = 1;
    }
    if (!ostride) {
        ostride = 1;
    }
    istride += istride;

    const float* src = input;
    float* dst = output;
    float real, imag;
    while(len--) {
        real = src[0];
        imag = src[1];
        *dst = real*real+imag*imag;
        src += istride;
        dst += ostride;
    }
}

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
void ATK_ComplexConj(const float* input, float* output, size_t len) {
    const float* src = input;
    float* dst = output;
    //float real, imag;
    while(len--) {
        *dst++ = *src++;
        *dst++ = -(*src++);
    }
}

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
void ATK_ComplexConj_stride(const float* input, size_t istride,
                            float* output, size_t ostride, size_t len) {
    if (!istride) {
        istride = 1;
    }
    if (!ostride) {
        ostride = 1;
    }
    istride += istride;
    ostride += ostride;

    const float* src = input;
    float* dst = output;
    //float real, imag;
    while(len--) {
        dst[0] = src[0];
        dst[1] = -(src[1]);
        src += istride;
        dst += ostride;
    }
}

#pragma mark -
#pragma mark Complex Angles

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
void ATK_ComplexAngle(const float* input, float* output, size_t len) {
    const float* src = input;
    float* dst = output;
    float real, imag;
    while(len--) {
        real = *src++;
        imag = *src++;
        *dst++ = atan2f(imag,real);
    }
}

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
void ATK_ComplexAngle_stride(const float* input, size_t istride,
                             float* output, size_t ostride, size_t len) {
    if (!istride) {
        istride = 1;
    }
    if (!ostride) {
        ostride = 1;
    }
    istride += istride;

    const float* src = input;
    float* dst = output;
    float real, imag;
    while(len--) {
        real = src[0];
        imag = src[1];
        *dst = atan2f(imag,real);
        src += istride;
        dst += ostride;
    }
}

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
void ATK_ComplexRot(const float* input, float angle, float* output, size_t len) {
    const float* src = input;
    float* dst = output;
    float rfact = cosf(angle);
    float ifact = sinf(angle);
    float real, imag;
    while(len--) {
        real = *src++;
        imag = *src++;
        *dst++ = real*rfact-imag*ifact;
        *dst++ = imag*rfact+real*ifact;
    }
}

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
void ATK_ComplexRot_stride(const float* input, size_t istride, float angle,
                           float* output, size_t ostride, size_t len) {
    if (!istride) {
        istride = 1;
    }
    if (!ostride) {
        ostride = 1;
    }
    istride += istride;
    ostride += ostride;

    const float* src = input;
    float* dst = output;
    float rfact = cosf(angle);
    float ifact = sinf(angle);
    float real, imag;
    while(len--) {
        real = src[0];
        imag = src[1];
        dst[0] = real*rfact-imag*ifact;
        dst[1] = imag*rfact+real*ifact;
        src += istride;
        dst += ostride;
    }
}

#pragma mark -
#pragma mark Complex Arithmetic
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
void ATK_ComplexNeg_stride(const float* input, size_t istride,
                           float* output, size_t ostride, size_t len) {
    if (!istride) {
        istride = 1;
    }
    if (!ostride) {
        ostride = 1;
    }
    istride += istride;
    ostride += ostride;

    const float* src = input;
    float* dst = output;
    while(len--) {
        dst[0] = -src[0];
        dst[1] = -src[1];
        src += istride;
        dst += ostride;
    }
}

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
void ATK_ComplexInv(const float* input, float* output, size_t len) {
    const float* src = input;
    float* dst = output;
    float real, imag, norm;
    while(len--) {
        real = *src++;
        imag = *src++;
        norm = (real*real+imag*imag);
        if (norm > 0) {
            *dst++ =  real/norm;
            *dst++ = -imag/norm;
        } else {
            *dst++ = 0;
            *dst++ = 0;
        }
    }
}

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
void ATK_ComplexInv_stride(const float* input, size_t istride,
                           float* output, size_t ostride, size_t len) {
    if (!istride) {
        istride = 1;
    }
    if (!ostride) {
        ostride = 1;
    }
    istride += istride;
    ostride += ostride;

    const float* src = input;
    float* dst = output;
    float real, imag, norm;
    while(len--) {
        real = src[0];
        imag = src[1];
        norm = (real*real+imag*imag);
        if (norm > 0) {
            dst[0] =  real/norm;
            dst[1] = -imag/norm;
        } else {
            dst[0] = 0;
            dst[1] = 0;
        }
        src += istride;
        dst += ostride;
    }
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
void ATK_ComplexAdd_stride(const float* input1, size_t istride1,
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
    istride1 += istride1;
    istride2 += istride2;
    ostride  += ostride;

    const float* src1 = input1;
    const float* src2 = input2;
    float* dst = output;
    while(len--) {
        dst[0] = src1[0]+src2[0];
        dst[1] = src1[1]+src2[1];
        dst  += ostride;
        src1 += istride1;
        src2 += istride2;
    }
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
void ATK_ComplexSub_stride(const float* input1, size_t istride1,
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
    istride1 += istride1;
    istride2 += istride2;
    ostride  += ostride;

    const float* src1 = input1;
    const float* src2 = input2;
    float* dst = output;
    while(len--) {
        dst[0] = src1[0]-src2[0];
        dst[1] = src1[1]-src2[1];
        dst  += ostride;
        src1 += istride1;
        src2 += istride2;
    }
}

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
void ATK_ComplexMult(const float* input1, const float* input2,
                     float* output, size_t len) {
    const float* src1 = input1;
    const float* src2 = input2;
    float* dst = output;
    float real1, real2, imag1, imag2;
    while(len--) {
        real1 = *src1++;
        imag1 = *src1++;
        real2 = *src2++;
        imag2 = *src2++;
        *dst++ = real1*real2-imag1*imag2;
        *dst++ = imag1*real2+real1*imag2;
    }
}

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
void ATK_ComplexMult_stride(const float* input1, size_t istride1,
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
    istride1 += istride1;
    istride2 += istride2;
    ostride  += ostride;

    const float* src1 = input1;
    const float* src2 = input2;
    float* dst = output;
    float real1, real2, imag1, imag2;
    while(len--) {
        real1 = src1[0];
        imag1 = src1[1];
        real2 = src2[0];
        imag2 = src2[1];

        dst[0] = real1*real2-imag1*imag2;
        dst[1] = imag1*real2+real1*imag2;
        src1 += istride1;
        src2 += istride2;
        dst  += ostride;
    }
}

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
void ATK_ComplexScale(const float* input, float real, float imag,
                      float* output, size_t len) {
    const float* src = input;
    float* dst = output;
    float sreal, simag;
    while(len--) {
        sreal = *src++;
        simag = *src++;
        *dst++ = sreal*real-simag*imag;
        *dst++ = sreal*imag+simag*real;
    }
}

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
void ATK_ComplexScale_stride(const float* input, size_t istride,
                             float real, float imag,
                             float* output, size_t ostride, size_t len) {
    if (!istride) {
        istride = 1;
    }
    if (!ostride) {
        ostride = 1;
    }
    istride += istride;
    ostride += ostride;

    const float* src = input;
    float* dst = output;
    //float sreal, simag;
    while(len--) {
        dst[0] = src[0]*real-src[1]*imag;
        dst[1] = src[0]*imag+src[1]*real;
        src += istride;
        dst += ostride;
    }
}

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
void ATK_ComplexDiv(const float* input1, const float* input2,
                    float* output, size_t len) {
    const float* src1 = input1;
    const float* src2 = input2;
    float* dst = output;
    float real1, real2, imag1, imag2, norm;
    while(len--) {
        real1 = *src1++;
        imag1 = *src1++;
        real2 = *src2++;
        imag2 = *src2++;
        norm = (real2*real2+imag2*imag2);
        if (norm > 0) {
            *dst++ = (real1*real2+imag1*imag2)/norm;
            *dst++ = (imag1*real2-real1*imag2)/norm;
        } else {
            *dst++ = 0;
            *dst++ = 0;
        }
    }
}

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
void ATK_ComplexDiv_stride(const float* input1, size_t istride1,
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
    istride1 += istride1;
    istride2 += istride2;
    ostride  += ostride;

    const float* src1 = input1;
    const float* src2 = input2;
    float* dst = output;
    float real1, real2, imag1, imag2, norm;
    while(len--) {
        real1 = src1[0];
        imag1 = src1[1];
        real2 = src2[0];
        imag2 = src2[1];
        norm = (real2*real2+imag2*imag2);
        if (norm > 0) {
            dst[0] = (real1*real2+imag1*imag2)/norm;
            dst[1] = (imag1*real2-real1*imag2)/norm;
        } else {
            dst[0] = 0;
            dst[1] = 0;
        }
        src1 += istride1;
        src2 += istride2;
        dst += ostride;
    }
}

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
 * @param real      The real component of the scalar (applied to input1)
 * @param imag      The imaginary component of the scalar (applied to input1)
 * @param output    The output buffer
 * @param len       The number of elements to process
 */
void ATK_ComplexScaleAdd(const float* input1, const float* input2,
                         float real, float imag,
                         float* output, size_t len) {
    const float* src1 = input1;
    const float* src2 = input2;
    float* dst = output;
    float sreal, simag;
    while(len--) {
        sreal = *src1++;
        simag = *src1++;
        *dst++ = sreal*real-simag*imag+*src2++;
        *dst++ = sreal*imag+simag*real+*src2++;
    }
}

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
 * @param real      The real component of the scalar (applied to input1)
 * @param imag      The imaginary component of the scalar (applied to input1)
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to add
 */
void ATK_ComplexScaleAdd_stride(const float* input1, size_t istride1,
                                const float* input2, size_t istride2,
                                float real, float imag,
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
    istride1 += istride1;
    istride2 += istride2;
    ostride  += ostride;

    const float* src1 = input1;
    const float* src2 = input2;
    float* dst = output;
    while(len--) {
        dst[0] = src1[0]*real-src1[1]*imag+src2[0];
        dst[1] = src1[0]*imag+src1[1]*real+src2[1];
        src1 += istride1;
        src2 += istride2;
        dst  += ostride;
    }
}
