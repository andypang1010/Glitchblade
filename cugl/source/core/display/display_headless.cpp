//
//  display_headless.cpp
//  Cornell University Game Library (CUGL)
//
//  This module is a headless implementation of Display for non-graphical
//  simulations. This module was created because students did not want to have
//  to load OpenGL onto Docker containers when running a CUGL server.
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
//  Author: Walker White
//  Version: 7/3/24 (CUGL 3.0 reorganization)
//
#include <cugl/core/CUDisplay.h>

using namespace cugl;

/** The display singleton */
Display* Display::_thedisplay = nullptr;

// Flags for the device initialization
/** Whether this display should use the fullscreen */
Uint32 Display::INIT_FULLSCREEN   = 1;
/** Whether this display should support a High DPI screen */
Uint32 Display::INIT_HIGH_DPI     = 2;
/** Whether this display should be multisampled */
Uint32 Display::INIT_MULTISAMPLED = 4;
/** Whether this display should be centered (on windowed screens) */
Uint32 Display::INIT_CENTERED     = 8;
/** Whether this display should have VSync enabled. */
Uint32 Display::INIT_VSYNC = 16;

#pragma mark Constructors
/**
 * Creates a new, unitialized Display.
 *
 * All of the values are set to 0 or UNKNOWN, depending on their type. You
 * must initialize the Display to access its values.
 *
 * WARNING: This class is a singleton.  You should never access this
 * constructor directly.  Use the {@link start()} method instead.
 */
Display::Display() :
_window(nullptr),
_display(0),
_glContext(NULL),
_framebuffer(0),
_rendbuffer(0),
_fullscreen(false),
_initialOrientation(Orientation::UNKNOWN),
_displayOrientation(Orientation::UNKNOWN),
_deviceOrientation(Orientation::UNKNOWN) {}

/**
 * Initializes the display with the current screen information.
 *
 * This method creates a display with the given title and bounds. As part
 * of this initialization, it will create the OpenGL context, using
 * the flags provided.  The bounds are ignored if the display is fullscreen.
 * In that case, it will use the bounds of the display.
 *
 * This method gathers the native resolution bounds, pixel density, and
 * orientation  using platform-specific tools.
 *
 * WARNING: This class is a singleton.  You should never access this
 * initializer directly.  Use the {@link start()} method instead.
 *
 * @param title     The window/display title
 * @param bounds    The window/display bounds
 * @param flags     The initialization flags
 *
 * @return true if initialization was successful.
 */
bool Display::init(std::string title, Rect bounds, Uint32 flags) {
    // Still doing everything, but no window
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        CULogError("Could not initialize display: %s",SDL_GetError());
        return false;
    }
    
    
    _initialOrientation = Display::Orientation::UNKNOWN;
    _displayOrientation = Display::Orientation::UNKNOWN;
    _deviceOrientation  = Display::Orientation::UNKNOWN;
    _defaultOrientation = Display::Orientation::UNKNOWN;
    return true;
}

/**
 * Uninitializes this object, releasing all resources.
 *
 * This method quits the SDL video system and disposes the OpenGL context,
 * effectively exitting and shutting down the entire program.
 *
 * WARNING: This class is a singleton.  You should never access this
 * method directly.  Use the {@link stop()} method instead.
 */
void Display::dispose() {
    _display = 0;
    _framebuffer = 0;
    _fullscreen = false;
    _bounds.size.set(0,0);
    _usable.size.set(0,0);
    _scale = 0.0f;

    _initialOrientation = Orientation::UNKNOWN;
    _displayOrientation = Orientation::UNKNOWN;
    _deviceOrientation  = Orientation::UNKNOWN;
    _defaultOrientation = Orientation::UNKNOWN;
    SDL_Quit();
}

#pragma mark -
#pragma mark Static Accessors
/**
 * Starts up the SDL display and video system.
 *
 * This static method needs to be the first line of any application, though
 * it is handled automatically in the {@link Application} class.
 *
 * This method creates the display with the given title and bounds. As part
 * of this initialization, it will create the OpenGL context, using
 * the flags provided.  The bounds are ignored if the display is fullscreen.
 * In that case, it will use the bounds of the display.
 *
 * Once this method is called, the {@link get()} method will no longer
 * return a null value.
 *
 * @param title     The window/display title
 * @param bounds    The window/display bounds
 * @param flags     The initialization flags
 *
 * @return true if the display was successfully initialized
 */
bool Display::start(std::string name, Rect bounds, Uint32 flags) {
    if (_thedisplay != nullptr) {
        CUAssertLog(false, "The display is already initialized");
        return false;
    }
    _thedisplay = new Display();
    return _thedisplay->init(name,bounds,flags);
}

