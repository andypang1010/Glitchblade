//
//  CULWDeserializer.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a lightweight deserializer for networked physics. It
//  removes the type safety of the Netcode Deserializer class, and relies on
//  the user to know the type of the data. However, it is a more space efficient
//  serializer, and is more appropriate for networked physics.
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
#ifndef __CU_LW_DESERIALIZER_H__
#define __CU_LW_DESERIALIZER_H__

#include <vector>
#include <SDL_stdinc.h>

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
 * A lightweight deserializer for networked physics.
 *
 * This class removes the type safety of {@link cugl::netcode::NetcodeDeserializer},
 * and requires that the user know the type of the data. However, it is a more
 * space efficient serializer, and is more appropriate for networked physics.
 *
 * This class is to be paired with {@link LWSerializer} for serialization.
 */
class LWDeserializer{
private:
    /** Currently loaded data */
    std::vector<std::byte> _data;
    /** Position in the data of next byte to read */
    size_t _pos;
    
public:
    /**
     * Creates a new Deserializer on the stack.
     *
     * Deserializers do not have any nontrivial state and so it is unnecessary
     * to use an init method. However, we do include a static {@link #alloc}
     * method for creating shared pointers.
     */
    LWDeserializer() : _pos(0) {}
    
    /**
     * Returns a newly allocated LWDeserializer.
     *
     * This method is solely include for convenience purposes.
     *
     * @return a newly allocated LWDeserializer.
     */
    static std::shared_ptr<LWDeserializer> alloc() {
        return std::make_shared<LWDeserializer>();
    }
    
    /**
     * Loads a new message to be read.
     *
     * Calling this method will discard any previously loaded messages. The
     * message must be serialized by {@link LWSerializer}. Otherwise, the
     * results are unspecified.
     *
     * Once loaded, call the various read methods to get the data. It is
     * up to the user to know the correct methods to be called. The values
     * are guaranteed to be delievered in the same order they were written.
     *
     * @param msg The byte vector serialized by {@link LWSerializer}
     */
    void receive(const std::vector<std::byte>& msg){
        _data = msg;
        _pos = 0;
    }
    
    /**
     * Returns a boolean read from the loaded byte vector.
     *
     * The method advances the read position. If called when no more data is
     * available, this method will return false.
     *
     * @return a boolean read from the loaded byte vector.
     */
    bool readBool(){
        if (_pos >= _data.size()) {
            return false;
        }
        uint8_t value = static_cast<uint8_t>(_data[_pos++]);
        return value == 1;
    }
    
    /**
     * Returns a byte from the loaded byte vector.
     *
     * The method advances the read position. If called when no more data is
     * available, this method will return 0.
     *
     * @return a byte from the loaded byte vector.
     */
    std::byte readByte(){
        if (_pos >= _data.size()) {
            return std::byte(0);
        }
        const std::byte b = _data[_pos++];
        return b;
    }
    
    /**
     * Returns a float from the loaded byte vector.
     *
     * The method advances the read position. If called when no more data is
     * available, this method will return 0.
     *
     * @return a float from the loaded byte vector.
     */
    float readFloat(){
        if (_pos >= _data.size()) {
            return 0.0f;
        }
        const float* r = reinterpret_cast<const float*>(_data.data() + _pos);
        _pos += sizeof(float);
        return marshall(*r);
    }
    
    /**
     * Returns a signed (32 bit) int from the loaded byte vector.
     *
     * The method advances the read position. If called when no more data is
     * available, this method will return 0.
     *
     * @return a signed (32 bit) int from the loaded byte vector.
     */
    Sint32 readSint32(){
        if (_pos >= _data.size()) {
            return 0;
        }
        const Sint32* r = reinterpret_cast<const Sint32*>(_data.data() + _pos);
        _pos += sizeof(Sint32);
        return marshall(*r);
    }
    
    /**
     * Returns an unsigned short from the loaded byte vector.
     *
     * The method advances the read position. If called when no more data is
     * available, this method will return 0.
     *
     * @return an unsigned short from the loaded byte vector.
     */
    Uint16 readUint16(){
        if (_pos >= _data.size()) {
            return 0;
        }
        const Uint16* r = reinterpret_cast<const Uint16*>(_data.data() + _pos);
        _pos += sizeof(Uint16);
        return marshall(*r);
    }
    
    /**
     * Returns an unsigned (32 bit) int from the loaded byte vector.
     *
     * The method advances the read position. If called when no more data is
     * available, this method will return 0.
     *
     * @return an unsigned (32 bit) int from the loaded byte vector.
     */
    Uint32 readUint32(){
        if (_pos >= _data.size()) {
            return 0;
        }
        const Uint32* r = reinterpret_cast<const Uint32*>(_data.data() + _pos);
        _pos += sizeof(Uint32);
        return marshall(*r);
    }
    
    /**
     * Returns an unsigned (64 bit) long from the loaded byte vector.
     *
     * The method advances the read position. If called when no more data is
     * available, this method will return 0.
     *
     * @return an unsigned (64 bit) long from the loaded byte vector.
     */
    Uint64 readUint64(){
        if (_pos >= _data.size()) {
            return 0;
        }
        const Uint64* r = reinterpret_cast<const Uint64*>(_data.data() + _pos);
        _pos += sizeof(Uint64);
        return marshall(*r);
    }
    
    /**
     * Resets the deserializer and clears the loaded byte vector.
     */
    void reset(){
        _pos = 0;
        _data.clear();
    }
    
};

        }
    }
}
#endif /* __CU_LW_DESERIALIZER_H__ */
