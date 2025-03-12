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
#ifndef __GB_INPUT_H__
#define __GB_INPUT_H__
#include <cugl/cugl.h>
#include <unordered_set>

/**
 * This class represents player input in the Platform demo.
 *
 * This input handler uses the CUGL input API.  It uses the polling API for
 * keyboard, but the callback API for touch.  This demonstrates a mix of ways
 * to handle input, and the reason for hiding it behind an abstraction like
 * this class.
 *
 * In the case of touch devices, this implements the platformer controls
 * found in the popular game Oddmar.  The left side is a floating joystick
 * for left-right controls.  The right side is swipe up to jump and double
 * tap to shoot.
 *
 * Unlike CUGL input devices, this class is not a singleton.  It must be
 * allocated before use.  However, you will notice that we do not do any
 * input initialization in the constructor.  This allows us to allocate this
 * controller as a field without using pointers. We simply add the class to the
 * header file of its owner, and delay initialization (via the method init())
 * until later. This is one of the main reasons we like to avoid initialization
 * in the constructor.
 */
class PlatformInput {
private:
    /** Reference to the left joystick image */
    std::shared_ptr<cugl::scene2::PolygonNode> _leftnode;
    /** Reference to the right joystick image */
    std::shared_ptr<cugl::scene2::PolygonNode> _rightnode;
    /** Whether or not this input is active */
    bool _active;
    // KEYBOARD EMULATION
    /** Whether the jump key is down */
    bool  _keyJump;
    /** Whether the fire key is down */
    bool  _keyFire;
    /** Whether the reset key is down */
    bool  _keyReset;
    /** Whether the debug key is down (Z)*/
    bool  _keyDebug;
    /** Whether the exit key is down */
    bool  _keyExit;
    /** Whether the left arrow key is down */
    bool  _keyLeft;
    /** Whether the right arrow key is down */
    bool  _keyRight;
    /** Whether the left dash key is down (A)*/
    bool  _keyLdash;
    /** Whether the right dash key is down (D) */
    bool  _keyRdash;
    /** Whether the guard key is down (spacebar) */
    bool  _keyGuard;
  
protected:
    // INPUT RESULTS
    /** Whether the reset action was chosen. */
    bool _resetPressed;
    /** Whether the debug toggle was chosen. */
    bool _debugPressed;
    /** Whether the exit action was chosen. */
    bool _exitPressed;
    /** Whether the fire action was chosen. */
    bool _firePressed;
    /** Whether the left dash action was chosen. */
    bool _ldashPressed;
    /** Whether the right dash action was chosen. */
    bool _rdashPressed;
    /** Whether the jump action was chosen. */
    bool _jumpPressed;
    /** Whether the guard action was chosen. */
    bool _guardPressed;
    /** Whether the strafe left action was chosen. */
    bool _leftPressed;
    /** Whether the strafe right action was chosen. */
    bool _rightPressed;
    /** How much did we move horizontally? */
    float _horizontal;

#pragma mark Internal Touch Management   
	// The screen is divided into two zones: Left, Right 
	// These are all shown in the diagram below.
    //
	// The meaning of any touch depends on the zone it begins in.

	/** Information representing a single "touch" (possibly multi-finger) */
	struct TouchInstance {
		/** The anchor touch position (on start) */
		cugl::Vec2 position;
        /** The current touch time */
        cugl::Timestamp timestamp;
		/** The touch id(s) for future reference */
        std::unordered_set<Uint64> touchids;
	};

	/** Enumeration identifying a zone for the current touch */
	enum class Zone {
		/** The touch was not inside the screen bounds */
		UNDEFINED,
		/** The touch was in the left zone (as shown above) */
		LEFT,
		/** The touch was in the right zone (as shown above) */
		RIGHT,
	};
    	/** Enumeration identifying a zone type for the current touch */
	enum class ZoneType {
		/** The touch was in the swiping zone (default left)*/
		SWIPE,
		/** The touch was in the strafe/joystick zone (default right) */
		STRAFE,
	};
        /** Enumeration identifying a swipe type */
    enum class SwipeType {
        /** Swiped left for left dash */
        LEFTDASH,
        /** Swiped right for right dash */
        RIGHTDASH,
        /** Swiped up (for jump) */
        JUMP,
        /** Swiped down (for guard) */
        GUARD,
        /** The "swipe" did not meet the swipe threshold */
        NONE,
    };

	/** The bounds of the entire game screen (in touch coordinates) */
    cugl::Rect _tbounds;
    /** The bounds of the entire game screen (in scene coordinates) */
	cugl::Rect _sbounds;
	/** The bounds of the left touch zone */
	cugl::Rect _lzone;
	/** The bounds of the right touch zone */
	cugl::Rect _rzone;

	// Each zone can have only one touch
	/** The current touch location for the left zone */
	TouchInstance _ltouch;
	/** The current touch location for the right zone */
	TouchInstance _rtouch;
	/** The current touch location for the bottom zone */
	TouchInstance _mtouch;
    
    /** Whether the virtual joystick is active */
    bool _joystick;
    /** The position of the virtual joystick */
    cugl::Vec2 _joycenter;
    /** Whether or not we have processed a jump for this swipe yet */
    bool _hasJumped;
    /** The timestamp for a double tap on the right */
    cugl::Timestamp _rtime;
	/** The timestamp for a double tap in the middle */
	cugl::Timestamp _mtime;
    /** The timestamp for a double tap on the left */
    cugl::Timestamp _ltime;

    /**
     * Defines the zone boundaries, so we can quickly categorize touches.
     */
	void createZones();
  
    /**
     * Populates the initial values of the TouchInstances
     */
    void clearTouchInstance(TouchInstance& touchInstance);

