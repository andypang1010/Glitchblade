//
//  CUTransformAction.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for transformation actions on 2d scene graphs.
//  These include things like moving, rotating, and scaling. In all cases, these
//  can be specified with either the end target or by an offset amount.
//
//  The classes in this module are actually factories for creating actions from
//  scene graph nodes. These classes use our standard shared-pointer
//  architecture.
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
//  Author: Walker White (based on work by Sophie Huang)
//  Version: 7/3/24 (CUGL 3.0 reorganization)
//
#ifndef __CU_TRANSFORM_ACTION_H__
#define __CU_TRANSFORM_ACTION_H__
#include <cugl/core/actions/CUAction.h>
#include <cugl/scene2/CUSceneNode2.h>

namespace cugl {

    /**
     * The classes to construct a 2-d scene graph.
     *
     * Even though this is an optional package, this is one of the core features
     * of CUGL. These classes provide basic UI support (including limited Figma)
     * support. Any 2-d game will make extensive use of these classes. And
     * even 3-d games may use these classes for the HUD overlay.
     */
    namespace scene2 {

#pragma mark MoveBy
/**
 * This factory creates an action for movement by a given vector amount.
 *
 * This class is actually a factory for creating movement actions. To create
 * an action, call {@link #attach} with the appropriate {@link SceneNode}.
 * Note that this class contains no duration information. That is supplied
 * when the action is added to {@link ActionTimeline}.
 */
class MoveBy {
private:
    /** Difference between the destination and initial position */
    Vec2 _delta;

public:
    /**
     * Creates an uninitialized movement animation.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    MoveBy() { _delta = Vec2::ZERO; }

    /**
     * Deletes this animation, disposing all resources
     */
    ~MoveBy() { dispose(); }
    
    /**
     * Disposes all of the resources used by this animation.
     *
     * A disposed animation can be safely reinitialized.
     */
    void dispose() { _delta = Vec2::ZERO; }
    
    /**
     * Initializes a degenerate movement animation.
     *
     * The movement amount is set to (0.0, 0.0), meaning no movement 
     * takes place.
     *
     * @return true if initialization was successful.
     */
    bool init() {
        return init(Vec2::ZERO);
    }
    
    /**
     * Initializes a movement animation by the given vector.
     *
     * When animated, this associated action will move its target by the given
     * delta.
     *
     * @param delta The amount to move the attached node
     *
     * @return true if initialization was successful.
     */
    bool init(const Vec2 delta);
    
    /**
     * Returns a newly allocated degenerate movement animation.
     *
     * The movement amount is set to (0.0, 0.0), meaning no movement
     * takes place.
     *
     * @return a newly allocated degenerate movement animation.
     */
    static std::shared_ptr<MoveBy> alloc() {
        std::shared_ptr<MoveBy> result = std::make_shared<MoveBy>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated movement animation over the given vector.
     *
     * When animated, this associated action will move its target by the given
     * delta.
     *
     * @param delta The amount to move the attached node
     *
     * @return a newly allocated movement animation over the given vector.
     */
    static std::shared_ptr<MoveBy> alloc(const Vec2 delta) {
        std::shared_ptr<MoveBy> result = std::make_shared<MoveBy>();
        return (result->init(delta) ? result : nullptr);
    }

    /**
     * Returns the movement delta for this animation.
     *
     * Changing this value for an actively animating action can have undefined
     * side effects.
     *
     * @return the movement delta for this action.
     */
    const Vec2& getDelta() const { return _delta; }

    /**
     * Sets the movement delta for this action.
     *
     * Changing this value for an actively animating action can have undefined
     * side effects.
     *
     * @param delta the movement delta for this action.
     */
    void setDelta(const Vec2& delta) { _delta = delta; }
    
    /**
     * Returns an action attaching this animation to the given scene node
     *
     * This action will reference this object during the animation. Any changes
     * to this object during that time may alter the animation.
     *
     * Note that the action has no associated duration. That should be set when
     * it is added to {@link ActionTimeline}.
     *
     * @param node  The node to attach
     *
     * @return an action attaching this animation to the given scene node
     */
    ActionFunction attach(const std::shared_ptr<scene2::SceneNode>& node);
};


#pragma mark -
#pragma mark MoveTo

/**
 * This factory creates an action for movement to a given position
 *
 * This class is actually a factory for creating movement actions. To create
 * an action, call {@link #attach} with the appropriate {@link SceneNode}.
 * Note that this class contains no duration information. That is supplied
 * when the action is added to {@link ActionTimeline}.
 */
class MoveTo {
private:
    /** The target destination for this action */
    Vec2 _target;

public:
    /**
     * Creates an uninitialized movement animation.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    MoveTo() { _target = Vec2::ZERO; }

    /**
    * Deletes this animation, disposing all resources
    */
    ~MoveTo() { dispose(); }
    
