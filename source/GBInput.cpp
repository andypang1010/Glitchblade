//
//  GBInput.h
//  PlatformDemo
//
//  This input controller is primarily designed for keyboard control.  On mobile
//  you will notice that we use gestures to emulate keyboard commands. They even
//  use the same variables (though we need other variables for internal keyboard
//  emulation).  This simplifies our design quite a bit.
//
//  This file is based on the CS 3152 PhysicsDemo Lab by Don Holden, 2007
//
//  Author:  Walker White and Anthony Perello
//  Version: 2/9/21
//
#include "GBInput.h"

using namespace cugl;
using namespace cugl::graphics;

#pragma mark Input Constants

/** The key to use for reseting the game */
#define RESET_KEY KeyCode::R
/** The key for toggling the debug display */
#define DEBUG_KEY KeyCode::Z
/** The key for exitting the game */
#define EXIT_KEY  KeyCode::ESCAPE

// player action keys
/** The key for firing a bullet */
#define FIRE_KEY KeyCode::SPACE
/** The key for jumping up */
#define JUMP_KEY KeyCode::ARROW_UP
/** The key for strafing left */
#define LEFT_KEY KeyCode::ARROW_LEFT
/** The key for strafing right */
#define RIGHT_KEY KeyCode::ARROW_RIGHT
/** The key for dashing left */
#define LDASH_KEY KeyCode::A
/** The key for dashing right */
#define RDASH_KEY KeyCode::D
/** The key for guarding */
#define GUARD_KEY KeyCode::S

/** How close we need to be for a multi touch */
#define NEAR_TOUCH      100
/** The key for the event handlers */
#define LISTENER_KEY      1

/** This defines the joystick "deadzone" (how far we must move) */
#define JSTICK_DEADZONE  15
/** This defines the joystick radial size (for reseting the anchor) */
#define JSTICK_RADIUS    25
/** How far to display the virtual joystick above the finger */
#define JSTICK_OFFSET    80
/** How far we must swipe up for swipe gestures */
#define SWIPE_LENGTH    70
/** How far we must swipe down for guard gestures */
#define GUARD_LENGTH    50
/** How fast a double click must be in milliseconds */
#define DOUBLE_CLICK    400

// The screen is divided into two zones: Left and RIGHT (split down the middle)
// These are all shown in the diagram below.
// The meaning of any touch depends on the zone it begins in.

/** The portion of the screen used for the left zone */
#define LEFT_ZONE  0.5f
/** The portion of the screen used for the right zone */
#define RIGHT_ZONE 0.5

#pragma mark Joystick Assets
/** The image for the left dpad/joystick */
#define LEFT_IMAGE      "dpad_left"
/** The image for the right dpad/joystick */
#define RIGHT_IMAGE     "dpad_right"

#pragma mark -
#pragma mark Input Controller
/**
 * Creates a new input controller.
 *
 * This constructor does NOT do any initialzation.  It simply allocates the
 * object. This makes it safe to use this class without a pointer.
 */
PlatformInput::PlatformInput() :
_active(false),
_resetPressed(false),
_debugPressed(false),
_exitPressed(false),
_firePressed(false),
_jumpPressed(false),
_rdashPressed(false),
_ldashPressed(false),
_guardPressed(false),
_keyGuard(false),
_keyLdash(false),
_keyRdash(false),
_leftPressed(false),
_rightPressed(false),
_keyJump(false),
_keyFire(false),
_keyReset(false),
_keyDebug(false),
_keyExit(false),
_keyLeft(false),
_keyRight(false),
_horizontal(0.0f),
_joystick(false),
_hasJumped(false) {
}

/**
 * Deactivates this input controller, releasing all listeners.
 *
 * This method will not dispose of the input controller. It can be reused
 * once it is reinitialized.
 */
void PlatformInput::dispose() {
    if (_active) {
#ifndef CU_TOUCH_SCREEN
        Input::deactivate<Keyboard>();
#else
        Touchscreen* touch = Input::get<Touchscreen>();
        touch->removeBeginListener(LISTENER_KEY);
        touch->removeEndListener(LISTENER_KEY);
#endif
        _active = false;
    }
}

/**
 * Initializes the input control for the given bounds
 *
 * The bounds are the bounds of the scene graph.  This is necessary because
 * the bounds of the scene graph do not match the bounds of the display.
 * This allows the input device to do the proper conversion for us.
 *
 * @param bounds    the scene graph bounds
 *
 * @return true if the controller was initialized successfully
 */
