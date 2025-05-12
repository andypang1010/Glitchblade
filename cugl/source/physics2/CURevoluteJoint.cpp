//
//  CURevoluteJoint.cpp
//  Cornell University Game Library (CUGL)
//
//  This module is a CUGL wrapper about b2_revolute_joint, implemented to make
//  networked physics a little simpler.
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
//  This file is based on the CS 3152 PhysicsDemo Lab by Don Holden, 2007
//
//  Author: Walker White
//  Version: 7/3/24 (CUGL 3.0 reorganization)
//
#include <cugl/physics2/CUObstacle.h>
#include <cugl/physics2/CURevoluteJoint.h>
#include <box2d/b2_revolute_joint.h>

using namespace cugl;
using namespace cugl::physics2;

/**
 * Creates a new revolute joint with no obstacles
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
 * the heap, use one of the static constructors instead (in this case, in
 * one of the subclasses).
 */
RevoluteJoint::RevoluteJoint() : Joint(),
_referenceAngle(0),
_enableLimit(false),
_enableMotor(false),
_lowerAngle(0),
_upperAngle(0),
_maxMotorTorque(0),
_motorSpeed(0) {
    _type = e_revoluteJoint;
}

/**
 * Initializes a new revolute joint with the given obstacles.
 *
 * All other attributes will be at their default values.
 *
 * @param obsA  The first obstacle to join
 * @param obsB  The second obstacle to join
 *
 * @return true if the obstacle is initialized properly, false otherwise.
 */
bool RevoluteJoint::initWithObstacles(const std::shared_ptr<Obstacle>& obsA,
                                      const std::shared_ptr<Obstacle>& obsB) {
    _bodyA = obsA;
    _bodyB = obsB;
    return true;
}

/**
 * Initializes a new revolute joint with the given obstacles and anchors
 *
 * All other attributes will be at their default values.
 *
 * @param obsA      The first obstacle to join
 * @param obsB      The second obstacle to join
 * @param localA    The local anchor of the first obstacle
 * @param localB    The local anchor of the second obstacle
 *
 * @return true if the obstacle is initialized properly, false otherwise.
 */
bool RevoluteJoint::initWithObstacles(const std::shared_ptr<Obstacle>& obsA,
                                      const std::shared_ptr<Obstacle>& obsB,
                                      const Vec2 localA, const Vec2 localB) {
    _bodyA = obsA;
    _bodyB = obsB;
    _localAnchorA = localA;
    _localAnchorB = localB;
    return true;
}

/**
 * Creates the Box2d joint, adding it to the world.
 *
 * Calling this method activates the physics of the associated obstacles,
 * if necessary.
 *
 * Implementations of this method should NOT retain ownership of the
 * Box2D world. That is a tight coupling that we should avoid.
 *
 * @param world Box2D world to store the joint
 *
 * @return true if object allocation succeeded
 */
bool RevoluteJoint::activatePhysics(b2World& world) {
    if (_joint != nullptr || _bodyA == nullptr || _bodyB == nullptr) {
        return false;
    }
    
    if (_bodyA->getBody() == nullptr) {
        _bodyA->activatePhysics(world);
    }
    if (_bodyB->getBody() == nullptr) {
        _bodyB->activatePhysics(world);
    }

    b2RevoluteJointDef def;
    def.bodyA = _bodyA->getBody();
    def.bodyB = _bodyB->getBody();
    def.referenceAngle = _referenceAngle;
    def.enableLimit = _enableLimit;
    def.enableMotor = _enableMotor;
    def.lowerAngle = _lowerAngle;
    def.upperAngle = _upperAngle;
    def.maxMotorTorque = _maxMotorTorque;
    def.motorSpeed = _motorSpeed;
    def.localAnchorA.Set(_localAnchorA.x, _localAnchorA.y);
    def.localAnchorB.Set(_localAnchorB.x, _localAnchorB.y);
    def.collideConnected = _collideConnected;
    def.userData.pointer = reinterpret_cast<intptr_t>(this);
    _joint = world.CreateJoint(&def);

    _dirty = false;
    return _joint != nullptr;
}