    /**
     * Disposes all of the resources used by this animation.
     *
     * A disposed animation can be safely reinitialized.
     */
    void dispose() { _target = Vec2::ZERO; }
    
    /**
     * Initializes a movement animation towards the origin.
     *
     * The target position is set to (0.0, 0.0), meaning that this action will
     * move a node towards the origin.
     *
     * @return true if initialization was successful.
     */
    bool init() {
        return init(Vec2(0.0, 0.0));
    }
    
    /**
     * Initializes a movement animation towards towards the given position.
     *
     * @param target    The target position
     *
     * @return true if initialization was successful.
     */
    bool init(const Vec2 target);
    
    /**
     * Returns a newly allocated motion animation towards the origin.
     *
     * The target position is set to (0.0, 0.0), meaning that this action will
     * move a node towards the origin.
     *
     * @return a newly allocated motion animation towards the origin.
     */
    static std::shared_ptr<MoveTo> alloc() {
        std::shared_ptr<MoveTo> result = std::make_shared<MoveTo>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated motion animation towards the given position.
     *
     * @param target    The target position
     *
     * @return a newly allocated motion animation towards the given position.
     */
    static std::shared_ptr<MoveTo> alloc(const Vec2 target) {
        std::shared_ptr<MoveTo> result = std::make_shared<MoveTo>();
        return (result->init(target) ? result : nullptr);
    }

    /**
     * Returns the movement target for this action.
     *
     * Changing this value for an actively animating action can have undefined
     * side effects.
     *
     * @return the movement target for this action.
     */
    const Vec2& getTarget() const { return _target; }
    
    /**
     * Sets the movement target for this action.
     *
     * Changing this value for an actively animating action can have undefined
     * side effects.
     *
     * @param target    the movement target for this action.
     */
    void setTarget(const Vec2& target) { _target = target; }
    
    /**
     * Returns an action attaching this animation to the given scene node
     *
     * This action will reference this object during the animation. Any changes
     * to this object during that time may alter the animation.
     *
     * Note that the action has no associated duration. That should be set when
     * it is added to {@link ActionTimeline}.
     *
     * @param node  The node to attach
     *
     * @return an action attaching this animation to the given scene node
     */
    ActionFunction attach(const std::shared_ptr<scene2::SceneNode>& node);
    
};
    
#pragma mark -
#pragma mark RotateBy

/**
 * This factory creates an action rotating by a given angle amount.
 *
 * The angle is measured in radians, counter-clockwise from the x-axis.
 *
 * This class is actually a factory for creating movement actions. To create
 * an action, call {@link #attach} with the appropriate {@link SceneNode}.
 * Note that this class contains no duration information. That is supplied
 * when the action is added to {@link ActionTimeline}.
 */
class RotateBy {
private:
    /** Difference between the final and initial angle in radians */
    float _delta;

public:
    /**
     * Creates an uninitialized rotation animation.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    RotateBy() : _delta(0) {}

    /**
     * Deletes this animation, disposing all resources
     */
    ~RotateBy() { dispose(); }
    
    /**
     * Disposes all of the resources used by this animation.
     *
     * A disposed animation can be safely reinitialized.
     */
    void dispose() { _delta = 0; }
    
    /**
     * Initializes a degenerate rotation animation.
     *
     * The rotation amount is set to 0.0, meaning no rotation takes place.
     *
     * @return true if initialization was successful.
     */
    bool init() {
        return init(0.0f);
    }
    
    /**
     * Initializes a rotation animation of the given angle.
     *
     * When animated, this action will rotate its target by the given delta.
     * The angle is measured in radians, counter-clockwise from the x-axis.
     *
     * @param delta The amount to rotate the target node
     *
     * @return true if initialization was successful.
     */
    bool init(float delta);
    
