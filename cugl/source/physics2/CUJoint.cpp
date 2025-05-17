//
//  CUJoint.h
//  Cornell University Game Library (CUGL)
//
//  Like the Obstacle class, this is class is used to introduce some additional
//  coupling into Box2d. In this case, it couples active joints with their
//  definitions. This makes it a little easier to turn joints on and off, and
//  to share them across physics worlds. The latter is necessary for networking.
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
#include <cugl/physics2/CUJoint.h>
#include <cugl/physics2/CUObstacle.h>
#include <cugl/core/util/CUDebug.h>
#include <box2d/b2_joint.h>

using namespace cugl;
using namespace cugl::physics2;
/**
 * Creates a new physics joint with no obstacles
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
 * the heap, use one of the static constructors instead (in this case, in
 * one of the subclasses).
 */
Joint::Joint() :
_joint(nullptr),
_type(e_unknownJoint),
_dirty(false),
_remove(false),
_collideConnected(false) {
}

/**
 * Deletes this physics joint and all of its resources.
 *
 * We have to make the destructor public so that we can polymorphically
 * delete physics objects. Note that we do not allow an joint to be
 * deleted while physics is still active. Doing so will result in an error.
 */
Joint::~Joint() {
    CUAssertLog(_joint == nullptr, "You must deactivate physics before deleting an joint");
}

/**
 * Initializes a new physics joint with the given obstacles.
 *
 * All other attributes will be at their default values.
 *
 * @param obsA  The first obstacle to join
 * @param obsB  The second obstacle to join
 *
 * @return true if the obstacle is initialized properly, false otherwise.
 */
bool Joint::initWithObstacles(const std::shared_ptr<Obstacle>& obsA,
                              const std::shared_ptr<Obstacle>& obsB) {
    _bodyA = obsA;
    _bodyB = obsB;
    return true;
}

/**
 * Instructs the object to release its Box2d joint.
 *
 * This method is required when a joint is deleted in response to a
 * deletion of one of its bodies.
 */
void Joint::release() {
    _joint = nullptr;
}
