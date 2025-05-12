//
//  CUActionTimeline.h
//  Cornell University Game Library (CUGL)
//
//  This module provides the timeline used to managed active animation actions.
//  Actions executed by call to a simple function that updates the state of
//  the timeline. Actions can be added or removed at any time. We also support
//  callback functions for monitoring the status of various actions.
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
#ifndef __CU_ACTION_TIMELINE_H__
#define __CU_ACTION_TIMELINE_H__
#include <cugl/core/actions/CUAction.h>
#include <cugl/core/actions/CUEasingFunction.h>
#include <unordered_map>
#include <algorithm>
#include <string>
#include <SDL.h>

namespace cugl {

/**
 * This is an opaque class representing an action being actively animated.
 *
 * The instance contains the state of an action including duration, elapsed
 * time, the interpolation function, and any other necessary state. This class
 * is only meant to be used by {@link ActionTimeline}, and not directly by the
 * user.
 */
class ActionInstance;
    
/**
 * @typedef ActionListener
 *
 * This type represents a listener for monitoring an {@link ActionTimeline}
 *
 * In CUGL, listeners are implemented as a set of callback functions, not as
 * objects. This allows each listener to implement as much or as little
 * functionality as it wants. A listener is identified by a key which should
 * be a globally unique unsigned int.
 *
 * Typically {@link ActionTimeline} listeners are used for notifying when an
 * action has completed. But like Unity, we allow for a callback function at
 * any point of the timeline. With that said, frame rate imprecision means that
 * the time at which the callback is executed is not exactly the same a time as
 * when the listener is invoked.
 *
 * Listener times are relative to the duration of the event, and are not
 * normalized in the range [0,1].
 *
 * The function type is equivalent to
 *
 *      std::function<void(const std::string key, float time, float actual)>
 *
 * @param key       The key identifying the action
 * @param time      The requested time of this callback
 * @param actual    The actual time of this callback
 */
typedef std::function<void(const std::string key, float time, float actual)> ActionListener;

/**
 * This class provides an timeline for managing active animations.
 *
 * Because we do not want to force the users to define hash functions for their
 * actions, actions are attached to the timeline via a key. This key allows the
 * user to pause an action or query when it is complete. With each call to
 * {@link #update} frame, this class moves the each action further along the
 * timeline it is complete.
 *
 * The timeline can only have one action for each key. However, it has no way
 * preventing the user from assigning the same action to different keys. This
 * is discouraged as the behavior in this case is undefined.
 *
 * It is possible to assign listeners to each action to monitor its progress.
 * As with the rest of our input listeners, attached listeners are assigned
 * a key when they are attached, which can be used to remove them from the
 * timeline. With that said, listeners are automatically removed when their
 * associated action is complete.
 */
class ActionTimeline {
#pragma mark Values
private:
    /** A map that associates keys with animations */
    std::unordered_map<std::string, std::shared_ptr<ActionInstance>> _actions;
    /** A map that associates listener keys with animations */
    std::unordered_map<Uint32, std::shared_ptr<ActionInstance>> _listeners;
    /** The next available listener key */
    Uint32 nextKey;
    
    friend class ActionInstance;
    
public:
#pragma mark Constructors
    /**
     * Creates a new degenerate timeline on the stack.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    ActionTimeline() : nextKey(1) {}
    
    /**
     * Deletes this timeline, disposing all resources
     */
    ~ActionTimeline() { dispose(); }
    
    /**
     * Disposes all of the resources used by this timeline.
     *
     * A disposed action manager can be safely reinitialized. Any animations
     * owned by this action manager will immediately stop and be released.
     */
    void dispose();
    
    /**
     * Initializes a new timeline.
     *
     * The timeline will have no attached actions and will have an initial
     * {@link #getElapsed} of 0.
     *
     * @return true if initialization was successful.
     */
    bool init() { return true; }
    
    /**
     * Returns a newly allocated timeline.
     *
     * The timeline will have no attached actions and will have an initial
     * {@link #getElapsed} of 0.
     *
     * @return a newly allocated timeline.
     */
    static std::shared_ptr<ActionTimeline> alloc() {
        std::shared_ptr<ActionTimeline> result = std::make_shared<ActionTimeline>();
        return (result->init() ? result : nullptr);
    }

#pragma mark -
#pragma mark Action Management
    /**
     * Adds an action with the given duration.
     *
     * The action will be invoked at the next call to {@link #update}.
     *
     * This method will fail if the provided key is already in use.
     *
     * @param key       The identifying key
     * @param action    The action to animate
     * @param duration  The action duration
     *
     * @return true if the action was successfully added
     */
    bool add(const std::string key, ActionFunction action, float duration) {
        return add(key, action, duration, nullptr);
    };

    /**
     * Adds an action with the given duration.
     *
     * The action will be invoked at the next call to {@link #update}.
     *
     * This method will fail if the provided key is already in use.
     *
     * @param key       The identifying key
     * @param action    The action to animate
     * @param duration  The action duration
     *
     * @return true if the animation was successfully started
     */
    bool add(const std::string key, const std::shared_ptr<Action>& action, float duration) {
        return add(key, action, duration, nullptr);
    };

