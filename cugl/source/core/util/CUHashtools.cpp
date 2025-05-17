//
//  CUHashtools.cpp
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
#include <cugl/core/util/CUHashtools.h>
#include <cugl/core/util/CUDebug.h>
#include <random>
#include <stduuid/uuid.h>
#include <SDL_app.h>

// Seed taken from documentation. Could be changed.
#define UUID_SEED "47183823-2574-4bfd-b411-99ed177d3e43"

// String for Base 64 conversion
#define BASE64_ALPHA "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
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
std::string cugl::hashtool::b64_encode(const std::vector<std::byte>& data) {
    // https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c
    std::string result;

    int val = 0, valb = -6;
    for (std::byte b : data) {
        val = (val << 8) + static_cast<int>(b);
        valb += 8;
        while (valb >= 0) {
            result.push_back(BASE64_ALPHA[(val>>valb)&0x3F]);
            valb -= 6;
        }
    }
 
    if (valb>-6) result.push_back(BASE64_ALPHA[((val<<8)>>(valb+8))&0x3F]);
    while (result.size() % 4) {
        result.push_back('=');
    }
    return result;
}

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
std::string cugl::hashtool::b64_encode(const std::string data) {
    // https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c
    std::string result;

    int val = 0, valb = -6;
    for (unsigned char c : data) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            result.push_back(BASE64_ALPHA[(val>>valb)&0x3F]);
            valb -= 6;
        }
    }
 
    if (valb>-6) result.push_back(BASE64_ALPHA[((val<<8)>>(valb+8))&0x3F]);
    while (result.size() % 4) {
        result.push_back('=');
    }
    return result;
}

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
std::vector<std::byte> cugl::hashtool::b64_decode(const std::string data) {
    std::vector<std::byte> result;

    // Create a reverse lookup table
    std::vector<int> lookup(256,-1);
    for (int i=0; i<64; i++) {
        lookup[BASE64_ALPHA[i]] = i;
    }

    int val=0, valb=-8;
    for (unsigned char c : data) {
        if (lookup[c] == -1) {
            break;
        }
        
        val = (val << 6) + lookup[c];
        valb += 6;
        if (valb >= 0) {
            result.push_back(static_cast<std::byte>((val>>valb)&0xFF));
            valb -= 8;
        }
    }
    return result;
}

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
std::string cugl::hashtool::b64_tostring(const std::string data) {
    std::string result;

    // Create a reverse lookup table
    std::vector<int> lookup(256,-1);
    for (int i=0; i<64; i++) {
        lookup[BASE64_ALPHA[i]] = i;
    }

    int val=0, valb=-8;
    for (unsigned char c : data) {
        if (lookup[c] == -1) {
            break;
        }
        
        val = (val << 6) + lookup[c];
        valb += 6;
        if (valb >= 0) {
            result.push_back(static_cast<char>((val>>valb)&0xFF));
            valb -= 8;
        }
    }
    return result;
}

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
std::string cugl::hashtool::generate_uuid() {
    std::random_device rd;
    auto seed_data = std::array<int, std::mt19937::state_size> {};
    std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
    std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
    std::mt19937 generator(seq);
    uuids::uuid_random_generator uuidgen{generator};
    
    uuids::uuid const typeduuid = uuidgen();
    std::string uuid = uuids::to_string(typeduuid);
    return uuid;
}

/**
 * Returns true if this device has a unique system UUID.
 *
 * If this function returns false, {@link system_uuid} is undefined.
 *
 * @return true if this device has a unique system UUID.
 */
bool cugl::hashtool::has_system_uuid() {
    return APP_GetDeviceID() != NULL;
}

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
 * separating the characters into groups of 8-4-4-4-12.
 *
 * @return a UUID representing this device.
 */
std::string cugl::hashtool::system_uuid() {
    // First we need to get a non-zero string
    std::string seed(APP_GetDeviceID());
    if (seed.empty()) {
        seed = APP_GetDeviceName();
    }
    if (seed.empty()) {
        CUAssertLog(false,"Could not acquire system information");
        return "";
    }
    
    uuids::uuid_name_generator uuidgen(uuids::uuid::from_string(UUID_SEED).value());
    uuids::uuid const typeduuid = uuidgen(seed);
    std::string uuid = uuids::to_string(typeduuid);
    return uuid;
}


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
std::string cugl::hashtool::create_uuid(const std::string seed) {
    uuids::uuid_name_generator uuidgen(uuids::uuid::from_string(UUID_SEED).value());
    uuids::uuid const typeduuid = uuidgen(seed);
    std::string uuid = uuids::to_string(typeduuid);
    return uuid;
}
