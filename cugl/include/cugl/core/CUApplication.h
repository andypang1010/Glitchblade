//
//  CUApplication.h
//  Cornell University Game Library (CUGL)
//
//  This class provides the core application class.  In initializes both the
//  SDL and CUGL settings, and creates the core loop.  You should inherit from
//  this class to make your root game class.
//
//  This class is always intended to be used on the stack of the main function.
//  Thererfore, this class has no allocators.
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
#ifndef __CU_APPLICATION_H__
#define __CU_APPLICATION_H__
#include <cugl/core/util/CUTimestamp.h>
#include <cugl/core/math/CUColor4.h>
#include <cugl/core/math/CURect.h>
#include <unordered_map>
#include <functional>
#include <deque>
#include <mutex>

namespace cugl {

/**
 * The storage type for all user-defined callbacks.
 *
 * The application API provides a way for the user to attach one-time or 
 * reoccuring callback functions.  This to allow the user to schedule 
 * activity in a future animation frame without having to create a separate
 * thread.  This is particularly important for functionality that accesses
 * the OpenGL context (or any of the low-level SDL subsystems), as that must 
 * be done in the main thread.
 *
 * To keep things simple, callbacks should never require arguments or
 * return a value.  If you wish to keep state, it should be done through
 * the appropriate closure.
 */
typedef struct {
    /** The callback function */
    std::function<bool()> callback;
    /** The reoccurrence period (0 if called every frame) */
    Uint32 period;
    /** The countdown until the next reoccurrence */
    Uint32 timer;
} scheduable;
    
/**
 * This class represents a basic CUGL application
 *
 * The application does not assume 2d or 3d.  This application can be used
 * with any type of graphics.
 *
 * This class is not intended to be passed around as a pointer, as it is 
 * the (subclass of the) root class.  Hence we only have a stack-based
 * initializer for this class.  
 * 
 * With that said, we do allow access to the application through static method
 * {@link get()}.  This allows other parts of the application to get important
 * information like the display size or orientation.
 */
class Application {
#pragma mark Values
public:
    /**
     * The current state of the application.
     *
     * This value is used by SDL to invoke the correct update method at
     * each frame
     */
    enum class State : unsigned int {
        /** 
         * The application is not yet initialized.
         *
         * This state indicates that there is no OpenGL context.  It is
         * unsafe to make OpenGL calls while in this state.
         */
        NONE = 0,
        /**
         * The application is initialized, but has not yet started
         *
         * This state indicates there is an OpenGL context, and OpenGL calls
         * are now safe. This is the state for initializing the application
         * with user-defined attributes.
         */
        STARTUP = 1,
        /**
         * The application is currently running in the foreground
         *
         * The update-draw loop will be invoked while the application is in
         * this state (and only in this state).
         */
        FOREGROUND = 2,
        /**
         * The application is currently suspended
         *
         * The update-draw loop will not be invoked while the application is in
         * this state.  However, no assets will be deleted unless manually
         * deleted by the programmer.
         */
        BACKGROUND = 3,
        /**
         * The application is shutting down.
         *
         * While in this state, the programmer should delete all custom data
         * in the application.  The OpenGL context will soon be deleted, and
         * the application will shift back to start {@link State#NONE}.
         */
        SHUTDOWN = 4
    };
    
    
protected:
    /** The name of this application */
    std::string _name;
    /** The organization name (company) of this application */
    std::string _org;

    /** The asset directory of this application */
    std::string _assetdir;
    /** The save directory of this application */
    std::string _savesdir;

    /** The current state of this application */
    State _state;

    /** The display bounds of this application */
    Rect _display;
    /** The SAFE display bounds of this application */
    Rect _safearea;
    /** Whether this application is running in fullscreen */
    bool _fullscreen;
    /** Whether this application is centered in the display */
    bool _centered;
    /** Whether this application supports high dpi resolution */
    bool _highdpi;
    /** Whether this application supports multisampling */
    bool _multisamp;
    
    /** The target FPS of this application */
    float _fps;
    /** The time step for the fixed loop */
    Uint64 _fixstep;
    /** Whether to respect the display vsync */
    bool _vsync;
    /** Whether to use a fixed timestep */
    bool _fixed;
    /** The default background color of this application */
    Color4f _clearColor;
    
