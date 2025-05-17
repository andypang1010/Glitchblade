//
//  CUAnimateSprite.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for sprite sheet animation. It is typically
//  used for scene graphs, but can be generalized to any class with a setFrame
//  method. The animation is is represented as a sequence of frames. There is
//  no tweening support between animation frames.
//
//  The class in this module is actually a factory for creating actions from
//  supporting classes. This class uses our standard shared-pointer architecture.
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
#ifndef __CU_ANIMATE_SPRITE_H__
#define __CU_ANIMATE_SPRITE_H__
#include <cugl/core/actions/CUAction.h>
#include <vector>

namespace cugl {

/**
 * This factory creates an action animating by spritesheet frames
 *
 * Each frame in the sequence is given a set amount of time to display. The
 * animation will not tween between frames, as it applies to a single scene
 * graph node, and cannot interpolate images.
 *
 * This class is actually a factory for creating movement actions. To create
 * an action, call {@link #attach} to an object with the method `setFrame`.
 * The action will call that method over time to perform the animation. Note
 * that this class contains no duration information. That is supplied when the
 * action is added to {@link ActionTimeline}.
 */
class AnimateSprite : public std::enable_shared_from_this<AnimateSprite> {
protected:
    /** The list of frames to animate */
    std::vector<int> _frameset;
    /** The percentage weight of each frame */
    std::vector<float> _weights;
    /** Whether or not the timestep is uniform */
    bool _uniform;

public:
#pragma mark Constructors
    /**
     * Creates an uninitialized animation.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    AnimateSprite() : _uniform(true) {}

    /**
     * Deletes this animation instance, disposing all resources
     */
    ~AnimateSprite() { dispose(); }
    
    /**
     * Disposes all of the resources used by this animation.
     *
     * A disposed action can be safely reinitialized.
     */
    void dispose();
    
    /**
     * Initializes a degenerate animation.
     *
     * The animation sequence is empty, meaning no animation takes place.
     *
     * @return true if initialization was successful.
     */
    bool init() { return true; }
    
    /**
     * Initializes an animation sequence of frames start to end (inclusive).
     *
     * The animation sequence has start as its first frame and end as its last.
     * Animation will be in frame order, with an equal amount of time spent
     * on each frame. The value start must be less than (or equal to) end, as
     * this action does not know the spritesheet size.
     *
     * The repeat argument is optional. It specifies the number of times to
     * repeat the animation sequence.
     *
     * @param start     The initial frame to animate
     * @param end       The final frame to animate
     * @param repeat    The number of times to repeat the sequence
     *
     * @return true if initialization was successful.
     */
    bool init(int start, int end, int repeat = 1);

    /**
     * Initializes an animation sequence of uniform speed
     *
     * The animation sequence is given by the specified vector. The animation
     * will spend an equal amount of time on each frame.
     *
     * @param frames    The animation sequence
     *
     * @return true if initialization was successful.
     */
    bool init(const std::vector<int>& frames);

    /**
     * Initializes an animation sequence of variable speed
     *
     * The animation sequence is given by the first specified vector. The
     * second vector specifies the weight of each frame. The weights specify
     * the percentage of time spent on each frame. These weights must sum to
     * 1. If they do not, then only the initial segment of weights suming to
     * 1 will be used; the other weights will be zeroed.
     *
     * Both vectors must be the same length. They can be empty.
     *
     * @param frames    The animation sequence
     * @param weights   The percentage of time to spend on each frame
     *
     * @return true if initialization was successful.
     */
    bool init(const std::vector<int>& frames, const std::vector<float>& weights);


#pragma mark Static Constructors
    /**
     * Returns a newly allocated, degenerate animation action.
     *
     * The animation sequence is empty, meaning no animation takes place.
     *
     * @return a newly allocated, degenerate animation action.
     */
    static std::shared_ptr<AnimateSprite> alloc() {
        std::shared_ptr<AnimateSprite> result = std::make_shared<AnimateSprite>();
        return (result->init() ? result : nullptr);
    }

    /**
     * Returns a newly allocated animation sequence of frames start to end (inclusive).
     *
     * The animation sequence has start as its first frame and end as its last.
     * Animation will be in frame order, with an equal amount of time spent
     * on each frame. The value start must be less than (or equal to) end, as
     * this action does not know the spritesheet size.
     *
     * The repeat argument is optional. It specifies the number of times to
     * repeat the animation sequence.
     *
     * @param start     The initial frame to animate
     * @param end       The final frame to animate
     * @param repeat    The number of times to repeat the sequence
     *
     * @return a newly allocated animation sequence of frames start to end (inclusive).
     */
    static std::shared_ptr<AnimateSprite> alloc(int start, int end, int repeat = 1) {
        std::shared_ptr<AnimateSprite> result = std::make_shared<AnimateSprite>();
        return (result->init(start,end,repeat) ? result : nullptr);
    }

