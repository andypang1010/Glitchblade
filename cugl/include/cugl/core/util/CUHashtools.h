//
//  CUHashtools.h
//  Cornell University Game Library (CUGL)
//
//  This module is a collection of functions to simplify the creation of
//  user-defined hash functions in CUGL. Most of these ideas have been
//  adapted from existing online tools like the BOOST library.
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
#ifndef __CU_HASHTOOLS_H__
#define __CU_HASHTOOLS_H__
#include <string>
#include <vector>
#include <memory>
#include <cstddef>

namespace cugl {
    /**
     * Functions for custom hash functions
     *
     * This namespace provides several tools creating user-defined hash functions
     * and universally unique identifiers (UUIDs). The latter is important for the
     * networking libraries, as it provides a way of uniquely referencing machines
     * across the network.
     */
    namespace hashtool {

/**
 * Provides the base case for a recursive hash_combiner.
 *
 * A hash_combiner is used to produce a hash function on a user-defined type
 * by recursively combining the (built-in) hash functions for the individual
 * attributes. It does this by performing recursion over variadic templates.
 *
 * This is function is the base (untemplated) version. It does nothing by
 * itself, and should never be called directly.
 *
 * This function is taken from
 *
 *      https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
 *
 * @param seed  The combination seed (communicates recursive depth)
 */
inline void hash_combine(std::size_t& seed) { }

/**
 * Provides the recursive case for a recursive hash_combiner.
 *
 * A hash_combiner is used to produce a hash function on a user-defined type
 * by recursively combining the (built-in) hash functions for the individual
 * attributes. It does this by performing recursion over variadic templates.
 *
 * This is function is the recursive (templated) version. To use it, call it
 * on all your attributes using a seed of 0.
 *
 * This function is taken from
 *
 *      https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
 *
 * @param seed  The combination seed (communicates recursive depth)
 * @param v     The first attribute to combine
 * @param rest  The remaining attributes to combine
 */
template <typename T, typename... Rest>
inline void hash_combine(std::size_t& seed, const T& v, Rest... rest) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
    hash_combine(seed, rest...);
}
    
/**
 * Returns a text representation of the given binary data in Base 64
 *
 * This function represents the given binary data as printable characters,
 * making it safe for network transfer over HTTP or similar protocols. See
 *
 * https://en.wikipedia.org/wiki/Base64
 *
 * @param data  The data to convert
 *
 * @return a text representation of the given binary data in Base 64
 */
std::string b64_encode(const std::vector<std::byte>& data);

/**
 * Returns a text representation of the given string in Base 64
 *
 * Strings in C++ are arbitrary lists of characters, and do not have to be
 * be printable. This means that they are not always safe for use with HTTP
 * or similar protocols. This function encodes the string to make it safe for
 * transmission. See
 *
 * https://en.wikipedia.org/wiki/Base64
 *
 * @param data  The data to convert
 *
 * @return a text representation of the given binary data in Base 64
 */
std::string b64_encode(const std::string data);

/**
 * Returns a byte vector decoded from the given Base 64 data
 *
 * It is assumed that the data provided is a Base 64 encoding of binary data.
 * This is a representation of binary data as all printable characters. See
 *
 * https://en.wikipedia.org/wiki/Base64
 *
 * Obviously {@link b64_encode} produces such data, but the data could have
 * been produced through other means, such as via Python.
 *
 * @param data  The data to convert
 *
 * @return a byte vector decoded from the given Base 64 data
 */
std::vector<std::byte> b64_decode(const std::string data);

/**
 * Returns a string decoded from the given Base 64 data
 *
 * This function is an alternative to {@link b64_decode} that produces a string
 * instead of a byte vector. It is for cases in which the user is working with
 * strings that contain unprintable characters (e.g. UTF-8 strings).
 *
 * @param data  The data to convert
 *
 * @return a string decoded from the given Base 64 data
 */
std::string b64_tostring(const std::string data);

/**
 * Returns a new randomly generated UUID
 *
 * This creates a Version 4 UUID generated by a fresh std::random_device. It
 * will be a 32 character hexadecimal string with dashes separating the
 * characters into groups of 8-4-4-4-12 (as well as supporting Version 4
 * markers).
 *
 * @return a new randomly generated UUID
 */
std::string generate_uuid();


/**
 * Returns true if this device has a unique system UUID.
 *
 * If this function returns false, {@link system_uuid} is undefined.
 *
 * @return true if this device has a unique system UUID.
 */
bool has_system_uuid();
    
/**
 * Returns a UUID representing this device.
 *
 * This creates a Version 5 UUID hashed upon a system identifer. This system
 * identifier is, at best, guaranteed to be a "vendor id". That means that can
 * identify this device across all applications deployed by a single 
 * developer/company but cannot be used cross developers. It can also be lost
 * if the user uninstalls and reinstalls the application.
 *
 * As a Version 5 uuid, this is a 32 character hexadecimal string with dashes
 * separating the characters into groups of 8-4-4-4-12 (as well as supporting 
 * Version 5 markers).
 *
 * @return a UUID representing this device.
 */
std::string system_uuid();

/**
 * Returns a UUID for the given seed.
 *
 * This creates a Version 5 UUID hashed the provided string. Identical seeds
 * will produce identical UUIDs. As a Version 5 uuid, this is a 32 character
 * hexadecimal string with dashes separating the characters into groups of
 * 8-4-4-4-12 (as well as supporting Version 5 markers).
 *
 * @return a UUID for the given seed.
 */
std::string create_uuid(const std::string seed);
    }
}

#endif /* __CU_HASHTOOLS_H__ */