    /** A weak pointer to the single application that is running */
    static Application* _theapp;

    
private:
    /** The millisecond equivalent of the FPS; used to delay the core loop */
    Uint32 _delay;
    
    /** A window of moving averages to track the FPS */
    std::deque<float> _fpswindow;

    /** The timestamp for application initialization */
    Timestamp _boot;
    /** The timestamp for the start of an animation frame */
    Timestamp _start;
    /** The timestamp for the end of an animation frame */
    Timestamp _finish;
    
    /** The number of times fixedUpdate has been called this application */
    Uint64 _fixedCounter;
    /** The time left over after the last call to fixed update */
    Uint32 _fixedRemainder;
    
    /** Counter to assign unique keys to callbacks */
    Uint32 _funcid;
    
    /** Callback functions (processed at the start of every loop) */
    std::unordered_map<Uint32, scheduable> _callbacks;
    /** A mutex lock for the schedule queue */
    std::mutex _queueMutex;
    /**
     * Processes all of the scheduled callback functions.
     *
     * This method wakes up any sleeping callbacks that should be executed.
     * If they are a one time callback, they are deleted.  If they are
     * a reoccuring callback, the timer is reset.
     *
     * @param millis    The number of milliseconds since last called
     */
    void processCallbacks(Uint32 millis);
    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates a degnerate application with no OpenGL context
     *
     * You must initialize the application to use it.  However, you may
     * set any of the attributes before initialization.
     */
    Application();
    
    /**
     * Deletes this application, disposing of all resources
     */
    ~Application() { dispose(); }
    
    /**
     * Disposes all of the resources used by this application.
     *
     * A disposed Application has no OpenGL context, and cannot be used.  
     * However, it can be safely reinitialized.
     */
    virtual void dispose();
    
    /**
     * Initializes this application, creating an OpenGL context.
     *
     * The initialization will use the current value of all of the attributes,
     * like application name, orientation, and size.  These values should be
     * set before calling init().
     *
     * CUGL only supports one application running at a time.  This method will
     * fail if there is another application object.
     *
     * You should not override this method to initialize user-defined attributes.
     * Use the method onStartup() instead.
     *
     * @return true if initialization was successful.
     */
    bool init();
    
    /**
     * Returns the current running application
     *
     * There can only be one application running a time.  While this should
     * never happen, this method will return nullptr if there is no application.
     *
     * @return the current running application
     */
    static Application* get() { return _theapp; }
    
    
#pragma mark -
#pragma mark Virtual Methods
    /**
     * The method called after OpenGL is initialized, but before running the application.
     *
     * This is the method in which all user-defined program intialization should
     * take place.  You should not create a new init() method.
     *
     * When overriding this method, you should call the parent method as the
     * very last line.  This ensures that the state will transition to FOREGROUND,
     * causing the application to run.
     */
    virtual void onStartup();
    
    /**
     * The method called when the application is ready to quit.
     *
     * This is the method to dispose of all resources allocated by this 
     * application.  As a rule of thumb, everything created in onStartup()
     * should be deleted here.
     *
     * When overriding this method, you should call the parent method as the
     * very last line.  This ensures that the state will transition to NONE,
     * causing the application to be deleted.
     */
    virtual void onShutdown();
    
    /**
     * The method called when you are running out of memory.
     *
     * When this method is called, you should immediately free memory or cause
     * the application to quit. Texture memory is generally the biggest 
     * candidate for freeing memory; delete all textures you are not using.
     *
     * When overriding this method, you do not need to call the parent method
     * at all. The default implmentation does nothing.
     */
    virtual void onLowMemory() { }

    /**
     * The method called when the application is suspended and put in the background.
     *
     * When this method is called, you should store any state that you do not
     * want to be lost.  There is no guarantee that an application will return
     * from the background; it may be terminated instead.
     *
     * If you are using audio, it is critical that you pause it on suspension.
     * Otherwise, the audio thread may persist while the application is in
     * the background.
     */
    virtual void onSuspend() { }
    
    /**
     * The method called when the application resumes and put in the foreground.
     *
     * If you saved any state before going into the background, now is the time
     * to restore it. This guarantees that the application looks the same as
     * when it was suspended.
     *
     * If you are using audio, you should use this method to resume any audio
     * paused before app suspension.
     */
    virtual void onResume()  { }
    
