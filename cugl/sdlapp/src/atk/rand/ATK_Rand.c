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
#include <ATK_rand.h>
#ifndef M_E
  #define M_E 2.71828182845904523536
#endif

/**
 * @file ATK_MathRand.c
 *
 * Header file for the random number generator component of SDL_atk
 *
 * This component provides a cross-platform pseudorandom number generator
 * built on top of a (64 bit) Marsenne Twister. The goal of this module is
 * to allow us to create randomized algorithms while ensuring that they
 * are reproducible across multiple platforms.
 *
 * The code for this component is adapted from the downloaded source
 *
 *     http://www.math.sci.hiroshima-u.ac.jp/m-mat/MT/VERSIONS/C-LANG/mt19937-64.c
 *
 * The license for this source is compatible with the SDL License.
 *
 * In addition, the code for the random distributions is adapted from the
 * random module in Python 3.10. The license of this code is also compatible
 * with the SDL License.
 *
 * MT19937-64 Attribution and License:
 *
 * A C-program for MT19937-64 (2004/9/29 version).
 * Coded by Takuji Nishimura and Makoto Matsumoto.
 *
 * This is a 64-bit version of Mersenne Twister pseudorandom number
 * generator.
 *
 * Copyright (C) 2004, Makoto Matsumoto and Takuji Nishimura,
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *    3. The names of its contributors may not be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  References:
 *  T. Nishimura, ``Tables of 64-bit Mersenne Twisters''
 *    ACM Transactions on Modeling and
 *    Computer Simulation 10. (2000) 348--357.
 *  M. Matsumoto and T. Nishimura,
 *    ``Mersenne Twister: a 623-dimensionally equidistributed
 *      uniform pseudorandom number generator''
 *    ACM Transactions on Modeling and
 *    Computer Simulation 8. (Jan. 1998) 3--30.
 *
 *  Any feedback is very welcome.
 *  http://www.math.hiroshima-u.ac.jp/~m-mat/MT/emt.html
 *  email: m-mat @ math.sci.hiroshima-u.ac.jp (remove spaces)
 *
 * PSF LICENSE AGREEMENT FOR PYTHON 3.10.11:
 *
 * 1. This LICENSE AGREEMENT is between the Python Software Foundation ("PSF"), and
 *    the Individual or Organization ("Licensee") accessing and otherwise using Python
 *    3.10.11 software in source or binary form and its associated documentation.
 *
 * 2. Subject to the terms and conditions of this License Agreement, PSF hereby
 *    grants Licensee a nonexclusive, royalty-free, world-wide license to reproduce,
 *    analyze, test, perform and/or display publicly, prepare derivative works,
 *    distribute, and otherwise use Python 3.10.11 alone or in any derivative
 *    version, provided, however, that PSF's License Agreement and PSF's notice of
 *    copyright, i.e., "Copyright © 2001-2023 Python Software Foundation; All Rights
 *    Reserved" are retained in Python 3.10.11 alone or in any derivative version
 *    prepared by Licensee.
 *
 * 3. In the event Licensee prepares a derivative work that is based on or
 *    incorporates Python 3.10.11 or any part thereof, and wants to make the
 *    derivative work available to others as provided herein, then Licensee hereby
 *    agrees to include in any such work a brief summary of the changes made to Python
 *    3.10.11.
 *
 * 4. PSF is making Python 3.10.11 available to Licensee on an "AS IS" basis.
 *    PSF MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED. BY WAY OF
 *    EXAMPLE, BUT NOT LIMITATION, PSF MAKES NO AND DISCLAIMS ANY REPRESENTATION OR
 *    WARRANTY OF MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE OR THAT THE
 *    USE OF PYTHON 3.10.11 WILL NOT INFRINGE ANY THIRD PARTY RIGHTS.
 *
 * 5. PSF SHALL NOT BE LIABLE TO LICENSEE OR ANY OTHER USERS OF PYTHON 3.10.11
 *    FOR ANY INCIDENTAL, SPECIAL, OR CONSEQUENTIAL DAMAGES OR LOSS AS A RESULT OF
 *    MODIFYING, DISTRIBUTING, OR OTHERWISE USING PYTHON 3.10.11, OR ANY DERIVATIVE
 *    THEREOF, EVEN IF ADVISED OF THE POSSIBILITY THEREOF.
 *
 * 6. This License Agreement will automatically terminate upon a material breach of
 *    its terms and conditions.
 *
 * 7. Nothing in this License Agreement shall be deemed to create any relationship
 *    of agency, partnership, or joint venture between PSF and Licensee.  This License
 *    Agreement does not grant permission to use PSF trademarks or trade name in a
 *    trademark sense to endorse or promote products or services of Licensee, or any
 *    third party.
 *
 * 8. By copying, installing or otherwise using Python 3.10.11, Licensee agrees
 *    to be bound by the terms and conditions of this License Agreement.
 */
