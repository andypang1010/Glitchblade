//
//  CUGameController.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for generic game controllers. Despite the name,
//  this classes are an interface built on top of SDL's Joystick interface, and
//  not its more robust GameController interface. That is because this is still
//  an experimental input device and we need a few more iterations to get this
//  one right.
//
//  This input devices are singletons and should never be allocated directly.
//  They should only be accessed via the Input dispatcher.
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
//  Author: Walker White
//  Version: 7/3/24 (CUGL 3.0 reorganization)
//
#ifndef __CU_GAME_CONTROLLER_H__
#define __CU_GAME_CONTROLLER_H__

#include <cugl/core/input/CUInput.h>
#include <cugl/core/math/CUVec2.h>

namespace cugl {

// Forward declarations
class GameControllerAxisEvent;
class GameControllerDPadEvent;
class GameControllerButtonEvent;

#pragma mark -
#pragma mark GameController
/**
 * This class is a reference to single game controller.
 *
 * This class is built on top of the SDL GameController interface. This allows
 * the developer to use a uniform input independent of the controller input
 * type (i.e. D-input vs Xinput). This API is slightly more restrictive than
 * the SDL joystick interface, in that it does not have support for hats
 * or track balls). Instead, the types of input are limited to the following:
 *
 *  - Axes: Analogue joysticks and triggers
 *  - Buttons: On/off input
 *
 * In this interface, D-Pads are treated as buttons and not hats. However, we
 * do abstract out a D-Pad interface to replicate joystick hat functionality.
 *
 * Note that SDL game controllers also support sensors (accelerometers) and
 * touch pads (such as the PS4 touchpad). None of that is support currently.
 *
 * The advantage of the SDL game controller API is that layout is uniform. All
 * controllers have buttons and axes in the same place. Our API uses the XBox
 * names of buttons (A, B, X, Y) instead of the Playstation names, as they are
 * easier to reference.
 *
 * There should only be one instance of a specific controller at any given
 * time. In addition, controllers can be connected and removed while the
 * application is running. For that reason, this class does not allow you
 * to allocate an game controller object. Instead, you must access each
 * game controller through the {@link GameControllerInput} interface.
 */
class GameController {
public:
    /**
     * An enumeration of the supported buttons.
     *
     * This is the list of all buttons supported by this interface. Note that
     * not all game controllers support all buttons. For example, most game
     * controllers do not support the paddles found on the XBox elite
     * controller. To determine if a button is supported, use the method
     * {@link GameController#hasButton}.
     */
    enum class Button : int {
        /** An invalid button */
        INVALID = -1,
        /** The A/cross button */
        A = 0,
        /** The B/circle button */
        B = 1,
        /** The X/square button */
        X = 2,
        /** The Y/triangle button */
        Y = 3,
        /** The back button */
        BACK = 4,
        /** The menu/guide button */
        GUIDE = 5,
        /** The start button */
        START = 6,
        /** The left-stick press */
        LEFT_STICK = 7,
        /** The right-stick press */
        RIGHT_STICK = 8,
        /** The left shoulder/bumper */
        LEFT_SHOULDER = 9,
        /** The right shoulder/bumper */
        RIGHT_SHOULDER = 10,
        /** The up D-Pad button */
        DPAD_UP = 11,
        /** The down D-Pad button */
        DPAD_DOWN = 12,
        /** The left D-Pad button */
        DPAD_LEFT = 13,
        /** The right D-Pad button */
        DPAD_RIGHT = 14,
        /** A miscellaneous button (X-Box share button, PS5 mike button) */
        MISC = 15,
        /* Xbox Elite paddle P1 (upper left, facing the back) */
        UPPER_LEFT_PADDLE = 16,
        /* Xbox Elite paddle P3 (upper right, facing the back) */
        UPPER_RIGHT_PADDLE = 17,
        /* Xbox Elite paddle P2 (lower left, facing the back) */
        LOWER_LEFT_PADDLE = 18,
        /* Xbox Elite paddle P4 (lower right, facing the back) */
        LOWER_RIGHT_PADDLE = 19,
        /* PS4/PS5 touchpad button (UNSUPPORTED) */
        TOUCHPAD = 20
    };
        
