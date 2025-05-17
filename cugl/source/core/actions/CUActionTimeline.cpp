//
//  CUActionTimeline.cpp
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
#include <cugl/core/actions/CUActionTimeline.h>
#include <vector>


#pragma mark ActionInstance
namespace cugl {
    
/**
 * This class is a wrapper for managing listener functions
 */
class ActionListenerWrapper {
public:
    ActionListener listener;
    double time;
    Uint32 key;

    ActionListenerWrapper(const ActionListener func, double t) {
        listener = func;
        time = t;
        key = 0;
    }

    ActionListenerWrapper(const ActionListener func, double t, Uint32 k) {
        listener = func;
        time = t;
        key = k;
    }
    
    bool operator<(const ActionListenerWrapper& rhs) {
        return time > rhs.time;
    }
    
};

/**
 * This is an opaque class representing an action being actively animated.
 *
 * The instance contains the state of an action including duration, elapsed
 * time, the interpolation function, and any other necessary state. This class
 * is only meant to be used by {@link ActionTimeline}, and not directly by the
 * user.
 */
class ActionInstance {
public:
    /** The key for this instance  */
    std::string key;
    
    /** The interpolation function for this action */
    ActionFunction action;
    
    /** The easing function to allow non-linear behavior */
    EasingFunction easing;
    
    ActionState state;
    
    /* The desired completion time of the action */
    double duration;
    
    /** The execution time since initialization */
    double elapsed;
    
    /** Whether or not this instance is currently paused */
    bool  paused;
    
    /** The listeners ordered in reverse call order*/
    std::vector<std::shared_ptr<ActionListenerWrapper>> listeners;
    
    /** A weak pointer to our parent */
    ActionTimeline* parent;
    
public:
    /**
     * Creates a new degenerate ActionInstance on the stack.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    ActionInstance() : duration(0.0f), elapsed(0.0f), paused(false) {
        state = ActionState::BEGIN;
    }
    
    /**
     * Deletes this action instance, disposing all resources
     */
    ~ActionInstance() { }
    
    void start() {
        action(elapsed,ActionState::BEGIN);
        state = ActionState::UPDATE;
    }
    
    void update(float dt) {
        if (paused || state == ActionState::FINISH) {
            return;
        }
        
        float normtime = 1.0;
        if (duration > 0) {
            normtime = (elapsed+dt) / duration;
            // Clamp to end
            if (normtime > 1.0f) {
                normtime = 1.0f;
            }
        } else {
            normtime = 0.0f;
        }
        
        if (easing) {
            normtime = easing(normtime);
        }
        
        if (state == ActionState::BEGIN) {
            start();
        }
        
        action(normtime,ActionState::UPDATE);

        if (elapsed+dt >= duration) {
            state = ActionState::FINISH;
            action(elapsed+dt,ActionState::FINISH);
        }
        
        // Update the time and invoke the listeners
        elapsed += dt;
        while (!listeners.empty() && listeners.back()->time <= elapsed) {
            auto wrapper = listeners.back();
            wrapper->listener(key,wrapper->time,elapsed);
            parent->_listeners.erase(wrapper->key);
            listeners.pop_back();
        }
    }
    
    void stop() {
        // Only call listeners registered for completion
        while (!listeners.empty()) {
            auto wrapper = listeners.back();
            if (wrapper->time >= duration) {
                wrapper->listener(key,wrapper->time,elapsed);
            }
            parent->_listeners.erase(wrapper->key);
            listeners.pop_back();
        }
    }
};

// Keep this from being a mouthful
typedef std::unordered_map<std::string,std::shared_ptr<ActionInstance>> InstanceMap;

}


using namespace cugl;

/**
 * Disposes all of the resources used by this timeline.
 *
 * A disposed action manager can be safely reinitialized. Any animations
 * owned by this action manager will immediately stop and be released.
 */
void ActionTimeline::dispose() {
    _listeners.clear();
    _actions.clear();
}



#pragma mark -
#pragma mark Action Management
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
bool ActionTimeline::add(const std::string key, ActionFunction action,
                         float duration, EasingFunction easing) {
    auto item = _actions.find(key);
    if (item != _actions.end()) {
        return false;
    } else if (action == nullptr) {
        return false;
    }
    
    auto instance = std::make_shared<ActionInstance>();
    instance->key = key;
    instance->action = action;
    instance->easing = easing;
    instance->duration = duration;
    instance->parent = this;
    _actions.emplace(key,instance);
    return true;
}

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
bool ActionTimeline::add(const std::string key, const std::shared_ptr<Action>& action,
                         float duration, EasingFunction easing) {
    auto item = _actions.find(key);
    if (item != _actions.end()) {
        return false;
    } else if (action == nullptr) {
        return false;
    }
    
    auto func = [=](float t, ActionState state) {
        if (state == ActionState::BEGIN) {
            action->start(t);
        } else if (state == ActionState::FINISH) {
            action->stop(t);
        } else {
            action->set(t);
        }
    };
    
    auto instance = std::make_shared<ActionInstance>();
    instance->key = key;
    instance->action = func;
    instance->easing = easing;
    instance->duration = duration;
    instance->parent = this;
    _actions.emplace(key,instance);
    return true;
}

/**
 * Removes the animation for the given key.
 *
 * This act will immediately stop the animation. The animated node will
 * continue to have whatever state it had when the animation stopped.
 *
 * If there is no animation for the give key (e.g. the animation is complete)
 * this method will return false.
 *
 * @param key       The identifying key
 *
 * @return true if the animation was successfully removed
 */