    /**
     * The method called to update the application during a non-deterministic loop.
     *
     * This is method is provided your core application loop, provided that 
     * {@link #setDeterministic} is false. This method should be replaced with 
     * your custom implementation to define your application. This method should 
     * contain any code that is not an explicit drawing call. When overriding 
     * this method, you do not need to call the parent method at all. The 
     * default implmentation does nothing.
     *
     * This method is not invoked if {@link #setDeterministic} is set to true. 
     * In that case, the application uses {@link #preUpdate}, {@link #fixedUpdate}, 
     * and {@link #postUpdate} instead.
     *
     * Note that the time passed as a parameter is the time measured from the
     * start of the previous frame to the start of the current frame. It is
     * measured before any input or callbacks are processed.
     *
     * @param dt    The amount of time (in seconds) since the last frame
     */
    virtual void update(float dt) { }
    
    /**
     * The method called to indicate the start of a deterministic loop.
     *
     * This method is used instead of {@link #update} if {@link #setDeterministic}  
     * is set to true. It marks the beginning of the core application loop, 
     * which is concluded with a call to {@link #postUpdate}. 
     *
     * This method should be used to process any events that happen early in 
     * the application loop, such as user input or events created by the 
     * {@link schedule} method. In particular, no new user input will be 
     * recorded between the time this method is called and {@link #postUpdate} 
     * is invoked.
     *
     * Note that the time passed as a parameter is the time measured from the
     * start of the previous frame to the start of the current frame. It is
     * measured before any input or callbacks are processed. It agrees with
     * the value sent to {@link #postUpdate} this animation frame.
     *
     * @param dt    The amount of time (in seconds) since the last frame
     */
    virtual void preUpdate(float dt) { }
    
    /**
     * The method called to provide a deterministic application loop.
     *
     * This method provides an application loop that runs at a guaranteed fixed
     * timestep. This method is (logically) invoked every {@link getFixedStep}
     * microseconds. By that we mean if the method {@link draw} is called at
     * time T, then this method is guaranteed to have been called exactly 
     * floor(T/s) times this session, where s is the fixed time step.
     *
     * This method is always invoked in-between a call to {@link #preUpdate} 
     * and {@link #postUpdate}. However, to guarantee determinism, it is 
     * possible that this method is called multiple times between those two
     * calls. Depending on the value of {@link #getFixedStep}, it can also 
     * (periodically) be called zero times, particularly if {@link #getFPS} 
     * is much faster. 
     *
     * As such, this method should only be used for portions of the application
     * that must be deterministic, such as the physics simulation. It should
     * not be used to process user input (as no user input is recorded between
     * {@link #preUpdate} and {@link #postUpdate}) or to animate models.
     */
    virtual void fixedUpdate() { }

    /**
     * The method called to indicate the end of a deterministic loop.
     *
     * This method is used instead of {@link #update} if {@link #setDeterministic}  
     * is set to true. It marks the end of the core application loop, which was 
     * begun with a call to {@link #preUpdate}. 
     *
     * This method is the final portion of the update loop called before any
     * drawing occurs. As such, it should be used to implement any final 
     * animation in response to the simulation provided by {@link #fixedUpdate}.
     * In particular, it should be used to interpolate any visual differences
     * between the the simulation timestep and the FPS.
     *
     * This method should not be used to process user input, as no new input
     * will have been recorded since {@link #preUpdate} was called.
     *
     * Note that the time passed as a parameter is the time measured from the
     * start of the previous frame to the start of the current frame. It is
     * measured before any input or callbacks are processed. It agrees with
     * the value sent to {@link #preUpdate} this animation frame.
     *
     * @param dt    The amount of time (in seconds) since the last frame
     */
    virtual void postUpdate(float dt) { }
    
    /**
     * The method called to draw the application to the screen.
     *
     * This is your core loop and should be replaced with your custom implementation.
     * This method should OpenGL and related drawing calls.
     *
     * When overriding this method, you do not need to call the parent method
     * at all. The default implmentation does nothing.
     */
    virtual void draw() { }

    
#pragma mark -
#pragma mark Application Loop
    /**
     * Gathers SDL input and distributes it to the event handlers.
     *
     * Input is gathered at the state of the animation frame, before update
     * is called.  As it sends all of its information to the appropriate
     * handlers, you should never need to override this method.
     *
     * @return false if the input indicates that the application should quit.
     */
    bool getInput();

