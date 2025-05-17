//
//  CURandom.h
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
#ifndef __CU_RANDOM_H__
#define __CU_RANDOM_H__
#include <vector>
#include <SDL_atk.h>
#include <cugl/core/util/CUDebug.h>

namespace cugl {

/**
 * This class is a 64 bit Marseene Twister psuedo-random generator.
 *
 * This generator is guaranteed to be cross-platform with respect to random
 * integers. So, given the same seed, any two different platforms will generate
 * the same sequence of random integers.
 *
 * For the case of reals (e.g. doubles), cross-platform support depends on IEEE
 * 754, which is supported by all modern hardware. Any two platforms that
 * support IEEE 754 should generate the same numbers for the same hardware.
 *
 * However, there are no cross-platform guarantees for any of the distribution
 * functions like {@link #getNormal} or {@link #getGamma}. This distributions
 * use complex mathematical functions that may be implemented differently on
 * different platforms.
 */
class Random {
private:
    /** The ATK generator that powers this C++ class */
    ATK_RandGen* _generator;
    /** The initial seed (0 if an array was used) */
    Uint64 _seed;
    /** The initial seed array (empty if a seed number was used) */
    std::vector<Uint64> _array;
public:
#pragma mark Constructors
    /**
     * Creates an uninitialized generator with no sequence.
     *
     * You must initialize the generator to create a pseudo-random sequence.
     */
    Random() : _seed(0) { _generator = NULL; }
    
    /**
     * Deletes this generator, disposing all resources.
     */
    ~Random() { dispose(); };

    /**
     * Disposes the psuedo-random sequence for this generator
     *
     * You must reinitialize the generator to use it.
     */
    void dispose();
    
    /**
     * Initializes a psuedorandom number generator with the current time.
     *
     * The random number generator is the classic 64 bit version implement here
     *
     *     http://www.math.sci.hiroshima-u.ac.jp/m-mat/MT/VERSIONS/C-LANG/mt19937-64.c
     *
     * @return true if initialization was successful
     */
    bool init();
    
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
    bool initWithSeed(Uint64 seed);
    
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
    bool initWithArray(Uint64* key, size_t len);
    