    /**
     * Returns a newly allocated animation sequence of uniform speed
     *
     * The animation sequence is given by the specified vector. The animation
     * will spend an equal amount of time on each frame.
     *
     * @param frames    The animation sequence
     *
     * @return a newly allocated animation sequence of uniform speed
     */
    static std::shared_ptr<AnimateSprite> alloc(const std::vector<int>& frames) {
        std::shared_ptr<AnimateSprite> result = std::make_shared<AnimateSprite>();
        return (result->init(frames) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated animation sequence of variable speed
     *
     * The animation sequence is given by the first specified vector. The
     * second vector specifies the weight of each frame. The weights specify
     * the percentage of time spent on each frame. These weights must sum to
     * 1. If they do not, then only the initial segment of weights suming to
     * 1 will be used; the other weights will be zeroed.
     *
     * Both vectors must be the same length. They can be empty.
     *
     * @param frames    The animation sequence
     * @param weights   The percentage of time to spend on each frame
     *
     * @return a newly allocated animation sequence of variable speed
     */
    static std::shared_ptr<AnimateSprite> alloc(const std::vector<int>& frames, 
                                                const std::vector<float>& weights) {
        std::shared_ptr<AnimateSprite> result = std::make_shared<AnimateSprite>();
        return (result->init(frames,weights) ? result : nullptr);
    }
    
#pragma mark Atributes
    /**
     * Returns the frame to be animated at normalized time t in [0,1].
     *
     * This method is used by the animation action to determine the current
     * frame.
     *
     * @return the frame to be animated at normalized time t in [0,1].
     */
    int getFrame(float t) const;
    
    /**
     * Returns the sequence of frames used in this animation
     *
     * Changing this value for an actively animating action can have undefined
     * side effects.
     *
     * @return the sequence of frames used in this animation
     */
    const std::vector<int>& getSequence() const { return _frameset; }
    
    /**
     * Returns individual weights for each frame
     *
     * The weights specify the percentage of time spent on each frame. These
     * weights must sum to 1. If they do not, then only the initial segment of
     * weights suming to 1 will be used; the other weights will be zeroed.
     *
     * If this animation uses a uniform time step for each frame, this set
     * will be empty.
     *
     * Changing this value for an actively animating action can have undefined
     * side effects.
     *
     * @return the sequence of frames used in this animation
     */
    const std::vector<float>& getWeights() const { return _weights; }

    /**
     * Sets the sequence of frames used in this animation
     *
     * If this set has a different size than the one initial set, this setter
     * will keep the overall animation duration, but will revert to a uniform
     * time step.
     *
     * Changing this value for an actively animating action can have undefined
     * side effects.
     *
     * @param frames    the sequence of frames used in this animation
     */
    void setSequence(const std::vector<int>& frames);
    
    /**
     * Sets the sequence of frames used in this animation
     *
     * The animation sequence is given by the first specified vector. The
     * second vector specifies the weight of each frame. The weights specify
     * the percentage of time spent on each frame. These weights must sum to
     * 1. If they do not, then only the initial segment of weights suming to
     * 1 will be used; the other weights will be zeroed.
     *
     * Both vectors must be the same length. They can be empty.
     *
     * Changing this value for an actively animating action can have undefined
     * side effects.
     *
     * @param frames    The animation sequence
     * @param weights   The percentage of time to spend on each frame
     */
    void setSequence(const std::vector<int>& frames, const std::vector<float>& weights);
    
    /**
     * Returns true if this animation uses a uniform time step for all frames
     *
     * Changing this value for an actively animating action can have undefined
     * side effects.
     *
     * @return true if this animation uses a uniform time step for all frames
     */
    int isUniform() const { return _uniform; }

    /**
     * Forces this animation to use a uniform time step for all frames
     *
     * Changing this value for an actively animating action can have undefined
     * side effects.
     */
    void setUniform();

#pragma mark Action Methods
    /**
     * Returns an action attaching this animation to the given object
     *
     * The object class must support the method `setFrame`, which it will call
     * to perform the animation. This action will reference this object during
     * the animation. Any changes to this object during that time may alter the
     * animation.
     *
     * Note that the action has no associated duration. That should be set when
     * it is added to {@link ActionTimeline}.
     *
     * @param obj   The object to attach
     *
     * @return an action attaching this animation to the given scene node
     */
    template <typename T>
    ActionFunction attach(const std::shared_ptr<T>& obj) {
        // This keeps this object from being GCd during the animation
        std::shared_ptr<AnimateSprite> self = shared_from_this();
        ActionFunction func = [=](float t, ActionState state) {
            int frame = 0;
            switch (state) {
                case ActionState::BEGIN:
                    frame = self->getFrame(0.0f);
                    break;
                case ActionState::UPDATE:
                    frame = self->getFrame(t);
                    break;
                case ActionState::FINISH:
                    frame = self->getFrame(1.0f);
                    break;
            }
            obj->setFrame(frame);
        };
        return func;
    }

};

}

#endif /* __CU_ANIMATE_SPRITE_H__ */