    /**
     * Processes a single animation frame.
     *
     * This method processes the input, calls the update method, and then
     * draws it.  It also updates any running statics, like the average FPS.
     *
     * @return false if the application should quit next frame
     */
    bool step();
    
    /** 
     * Cleanly shuts down the application.
     *
     * This method will shutdown the application in a way that is guaranteed
     * to call onShutdown() for clean-up.  You should use this method instead
     * of a general C++ exit() function.
     *
     * This method uses the SDL event system.  Therefore, the application will
     * quit at the start of the next animation frame, when all events are
     * processed.
     */
    void quit();
    
    /**
     * Schedules a reoccuring callback function time milliseconds in the future.
     *
     * This method allows the user to delay an operation until a certain
     * length of time has passed.  If time is 0, it will be called the next
     * animation frame.  Otherwise, it will be called the first animation
     * frame equal to or more than time steps in the future (so there is
     * no guarantee that the callback will be invoked at exactly time
     * milliseconds in the future).
     *
     * The callback will only be executed on a regular basis. Once it is 
     * called, the timer will be reset and it will not be called for another
     * time milliseconds.  If the callback started late, that extra time
     * waited will be credited to the next call.
     *
     * The callback is guaranteed to be executed in the main thread, so it
     * is safe to access the drawing context or any low-level SDL operations.
     * It will be executed after the input has been processed, but before
     * either {@link #update} or {@link #preUpdate} are invoked.
     *
     * @param callback  The callback function
     * @param time      The number of milliseconds to delay the callback.
     *
     * @return a unique identifier for the schedule callback
     */
    Uint32 schedule(std::function<bool()> callback, Uint32 time=0);

    /**
     * Schedules a reoccuring callback function time milliseconds in the future.
     *
     * This method allows the user to delay an operation until a certain
     * length of time has passed.  If time is 0, it will be called the next
     * animation frame.  Otherwise, it will be called the first animation
     * frame equal to or more than time steps in the future (so there is
     * no guarantee that the callback will be invoked at exactly time
     * milliseconds in the future).
     *
     * The callback will only be executed on a regular basis.  Once it is
     * called, the timer will be reset and it will not be called for another
     * period milliseconds.  Hence it is possible to delay the callback for
     * a long time, but then have it execute every frame. If the callback 
     * started late, that extra time waited will be credited to the next call.
     *
     * The callback is guaranteed to be executed in the main thread, so it
     * is safe to access the drawing context or any low-level SDL operations.
     * It will be executed after the input has been processed, but before
     * either {@link #update} or {@link #preUpdate} are invoked.
     *
     * @param callback  The callback function
     * @param time      The number of milliseconds to delay the callback.
     * @param period	The delay until the callback is executed again.
     *
     * @return a unique identifier for the schedule callback
     */
    Uint32 schedule(std::function<bool()> callback, Uint32 time, Uint32 period);
    
    /**
     * Stops a callback function from being executed.
     *
     * This method may be used to disable a reoccuring callback. If called
     * soon enough, it can also disable a one-time callback that is yet to
     * be executed. Once unscheduled, a callback must be re-scheduled in
     * order to be activated again.
     *
     * The callback is identified by the unique identifier returned by the
     * appropriate schedule function.  Hence this value should be saved if
     * you ever wish to unschedule a callback.
     *
     * @param id    The callback identifier
     */
    void unschedule(Uint32 id);

    
#pragma mark -
#pragma mark Initialization Attributes
    /**
     * Sets the screen size of this application.
     *
     * If the application is set to be full screen, this value will be ignored.
     * Instead, the application size will be the same as the {@link Display}.
     *
     * This method may only be safely called before the application is
     * initialized.  Once the application is initialized; this value may not
     * be changed.
     *
     * @param width     The screen width
     * @param height    The screen height
     */
    void setDisplaySize(int width, int height);

    /**
     * Returns the screen width of this application.
     *
     * This value is in pixels, representing the safe view port size of the
     * OpenGL context.  It is never changed, even if the orientation of
     * the device changes.  For display information that takes into account
     * the current device orientation, see {@link Display}.
     *
     * @return the screen width of this application.
     */
    int getDisplayWidth()  const { return (int)_display.size.width; }
    
