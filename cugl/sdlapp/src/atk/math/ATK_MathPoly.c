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
#include <ATK_rand.h>
#include <ATK_error.h>
#include <stdio.h>

/**
 * @file ATK_MathPoly.c
 *
 * This component contains the functions for optimized operations on real
 * valued polynomials. Polynomials are represented as a big-endian vector
 * of coefficients. So the polynomial has length degree+1, and the first
 * element is the degree-th coefficient, while the constant is the last
 * element.
 */

/** Whether or not a value is in the range epsilon */
#define INRANGE(x,epsilon)    (x < epsilon && -x < epsilon)

/** Minimum polynomial degree to switch to recursive multiplication */
#define MULT_THRESHOLD  5

#pragma mark -
#pragma mark Internal Functions
/**
 * Prints out the given polynomial as a string
 *
 * This function is for internal debugging.
 *
 * @param poly      The polynomial
 * @param degree    The polynomial degree
 */
static void print_poly(float* poly, size_t degree) {
    //printf("%.1f",*poly);
    printf("%f",*poly);
    if (degree) {
        printf("x^%zu",degree);
    }

    const float* src = poly+1;
    while(degree--) {
        float val = *src++;
        if (val < 0) {
            //printf(" - %.1f",-val);
            printf(" - %f",-val);
        } else {
            //printf(" + %.1f",val);
            printf(" + %f",val);
        }
        if (degree) {
            printf("x^%zu",degree);
        }
    }
    printf("\n");
}

/**
 * Standardizes the polynomial so that it is non-degenerate.
 *
 * A nondegenerate polynomial has a non-zero coefficient for the highest term,
 * unless the polynomial is the zero constant. Note that the operation may
 * reduce the degree of the input polynomial if it has leading zero coefficients.
 *
 * This function is a more efficient alternative to {@link ATK_PolyStandardize}
 * as no memory is copied. It just shifts the pointer to the leading coefficient
 * (which can be 0 if the degree is 0) and stores the degree in odegree.
 *
 * @param input     The input polynomial
 * @param idegree   The input polynomial degree
 * @param odegree   Pointer to store the output polynomial degree
 *
 * @return the location of the leading coefficient
 */
static const float* standardize_inplace(const float* input, size_t idegree, size_t* odegree) {
    const float* src = input;
    size_t actual = idegree;
    while(!(*src) && actual) {
        src++;
        actual--;
    }
    if (odegree) {
        *odegree = actual;
    }
    return src;
}

/**
 * Iteratively multiplies two polynomials together, storing the result in output
 *
 * This method multiplies the two polynomials with a nested for-loop. It is
 * O(degree1*degree2). It is, however, faster on small polynomials.
 *
 * This is the pure recursive helper for {@link ATK_PolyIterativeMult}. It does
 * not standardize the polynomial.
 *
 * @param poly1     The first polynomial
 * @param degree1   The degree of the first polynomial
 * @param poly2     The second polynomial
 * @param degree2   The degree of the second polynomial
 * @param output    The output polynomial
 */
static size_t iterative_mult(const float* poly1, size_t degree1,
                             const float* poly2, size_t degree2,
                             float* output) {
    memset(output,0,(degree1+degree2+1)*sizeof(float));
    for(size_t ii = 0; ii <= degree2; ii++) {
        for(size_t jj = 0; jj <= degree1; jj++) {
            output[ii+jj] += poly1[jj]*poly2[ii];
        }
    }
    return degree1+degree2;
}

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
 * This is the pure recursive helper for {@link ATK_PolyRecursiveMult}. It does
 * not standardize the polynomial. It also has {@link iterative_mult} as a base
 * case when a polynomial degree is below the threshold.
 *
 * @param poly1     The first polynomial
 * @param degree1   The degree of the first polynomial
 * @param poly2     The second polynomial
 * @param degree2   The degree of the second polynomial
 * @param output    The output polynomial
 * @param threshold The minimal degree to use iterative multiplication
 */