    /**
     * An enumeration of the supported axes.
     *
     * This is the list of all axes supported by this interface. Note that
     * not all game controllers support all axes. For example, classic Nintendo
     * gamepads have no axes at all! To determine if an axis is supported, use
     * the method {@link GameController#hasAxis}.
     */
    enum class Axis : int {
        /** An invalid axis */
        INVALID = -1,
        /** The horizontal component of the left analog joystick */
        LEFT_X = 0,
        /** The vertical component of the left analog joystick */
        LEFT_Y = 1,
        /** The horizontal component of the right analog joystick */
        RIGHT_X = 2,
        /** The vertical component of the right analog joystick */
        RIGHT_Y = 3,
        /** The left trigger */
        TRIGGER_LEFT = 4,
        /** The right trigger */
        TRIGGER_RIGHT = 5
    };

    /**
     * An enumeration of the D-Pad positions.
     *
     * Even though D-Pads are buttons, we allow the user to query the current
     * direction as a function of the (cumulative) pressed buttons. A D-Pad has
     * nine states -- the center and the eight cardinal directions.
     */
    enum class DPad : int {
        /** A D-Pad at rest */
        CENTERED = 0,
        /** A D-Pad pressed upwards to the left */
        LEFT_UP = 1,
        /** A D-Pad pressed entirely upwards */
        UP = 2,
        /** A D-Pad pressed upwards to the right */
        RIGHT_UP = 3,
        /** A D-Pad pressed entirely to the right */
        RIGHT = 4,
        /** A D-Pad pressed downwards to the right */
        RIGHT_DOWN = 5,
        /** A D-Pad pressed entirely downwards */
        DOWN = 6,
        /** A D-Pad pressed downwards to the left */
        LEFT_DOWN = 7,
        /** A D-Pad pressed upwards to the left */
        LEFT = 8
    };

#pragma mark Listener Types
    /**
     * @typedef AxisListener
     *
     * This type represents an axis listener for the {@link GameController} class.
     *
     * In CUGL, listeners are implemented as a set of callback functions, not 
     * as objects. This allows each listener to implement as much or as little
     * functionality as it wants. A listener is identified by a key which should
     * be a globally unique unsigned int.
     *
     * An event is delivered whenever an axis changes its position. See the
     * method {@link GameController#getAxisPosition} for more information.
     *
     * Listeners are guaranteed to be called at the start of an animation frame,
     * before the method {@link Application#update(float) }.
     *
     * While game controller listeners do not traditionally require focus like
     * a keyboard does, we have included that functionality. While only one
     * listener can have focus at a time, all listeners will receive input from
     * the game controller.
     *
     * The function type is equivalent to
     *
     *      std::function<void(const GameControllerAxisEvent event, bool focus)>
     *
     * @param event     The axis event
     * @param focus     Whether the listener currently has focus
     */
    typedef std::function<void(const GameControllerAxisEvent& event, bool focus)> AxisListener;

    /**
     * @typedef DPadListener
     *
     * This type represents a DPad listener for the {@link GameController} class.
     *
     * In CUGL, listeners are implemented as a set of callback functions, not 
     * as objects. This allows each listener to implement as much or as little
     * functionality as it wants. A listener is identified by a key which should
     * be a globally unique unsigned int.
     *
     * A DPad event is delivered whenever a D-Pad button is pressed or released,
     * changing the overall direction of the D-pad. See the method
     * {@link GameController#getDPadPosition} for more information. Note that
     * this listener is ptotentially redundant to the {@link ButtonListener},
     * as that listener will also report D-Pad state (as they are buttons).
     *
     * Listeners are guaranteed to be called at the start of an animation frame,
     * before the method {@link Application#update(float) }.
     *
     * While game controller listeners do not traditionally require focus like
     * a keyboard does, we have included that functionality. While only one
     * listener can have focus at a time, all listeners will receive input from
     * the game controller.
     *
     * The function type is equivalent to
     *
     *      std::function<void(const GameControllerHatEvent event, bool focus)>
     *
     * @param event     The hat event
     * @param focus     Whether the listener currently has focus
     */
    typedef std::function<void(const GameControllerDPadEvent& event, bool focus)> DPadListener;