bool PlatformInput::init(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& constantsRef) {
    bool success = true;
    Rect bounds;
    std::shared_ptr<JsonValue> boundsJ = constantsRef->get("scene")->get("bounds");
    bounds.origin.set(boundsJ->get("origin")->getFloat("x"), boundsJ->get("origin")->getFloat("y"));
    bounds.size.set(boundsJ->get("size")->getFloat("width"),boundsJ->get("size")->getFloat("height"));
    _tbounds = Application::get()->getDisplayBounds();
    _sbounds = bounds;
    
    
    createZones();
    clearTouchInstance(_ltouch);
    clearTouchInstance(_rtouch);
    clearTouchInstance(_mtouch);
    
    // joystick nodes
    _leftnode = scene2::PolygonNode::allocWithTexture(assetRef->get<Texture>(LEFT_IMAGE));
    _leftnode->SceneNode::setAnchor(Vec2::ANCHOR_MIDDLE_RIGHT);
    _leftnode->setScale(0.35f);
    _leftnode->setVisible(false);

    _rightnode = scene2::PolygonNode::allocWithTexture(assetRef->get<Texture>(RIGHT_IMAGE));
    _rightnode->SceneNode::setAnchor(Vec2::ANCHOR_MIDDLE_LEFT);
    _rightnode->setScale(0.35f);
    _rightnode->setVisible(false);
    
#ifndef CU_TOUCH_SCREEN
    success = Input::activate<Keyboard>();
#else
    Touchscreen* touch = Input::get<Touchscreen>();
    touch->addBeginListener(LISTENER_KEY,[=](const TouchEvent& event, bool focus) {
        this->touchBeganCB(event,focus);
    });
    touch->addEndListener(LISTENER_KEY,[=](const TouchEvent& event, bool focus) {
        this->touchEndedCB(event,focus);
    });
    touch->addMotionListener(LISTENER_KEY,[=](const TouchEvent& event, const Vec2& previous, bool focus) {
        this->touchesMovedCB(event, previous, focus);
    });
	
#endif
    _active = success;
    return success;
}


/**
 * Processes the currently cached inputs.
 *
 * This method is used to to poll the current input state.  This will poll the
 * keyboad and accelerometer.
 *
 * This method also gathers the delta difference in the touches. Depending on
 * the OS, we may see multiple updates of the same touch in a single animation
 * frame, so we need to accumulate all of the data together.
 */
void PlatformInput::update(float dt) {
#ifndef CU_TOUCH_SCREEN
    // DESKTOP CONTROLS
    Keyboard* keys = Input::get<Keyboard>();

    // Map "keyboard" events to the current frame boundary
    _keyReset  = keys->keyPressed(RESET_KEY);
    _keyDebug = keys->keyPressed(DEBUG_KEY);
    _keyExit   = keys->keyPressed(EXIT_KEY);
    _keyFire   = keys->keyPressed(FIRE_KEY);
    _keyJump   = keys->keyPressed(JUMP_KEY);
    _keyLeft = keys->keyDown(LEFT_KEY);
    _keyRight = keys->keyDown(RIGHT_KEY);
    _keyLdash = keys->keyDown(LDASH_KEY);
    _keyRdash = keys->keyDown(RDASH_KEY);
    _keyGuard = keys->keyDown(GUARD_KEY);
#endif

    _resetPressed = _keyReset;
    _debugPressed = _keyDebug;
    _exitPressed  = _keyExit;
	_firePressed  = _keyFire;
	_jumpPressed  = _keyJump;
    _ldashPressed = _keyLdash;
    _rdashPressed = _keyRdash;
    _leftPressed = _keyLeft;
    _rightPressed = _keyRight;
    _guardPressed = _keyGuard;
    
	// Directional controls
	_horizontal = 0.0f;
	if (_leftPressed) {
		_horizontal -= 1.0f;
	}
	if (_rightPressed) {
		_horizontal += 1.0f;
	}

// If it does not support keyboard, we must reset "virtual" keyboard
#ifdef CU_TOUCH_SCREEN
    _keyExit = false;
    _keyReset = false;
    _keyDebug = false;
    _keyJump  = false;
    _keyFire  = false;
    _keyLeft = false;
    _keyRight = false;
    _keyGuard = false;
    _keyLdash = false;
    _keyRdash = false;
        if (_joystick){
        CULog("Calling process joystick\n");
        processJoystick();
    }
#endif
}

/**
 * Clears any buffered inputs so that we may start fresh.
 */
void PlatformInput::clear() {
    _resetPressed = false;
    _debugPressed = false;
    _exitPressed  = false;
    _jumpPressed = false;
    _firePressed = false;
    _leftPressed = false;
    _rightPressed = false;
    _rdashPressed = false;
    _ldashPressed = false;
    _guardPressed = false;
}

#pragma mark -
#pragma mark Touch Controls

