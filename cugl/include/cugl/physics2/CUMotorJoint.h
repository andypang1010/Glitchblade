//
//  CUMotorJoint.h
//  Cornell University Game Library (CUGL)
//
//  This module is a CUGL wrapper about b2_motor_joint, implemented to make
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
#ifndef __CU_MOTOR_JOINT_H__
#define __CU_MOTOR_JOINT_H__
#include <cugl/physics2/CUJoint.h>

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
 * The motor joint class.
 */
class MotorJoint : public Joint {
protected:
    /** The position of bodyB minus the position of bodyA, in bodyA's frame, in meters. */
    Vec2 _linearOffset;
    
    /** The bodyB angle minus bodyA angle in radians. */
    float _angularOffset;
    
    /** The maximum motor force in N. */
    float _maxForce;
    
    /** The maximum motor torque in N-m. */
    float _maxTorque;
    
    /** Position correction factor in the range [0,1]. */
    float _correctionFactor;
    
#pragma mark Constructors
public:
    /**
     * Creates a new motor joint with no obstacles
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead (in this case, in
     * one of the subclasses).
     */
    MotorJoint();
    
    /**
     * Returns a newly allocated motor joint with default values.
     *
     * The joint will not have any associated obstacles and so attempting
     * to activate it will fail.
     *
     * @return a newly allocated motor joint with default values.
     */
    static std::shared_ptr<MotorJoint> alloc() {
        std::shared_ptr<MotorJoint> result = std::make_shared<MotorJoint>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated motor joint with the given obstacles.
     *
     * All other attributes will be at their default values.
     *
     * @param obsA  The first obstacle to join
     * @param obsB  The second obstacle to join
     *
     * @return a newly allocated motor joint with the given obstacles.
     */
    static std::shared_ptr<MotorJoint> allocWithObstacles(const std::shared_ptr<Obstacle>& obsA,
                                                          const std::shared_ptr<Obstacle>& obsB) {
        std::shared_ptr<MotorJoint> result = std::make_shared<MotorJoint>();
        return (result->initWithObstacles(obsA,obsB) ? result : nullptr);
    }
        
#pragma mark Joint Attributes
    
    /**
     * Returns the position of bodyB minus the position of bodyA.
     *
     * The value is measured in meters, with respect to bodyA's frame.
     *
     * @return the position of bodyB minus the position of bodyA.
     */
    const Vec2 getLinearOffset() const { return _linearOffset; }
    
    /**
     * Sets the position of bodyB minus the position of bodyA.
     *
     * The value is measured in meters, with respect to bodyA's frame.
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param pos   the position of bodyB minus the position of bodyA.
     */
    void setLinearOffset(const Vec2 pos) {
        _linearOffset = pos;
        _dirty = true;
    }
    
    /**
     * Sets the position of bodyB minus the position of bodyA.
     *
     * The value is measured in meters, with respect to bodyA's frame.
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param x the x-coordinate of the position of bodyB minus that of bodyA.
     * @param y the y-coordinate of the position of bodyB minus that of bodyA.
     */
    void setLinearOffset(float x, float y) {
        _linearOffset.set(x,y);
        _dirty = true;
    }
    
    /**
     * Returns the bodyB angle minus bodyA angle in radians.
     *
     * @return the bodyB angle minus bodyA angle in radians.
     */
    float getAngularOffset() const { return _angularOffset; }
    
    /**
     * Sets the bodyB angle minus bodyA angle in radians.
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param value the bodyB angle minus bodyA angle in radians.
     */
    void setAngularOffset(float value) {
        if (value != _angularOffset) {
            _dirty = true;
        }
        _angularOffset = value;
    }
    
    /**
     * Returns the maximum motor force in N.
     *
     * @return the maximum motor force in N.
     */
    float getMaxForce() const { return _maxForce; }
    
    /**
     * Sets the maximum motor force in N.
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param value the maximum motor force
     */
    void setMaxForce(float value) {
        if (value != _maxForce) {
            _dirty = true;
        }
        _maxForce = value;
    }
    
    /// The maximum motor torque in N-m.
    /**
     * Returns the maximum motor torque in N-m.
     *
     * @return the maximum motor torque in N-m.
     */
    float getMaxTorque() const { return _maxTorque; }
    
    /**
     * Sets the maximum motor torque in N-m.
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param value the maximum motor torque
     */
    void setMaxTorque(float value) {
        if (value != _maxTorque) {
            _dirty = true;
        }
        _maxTorque = value;
    }
    
    /**
     * Returns the position correction factor in the range [0,1].
     *
     * @return the position correction factor in the range [0,1].
     */
    float getCorrectionFactor() const { return _correctionFactor; }
    
    /**
     * Sets the position correction factor in the range [0,1].
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param value the position correction factor
     */
    void setCorrectionFactor(float value) {
        if (value != _correctionFactor) {
            _dirty = true;
        }
        _correctionFactor = value;
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
    virtual bool activatePhysics(b2World& world) override;
};
    
    }
}
#endif /* __CU_MOTOR_JOINT_H__ */