    /**
     * @typedef ButtonListener
     *
     * This type represents a button listener for the {@link GameController} class.
     *
     * In CUGL, listeners are implemented as a set of callback functions, not 
     * as objects. This allows each listener to implement as much or as little
     * functionality as it wants. A listener is identified by a key which should
     * be a globally unique unsigned int.
     *
     * A button event is delivered whenever a buttons changes state between up
     * and/or down. See the methods {@link GameController#isButtonPressed} and
     * {@link GameController#isButtonReleased} for more information. Note that
     * this listener is ptotentially redundant to the {@link DPadListener}, as
     * that listener will also report D-Pad state.
     *
     * Listeners are guaranteed to be called at the start of an animation frame,
     * before the method {@link Application#update(float) }.
     *
     * While game controller listeners do not traditionally require focus like
     * a keyboard does, we have included that functionality. While only one
     * listener can have focus at a time, all listeners will receive input from
     * the game controller.
     *
     * The function type is equivalent to
     *
     *      std::function<void(const GameControllerButtonEvent event, bool focus)>
     *
     * @param event     The button event
     * @param focus     Whether the listener currently has focus
     */
    typedef std::function<void(const GameControllerButtonEvent& event, bool focus)> ButtonListener;
    
protected:
    /** The SDL game controller reference */
    SDL_GameController* _input;
    /** The joystick UID assigned by GameControllerInput */
    std::string _uid;
    /** The game controller description */
    std::string _name;
    
    /** Whether an axis changed state this animation frame */
    std::vector<bool> _axisState;
    /** Whether a button changed state this animation frame */
    std::vector<bool> _buttonState;
    /** Whether the D-Pad changed state this animation frame */
    bool _dpadState;

    /** The listener with focus */
   	Uint32 _focus;
   	 
    /** The set of listeners called on axis movement */
    std::unordered_map<Uint32, AxisListener> _axisListeners;
    /** The set of listeners called on button state changes */
    std::unordered_map<Uint32, ButtonListener> _buttonListeners;
    /** The set of listeners called on D-Pad movement */
    std::unordered_map<Uint32, DPadListener> _dpadListeners;

#pragma mark Constructors
public:
    /**
     * Creates a degnerate game controller
     *
     * This game controller is not actually attached to any devices. To
     * activate a game controller, use {@link GameControllerInput#open}
     * instead.
     */
    GameController();
    
    /**
     * Deletes this input device, disposing of all resources
     */
    ~GameController() { dispose(); }

private:
    /**
     * Initializes this device, acquiring any necessary resources
     *
     * @param index The device index
     * @param uid   The device uid (assigned by {@link GameControllerInput})
     *
     * @return true if initialization was successful
     */
    bool init(int index, const std::string uid);
    
    /**
     * Unintializes this device, returning it to its default state
     *
     * An uninitialized device may not work without reinitialization.
     */
    void dispose();
    
#pragma mark Input Methods
	/**
     * Cleans up a game controller after it has been disconnected.
     *
     * This method is similar to {@link dispose} except that it is aware that
     * SDL has closed the game controller already.
     */
    void forceClose();
    
    /**
     * Clears the state of this input device, readying it for the next frame.
     *
     * Many devices keep track of what happened "this" frame. This method is
     * necessary to advance the frame.
     */
    void clearState();
    
    /**
     * Returns the internal SDL identifier for this game controller
     *
     * @return the internal SDL identifier for this game controller
     */
    SDL_JoystickID getJoystickID() const;
    
    /**
     * Records that an {@link GameControllerAxisEvent} has occured
     *
     * @param axis  The axis index
     * @param value The axis value in [-1,1]
     * @param stamp The event timestamp
     */
    void reportAxis(Axis axis, float value, const Timestamp& stamp);
        
    /**
     * Records that an {@link GameControllerButtonEvent} has occured
     *
     * @param button    The button index
     * @param down      Whether the button is down
     * @param stamp     The event timestamp
     */
    void reportButton(Button button, bool down, const Timestamp& stamp);
    
    /**
     * Records that an {@link GameControllerDPadEvent} has occured
     *
     * @param stamp The event timestamp
     */
    void reportDPad(const Timestamp& stamp);

    friend class GameControllerInput;

public:
    /**
     * Closes this game controller, releasing all resources.
     *
     * This method not invalidates this game controller, so any shared pointers
     * still referring to this controller are no longer usable. The only way
     * to access the game controller again is to call
     * {@link GameControllerInput#open}.
     *
     * It is often better to call the method {@link GameControllerInput#close}
     * instead of this one.
     */
    void close();

#pragma mark Attributes
    /**
     * Returns the name of this game controller.
     *
     * Names are descriptions provided by the vendor. They are not unique, and
     * it is possible to have multiple devices with the same name connected at
     * once.
     *
     * @return the name of this game controller.
     */
    std::string getName() const { return _name; }

