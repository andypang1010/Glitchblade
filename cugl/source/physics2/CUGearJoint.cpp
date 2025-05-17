//
//  CUGearJoint.cpp
//  Cornell University Game Library (CUGL)
//
//  This module is a CUGL wrapper about b2_gear_joint, implemented to make
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
#include <cugl/physics2/CUGearJoint.h>
#include <box2d/b2_gear_joint.h>

using namespace cugl;
using namespace cugl::physics2;

/**
 * Creates a new gear joint with no joints
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
 * the heap, use one of the static constructors instead (in this case, in
 * one of the subclasses).
 */
GearJoint::GearJoint() : Joint(),
_ratio(1.0f) {
    _type = e_gearJoint;
}

/**
 * Initializes a new gear joint with the given prismatic/revolute joints.
 *
 * All other attributes will be at their default values.
 *
 * @param joint1    The first prismatic/revolute joint to use
 * @param joint2    The second prismatic/revolute joint to use
 *
 * @return true if the obstacle is initialized properly, false otherwise.
 */
bool GearJoint::initWithJoints(const std::shared_ptr<Joint>& joint1,
                               const std::shared_ptr<Joint>& joint2) {
    b2JointType type1 = joint1 == nullptr ? e_revoluteJoint : joint1->getType();
    b2JointType type2 = joint2 == nullptr ? e_revoluteJoint : joint2->getType();
    if (type1 != e_revoluteJoint && type1 != e_prismaticJoint) {
        CUAssertLog(false,"First joint has invalid type");
        return false;
    } else if (type2 != e_revoluteJoint && type2 != e_prismaticJoint) {
        CUAssertLog(false,"Second joint has invalid type");
        return false;
    }
    _joint1 = joint1;
    _joint2 = joint2;
    return true;
}

/**
 * Sets the first prismatic/revolute joint
 *
 * If this method is called while the joint is active, then the
 * joint will be marked as dirty. It will need to be deactivated
 * and reactivated to work properly.
 *
 * @param joint the first prismatic/revolute joint
 */
void GearJoint::setJoint1(const std::shared_ptr<Joint>& joint) {
    b2JointType type = joint == nullptr ? e_revoluteJoint : joint->getType();
    if (type != e_revoluteJoint && type != e_prismaticJoint) {
        CUAssertLog(false,"Joint has invalid type");
    } else {
        _joint1 = joint;
    }
}

/**
 * Sets the second prismatic/revolute joint
 *
 * If this method is called while the joint is active, then the
 * joint will be marked as dirty. It will need to be deactivated
 * and reactivated to work properly.
 *
 * @param joint the second prismatic/revolute joint
 */
void GearJoint::setJoint2(const std::shared_ptr<Joint>& joint) {
    b2JointType type = joint == nullptr ? e_revoluteJoint : joint->getType();
    if (type != e_revoluteJoint && type != e_prismaticJoint) {
        CUAssertLog(false,"Joint has invalid type");
    } else {
        _joint2 = joint;
    }
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
bool GearJoint::activatePhysics(b2World& world) {
    if (_joint != nullptr || _joint1 == nullptr || _joint2 == nullptr) {
        return false;
    }
    
    b2Joint* j1;
    b2Joint* j2;
    bool bootup = false;
    if (_joint1->getJoint() == nullptr) {
        _joint1->activatePhysics(world);
        bootup = true;
    }
    j1 = _joint1->getJoint();
    if (j1 == nullptr) {
        return false;
    }
    
    if (_joint2->getJoint() == nullptr) {
        _joint2->activatePhysics(world);
    }
    j2 = _joint2->getJoint();
    if (j2 == nullptr) {
        if (bootup) {
            _joint1->deactivatePhysics(world);
        }
        return false;
    }

    
    b2GearJointDef def;
    def.joint1 = j1;
    def.joint2 = j2;
    def.ratio = _ratio;
    def.collideConnected = _collideConnected;
    def.userData.pointer = reinterpret_cast<intptr_t>(this);
    _joint = world.CreateJoint(&def);

    _dirty = false;
    return _joint != nullptr;
}