#include <ATK_math.h>
#include <ATK_error.h>

#define NN 312
#define MM 156
#define MATRIX_A 0xB5026F5AA96619E9ULL
#define UM 0xFFFFFFFF80000000ULL    /* Most significant 33 bits */
#define LM 0x7FFFFFFFULL            /* Least significant 31 bits */

#define NV_MAGICCONST  (4 * exp(-0.5) / sqrt(2.0))
#define SG_MAGICCONST  (1.0 + log(4.5))

/**
 * A (64 bit) Marseene Twister generator
 */
typedef struct ATK_RandGen {
    /* The array for the state vector */
    Uint64* state;
    /* The current state offset */
    size_t offset;
} ATK_RandGen;

#pragma mark -
#pragma mark Random Generator

 /**
  * Returns a newly allocated psuedorandom number generator with the given seed
  *
  * The random number generator is the classic 64 bit version implement here
  *
  *     http://www.math.sci.hiroshima-u.ac.jp/m-mat/MT/VERSIONS/C-LANG/mt19937-64.c
  *
  * Generators with the same seed will generate the same numbers. When done,
  * the generator should be freed with {@link ATK_FreeRand}.
  *
  * @param seed The generator seed
  *
  * @return a newly allocated psuedorandom number generator with the given seed
  */
 ATK_RandGen* ATK_AllocRand(Uint64 seed) {
     ATK_RandGen* result = ATK_malloc(sizeof(ATK_RandGen));
     result->state = ATK_malloc(sizeof(Uint64)*NN);
     ATK_ResetRand(result, seed);
     return result;
 }

/**
 * Returns a newly allocated psuedorandom number generator with the given keys
 *
 * The random number generator is the classic 64 bit version implement here
 *
 *     http://www.math.sci.hiroshima-u.ac.jp/m-mat/MT/VERSIONS/C-LANG/mt19937-64.c
 *
 * Generators with the same key sequence will generate the same numbers. When done,
 * the generator should be freed with {@link ATK_FreeRand}.
 *
 * @param key   The array of generator keys
 * @param lken  The key length
 *
 * @return a newly allocated psuedorandom number generator with the given ekys
 */
ATK_RandGen* ATK_AllocRandByArray(Uint64* key, size_t len) {
    ATK_RandGen* result = ATK_malloc(sizeof(ATK_RandGen));
    result->state = ATK_malloc(sizeof(Uint64)*NN);
    ATK_ResetRandByArray(result, key, len);
    return result;
}

/**
 * Frees a previously allocated psuedorandom number generator
 *
 * @param gen   The psuedorandom number generator
 */
void ATK_FreeRand(ATK_RandGen* gen) {
    if (gen == NULL) {
        ATK_SetError("Invalid random number generator");
        return;
    }

    if (gen->state != NULL) {
        ATK_free(gen->state);
        gen->state = NULL;
    }
    ATK_free(gen);
}

/**
 * Resets the psuedorandom number generator to its initial state for the given seed
 *
 * The random number generator is the classic 64 bit version implement here
 *
 *     http://www.math.sci.hiroshima-u.ac.jp/m-mat/MT/VERSIONS/C-LANG/mt19937-64.c
 *
 * @param gen   The psuedorandom number generator
 * @param seed  The generator seed
 */
void ATK_ResetRand(ATK_RandGen* gen, Uint64 seed) {
    gen->state[0] = seed;
    for (Uint32 ii=1; ii < NN; ii++) {
        gen->state[ii] =  (6364136223846793005ULL * (gen->state[ii-1] ^ (gen->state[ii-1] >> 62)) + ii);
    }
    gen->offset = NN;
}

/**
 * Resets the psuedorandom number generator to its initial state for the given keys
 *
 * The random number generator is the classic 64 bit version implement here
 *
 *     http://www.math.sci.hiroshima-u.ac.jp/m-mat/MT/VERSIONS/C-LANG/mt19937-64.c
 *
 * @param gen   The psuedorandom number generator
 * @param key   The array of generator keys
 * @param lken  The key length
 *
 * @return a newly allocated psuedorandom number generator with the given ekys
 */
