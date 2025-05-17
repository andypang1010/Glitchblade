//
//  CUWheelJoint.h
//  Cornell University Game Library (CUGL)
//
//  This module is a CUGL wrapper about b2_wheel_joint, implemented to make
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
#ifndef __CU_WHEEL_JOINT_H__
#define __CU_WHEEL_JOINT_H__
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
 * The wheel joint class.
 *
 * This joint requires defining a line of motion using an axis and an anchor
 * point. The definition uses local anchor points and a local axis so that the
 * initial configuration can violate the constraint slightly. The joint
 * translation is zero when the local anchor points coincide in world space.
 * Using local anchors and a local axis helps when saving and loading a game.
 */
class WheelJoint : public Joint {
private:
    /** The local anchor point relative to bodyA's origin. */
    Vec2 _localAnchorA;
    
    /** The local anchor point relative to bodyB's origin. */
    Vec2 _localAnchorB;
    
    /** The local translation unit axis in bodyA. */
    Vec2 _localAxisA;
    
    /** The constrained angle between the bodies. */
    float _referenceAngle;
    
    /** Enable/disable the joint limit. */
    bool _enableLimit;
    
    /** The lower translation limit, usually in meters. */
    float _lowerTranslation;
    
    /** The upper translation limit, usually in meters. */
    float _upperTranslation;
    
    /** Enable/disable the joint motor. */
    bool _enableMotor;
    
    /** The maximum motor torque, usually in N-m. */
    float _maxMotorTorque;
    
    /** The desired motor speed in radians per second. */
    float _motorSpeed;
    
    /** The linear stiffness in N/m. */
    float _stiffness;
    
    /** The linear damping in N*s/m. */
    float _damping;

    
#pragma mark Constructors
public:
    /**
     * Creates a new wheel joint with no obstacles
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead (in this case, in
     * one of the subclasses).
     */
    WheelJoint();
    
    /**
     * Initializes a new wheel joint with the given obstacles.
     *
     * All other attributes will be at their default values.
     *
     * @param obsA  The first obstacle to join
     * @param obsB  The second obstacle to join
     *
     * @return true if the obstacle is initialized properly, false otherwise.
     */
    bool initWithObstacles(const std::shared_ptr<Obstacle>& obsA,
                           const std::shared_ptr<Obstacle>& obsB) override;
    
    /**
     * Initializes a new wheel joint with the given obstacles and anchors
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
    bool initWithObstacles(const std::shared_ptr<Obstacle>& obsA,
                           const std::shared_ptr<Obstacle>& obsB,
                           const Vec2 localA, const Vec2 localB);
    

    /**
     * Returns a newly allocated wheel joint with default values.
     *
     * The joint will not have any associated obstacles and so attempting
     * to activate it will fail.
     *
     * @return a newly allocated wheel joint with default values.
     */
    static std::shared_ptr<WheelJoint> alloc() {
        std::shared_ptr<WheelJoint> result = std::make_shared<WheelJoint>();
        return (result->init() ? result : nullptr);
    }