    /**
     * Returns a newly allocated, degenerate rotation animation.
     *
     * The rotation amount is set to 0.0, meaning no rotation takes place.
     *
     * @return a newly allocated, degenerate rotation animation.
     */
    static std::shared_ptr<RotateBy> alloc() {
        std::shared_ptr<RotateBy> result = std::make_shared<RotateBy>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated rotation animation of the given angle.
     *
     * When animated, this action will rotate its target by the given delta.
     * The angle is measured in radians, counter-clockwise from the x-axis.
     *
     * @param delta The amount to rotate the target node
     *
     * @return a newly allocated rotation animation of the given angle.
     */
    static std::shared_ptr<RotateBy> alloc(float delta) {
        std::shared_ptr<RotateBy> result = std::make_shared<RotateBy>();
        return (result->init(delta) ? result : nullptr);
    }
    
    /**
     * Returns the rotation delta for this action.
     *
     * Changing this value for an actively animating action can have undefined
     * side effects.
     *
     * @return the rotation delta for this action.
     */
    float getDelta() const { return _delta; }
    
    /**
     * Sets the rotation delta for this action.
     *
     * Changing this value for an actively animating action can have undefined
     * side effects.
     *
     * @param delta the rotation delta for this action.
     */
    void setDelta(float delta) { _delta = delta; }
    
    /**
     * Returns an action attaching this animation to the given scene node
     *
     * This action will reference this object during the animation. Any changes
     * to this object during that time may alter the animation.
     *
     * Note that the action has no associated duration. That should be set when
     * it is added to {@link ActionTimeline}.
     *
     * @param node  The node to attach
     *
     * @return an action attaching this animation to the given scene node
     */
    ActionFunction attach(const std::shared_ptr<scene2::SceneNode>& node);
    
};


#pragma mark -
#pragma mark RotateTo
/**
 * This factory creates an action rotating to a specific angle.
 *
 * The angle is measured in radians, counter-clockwise from the x-axis.
 *
 * This class is actually a factory for creating movement actions. To create
 * an action, call {@link #attach} with the appropriate {@link SceneNode}.
 * Note that this class contains no duration information. That is supplied
 * when the action is added to {@link ActionTimeline}.
 */
class RotateTo {
private:
    /** The target angle for this action */
    float _angle;
    
public:
    /**
     * Creates an uninitialized rotation action.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    RotateTo() : _angle(0) {}
    
    /**
     * Deletes this action instance, disposing all resources
     */
    ~RotateTo() { dispose(); }

    /**
     * Disposes all of the resources used by this action.
     *
     * A disposed action can be safely reinitialized.
     */
    void dispose() { _angle = 0; }
    
    /**
     * Initializes a rotation animation towards the x-axis
     *
     * The target angle is set to 0.0. Because of how rotatations are
     * interpolated, this guarantees that the rotation will be clockwise.
     *
     * @return true if initialization was successful.
     */
    bool init() {
        return init(0.0f);
    }
    
    /**
     * Initializes a rotation animation towards the given angle
     *
     * This angle is measured in radians, counter-clockwise from the x-axis.
     * The animation will be counter-clockwise if the target angle is larger
     * than the current one. Otherwise it will be clockwise.
     *
     * @param angle The target rotation angle
     *
     * @return true if initialization was successful.
     */
    bool init(float angle);
    