    /**
     * Returns the UID of this game controller.
     *
     * UIDs are unique identifiers assigned by {@link GameControllerInput}.
     *
     * @return the UID of this game controller.
     */
    std::string getUID() const { return _uid; }
    
#pragma mark Haptics
    /**
     * Returns true if this controller supports general rumble effects
     *
     * @return true if this controller supports general rumble effects
     */
    bool hasRumble() const;

    /**
     * Returns true if this controller supports trigger rumble effects
     *
     * @return true if this controller supports trigger rumble effects
     */
    bool hasRumbleTriggers() const;
    
    /**
     * Starts a rumble effect for this controller.
     *
     * @param low_freq  The intensity of the low frequency (left) rumble motor
     * @param high_freq The intensity of the high frequency (right) rumble motor
     * @param duration  The rumble duration in milliseconds
     */
    void applyRumble(Uint16 low_freq, Uint16 high_freq, Uint32 duration);
    
    /**
     * Starts a rumble effect for this controller.
     *
     * @param left      The intensity of the left trigger rumble motor
     * @param right     The intensity of the right trigger rumble motor
     * @param duration  The rumble duration in milliseconds
     */
    void hasRumbleTriggers(Uint16 left, Uint16 right, Uint32 duration) const;

#pragma mark Listener Methods
    /**
     * Requests focus for the given identifier
     *
     * Only an active listener can have focus. This method returns false if
     * the key does not refer to an active listener (of any type). Note that
     * keys may be shared across listeners of different types, but must be
     * unique for each listener type.
     *
     * @param key   The identifier for the focus object
     *
     * @return false if key does not refer to an active listener
     */
    bool requestFocus(Uint32 key);
    
    /**
     * Returns true if key represents a listener object
     *
     * An object is a listener if it is a listener for any of the three actions:
     * axis movement, button press/release, or D-Pad movement
     *
     * @param key   The identifier for the listener
     *
     * @return true if key represents a listener object
     */
    bool isListener(Uint32 key) const;
    

#pragma mark Axis State
    /**
     * Returns true if this game controller supports the specified axis
     *
     * Note that not all game controllers support all axes. In particular, the
     * classic Nintendo controllers have no axes at all.
     *
     * @param axis  The axis to query
     *
     * @return true if this game controller supports the specified axis
     */
    bool hasAxis(Axis axis) const;
    
    /**
     * Returns the current axis position.
     *
     * The default value of any axis is 0. The joysticks all range from -1 to 1
     * (with negative values being left and down). The triggers all range from
     * 0 to 1.
     *
     * Note that the SDL only guarantees that a trigger at rest will be within
     * 0.2 of zero. Most applications implement "dead zones" to ignore values
     * in this range. However, this class does not implement any dead zones;
     * that is the responsibility of the user.
     *
     * If the axis is not supported by this controller, this method will return
     * 0.
     *
     * @param axis  The axis index
     *
     * @return the current axis position.
     */
    float getAxisPosition(Axis axis) const;
    
    /**
     * Returns true if the given axis changed position this frame.
     *
     * @param axis  The axis index
     *
     * @return true if the given axis changed position this frame.
     */
    bool axisDidChange(Axis axis) const;
    
    /**
     * Returns the axis motion listener for the given object key
     *
     * This listener is invoked when an axis changes position.
     *
     * If there is no listener for the given key, it returns nullptr.
     *
     * @param key   The identifier for the listener
     *
     * @return the axis motion listener for the given object key
     */
    const AxisListener getAxisListener(Uint32 key) const;

    /**
     * Adds an axis motion listener for the given object key
     *
     * There can only be one axis listener for a given key (though you may
     * share keys across other listener types). If a listener already exists
     * for the key, the method will fail and return false. You must remove
     * a listener before adding a new one for the same key.
     *
     * This listener is invoked when an axis changes position.
     *
     * @param key       The identifier for the listener
     * @param listener  The listener to add
     *
     * @return true if the listener was succesfully added
     */
    bool addAxisListener(Uint32 key, AxisListener listener);
    
    /**
     * Removes the axis motion listener for the given object key
     *
     * If there is no active listener for the given key, this method fails and
     * returns false.
     *
     * This listener is invoked when an axis changes position.
     *
     * @param key   The identifier for the listener
     *
     * @return true if the listener was succesfully removed
     */
    bool removeAxisListener(Uint32 key);
    
#pragma mark Button State
    /**
     * Returns true if this game controller supports the specified button
     *
     * Note that not all game controllers support all buttons. In the paddles
     * are currently unique to the XBox Elite controller.
     *
     * @param button    The button to query
     *
     * @return true if this game controller supports the specified button
     */
    bool hasButton(Button button) const;
    