    /**
     * Adds an action with the given duration and easing function
     *
     * The action will be invoked at the next call to {@link #update}. The
     * easing function allows for effects like bouncing or elasticity in the
     * linear interpolation. If null, the animation will use the standard
     * linear easing.
     *
     * This method will fail if the provided key is already in use.
     *
     * @param key       The identifying key
     * @param action    The action to animate
     * @param duration  The action duration
     * @param easing    The easing (interpolation) function
     *
     * @return true if the animation was successfully started
     */
    bool add(const std::string key, ActionFunction action, 
             float duration, EasingFunction easing);
    
    /**
     * Actives an animation with the given action and easing function
     *
     * The action will be invoked at the next call to {@link #update}. The
     * easing function allows for effects like bouncing or elasticity in the
     * linear interpolation. If null, the animation will use the standard
     * linear easing.
     *
     * This method will fail if the provided key is already in use.
     *
     * @param key       The identifying key
     * @param action    The action to animate
     * @param duration  The action duration
     * @param easing    The easing (interpolation) function
     *
     * @return true if the animation was successfully started
     */
    bool add(const std::string key, const std::shared_ptr<Action>& action,
             float duration, EasingFunction easing);
    
    /**
     * Removes the action for the given key.
     *
     * This method will immediately stop the animation. In particular, it will
     * invoke any listeners waiting on completion.
     *
     * If there is no animation for the give key (e.g. the animation is 
     * complete) this method will return false.
     *
     * @param key       The identifying key
     *
     * @return true if the animation was successfully removed
     */
    bool remove(const std::string key);

    /**
     * Updates all non-paused actions by dt seconds
     *
     * Each action is moved forward by dt seconds. If this causes an action to
     * reach its duration, the action is removed and the key is once again
     * available.
     *
     * @param dt    The number of seconds to animate
     */
    void update(float dt);
    
    /**
     * Returns true if the given key represents an active action
     *
     * Note that paused actions are still active, even though they are paused.
     *
     * @param key       The identifying key
     *
     * @return true if the given key represents an active action
     */
    bool isActive(const std::string key) const;

#pragma mark -
#pragma mark Listeners
    /**
     * Adds a listener for the specified action at the given time.
     *
     * This listener will be invoked when the timeline first passes the given
     * time for the specified object. Due to framerate imprecision, the actual
     * time the listener is invoked may be slightly greater than the time
     * requested.
     *
     * If time is greater than or equal to the duration of action, this listener
     * will be invoked once the action is completed. If it is less than or equal
     * to 0, it will be invoked once the action is started.
     *
     * If there is no action for the given key, this method will return 0,
     * indicating failure.
     *
     * @param key       The key identifying the action
     * @param time      The invocation time
     * @param listener  The listener to add
     *
     * @return a key identifying this listener (or 0 for failure)
     */
    Uint32 addListener(const std::string key, float time, ActionListener listener);

    /**
     * Adds a listener for  action completion.
     *
     * This listener will be invoked when action is completed, just before it
     * is removed from this timeline. This method is the same as calling
     * {@link #addListener} with a time greater than the duration.
     *
     * If there is no action for the given key, this method will return 0,
     * indicating failure.
     *
     * @param key       The key identifying the action
     * @param listener  The listener to add
     *
     * @return a key identifying this listener (or 0 for failure)
     */
    Uint32 addCompletionListener(const std::string key, ActionListener listener);
    
    /**
     * Returns the action listener for the given key
     *
     * If there is no listener for the given key, it returns nullptr.
     *
     * @param key   The identifier for the listener
     *
     * @return the action listener for the given key
     */
    const ActionListener getListener(Uint32 key) const;
    
    /**
     * Removes the action listener for the given key
     *
     * If there is no active listener for the given key, this method fails and
     * returns false.
     *
     * @param key   The identifier for the listener
     *
     * @return true if the listener was succesfully removed
     */
    bool removeListener(Uint32 key);

#pragma mark -
#pragma mark Pausing
    /**
     * Returns the elapsed time of the given action.
     *
     * If there is no animation for the give key (e.g. the animation is complete)
     * this method will return 0.
     *
     * @param key       The identifying key
     *
     * @return the elapsed time of the given action.
     */
    float getElapsed(const std::string key) const;
    
    /**
     * Returns true if the animation for the given key is paused
     *
     * This method will return false if there is no active animation with the
     * given key.
     *
     * @param key       The identifying key
     *
     * @return true if the animation for the given key is paused
     */
    bool isPaused(const std::string key);

    /**
     * Pauses the animation for the given key.
     *
     * If there is no active animation for the given key, or if it is already
     * paused, this method does nothing.
     *
     * @param key       The identifying key
     */
    void pause(const std::string key);

    /**
     * Unpauses the animation for the given key.
     *
     * If there is no active animation for the given key, or if it is not
     * currently paused, this method does nothing.
     *
     * @param key       The identifying key
     */
    void unpause(const std::string key);

};

}
#endif /* __CU_ACTION_TIMELINE_H__ */
