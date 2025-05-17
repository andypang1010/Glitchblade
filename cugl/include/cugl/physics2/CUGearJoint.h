//
//  CUGearJoint.h
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
#ifndef __CU_GEAR_JOINT_H__
#define __CU_GEAR_JOINT_H__
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
 * The gear joint class.
 *
 * This definition requires two existing revolute or prismatic joints (any
 * combination will work). The second body on the input joints must both be
 * dynamic.
 *
 * You specify a gear ratio to bind the motions together:
 *
 *      coordinate1 + ratio * coordinate2 = constant
 *
 * The ratio can be negative or positive. If one joint is a revolute joint
 * and the other joint is a prismatic joint, then the ratio will have units
 * of length or units of 1/length.
 *
 * WARNING: You have to manually destroy the gear joint if joint1 or joint2
 * is destroyed.
 */
class GearJoint : public Joint {
protected:
    /** The first revolute/prismatic joint attached to the gear joint. */
    std::shared_ptr<Joint> _joint1;
    
    /** The second revolute/prismatic joint attached to the gear joint. */
    std::shared_ptr<Joint> _joint2;
    
    /** The gear ratio. */
    float _ratio;
    
#pragma mark Constructors
public:
    /**
     * Creates a new gear joint with no joints
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead (in this case, in
     * one of the subclasses).
     */
    GearJoint();
    
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
    bool initWithJoints(const std::shared_ptr<Joint>& joint1,
                        const std::shared_ptr<Joint>& joint2);
    
    
    /**
     * Returns a newly allocated gear joint with default values.
     *
     * The joint will not have any associated prismatic/revolute joints and so
     * attempting to activate it will fail.
     *
     * @return a newly allocated gear joint with default values.
     */
    static std::shared_ptr<GearJoint> alloc() {
        std::shared_ptr<GearJoint> result = std::make_shared<GearJoint>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated gear joint with the given prismatic/revolute joints.
     *
     * All other attributes will be at their default values.
     *
     * @param joint1    The first prismatic/revolute joint to use
     * @param joint2    The second prismatic/revolute joint to use
     *
     * @return a newly allocated gear joint with the given prismatic/revolute joints.
     */
    static std::shared_ptr<GearJoint> allocWithJoints(const std::shared_ptr<Joint>& joint1,
                                                      const std::shared_ptr<Joint>& joint2) {
        std::shared_ptr<GearJoint> result = std::make_shared<GearJoint>();
        return (result->initWithJoints(joint1,joint2) ? result : nullptr);
    }
    
    
#pragma mark Joint Attributes
    /**
     * Returns the first prismatic/revolute joint
     *
     * @return the first prismatic/revolute joint
     */
    const std::shared_ptr<Joint>& getJoint1() const { return _joint1; }
    
    /**
     * Sets the first prismatic/revolute joint
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param joint the first prismatic/revolute joint
     */
    void setJoint1(const std::shared_ptr<Joint>& joint);
    
    /**
     * Returns the second prismatic/revolute joint
     *
     * @return the second prismatic/revolute joint
     */
    const std::shared_ptr<Joint>& getJoint2() const { return _joint2; }
    
    /**
     * Sets the second prismatic/revolute joint
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param joint the second prismatic/revolute joint
     */
    void setJoint2(const std::shared_ptr<Joint>& joint);
    
    /**
     * Returns the gear ratio.
     *
     * You specify a gear ratio to bind the motions together:
     *
     *      coordinate1 + ratio * coordinate2 = constant
     *
     * The ratio can be negative or positive. If one joint is a revolute joint
     * and the other joint is a prismatic joint, then the ratio will have units
     * of length or units of 1/length.
     *
     * @return the gear ratio.
     */
    float getRatio() const { return _ratio; }
    
    /**
     * Sets the gear ratio.
     *
     * You specify a gear ratio to bind the motions together:
     *
     *      coordinate1 + ratio * coordinate2 = constant
     *
     * The ratio can be negative or positive. If one joint is a revolute joint
     * and the other joint is a prismatic joint, then the ratio will have units
     * of length or units of 1/length.
     *
     * If this method is called while the joint is active, then the
     * joint will be marked as dirty. It will need to be deactivated
     * and reactivated to work properly.
     *
     * @param value the gear ratio
     */
    void setGearRatio(float value) {
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

#endif /* __CU_GEAR_JOINT_H__ */