    /**
     * Returns the correct zone for the given position.
     *
     * See the comments above for a description of how zones work.
     *
     * @param  pos  a position in screen coordinates
     *
     * @return the correct zone for the given position.
     */
    Zone getZone(const cugl::Vec2 pos) const;
    
    /**
     * Returns the scene location of a touch
     *
     * Touch coordinates are inverted, with y origin in the top-left
     * corner. This method corrects for this and scales the screen
     * coordinates down on to the scene graph size.
     *
     * @return the scene location of a touch
     */
    cugl::Vec2 touch2Screen(const cugl::Vec2 pos) const;
    
    /**
     * Set visibility and position of joystick scene graph nodes
     *
     */
    void renderJoystick();
    /**
     * Processes movement for the floating joystick.
     *
     * This will register movement as left or right (or neither).  It
     * will also move the joystick anchor if the touch position moves
     * too far.
     *
     */
    void processJoystick();
    
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
    SwipeType processSwipe(const cugl::Vec2 start, const cugl::Vec2 stop, cugl::Timestamp current);
  
public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new input controller.
     *
     * This constructor does NOT do any initialzation.  It simply allocates the
     * object. This makes it safe to use this class without a pointer.
     */
    PlatformInput(); // Don't initialize.  Allow stack based
    
    /**
     * Disposes of this input controller, releasing all listeners.
     */
    ~PlatformInput() { dispose(); }
    
    /**
     * Deactivates this input controller, releasing all listeners.
     *
     * This method will not dispose of the input controller. It can be reused
     * once it is reinitialized.
     */
    void dispose();
    /**
     * Creates a new player at the given position.
     *
     * The player has the given size, scaled so that 1 pixel = 1 Box2d unit
     *
      * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param pos   Initial position in world coordinates
     * @param size  The size of the player in world units
     *
     * @return  A newly allocated PlayerModel at the given position with the given scale
     */
    static std::shared_ptr<PlatformInput> alloc(const std::shared_ptr<cugl::AssetManager>& assetRef, const cugl::Rect bounds) {
        std::shared_ptr<PlatformInput> result = std::make_shared<PlatformInput>();
        return (result->init(assetRef, bounds)? result : nullptr);
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
    bool init(const std::shared_ptr<cugl::AssetManager>& assetRef, const cugl::Rect bounds);
    
#pragma mark -
#pragma mark Input Detection
    /**
     * Returns true if the input handler is currently active
     *
     * @return true if the input handler is currently active
     */
    bool isActive( ) const { return _active; }

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
    void  update(float dt);

    /**
     * Clears any buffered inputs so that we may start fresh.
     */
    void clear();
    
#pragma mark -
#pragma mark Input Results
    /**
     * Returns the amount of sideways movement.
     *
     * -1 = left, 1 = right, 0 = still
     *
     * @return the amount of sideways strafing movement. 
     */
	float getHorizontal() const { return _horizontal; }

    /**
     * Returns if the jump input was received.
     *
     * @return if the jump input was received.
     */
	float didJump() const { return _jumpPressed; }
    /**
     * Returns if the left strafe input was received.
     *
     * @return if the left strafe input was received.
     */
    float didStrafeLeft() const { return _leftPressed; }

    /**
     * Returns if the right strafe input was received.
     *
     * @return if the right strafe input was received.
     */
    float didStrafeRight() const { return _rightPressed; }
    /**
     * Returns if the left dash input was received.
     *
     * @return if the left dash input was received.
     */
    float didDashLeft() const { return _ldashPressed; }

    /**
     * Returns if the right dash input was received.
     *
     * @return if the right dash input was received.
     */
    float didDashRight() const { return _rdashPressed; }

    /**
     * Returns if the guard input was received.
     *
     * @return if the guard input was received.
     */
    float didGuard() const { return _guardPressed; }

    /**
     * Returns true if the fire input was received.
     *
     * @return true if the fire input was received.
     */
	bool didFire() const { return _firePressed; }

    /**
     * Returns true if the reset input was received.
     *
     * @return true if the reset input was received.
     */
	bool didReset() const { return _resetPressed; }

    /**
     * Returns true if the player wants to go toggle the debug mode.
     *
     * @return true if the player wants to go toggle the debug mode.
     */
	bool didDebug() const { return _debugPressed; }

	/**
	 * Returns true if the exit input was received.
	 *
	 * @return true if the exit input was received.
	 */
	bool didExit() const { return _exitPressed; }
    
    /**
     * Returns true if the virtual joystick is in use (touch only)
     *
     * @return true if the virtual joystick is in use (touch only)
     */
    bool withJoystick() const { return _joystick; }

    /**
     * Returns the scene graph position of the virtual joystick
     *
     * @return the scene graph position of the virtual joystick
     */
    cugl::Vec2 getJoystick() const { return _joycenter; }

#pragma mark -
#pragma mark Touch and Mouse Callbacks
    /**
     * Callback for the beginning of a touch event
     *
     * @param event The associated event
     * @param focus	Whether the listener currently has focus
     *
     */
    void touchBeganCB(const cugl::TouchEvent& event, bool focus);

    /**
     * Callback for the end of a touch event
     *
     * @param event The associated event
     * @param focus	Whether the listener currently has focus
     */
    void touchEndedCB(const cugl::TouchEvent& event, bool focus);
  
    /**
     * Callback for a mouse release event.
     *
     * @param event The associated event
     * @param previous The previous position of the touch
     * @param focus	Whether the listener currently has focus
     */
    void touchesMovedCB(const cugl::TouchEvent& event, const cugl::Vec2& previous, bool focus);
  
};

#endif /* __GB_INPUT_H__ */
