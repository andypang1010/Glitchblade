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
 * @file ATK_rand.h
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
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. The names of its contributors may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 *  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
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
 */
#ifndef __ATK_RAND_H__
#define __ATK_RAND_H__
#include <SDL.h>
#include <SDL_version.h>
#include <begin_code.h>
#include <math.h>

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * A (64 bit) Marseene Twister generator
 *
 * This generator is guaranteed to be cross-platform with respect to
 * random integers. So, given the same seed, any two different platforms
 * will generate the same sequence of random integers.
 *
 * For the case of reals (e.g. doubles), cross-platform support depends
 * on IEEE 754, which is supported by all modern hardware. Any two
 * platforms that support IEEE 754 should generate the same numbers for
 * the same hardware.
 */
typedef struct ATK_RandGen ATK_RandGen;

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
 * @param seed  The generator seed
 *
 * @return a newly allocated psuedorandom number generator with the given seed
 */
extern DECLSPEC ATK_RandGen* SDLCALL ATK_AllocRand(Uint64 seed);

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
extern DECLSPEC ATK_RandGen* SDLCALL ATK_AllocRandByArray(Uint64* key, size_t len);

/**
 * Frees a previously allocated psuedorandom number generator
 *
 * @param gen   The psuedorandom number generator
 */
extern DECLSPEC void SDLCALL ATK_FreeRand(ATK_RandGen* gen);

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
extern DECLSPEC void SDLCALL ATK_ResetRand(ATK_RandGen* gen, Uint64 seed);

/**
 * Resets the psuedorandom number generator to its initial state for the given keys
 *
 * The random number generator is the classic 64 bit version implement here
 *
 *     http://www.math.sci.hiroshima-u.ac.jp/m-mat/MT/VERSIONS/C-LANG/mt19937-64.c
 *
 * @param gen   The psuedorandom number generator
 * @param key   The array of generator keys
 * @param len   The key length
 */
extern DECLSPEC void SDLCALL ATK_ResetRandByArray(ATK_RandGen* gen, Uint64* key, size_t len);

#pragma mark -
#pragma mark Random Integers
/**
 * Returns the next pseudorandom integer in [0, 2^64-1]
 *
 * @param gen   The psuedorandom number generator
 *
 * @return the next pseudorandom integer in [0, 2^64-1]
 */
extern DECLSPEC Uint64 SDLCALL ATK_RandUint64(ATK_RandGen* gen);

/**
 * Returns the next pseudorandom integer in [-2^63, 2^63-1]
 *
 * @param gen   The psuedorandom number generator
 *
 * @return the next pseudorandom integer in [-2^63, 2^63-1]
 */
