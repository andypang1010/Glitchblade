//
//  display_opengl.cpp
//  Cornell University Game Library (CUGL)
//
//  This module is an implementation of Display that relies on OpenGL. Using it
//  requires the graphics package. If it is not linked into application, it
//  will fail. If you wish to use only the core packages, you should use the
//  headless implementation instead.
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
#include <cugl/core/util/CUDebug.h>
#include <cugl/core/CUBase.h>
#include <cugl/core/CUDisplay.h>
#include <cugl/graphics/CUGraphicsBase.h>
#include <cugl/graphics/CUTexture.h>


#include <SDL_ttf.h>
#include <SDL_app.h>

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

// TODO: Update SDL_DisplayOrientation to include device orientations
/**
 * Returns the CUGL orientation for the given SDL orientation
 *
 * @param orientation   the SDL orientation
 *
 * @return the CUGL orientation for the given SDL orientation
 */
static cugl::Display::Orientation translateOrientation(SDL_DisplayOrientation orientation) {
    switch (orientation) {
        case SDL_ORIENTATION_UNKNOWN:
            return cugl::Display::Orientation::FIXED;
        case SDL_ORIENTATION_PORTRAIT:
            return cugl::Display::Orientation::PORTRAIT;
        case SDL_ORIENTATION_PORTRAIT_FLIPPED:
            return cugl::Display::Orientation::UPSIDE_DOWN;
        case SDL_ORIENTATION_LANDSCAPE:
            return cugl::Display::Orientation::LANDSCAPE;
        case SDL_ORIENTATION_LANDSCAPE_FLIPPED:
            return cugl::Display::Orientation::LANDSCAPE_REVERSED;

    }
    return cugl::Display::Orientation::UNKNOWN;
}

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
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        CULogError("Could not initialize display: %s",SDL_GetError());
        return false;
    }
    
    // Initialize the TTF library
    if ( TTF_Init() < 0 ) {
        CULogError("Could not initialize TTF: %s",SDL_GetError());
        return false;
    }
    
    // We have to set the OpenGL prefs BEFORE creating window
    if (!prepareOpenGL(flags & INIT_MULTISAMPLED)) {
        return false;
    }
    Uint32 sdlflags = SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL;
    
    if (flags & INIT_HIGH_DPI) {
        sdlflags |= SDL_WINDOW_ALLOW_HIGHDPI;
    }
    
    _display = 0;
    SDL_Rect native;
    _scale = APP_GetDisplayPixelDensity(_display);

    if (flags & INIT_FULLSCREEN) {
        APP_GetDisplayPixelBounds(_display,&native);
        _fullscreen = true;
        SDL_ShowCursor(0);
        sdlflags |= SDL_WINDOW_FULLSCREEN;
        _bounds.origin.x = native.x;
        _bounds.origin.y = native.y;
        _bounds.size.width  = native.w;
        _bounds.size.height = native.h;
    } else if (flags & INIT_CENTERED) {
        SDL_GetDisplayBounds(_display,&native);
        _fullscreen = false;
        native.x = (native.w - bounds.size.width)/2;
        native.y = (native.h - bounds.size.height)/2;
        _bounds.origin.x = native.x;
        _bounds.origin.y = native.y;
        _bounds.size = bounds.size;
    }

    // Make the window
    _title  = title;
    _window = SDL_CreateWindow(title.c_str(),
                               (int)_bounds.origin.x,   (int)_bounds.origin.y,
                               (int)_bounds.size.width, (int)_bounds.size.height,
                               sdlflags);
    
    if (!_window) {
        CULogError("Could not create window: %s", SDL_GetError());
        return false;
    }
    
#if CU_PLATFORM == CU_PLATFORM_MACOS || CU_PLATFORM == CU_PLATFORM_IPHONE
    if (!_fullscreen) {
        _bounds.size   *= _scale;
    }
#endif
    
    // Now we can create the OpenGL context
    if (!initOpenGL(flags & INIT_MULTISAMPLED)) {
        SDL_DestroyWindow(_window);
        _window = nullptr;
        return false;
    }
    
    if (_fullscreen) {
        SDL_Rect usable;
        APP_GetDisplaySafeBounds(_display,&usable);
        _usable.origin.x = usable.x;
        _usable.origin.y = _bounds.size.height-(usable.h+usable.y);
        _usable.size.width  = usable.w;
        _usable.size.height = usable.h;
    } else {
        _usable = _bounds;
    }
    _notched = APP_CheckDisplayNotch(_display);


// The mobile devices have viewport problems
#if CU_PLATFORM == CU_PLATFORM_ANDROID || CU_PLATFORM == CU_PLATFORM_IPHONE
    glViewport(0, 0, (int)_bounds.size.width, (int)_bounds.size.height);