    /**
     * Sets the position of the game window
     *
     * If the application is set to be full screen, this value will be ignored.
     * Otherwise, this value specifies the location of the top left corner of
     * the window on the display. By default, a window is centered in the
     * display.
     *
     * This method may only be safely called before the application is
     * initialized.  Once the application is initialized; this value may not
     * be changed.
     *
     * @param x     The x-coordinate of the top-left corner
     * @param y     The y-coordinate of the top-left corner
     */
    void setDisplayPosition(int x, int y);
    
    /**
     * Returns the screen height of this application.
     *
     * This value is in pixels, representing the safe view port size of the
     * OpenGL context.  It is never changed, even if the orientation of
     * the device changes.  For display information that takes into account
     * the current device orientation, see {@link Display}.
     *
     * @return the screen height of this application.
     */
    int getDisplayHeight() const { return (int)_display.size.height; }
    
    /**
     * Returns the screen size of this application.
     *
     * This value is in pixels, representing the safe view port size of the
     * OpenGL context.  It is never changed, even if the orientation of
     * the device changes.  For display information that takes into account
     * the current device orientation, see {@link Display}.
     *
     * @return the screen size of this application.
     */
    Size getDisplaySize() const { return _display.size; }

    /**
     * Returns the screen bounds of this application.
     *
     * If the application is running in windowed mode on a desktop, the
     * bounds origin is the position of the bottom left corner of the window.
     * Otherwise the origin is (0,0).
     *
     * This value is in pixels, representing the view port size of the
     * OpenGL context.  It is never changed, even if the orientation of
     * the device changes.  For display information that takes into account
     * the current device orientation, see {@link Display}.
     *
     * @return the screen bounds of this application.
     */
    Rect getDisplayBounds() const { return _display; }

    /**
     * Returns the safe area of this application.
     *
     * The safe area is a subset of {@link getDisplayBounds()} that is safe
     * for UI and interactive elements.  For phones with notches or rounded
     * corners, it removes those areas that may be hidden.
     *
     * This value is in pixels, representing the safe view port size of the
     * OpenGL context.  It is never changed, even if the orientation of
     * the device changes.  For display information that takes into account
     * the current device orientation, see {@link Display}.
     *
     * @return the safe area of this application.
     */
    Rect getSafeBounds() const { return _safearea; }
    
    /**
     * Sets whether this application is running fullscreen
     *
     * Mobile devices must always run fullscreen, and can never be windowed. In
     * addition, this method may only be safely called before the application
     * is initialized.  Once the application is initialized; this value may not
     * be changed.
     *
     * On desktop/laptop platforms, going fullscreen will hide the mouse.
     * The mouse cursor is only visible in windowed mode.
     *
     * @param value  Whether this application is running fullscreen
     */
    void setFullscreen(bool value);
    
    /**
     * Returns true if this application is running fullscreen
     *
     * Mobile devices must always run fullscreen, and can never be windowed.
     *
     * On desktop/laptop platforms, going fullscreen will hide the mouse.
     * The mouse cursor is only visible in windowed mode.
    *
     * @return true if this application is running fullscreen
     */
    bool isFullscreen() const { return _fullscreen; }

    /**
     * Sets whether this application supports high dpi resolution.
     *
     * For devices that have high dpi screens (e.g. a pixel ration greater
     * than 1), this will enable that feature.  Otherwise, this value will
     * do nothing.
     *
     * Setting high dpi to true is highly recommended for devides that support
     * it (e.g. iPhones).  It makes the edges of textures much smoother.
     * However, rendering is slightly slower as it effectively doubles (and in
     * some cases triples) the resolution.
     *
     * This method may only be safely called before the application is
     * initialized.  Once the application is initialized; this value may not
     * be changed.
     *
     * @param highDPI   Whether to enable high dpi
     */
    void setHighDPI(bool highDPI);
    
    /**
     * Returns true if this application supports high dpi resolution.
     *
     * For devices that have high dpi screens (e.g. a pixel ration greater
     * than 1), this will enable that feature.  Otherwise, this value will
     * do nothing.
     *
     * Setting high dpi to true is highly recommended for devides that support
     * it (e.g. iPhones).  It makes the edges of textures much smoother.
     * However, rendering is slightly slower as it effectively doubles (and in
     * some cases triples) the resolution.
     *
     * @return true if this application supports high dpi resolution.
     */
    bool isHighDPI() const { return _highdpi; }
  