/**
 * Defines the zone boundaries, so we can quickly categorize touches.
 */
void PlatformInput::createZones() {
	_lzone = _tbounds;
	_lzone.size.width *= LEFT_ZONE;
	_rzone = _tbounds;
	_rzone.size.width *= RIGHT_ZONE;
	_rzone.origin.x = _tbounds.origin.x+_tbounds.size.width-_rzone.size.width;
}

/**
 * Populates the initial values of the input TouchInstance
 */
void PlatformInput::clearTouchInstance(TouchInstance& touchInstance) {
    touchInstance.touchids.clear();
    touchInstance.position = Vec2::ZERO;
}


/**
 * Returns the correct zone for the given position.
 *
 * See the comments above for a description of how zones work.
 *
 * @param  pos  a position in screen coordinates
 *
 * @return the correct zone for the given position.
 */
PlatformInput::Zone PlatformInput::getZone(const Vec2 pos) const {
	if (_lzone.contains(pos)) {
		return Zone::LEFT;
	} else if (_rzone.contains(pos)) {
		return Zone::RIGHT;
	}
	return Zone::UNDEFINED;
}

/**
 * Returns the scene location of a touch
 *
 * Touch coordinates are inverted, with y origin in the top-left
 * corner. This method corrects for this and scales the screen
 * coordinates down on to the scene graph size.
 *
 * @return the scene location of a touch
 */
Vec2 PlatformInput::touch2Screen(const Vec2 pos) const {
    float px = pos.x/_tbounds.size.width -_tbounds.origin.x;
    float py = pos.y/_tbounds.size.height-_tbounds.origin.y;
    Vec2 result;
    result.x = px*_sbounds.size.width +_sbounds.origin.x;
    result.y = (1-py)*_sbounds.size.height+_sbounds.origin.y;
    return result;
}

void PlatformInput::renderJoystick() {
    if (withJoystick()) {
        if (getHorizontal() < 0) {
            _leftnode->setVisible(true);
            _rightnode->setVisible(false);
        }
        else if (getHorizontal() > 0) {
            _leftnode->setVisible(false);
            _rightnode->setVisible(true);
        }
        else {
            _leftnode->setVisible(false);
            _rightnode->setVisible(false);
        }
        _leftnode->setPosition(getJoystick());
        _rightnode->setPosition(getJoystick());
    }
    else {
        _leftnode->setVisible(false);
        _rightnode->setVisible(false);
    }
}
/**
 * Processes movement for the floating joystick and move the joystick scene nodes.
 *
 * This will register movement as left or right (or neither).  It
 * will also move the joystick anchor if the touch position moves
 * too far.
 *
 */
void PlatformInput::processJoystick() {
    Vec2 diff = touch2Screen(_ltouch.position) - _joycenter;
    // CULog("position is (%f,%f) and joycenter is (%f, %f)", _ltouch.position.x, _ltouch.position.y, _joycenter.x, _joycenter.y);
    // CULog("diff is (%f, %f)", diff.x, diff.y);
    if (std::fabsf(diff.x) > JSTICK_DEADZONE) {
        if (diff.x > 0) {
            _keyLeft = false;
            _keyRight = true;
        }
        else {
            _keyLeft = true;
            _keyRight = false;
        }
    }
    renderJoystick();
    
}

/**
* Returns an vector of the swipe direction
*
* The function returns a vector representing the X difference and Y difference respectively
*
* @param  start    the start position of the candidate swipe
* @param  stop     the end position of the candidate swipe
* @param  current  the current timestamp of the gesture
*
* @return a Vec2 indicating the vector of the swipe
*/
Vec2 processSwipeVec(const Vec2 start, const Vec2 stop, Timestamp current) {
    return (stop - start);
}

/**
* Returns an enum indicating the type of swipe
*
* The function returns LEFTDASH, RIGHTDASH, JUMP, GUARD, or NO
*
* @param  start    the start position of the candidate swipe
* @param  stop     the end position of the candidate swipe
* @param  current  the current timestamp of the gesture
*
* @return an Enum indicating the type of swipe
*/
 PlatformInput::SwipeType PlatformInput::processSwipe(const Vec2 start, const Vec2 stop, Timestamp current) {
    // TODO: need to make this physical size for swipe lengths, not pixel distances
	float xdiff = (stop.x-start.x);
    float ydiff = (stop.y - start.y);
    float thresh = SWIPE_LENGTH;
    float absxdiff = std::abs(xdiff);
    float absydiff = std::abs(ydiff);

    bool xpass = absxdiff > thresh;
    bool ypass = absydiff > thresh;

    bool yguardpass = ydiff > GUARD_LENGTH;

    // Ensure at least one axis has passed the threshold
    if (xpass || ypass) {
        // X magnitude > Y magnitude (so X must have passed AND be dominant axis)
        if (absxdiff > absydiff) {
            return (xdiff > 0) ? SwipeType::RIGHTDASH : SwipeType::LEFTDASH;
        // Y magnitude > X magnitude (so Y must have passed AND be dominant axis)
        } else {
            return (ydiff > 0) ? SwipeType::GUARD : SwipeType::JUMP;
        }
    }
    else if (yguardpass) {
        return SwipeType::GUARD;
    }
    
    // Return NONE if one of the other swipe types haven't been returned already
    return SwipeType::NONE;
}