bool ActionTimeline::remove(std::string key) {
    auto action = _actions.find(key);
    if (action == _actions.end()) {
        return false;
    }
    auto instance = action->second;
    instance->stop();
    
    action->second = nullptr;
    _actions.erase(action);
    return true;
}

/**
 * Updates all non-paused animations by dt seconds
 *
 * Each animation is moved forward by dt second. If this causes an animation
 * to reach its duration, the animation is removed and the key is once
 * again available.
 *
 * @param dt    The number of seconds to animate
 */
void ActionTimeline::update(float dt) {
    auto completed = std::vector<InstanceMap::iterator>();
    for(auto it = _actions.begin(); it != _actions.end(); ++it) {
        auto instance = it->second;
        instance->update(dt);
        if (instance->state == ActionState::FINISH) {
            completed.push_back(it);
        }
    }
    
    for (auto it = completed.begin(); it != completed.end(); ++it) {
        auto instance = (*it)->second;
        instance->stop();
        (*it)->second = nullptr;
        _actions.erase(*it);
    }
}

/**
 * Returns true if the given key represents an active animation
 *
 * @param key       The identifying key
 *
 * @return true if the given key represents an active animation
 */
bool ActionTimeline::isActive(std::string key) const {
    auto action = _actions.find(key);
    return (action != _actions.end());
}

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
Uint32 ActionTimeline::addListener(const std::string key, float time, ActionListener listener) {
    auto jt = _actions.find(key);
    if (jt == _actions.end()) {
        return 0;
    }
    auto instance = jt->second;
    
    Uint32 lkey = nextKey++;
    auto wrapper = std::make_shared<ActionListenerWrapper>(listener, time, lkey);

    bool placed = false;
    for(auto it = instance->listeners.begin(); !placed && it != instance->listeners.end(); ++it ) {
        if ((*it)->time <= wrapper->time) {
            instance->listeners.insert(it,wrapper);
            placed = true;
        }
    }
    
    if (!placed) {
        instance->listeners.push_back(wrapper);
    }

    _listeners[lkey] = instance;
    return lkey;
}
        
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
Uint32 ActionTimeline::addCompletionListener(const std::string key, ActionListener listener) {
    auto jt = _actions.find(key);
    if (jt == _actions.end()) {
        return 0;
    }
    auto instance = jt->second;
    
    Uint32 lkey = nextKey++;
    auto wrapper = std::make_shared<ActionListenerWrapper>(listener, instance->duration, lkey);
    
    bool placed = false;
    for(auto it = instance->listeners.begin(); !placed && it != instance->listeners.end(); ++it ) {
        if ((*it)->time <= wrapper->time) {
            instance->listeners.insert(it,wrapper);
            placed = true;
        }
    }
    
    if (!placed) {
        instance->listeners.push_back(wrapper);
    }
    
    _listeners[lkey] = instance;
    return lkey;
}

/**
 * Returns the action listener for the given key
 *
 * If there is no listener for the given key, it returns nullptr.
 *
 * @param key   The identifier for the listener
 *
 * @return the action listener for the given key
 */
const ActionListener ActionTimeline::getListener(Uint32 key) const {
    auto jt = _listeners.find(key);
    if (jt == _listeners.end()) {
        return 0;
    }
    auto instance = jt->second;
    
    std::shared_ptr<ActionListenerWrapper> wrapper = nullptr;
    for(auto it = instance->listeners.begin(); wrapper == nullptr && it != instance->listeners.end(); ++it) {
        if ((*it)->key == key) {
            wrapper = *it;
        }
    }
    
    return wrapper == nullptr ? nullptr : wrapper->listener;
}

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
bool ActionTimeline::removeListener(Uint32 key) {
    auto jt = _listeners.find(key);
    if (jt == _listeners.end()) {
        return false;
    }
    auto instance = jt->second;
    
    std::shared_ptr<ActionListenerWrapper> wrapper = nullptr;
    for(auto it = instance->listeners.begin(); wrapper == nullptr && it != instance->listeners.end(); ++it) {
        if ((*it)->key == key) {
            wrapper = *it;
            instance->listeners.erase(it);
        }
    }
    
    _listeners.erase(key);
    return true;
}

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
float ActionTimeline::getElapsed(const std::string key) const {
    auto action = _actions.find(key);
    if (action == _actions.end()) {
        return false;
    }
    return action->second->elapsed;
}

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
bool ActionTimeline::isPaused(std::string key) {
    auto action = _actions.find(key);
    if (action == _actions.end()) {
        return false;
    }
    return action->second->paused;
}

/**
 * Pauses the animation for the given key.
 *
 * If there is no active animation for the given key, or if it is already
 * paused, this method does nothing.
 *
 * @param key       The identifying key
 */
void ActionTimeline::pause(std::string key) {
    auto action = _actions.find(key);
    if (action == _actions.end()) {
        return;
    }
    action->second->paused = true;
}

/**
 * Unpauses the animation for the given key.
 *
 * If there is no active animation for the given key, or if it is not
 * currently paused, this method does nothing.
 *
 * @param key       The identifying key
 */
void ActionTimeline::unpause(std::string key) {
    auto action = _actions.find(key);
    if (action == _actions.end()) {
        return;
    }
    action->second->paused = false;
}