static inline Sint64 ATK_RandSint64(ATK_RandGen* gen) {
    Uint64 source = ATK_RandUint64(gen);
    Sint64* val = (Sint64*)(&source);
    return *val;
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
extern DECLSPEC Uint64 SDLCALL ATK_RandUint64OpenRange(ATK_RandGen* gen, Uint64 min, Uint64 max);

/**
 * Returns the next pseudorandom integer in [min, max]
 *
 * If min > max, this function sets an error and returns 0.
 *
 * @param gen   The psuedorandom number generator
 *
 * @return the next pseudorandom integer in [min, max]
 */
static inline Uint64 ATK_RandUint64ClosedRange(ATK_RandGen* gen, Uint64 min, Uint64 max) {
    return ATK_RandUint64OpenRange(gen,min,max+1);
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
extern DECLSPEC Sint64 SDLCALL ATK_RandSint64OpenRange(ATK_RandGen* gen, Sint64 min, Sint64 max);

/**
 * Returns the next pseudorandom signed integer in [min, max]
 *
 * If min > max, this function sets an error and returns 0.
 *
 * @param gen   The psuedorandom number generator
 *
 * @return the next pseudorandom signed integer in [min, max]
 */
static inline Sint64 ATK_RandSint64ClosedRange(ATK_RandGen* gen, Sint64 min, Sint64 max) {
    return ATK_RandSint64OpenRange(gen,min,max+1);
}

/**
 * Returns the next pseudorandom integer in [0, 2^32-1]
 *
 * @param gen   The psuedorandom number generator
 *
 * @return the next pseudorandom integer in [0, 2^32-1]
 */
static inline Uint32 ATK_RandUint32(ATK_RandGen* gen) {
    return (Uint32)ATK_RandUint64OpenRange(gen,0,0x100000000UL);
}

/**
 * Returns the next pseudorandom integer in [-2^31, 2^31-1]
 *
 * @param gen   The psuedorandom number generator
 *
 * @return the next pseudorandom integer in [-2^31, 2^31-1]
 */
static inline Sint32 ATK_RandSint32(ATK_RandGen* gen) {
    return (Sint32)ATK_RandSint64OpenRange(gen,-((Sint64)0x80000000L),0x80000000L);
}

/**
 * Returns the next pseudorandom integer in [0, 2^16-1]
 *
 * @param gen   The psuedorandom number generator
 *
 * @return the next pseudorandom integer in [0, 2^16-1]
 */
static inline Uint16 ATK_RandUint16(ATK_RandGen* gen) {
    return (Uint16)ATK_RandUint64OpenRange(gen,0,0x10000UL);
}

/**
 * Returns the next pseudorandom integer in [-2^15, 2^15-1]
 *
 * @param gen   The psuedorandom number generator
 *
 * @return the next pseudorandom integer in [-2^15, 2^15-1]
 */
static inline Sint16 ATK_RandSint16(ATK_RandGen* gen) {
    return (Sint16)ATK_RandSint64OpenRange(gen,-0x8000L,0x8000L);
}

/**
 * Returns the next pseudorandom integer in [0, 255]
 *
 * @param gen   The psuedorandom number generator
 *
 * @return the next pseudorandom integer in [0, 255]
 */
static inline Uint8 ATK_RandUint8(ATK_RandGen* gen) {
    return (Uint8)ATK_RandUint64OpenRange(gen,0,256);
}

/**
 * Returns the next pseudorandom integer in [-128,127]
 *
 * @param gen   The psuedorandom number generator
 *
 * @return the next pseudorandom integer in [-128,127]
 */
static inline Sint8 ATK_RandSint8(ATK_RandGen* gen) {
    return (Sint8)ATK_RandSint64OpenRange(gen,-128,128);
}

/**
 * Returns the next pseudorandom value SDL_TRUE or SDL_FALSE
 *
 * @param gen   The psuedorandom number generator
 *
 * @return the next pseudorandom value SDL_TRUE or SDL_FALSE
 */
static inline SDL_bool ATK_RandBool(ATK_RandGen* gen) {
    return ATK_RandUint64(gen) % 2 == 0 ? SDL_TRUE : SDL_FALSE;
}

#pragma mark -
#pragma mark Random Reals
/**
 * Returns the next pseudorandom double in [0,1)
 *
 * Only the endpoint 0 is included. To get a random double in the
 * interval (0,1], simply subtract this number from 1.
 *
 * This function is equivalent to {@link ATK_RandHalfOpenDouble}
 * as that is often the desired behavior of random generators.
 *
 * @param gen   The psuedorandom number generator
 *
 * @return the next pseudorandom double in [0,1)
 */
static inline double SDLCALL ATK_RandDouble(ATK_RandGen* gen) {
    return (ATK_RandUint64(gen) >> 11) * (1.0/9007199254740992.0);
}

/**
 * Returns the next pseudorandom double in [0,1]
 *
 * Both endpoints of the interval are included.
 *
 * @param gen   The psuedorandom number generator
 *
 * @return the next pseudorandom double in [0,1]
 */
static inline double ATK_RandClosedDouble(ATK_RandGen* gen) {
    return (ATK_RandUint64(gen) >> 11) * (1.0/9007199254740991.0);
}

/**
 * Returns the next pseudorandom double in [0,1)
 *
 * Only the endpoint 0 is included. To get a random double in the
 * interval (0,1], simply subtract this number from 1.
 *
 * @param gen   The psuedorandom number generator
 *
 * @return the next pseudorandom double in [0,1)
 */
static inline double ATK_RandHalfOpenDouble(ATK_RandGen* gen) {
    return (ATK_RandUint64(gen) >> 11) * (1.0/9007199254740992.0);
}

/**
 * Returns the next pseudorandom double in (0,1)
 *
 * Neither endpoint in the interval is included.
 *
 * @param gen   The psuedorandom number generator
 *
 * @return the next pseudorandom double in (0,1)
 */
static inline double ATK_RandOpenDouble(ATK_RandGen* gen) {
    return ((ATK_RandUint64(gen) >> 12) + 0.5) * (1.0/4503599627370496.0);
}

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
extern DECLSPEC double SDLCALL ATK_RandDoubleClosedRange(ATK_RandGen* gen, double min, double max);

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
extern DECLSPEC double SDLCALL ATK_RandDoubleOpenRange(ATK_RandGen* gen, double min, double max);

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
extern DECLSPEC double SDLCALL ATK_RandDoubleRightOpenRange(ATK_RandGen* gen, double min, double max);

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
extern DECLSPEC double SDLCALL ATK_RandDoubleLeftOpenRange(ATK_RandGen* gen, double min, double max);

/**
 * Returns the next pseudorandom float in [0,1)
 *
 * Only the endpoint 0 is included. To get a random double in the
 * interval (0,1], simply subtract this number from 1.
 *
 * @param gen   The psuedorandom number generator
 *
 * @return the next pseudorandom float in [0,1)
 */
static inline float ATK_RandFloat(ATK_RandGen* gen) {
    return (float)ATK_RandHalfOpenDouble(gen);
}

/**
 * Returns the next pseudorandom float in [min,max]
 *
 * Both endpoints of the interval are included. If min > max, this
 * function sets an error and returns 0.
 *
 * @param gen   The psuedorandom number generator
 *
 * @return the next pseudorandom float in [min,max]
 */
static inline float ATK_RandFloatClosedRange(ATK_RandGen* gen, float min, float max) {
    return (float)ATK_RandDoubleClosedRange(gen,min,max);
}

/**
 * Returns the next pseudorandom float in (min,max)
 *
 * Neither of the endpoints of the interval are included. If min >= max,
 * this function sets an error and returns 0.
 *
 * @param gen   The psuedorandom number generator
 *
 * @return the next pseudorandom float in (min,max)
 */
static inline float ATK_RandFloatOpenRange(ATK_RandGen* gen, float min, float max) {
    return (float)ATK_RandDoubleOpenRange(gen,min,max);
}

/**
 * Returns the next pseudorandom float in [min,max)
 *
 * Only the first endpoint of the interval is included. If min >= max,
 * this function sets an error and returns 0.
 *
 * @param gen   The psuedorandom number generator
 *
 * @return the next pseudorandom float in [min,max)
 */
static inline float ATK_RandFloatRightOpenRange(ATK_RandGen* gen, float min, float max) {
    return (float)ATK_RandDoubleRightOpenRange(gen,min,max);
}

/**
 * Returns the next pseudorandom float in (min,max]
 *
 * Only the second endpoint of the interval is included. If min >= max,
 * this function sets an error and returns 0.
 *
 * @param gen   The psuedorandom number generator
 *
 * @return the next pseudorandom float in (min,max]
 */
static inline float ATK_RandFloatLeftOpenRange(ATK_RandGen* gen, float min, float max) {
    return (float)ATK_RandDoubleLeftOpenRange(gen,min,max);
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
 * @return a pointer to a randomly selected item in data
 */
extern DECLSPEC void* SDLCALL ATK_RandChoice(ATK_RandGen* gen, void* data, size_t size, size_t len);
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
extern DECLSPEC void SDLCALL ATK_RandShuffle(ATK_RandGen* gen, void* data, size_t size, size_t len);

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
extern DECLSPEC double SDLCALL ATK_RandNormal(ATK_RandGen* gen, double mu, double sigma);

/**
 * Returns the next element in the log normal distribution.
 *
 * If you take the natural logarithm of this distribution, you will get a
 * normal distribution with mean mu and standard deviation sigma. mu can
 * have any value, and sigma must be greater than zero.
 *
 * @param gen   The psuedorandom number generator
 * @param mu    The mean
 * @param sigma The standard deviation
 *
 * @return the next element in the log normal distribution.
 */
static inline double ATK_RandLogNorm(ATK_RandGen* gen, double mu, double sigma) {
    // Adapted from Python random module
    return exp(ATK_RandNormal(gen, mu, sigma));
}

/**
 * Returns the next element in the exponential distribution.
 *
 * The value mu is the desired mean. It should be nonzero. Returned values
 * range from 0 to positive infinity if mu is positive, and from negative
 * infinity to 0 if mu is negative.
 *
 * @param gen   The psuedorandom number generator
 * @param mu    The standard deviation
 *
 * @return the next element in the exponential distribution.
 */
static inline double ATK_RandExp(ATK_RandGen* gen, double mu) {
    // Adapted from Python random module
    double lambda = 1.0/mu;
    return -log(1.0 - ATK_RandHalfOpenDouble(gen)) / lambda;
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
 * The mean is is alpha/beta, and the variance is alpha/(beta^2).
 *
 * @param gen   The psuedorandom number generator
 * @param alpha	The shape parameter (should be > 0)
 * @param beta	The rate parameter (should be > 0)
 *
 * @return the next element in the gamma distribution.
 */
extern DECLSPEC double SDLCALL ATK_RandGamma(ATK_RandGen* gen, double alpha, double beta);

/**
 * Returns the next element in the beta distribution.
 *
 * The parameters alpha and beta should be positive. The values returned
 * are between 0 and 1.
 *
 * The mean is alpha/(alpha+beta) and the variance is
 * (alpha*beta)/((alpha+beta+1)*(alpha+beta)^2)
 *
 * @param gen   The psuedorandom number generator
 * @param alpha The first shape parameter (should be > 0)
 * @param beta  The second shape parameter (should be > 0)
 *
 * @return the next element in the beta distribution.
 */
extern DECLSPEC double SDLCALL ATK_RandBeta(ATK_RandGen* gen, double alpha, double beta);

/**
 * Returns the next element in the Pareto distribution.
 *
 * The mean is infty for alpha <= 1 and (alpha*xm)/(alpha-1) for alpha > 1.
 * The variance is infty for alpha <= 2 and (alpha*xm^2)/((alpha-2)*(alpha-1)^2)
 * for alpha > 2.
 *
 * @param gen   The psuedorandom number generator
 * @param xm    The scale parameter
 * @param alpha The shape parameter
 *
 * @return the next element in the Pareto distribution.
 */
static inline double ATK_RandPareto(ATK_RandGen* gen, double xm, double alpha) {
    // From https://en.wikipedia.org/wiki/Pareto_distribution
    double u = 1.0 - ATK_RandHalfOpenDouble(gen);
    return pow(u,-1.0 / alpha)*xm;
}

/**
 * Returns the next element in the Weibull distribution.
 *
 * The mean is lambda * gamma(1+1/k) and the variance is
 * lambda^2 * (gamma(1+2/k)-gamma(1+1/k)^2), where gamma()
 * is the gamma function.
 *
 * @param gen       The psuedorandom number generator
 * @param k         The shape parameter
 * @param lambda    The scale parameter
 *
 * @return the next element in the Weibull distribution.
 */
static inline double ATK_RandWeibull(ATK_RandGen* gen, double k, double lambda) {
    // From https://en.wikipedia.org/wiki/Weibull_distribution
    double u = 1.0 - ATK_RandHalfOpenDouble(gen);
    return lambda * pow(-log(u), 1.0 / k);
}

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif /* __SDL_RAND_H__ */
