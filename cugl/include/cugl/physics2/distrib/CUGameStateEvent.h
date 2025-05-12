//
//  CUGameStateEvent.h
//  Cornell University Game Library (CUGL)
//
//  This module provides an event for a game state change. It is handled by
//  the NetEventController internally.
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
#ifndef __CU_GAME_STATE_EVENT_H__
#define __CU_GAME_STATE_EVENT_H__

#include <cugl/physics2/distrib/CUNetEvent.h>
#include <cugl/core/util/CUDebug.h>

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
 * This class represents a game state change during a session.
 *
 * This class allows the user to extend the networked physics library to notify
 * changes in game state, such as starting the game, reseting it, or pausing it.
 */
class GameStateEvent : public NetEvent {
public:
    /** Enum for the type of the event. */
    enum class EventType : int  {
        /** Assigning a short id to a simulation */
        UID_ASSIGN  = 100,
        /** Notifying that the client is read */
        CLIENT_RDY = 101,
        /** Starting the game */
        GAME_START = 102,
        /** Reseting the game */
        GAME_RESET = 103, // Not used
        /** Pausing the game */
        GAME_PAUSE = 104, // Not used
        /** Resuming the game */
        GAME_RESUME = 105 // Not used
    };
    
protected:
    /** An internal type of the game state message */
    EventType _type;
    /** The shortUID of the associated physics world */
    Uint32 _shortUID;
    
#pragma mark Constructors
public:
    /**
     *  Constructs an event with default values.
     */
    GameStateEvent() : _shortUID(0) {
        _type = EventType::GAME_START;
    }
    
    /**
     *  Constructs an event with the given type.
     *
     *  @param t The type of the event
     */
    GameStateEvent(EventType t) : _shortUID(0) {
        _type = t;
    }

    
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
    std::shared_ptr<NetEvent> newEvent() override {
        return std::make_shared<GameStateEvent>();
    }
    
    /**
     * Returns a newly allocated event of this type.
     *
     * This is a static version of {@link #newEvent}.
     *
     * @return a newly allocated event of this type.
     */
    static std::shared_ptr<GameStateEvent> alloc() {
        return std::make_shared<GameStateEvent>();
    }

    /**
     * Returns a newly allocated event for broadcasting the game start
     *
     * @return a newly allocated event for broadcasting the game start
     */
    static std::shared_ptr<NetEvent> allocGameStart() {
        std::shared_ptr<GameStateEvent> ptr = std::make_shared<GameStateEvent>();
        ptr->setType(EventType::GAME_START);
        return ptr;
    }
    
    /**
     * Returns a newly allocated event for marking the client as ready
     *
     * @return a newly allocated event for marking the client as ready
     */
    static std::shared_ptr<NetEvent> allocReady() {
        std::shared_ptr<GameStateEvent> ptr = std::make_shared<GameStateEvent>();
        ptr->setType(EventType::CLIENT_RDY);
        return ptr;
    }
    
    /**
     * Returns a newly allocated event for assigning ids for clients
     *
     * This event is sent from the host to one client only. It is not meant
     * to be broadcasted.
     *
     * @param sid   The short id to assign
     */
    static std::shared_ptr<NetEvent> allocUIDAssign(Uint32 sid) {
        std::shared_ptr<GameStateEvent> ptr = std::make_shared<GameStateEvent>();
        ptr->setType(EventType::UID_ASSIGN);
        ptr->_shortUID = sid;
        return ptr;
    }
    
#pragma mark Event Attributes
    /**
     * Returns the event type
     *
     * @return the event type
     */
    EventType getType() const {
        return _type;
    }

    /**
     * Sets the event type
     *
     * @param t The type of the event
     */
    void setType(EventType t) {
        _type = t;
    }
    
    /**
     * Returns the shortUID of the event
     *
     * If the event is not {@link EventType#UID_ASSIGN}, this method returns 0.
     * Valid shortUIDs are guaranteed to be greater than 0.
     *
     * @return the shortUID of the event
     */
    Uint8 getShortUID() const {
        return _shortUID;
    }
    
#pragma mark Serialization/Deserialization 
    /**
     * Returns a byte vector serializing this event
     *
     * @return a byte vector serializing this event
     */
    std::vector<std::byte> serialize() override;
    
    /**
     * Deserializes this event from a byte vector.
     *
     * This method will set the type of the event and all relevant fields.
     */
    void deserialize(const std::vector<std::byte>& data) override;
    
};
        }
    }
}

#endif /* __CU_GAME_STATE_EVENT_H__ */
