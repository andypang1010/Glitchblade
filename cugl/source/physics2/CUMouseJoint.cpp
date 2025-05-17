//
//  CUMouseJoint.cpp
//  Cornell University Game Library (CUGL)
//
//  This module is a CUGL wrapper about b2_mouse_joint, implemented to make
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
#include <cugl/physics2/CUMouseJoint.h>
#include <box2d/b2_mouse_joint.h>

using namespace cugl;
using namespace cugl::physics2;

/**
 * Creates a new mouse joint with no obstacles
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
 * the heap, use one of the static constructors instead (in this case, in
 * one of the subclasses).
 */
MouseJoint::MouseJoint() : Joint(),
_maxForce(0.0f),
_stiffness(0.0f),
_damping(0.0f) {
    _type = e_mouseJoint;
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
bool MouseJoint::activatePhysics(b2World& world) {
    if (_joint != nullptr || _bodyA == nullptr || _bodyB == nullptr) {
        return false;
    }
    
    if (_bodyA->getBody() == nullptr) {
        _bodyA->activatePhysics(world);
    }
    if (_bodyB->getBody() == nullptr) {
        _bodyB->activatePhysics(world);
    }
    
    b2MouseJointDef def;
    def.bodyA = _bodyA->getBody();
    def.bodyB = _bodyB->getBody();
    def.maxForce = _maxForce;
    def.stiffness = _stiffness;
    def.damping = _damping;
    def.target.Set(_target.x, _target.y);
    def.collideConnected = _collideConnected;
    def.userData.pointer = reinterpret_cast<intptr_t>(this);
    _joint = world.CreateJoint(&def);
    
    _dirty = false;
    return _joint != nullptr;
}