static size_t recursive_mult(const float* poly1, size_t degree1,
                             const float* poly2, size_t degree2,
                             float* output, size_t threshold) {
    if (!degree1) {
        float val = *poly1;
        float* dst = output;
        size_t pos = degree2+1;
        while (pos--) {
            *dst++ = val * (*poly2++);
        }
        return degree2;
    } else if (!degree2) {
        float val = *poly2;
        float* dst = output;
        size_t pos = degree1+1;
        while (pos--) {
            *dst++ = val * (*poly1++);
        }
        return degree1;
    } else if (degree1 <= threshold || degree2 <= threshold) {
        return iterative_mult(poly1,degree1,poly2,degree2,output);
    }

    size_t size1 = (degree1+1)/2;
    size_t size2 = (degree2+1)/2;
    float* temp = ATK_malloc((size1+size2+2)*sizeof(float));

    size_t result = degree1+degree2;
    size_t dem = recursive_mult(poly1, size1-1, poly2, size2-1, output, threshold);
    memset(output+dem+1,0,(result-dem)*sizeof(float));

    dem = recursive_mult(poly1+size1, degree1-size1, poly2, size2-1, temp, threshold);
    for(size_t ii = 1; ii <= dem+1; ii++) {
        output[(result+1)-ii-(degree2-size2+1)] += temp[dem+1-ii];
    }

    dem = recursive_mult(poly1, size1-1, poly2+size2, degree2-size2, temp, threshold);
    for(int ii = 1; ii <= dem+1; ii++) {
        output[(result+1)-ii-(degree1-size1+1)] += temp[dem+1-ii];
    }

    dem = recursive_mult(poly1+size1, degree1-size1, poly2+size2, degree2-size2, temp, threshold);
    for(int ii = 1; ii <= dem+1; ii++) {
        output[(result+1)-ii] += temp[dem+1-ii];
    }

    ATK_free(temp);
    return degree1+degree2;
}

/**
 * Computes the synthetic division of the first polynomial by the second
 *
 * This method is adopted from the python code provided at
 *
 *   https://en.wikipedia.org/wiki/Synthetic_division
 *
 * The output buffer for synthetic division should have size degree1+1 (e.g
 * it is the same size as poly1). The beginning is the result and the tail
 * is the remainder. If it exists, the degree of the remainder tail is
 * (d-degree1-1) where d is the result degree. This value must be broken
 * up to implement the / and % operators. However, some algorithms (like
 * Bairstow's method) prefer this method just the way it is.
 *
 * This function is an internal helper for {@link ATK_PolySyntheticDiv} that
 * does not perform any error checking for the case where poly2 is degenerate
 * or too large to be a dividend.
 *
 * @param poly1     The divisor polynomial
 * @param degree1   The degree of the divisor polynomial
 * @param poly2     The dividend polynomial
 * @param degree2   The degree of the dividend polynomial
 * @param output    The output result with quotient and remainder
 *
 * @return the degree of the quotient polynomial
 */
static size_t synthetic_divide(const float* poly1, size_t degree1,
                               const float* poly2, size_t degree2,
                               float* output) {
    float normalizer = poly2[0];
    size_t cols = degree1-degree2+1;
    if (output != poly1) {
        memmove(output, poly1, (degree1+1)*sizeof(float));
    }

    for(size_t ii = 0; ii < cols; ii++) {
        // Normalize the divisor for synthetic division
        output[ii] /= normalizer;
        float coeff = output[ii];
        if (coeff != 0) {    // useless to multiply if coef is 0
            for(int jj = 1; jj <= degree2; jj++) {
                output[ii + jj] += -poly2[jj] * coeff;
            }
        }
    }
    return degree1-degree2;
}

/**
 * Uses Bairstow's method to find a quadratic polynomial dividing this one.
 *
 * Bairstow's method iteratively divides this polynomial by quadratic
 * factors, until it finds one that divides it within epsilon.  This
 * function can fail (takes too many iterations; the Jacobian is singular),
 * hence the return value. For more information, see
 *
 *    http://nptel.ac.in/courses/122104019/numerical-analysis/Rathish-kumar/ratish-1/f3node9.html
 *
 * When calling this method, quad must be provided as an initial guess,
 * while result can be empty.  This method will modify both quad and
 * result. The polynomial quad is the final quadratic divider and should
 * have degree 2. The polynomial result is the result of the division
 * and should be degree-2.
 *
 * @param poly      The final quadratic divisor chosen
 * @param quad      The final quadratic divisor chosen
 * @param result    The result of the final division
 * @param degree    The original polynomial degree
 *
 * @return true if Bairstow's method completes successfully
 */