void ATK_ResetRandByArray(ATK_RandGen* gen, Uint64* key, size_t len) {
    ATK_ResetRand(gen,19650218ULL);
    size_t ii=1;
    size_t jj=0;
    size_t kk = (NN>len ? NN : len);
    for (; kk; kk--) {
        gen->state[ii] =
            (gen->state[ii] ^ ((gen->state[ii-1] ^ (gen->state[ii-1] >> 62)) * 3935559000370003845ULL))
             + key[jj] + jj; /* non linear */
         ii++; jj++;
         if (ii>=NN) { gen->state[0] = gen->state[NN-1]; ii=1; }
         if (jj>=len) jj=0;
    }
    for (kk=NN-1; kk; kk--) {
        gen->state[ii] =
            (gen->state[ii] ^ ((gen->state[ii-1] ^ (gen->state[ii-1] >> 62)) * 2862933555777941757ULL))
            - ii; /* non linear */
         ii++;
         if (ii>=NN) { gen->state[0] = gen->state[NN-1]; ii=1; }
    }

    gen->state[0] = 1ULL << 63; /* MSB is 1; assuring non-zero initial array */
}

#pragma mark -
#pragma mark Random Integers
/**
 * Returns the next pseudorandom integer in [0, 2^64-1]
 *
 * @param gen   The psuedorandom number generator
 *
 * @return the next pseudorandom integer in [0, 2^64-1]
 */
Uint64 ATK_RandUint64(ATK_RandGen* gen) {
    if (gen == NULL) {
        return 0;
    }

    Uint64 x;
    Uint64 mag01[2] = {0ULL, MATRIX_A};
    if (gen->offset >= NN) {    /* generate NN words at one time */
        size_t ii;
        /* if init_genrand64() has not been called, */
        /* a default initial seed is used     */
        for (ii=0; ii < NN-MM; ii++) {
            x = (gen->state[ii] & UM) | (gen->state[ii+1] & LM);
            gen->state[ii] = gen->state[ii+MM] ^ (x>>1) ^ mag01[(int)(x&1ULL)];
        }
        for (; ii < NN-1; ii++) {
            x = (gen->state[ii] & UM) | (gen->state[ii+1] & LM);
            gen->state[ii] = gen->state[ii+(MM-NN)] ^ (x>>1) ^ mag01[(int)(x&1ULL)];
        }
        x = (gen->state[NN-1] & UM) | (gen->state[0] & LM);
        gen->state[NN-1] = gen->state[MM-1] ^ (x>>1) ^ mag01[(int)(x&1ULL)];
        gen->offset = 0;
     }

     x = gen->state[gen->offset++];

     x ^= (x >> 29) & 0x5555555555555555ULL;
     x ^= (x << 17) & 0x71D67FFFEDA60000ULL;
     x ^= (x << 37) & 0xFFF7EEE000000000ULL;
     x ^= (x >> 43);

     return x;
 }

/**
 * Returns the next pseudorandom integer in [min, max)
 *
 * If min >= max, this function sets an error and returns 0.
 *
 * @param gen   The psuedorandom number generator
 *
 * @return the next pseudorandom integer in [min, max)
 */
Uint64 ATK_RandUint64OpenRange(ATK_RandGen* gen, Uint64 min, Uint64 max) {
    if (min >= max) {
        ATK_SetError("Invalid range [%lu, %lu]",(unsigned long)min,(unsigned long)max);
        return 0;
    }

    Uint64 value = ATK_RandUint64(gen);
    value = value % (max-min);
    return value + min;
}

/**
 * Returns the next pseudorandom signed integer in [min, max)
 *
 * If min >= max, this function sets an error and returns 0.
 *
 * @param gen   The psuedorandom number generator
 *
 * @return the next pseudorandom signed integer in [min, max)
 */
Sint64 ATK_RandSint64OpenRange(ATK_RandGen* gen, Sint64 min, Sint64 max) {
    if (min > max) {
        ATK_SetError("Invalid range [%ld, %ld]",(long)min,(long)max);
        return 0;
    } else if (min == max) {
        return min;
    }

    Sint64 value = ATK_RandSint64(gen);
    value = value % (max-min);
    value += (value < 0) ? max : min;
    return value;
}

#pragma mark -
#pragma mark Random Reals
/**
 * Returns the next pseudorandom double in [min,max]
 *
 * Both endpoints of the interval are included. If min > max, this
 * function sets an error and returns 0.
 *
 * @param gen   The psuedorandom number generator
 *
 * @return the next pseudorandom double in [min,max]
 */