    /**
     * Returns a newly allocated psuedorandom number generator with the given time
     *
     * The random number generator is the classic 64 bit version implement here
     *
     *     http://www.math.sci.hiroshima-u.ac.jp/m-mat/MT/VERSIONS/C-LANG/mt19937-64.c
     *
     * @return a newly allocated psuedorandom number generator with the given time
     */
    static std::shared_ptr<Random> alloc() {
        std::shared_ptr<Random> result = std::make_shared<Random>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated psuedorandom number generator with the given seed
     *
     * The random number generator is the classic 64 bit version implement here
     *
     *     http://www.math.sci.hiroshima-u.ac.jp/m-mat/MT/VERSIONS/C-LANG/mt19937-64.c
     *
     * Generators with the same seed will generate the same numbers.
     *
     * @param seed  The generator seed
     *
     * @return a newly allocated psuedorandom number generator with the given seed
     */
    static std::shared_ptr<Random> allocWithSeed(Uint64 seed) {
        std::shared_ptr<Random> result = std::make_shared<Random>();
        return (result->initWithSeed(seed) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated psuedorandom number generator with the given keys
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
     * @return a newly allocated psuedorandom number generator with the given keys
     */
    static std::shared_ptr<Random> allocWithArray(Uint64* key, size_t len) {
        std::shared_ptr<Random> result = std::make_shared<Random>();
        return (result->initWithArray(key,len) ? result : nullptr);
    }
    
#pragma mark Seeding
    /**
     * Resets the random generator to use the current time as the seed.
     *
     * The previous pseudo-random sequence will be discarded and replaced
     * with the new one. Generation will start at the beginning of the new
     * sequence.
     */
    void reset();
    
    /**
     * Resets the random generator to use the given seed.
     *
     * The previous pseudo-random sequence will be discarded and replaced
     * with the new one. Generation will start at the beginning of the new
     * sequence.
     *
     * @param seed  The new generator seed
     */
    void reset(Uint64 seed);
    
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
    void reset(Uint64* key, size_t len);
    
    /**
     * Returns the seed for the given generator.
     *
     * If a key sequence was used in place of a seed, this method will return
     * 0.
     *
     * @return the seed for the given generator.
     */
    Uint64 getSeed() const { return _seed; }

    /**
     * Returns the key sequence for the given generator.
     *
     * If a seed was used in place of a key sequence, this method will return
     * an empty vector.
     *
     * @return the seed for the given generator.
     */
    const std::vector<Uint64>& getKeys() const { return _array; }

#pragma mark Integers
    /**
     * Returns the next pseudorandom integer in [0, 2^64-1]
     *
     * @return the next pseudorandom integer in [0, 2^64-1]
     */
    Uint64 getUint64() {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        return ATK_RandUint64(_generator);
    }
    
    /**
     * Returns the next pseudorandom integer in [-2^63, 2^63-1]
     *
     * @return the next pseudorandom integer in [-2^63, 2^63-1]
     */
    Sint64 getSint64() {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        return ATK_RandSint64(_generator);
    }
    
    /**
     * Returns the next pseudorandom integer in [min, max)
     *
     * If min >= max, the result is undefined.
     *
     * @return the next pseudorandom integer in [min, max)
     */
    Uint64 getOpenUint64(Uint64 min, Uint64 max) {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        CUAssertLog(min < max, "Range %lu..%lu is undefined",(unsigned long)min,(unsigned long)max);
        return ATK_RandUint64OpenRange(_generator, min, max);
    }
    
    /**
     * Returns the next pseudorandom integer in [min, max]
     *
     * If min >= max, the result is undefined.
     *
     * @return the next pseudorandom integer in [min, max]
     */
    Uint64 getClosedUint64(Uint64 min, Uint64 max) {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        CUAssertLog(min < max, "Range %lu..%lu is undefined",(unsigned long)min,(unsigned long)max);
       return ATK_RandUint64ClosedRange(_generator, min, max);
    }
    
    /**
     * Returns the next pseudorandom signed integer in [min, max)
     *
     * If min >= max, the result is undefined.
     *
     * @return the next pseudorandom signed integer in [min, max)
     */
    Sint64 getOpenSint64(Sint64 min, Sint64 max) {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        CUAssertLog(min < max, "Range %ld..%ld is undefined",(long)min,(long)max);
        return ATK_RandSint64OpenRange(_generator, min, max);
    }
    
    /**
     * Returns the next pseudorandom signed integer in [min, max]
     *
     * If min >= max, the result is undefined.
     *
     * @return the next pseudorandom signed integer in [min, max]
     */
    Sint64 getClosedSint64(Sint64 min, Sint64 max) {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        CUAssertLog(min < max, "Range %ld..%ld is undefined",(long)min,(long)max);
        return ATK_RandSint64ClosedRange(_generator, min, max);
    }

    /**
     * Returns the next pseudorandom integer in [0, 2^32-1]
     *
     * @return the next pseudorandom integer in [0, 2^32-1]
     */
    Uint32 getUint32() {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        return ATK_RandUint32(_generator);
    }

    /**
     * Returns the next pseudorandom integer in [-2^31, 2^31-1]
     *
     * @return the next pseudorandom integer in [-2^31, 2^31-1]
     */
    Sint32 getSint32() {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        return ATK_RandSint32(_generator);
    }

    /**
     * Returns the next pseudorandom integer in [0, 2^16-1]
     *
     * @return the next pseudorandom integer in [0, 2^16-1]
     */
    Uint16 getUint16() {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        return ATK_RandUint16(_generator);
    }

    /**
     * Returns the next pseudorandom integer in [-2^15, 2^15-1]
     *
     * @return the next pseudorandom integer in [-2^15, 2^15-1]
     */
    Sint16 getSint16() {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        return ATK_RandSint16(_generator);
    }

    /**
     * Returns the next pseudorandom integer in [0, 255]
     *
     * @return the next pseudorandom integer in [0, 255]
     */
    Uint8 getUint8() {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        return ATK_RandUint8(_generator);
    }


    /**
     * Returns the next pseudorandom integer in [-128,127]
     *
     * @return the next pseudorandom integer in [-128,127]
     */
    Sint8 getSint8() {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        return ATK_RandSint8(_generator);
    }


    /**
     * Returns the next pseudorandom value true or false.
     *
     * @return the next pseudorandom value true or false.
     */
    bool getBool() {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        return ATK_RandUint64(_generator) % 2 == 0;
    }
    
#pragma mark Reals
    /**
     * Returns the next pseudorandom double in [0,1)
     *
     * Only the endpoint 0 is included. To get a random double in the interval
     * (0,1], simply subtract this number from 1.
     *
     * This function is equivalent to {@link #getHalfOpenDouble}, as that is
     * often the desired behavior of random generators.
     *
     * @return the next pseudorandom double in [0,1)
     */
    double getDouble() {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        return ATK_RandDouble(_generator);
    }

    /**
     * Returns the next pseudorandom double in [0,1]
     *
     * Both endpoints of the interval are included.
     *
     * @return the next pseudorandom double in [0,1]
     */
    double getClosedDouble() {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        return ATK_RandClosedDouble(_generator);
    }

    /**
     * Returns the next pseudorandom double in [0,1)
     *
     * Only the endpoint 0 is included. To get a random double in the interval
     * (0,1], simply subtract this number from 1.
     *
     * @return the next pseudorandom double in [0,1)
     */
    double getHalfOpenDouble() {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        return ATK_RandHalfOpenDouble(_generator);
    }

    /**
     * Returns the next pseudorandom double in (0,1)
     *
     * Neither endpoint in the interval is included.
     *
     * @return the next pseudorandom double in (0,1)
     */
    double getOpenDouble() {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        return ATK_RandOpenDouble(_generator);
    }

    /**
     * Returns the next pseudorandom double in [min,max]
     *
     * Both endpoints of the interval are included. If min > max, the result
     * is undefined.
     *
     * @return the next pseudorandom double in [min,max]
     */
    double getClosedDouble(double min, double max) {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        CUAssertLog(min < max, "Range %f..%f is undefined",min,max);
        return ATK_RandDoubleClosedRange(_generator,min,max);
    }

    /**
     * Returns the next pseudorandom double in (min,max)
     *
     * Neither of the endpoints of the interval are included. If min >= max,
     * the result is undefined.
     *
     * @return the next pseudorandom double in (min,max)
     */
    double getOpenDouble(double min, double max) {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        CUAssertLog(min < max, "Range %f..%f is undefined",min,max);
        return ATK_RandDoubleOpenRange(_generator,min,max);
    }

    /**
     * Returns the next pseudorandom double in [min,max)
     *
     * Only the first endpoint of the interval is included. If min >= max,
     * the result is undefined.
     *
     * @return the next pseudorandom double in [min,max)
     */
    double getRightOpenDouble(double min, double max) {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        CUAssertLog(min < max, "Range %f..%f is undefined",min,max);
        return ATK_RandDoubleRightOpenRange(_generator,min,max);
    }

    /**
     * Returns the next pseudorandom double in (min,max]
     *
     * Only the second endpoint of the interval is included. If min >= max,
     * the result is undefined.
     *
     * @return the next pseudorandom double in (min,max]
     */
    double getLeftOpenDouble(double min, double max) {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        CUAssertLog(min < max, "Range %f..%f is undefined",min,max);
        return ATK_RandDoubleLeftOpenRange(_generator,min,max);
    }

    /**
     * Returns the next pseudorandom float in [0,1)
     *
     * Only the endpoint 0 is included. To get a random double in the interval
     * (0,1], simply subtract this number from 1.
     *
     * @return the next pseudorandom float in [0,1)
     */
    double getFloat() {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        return ATK_RandFloat(_generator);
    }

    /**
     * Returns the next pseudorandom float in [min,max]
     *
     * Both endpoints of the interval are included. If min > max, the result
     * is undefined.
     *
     * @return the next pseudorandom float in [min,max]
     */
    float getClosedFloat(float min, float max) {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        CUAssertLog(min < max, "Range %f..%f is undefined",min,max);
        return ATK_RandFloatClosedRange(_generator,min,max);
    }

    /**
     * Returns the next pseudorandom float in (min,max)
     *
     * Neither of the endpoints of the interval are included. If min >= max,
     * the result is undefined.
     *
     * @return the next pseudorandom float in (min,max)
     */
    float getOpenFloat(float min, float max) {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        CUAssertLog(min < max, "Range %f..%f is undefined",min,max);
        return ATK_RandFloatOpenRange(_generator,min,max);
    }

    /**
     * Returns the next pseudorandom float in [min,max)
     *
     * Only the first endpoint of the interval is included. If min >= max,
     * the result is undefined.
     *
     * @return the next pseudorandom float in [min,max)
     */
    float getRightOpenFloat(float min, float max) {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        CUAssertLog(min < max, "Range %f..%f is undefined",min,max);
        return ATK_RandFloatRightOpenRange(_generator,min,max);
    }

    /**
     * Returns the next pseudorandom float in (min,max]
     *
     * Only the second endpoint of the interval is included. If min >= max,
     * the result is undefined.
     *
     * @return the next pseudorandom float in (min,max]
     */
    float getLeftOpenFloat(float min, float max) {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        CUAssertLog(min < max, "Range %f..%f is undefined",min,max);
        return ATK_RandFloatLeftOpenRange(_generator,min,max);
    }

#pragma mark Selection
    /**
     * Returns a pointer to a randomly selected item in data
     *
     * The function works on an array of data. The length must be specified.
     * If len == 0, this function returns nullptr.
     *
     * @param data  The array to select from
     * @param len   The number of elements in the array
     *
     * @return a pointer to a randomly selected item in data
     */
    template <typename T>
    T* getChoice(T* data, size_t len) {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        return (T*)ATK_RandChoice(_generator,data,sizeof(T),len);
    }

    /**
     * Returns a pointer to a randomly selected item in data
     *
     * The function works on a vector. If the vector is empty, this function
     * returns nullptr.
     *
     * @param data  The vector to select from
     *
     * @return a pointer to a randomly selected item in data
     */
    template <typename T>
    T* getChoice(const std::vector<T>& data) {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        if (data.empty()) {
            return nullptr;
        }
        T* array = data.data();
        return (T*)ATK_RandChoice(_generator,array,sizeof(T),data.size());
    }
    
    /**
     * Randomly shuffles the data in place
     *
     * The function works on an array of data. The length must be specified.
     *
     * @param data  The array to shuffle
     * @param len   The number of elements in the array
     */
    template <typename T>
    void shuffle(T* data, size_t len) {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        ATK_RandShuffle(_generator,data,sizeof(T),len);
    }
    
    /**
     * Randomly shuffles the data in place
     *
     * The function works on a vector.
     *
     * @param data  The vector to shuffle
     */
    template <typename T>
    void shuffle(std::vector<T>& data) {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        T* array = data.data();
        ATK_RandShuffle(_generator,array,sizeof(T),data.size());
    }

#pragma mark Distributions
    /**
     * Returns the next element in the normal distribution.
     *
     * The value mu is the mean, and sigma is the standard deviation. Parameter
     * mu can have any value, and sigma must be greater than zero.
     *
     * @param mu    The mean
     * @param sigma The standard deviation
     *
     * @return the next element in the normal distribution.
     */
    double getNormal(double mu, double sigma)  {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        return ATK_RandNormal(_generator,mu,sigma);
    }

    /**
     * Returns the next element in the log normal distribution.
     *
     * If you take the natural logarithm of this distribution, you will get a
     * normal distribution with mean mu and standard deviation sigma. Parameter
     * mu can have any value, and sigma must be greater than zero.
     *
     * @param mu    The mean
     * @param sigma The standard deviation
     *
     * @return the next element in the log normal distribution.
     */
    double getLogNorm(double mu, double sigma) {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        return ATK_RandLogNorm(_generator,mu,sigma);
    }

    /**
     * Returns the next element in the exponential distribution.
     *
     * The value mu is the desired mean. It should be nonzero. Returned values
     * range from 0 to positive infinity if mu is positive, and from negative
     * infinity to 0 if mu is negative.
     *
     * @param mu    The standard deviation
     *
     * @return the next element in the exponential distribution.
     */
    double getExp(double mu) {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        return ATK_RandExp(_generator,mu);
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
     * @param alpha The shape parameter (should be > 0)
     * @param beta  The rate parameter (should be > 0)
     *
     * @return the next element in the gamma distribution.
     */
    double getGamma(double alpha, double beta) {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        return ATK_RandGamma(_generator,alpha,beta);
    }

    /**
     * Returns the next element in the beta distribution.
     *
     * The parameters alpha and beta should be positive. The values returned
     * are between 0 and 1.
     *
     * The mean is alpha/(alpha+beta) and the variance is
     * (alpha*beta)/((alpha+beta+1)*(alpha+beta)^2)
     *
     * @param alpha The first shape parameter (should be > 0)
     * @param beta  The second shape parameter (should be > 0)
     *
     * @return the next element in the beta distribution.
     */
    double getBeta(double alpha, double beta) {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        return ATK_RandBeta(_generator,alpha,beta);
    }
    
    /**
     * Returns the next element in the Pareto distribution.
     *
     * The mean is infty for alpha <= 1 and (alpha*xm)/(alpha-1) for alpha > 1.
     * The variance is infty for alpha <= 2 and (alpha*xm^2)/((alpha-2)*(alpha-1)^2)
     * for alpha > 2.
     *
     * @param xm    The scale parameter
     * @param alpha The shape parameter
     *
     * @return the next element in the Pareto distribution.
     */
    double getPareto(double xm, double alpha) {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        return ATK_RandPareto(_generator,xm,alpha);
    }

    /**
     * Returns the next element in the Weibull distribution.
     *
     * The mean is lambda * gamma(1+1/k) and the variance is
     *
     *     lambda^2 * (gamma(1+2/k)-gamma(1+1/k)^2)
     *
     * where gamma() is the gamma function.
     *
     * @param k         The shape parameter
     * @param lambda    The scale parameter
     *
     * @return the next element in the Weibull distribution.
     */
    double getWeibull(double k, double lambda) {
        CUAssertLog(_generator != nullptr, "Random generator has not be initialized");
        return ATK_RandWeibull(_generator,k,lambda);
    }
    
};
}

#endif /* __CU_RANDOM_H__ */