#endif

    // Prevent this from happening in loading threads
    graphics::Texture::getBlank();

    _initialOrientation = translateOrientation(SDL_GetDisplayOrientation(_display));
    _displayOrientation = _initialOrientation;
    _deviceOrientation  = translateOrientation(APP_GetDeviceOrientation(_display));
    _defaultOrientation = translateOrientation(APP_GetDefaultOrientation(_display));
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
    if (_window != nullptr) {
        SDL_GL_DeleteContext(_glContext);
        _glContext = NULL;
        SDL_DestroyWindow(_window);
        _window = nullptr;
    }
    
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
    if (_window != nullptr) {
        SDL_SetWindowTitle(_window, title.c_str());
    }
}

/**
 * Shows the window for this display (assuming it was hidden).
 *
 * This method does nothing if the window was not hidden.
 */
void Display::show() {
    SDL_ShowWindow(_window);
}

/**
 * Hides the window for this display (assuming it was visible).
 *
 * This method does nothing if the window was not visible.
 */
void Display::hide() {
    SDL_HideWindow(_window);
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
    const char* glinfo = (const char*)glGetString(GL_VERSION);
    return std::string(glinfo);
}


/**
 * Returns true if this device has a landscape orientation
 *
 * @return true if this device has a landscape orientation
 */
bool Display::isLandscape() const {
    return _deviceOrientation == cugl::Display::Orientation::LANDSCAPE ||
           _deviceOrientation == cugl::Display::Orientation::LANDSCAPE_REVERSED;
}

/**
 * Returns true if this device has a portrait orientation
 *
 * @return true if this device has a portrait orientation
 */
bool Display::isPortrait() const {
    return _deviceOrientation == cugl::Display::Orientation::PORTRAIT ||
           _deviceOrientation == cugl::Display::Orientation::UPSIDE_DOWN;
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
    glClearColor(color.r, color.g, color.b, color.a);
    glStencilMask(0xffffffff);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
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
    SDL_GL_SwapWindow(_window);

    Orientation oldDisplay = _displayOrientation;
    Orientation oldDevice  = _deviceOrientation;
    _displayOrientation = translateOrientation(SDL_GetDisplayOrientation(_display));
    _deviceOrientation  = translateOrientation(APP_GetDeviceOrientation(_display));
    if (oldDisplay != _displayOrientation) {
        SDL_Rect temp;
        APP_GetDisplayPixelBounds(_display,&temp);
        _bounds.origin.x = temp.x;
        _bounds.origin.y = temp.y;
        _bounds.size.width  = temp.w;
        _bounds.size.height = temp.h;
        APP_GetDisplaySafeBounds(_display,&temp);
        _usable.origin.x = temp.x;
        _usable.origin.y = _bounds.size.height-(temp.h+temp.y);
        _usable.size.width  = temp.w;
        _usable.size.height = temp.h;
    }
    if (_orientationListener &&
        (oldDevice != _deviceOrientation || oldDisplay != _displayOrientation)) {
        _orientationListener(oldDevice,_deviceOrientation,oldDisplay != _displayOrientation);
    }
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
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _rendbuffer);
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
    glGetIntegerv(GL_FRAMEBUFFER_BINDING,  &_framebuffer);
    glGetIntegerv(GL_RENDERBUFFER_BINDING, &_rendbuffer);
}

/**
 * Returns true if this display supports multisampling
 *
 * @return true if this display supports multisampling
 */
bool Display::supportsMultisample() {
#if CU_GL_PLATFORM == CU_GL_OPENGLES
    return false;
#endif
    return true;
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
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    
#if CU_GL_PLATFORM == CU_GL_OPENGLES
    int profile = SDL_GL_CONTEXT_PROFILE_ES;
    int version = 3; // Force 3 on mobile
#else
    int profile = SDL_GL_CONTEXT_PROFILE_CORE;
    int version = 4; // Force 4 on desktop
    if (multisample) {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    }
#endif
    
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, profile) != 0) {
        CULogError("OpenGL is not supported on this platform: %s", SDL_GetError());
        return false;
    }
    
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, version) != 0) {
        CULogError("OpenGL %d is not supported on this platform: %s", version, SDL_GetError());
        return false;
    }
    
    // Enable stencil support for sprite batch
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    
    return true;
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
#if CU_GL_PLATFORM != CU_GL_OPENGLES
    if (multisample) {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    }
#endif

    // Create the OpenGL context
    _glContext = SDL_GL_CreateContext( _window );
    if( _glContext == NULL )  {
        CULogError("Could not create OpenGL context: %s", SDL_GetError() );
        return false;
    }

// Multisampling support
#if CU_GL_PLATFORM != CU_GL_OPENGLES
    glEnable(GL_LINE_SMOOTH);
    if (multisample) {
        glEnable(GL_MULTISAMPLE);
    }
#endif

#if CU_PLATFORM == CU_PLATFORM_WINDOWS || CU_PLATFORM == CU_PLATFORM_LINUX
    //Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        SDL_Log("Error initializing GLEW: %s", glewGetErrorString(glewError));
    }
#endif

    queryRenderTarget();
    return true;
}