    /**
     * Returns true if the given button is currently down.
     *
     * This method does not distinguish presses or releases and will return
     * true the entire duration of a button hold.
     *
     * If the button is not supported by this controller, this method will
     * return false.
     *
     * @param button    The button to query
     *
     * @return true if the given button is currently down.
     */
    bool isButtonDown(Button button) const;

    /**
     * Returns true if the given button was pressed this frame.
     *
     * A press means that the button is down this animation frame, but was not
     * down the previous frame.
     *
     * If the button is not supported by this controller, this method will
     * return false.
     *
     * @param button    The button to query
     *
     * @return true if the given button was pressed this frame.
     */
    bool isButtonPressed(Button button) const;
    
    /**
     * Returns true if the given button was released this frame.
     *
     * A release means that the button is up this animation frame, but was not
     * up the previous frame.
     *
     * If the button is not supported by this controller, this method will
     * return false.
     *
     * @param button    The button to query
     *
     * @return true if the given button was released this frame.
     */
    bool isButtonReleased(Button button) const;
    
    /**
     * Returns the button listener for the given object key
     *
     * This listener is invoked when the button changes state. So it is
     * invoked on a press or a release, but not a hold.
     *
     * If there is no listener for the given key, it returns nullptr.
     *
     * @param key   The identifier for the listener
     *
     * @return the button listener for the given object key
     */
    const ButtonListener getButtonListener(Uint32 key) const;

    /**
     * Adds a button listener for the given object key
     *
     * There can only be one button listener for a given key (though you may
     * share keys across other listener types). If a listener already exists
     * for the key, the method will fail and return false. You must remove
     * a listener before adding a new one for the same key.
     *
     * This listener is invoked when the button changes state. So it is
     * invoked on a press or a release, but not a hold.
     *
     * @param key       The identifier for the listener
     * @param listener  The listener to add
     *
     * @return true if the listener was succesfully added
     */
    bool addButtonListener(Uint32 key, ButtonListener listener);
    
    /**
     * Removes the button listener for the given object key
     *
     * If there is no active listener for the given key, this method fails and
     * returns false.
     *
     * This listener is invoked when the button changes state. So it is
     * invoked on a press or a release, but not a hold.
     *
     * @param key   The identifier for the listener
     *
     * @return true if the listener was succesfully removed
     */
    bool removeButtonListener(Uint32 key);
    
#pragma mark D-Pad State
    /**
     * Returns true if the controller has a directional pad.
     *
     * This method is the same a querying all four D-pad buttons.
     *
     * @return true if the controller has a directional pad.
     */
    bool hasDPad() const;
    
    /**
     * Returns the D-Pad position
     *
     * This method converts the current D-Pad button state into a directional
     * state. This state can be centered (untouched) or one of the eight
     * cardinal directions.
     *
     * If this controller does not have a D-Pad, this method will always
     * return CENTERED.
     *
     * @return the D-Pad position
     */
    DPad getDPadPosition() const;
    
    /**
     * Returns true if the D-Pad changed position this frame.
     *
     * @return true if the D-Pad changed position this frame.
     */
    bool dPadDidChange() const;
    
    /**
     * Returns the D-Pad listener for the given object key
     *
     * This listener is invoked when the D-Pad changes position.
     *
     * If there is no listener for the given key, it returns nullptr.
     *
     * @param key   The identifier for the listener
     *
     * @return the D-Pad listener for the given object key
     */
    const DPadListener getDPadListener(Uint32 key) const;

    /**
     * Adds a D-Pad listener for the given object key
     *
     * There can only be one D-Pad listener for a given key (though you may
     * share keys across other listener types). If a listener already exists
     * for the key, the method will fail and return false. You must remove
     * a listener before adding a new one for the same key.
     *
     * This listener is invoked when the D-Pad changes position.
     *
     * @param key       The identifier for the listener
     * @param listener  The listener to add
     *
     * @return true if the listener was succesfully added
     */
    bool addDPadListener(Uint32 key, DPadListener listener);

    /**
     * Removes the D-Pad listener for the given object key
     *
     * If there is no active listener for the given key, this method fails and
     * returns false.
     *
     * This listener is invoked when the D-Pad changes position.
     *
     * @param key   The identifier for the listener
     *
     * @return true if the listener was succesfully removed
     */
    bool removeDPadListener(Uint32 key);

};

#pragma mark -
#pragma mark Game Controller Events
/**
 * This simple class is a struct to hold information about an axis movement.
 *
 * This event is generated when an axis changes position. Remember that
 * analogue joysticks are composed of two axes, and thuse will have two
 * events associated with them.
 */
class GameControllerAxisEvent {
public:
    /** The time of the input event */
    Timestamp timestamp;
    /** The UID of the relevant device */
    std::string uuid;
    /** The axis reference */
    GameController::Axis axis;
    /** The value in the range [-1,1] */
    float value;
    
