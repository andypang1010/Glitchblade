//
//  CUNetWorld.h
//  Cornell University Game Library (CUGL)
//
//  This module is an extension to ObstacleWorld from the physics2 package to
//  enable networked physics. Its primary purpose is id management for pointer
//  swizzling obstacles and joints.
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
//  Author: Walker White, Barry Lyu
//  Version: 7/3/24 (CUGL 3.0 reorganization)
//
#ifndef __CU_NET_WORLD_H__
#define __CU_NET_WORLD_H__

#include <cugl/physics2/CUObstacle.h>
#include <cugl/physics2/CUJoint.h>
#include <cugl/physics2/CUObstacleWorld.h>
#include <unordered_map>

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
 * This class represents a shared, networked physics world.
 *
 * This class is an extension of {@link ObstacleWorld} to support networked
 * physics. Its primary purpose is to assign id numbers to obstacles and joints
 * for the purposes of pointer swizzling. However, it does not explicitly
 * synchronize objects across the network. That is done by other classes.
 */
class NetWorld : public ObstacleWorld {
protected:
    /** UUID of the NetcodeConnection that established this world */
    std::string _uuid;
    /** A shortened version of the identifer for this session */
    Uint32 _shortUID;
    
    /** Map from obstacle pointers to ids (for pointer swizzling) */
    std::unordered_map<std::shared_ptr<physics2::Obstacle>,Uint64>  _obsToId;
    /** Map from ids to obstacle pointers (for pointer swizzling) */
    std::unordered_map<Uint64, std::shared_ptr<physics2::Obstacle>> _idToObs;
    /** A reference counter to the number of obstacle owners */
    std::unordered_map<std::shared_ptr<physics2::Obstacle>,Uint64> _ownedObs;
    /** An iterator to keep track of obstacles for queueing purposes */
    std::unordered_set<std::shared_ptr<physics2::Obstacle>>::iterator _nextObstacle;
    
    /** Map from joint pointers to ids (for pointer swizzling) */
    std::unordered_map<std::shared_ptr<physics2::Joint>,Uint64>  _jntToId;
    /** Map from ids to joint pointers (for pointer swizzling) */
    std::unordered_map<Uint64, std::shared_ptr<physics2::Joint>> _idToJnt;
    /** A reference counter to the number of joint owners */
    std::unordered_map<std::shared_ptr<physics2::Joint>,Uint64> _ownedJoints;

    /** The next available id for initial objects */
    Uint32 _nextInitObj;
    /** The next available id for shared objects */
    Uint32 _nextSharedObj;
    /** The next available id for initial joints */
    Uint32 _nextInitJoint;
    /** The next available id for shared joints */
    Uint32 _nextSharedJoint;
    
    /**
     * Activates this obstacle in the shared physics world
     *
     * This method will activate the underlying physics. The obstacle will now
     * have a body. The physics world will include the obstacle in its next
     * call to update. In addition, the obstacle will be assigned an identifier
     * for the purpose of sharing cross-network.
     *
     * The obstacle will be retained by this world, preventing it from being
     * garbage collected.
     *
     * param oid The obstacle identifier
     * param obj The obstacle to add
     */
    void activateObstacle(Uint64 oid, const std::shared_ptr<Obstacle>& obj);
    
    /**
     * Activates a joint in the shared physics world
     *
     * This method will activate the underlying physics. The will link its
     * associated obstacles. In addition, the physics world will include the
     * joint in its next call to update. Finally, the joint will be assigned
     * an identifier for the purpose of sharing cross-network.
     *
     * The joint will be retained by this world, preventing it from being
     * garbage collected.
     *
     * param jid    The joint identifier
     * param joint  The joint to add
     */
    void activateJoint(Uint64 jid, const std::shared_ptr<Joint>& joint);
    
    // Allow network controller access to these
    friend class NetPhysicsController;
    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates a new degenerate NetWorld on the stack.
     *
     * The scene has no backing Box2d world and must be initialized.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    NetWorld();
    
    /**
     * Deletes this world, disposing all resources
     */
    ~NetWorld() { dispose(); }
    