static SDL_bool bairstow_factor(const float*poly, float* quad, float* result,
                                size_t degree, const ATK_BairstowPrefs* prefs) {

    float* temp = (float*)ATK_malloc((degree+1)*sizeof(float));
    if (temp == NULL) {
        ATK_OutOfMemory();
        return SDL_FALSE;
    }

    memset(temp, 0, (degree+1)*sizeof(float));

    size_t deg1, deg2;
    float dr = (float)(2*prefs->epsilon);
    float ds = (float)(2*prefs->epsilon);
    for(int ii = 0; ii < (int)prefs->maxIterations; ii++) {
        deg1 = synthetic_divide(poly, degree, quad, 2, result);
        deg2 = synthetic_divide(result, degree, quad, 2, temp);

        float b1 = result[degree-1];
        float b0 = result[degree]-quad[1]*b1;

        float c1 = temp[degree-1];
        float c2 = temp[degree-2];
        float c3 = (degree > 2 ? temp[degree-3] : 0.0f);

        float det = c3*c1-c2*c2;
        if (b0 == 0 && b1 == 0) {
            dr = 0; ds = 0;
        } else if (det != 0) {
            dr = (b1*c2-b0*c3)/det;
            ds = (b0*c2-b1*c1)/det;
        }

        float rerr = 100*dr/quad[1];
        float serr = 100*ds/quad[2];

        if ((INRANGE(rerr,prefs->epsilon) && INRANGE(serr,prefs->epsilon)) || det == 0) {
            break;
        }
        quad[1] -= dr;
        quad[2] -= ds;
    }

    ATK_free(temp);

    // Test for success
    if (INRANGE(dr,prefs->epsilon) && INRANGE(ds,prefs->epsilon)) {
        return SDL_TRUE;
    }
    return SDL_FALSE;
}

/**
 * Solve for the roots of this polynomial with the quadratic formula.
 *
 * Obviously, this method will fail if the polynomial is not quadratic. The
 * roots are added to the provided vector (the original contents are not erased).
 * If any root is complex, this method will have added NaN in its place.
 *
 * @param  roots    the vector to store the root values
 */
static void solve_quadratic(const float* quad, float* roots) {
    float first = quad[0];
    float secnd = quad[1];
    if (!first) {
        return;
    }

    float det = secnd*secnd-4*first*quad[2];
    float fac = 1/(2.0f*first);
    if (det < 0) {
        det = (float)sqrt(-det);
        roots[0] = -secnd*fac;
        roots[1] =  det*fac;
        roots[2] = -secnd*fac;
        roots[3] = -det*fac;
    } else {
        det = (float)sqrt(det);
        roots[0] = (-secnd+det)*fac;
        roots[1] = 0;
        roots[2] = (-secnd-det)*fac;
        roots[3] = 0;
    }
}


#pragma mark -
#pragma mark Polynomial Arithmetic
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
size_t ATK_PolyStandardize(const float* input, size_t degree, float* output) {
    const float* src = input;
    size_t actual = degree;
    while(!(*src++) && actual) {
        actual--;
    }
    memmove(output, input+(degree-actual), (actual+1)*sizeof(float));
    return actual;
}

/**
 * Normalize the given polynomial into a mononomial.
 *
 * The polynomial is represented as a big-endian vector of coefficients. So
 * the polynomial has length degree+1, and the first element is the degree-th
 * coefficient, while the constant is the last element.
 *
 * A normalized polynomial has coefficient 1 for the highest term, unless
 * the polynomial is the zero constant. Note that the operation may reduce
 * the degree of the input polynomial if it has leading zero coefficients.
 * It is safe for input and output to be the same buffer.
 *
 * @param input     The input polynomial
 * @param degree    The input polynomial degree
 * @param output    The output polynomial
 *
 * @return the degree of the output polynomial
 */