    /**
     * Constructs a new axis event with the default values
     */
    GameControllerAxisEvent() : axis(GameController::Axis::INVALID), value(0) {}
    
    /**
     * Constructs a new axis event with the given values
     *
     * @param key   The device UID
     * @param index The axis index
     * @param pos   The axis position in [-1,1]
     * @param stamp The timestamp for the event
     */
    GameControllerAxisEvent(const std::string key, GameController::Axis index,
                            float pos, const Timestamp& stamp) {
        uuid = key; axis = index; value = pos; timestamp = stamp;
    }
    
};

/**
 * This simple class is a struct to hold information about button presses
 *
 * A button has only two states: up and down. This event will fire only when
 * this state changes.
 */
class GameControllerButtonEvent {
public:
    /** The time of the input event */
    Timestamp timestamp;
    /** The UID of the relevant device */
    std::string uuid;
    /** The button reference */
    GameController::Button button;
    /** Whether the button event is from a press (not a release) */
    bool down;
    
    /**
     * Constructs a new button event with the default values
     */
    GameControllerButtonEvent() : button(GameController::Button::INVALID), down(false) {}
    
    /**
     * Constructs a new button event with the given values
     *
     * @param key   The device UID
     * @param index The button index
     * @param dwn   Whether the button is pressed
     * @param stamp The timestamp for the event
     */
    GameControllerButtonEvent(const std::string key, GameController::Button index,
                              bool dwn, const Timestamp& stamp) {
        uuid = key; button = index; down = dwn; timestamp = stamp;
    }
};

/**
 * This simple class is a struct to hold information about hat movement
 *
 * A hat is a directional pad with 9 different states. This event will fire
 * only when this state changes.
 */
class GameControllerDPadEvent {
public:
    /** The time of the input event */
    Timestamp timestamp;
    /** The UID of the relevant device */
    std::string uuid;
    /** The new D-Pad position */
    GameController::DPad state;
    
    /**
     * Constructs a new acceleration event with the default values
     */
    GameControllerDPadEvent() : state(GameController::DPad::CENTERED) {}
    
    /**
     * Constructs a new hat event with the given values
     *
     * @param key   The device UID
     * @param pos   The hat position
     * @param stamp The timestamp for the event
     */
    GameControllerDPadEvent(const std::string key, GameController::DPad pos,
                           const Timestamp& stamp) {
    	uuid = key; state = pos; timestamp = stamp;                       
    }
    
};
    
#pragma mark -
#pragma mark GameControllerInputEvent
/**
 * This simple class is a struct to hold information about a device change.
 *
 * This event is generated when new devices are added to the device list, or
 * when an existing device is removed.
 */
class GameControllerInputEvent {
public:
    /** The time of the device event */
    Timestamp timestamp;
    /** The UID of the relevant device */
    std::string uuid;
    /** Whether this device is newly added (false means it was removed) */
    bool added;
    
    /**
     * Constructs a new device change event with the default values
     */
    GameControllerInputEvent() { }
    
