//
//  CUAction.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for animation actions. Typically actions are
//  key-framed operations on scene graph nodes. They take an start state and an
//  end state and linearly interpolate them over a period of time. Examples of
//  such operations include Move, Scale, Rotate, Fade, and Animate.
//
//  However, since CUGL 3.0, we have generalized this concept to support
//  anything that can be interpolated. Hence actions can be applied directly
//  to the rendering pipeline and do not need a scene graph.
//
//  As this module is composed of only a function type and an abstract class,
//  we do not use our shared-pointer architecture here.
//
//  CUGL MIT License:
//      This software is provided 'as-is', without any express or implied
//      warranty. In no event will the authors be held liable for any damages
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
#ifndef __CU_ACTION_H__
#define __CU_ACTION_H__
#include <memory>
#include <functional>

namespace cugl {
    
/**
 * This class represents the state of an action.
 *
 * Actions have a beginning, a middle, and end. Because of easing functions, it
 * is never possible to tell which part we are in from the time alone. Therefore
 * we use this enum to track this state.
 */
enum class ActionState : int {
    /** The start of this action (before the first update) */
    BEGIN,
    /** The middle of this action (currently updating) */
    UPDATE,
    /** The end of this action (after the last update) */
    FINISH
};
    
/**
 * @typedef ActionFunction
 *
 * This type represents an action to interpolate.
 *
 * Actions can be represented as any function on the range [0,1], where 0 is
 * the beginning of the action and 1 is the end of the action (e.g. time is
 * normalized). The effect of this function on game state is user-defined.
 *
 * For easing purposes, the interpolation function should support inputs 
 * outside the range [0,1], even if that just means clamping the input. In
 * addition, the function should make no assumptions about monotonicity of
 * the function calls; the value t can jump about unexpectedly.
 *
 * Because of this, it can be hard to tell when the animation has just started
 * and when it has ended. That is the purpose of the state attribute.
 *
 * The function type is equivalent to
 *
 *      std::function<void(float t, ActionState state)>
 *
 * @param t     The interpolation value [0,1]
 * @param state The action state
 */
typedef std::function<void(float t, ActionState state)> ActionFunction;
    
/**
 * This is a base class for definining animation actions.
 *
 * For the most part,the type {@link ActionFunction} is sufficient for
 * representing actions. However, for users that need the action to hold
 * intermediate state, it may be easier to represent the action as an object.
 * That is the purpose of this class.
 *
 * To create an animation using this class, subclass it and implement the three
 * methods {@link #start}, {@link #stop}, and {@link #set}. These correspond
 * to the three {@link ActionState} values that an action can be in.
 *
 * All actions occur during a normalized time [0,1]. However, for the purposes
 * of easing, these methods should be able to support values outside of that
 * range.
 */
class Action {
public:
    /**
     * Creates an uninitialized action.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    Action() {}
    
    /**
     * Deletes this action, disposing all resources
     */
    ~Action() {}

    /**
     * Prepares a target for action
     *
     * This method should initialize any state necessary for the animation
     * action. The value t represents the (normalized) start time of the action,
     * which is typically 0.
     *
     * @param t     The start time
     */
    virtual void start(float t) {}

    /**
     * Cleans up a target after an action
     *
     * This method clearn any state that was used in the animation action. The 
     * value t represents the (normalized) completion time of the action, which
     * is typically 1.
     *
     * @param t     The completion time
     */
    virtual void stop(float t) {}

    /**
     * Sets the time of this action to t.
     *
     * All actions occur during a normalized time [0,1]. However, for the 
     * purposes of easing, this method should be able to support values outside
     * of that range.
     *
     * @param t     The current time of this action
     */
    virtual void set(float t) {}

};
    
}

#endif /* __CU_ACTION_H__ */
