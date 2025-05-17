//
//  CUPhysSyncEvent.h
//  Cornell University Game Library (CUGL)
//
//  This module provides events for physics synchronization, which are handled
//  by the NetEventController internally.
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
#ifndef __CU_PHYS_SYNC_EVENT_H__
#define __CU_PHYS_SYNC_EVENT_H__

#include <cugl/netcode/CUNetcodeSerializer.h>
#include <cugl/physics2/CUObstacle.h>
#include <cugl/physics2/distrib/CUNetEvent.h>
#include <SDL_stdinc.h>
#include <unordered_set>

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
 * This class represents a message to synchronize obstacle positions.
 *
 * This class should only be used internally by the networked physics library.
 * It is not designed to synchronize customs state. For that, you should use
 * {@link GameStateEvent} instead.
 */
class PhysSyncEvent : public NetEvent {
public:
    /**
     * The type for the data in an object snapshot.
     *
     * This contains the obstacles's global Id, position, and velocity.
     */
    class Parameters {
    public:
        /** The obstacle id */
        Uint64 obsId;
        /** The x-coordinate of the position */
        float x;
        /** The y-coordinate of the position */
        float y;
        /** The x-coordinate of the velocity */
        float vx;
        /** The y-coordinate of the velocity */
        float vy;
        /** The obstacle angle */
        float angle;
        /** The angular velocity */
        float vAngular;
        
        /** Creates a new parameter set with default values */
        Parameters();
    };

protected:
    /** The vector of added object snapshots. */
    std::vector<Parameters> _syncList;

private:
    /** The set of ids of all obstacles added to be serialized. */
    std::unordered_set<Uint64> _obsSet;
    // TODO: Is there a reason we are not using lightweights?
    /** The serializer for converting basic types to byte vectors. */
    cugl::netcode::NetcodeSerializer _serializer;
    /** The deserializer for converting byte vectors to basic types. */
    cugl::netcode::NetcodeDeserializer _deserializer;
    
#pragma mark Constructors
public:
    /**
     * Returns a newly allocated event of this type.
     *
     * This is a static version of {@link #newEvent}.
     *
     * @return a newly allocated event of this type.
     */
    static std::shared_ptr<PhysSyncEvent> alloc() {
        return std::make_shared<PhysSyncEvent>();
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
        return std::make_shared<PhysSyncEvent>();
    }
    

#pragma mark Serialization/Deserialization
    /**
     * Returns a reference to the obstacle snapshots added so far.
     *
     * @return a reference to the obstacle snapshots added so far.
     */
    const std::vector<Parameters>& getSyncList() const {
        return _syncList;
    }

    /**
     * Snapshots an obstacle's current position and velocity.
     *
     * This snapshot is then added to the list for serialization.
     *
     * @param obs   the obstacle reference to add
     * @param id    the global id of the obstacle
     */
    void addObstacle(Uint64 id, const std::shared_ptr<physics2::Obstacle>& obs);
    
    /**
     * Returns a byte vector serializing the current list of snapshots.
     *
     * @return a byte vector serializing the current list of snapshots.
     */
    std::vector<std::byte> serialize() override;
    
    /**
     * Unpacks a byte vector into a list of snapshots.
     *
     * These snapshots can then be used in physics synchronizations.
     *
     * @param data the byte vector to deserialize
     */
    void deserialize(const std::vector<std::byte>& data) override;
    
};

        }
    }
}

#endif /* __CU_PHYS_SYNC_EVENT_H__ */