    /**
     * Sets whether this application supports graphics multisampling.
     *
     * Multisampling adds anti-aliasing to OpenGL so that polygon edges are
     * not so hard and jagged.  This does add some extra overhead, and is
     * not really necessary on Retina or high DPI displays.  However, it is
     * pretty much a must in Windows and normal displays.
     *
     * By default, this is false on any platform other than Windows.
     *
     * @param flag	Whether this application should support graphics multisampling.
     */
    void setMultiSampled(bool flag);

    /**
     * Returns true if this application supports graphics multisampling.
     *
     * Multisampling adds anti-aliasing to OpenGL so that polygon edges are
     * not so hard and jagged.  This does add some extra overhead, and is
     * not really necessary on Retina or high DPI displays.  However, it is
     * pretty much a must in Windows and normal displays.
     *
     * By default, this is false on any platform other than Windows.
     *
     * @return true if this application supports graphics multisampling.
     */
    bool isMultiSampled() const { return _multisamp; }

#pragma mark -
#pragma mark Runtime Attributes
    /**
     * Sets the name of this application
     *
     * On a desktop, the name will be displayed at the top of the window. The
     * name also defines the preferences directory -- the place where it is 
     * safe to write save files.
     *
     * This method may be safely changed at any time while the application
     * is running.
     *
     * @param name  The name of this application
     */
    void setName(const char* name);
    
    /**
     * Sets the name of this application
     *
     * On a desktop, the name will be displayed at the top of the window. The
     * name also defines the preferences directory -- the place where it is
     * safe to write save files.
     *
     * This method may be safely changed at any time while the application
     * is running.
     *
     * @param name  The name of this application
     */
    void setName(const std::string name);
    
    /**
     * Returns the name of this application
     *
     * On a desktop, the name will be displayed at the top of the window. The
     * name also defines the preferences directory -- the place where it is
     * safe to write save files.
     *
     * @return the name of this application
     */
    const std::string getName() const { return _name; }
    
    /**
     * Sets the organization name for this application
     *
     * This name defines the preferences directory -- the place where it is
     * safe to write save files. Applications of the same organization will
     * save in the same location.
     *
     * This method may be safely changed at any time while the application
     * is running.
     *
     * @param name  The organization name for this application
     */
    void setOrganization(const char* name);
    
    /**
     * Sets the organization name for this application
     *
     * This name defines the preferences directory -- the place where it is
     * safe to write save files. Applications of the same organization will
     * save in the same location.
     *
     * This method may be safely changed at any time while the application
     * is running.
     *
     * @param name  The organization name for this application
     */
    void setOrganization(const std::string name);
    
    /**
     * Returns the organization name for this application
     *
     * This name defines the preferences directory -- the place where it is
     * safe to write save files. Applications of the same organization will
     * save in the same location.
     *
     * @return the organization name for this application
     */
    const std::string getOrganization() const { return _org; }
    
    /**
     * Returns true if this application obeys the display refresh rate
     *
     * A vsync-enabled application will always match the refresh rate of
     * the display. Otherwise, the application will attempt to match the
     * value of {@link #getFPS()}, which could be faster than the refresh
     * rate.
     *
     * Note that some platforms (notably macOS) will always use vsync
     * no matter the settings. In that case, setting this value to false
     * will actually hurt the performance of your application. As a
     * general rule, it is best to set this value to true, and perform
     * any simulations that must be done at a faster rate in a separate
     * thread.
     *
     * @return true if this application obeys the display refresh rate
     */
    bool getVSync() const { return _vsync; }
    
    /**
     * Sets whether this application obeys the display refresh rate
     *
     * A vsync-enabled application will always match the refresh rate of
     * the display. Otherwise, the application will attempt to match the
     * value of {@link #getFPS()}, which could be faster than the refresh
     * rate.
     *
     * Note that some platforms (notably macOS) will always use vsync
     * no matter the settings. In that case, setting this value to false
     * will actually hurt the performance of your application. As a
     * general rule, it is best to set this value to true, and perform
     * any simulations that must be done at a faster rate in a separate
     * thread.
     *
     * @param vsync Whether this application obeys the display refresh rate
     */
    void setVSync(bool vsync);
    