double ATK_RandDoubleClosedRange(ATK_RandGen* gen, double min, double max) {
    if (min > max) {
        ATK_SetError("Invalid range [%f, %f]",min,max);
        return 0;
    } else if (min == max) {
        return min;
    }

    double value = ATK_RandClosedDouble(gen);
    value *= (max-min);
    return value+min;
}

/**
 * Returns the next pseudorandom double in (min,max)
 *
 * Neither of the endpoints of the interval are included. If min >= max,
 * this function sets an error and returns 0.
 *
 * @param gen   The psuedorandom number generator
 *
 * @return the next pseudorandom double in (min,max)
 */
double ATK_RandDoubleOpenRange(ATK_RandGen* gen, double min, double max) {
    if (min >= max) {
        ATK_SetError("Invalid range [%f, %f]",min,max);
        return 0;
    }

    double value = ATK_RandOpenDouble(gen);
    value *= (max-min);
    return value+min;
}

/**
 * Returns the next pseudorandom double in [min,max)
 *
 * Only the first endpoint of the interval is included. If min >= max,
 * this function sets an error and returns 0.
 *
 * @param gen   The psuedorandom number generator
 *
 * @return the next pseudorandom double in [min,max)
 */
double ATK_RandDoubleRightOpenRange(ATK_RandGen* gen, double min, double max) {
    if (min >= max) {
        ATK_SetError("Invalid range [%f, %f]",min,max);
        return 0;
    }

    double value = ATK_RandHalfOpenDouble(gen);
    value *= (max-min);
    return value+min;
}

/**
 * Returns the next pseudorandom double in (min,max]
 *
 * Only the second endpoint of the interval is included. If min >= max,
 * this function sets an error and returns 0.
 *
 * @param gen   The psuedorandom number generator
 *
 * @return the next pseudorandom double in (min,max]
 */
double ATK_RandDoubleLeftOpenRange(ATK_RandGen* gen, double min, double max) {
    if (min >= max) {
        ATK_SetError("Invalid range [%f, %f]",min,max);
        return 0;
    }

    double value = 1.0-ATK_RandHalfOpenDouble(gen);
    value *= (max-min);
    return value+min;
}

#pragma mark -
#pragma mark Random Selection

/**
 * Returns a pointer to a randomly selected item in data
 *
 * The function works on type of array. The value size should be the number
 * of bytes of a single element in data, while len should be the length of
 * the array in its native type. Hence the array should consist of size*len
 * bytes.
 *
 * If len == 0, this function returns NULL.
 *
 * @param gen   The psuedorandom number generator
 * @param data  The array to select from
 * @param size  The size (in bytes) of a single array element
 * @param len   The number of elements in the array
 *
 * @param a pointer to a randomly selected item in data
 */
void* ATK_RandChoice(ATK_RandGen* gen, void* data, size_t size, size_t len) {
    if (!len) {
        return NULL;
    }
    Uint8* array = data;
    Uint64 pos = ATK_RandUint64(gen) % len;
    array = array+size*pos;
    return array;
}

/**
 * Randomly shuffles the data in place
 *
 * The function works on type of array. The value size should be the number
 * of bytes of a single element in data, while len should be the length of
 * the array in its native type. Hence the array should consist of size*len
 * bytes. The shuffling is guaranteed to be size-aligned so that elements
 * in the array are preserved.
 *
 * @param gen   The psuedorandom number generator
 * @param data  The array to shuffle
 * @param size  The size (in bytes) of a single array element
 * @param len   The number of elements in the array
 */
void ATK_RandShuffle(ATK_RandGen* gen, void* data, size_t size, size_t len) {
    Uint8* temp = ATK_malloc(size);
    Uint8* array = (Uint8*)data;
    for(Uint64 ii = len-1; ii >= 1; ii--) {
        // Pick an element in x[:ii+1] with which to exchange x[ii]
        Uint64 pos = ATK_RandUint64(gen) % (ii+1);
        if (ii != pos) {
            memcpy(temp, array+size*pos, size);
            memcpy(array+size*pos, array+size*ii, size);
            memcpy(array+size*ii, temp, size);
        }
    }

    ATK_free(temp);
}

#pragma mark -
#pragma mark Random Distributions
/**
 * Returns the next element in the normal distribution.
 *
 * The value mu is the mean, and sigma is the standard deviation.
 * mu can have any value, and sigma must be greater than zero.
 *
 * @param gen   The psuedorandom number generator
 * @param mu    The mean
 * @param sigma The standard deviation
 *
 * @return the next element in the normal distribution.
 */