    /**
     * Returns a newly allocated rotation animation towards the x-axis
     *
     * The target angle is set to 0.0. Because of how rotatations are
     * interpolated, this guarantees that the rotation will be clockwise.
     *
     * @return a newly allocated rotation animation towards the x-axis
     */
    static std::shared_ptr<RotateTo> alloc() {
        std::shared_ptr<RotateTo> result = std::make_shared<RotateTo>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated rotation animation towards the given angle
     *
     * This angle is measured in radians, counter-clockwise from the x-axis.
     * The animation will be counter-clockwise if the target angle is larger
     * than the current one. Otherwise it will be clockwise.
     *
     * @param angle The target rotation angle
     *
     * @return a newly allocated rotation animation towards the given angle
     */
    static std::shared_ptr<RotateTo> alloc(float angle) {
        std::shared_ptr<RotateTo> result = std::make_shared<RotateTo>();
        return (result->init(angle) ? result : nullptr);
    }
    
    /**
     * Returns the rotation target angle for this action.
     *
     * Changing this value for an actively animating action can have undefined
     * side effects.
     *
     * @return the rotation target angle for this action.
     */
    float getAngle() const { return _angle; }
    
    /**
     * Sets the rotation target angle for this action.
     *
     * Changing this value for an actively animating action can have undefined
     * side effects.
     *
     * @param angle    The rotation target angle for this action.
     */
    void setAngle(float angle) { _angle = angle; }
    
    /**
     * Returns an action attaching this animation to the given scene node
     *
     * This action will reference this object during the animation. Any changes
     * to this object during that time may alter the animation.
     *
     * Note that the action has no associated duration. That should be set when
     * it is added to {@link ActionTimeline}.
     *
     * @param node  The node to attach
     *
     * @return an action attaching this animation to the given scene node
     */
    ActionFunction attach(const std::shared_ptr<scene2::SceneNode>& node);
    
};
    
#pragma mark -
#pragma mark ScaleBy

/**
 * This factory creates an action scaling by a given factor.
 *
 * This class is actually a factory for creating movement actions. To create
 * an action, call {@link #attach} with the appropriate {@link SceneNode}.
 * Note that this class contains no duration information. That is supplied
 * when the action is added to {@link ActionTimeline}.
 */
class ScaleBy : public Action {
private:
    /** The scaling factor */
    Vec2 _delta;

public:
#pragma mark Constructors
    /**
     * Creates an uninitialized scaling animation.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    ScaleBy() { _delta = Vec2::ONE; }

    /**
     * Deletes this animation, disposing all resources
     */
    ~ScaleBy() { dispose(); }
    
    /**
     * Disposes all of the resources used by this animation.
     *
     * A disposed animation can be safely reinitialized.
     */
    void dispose() { _delta = Vec2::ONE; }
    
    /**
     * Initializes a degenerate scaling animation.
     *
     * The scale amount is set to (1.0, 1.0), meaning no adjustment takes place.
     *
     * @return true if initialization was successful.
     */
    bool init() {
        return init(Vec2::ONE);
    }

    /**
     * Initializes a scaling animation for the given factor
     *
     * When animated, this action will adjust the scale of the node so that it
     * is multiplied by the given factor.
     *
     * @param factor    The amount to scale the attached node
     *
     * @return true if initialization was successful.
     */
    bool init(const float factor) {
        return init(Vec2(factor,factor));
    }
    
    /**
     * Initializes a scaling animation for the given factor
     *
     * When animated, this action will adjust the scale of the node so that it
     * is multiplied by the given factor.
     *
     * @param factor    The amount to scale the attached node
     *
     * @return true if initialization was successful.
     */
    bool init(const Vec2 factor);
    
    /**
     * Returns a newly allocated degenerate scaling action.
     *
     * The scale amount is set to (1.0, 1.0), meaning no adjustment takes place.
     *
     * @return a newly allocated degenerate scaling action.
     */
    static std::shared_ptr<ScaleBy> alloc() {
        std::shared_ptr<ScaleBy> result = std::make_shared<ScaleBy>();
        return (result->init() ? result : nullptr);
    }