    /**
     * Constructs a new device change event with the given values
     *
     * @param key   The device UID
     * @param add   Whether the device was added (not removed)
     * @param stamp The timestamp for the event
     */
    GameControllerInputEvent(const std::string key, bool add,
                             const Timestamp& stamp) {
        uuid = key; added = add; timestamp = stamp;
    }
    
};

#pragma mark -
#pragma mark GameControllerInput
/**
 * This class is an input manager for a collection of game controllers
 *
 * While it is possible to have more than one game controller attached at any
 * time, SDL broadcasts all controller state changes. Therefore, it is useful
 * to have a central hub that manages controllers and dispatches events to the
 * appropriate controller. In addition, this particular input device monitors
 * when controllers are added and removed.
 *
 * Game controllers only receive events when they are activated. See the
 * methods {@link GameControllerInput#open} and {@link GameControllerInput#close}
 * for how to activate and deactivate controllers.
 */
class GameControllerInput : public InputDevice {
public:
    /**
     * @typedef Listener
     *
     * This type represents a listener for the {@link GameControllerInput} class.
     *
     * In CUGL, listeners are implemented as a set of callback functions, not as
     * objects. This allows each listener to implement as much or as little
     * functionality as it wants. A listener is identified by a key which should
     * be a globally unique unsigned int.
     *
     * An event is delivered whenever a new game controller is either added to
     * or removed from the list of devices. This can happen when a device loses
     * power, or it connected during a play session.
     *
     * Listeners are guaranteed to be called at the start of an animation frame,
     * before the method {@link Application#update(float) }.
     *
     * While game controller input listeners do not traditionally require focus
     * like a keyboard does, we have included that functionality. While only one
     * listener can have focus at a time, all listeners will receive input from
     * the Game Controller input device.
     *
     * The function type is equivalent to
     *
     *      std::function<void(const GameControllerInputEvent event, bool focus)>
     *
     * @param event     The game controller input event
     * @param focus     Whether the listener currently has focus
     */
    typedef std::function<void(const GameControllerInputEvent event,
                               bool focus)> Listener;

private:
    /** The list of all devices connected (identified by UIDs) */
    std::unordered_map<std::string, int> _devices;
    /** The descriptive names for these devices */
    std::unordered_map<std::string, std::string> _names;
    /** A map from the SDL identifiers to our UIDs */
    std::unordered_map<SDL_JoystickID, std::string> _joyids;
    /** The active game controllers, identified by SDL id */
    std::unordered_map<SDL_JoystickID, std::shared_ptr<GameController>> _bysdl;
    /** The active game controllers, identified by UID */
    std::unordered_map<std::string, std::shared_ptr<GameController>> _byname;

    /** The set of listeners called whenever we update the device list */
    std::unordered_map<Uint32, Listener> _listeners;
    
    /** Whether to filter out non-gamepad joysticks */
    bool _filter;

#pragma mark Internals
    /**
     * Adds a new device to this manager.
     *
     * This method generates the UID that we use to reference the devices.
     * It is called when the manager is first initialized (when it scans
     * all connected devices) and when a new device is connected.
     *
     * @param index The SDL device index
     *
     * @return the UID of the new device
     */
    std::string addDevice(int index);
    
    /**
     * Removes a new device to this manager.
     *
     * This method will close the associated game controller, invalidating
     * any references to it. It is called whenever a device becomes
     * disconnected.
     *
     * @param jid   The SDL instance identifier
     *
     * @return the UID of the removed device
     */
    std::string removeDevice(SDL_JoystickID jid);

    /**
     * Scans all connected game controllers, adding them to this manager.
     *
     * This method depends on {@link #doesFilter} for what it considers a
     * valid game controller.
     */
    void scanDevices();
    
#pragma mark Constructors
    /**
     * Creates and initializes a new game controller manager.
     *
     * WARNING: Never allocate a game controller manager directly. Always
     * use the {@link Input#activate()} method instead.
     */
    GameControllerInput() : _filter(true) { }
    
    /**
     * Deletes this input device, disposing of all resources
     */
    virtual ~GameControllerInput() { dispose(); }
    
    /**
     * Initializes this device, acquiring any necessary resources
     *
     * @return true if initialization was successful
     */
    bool init();
    
    /**
     * Unintializes this device, returning it to its default state
     *
     * An uninitialized device may not work without reinitialization.
     */
    virtual void dispose() override;

#pragma mark Attributes
public:
    /**
     * Returns true if this game controller manager filters its devices
     *
     * Our game controller manager is an interface built on top of the SDL
     * joystick functions. However, SDL has a very broad definition of
     * joystick, and uses it to include things like an accelerometer. If
     * this value is true (which is the default), then only devices which
     * match traditional game controllers will be listed.
     *
     * @return true if this game controller manager filters its devices
     */
    bool doesFilter() const { return _filter; }
    
    /**
     * Sets whether this game controller manager filters its devices
     *
     * Our game controller manager is an interface built on top of the SDL
     * joystick functions. However, SDL has a very broad definition of
     * joystick, and uses it to include things like an accelerometer. If
     * this value is true (which is the default), then only devices which
     * match traditional game controllers will be listed.
     *
     * @param value whether this game controller manager filters its devices
     */
    void filter(bool value);
    
    /**
     * Returns the number of connected devices.
     *
     * This value will be affected by {@link #doesFilter}.
     *
     * @return the number of connected devices.
     */
    size_t size() const;
    
#pragma mark Device Access
    /**
     * Returns the list of connected devices.
     *
     * The list is a vector of unique identifiers (UID) used to identify
     * each connected controller. These identifiers are not very descriptive,
     * as they are designed to be compact and unique. For a description of
     * each device, use {@link #getName}.
     *
     * @return the list of connected devices.
     */
	std::vector<std::string> devices() const;

