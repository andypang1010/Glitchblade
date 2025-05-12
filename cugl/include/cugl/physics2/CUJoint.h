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
#ifndef __CU_JOINT_H__
#define __CU_JOINT_H__
#include <cugl/core/math/CUVec2.h>
#include <box2d/b2_joint.h>
#include <box2d/b2_world.h>
#include <memory>

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

// Forward declaration of obstacle
class Obstacle;

#pragma mark -
#pragma mark Base Joint

/**
 * The base joint class.
 *
 * This is an extension of the Box2d b2Joint class to make it easier to use
 * with the {@link Obstacle}. Like that class, we combine the definition and
 * the joint itself into a single class.
 *
 * This class stores the base attributes for a joint, as well as the methods for
 * managing physics and garbage collection. However, there is type information,
 * and you should never instantiate objects of the class. Use one of the
 * subclasses instead.
 *
 * Many of the method comments in this class are taken from the Box2d manual by
 * Erin Catto (2011).
 */
class Joint {
protected:
    /** A reference to the joint (nullptr if it is not active) */
    b2Joint* _joint;
    
    /** The underlying joint type (set automatically for concrete joint types) */
    b2JointType _type;
    
    /** The first attached obstacle. */
    std::shared_ptr<Obstacle> _bodyA;
    
    /** The second attached obstacle. */
    std::shared_ptr<Obstacle> _bodyB;
    
    /** Set this flag to true if the attached bodies should collide. */
    bool _collideConnected;
    
    /// Track garbage collection status
    /** Whether the joint should be removed from the world on next pass */
    bool _remove;
    /** Whether the joint has changed properties and needs to be rebuilt */
    bool _dirty;
    
#pragma mark Constructors
public:
    /**
     * Creates a new physics joint with no obstacles
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead (in this case, in
     * one of the subclasses).
     */
    Joint();
    
    /**
     * Deletes this physics joint and all of its resources.
     *
     * We have to make the destructor public so that we can polymorphically
     * delete physics objects. Note that we do not allow an joint to be
     * deleted while physics is still active. Doing so will result in an error.
     */
    virtual ~Joint();
    
    /**
     * Initializes a new physics joint with no obstacles.
     *
     * You should set the obstacles (and other attributes) before activating
     * this joint.
     *
     * @return true if the obstacle is initialized properly, false otherwise.
     */
    bool init() { return true; }
    
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
    virtual bool initWithObstacles(const std::shared_ptr<Obstacle>& obsA,
                                   const std::shared_ptr<Obstacle>& obsB);
    
    
#pragma mark Joint Attributes
    /**
     * Returns the type of this joint
     *
     * @return the type of this joint
     */
    b2JointType getType() const {
        return _joint == nullptr ? _type : _joint->GetType();
    }
    
    /**
     * Sets the first obstacle attached to this joint.
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param obs   the first body attached to this joint.
     */
    void setObstacleA(const std::shared_ptr<Obstacle>& obs) {
        if (_bodyA.get() != obs.get()) {
            _dirty = true;
        }
        _bodyA = obs;
    }
    
    /**
     * Returns the first obstacle attached to this joint.
     *
     * @return the first obstacle attached to this joint.
     */
    std::shared_ptr<Obstacle>& getObstacleA() {
        return _bodyA;
    }
    
    /**
     * Sets the second obstacle attached to this joint.
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param obs   the second obstacle attached to this joint.
     */
    void setObstacleB(const std::shared_ptr<Obstacle>& obs) {
        if (_bodyB.get() != obs.get()) {
            _dirty = true;
        }
        _bodyA = obs;
    }
    
    /**
     * Returns the second obstacle attached to this joint.
     *
     * @return the second obstacle attached to this joint.
     */
    std::shared_ptr<Obstacle>& getObstacleB() {
        return _bodyB;
    }
    
    /**
     * Returns true if the attached bodies should collide.
     *
     * @return true if the attached bodies should collide.
     */
    bool getCollideConnected() {
        return (_joint != nullptr) ? _joint->GetCollideConnected() : _collideConnected;
    }
    
    /**
     * Sets the flag for whether the attached bodies should collide.
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param flag  Whether the attached bodies should collide.
     */
    void setCollideConnected(bool flag) {
        if (_collideConnected != flag) {
            _dirty = true;
        }
        _collideConnected = flag;
    }

#pragma mark Garbage Collection
    /**
     * Instructs the object to release its Box2d joint.
     *
     * This method is required when a joint is deleted in response to a
     * deletion of one of its bodies.
     */
    void release();
    
    /**
     * Returns true if our object has been flagged for garbage collection
     *
     * A garbage collected object will be removed from the physics world at
     * the next time step.
     *
     * @return true if our object has been flagged for garbage collection
     */
    bool isRemoved() const { return _remove; }
    
    /**
     * Sets whether our object has been flagged for garbage collection
     *
     * A garbage collected object will be removed from the physics world at
     * the next time step.
     *
     * @param value  whether our object has been flagged for garbage collection
     */
    void markRemoved(bool value) { _remove = value; }
    
    /**
     * Returns true if the shape information must be updated.
     *
     * The joint must wait for collisions to complete before it can be reset.
     * Reseting the joint is managed by {@link ObstacleWorld}.
     *
     * @return true if the shape information must be updated.
     */
    bool isDirty() const { return _dirty; }
    
    /**
     * Sets whether the shape information must be updated.
     *
     * The joint must wait for collisions to complete before it can be reset.
     * Reseting the joint is managed by {@link ObstacleWorld}.
     *
     * @param value  whether the shape information must be updated.
     */
    void markDirty(bool value) { _dirty = value; }
    
#pragma mark Physics Methods
    /**
     * Returns a (weak) reference to the Box2D joint
     *
     * You use this joint to access Box2D primitives. As a weak reference,
     * this physics obstacle does not transfer ownership of this body. In
     * addition, the value may be a nullptr.
     *
     * @return a (weak) reference to the Box2D joint
     */
    b2Joint* getJoint() { return _joint; }
    
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
    virtual bool activatePhysics(b2World& world) { return false; }
    
    /**
     * Destroys the Box2D joint if applicable.
     *
     * This removes the joint from the Box2D world.
     *
     * @param world Box2D world that stores the joint
     */
    void deactivatePhysics(b2World& world) {
        if (_joint != nullptr) {
            world.DestroyJoint(_joint);
            _joint = nullptr;
        }
    }
    
};

    }
}
#endif /* __CU_JOINT_H__ */