    /**
     * Sets the target frames per second of this application.
     *
     * The application does not guarantee that the fps target will always be
     * met. In particular, if the {@link #update} and {@link #draw} methods 
     * are expensive, it may run slower. In addition, if vsync is enabled, 
     * it may be ignored entirely (especially if the refresh rate is faster).
     *
     * Note that the FPS is distinct from {@link #setFixedStep}, which
     * sets the timestep of a deterministic loop. The FPS sets the draw speed,
     * not the simulation speed.
     *
     * This method may be safely changed at any time while the application
     * is running. By default, this value will match the refresh rate of the
     * display, even if vsync is disabled.
     *
     * @param fps   The target frames per second
     */
    void setFPS(float fps);
    
    /**
     * Returns the target frames per second of this application.
     *
     * The application does not guarantee that the fps target will always be
     * met. In particular, if the {@link #update} and {@link #draw} methods 
     * are expensive, it may run slower. In addition, if vsync is enabled, 
     * it may be ignored entirely (especially if the refresh rate is faster).
     *
     * Note that the FPS is distinct from {@link #setFixedStep}, which
     * sets the timestep of a deterministic loop. The FPS sets the draw speed,
     * not the simulation speed.
     *
     * This method may be safely changed at any time while the application
     * is running. By default, this value will match the refresh rate of the
     * display, even if vsync is disabled.
     *
     * @return the target frames per second of this application.
     */
    float getFPS() const { return _fps; }
    
    /**
     * Returns the average frames per second over the last 10 frames.
     *
     * The method provides a way of computing the curren frames per second that
     * smooths out any one-frame anomolies.  The FPS is averages over the 
     * exact rate of the past 10 frames.
     *
     * @return the average frames per second over the last 10 frames.
     */
    float getAverageFPS() const;
    
    /**
     * Sets the simulation timestep of this application.
     *
     * The value defines the rate at which {@link #fixedUpdate} is called.
     * The rate is a logical value, not a wall-clock value.  That is, if
     * {@link #draw} is called at time T, then the method {@link #fixedUpdate}
     * will have been called T/s times, where s is the simulation timestep.
     *
     * This timestep is set in microseconds for the purposes of precision.
     * Note that this value does nothing if {@link #setDeterministic} is set
     * to false. In that case, {@link #fixedUpdate} is never called, and
     * {@link #update} is called instead.
     *
     * This method may be safely changed at any time while the application
     * is running. By default, this value will match the initial FPS of the
     * application.
     *
     * @param step  The simulation timestep
     */
    void setFixedStep(Uint64 step);
    
    /**
     * Returns the simulation timestep of this application.
     *
     * The value defines the rate at which {@link #fixedUpdate} is called.
     * The rate is a logical value, not a wall-clock value.  That is, if
     * {@link #draw} is called at time T, then the method {@link #fixedUpdate}
     * will have been called T/s times, where s is the simulation timestep.
     *
     * This timestep is set in microseconds for the purposes of precision.
     * Note that this value does nothing if {@link #setDeterministic} is set
     * to false. In that case, {@link #fixedUpdate} is never called, and
     * {@link #update} is called instead.
     *
     * This method may be safely changed at any time while the application
     * is running. By default, this value will match the initial FPS of the
     * application.
     *
     * @return the simulation timestep of this application.
     */
    long getFixedStep() const { return _fixstep; }
    
    /**
     * Instructs the application to use the deterministic loop.
     *
     * If this value is set to false, then the application will use the simple
     * structure of alternating between {@link #update} and {@link #draw}.
     * However, if it is set to true, it will use a more complicated loop in
     * place of {@link #update}, consisting of a call to {@link #preUpdate},
     * followed by zero or more calls to {@link #fixedUpdate}.
     *
     * This method may be safely changed at any time while the application
     * is running. By default, this value is false.
     *
     * @param value Whether to use the deterministic loop
     */
    void setDeterministic(bool value);

    /**
     * Returns whether the application uses the deterministic loop.
     *
     * If this value is set to false, then the application will use the simple
     * structure of alternating between {@link #update} and {@link #draw}.
     * However, if it is set to true, it will use a more complicated loop in
     * place of {@link #update}, consisting of a call to {@link #preUpdate},
     * followed by zero or more calls to {@link #fixedUpdate}.
     *
     * This method may be safely changed at any time while the application
     * is running. By default, this value is false.
     *
     * @return whether the application uses the deterministic loop.
     */
    bool isDeterministic() { return _fixed; }
    
