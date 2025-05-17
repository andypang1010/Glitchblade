//
//  CUMoveAction.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides support for the movement actions on 2d scene graphs.
//  Movement can specified as either the end target or the movement amount.
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
#include <cugl/scene2/CUTransformAction2.h>

using namespace cugl;
using namespace cugl::scene2;

#pragma mark AnimationState
/**
 * This is an internal data structure for tracking animation of a float property.
 *
 * This class is essentially a struct, though we refer to it with shared
 * pointers for garbage collection.
 */
class FloatState {
public:
    /** The state origin */
    float origin;
    /** The state target */
    float target;
};

/**
 * This is an internal data structure for tracking animation of Vec2 property.
 *
 * This class is essentially a struct, though we refer to it with shared
 * pointers for garbage collection.
 */
class Vec2State {
public:
    /** The state origin */
    Vec2 origin;
    /** The state target */
    Vec2 target;
};

#pragma mark -
#pragma mark MoveBy

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
bool MoveBy::init(const Vec2 delta) {
    _delta = delta;
    return true;
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
ActionFunction MoveBy::attach(const std::shared_ptr<scene2::SceneNode>& node) {
    // This value keeps track of how we should be moving
    std::shared_ptr<Vec2State> span  = std::make_shared<Vec2State>();
    Vec2 delta = _delta;
    
    ActionFunction func = [=](float t, ActionState state) {
        Vec2 pos;
        switch (state) {
            case ActionState::BEGIN:
                span->origin = node->getPosition();
                span->target = span->origin+delta;
                break;
            case ActionState::UPDATE:
                Vec2::lerp(span->origin,span->target,t,&pos);
                node->setPosition(pos);
                break;
            case ActionState::FINISH:
                node->setPosition(span->target);
                break;
        }
    };
    return func;
}

#pragma mark -
#pragma mark MoveTo

/**
 * Initializes a movement animation towards towards the given position.
 *
 * @param target    The target position
 *
 * @return true if initialization was successful.
 */
bool MoveTo::init(const Vec2 target) {
    _target = target;
    return true;
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
ActionFunction MoveTo::attach(const std::shared_ptr<scene2::SceneNode>& node) {
    // This value keeps track of how we should be moving
    std::shared_ptr<Vec2State> span  = std::make_shared<Vec2State>();
    Vec2 target = _target;

    ActionFunction func = [=](float t, ActionState state) {
        Vec2 pos;
        switch (state) {
            case ActionState::BEGIN:
                span->origin = node->getPosition();
                span->target = target;
                break;
            case ActionState::UPDATE:
                Vec2::lerp(span->origin,span->target,t,&pos);
                node->setPosition(pos);
                break;
            case ActionState::FINISH:
                node->setPosition(span->target);
                break;
        }
    };
    return func;
}

#pragma mark -
#pragma mark RotateBy
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
bool RotateBy::init(float delta) {
    _delta = delta;
    return true;
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
ActionFunction RotateBy::attach(const std::shared_ptr<scene2::SceneNode>& node) {
    // This value keeps track of how we should be moving
    std::shared_ptr<FloatState> span  = std::make_shared<FloatState>();
    float delta = _delta;
    
    ActionFunction func = [=](float t, ActionState state) {
        float theta;
        switch (state) {
            case ActionState::BEGIN:
                span->origin = node->getAngle();
                span->target = span->origin+delta;
                break;
            case ActionState::UPDATE:
                theta = span->origin*(1-t)+span->target*t;
                node->setAngle(theta);
                break;
            case ActionState::FINISH:
                node->setAngle(span->target);
                break;
        }
    };
    return func;
}

#pragma mark -
#pragma mark RotateTo

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
bool RotateTo::init(float angle) {
    _angle = angle;
    return true;
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
ActionFunction RotateTo::attach(const std::shared_ptr<scene2::SceneNode>& node) {
    // This value keeps track of how we should be moving
    std::shared_ptr<FloatState> span  = std::make_shared<FloatState>();
    float angle = _angle;
    
    ActionFunction func = [=](float t, ActionState state) {
        float theta;
        switch (state) {
            case ActionState::BEGIN:
                span->origin = node->getAngle();
                span->target = angle;
                break;
            case ActionState::UPDATE:
                theta = span->origin*(1-t)+span->target*t;
                node->setAngle(theta);
                break;
            case ActionState::FINISH:
                node->setAngle(span->target);
                break;
        }
    };
    return func;
}

#pragma mark -
#pragma mark ScaleBy
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
bool ScaleBy::init(const Vec2 factor) {
    _delta = factor;
    return true;
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
ActionFunction ScaleBy::attach(const std::shared_ptr<scene2::SceneNode>& node) {
    // This value keeps track of how we should be moving
    std::shared_ptr<Vec2State> span  = std::make_shared<Vec2State>();
    Vec2 delta = _delta;
    
    ActionFunction func = [=](float t, ActionState state) {
        Vec2 scale;
        switch (state) {
            case ActionState::BEGIN:
                span->origin = node->getScale();
                span->target = span->origin*delta;
                break;
            case ActionState::UPDATE:
                Vec2::lerp(span->origin,span->target,t,&scale);
                node->setScale(scale);
                break;
            case ActionState::FINISH:
                node->setScale(span->target);
                break;
        }
    };
    return func;
}

#pragma mark -
#pragma mark ScaleTo

/**
 * Initializes a scaling action towards the given scale amount
 *
 * @param scale The target scaling amount
 *
 * @return true if initialization was successful.
 */
bool ScaleTo::init(const Vec2 scale) {
    _target = scale;
    return true;
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
ActionFunction ScaleTo::attach(const std::shared_ptr<scene2::SceneNode>& node) {
    // This value keeps track of how we should be moving
    std::shared_ptr<Vec2State> span  = std::make_shared<Vec2State>();
    Vec2 target = _target;
    
    ActionFunction func = [=](float t, ActionState state) {
        Vec2 scale;
        switch (state) {
            case ActionState::BEGIN:
                span->origin = node->getScale();
                span->target = target;
                break;
            case ActionState::UPDATE:
                Vec2::lerp(span->origin,span->target,t,&scale);
                node->setScale(scale);
                break;
            case ActionState::FINISH:
                node->setScale(span->target);
                break;
        }
    };
    return func;
}

#pragma mark -
#pragma mark FadeBy
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
bool FadeBy::init(float factor) {
    _factor = factor;
    return true;
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
ActionFunction FadeBy::attach(const std::shared_ptr<scene2::SceneNode>& node) {
    // This value keeps track of how we should be moving
    std::shared_ptr<FloatState> span  = std::make_shared<FloatState>();
    float factor = _factor;
    
    ActionFunction func = [=](float t, ActionState state) {
        Vec2 scale;
        Color4f color = node->getColor();
        switch (state) {
            case ActionState::BEGIN:
                span->origin = color.a;
                span->target = span->origin*factor;
                span->target = std::min(std::max(span->target,0.0f),1.0f);
                break;
            case ActionState::UPDATE:
                color.a = span->origin*(1-t)+span->target*t;
                color.a = std::min(std::max(color.a,0.0f),1.0f);
                node->setColor(color);
                break;
            case ActionState::FINISH:
                color.a = span->target;
                node->setColor(color);
                break;
        }
    };
    return func;
}

#pragma mark -
#pragma mark FadeTo
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
bool FadeTo::init(float target) {
    CUAssertLog(target >= 0 && target <= 1, "Opacity %f is not in the range [0,1]",target);
    _target = target;
    return true;
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
ActionFunction FadeTo::attach(const std::shared_ptr<scene2::SceneNode>& node) {
    // This value keeps track of how we should be moving
    std::shared_ptr<FloatState> span  = std::make_shared<FloatState>();
    float target = _target;
    
    ActionFunction func = [=](float t, ActionState state) {
        Vec2 scale;
        Color4f color = node->getColor();
        switch (state) {
            case ActionState::BEGIN:
                span->origin = color.a;
                span->target = target;
                break;
            case ActionState::UPDATE:
                color.a = span->origin*(1-t)+span->target*t;
                color.a = std::min(std::max(color.a,0.0f),1.0f);
                node->setColor(color);
                break;
            case ActionState::FINISH:
                color.a = span->target;
                node->setColor(color);
                break;
        }
    };
    return func;
}