    /**
     * Disposes all of the resources used by this world.
     *
     * A disposed NetWorld can be safely reinitialized. Any obstacles owned
     * by this world will be deactivated. They will be deleted if no other
     * object owns them.
     */
    void dispose();
    
    /**
     * Initializes a new networked world
     *
     * The specified bounds are in terms of the Box2d world, not the screen.
     * A view attached to this Box2d world should have ways to convert between
     * the coordinate systems.
     *
     * This constructor will use the default gravitational value.
     *
     * @param  bounds   The game bounds in Box2d coordinates
     * @param  uuid     The UUID of the netcode connection that established the world.
     *
     * @return  true if the controller is initialized properly, false otherwise.
     */
    bool initWithUUID(const Rect bounds, std::string uuid);
    
    /**
     * Initializes a new physics world
     *
     * The specified bounds are in terms of the Box2d world, not the screen.
     * A few attached to this Box2d world should have ways to convert between
     * the coordinate systems.
     *
     * @param  bounds   The game bounds in Box2d coordinates
     * @param  gravity  The gravitational force on this Box2d world
     * @param  uuid     The UUID of the netcode connection that established the world.
     *
     * @return  true if the controller is initialized properly, false otherwise.
     */
    bool initWithUUID(const Rect bounds, const Vec2 gravity, std::string uuid);
    
#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a newly allocated physics world
     *
     * The specified bounds are in terms of the Box2d world, not the screen.
     * A view attached to this Box2d world should have ways to convert between
     * the coordinate systems.
     *
     * This constructor will use the default gravitational value. In addition,
     * it will assign the physics world a fresh UUID.
     *
     * @param  bounds   The game bounds in Box2d coordinates
     *
     * @return a newly allocated physics world
     */
    static std::shared_ptr<NetWorld> alloc(const Rect bounds) {
        std::shared_ptr<NetWorld> result = std::make_shared<NetWorld>();
        return (result->init(bounds) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated physics world
     *
     * The specified bounds are in terms of the Box2d world, not the screen.
     * A view attached to this Box2d world should have ways to convert between
     * the coordinate systems.
     *
     * This constructor will assign the physics world a fresh UUID.
     *
     * @param  bounds   The game bounds in Box2d coordinates
     * @param  gravity  The gravitational force on this Box2d world
     *
     * @return a newly allocated physics world
     */
    static std::shared_ptr<NetWorld> alloc(const Rect bounds, const Vec2 gravity) {
        std::shared_ptr<NetWorld> result = std::make_shared<NetWorld>();
        return (result->init(bounds,gravity) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated physics world
     *
     * The specified bounds are in terms of the Box2d world, not the screen.
     * A view attached to this Box2d world should have ways to convert between
     * the coordinate systems.
     *
     * This constructor will use the default gravitational value. It will also
     * assign the world a fresh UUID.
     *
     * @param  bounds   The game bounds in Box2d coordinates
     * @param  UUID     The UUID of the netcode connection that established the world.
     *
     * @return a newly allocated physics world
     */
    static std::shared_ptr<NetWorld> allocWithUUID(const Rect bounds, std::string UUID) {
        std::shared_ptr<NetWorld> result = std::make_shared<NetWorld>();
        return (result->initWithUUID(bounds,UUID) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated physics world
     *
     * The specified bounds are in terms of the Box2d world, not the screen.
     * A few attached to this Box2d world should have ways to convert between
     * the coordinate systems.
     *
     * @param  bounds   The game bounds in Box2d coordinates
     * @param  gravity  The gravitational force on this Box2d world
     * @param  UUID     The UUID of the netcode connection that established the world.
     *
     * @return a newly allocated physics world
     */
    static std::shared_ptr<NetWorld> allocWithUUID(const Rect bounds, const Vec2 gravity,
                                                   std::string UUID) {
        std::shared_ptr<NetWorld> result = std::make_shared<NetWorld>();
        return (result->initWithUUID(bounds,gravity) ? result : nullptr);
    }

#pragma mark -
#pragma mark Network Attributes
    /**
     * Returns the UUID of the netcode connection that created this world.
     *
     * @return the UUID of the netcode connection that created this world.
     */
    std::string getUUID() {
        return _uuid;
    }

    /**
     * Returns the short id for this network session.
     *
     * @return the short id for this network session.
     */
    Uint32 getshortUID() {
        return _shortUID;
    }

    /**
     * Sets the short id for this network session.
     *
     * @param sid   the short id for this network session.
     */
    void setshortUID(Uint32 sid) {
        _shortUID = sid;
    }

#pragma mark -
#pragma mark Object Management
    /**
     * Adds an initial obstacle to the physics world.
     *
     * This method is for obstacles that are created at the start of the
     * simulation, before any networking is underway.
     *
     * Adding an obstacle activates the underlying physics. It will now have
     * a body. The physics world will include the obstacle in its next call
     * to update.
     *
     * @param obj The obstacle to add
     *
     * @return the obstacle id
     */
    Uint64 initObstacle(const std::shared_ptr<Obstacle>& obj);

    /**
     * Adds an obstacle to the physics world.
     *
     * This method is for obstacles that are created while the simulation
     * is ongoing.
     *
     * Adding an obstacle activates the underlying physics. It will now have
     * a body. The physics world will include the obstacle in its next call
     * to update.
     *
     * @param obj The obstacle to add
     *
     * @return the obstacle id
     */
    Uint64 placeObstacle(const std::shared_ptr<Obstacle>& obj);
    
    /**
     * Immediately adds the obstacle to the physics world
     *
     * Adding an obstacle activates the underlying physics. It will now have
     * a body. The physics world will include the obstacle in its next call
     * to update.
     *
     * The obstacle will be retained by this world, preventing it from being
     * garbage collected. This obstacle will also be assigned an object id,
     * using the rules of {@link #placeObstacle}. You can use the method
     * {@link #getObstacleId} to find this id.
     *
     * @param obj The obstacle to add
     */
    void addObstacle(const std::shared_ptr<Obstacle>& obj) override;

    /**
     * Immediately removes an obstacle from the physics world
     *
     * The obstacle will be released immediately. The physics will be
     * deactivated and it will be removed from the Box2D world. This method of
     * removing obstacles is very heavy weight, and should only be used for
     * single object removal. If you want to remove multiple obstacles, then
     * you should mark them for removal and call {@link #garbageCollect}.
     *
     * Removing an obstacle does not automatically delete the obstacle itself.
     * However, this world releases ownership, which may lead to it being
     * garbage collected.
     *
     * @param obj The obstacle to remove
     */
    void removeObstacle(const std::shared_ptr<Obstacle>& obj) override;
    
    /**
     * Adds an initial joint to the physics world.
     *
     * This method is for joints that are created at the start of the
     * simulation, before any networking is underway.
     *
     * Adding an joints activates the underlying physics. It will now connect
     * its two obstacles. The physics world will include the joint in its next
     * call to update.
     *
     * @param joint The joint to add
     *
     * @return the joint id
     */
    Uint64 initJoint(const std::shared_ptr<Joint>& joint);

    /**
     * Adds a joint to the physics world.
     *
     * This method is for joints that are created while the simulation
     * is ongoing.
     *
     * Adding an joints activates the underlying physics. It will now connect
     * its two obstacles. The physics world will include the joint in its next
     * call to update.
     *
     * @param joint The joint to add
     *
     * @return the joint id
     */
    Uint64 placeJoint(const std::shared_ptr<Joint>& joint);
    
    /**
     * Immediately adds a joint to the physics world
     *
     * This method will fail if the joint obstacles are not in this world.
     * The joint will be activated so that it contains those two obstacles.
     * The physics world will include the joint in its next call to update.
     *
     * The joint will be retained by this world, preventing it from being
     * garbage collected. This obstacle will also be assigned an object id,
     * using the rules of {@link #placeJoint}. You can use the method
     * {@link #getJointId} to find this id.
     *
     * @param joint  The joint to add
     */
    virtual void addJoint(const std::shared_ptr<Joint>& joint) override;
    
    /**
     * Immediately removes a joint from the physics world
     *
     * The joint will be released immediately. The physics will be
     * deactivated and it will be removed from the Box2D world. Note that only
     * the joint is removed.  The bodies attached to the joint will still be
     * present.
     *
     * This method of removing joints is very heavy weight, and should only be
     * used for single joint removal. If you want to remove multiple joints,
     * then you should mark them for removal and call {@link #garbageCollect}.
     *
     * Removing a joint does not automatically delete the joint itself.
     * However, this world releases ownership, which may lead to it being
     * garbage collected.
     *
     * @param joint  The joint to remove
     */
    virtual void removeJoint(const std::shared_ptr<Joint>& joint) override;

#pragma mark -
#pragma mark Id Management
    /**
     * Returns the next obstacle for synchronization
     *
     * This goes around the obstacle set in a round-robin fashion.
     *
     * @return the next obstacle for synchronization
     */
    std::shared_ptr<Obstacle> getNextObstacle();
    
    /**
     * Returns the obstacle for the given id.
     *
     * This method returns nullptr if there is no such joint.
     *
     * @param oid   The obstacle id
     *
     * @return the obstacle for the given id.
     */
    std::shared_ptr<Obstacle> getObstacle(Uint64 oid) {
        return _idToObs.at(oid);
    }
    
    /**
     * Returns id for the given obstacle.
     *
     * This method returns -1 if there is no such obstacle.
     *
     * @param obs   The obstacle to query
     *
     * @return id for the given obstacle.
     */
    Sint64 getObstacleId(const std::shared_ptr<Obstacle>& obs) {
        return _obsToId.at(obs);
    }
    
    /**
     * Returns the joint for the given id.
     *
     * This method returns nullptr if there is no such joint.
     *
     * @param jid   The joint id
     *
     * @return the joint for the given id.
     */
    std::shared_ptr<Joint> getJoint(Uint64 jid) {
        return _idToJnt.at(jid);
    }
    
    /**
     * Returns id for the given joint.
     *
     * This method returns -1 if there is no such joint.
     *
     * @param joint The joint to query
     *
     * @return id for the given joint.
     */
    Sint64 getJointId(const std::shared_ptr<Joint>& joint) {
        return _jntToId.at(joint);
    }
    
    /**
     * Returns the map from obstacle ids to the objects
     *
     * @return the map from obstacle ids to the objects
     */
    const std::unordered_map<Uint64, std::shared_ptr<physics2::Obstacle>>& getObstacleMap() {
        return _idToObs;
    }

    /**
     * Returns the map from obstacles to their ids
     *
     * @return the map from obstacles to their ids
     */
    const std::unordered_map<std::shared_ptr<physics2::Obstacle>,Uint64>& getObstacleIds() {
        return _obsToId;
    }
    
    /**
     * Returns the map of obstacles owned by this shared physics world.
     *
     * The keys are the obstacle pointers, while the values are the ownership
     * duration. If the value is 0, then this obstacle is permanently owned by
     * this copy of the world.
     *
     * @return the map of obstacles owned by this shared physics world.
     */
    std::unordered_map<std::shared_ptr<physics2::Obstacle>,Uint64>& getOwnedObstacles() {
        return _ownedObs;
    }
    
    /**
     * Returns the map from joint ids to the objects
     *
     * @return the map from joint ids to the objects
     */
    const std::unordered_map<Uint64, std::shared_ptr<physics2::Joint>>& getJointMap() {
        return _idToJnt;
    }

    /**
     * Returns the map from joints to their ids
     *
     * @return the map from joints to their ids
     */
    const std::unordered_map<std::shared_ptr<physics2::Joint>,Uint64>& getJointIds() {
        return _jntToId;
    }
    
    /**
     * Returns the map of joints owned by this shared physics world.
     *
     * The keys are the joints pointers, while the values are the ownership
     * duration. If the value is 0, then this joint is permanently owned by
     * this copy of the world.
     *
     * @return the map of joints owned by this shared physics world.
     */
    std::unordered_map<std::shared_ptr<physics2::Joint>,Uint64>& getOwnedJoints() {
        return _ownedJoints;
    }
    
#pragma mark -
#pragma mark Destruction Callback Functions
    /**
     * Called when a joint is about to be destroyed.
     *
     * This function is only called when the destruction is the result of the
     * destruction of one of its attached bodies.
     *
     * @param  joint    the joint to be destroyed
     */
    void SayGoodbye(b2Joint* joint) override;
    
};
        }
    }
}
#endif /* __CU_NET_WORLD_H__ */
