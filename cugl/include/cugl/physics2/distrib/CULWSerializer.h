//
//  CULWSerializer.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a lightweight serializer for networked physics. It
//  removes the type safety of the Netcode Serializer class, and relies on
//  the user to know the type of the data. However, it is a more space
//  efficient serializer, and is more appropriate for networked physics.
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
//  Author: Barry Lyu
//  Version: 7/3/24 (CUGL 3.0 reorganization)
//
#ifndef __CU_LW_SERIALIZER_H__
#define __CU_LW_SERIALIZER_H__

#include <cugl/core/util/CUEndian.h>
#include <SDL_stdinc.h>
#include <vector>
#include <memory>

namespace cugl {

    /**
     * The classes to represent 2-d physics.
     *
     * For 2-d physics, CUGL uses the venerable box2d. For the most part, we
     * do not need anything more than that. However, box2d does involve a lot
     * of boilerplate code in setting up bodies and fixtures. Students have
     * found that they like the "training wheel" classes in this package.
     */
    namespace physics2 {
    
        /**
         * The classes to implement distributed box2d physics.
         *
         * This namespace represents an extension of our 2-d physics engine
         * to support networking. This package provides automatic synchronization
         * of physics objects across devices.
         */
        namespace distrib {

    
/**
 * A lightweight serializer for networked physics.
 *
 * This class removes the type safety of {@link cugl::netcode::NetcodeSerializer},
 * and requires that the user know the type of the data. However, it is a more
 * space efficient serializer, and is more appropriate for networked physics.
 *
 * This class is to be paired with {@link LWDeserializer} for deserialization.
 */
class LWSerializer{
private:
    /** The buffered serialized data */
    std::vector<std::byte> _data;
    
public:
    /**
     * Creates a new LWSerializer on the stack.
     *
     * Serializers do not have any nontrivial state and so it is unnecessary
     * to use an init method. However, we do include a static {@link #alloc}
     * method for creating shared pointers.
     */
    LWSerializer() {}
    
    /**
     * Returns a newly allocated LWSerializer.
     *
     * This method is solely include for convenience purposes.
     *
     * @return a newly allocated LWSerializer.
     */
    static std::shared_ptr<LWSerializer> alloc() {
        return std::make_shared<LWSerializer>();
    }
    
    /**
     * Writes a single boolean value to the buffer.
     *
     * Values will be deserialized on other machines in the same order they were
     * written in.
     *
     * @param b The value to write
     */
    void writeBool(bool b){
        _data.push_back(b ? std::byte(1) : std::byte(0));
    }
    
    /**
     * Writes a single byte value to the buffer.
     *
     * Values will be deserialized on other machines in the same order they were
     * written in.
     *
     * @param b The value to write
     */
    void writeByte(std::byte b){
        _data.push_back(b);
    }
    
    /**
     * Writes a single float value to the input buffer.
     *
     * Values will be deserialized on other machines in the same order they were
     * written in.
     *
     * @param f The float to write
     */
    void writeFloat(float f) {
        float ii = marshall(f);
        const std::byte* bytes = reinterpret_cast<const std::byte*>(&ii);
        for (size_t j = 0; j < sizeof(float); j++) {
            _data.push_back(bytes[j]);
        }
    }
    
    /**
     * Writes a signed 32-bit integer to the input buffer.
     *
     * Values will be deserialized on other machines in the same order they were
     * written in.
     *
     * @param i The Sint32 to write
     */
    void writeSint32(Sint32 i){
        Sint32 ii = marshall(i);
        const std::byte* bytes = reinterpret_cast<const std::byte*>(&ii);
        for (size_t j = 0; j < sizeof(Sint32); j++) {
            _data.push_back(bytes[j]);
        }
    }
    
    /**
     * Writes an unsigned 16-bit integer to the input buffer.
     *
     * Values will be deserialized on other machines in the same order they were
     * written in.
     *
     * @param i The Uint16 to write
     */
    void writeUint16(Uint16 i){
        Uint16 ii = marshall(i);
        const std::byte* bytes = reinterpret_cast<const std::byte*>(&ii);
        for (size_t j = 0; j < sizeof(Uint16); j++) {
            _data.push_back(bytes[j]);
        }
    }
    
    /**
     * Writes a unsigned 32-bit integer to the input buffer.
     *
     * Values will be deserialized on other machines in the same order they were
     * written in.
     *
     * @param i the unsigned Uint32 to write
     */
    void writeUint32(Uint32 i){
        Uint32 ii = marshall(i);
        const std::byte* bytes = reinterpret_cast<const std::byte*>(&ii);
        for (size_t j = 0; j < sizeof(Uint32); j++) {
            _data.push_back(bytes[j]);
        }
    }
    
    /**
     * Writes a unsigned 64-bit integer to the input buffer.
     *
     * Values will be deserialized on other machines in the same order they were
     * written in.
     *
     * @param i the unsigned Uint64 to write
     */
    void writeUint64(Uint64 i){
        Uint64 ii = marshall(i);
        const std::byte* bytes = reinterpret_cast<const std::byte*>(&ii);
        for (size_t j = 0; j < sizeof(Uint64); j++) {
            _data.push_back(bytes[j]);
        }
    }
    
    /**
     * Writes a byte vector to the buffer.
     *
     * Values will be deserialized on other machines in the same order they were
     * written in.
     *
     * @param v The byte vector to write
     */
    void writeByteVector(const std::vector<std::byte>& v){
        _data.insert(_data.end(), v.begin(), v.end());
    }
    
    /**
     * Rewrites the first four bytes of the buffer with the given Uint32.
     *
     * This method requires that the input buffer has at least four bytes.
     * It can be used to add header information once a payload has been
     * constructed.
     *
     * @param i The new header
     */
    void rewriteFirstUint32(Uint32 i) {
        Uint32 ii = marshall(i);
        const std::byte* bytes = reinterpret_cast<const std::byte*>(&ii);
        for (size_t j = 0; j < sizeof(Uint32); j++) {
            _data[j] = bytes[j];
        }
    }
    
    /**
     * Returns the serialized data.
     *
     * @return A const reference to the serialized data. (Will be lost if reset)
     */
    const std::vector<std::byte>& serialize() {
        return _data;
    }
    
    /**
     * Clears the input buffer.
     *
     * Note that this will make previous serialize() returns invalid.
     */
    void reset() {
        _data.clear();
    }
};
        
        }
    }
}

#endif /* __CU_LW_SERIALIZER_H__ */
