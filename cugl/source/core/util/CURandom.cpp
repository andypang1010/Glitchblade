//
//  CURandom.cpp
//  Cornell University Game Library (CUGL)
//
//  This module an implementation of a Marsenne Twister pseudo-random generator.
//  While this functionality is built directly into modern C++, this version
//  is a lot easier for students to use, and it comes with additional methods
//  that replicate the random package found in Python.
//
//  This module mostly guarantees cross-platform determinsm. All of the base
//  methods will return the same value sequence on any platform. However, the
//  distribution functions use complex math functions and therefore are not
//  guaranteed to agree cross platform.
//
//  This class uses our standard shared-pointer architecture.
//
//  1. The constructor does not perform any initialization; it just sets all
//     attributes to their defaults.
//
//  2. All initialization takes place via init methods, which can fail if an
//     object is initialized more than once.
//
//  3. All allocation takes place via static constructors which return a shared
//     pointer.
//
//
//  CUGL MIT License:
//      This software is provided 'as-is', without any express or implied
//      warranty.  In no event will the authors be held liable for any damages
//      arising from the use of this software.
//
//      Permission is granted to anyone to use this software for any purpose,
//      including commercial applications, and to alter it and redistribute it
//      freely, subject to the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not
//      be misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
//
//  Author: Walker White
//  Version: 7/3/24 (CUGL 3.0 reorganization)
//
#include <cugl/core/util/CURandom.h>
#include <ctime>

using namespace cugl;

/**
 * Initializes a psuedorandom number generator with the current time.
 *
 * The random number generator is the classic 64 bit version implement here
 *
 *     http://www.math.sci.hiroshima-u.ac.jp/m-mat/MT/VERSIONS/C-LANG/mt19937-64.c
 *
 * @return true if initialization was successful
 */
bool Random::init() {
    if (_generator != NULL) {
        return false;
    }
    _seed = time(NULL);
    _generator = ATK_AllocRand(_seed);
    return _generator != NULL;
}

/**
 * Initializes a psuedorandom number generator with the given seed
 *
 * The random number generator is the classic 64 bit version implement here
 *
 *     http://www.math.sci.hiroshima-u.ac.jp/m-mat/MT/VERSIONS/C-LANG/mt19937-64.c
 *
 * Generators with the same seed will generate the same numbers.
 *
 * @param seed  The generator seed
 *
 * @return true if initialization was successful
 */
bool Random::initWithSeed(Uint64 seed) {
    if (_generator != NULL) {
        return false;
    }
    _seed = seed;
    _generator = ATK_AllocRand(seed);
    return _generator != NULL;
}

/**
 * Initializes a psuedorandom number generator with the given keys
 *
 * The random number generator is the classic 64 bit version implement here
 *
 *     http://www.math.sci.hiroshima-u.ac.jp/m-mat/MT/VERSIONS/C-LANG/mt19937-64.c
 *
 * Generators with the same key sequence will generate the same numbers.
 *
 * @param key   The array of generator keys
 * @param len   The key length
 *
 * @return true if initialization was successful
 */
bool Random::initWithArray(Uint64* key, size_t len) {
    if (_generator != NULL) {
        return false;
    }
    _seed = 0;
    _array.assign(key,key+len);
    _generator = ATK_AllocRandByArray(key,len);
    return _generator != NULL;
}


/**
 * Dispose the psuedo-random sequence for this generator
 *
 * You must reinitialize the generator to use it.
 */
void Random::dispose() {
    if (_generator != NULL) {
        ATK_FreeRand(_generator);
        _generator = NULL;
    }
    _seed = 0;
    _array.clear();
}


/**
 * Resets the random generator to use the current time as the seed.
 *
 * The previous pseudo-random sequence will be discarded and replaced
 * with the new one. Generation will start at the beginning of the new
 * sequence.
 */
void Random::reset() {
    reset(time(NULL));
}

/**
 * Resets the random generator to use the given seed.
 *
 * The previous pseudo-random sequence will be discarded and replaced
 * with the new one. Generation will start at the beginning of the new
 * sequence.
 *
 * @param seed  The new generator seed
 */
void Random::reset(Uint64 seed) {
    _seed = seed;
    _array.clear();
    if (_generator != NULL) {
        ATK_ResetRand(_generator, seed);
    } else {
        _generator = ATK_AllocRand(seed);
    }
}

/**
 * Resets the random generator to use the given keys.
 *
 * The previous pseudo-random sequence will be discarded and replaced
 * with the new one. Generation will start at the beginning of the new
 * sequence.
 *
 * @param key   The array of generator keys
 * @param len   The key length
 */
void Random::reset(Uint64* key, size_t len) {
    _seed = 0;
    _array.clear();
    _array.assign(key, key+len);
    if (_generator != NULL) {
        ATK_ResetRandByArray(_generator, key, len);
    } else {
        _generator = ATK_AllocRandByArray(key,len);
    }
}