    /**
     * Returns the number of times {@link #fixedUpdate} has been called.
     *
     * This value is reset to 0 if {@link #setDeterministic} is set to false.
     *
     * @return the number of times {@link #fixedUpdate} has been called.
     */
    Uint64 getFixedCount() const { return _fixedCounter; }
    
    /**
     * Returns the time "left over" after the call to {@link #fixedUpdate}.
     *
     * If the FPS and the simulation timestep do not perfectly match, the
     * {@link #draw} method will be invoked with some extra time after the
     * last call to {@link #fixedUpdate}. It is useful to know this amount
     * of time for the purposes of interpolation. The value returned is in
     * microseconds.
     *
     * This value is always guaranteed to be less than {@link #getFixedStep}.
     *
     * @return the time "left over" after the call to {@link #fixedUpdate}.
     */
    Uint32 getFixedRemainder() const { return _fixedRemainder; }

    /**
     * Resets the time "left over" for {@link #fixedUpdate} to 0.
     *
     * This method is for when you need to reset a simulation back to its
     * initial state.
     */
    void resetFixedRemainder() { _fixedRemainder = 0; }

    /**
     * Sets the clear color of this application
     *
     * This color is the default background color.  The window will be cleared
     * using this color before draw() is called.
     *
     * This method may be safely changed at any time while the application
     * is running.
     *
     * @param color The clear color of this application
     */
    void setClearColor(Color4 color) { _clearColor = color; }

    /**
     * Returns the clear color of this application
     *
     * This color is the default background color.  The window will be cleared
     * using this color before draw() is called.
     *
     * @return the clear color of this application
     */
    Color4 getClearColor() const { return (Color4)_clearColor; }

    /**
     * Returns the number of total microseconds that have ellapsed
     *
     * This is value is measured from the call to {@link #init} to the current
     * time step. The value is undefined if the application has not been
     * initialized.
     *
     * @return the number of total microseconds that have ellapsed
     */
    Uint64 getEllapsedMicros() const {
        Timestamp now;
        return now.ellapsedMicros(_boot);
    }
    
    /**
     * Returns the current state of this application.
     *
     * This state is guaranteed to be FOREGROUND whenever update() or draw()
     * are called.
     */
    State getState() const { return _state; }
    
    /**
     * Returns a description of this graphics API for this application.
     *
     * For example, if this application is backed by OpenGL, this method
     * returns the OpenGL version. If it is a headless client, it returns
     * "Headless".
     *
     * @return a description of this graphics API for this application.
     */
    const std::string getGraphicsDescription() const;
    
#pragma mark -
#pragma mark File Directories
    /**
     * Returns the base directory for all assets (e.g. the assets folder).
     *
     * The assets folder is a READ-ONLY folder for providing assets for the
     * game.  Its path depends on the platform involved.  Android uses
     * this to refer to the dedicated assets folder, while MacOS/iOS refers
     * to the resource bundle.  On Windows, this is the working directory.
     *
     * The value returned is an absolute path in UTF-8 encoding, and has the
     * appropriate path separator for the given platform ('\\' on Windows, 
     * '/' most other places). In addition, it is guaranteed to end with a
     * path separator, so that you can append a file name to the path.
     *
     * It is possible that the the string is empty.  For example, the assets
     * directory for Android is not a proper directory (unlike the save 
     * directory) and should not be treated as such.
     *
     * Asset loaders use this directory by default.
     *
     * @return the base directory for all assets (e.g. the assets folder).
     */
    std::string getAssetDirectory();
    
    /**
     * Returns the base directory for writing save files and preferences.
     *
     * The save folder is a READ-WRITE folder for storing saved games and 
     * preferences.  The folder is unique to the current user.  On desktop
     * platforms, it is typically in the user's home directory.  You must
     * use this folder (and not the asset folder) if you are writing any
     * files.
     *
     * The value returned is an absolute path in UTF-8 encoding, and has the
     * appropriate path separator for the given platform ('\\' on Windows,
     * '/' most other places). In addition, it is guaranteed to end with a
     * path separator, so that you can append a file name to the path.
     *
     * I/O classes (both readers and writers) use this directory by default.  
     * However, if you are want to use this directory in an asset loader (e.g.
     * for a saved game file), you you may want to refer to the path directly.
     *
     * @return the base directory for writing save files and preferences.
     */
    std::string getSaveDirectory();
};

}


#endif /* __CU_APPLICATION_H__ */
