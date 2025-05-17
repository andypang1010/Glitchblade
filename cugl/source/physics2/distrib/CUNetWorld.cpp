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
#include <cugl/physics2/distrib/CUNetWorld.h>
#include <cugl/core/util/CUHashtools.h>
#include <algorithm>
#include <random>

using namespace cugl;
using namespace cugl::physics2;
using namespace cugl::physics2::distrib;

#pragma mark -
#pragma mark Constructors
/**
 * Creates a new degenerate NetWorld on the stack.
 *
 * The scene has no backing Box2d world and must be initialized.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
 * the heap, use one of the static constructors instead.
 */
NetWorld::NetWorld() : ObstacleWorld(),
_nextInitObj(0),
_nextSharedObj(0),
_nextInitJoint(0),
_nextSharedJoint(0) {
    _uuid = hashtool::generate_uuid();
    std::hash<std::string> hasher;
    _shortUID = (Uint32)hasher(_uuid);
}

/**
 * Disposes all of the resources used by this world.
 *
 * A disposed NetWorld can be safely reinitialized. Any obstacles owned
 * by this world will be deactivated. They will be deleted if no other
 * object owns them.
 */
void NetWorld::dispose() {
    _obsToId.clear();
    _idToObs.clear();
    _ownedObs.clear();
    _jntToId.clear();
    _idToJnt.clear();
    _ownedJoints.clear();
    _nextInitObj = 0;
    _nextSharedObj = 0;
    _nextInitJoint = 0;
    _nextSharedJoint = 0;
    ObstacleWorld::dispose();
}

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
bool NetWorld::initWithUUID(const Rect bounds, std::string uuid) {
    if (init(bounds)) {
        _uuid = uuid;
        std::hash<std::string> hasher;
        _shortUID = (Uint32)hasher(_uuid);
        return true;
    }
    return false;
}

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
bool NetWorld::initWithUUID(const Rect bounds, const Vec2 gravity, std::string uuid) {
    if (init(bounds,gravity)) {
        _uuid = uuid;
        std::hash<std::string> hasher;
        _shortUID = (Uint32)hasher(_uuid);
        return true;
    }
    return false;
}

#pragma mark -
#pragma mark Object Management
/**
 * Returns the next obstacle for synchronization
 *
 * This goes around the obstacle set in a round-robin fashion.
 *
 * @return the next obstacle for synchronization
 */
std::shared_ptr<Obstacle> NetWorld::getNextObstacle() {
    if (_nextObstacle == _obstacles.end()) {
        return nullptr;
    }
    std::shared_ptr<Obstacle> obs = *_nextObstacle;
    _nextObstacle++;
    return obs;
}


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
void NetWorld::activateObstacle(Uint64 oid, const std::shared_ptr<Obstacle>& obj) {
    CUAssertLog(inBounds(obj.get()), "Obstacle is not in bounds");
    CUAssertLog(!_idToObs.count(oid), "Duplicate obstacle ids are not allowed");
    _obstacles.emplace(obj);
    obj->activatePhysics(*_world);
    _idToObs[oid] = obj;
    _obsToId[obj] = oid;
    _nextObstacle = _obstacles.find(obj);
}

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
Uint64 NetWorld::initObstacle(const std::shared_ptr<Obstacle>& obj) {
    Uint64 oid = (((Uint64)0xffffffff) << 32) | _nextInitObj++;
    obj->setShared(true);
    activateObstacle(oid, obj);
    return oid;
}

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
Uint64 NetWorld::placeObstacle(const std::shared_ptr<Obstacle>& obj) {
    Uint64 oid = (((Uint64)_shortUID) << 32) | _nextSharedObj++;
    activateObstacle(oid, obj);
    return oid;
}

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
void NetWorld::addObstacle(const std::shared_ptr<Obstacle>& obj) {
    Uint64 oid = (((Uint64)_shortUID) << 32) | _nextSharedObj++;
    activateObstacle(oid, obj);
}

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
void NetWorld::removeObstacle(const std::shared_ptr<Obstacle>& obj) {
    auto pos = _obsToId.find(obj);
    if (pos != _obsToId.end()) {
        Uint64 oid = pos->second;
        _obsToId.erase(pos);
        _idToObs.erase(oid);
        _ownedObs.erase(obj);
        _nextObstacle = _obstacles.begin();
        ObstacleWorld::removeObstacle(obj);
    }
}

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
void NetWorld::activateJoint(Uint64 jid, const std::shared_ptr<Joint>& joint) {
    // Look for obstacles
    auto it = _obstacles.find(joint->getObstacleA());
    CUAssertLog(it != _obstacles.end(), "Obstacle A not found in physics world");
    auto jt = _obstacles.find(joint->getObstacleB());
    CUAssertLog(jt != _obstacles.end(), "Obstacle B not found in physics world");
    CUAssertLog(!_idToJnt.count(jid), "Duplicate joint ids are not allowed");

    joint->activatePhysics(*_world);
    _joints[joint->getJoint()] = joint;
    _idToJnt[jid] = joint;
    _jntToId[joint] = jid;
}

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
Uint64 NetWorld::initJoint(const std::shared_ptr<Joint>& joint) {
    Uint64 jid = (((Uint64)0xffffffff) << 32) | _nextInitJoint++;
    activateJoint(jid, joint);
    return jid;
}


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
Uint64 NetWorld::placeJoint(const std::shared_ptr<Joint>& joint) {
    Uint64 jid = (((Uint64)_shortUID) << 32) | _nextSharedJoint++;
    activateJoint(jid, joint);
    return jid;
}

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
void NetWorld::addJoint(const std::shared_ptr<Joint>& joint) {
    Uint64 jid = (((Uint64)_shortUID) << 32) | _nextSharedJoint++;
    activateJoint(jid, joint);
}

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
void NetWorld::removeJoint(const std::shared_ptr<Joint>& joint) {
    auto pos = _jntToId.find(joint);
    if (pos != _jntToId.end()) {
        Uint64 jid = pos->second;
        _jntToId.erase(pos);
        _idToJnt.erase(jid);
        _ownedJoints.erase(joint);
        ObstacleWorld::removeJoint(joint);
    }
}

#pragma mark Destruction Callback Functions
/**
 * Called when a joint is about to be destroyed.
 *
 * This function is only called when the destruction is the result of the
 * destruction of one of its attached bodies.
 *
 * @param  joint    the joint to be destroyed
 */
void NetWorld::SayGoodbye(b2Joint* joint) {
    auto jt = _joints.find(joint);
    if (jt != _joints.end()) {
        std::shared_ptr<Joint> jobj = jt->second;
        Uint64 jid = _jntToId.at(jobj);
        jobj->release();
        jt = _joints.erase(jt);
        _jntToId.erase(jobj);
        _idToJnt.erase(jid);
        _ownedJoints.erase(jobj);
    }
    if (destroyJoint != nullptr) {
        destroyJoint(joint);
    }
}