    /**
     * Returns a newly allocated scaling animation for the given factor
     *
     * When animated, this action will adjust the scale of the node so that it
     * is multiplied by the given factor.
     *
     * @param factor    The amount to scale the target node
     *
     * @return a newly allocated scaling animation for the given factor
     */
    static std::shared_ptr<ScaleBy> alloc(float factor) {
        std::shared_ptr<ScaleBy> result = std::make_shared<ScaleBy>();
        return (result->init(factor) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated scaling animation for the given factor
     *
     * When animated, this action will adjust the scale of the node so that it
     * is multiplied by the given factor.
     *
     * @param factor    The amount to scale the target node
     *
     * @return a newly allocated scaling animation for the given factor
     */
    static std::shared_ptr<ScaleBy> alloc(const Vec2 factor) {
        std::shared_ptr<ScaleBy> result = std::make_shared<ScaleBy>();
        return (result->init(factor) ? result : nullptr);
    }
    
    /**
     * Returns the scaling factor for this action.
     *
     * Changing this value for an actively animating action can have
     * undefined effects.
     *
     * @return the scaling factor for this action.
     */
    const Vec2& getFactor() const { return _delta; }
    
    /**
     * Sets the scaling factor for this action.
     *
     * Changing this value for an actively animating action can have
     * undefined effects.
     *
     * @param factor the scaling factor for this action.
     */
    void setFactor(const Vec2 factor) { _delta = factor; }
    
    /**
     * Returns an action attaching this animation to the given scene node
     *
     * This action will reference this object during the animation. Any changes
     * to this object during that time may alter the animation.
     *
     * Note that the action has no associated duration. That should be set when
     * it is added to {@link ActionTimeline}.
     *
     * @param node  The node to attach
     *
     * @return an action attaching this animation to the given scene node
     */
    ActionFunction attach(const std::shared_ptr<scene2::SceneNode>& node);
    
};

    
#pragma mark -
/**
 * This factor creates an action scaling towards a fixed magnification
 *
 * This class is actually a factory for creating movement actions. To create
 * an action, call {@link #attach} with the appropriate {@link SceneNode}.
 * Note that this class contains no duration information. That is supplied
 * when the action is added to {@link ActionTimeline}.
 */
class ScaleTo : public Action {
protected:
    /** The target scaling factor at the end of the animation */
    Vec2 _target;

public:
#pragma mark Constructors
    /**
     * Creates an uninitialized scaling animation.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    ScaleTo() { _target = Vec2::ONE; }

    /**
     * Deletes this animation, disposing all resources
     */
    ~ScaleTo() { dispose(); }
    
    /**
     * Disposes all of the resources used by this animation.
     *
     * A disposed animation can be safely reinitialized.
     */
    void dispose() { _target = Vec2::ONE; }
    
    /**
     * Initializes a scaling action returning the node to normal size
     *
     * @return true if initialization was successful.
     */
    bool init() {
        return init(Vec2::ONE);
    }

    /**
     * Initializes a scaling action towards the given scale amount
     *
     * @param scale The target scaling amount
     *
     * @return true if initialization was successful.
     */
    bool init(float scale) {
        return init(Vec2(scale,scale));
    }

    
    /**
     * Initializes a scaling action towards the given scale amount
     *
     * @param scale The target scaling amount
     *
     * @return true if initialization was successful.
     */
    bool init(const Vec2 scale);

    /**
     * Returns a newly allocated scaling action returning the node to normal size
     *
     * @return a newly allocated scaling action returning the node to normal size
     */
    static std::shared_ptr<ScaleTo> alloc() {
        std::shared_ptr<ScaleTo> result = std::make_shared<ScaleTo>();
        return (result->init() ? result : nullptr);
    }

    /**
     * Returns a newly allocated scaling action towards the given scale amount
     *
     * @param scale The target scaling amount
     *
     * @return a newly allocated scaling action towards the given scale amount
     */
    static std::shared_ptr<ScaleTo> alloc(float scale) {
        std::shared_ptr<ScaleTo> result = std::make_shared<ScaleTo>();
        return (result->init(scale) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated scaling action towards the given scale amount
     *
     * @param scale The target scaling amount
     *
     * @return a newly allocated scaling action towards the given scale amount
     */
    static std::shared_ptr<ScaleTo> alloc(const Vec2 scale) {
        std::shared_ptr<ScaleTo> result = std::make_shared<ScaleTo>();
        return (result->init(scale) ? result : nullptr);
    }

    /**
     * Returns the movement delta for this action.
     *
     * Changing this value for an actively animating action can have
     * undefined effects.
     *
     * @return the movement delta for this action.
     */
    const Vec2& getScale() const { return _target; }
    
    /**
     * Sets the movement delta for this action.
     *
     * Changing this value for an actively animating action can have
     * undefined effects.
     *
     * @param scale    The scale amount for this action.
     */
    void setScale(const Vec2 scale) { _target = scale; }
    
    /**
     * Returns an action attaching this animation to the given scene node
     *
     * This action will reference this object during the animation. Any changes
     * to this object during that time may alter the animation.
     *
     * Note that the action has no associated duration. That should be set when
     * it is added to {@link ActionTimeline}.
     *
     * @param node  The node to attach
     *
     * @return an action attaching this animation to the given scene node
     */
    ActionFunction attach(const std::shared_ptr<scene2::SceneNode>& node);
    
};
    
#pragma mark -
#pragma mark FadeBy
/**
 * This factory creates a fade-in/out animation by a certain factor.
 *
 * Transparency is defined by the alpha value of the node color. Altering this
 * value affects the visibility of the node. Unless the node is set for its
 * children to inherit is color, this has no affect on the children of the node.
 *
 * This animation multiplies the existing alpha of a node by a factor at the
 * start of the animation. It then adjusts the alpha of the node until it
 * reaches that value. A value greater than 1 increaes the node towards opacity
 * while a factor less than 1 decreases it towards opacity. Note that a node
 * that has an alpha of 0 (completely transparent) is unaffected by this
 * animation.
 *
 * This class is actually a factory for creating movement actions. To create
 * an action, call {@link #attach} with the appropriate {@link SceneNode}.
 * Note that this class contains no duration information. That is supplied
 * when the action is added to {@link ActionTimeline}.
 */
class FadeBy {
private:
    /** The fade-in/out factor */
    float _factor;
    
public:
    /**
     * Creates an uninitialized fade-in/out animation
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    FadeBy() {}
    
    /**
     * Deletes this animation, disposing all resources
     */
    ~FadeBy() { dispose(); }
    
    /**
     * Disposes all of the resources used by this animation.
     *
     * A disposed action can be safely reinitialized.
     */
    void dispose() {}
    
    /**
     * Initializes a degenerate fade-in/out animation.
     *
     * This animation has a factor of 1, which means there is no change.
     *
     * @return true if initialization was successful.
     */
    bool init() {
        return init(1);
    }
    
    /**
     * Initializes a fade-in/out animation for the given factor.
     *
     * This animation multiplies the existing alpha of a node by the factor at
     * the start of the animation. It then adjusts the alpha of the node until
     * it reaches that value. A value greater than 1 increaes the node towards
     * opacity while a factor less than 1 decreases it towards opacity. Note
     * that a node that has an alpha of 0 (completely transparent) is unaffected
     * by this animation.
     *
     * The factor value should be >= 0, where 0 fades an object out to total
     * transparency.
     *
     * @param factor    The fade-in/out factor
     *
     * @return true if initialization was successful.
     */
    bool init(float factor);

    /**
     * Returns a newly allocated degenerate fade-in/out animation.
     *
     * This animation has a factor of 1, which means there is no change.
     *
     * @return a newly allocated degenerate fade-in/out animation.
     */
    static std::shared_ptr<FadeBy> alloc() {
        std::shared_ptr<FadeBy> result = std::make_shared<FadeBy>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated fade-in/out animation for the given factor.
     *
     * This animation multiplies the existing alpha of a node by the factor at
     * the start of the animation. It then adjusts the alpha of the node until
     * it reaches that value. A value greater than 1 increaes the node towards
     * opacity while a factor less than 1 decreases it towards opacity. Note
     * that a node that has an alpha of 0 (completely transparent) is unaffected
     * by this animation.
     *
     * The factor value should be >= 0, where 0 fades an object out to total
     * transparency.
     *
     * @param factor    The fade-in/out factor
     *
     * @return a newly allocated fade-in/out animation for the given factor.
     */
    static std::shared_ptr<FadeBy> alloc(float factor) {
        std::shared_ptr<FadeBy> result = std::make_shared<FadeBy>();
        return (result->init(factor) ? result : nullptr);
    }
    
    /**
     * Returns the fade-in/out factor for this animation.
     *
     * This animation multiplies the existing alpha of a node by the factor at
     * the start of the animation. It then adjusts the alpha of the node until
     * it reaches that value. A value greater than 1 increaes the node towards
     * opacity while a factor less than 1 decreases it towards opacity. Note
     * that a node that has an alpha of 0 (completely transparent) is unaffected
     * by this animation.
     *
     * Changing this value for an actively animating action can have undefined
     * side effects.
     *
     * @return the fade-in/out factor for this animation.
     */
    float getFactor() const { return _factor; }
    
    /**
     * Sets the fade-in/out factor for this animation.
     *
     * This animation multiplies the existing alpha of a node by the factor at
     * the start of the animation. It then adjusts the alpha of the node until
     * it reaches that value. A value greater than 1 increaes the node towards
     * opacity while a factor less than 1 decreases it towards opacity. Note
     * that a node that has an alpha of 0 (completely transparent) is unaffected
     * by this animation.
     *
     * Changing this value for an actively animating action can have undefined
     * side effects.
     *
     * @param factor    The fade-in/out factor for this animation.
     */
    void setFactor(float factor) { _factor = factor; }

    /**
     * Returns an action attaching this animation to the given scene node
     *
     * This action will reference this object during the animation. Any changes
     * to this object during that time may alter the animation.
     *
     * Note that the action has no associated duration. That should be set when
     * it is added to {@link ActionTimeline}.
     *
     * @param node  The node to attach
     *
     * @return an action attaching this animation to the given scene node
     */
    ActionFunction attach(const std::shared_ptr<scene2::SceneNode>& node);
    
};

    
#pragma mark -
/**
 * This factory creates a fade-in/out animation towards a specific opacity.
 *
 * When applied to a node, this action will adjust the alpha value of the node
 * color until it reaches the target value (which should be between 0 and 1).
 * Unless the node is set for its children to inherit is color, this will have
 * no affect on the children of the node.
 *
 * This class is actually a factory for creating movement actions. To create
 * an action, call {@link #attach} with the appropriate {@link SceneNode}.
 * Note that this class contains no duration information. That is supplied
 * when the action is added to {@link ActionTimeline}.
 */
class FadeTo {
private:
    /** The opacity target */
    float _target;

public:
#pragma mark Constructors
    /**
     * Creates an uninitialized fade-in/out animation.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    FadeTo() {}
    
    /**
     * Deletes this animation, disposing all resources
     */
    ~FadeTo() { dispose(); }
    
    /**
     * Disposes all of the resources used by this animation.
     *
     * A disposed action can be safely reinitialized.
     */
    void dispose() {}
    
    /**
     * Initializes a fade animation towards total transparency
     *
     * When applied to a node, this action will adjust the alpha value of the
     * node color until it is eventually 0.  Unless the node is set for its
     * children to inherit its color, this will have no affect on the children
     * of the node.
     *
     * @return true if initialization was successful.
     */
    bool initOut() {
        return init(0);
    }
    
    /**
     * Initializes a fade animation towards total opacity
     *
     * When applied to a node, this action will adjust the alpha value of the
     * node color until it is eventually 1.  Unless the node is set for its
     * children to inherit its color, this will have no affect on the children
     * of the node.
     *
     * @return true if initialization was successful.
     */
    bool initIn() {
        return init(1);
    }
    
    /**
     * Initializes a fade animation towards a target opacity
     *
     * When applied to a node, this action will adjust the alpha value of the
     * node color until it is eventually the target value (which should be
     * between 0 and 1).  Unless the node is set for its children to inherit
     * its color, this will have no affect on the children of the node.
     *
     * @param target    The target opacity
     *
     * @return true if initialization was successful.
     */
    bool init(float target);

    /**
     * Returns a newly allocated fade animation towards total transparency
     *
     * When applied to a node, this action will adjust the alpha value of the
     * node color until it is eventually 0.  Unless the node is set for its
     * children to inherit its color, this will have no affect on the children
     * of the node.
     *
     * @return a newly allocated fade animation towards total transparency
     */
    static std::shared_ptr<FadeTo> allocOut() {
        std::shared_ptr<FadeTo> result = std::make_shared<FadeTo>();
        return (result->initOut() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated fade animation towards total opacity
     *
     * When applied to a node, this action will adjust the alpha value of the
     * node color until it is eventually 1.  Unless the node is set for its
     * children to inherit its color, this will have no affect on the children
     * of the node.
     *
     * @return a newly allocated fade animation towards total opacity
     */
    static std::shared_ptr<FadeTo> allocIn() {
        std::shared_ptr<FadeTo> result = std::make_shared<FadeTo>();
        return (result->initIn() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated fade animation towards a target opacity
     *
     * When applied to a node, this action will adjust the alpha value of the
     * node color until it is eventually the target value (which should be
     * between 0 and 1).  Unless the node is set for its children to inherit
     * its color, this will have no affect on the children of the node.
     *
     * @param target    The target opacity
     *
     * @return a newly allocated fade animation towards a target opacity
     */
    static std::shared_ptr<FadeTo> alloc(float target) {
        std::shared_ptr<FadeTo> result = std::make_shared<FadeTo>();
        return (result->init(target) ? result : nullptr);
    }
    
    /**
     * Returns an action attaching this animation to the given scene node
     *
     * This action will reference this object during the animation. Any changes
     * to this object during that time may alter the animation.
     *
     * Note that the action has no associated duration. That should be set when
     * it is added to {@link ActionTimeline}.
     *
     * @param node  The node to attach
     *
     * @return an action attaching this animation to the given scene node
     */
    ActionFunction attach(const std::shared_ptr<scene2::SceneNode>& node);
    
};
    }
}
#endif /* __CU_TRANSFORM_ACTION_H__ */