double ATK_RandNormal(ATK_RandGen* gen, double mu, double sigma) {
    // Adapted from Python random module
    // Uses Kinderman and Monahan method. Reference: Kinderman,
    // A.J. and Monahan, J.F., "Computer generation of random
    // variables using the ratio of uniform deviates", ACM Trans
    // Math Software, 3, (1977), pp257-260.
    double z = 0.0;
    while (1) {
        double u1 = ATK_RandHalfOpenDouble(gen);
        double u2 = 1.0 - ATK_RandHalfOpenDouble(gen);
        z = NV_MAGICCONST * (u1 - 0.5) / u2;
        double zz = z * z / 4.0;
        if (zz <= -log(u2)) {
            break;
        }
    }
    return mu + z * sigma;
}

/**
 * Returns the next element in the gamma distribution.
 *
 * The parameters alpha and beta should be positive. The probability
 * distribution function is
 *
 *                  x^(alpha - 1) * exp(-x * beta) * beta^alpha
 *      pdf(x) =  -----------------------------------------------
 *                                   gamma(alpha)
 *
 * where gamma() is the gamma function. See
 *
 *     https://en.wikipedia.org/wiki/Gamma_distribution
 *
 * The mean is is alpha/beta, and the variance is sqrt(alpha/(beta^2)).
 *
 * @param gen   The psuedorandom number generator
 * @param alpha The shape parameter (should be > 0)
 * @param beta  The rate parameter (should be > 0)
 *
 * @return the next element in the gamma distribution.
 */
double ATK_RandGamma(ATK_RandGen* gen, double alpha, double beta) {
    // Adapted from Python random module
    if (alpha <= 0.0 || beta <= 0.0) {
        ATK_SetError("Gamma distribution: alpha and beta must be > 0.0");
        return 0;
    }

    if (alpha > 1.0) {
        //# Uses R.C.H. Cheng, "The generation of Gamma
        //# variables with non-integral shape parameters",
        //# Applied Statistics, (1977), 26, No. 1, p71-74
        double ainv = sqrt(2.0 * alpha - 1.0);
        double bbb = alpha - log(4.0);
        double ccc = alpha + ainv;

        while (1) {
            double u1 = ATK_RandHalfOpenDouble(gen);
            if (u1 <= 1e-7 || u1 >= 0.9999999) {
                continue;
            }
            double u2 = 1.0 - ATK_RandHalfOpenDouble(gen);
            double v = log(u1 / (1.0 - u1)) / ainv;
            double x = alpha * exp(v);
            double z = u1 * u1 * u2;
            double r = bbb + ccc * v - x;
            if (r + SG_MAGICCONST - 4.5 * z >= 0.0 || r >= log(z)) {
                return x / beta;
            }
        }
    } else if (alpha == 1.0) {
        //# expovariate(1/beta)
        return -log(1.0 -ATK_RandHalfOpenDouble(gen)) / beta;
    } else {
        double x;
        //# alpha is between 0 and 1 (exclusive)
        //# Uses ALGORITHM GS of Statistical Computing - Kennedy & Gentle
        while (1) {
            double u = ATK_RandHalfOpenDouble(gen);
            double b = (M_E + alpha) / M_E;
            double p = b * u;
            x = (p <= 1.0) ?  pow(p, 1.0 / alpha) : -log((b - p) / alpha);
            double u1 = ATK_RandHalfOpenDouble(gen);
            if (p > 1.0) {
                if (u1 <= pow(x,alpha - 1.0)) {
                    break;
                }
            } else if (u1 < exp(-x)) {
                break;
            }
        }
        return x / beta;
    }
    return 0;
}

/**
 * Returns the next element in the beta distribution.
 *
 * The parameters alpha and beta should be positive. The values returned
 * are between 0 and 1.
 *
 * The mean is alpha/(alpha+beta) and the variance is (alpha*beta)/((alpha+beta+1)*(alpha+beta)^2)
 *
 * @param gen   The psuedorandom number generator
 * @param alpha The first shape parameter (should be > 0)
 * @param beta  The second shape parameter (should be > 0)
 *
 * @return the next element in the beta distribution.
 */
double ATK_RandBeta(ATK_RandGen* gen, double alpha, double beta) {
    // Adapted from Python random module
    // It matches Knuth Vol 2 Ed 3 pg 134 "the beta distribution".
    double y = ATK_RandGamma(gen, alpha, 1.0);
    if (y) {
        return y / (y + ATK_RandGamma(gen, beta, 1.0));
    }
    return 0.0;
}