#pragma mark -
#pragma mark Touch and Mouse Callbacks
/**
 * Callback for the beginning of a touch event
 *
 * @param event The associated event
 * @param focus	Whether the listener currently has focus
 */
void PlatformInput::touchBeganCB(const TouchEvent& event, bool focus) {
    //CULog("Touch began %lld", event.touch);
    Vec2 pos = event.position;
    Zone zone = getZone(pos);
    switch (zone) {
        case Zone::LEFT: // may change to allow switching sides
            // Left half now controls the movement joystick (strafing)
            if (_ltouch.touchids.empty()) {
                _ltouch.position = event.position;
                _ltouch.timestamp.mark();
                _ltouch.touchids.insert(event.touch);
                _joystick = true;
                _joycenter = touch2Screen(event.position);
                _joycenter.y += JSTICK_OFFSET;
            }
            break;
        case Zone::RIGHT:
            // Only process if no touch in zone
            // Right half is now for other actions (jump/fire/dash)
            if (_rtouch.touchids.empty()) {
                _keyFire = (event.timestamp.ellapsedMillis(_rtime) <= DOUBLE_CLICK);
                _rtouch.position = event.position;
                _rtouch.timestamp.mark();
                _rtouch.touchids.insert(event.touch);
                _hasJumped = false;
            }
            break;
        default:
            CUAssertLog(false, "Touch is out of bounds");
            break;
    }
}

 
/**
 * Callback for the end of a touch event
 *
 * @param event The associated event
 * @param focus	Whether the listener currently has focus
 */
void PlatformInput::touchEndedCB(const TouchEvent& event, bool focus) {
    // Reset all keys that might have been set
    Vec2 pos = event.position;
    Zone zone = getZone(pos);
    if (_rtouch.touchids.find(event.touch) != _rtouch.touchids.end()) {
        _rtime = event.timestamp;
        _rtouch.touchids.clear();
    } else if (_ltouch.touchids.find(event.touch) != _ltouch.touchids.end()) {
        _keyLeft = false;
        _keyRight = false;
        _joystick = false;
        _ltime = event.timestamp;
        _ltouch.touchids.clear();
    }
}


/**
 * Callback for a touch moved event.
 *
 * @param event The associated event
 * @param previous The previous position of the touch
 * @param focus	Whether the listener currently has focus
 */
void PlatformInput::touchesMovedCB(const TouchEvent& event, const Vec2& previous, bool focus) {
    Vec2 pos = event.position;
    // this logic should change if we want to allow switching input sides
    if (_ltouch.touchids.find(event.touch) != _ltouch.touchids.end()) {
        SwipeType s_type = processSwipe(_ltouch.position, event.position, event.timestamp);
        //switch (s_type) {
        //    // can handle actions on left side here if desired
        //    default:
        //        // CULog("Doing nothing");
        //        break;
        //}

        /*if (s_type != SwipeType::NONE) {
            clearTouchInstance(_ltouch);
        }*/

        _ltouch.position = pos;
    //swipe side
    } else if (_rtouch.touchids.find(event.touch) != _rtouch.touchids.end()) {
        SwipeType s_type = processSwipe(_rtouch.position, event.position, event.timestamp);

        switch (s_type) {
            case SwipeType::LEFTDASH:
                CULog("SWIPE TYPE IS LEFTDASH");
                _keyLdash = true;
                break;
            case SwipeType::RIGHTDASH:
                CULog("SWIPE TYPE IS RIGHTDASH");
                _keyRdash = true;
                break;
            case SwipeType::JUMP:
                CULog("SWIPE TYPE IS JUMP");
                _keyJump = true;
                break;
            case SwipeType::GUARD:
                CULog("SWIPE TYPE IS GUARD");
                _keyGuard = true;
                break;
            case SwipeType::NONE:
                CULog("SWIPE TYPE IS NONE");
                break;
            default:
                CULog("Why is this printing :|");
                break;
        }

        // If we have used a swipe type, then clear that touch instance
        if (s_type != SwipeType::NONE) {
            clearTouchInstance(_rtouch);
        }
    }
}