    /**
     * Returns a descriptive name for the given device.
     *
     * The UID for the device should be one listed in {@link #devices()}. If
     * the device does not exist, it will return the empty string.
     *
     * @param uid   The device UID
     *
     * @return a descriptive name for the given device.
     */
    std::string getName(const std::string uid);

    /**
     * Returns a reference to a newly actived game controller.
     *
     * This method activates the game controller with the given UID. If the
     * game controller is already active, it simply returns a reference to
     * the existing game controller. The UID for the device should be one
     * listed in {@link #devices()}. If the device does not exist, this method
     * will return nullptr.
     *
     * It is generally a good idea to close all game controllers when you
     * are done with them. However, deactivating this game controller
     * automatically disposes of any active controllers.
     *
     * @param uid   The device UID
     *
     * @return a reference to a newly actived game controller.
     */
    std::shared_ptr<GameController> open(const std::string uid);
    
    /**
     * Returns a reference to the given game controller.
     *
     * This method assumes the game controller for this UID has already been
     * activated. The UID for the device should be one listed in
     * {@link #devices()}. If the device does not exist, or the device has
     * not been activate, this method will return nullptr.
     *
     * @param uid   The device UID
     *
     * @return a reference to the given game controller.
     */
    std::shared_ptr<GameController> get(const std::string uid);
    
    /**
     * Closes the game controller for the given UID
     *
     * This invalidates all references to the game controller, making them
     * no longer usable. The only way to access the game controller again is
     * to call {@link #open}.
     *
     * @param uid   The device UID
     */
    void close(const std::string uid);

#pragma mark Listener Methods
    /**
     * Requests focus for the given identifier
     *
     * Only a listener can have focus. This method returns false if key
     * does not refer to an active listener
     *
     * @param key   The identifier for the focus object
     *
     * @return false if key does not refer to an active listener
     */
    virtual bool requestFocus(Uint32 key) override;
    
    /**
     * Returns true if key represents a listener object
     *
     * @param key   The identifier for the listener
     *
     * @return true if key represents a listener object
     */
    bool isListener(Uint32 key) const;
    
    /**
     * Returns the game controller manager listener for the given object key
     *
     * If there is no listener for the given key, it returns nullptr.
     *
     * @param key   The identifier for the listener
     *
     * @return the game controller manager listener for the given object key
     */
    const Listener getListener(Uint32 key) const;
    
    /**
     * Adds an game controller manager listener for the given object key
     *
     * There can only be one listener for a given key. If there is already
     * a listener for the key, the method will fail and return false. You
     * must remove a listener before adding a new one for the same key.
     *
     * @param key       The identifier for the listener
     * @param listener  The listener to add
     *
     * @return true if the listener was succesfully added
     */
    bool addListener(Uint32 key, Listener listener);
    
    /**
     * Removes the game controller manager listener for the given object key
     *
     * If there is no active listener for the given key, this method fails and
     * returns false.
     *
     * @param key   The identifier for the listener
     *
     * @return true if the listener was succesfully removed
     */
    bool removeListener(Uint32 key);
    
#pragma mark Input Device Methods
    /**
     * Clears the state of this input device, readying it for the next frame.
     *
     * Many devices keep track of what happened "this" frame. This method is
     * necessary to advance the frame.
     */
    virtual void clearState() override;
    
    /**
     * Processes an SDL_Event
     *
     * The dispatcher guarantees that an input device only receives events that
     * it subscribes to.
     *
     * @param event The input event to process
     * @param stamp The event timestamp in CUGL time
     *
     * @return false if the input indicates that the application should quit.
     */
    virtual bool updateState(const SDL_Event& event, const Timestamp& stamp) override;
    
    /**
     * Determine the SDL events of relevance and store there types in eventset.
     *
     * An SDL_EventType is really Uint32. This method stores the SDL event
     * types for this input device into the vector eventset, appending them
     * to the end. The Input dispatcher then uses this information to set up
     * subscriptions.
     *
     * @param eventset  The set to store the event types.
     */
    virtual void queryEvents(std::vector<Uint32>& eventset) override;
    
    // Apparently friends are not inherited
    friend class Input;
};

}

#endif /* __CU_GAME_CONTROLLER_H__ */