/**
 * Shuts down the SDL display and video system.
 *
 * This static method needs to be the last line of any application, though
 * it is handled automatically in the {@link Application} class. It will
 * dipose of the display and the OpenGL context.
 *
 * Once this method is called, the {@link get()} method will return nullptr.
 * More importantly, no SDL function calls will work anymore.
 */
void Display::stop() {
    if (_thedisplay == nullptr) {
        CUAssertLog(false, "The display is not initialized");
    }
    delete _thedisplay;
    _thedisplay = nullptr;
}

#pragma mark -
#pragma mark Window Management
/**
 * Sets the title of this display
 *
 * On a desktop, the title will be displayed at the top of the window.
 *
 * @param title  The title of this display
 */
void Display::setTitle(const std::string title) {
    _title = title;
}

/**
 * Shows the window for this display (assuming it was hidden).
 *
 * This method does nothing if the window was not hidden.
 */
void Display::show() {
    // Does nothing
}

/**
 * Hides the window for this display (assuming it was visible).
 *
 * This method does nothing if the window was not visible.
 */
void Display::hide() {
    // Does nothing
}

#pragma mark -
#pragma mark Attributes

/**
 * Returns a description of this graphics API for this display.
 *
 * For example, if this application is backed by OpenGL, this method
 * returns the OpenGL version. If it is a headless client, it returns
 * "Headless".
 *
 * @return a description of this graphics API for this display.
 */
const std::string Display::getGraphicsDescription() const {
    return "Headless";
}

/**
 * Returns true if this device has a landscape orientation
 *
 * @return true if this device has a landscape orientation
 */
bool Display::isLandscape() const {
    return false;
}

/**
 * Returns true if this device has a portrait orientation
 *
 * @return true if this device has a portrait orientation
 */
bool Display::isPortrait() const {
    return false;
}

/**
 * Removes the display orientation listener for this display.
 *
 * This listener handles changes in either the device orientation (see
 * {@link getDeviceOrientation()} or the display orientation (see
 * {@link getDeviceOrientation()}. Since the device orientation will always
 * change when the display orientation does, this callback can easily safely
 * handle both. The boolean parameter in the callback indicates whether or
 * not a display orientation change has happened as well.
 *
 * Unlike other events, this listener will be invoked at the end of an
 * animation frame, after the screen has been drawn.  So it will be
 * processed before any input events waiting for the next frame.
 *
 * A display may only have one orientation listener at a time.  If this
 * display does not have an orientation listener, this method will fail.
 *
 * @return true if the listener was succesfully removed
 */
bool Display::removeOrientationListener() {
    bool result = _orientationListener != nullptr;
    _orientationListener = nullptr;
    return result;
}


#pragma mark -
#pragma mark Drawing Support
/**
 * Clears the screen to the given clear color.
 *
 * This method should be called before any user drawing happens.
 *
 * @param color The clear color
 */
void Display::clear(Color4f color) {
    // Does nothing
}

/**
 * Refreshes the display.
 *
 * This method will swap the framebuffers, drawing the screen. This
 * method should be called after any user drawing happens.
 *
 * It will also reassess the orientation state and call the listener as
 * necessary
 */
void Display::refresh() {
    // Does nothing
}


#pragma mark -
#pragma mark OpenGL Support
/**
 * Restores the default frame/render buffer.
 *
 * This is necessary when you are using a {@link RenderTarget} and want
 * to restore control the frame buffer.  It is necessary because 0 is
 * NOT necessarily the correct id of the default framebuffer (particularly
 * on iOS).
 */
void Display::restoreRenderTarget() {
    // Does nothing
}

/**
 * Queries the identity of the default frame/render buffer.
 *
 * This is necessary when you are using a {@link RenderTarget} and want
 * to restore control the frame buffer.  It is necessary because 0 is
 * NOT necessarily the correct id of the default framebuffer (particularly
 * on iOS).
 */
void Display::queryRenderTarget() {
    // Does nothing
}

/**
 * Returns true if this display supports multisampling
 *
 * @return true if this display supports multisampling
 */
bool Display::supportsMultisample() {
    return false;
}

/**
 * Assign the default settings for OpenGL
 *
 * This has to be done before the Window is created.
 *
 * @param multisample   Whether to support multisampling.
 *
 * @return true if preparation was successful
 */
bool Display::prepareOpenGL(bool multisample) {
    return false;
}

/**
 * Initializes the OpenGL context
 *
 * This has to be done after the Window is created.
 *
 * @param multisample   Whether to support multisampling.
 *
 * @return true if initialization was successful
 */
bool Display::initOpenGL(bool multisample) {
    return false;
}