    /**
     * Returns a newly allocated wheel joint with the given obstacles.
     *
     * All other attributes will be at their default values.
     *
     * @param obsA  The first obstacle to join
     * @param obsB  The second obstacle to join
     *
     * @return a newly allocated wheel joint with the given obstacles.
     */
    static std::shared_ptr<WheelJoint> allocWithObstacles(const std::shared_ptr<Obstacle>& obsA,
                                                          const std::shared_ptr<Obstacle>& obsB) {
        std::shared_ptr<WheelJoint> result = std::make_shared<WheelJoint>();
        return (result->initWithObstacles(obsA,obsB) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated wheel joint with the given obstacles and anchors
     *
     * All other attributes will be at their default values.
     *
     * @param obsA      The first obstacle to join
     * @param obsB      The second obstacle to join
     * @param localA    The local anchor of the first obstacle
     * @param localB    The local anchor of the second obstacle
     *
     * @return a newly allocated wheel joint with the given obstacles and anchors
     */
    static std::shared_ptr<WheelJoint> allocWithObstacles(const std::shared_ptr<Obstacle>& obsA,
                                                          const std::shared_ptr<Obstacle>& obsB,
                                                          const Vec2 localA, const Vec2 localB) {
        std::shared_ptr<WheelJoint> result = std::make_shared<WheelJoint>();
        return (result->initWithObstacles(obsA,obsB,localA,localB) ? result : nullptr);
    }
    
#pragma mark Joint Attributes
    /**
     * Returns the local anchor point relative to obstacle A's origin.
     *
     * @return the local anchor point relative to obstacle A's origin.
     */
    const Vec2& getLocalAnchorA() const { return _localAnchorA; }

    /**
     * Sets the local anchor point relative to obstacle A's origin.
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param point the local anchor point
     */
    void setLocalAnchorA(const Vec2 point) {
        _localAnchorA = point;
        _dirty = true;
    }

    /**
     * Sets the local anchor point relative to obstacle A's origin.
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param x     the x-coordinate of the local anchor point
     * @param y     the y-coordinate of the local anchor point
     */
    void setLocalAnchorA(float x, float y) {
        _localAnchorA.set(x,y);
        _dirty = true;
    }

    /**
     * Returns the local anchor point relative to obstacle B's origin.
     *
     * @return the local anchor point relative to obstacle B's origin.
     */
    const Vec2& getLocalAnchorB() const { return _localAnchorB; }

    /**
     * Sets the local anchor point relative to obstacle B's origin.
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param point the local anchor point
     */
    void setLocalAnchorB(const Vec2 point) {
        _localAnchorB = point;
        _dirty = true;
    }

    /**
     * Sets the local anchor point relative to obstacle B's origin.
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param x     the x-coordinate of the local anchor point
     * @param y     the y-coordinate of the local anchor point
     */
    void setLocalAnchorB(float x, float y) {
        _localAnchorB.set(x,y);
        _dirty = true;
    }
    
    /**
     * Returns the local translation unit axis.
     *
     * This axis is measured with respect to bodyA.
     *
     * @return the local translation unit axis.
     */
    const Vec2& getLocalAxisA() const { return _localAxisA; }

    /**
     * Sets the local translation unit axis.
     *
     * This axis is measured with respect to bodyA.
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param point the local translation unit axis.
     */
    void setLocalAxisA(const Vec2 point) {
        _localAxisA = point;
        _dirty = true;
    }

    /**
     * Sets the local translation unit axis.
     *
     * This axis is measured with respect to bodyA.
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param x     the x-coordinate of the local translation unit axis
     * @param y     the y-coordinate of the local translation unit axis
     */
    void setLocalAxisA(float x, float y) {
        _localAxisA.set(x,y);
        _dirty = true;
    }
    
    /**
     * Returns the constrained angle between the bodies.
     *
     * This value is measured bodyB - bodyA in radians.
     *
     * @return the constrained angle between the bodies.
     */
    float getReferenceAngle() const { return _referenceAngle; }

    /**
     * Sets the constrained angle between the bodies.
     *
     * This value is measured bodyB - bodyA in radians.
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param value the constrained angle between the bodies.
     */
    void setReferenceAngle(float value) {
        if (value != _referenceAngle) {
            _dirty = true;
        }
        _referenceAngle = value;
    }
    
    /**
     * Returns true if the joint limit is enabled.
     *
     * @return true if the joint limit is enabled.
     */
    bool hasLimit() const { return _enableLimit; }

    /**
     * Enables/disables the joint limit.
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param value Whether to enable the joint limit
     */
    void enableLimit(bool value) {
        if (value != _enableLimit) {
            _dirty = true;
        }
        _enableLimit = value;
    }

    /**
     * Returns true if the joint motor is enabled.
     *
     * @return true if the joint motor is enabled.
     */
    bool hasMotor() const { return _enableMotor; }
    
    /**
     * Enables/disables the joint motor.
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param value Whether to enable the joint motor
     */
    void enableMotor(bool value) {
        if (value != _enableMotor) {
            _dirty = true;
        }
        _enableMotor = value;
    }

    /**
     * Returns the lower translation limit.
     *
     * This is usually measured in meters.
     *
     * @return the lower translation limit.
     */
    float getLowerTranslation() const { return _lowerTranslation; }
    
    /**
     * Sets the lower translation limit.
     *
     * This is usually measured in meters.
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param value the lower translation limit.
     */
    void setLowerTranslation(float value) {
        if (value != _lowerTranslation) {
            _dirty = true;
        }
        _lowerTranslation = value;
    }

    /**
     * Returns the upper translation limit.
     *
     * This is usually measured in meters.
     *
     * @return the upper translation limit.
     */
    float getUpperTranslation() const { return _upperTranslation; }
    
    /**
     * Sets the upper translation limit.
     *
     * This is usually measured in meters.
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param value the upper translation limit.
     */
    void setUpperTranslation(float value) {
        if (value != _upperTranslation) {
            _dirty = true;
        }
        _upperTranslation = value;
    }

    /**
     * Returns the maximum motor torque, usually in N-m.
     *
     * @return the maximum motor torque, usually in N-m.
     */
    float getMaxMotorTorque() const { return _maxMotorTorque; }

    /**
     * Sets the maximum motor torque, usually in N-m.
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param value the maximum motor torque
     */
    void getMaxMotorTorque(float value) {
        if (value != _maxMotorTorque) {
            _dirty = true;
        }
        _maxMotorTorque = value;
    }

    /**
     * Returns the desired motor speed in radians per second.
     *
     * @return the desired motor speed in radians per second.
     */
    float getMotorSpeed() const { return _motorSpeed; }
    
    /**
     * Sets the desired motor speed in radians per second.
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param value the desired motor speed
     */
    void setMotorSpeed(float value) {
        if (value != _motorSpeed) {
            _dirty = true;
        }
        _motorSpeed = value;
    }
    
    /**
     * Returns the linear stiffness in N/m.
     *
     * @return the linear stiffness in N/m.
     */
    float getStiffness() const { return _stiffness; }
    
    /**
     * Sets the linear stiffness in N/m.
     *
     * @param value the linear stiffness
     */
    void setStiffness(float value) {
        if (value != _stiffness) {
            _dirty = true;
        }
        _stiffness = value;
    }
    
    /**
     * Returns the linear damping in N*s/m.
     *
     * @return the linear damping in N*s/m.
     */
    float getDamping() const { return _damping; }
    
    /**
     * Sets the linear damping in N*s/m.
     *
     * @param value the linear damping
     */
    void setDamping(float value) {
        if (value != _damping) {
            _dirty = true;
        }
        _damping = value;
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
#endif /* __CU_WHEEL_JOINT_H__ */
