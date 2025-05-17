//
//  CUNetEvent.h
//  Cornell University Game Library (CUGL)
//
//  This module provides an object-oriented approach for representing shared
//  data over the network. It is the base class for all events that are sent
//  through the network. Users can use this class to encapsulate serialization
//  and deserialization of data send through the network.
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
#ifndef __CU_NET_EVENT_H__
#define __CU_NET_EVENT_H__

#include <cugl/physics2/distrib/CULWSerializer.h>
#include <cugl/physics2/distrib/CULWDeserializer.h>
#include <SDL_stdinc.h>
#include <vector>
#include <string>
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
 * A template class for all communication messages between machines.
 *
 * Any information that needs to be sent through the network during gameplay
 * should be wrapped in a NetEvent object. Custom events types can be made by
 * subclassing this class and adding parameters as necessary.
 *
 * It is your responsibility to serialize and deserialize your custom classes.
 * However, you should only serialize/deserialize the new attributes that you
 * provide, and not the ones inherited from this class.
 */
class NetEvent {
private:
    //==============================META DATA================================
    // These fields are set by the NetEventController when an event is sent or
    // received. Don't include them in the serialize() and deserialize() methods.
    
    /** The time of the event from the sender. */
    Uint64 _eventTimeStamp;
    /** The time when the event was received by the recipient. */
    Uint64 _receiveTimeStamp;
    /** The ID of the sender. */
    std::string _sourceID;
    
    //==============================META DATA================================
    
    /**
     * Set the to meta data of the event
     *
     * This method is used by the {@link NetEventController} and should not
     * be called by the user.
     *
     * @param eventTimeStamp    the timestamp of the event from the sender
     * @param receiveTimeStamp  the timestamp when the event was received
     * @param sourceID          the ID of the sender
     */
    void setMetaData(Uint64 eventTimeStamp, Uint64 receiveTimeStamp, const std::string sourceID) {
        _eventTimeStamp = eventTimeStamp;
        _receiveTimeStamp = receiveTimeStamp;
        _sourceID = sourceID;
    }
    
    // Allow NetEventController access to the method above.
    friend class NetEventController;
    
public:
    /**
     * Returns a newly allocated event of this type.
     *
     * This method is used by the NetEventController to create a new event with
     * this type as a reference.
     
     * Note that this method is not static, unlike the alloc method present
     * in most of CUGL. That is because we need this factory method to be
     * polymorphic. All custom subclasses must implement this method.
     *
     * @return a newly allocated event of this type.
     */
    virtual std::shared_ptr<NetEvent> newEvent() {
        return std::make_shared<NetEvent>();
    }
    /**
     * Returns a byte vector serializing this event.
     *
     * @return a byte vector serializing this event.
     */
    virtual std::vector<std::byte> serialize() {
        return std::vector<std::byte>();
    }
    /**
     * Deserializes a vector of bytes and set the corresponding parameters.
     *
     * This function should be the "reverse" of the {@link #serialize} function.
     * It should be able to recreate a serialized event entirely, setting all
     * the useful parameters of this class.
     *
     * @param data  a serialized byte vector
     */
    virtual void deserialize(const std::vector<std::byte>& data) { }
    
    /**
     * Returns the timestamp of the event set by the sender.
     *
     * This attribute is valid only if the event was received by this client.
     *
     * @return the timestamp of the event set by the sender.
     */
    Uint64 getEventTimeStamp() const { return _eventTimeStamp; }
    
    /**
     * Returns the timestamp when the event was received by this client.
     *
     * This attribute is valid only if the event was received by this client.
     *
     * @return the timestamp when the event was received by this client.
     */
    Uint64 getReceiveTimeStamp() const { return _receiveTimeStamp; }
    
    /**
     * Returns the ID of the sender.
     *
     * This attribute is valid only if the event was received by this client.
     *
     * @return the ID of the sender.
     */
    const std::string getSourceId() const { return _sourceID; }
};

        }
    }
}
#endif /* __CU_NET_EVENT_H__ */