size_t ATK_PolyNormalize(const float* input, size_t degree, float* output) {
    const float* src = input;
    float* dst = output;
    size_t actual = degree;
    float val = *src++;
    while(!val && actual) {
        actual--;
        val = *src++;
    }
    if (!val) {
        *dst = 0;
        return 0;
    }
    *dst++ = 1.0f;
    size_t pos = actual;
    while(pos--) {
        *dst++ = (*src++)/val;
    }
    return actual;
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
 * @param the degree of the result
 */
size_t ATK_PolyAdd(const float* poly1, size_t degree1,
                   const float* poly2, size_t degree2,
                   float* output) {
    const float* src1 = standardize_inplace(poly1, degree1, &degree1);
    const float* src2 = standardize_inplace(poly2, degree2, &degree2);

    // Simplify by making poly1 the higher degree
    int swap = 0;
    if (degree1 < degree2) {
        swap = 1;
        size_t temp2 = degree1;
        degree1 = degree2;
        degree2 = temp2;
    }

    float* dst = output;
    size_t pos = degree1-degree2;
    if (swap) {
        while  (pos--) {
            *dst++ = *src2++;
        }
    } else if (pos) {
        while  (pos--) {
            *dst++ = *src1++;
        }
        swap = 1;
    }

    pos = degree2;
    float diff = *src1++ + *src2++;
    if (!swap) {
        while (!diff && pos) {
            pos--; degree1--;
            diff = *src1++ + *src2++;
        }
    }

    pos += 1;
    *dst++ = diff;
    while(pos--) {
        *dst++ = *src1++ + *src2++;
    }

    return degree1;
}

/**
 * Subtracts the second polynomial from the first, storing the result in output
 *
 * Each polynomial is represented as a big-endian vector of coefficients. So
 * a polynomial has length degree+1, and the first element is the degree-th
 * coefficient, while the constant is the last element.
 *
 * The output should have degree max(degree1,degree2) to store the result. It
 * is safe for the output to be the size of the largest input polynomial. The
 * resulting polynomial is guaranteed to be standardized.
 *
 * @param poly1     The first polynomial
 * @param degree1   The degree of the first polynomial
 * @param poly2     The second polynomial
 * @param degree2   The degree of the second polynomial
 * @param output    The output polynomial
 *
 * @return the degree of the resulting polynomial
 */
size_t ATK_PolySub(const float* poly1, size_t degree1,
                   const float* poly2, size_t degree2,
                   float* output){
    const float* src1 = standardize_inplace(poly1, degree1, &degree1);
    const float* src2 = standardize_inplace(poly2, degree2, &degree2);

    // Simplify by making poly1 the higher degree
    int swap = 0;
    if (degree1 < degree2) {
        swap = 1;
        size_t temp2 = degree1;
        degree1 = degree2;
        degree2 = temp2;
    }

    float* dst = output;
    size_t pos = degree1-degree2;
    if (swap) {
        while  (pos--) {
            *dst++ = -(*src2++);
        }
    } else if (pos) {
        while  (pos--) {
            *dst++ = *src1++;
        }
        swap = 1;
    }

    pos = degree2;
    float diff = *src1++ - *src2++;
    if (!swap) {
        while (!diff && pos) {
            pos--; degree1--;
            diff = *src1++ - *src2++;
        }
    }

    pos += 1;
    *dst++ = diff;
    while(pos--) {
        *dst++ = *src1++ - *src2++;
    }

    return degree1;
}

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
 * @param scalar    The scalar to mutliply by
 * @param degree    The input polynomial degree
 * @param output    The output degree
 */
extern DECLSPEC size_t SDLCALL ATK_PolyScale(const float* poly, size_t degree,
                                             float scalar, float* output) {
    if (scalar == 0) {
        *output = 0;
        return 0;
    }
    const float* src = standardize_inplace(poly, degree, &degree);
    size_t len = degree+1;

    float* dst = output;
    while(len--) {
        *dst++ = *(src++) * scalar;
    }
    return degree;
}

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
size_t ATK_PolyScaleAdd(const float* poly1, size_t degree1, float scalar,
                        const float* poly2, size_t degree2, float* output) {
    const float* src1 = standardize_inplace(poly1, degree1, &degree1);
    const float* src2 = standardize_inplace(poly2, degree2, &degree2);

    // Simplify by making poly1 the higher degree
    int swap = 0;
    if (degree1 < degree2) {
        swap = 1;
        size_t temp2 = degree1;
        degree1 = degree2;
        degree2 = temp2;
    }

    float* dst = output;
    size_t pos = degree1-degree2;
    if (swap) {
        while  (pos--) {
            *dst++ = (*src2++);
        }
    } else if (pos) {
        while  (pos--) {
            *dst++ = scalar * (*src1++);
        }
        swap = 1;
    }

    pos = degree2;
    float diff = scalar * (*src1++) + *src2++;
    if (!swap) {
        while (!diff && pos) {
            pos--; degree1--;
            diff = scalar * (*src1++) + *src2++;
        }
    }

    pos += 1;
    *dst++ = diff;
    while(pos--) {
        *dst++ = scalar * (*src1++) + *src2++;
    }

    return degree1;
}

/**
 * Multiplies two polynomials together, storing the result in output
 *
 * This function uses either iterative or recursive multiplication, optimizing
 * for the degree of the polynomial.
 *
 * Each polynomial is represented as a big-endian vector of coefficients. So
 * a polynomial has length degree+1, and the first element is the degree-th
 * coefficient, while the constant is the last element.
 *
 * The output should have degree degree1+degree2 to store the result. It is
 * generally not safe for output to be the same any of the input polynomials.
 * The resulting polynomial is guaranteed to be standardized.
 *
 * @param poly1     The first polynomial
 * @param degree1   The degree of the first polynomial
 * @param poly2     The second polynomial
 * @param degree2   The degree of the second polynomial
 * @param output    The output polynomial
 *
 * @return the degree of the resulting polynomial
 */
size_t ATK_PolyMult(const float* poly1, size_t degree1,
                    const float* poly2, size_t degree2,
                    float* output) {
    const float* src1 = standardize_inplace(poly1, degree1, &degree1);
    const float* src2 = standardize_inplace(poly2, degree2, &degree2);
    if (degree1 >= MULT_THRESHOLD && degree2 >= MULT_THRESHOLD) {
        return recursive_mult(src1,degree1,src2,degree2,output,MULT_THRESHOLD);
    }
    return iterative_mult(src1,degree1,src2,degree2,output);
}



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
 * The resulting polynomial is guaranteed to be standardized.
 *
 * @param poly1     The first polynomial
 * @param degree1   The degree of the first polynomial
 * @param poly2     The second polynomial
 * @param degree2   The degree of the second polynomial
 * @param output    The output polynomial
 *
 * @return the degree of the resulting polynomial
 */
size_t ATK_PolyIterativeMult(const float* poly1, size_t degree1,
                             const float* poly2, size_t degree2,
                             float* output) {
    const float* src1 = standardize_inplace(poly1, degree1, &degree1);
    const float* src2 = standardize_inplace(poly2, degree2, &degree2);
    return iterative_mult(src1,degree1,src2,degree2,output);
}

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
 * The resulting polynomial is guaranteed to be standardized.
 *
 * @param poly1     The first polynomial
 * @param degree1   The degree of the first polynomial
 * @param poly2     The second polynomial
 * @param degree2   The degree of the second polynomial
 * @param output    The output polynomial
 *
 * @return the degree of the resulting polynomial
 */
size_t ATK_PolyRecursiveMult(const float* poly1, size_t degree1,
                             const float* poly2, size_t degree2,
                             float* output) {
    const float* src1 = standardize_inplace(poly1, degree1, &degree1);
    const float* src2 = standardize_inplace(poly2, degree2, &degree2);
    return recursive_mult(src1, degree1, src2, degree2, output, 0);
}

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
size_t ATK_PolySyntheticDiv(const float* poly1, size_t degree1,
                            const float* poly2, size_t degree2,
                            float* output) {
    if (!(*poly2) || degree2 > degree1) {
        if (output != poly1) {
            memmove(output,poly1,(degree1+1)*sizeof(float));
        }
        return degree1;
    }
    return synthetic_divide(poly1, degree1, poly2, degree2, output);
}

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
size_t ATK_PolyDiv(const float* poly1, size_t degree1,
                   const float* poly2, size_t degree2, float* output) {

    const float* src1 = standardize_inplace(poly1, degree1, &degree1);
    const float* src2 = standardize_inplace(poly2, degree2, &degree2);
    if (!(*src2) || degree2 > degree1) {
        *output = 0;
        return 0;
    }

    return synthetic_divide(src1, degree1, src2, degree2, output);
}

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
size_t ATK_PolyRem(const float* poly1, size_t degree1,
                   const float* poly2, size_t degree2, float* output) {
    const float* src1 = standardize_inplace(poly1, degree1, &degree1);
    const float* src2 = standardize_inplace(poly2, degree2, &degree2);
    if (!(*src2) || degree2 > degree1) {
        if (output != src1) {
            memmove(output,src1,(degree1+1)*sizeof(float));
        }
        return degree1;
    }

    size_t result = synthetic_divide(src1, degree1, src2, degree2, output);
    if (result == degree1) {
        *output = 0;
        return 0;
    }


    const float* dst = standardize_inplace(output+result+1, degree1-result-1,&result);
    memmove(output, dst, (result+1)*sizeof(float));
    return result;
}


#pragma mark -
#pragma mark Polynomial Evaluation
/**
 * Returns the result of polynomial for the given value
 *
 * The polynomial is represented as a big-endian vector of coefficients. So
 * the polynomial has length degree+1, and the first element is the degree-th
 * coefficient, while the constant is the last element.  This function will
 * substitute in value for the polynomial variable to get the final result.
 *
 * @param input     The polynomial
 * @param degree    The polynomial degree
 * @param value     The value to evaluate
 *
 * @return the result of polynomial for the given value
 */
float ATK_PolyEvaluate(const float* poly, size_t degree, float value) {
    const float* src = poly;
    size_t pos = degree+1;
    float result = 0;
    while(pos--) {
        result = result*value + (*src++);
    }
    return result;
}

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
 * @param input     The polynomial
 * @param degree    The polynomial degree
 * @param roots     The buffer to store the root values
 * @param epsilon   The error tolerance for the root values
 *
 * @return SDL_TRUE if Bairstow's method completes successfully
 */
SDL_bool ATK_PolyRoots(const float* poly, size_t degree, float* roots,
                       const ATK_BairstowPrefs* prefs) {
    size_t deg, left;
    const float* src = standardize_inplace(poly, degree, &deg);
    left = degree-deg;

    float* output = roots;

    // Remove the x's
    while (!src[deg] && deg) {
        *output++ = 0;
        *output++ = 0;
        deg--;
    }

    float quad[3];
    quad[0] = 1;

    float* result1 = ATK_malloc((deg+1)*sizeof(float));
    if (result1 == NULL) {
        ATK_OutOfMemory();
        return SDL_FALSE;
    }
    float* result2 = ATK_malloc((deg+1)*sizeof(float));
    if (result2 == NULL) {
        ATK_OutOfMemory();
        ATK_free(result1);
        return SDL_FALSE;
    }

    memcpy(result1, src, (deg+1)*sizeof(float));
    float* swap;
    int attempts = 0;
    while (deg > 2 && attempts <= (int)prefs->maxAttempts) {
        double a = ATK_RandClosedDouble(prefs->random);
        double b = ATK_RandClosedDouble(prefs->random);
        quad[1] = (float)(-a-b);
        quad[2] = (float)(a*b);
        if (bairstow_factor(result1,quad,result2,deg,prefs)) {
            solve_quadratic(quad, output);
            output += 4; // Complex
            deg -= 2; attempts = 0;

            // Swap the buffers
            swap = result1;
            result1 = result2;
            result2 = swap;
        } else {
            attempts++;
        }
    }

    if (attempts > (int)prefs->maxAttempts) {
        ATK_free(result1);
        ATK_free(result2);
        return SDL_FALSE;
    }

    if (deg == 2) {
        solve_quadratic(result1, output);
        output += 4;
    } else if (deg == 1) {
        *output++ = -result1[1]/result1[0];
        *output++ = 0;
    }

    while (left--) {
        *output++ = nanf("");
        *output++ = nanf("");
    }

    ATK_free(result1);
    ATK_free(result2);
    return SDL_TRUE;
}

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
 * the root list to represent the missing roots. All NaN values (for both
 * missing and complex roots) are guaranteed to be at the end.
 *
 * It is possible for Bairstow's method to fail, which is why this function
 * has a return value. Adjusting the error tolerance or number of attempts can
 * improve the success rate.
 *
 * @param input     The polynomial
 * @param degree    The polynomial degree
 * @param roots     The buffer to store the root values
 * @param epsilon   The error tolerance for the root values
 *
 * @return SDL_TRUE if Bairstow's method completes successfully
 */
SDL_bool ATK_PolyRealRoots(const float* poly, size_t degree, float* roots,
                           const ATK_BairstowPrefs* prefs) {
    size_t deg, left;
    const float* src = standardize_inplace(poly, degree, &deg);
    left = degree-deg;

    float* output = roots;

    // Remove the x's
    while (!src[deg] && deg) {
        *output++ = 0;
        *output++ = 0;
        deg--;
    }

    float quad[3];
    quad[0] = 1;

    float* result1 = ATK_malloc((deg+1)*sizeof(float));
    if (result1 == NULL) {
        ATK_OutOfMemory();
        return SDL_FALSE;
    }
    float* result2 = ATK_malloc((deg+1)*sizeof(float));
    if (result2 == NULL) {
        ATK_OutOfMemory();
        ATK_free(result1);
        return SDL_FALSE;
    }
    memcpy(result1, src, (deg+1)*sizeof(float));

    float* swap;
    float temp[4];

    int attempts = 0;
    while (deg > 2 && attempts <= (int)prefs->maxAttempts) {
        double a = ATK_RandClosedDouble(prefs->random);
        double b = ATK_RandClosedDouble(prefs->random);
        quad[1] = (float)(-a-b);
        quad[2] = (float)(a*b);
        if (bairstow_factor(result1,quad,result2,deg,prefs)) {
            solve_quadratic(quad, temp);
            if (temp[1] != 0 || temp[3] != 0) {
                left += 2;
            } else {
                *output++ = temp[0];
                *output++ = temp[2];
            }
            deg -= 2; attempts = 0;

            // Swap the buffers
            swap = result1;
            result1 = result2;
            result2 = swap;
        } else {
            attempts++;
        }
    }

    if (attempts > (int)prefs->maxAttempts) {
        ATK_free(result1);
        ATK_free(result2);
        return SDL_FALSE;
    }

    if (deg == 2) {
        solve_quadratic(result1, temp);
        if (temp[1] != 0 || temp[3] != 0) {
            left += 2;
        } else {
            *output++ = temp[0];
            *output++ = temp[2];
        }
    } else if (deg == 1) {
        *output++ = -result1[1]/result1[0];
    }

    while (left--) {
        *output++ = nanf("");
    }

    ATK_free(result1);
    ATK_free(result2);
    return SDL_TRUE;
}

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
void ATK_PolyDerive(const float* input, size_t degree, float* output) {
    if (degree == 0) {
        *output = 0;
        return;
    }
    const float* src = input;
    float* dst = output;
    size_t pos = degree;
    while(pos) {
        *dst++ = (pos--)*(*src++);
    }
}

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
void ATK_PolyIntegrate(const float* input, size_t degree, float* output) {
    const float* src = input;
    float* dst = output;
    size_t pos = degree+1;
    while(pos) {
        *dst++ = (*src++)/(pos--);
    }
    *dst = 0;
}
