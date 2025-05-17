//
//  CUPulleyJoint.h
//  Cornell University Game Library (CUGL)
//
//  This module is a CUGL wrapper about b2_pulley_joint, implemented to make
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
#ifndef __CU_PULLEY_JOINT_H__
#define __CU_PULLEY_JOINT_H__
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
 * The pulley joint class.
 *
 * This class requires two ground anchors, two dynamic body anchor points, and
 * a pulley ratio.
 */
class PulleyJoint : public Joint {
private:
    /** The first ground anchor in world coordinates. */
    Vec2 _groundAnchorA;

    /** The second ground anchor in world coordinates. */
    Vec2 _groundAnchorB;

    /** The local anchor point relative to bodyA's origin. */
    Vec2 _localAnchorA;

    /** The local anchor point relative to bodyB's origin. */
    Vec2 _localAnchorB;

    /** The reference length for the segment attached to bodyA. */
    float _lengthA;

    /** The reference length for the segment attached to bodyB. */
    float _lengthB;

    /** The pulley ratio, used to simulate a block-and-tackle. */
    float _ratio;
    
#pragma mark Constructors
public:
    /**
     * Creates a new pulley joint with no obstacles
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead (in this case, in
     * one of the subclasses).
     */
    PulleyJoint();
    
    /**
     * Initializes a new pulley joint with the given obstacles.
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
     * Initializes a new pulley joint with the given obstacles and anchors
     *
     * The ground anchors are specified in world coordinate, not local
     * coordinates. All other attributes will be at their default values.
     *
     * @param obsA      The first obstacle to join
     * @param obsB      The second obstacle to join
     * @param groundA   The ground anchor of the first obstacle
     * @param groundB   The ground anchor of the second obstacle
     *
     * @return true if the obstacle is initialized properly, false otherwise.
     */
    bool initWithObstacles(const std::shared_ptr<Obstacle>& obsA,
                           const std::shared_ptr<Obstacle>& obsB,
                           const Vec2 groundA, const Vec2 groundB);
    

    /**
     * Returns a newly allocated pulley joint with default values.
     *
     * The joint will not have any associated obstacles and so attempting
     * to activate it will fail.
     *
     * @return a newly allocated pulley joint with default values.
     */
    static std::shared_ptr<PulleyJoint> alloc() {
        std::shared_ptr<PulleyJoint> result = std::make_shared<PulleyJoint>();
        return (result->init() ? result : nullptr);
    }

    /**
     * Returns a newly allocated pulley joint with the given obstacles.
     *
     * All other attributes will be at their default values.
     *
     * @param obsA  The first obstacle to join
     * @param obsB  The second obstacle to join
     *
     * @return a newly allocated pulley joint with the given obstacles.
     */
    static std::shared_ptr<PulleyJoint> allocWithObstacles(const std::shared_ptr<Obstacle>& obsA,
                                                           const std::shared_ptr<Obstacle>& obsB) {
        std::shared_ptr<PulleyJoint> result = std::make_shared<PulleyJoint>();
        return (result->initWithObstacles(obsA,obsB) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated pulley joint with the given obstacles and anchors
     *
     * The ground anchors are specified in world coordinate, not local
     * coordinates. All other attributes will be at their default values.
     *
     * @param obsA      The first obstacle to join
     * @param obsB      The second obstacle to join
     * @param groundA   The ground anchor of the first obstacle
     * @param groundB   The ground anchor of the second obstacle
     *
     * @return a newly allocated pulley joint with the given obstacles and anchors
     */
    static std::shared_ptr<PulleyJoint> allocWithObstacles(const std::shared_ptr<Obstacle>& obsA,
                                                           const std::shared_ptr<Obstacle>& obsB,
                                                           const Vec2 groundA, const Vec2 groundB) {
        std::shared_ptr<PulleyJoint> result = std::make_shared<PulleyJoint>();
        return (result->initWithObstacles(obsA,obsB,groundA,groundA) ? result : nullptr);
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
     * Returns the ground anchor point for obstacle A's in world coordinate.
     *
     * @return the ground anchor point for obstacle A's in world coordinate.
     */
    const Vec2& getGroundAnchorA() const { return _groundAnchorA; }

    /**
     * Sets the ground anchor point for obstacle A's in world coordinate.
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param point the ground anchor point
     */
    void setGroundAnchorA(const Vec2 point) {
        _groundAnchorA = point;
        _dirty = true;
    }

    /**
     * Sets the ground anchor point for obstacle A's in world coordinate.
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param x     the x-coordinate of the ground anchor point
     * @param y     the y-coordinate of the ground anchor point
     */
    void setGroundAnchorA(float x, float y) {
        _groundAnchorA.set(x,y);
        _dirty = true;
    }

    /**
     * Returns the ground anchor point for obstacle A's in world coordinate.
     *
     * @return the ground anchor point for obstacle A's in world coordinate.
     */
    const Vec2& getGroundAnchorB() const { return _groundAnchorB; }

    /**
     * Sets the ground anchor point for obstacle A's in world coordinate.
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param point the ground anchor point
     */
    void setGroundAnchorB(const Vec2 point) {
        _groundAnchorB = point;
        _dirty = true;
    }

    /**
     * Sets the ground anchor point for obstacle A's in world coordinate.
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param x     the x-coordinate of the ground anchor point
     * @param y     the y-coordinate of the ground anchor point
     */
    void setGroundAnchorB(float x, float y) {
        _groundAnchorB.set(x,y);
        _dirty = true;
    }

    /**
     * Returns the reference length for the segment attached to bodyA.
     *
     * @return the reference length for the segment attached to bodyA.
     */
    float getLengthA() const { return _lengthA; }

    
    /**
     * Sets the reference length for the segment attached to bodyA.
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param value the reference length for the segment attached to bodyA.
     */
    void setLengthA(float value) {
        if (value != _lengthA) {
            _dirty = true;
        }
        _lengthA = value;
    }


    /**
     * Returns the reference length for the segment attached to bodyB.
     *
     * @return the reference length for the segment attached to bodyB.
     */
    float getLengthB() const { return _lengthB; }

    /**
     * Sets the reference length for the segment attached to bodyB.
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param value the reference length for the segment attached to bodyB.
     */
    void setLengthB(float value) {
        if (value != _lengthB) {
            _dirty = true;
        }
        _lengthB = value;
    }

    /**
     * Returns the pulley ratio.
     *
     * This value is used to simulate a block-and-tackle.
     *
     * @return the pulley ratio.
     */
    float getRatio() const { return _ratio; }

    /**
     * Returns the pulley ratio.
     *
     * This value is used to simulate a block-and-tackle.
     *
     * @param value the pulley ratio.
     */
    void setRatio(float value) {
        if (value != _ratio) {
            _dirty = true;
        }
        _ratio = value;
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

#endif /* __CU_PULLEY_JOINT_H__ */
