//
//  GBApp.h
//  Platform Demo
//  This is the root class for your game.  The file main.cpp accesses this class
//  to run the application.  While you could put most of your game logic in
//  this class, we prefer to break the game up into player modes and have a
//  class for each mode.
//
//  This file is based on the CS 3152 PhysicsDemo Lab by Don Holden, 2007
//
//  Author: Walker White and Anthony Perello
//  Version:  2/9/17
//
#include "GBApp.h"

using namespace cugl;
using namespace cugl::graphics;
using namespace cugl::scene2;
using namespace cugl::audio;

#define MAX_SCENE 3

#pragma mark -
#pragma mark Application State
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
void GlitchbladeApp::onStartup() {
    _assets = AssetManager::alloc();
    _batch  = SpriteBatch::alloc();

    // Start-up basic input
#ifdef CU_TOUCH_SCREEN
    Input::activate<Touchscreen>();
#else
    Input::activate<Mouse>();
#endif

    _assets->attach<Font>(FontLoader::alloc()->getHook());
    _assets->attach<Texture>(TextureLoader::alloc()->getHook());
    _assets->attach<Sound>(SoundLoader::alloc()->getHook());
    _assets->attach<SceneNode>(Scene2Loader::alloc()->getHook());
    _assets->attach<WidgetValue>(WidgetLoader::alloc()->getHook());
    _assets->loadDirectory("json/loading.json");

    // Create a "loading" screen
    _loaded = false;
    _loading.init(_assets,"json/assets.json");
    _loading.setSpriteBatch(_batch);

    // Set background color to black
    setClearColor(Color4f::BLACK);

    // Queue up the other assets
    _loading.start();
    AudioEngine::start();
    Application::onStartup(); // YOU MUST END with call to parent
}

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
void GlitchbladeApp::onShutdown() {
    _loading.dispose();
    if (_gameplay != nullptr)
        _gameplay->dispose();
    _assets = nullptr;
    _batch = nullptr;

    // Shutdown input
#ifdef CU_TOUCH_SCREEN
    Input::deactivate<Touchscreen>();
#else
    Input::deactivate<Mouse>();
#endif

    AudioEngine::stop();
    Application::onShutdown();  // YOU MUST END with call to parent
}

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
void GlitchbladeApp::onSuspend() {
    AudioEngine::get()->pause();
}

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
void GlitchbladeApp::onResume() {
    AudioEngine::get()->resume();
}

// TODO TODO TODO TODO call this when ready
// initGameScene("Level 1");

#pragma mark -
#pragma mark Application Loop

/**
 * The method called to update the application data.
 *
 * This is your core loop and should be replaced with your custom implementation.
 * This method should contain any code that is not an OpenGL call.
 *
 * When overriding this method, you do not need to call the parent method
 * at all. The default implmentation does nothing.
 *
 * @param dt    The amount of time (in seconds) since the last frame
 */
void GlitchbladeApp::update(float dt) {
    if (!_loaded && _loading.isActive()) {
        _loading.update(0.01f);
    } else if (!_loaded) {
        _loading.dispose(); // Disables the input listeners in this mode
        _loaded = true;
        loadProgress();
        setDeterministic(true);
        initLevelSelectScene();
    }
}

// Create and init _gameplay
void GlitchbladeApp::initGameScene(int levelNum) {
    _levelSelect = nullptr;
    _gameplay = std::make_shared<GameScene>();
    _gameplay->init(_assets, levelNum);
    _gameplay->setLevelCompleteCallback([this, levelNum]() {
        this->onLevelCompleted(levelNum);
        });
    _gameplay->setSpriteBatch(_batch);
}

void GlitchbladeApp::onLevelCompleted(int levelNum) {
    CULog("On level %d completed; saving progress", levelNum);

    size_t index = static_cast<size_t>(levelNum - 1);
    if (index < _levelComplete.size()) {
        _levelComplete[index] = true;
        saveProgress();
    }
    else {
        CULogError("Invalid levelNum %d in onLevelCompleted", levelNum);
    }
}

void GlitchbladeApp::initLevelSelectScene() {
    _gameplay = nullptr;
    _levelSelect = std::make_shared<LevelSelectScene>();
    _levelSelect->init(_assets);
    _levelSelect->setSpriteBatch(_batch);
}

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
void GlitchbladeApp::preUpdate(float dt) {
    if (_gameplay != nullptr)
        _gameplay->preUpdate(dt);
}

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
void GlitchbladeApp::fixedUpdate() {
    // Compute time to report to game scene version of fixedUpdate
    float time = getFixedStep()/1000000.0f;
    if (_gameplay != nullptr)
        _gameplay->fixedUpdate(time);
}

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
void GlitchbladeApp::postUpdate(float dt) {
    // Compute time to report to game scene version of postUpdate
    float time = getFixedRemainder()/1000000.0f;
    if (_gameplay != nullptr)
        _gameplay->postUpdate(time);
}

/**
 * Load existing game save
 */
void GlitchbladeApp::loadProgress() {
    std::string saveDir = cugl::Application::getSaveDirectory();
    std::string saveFile = saveDir + "levelProgress.dat";

    int expectedNumOfLevels = 20;

    cugl::BinaryReader reader;
    if (reader.init(saveFile)) {
        if (reader.ready(sizeof(Uint32))) {
            Uint32 count = reader.readUint32();
            _levelComplete.assign(count, false);
            for (Uint32 i = 0; i < count && reader.ready(); ++i) {
                _levelComplete[i] = (reader.readByte() != 0);
            }
        }
        else {
            _levelComplete.assign(expectedNumOfLevels, false);
        }
        reader.close();
    }
    else {
        _levelComplete.assign(expectedNumOfLevels, false);
    }

    bool anyDone = false;
    for (size_t i = 0; i < _levelComplete.size(); ++i) {
        if (_levelComplete[i]) {
            // CULog("Level %zu is completed", i + 1);
            anyDone = true;
        }
    }
    /*if (!anyDone) {
        CULog("No levels completed yet");
    }*/
}


void GlitchbladeApp::saveProgress() {
    std::string saveDir = cugl::Application::getSaveDirectory();
    std::string saveFile = saveDir + "levelProgress.dat";

    cugl::BinaryWriter writer;
    if (!writer.init(saveFile)) {
        CULogError("Unable to open save file for writing: %s", saveFile.c_str());
        return;
    }

    Uint32 count = static_cast<Uint32>(_levelComplete.size());
    writer.writeUint32(count);

    for (bool done : _levelComplete) {
        writer.writeUint8(done ? 1 : 0);
    }

    writer.flush();
    writer.close();
}

/**
 * The method called to draw the application to the screen.
 *
 * This is your core loop and should be replaced with your custom implementation.
 * This method should OpenGL and related drawing calls.
 *
 * When overriding this method, you do not need to call the parent method
 * at all. The default implmentation does nothing.
 */
void GlitchbladeApp::draw() {
    if (!_loaded) {
        _loading.render();
    } else {
        if (_gameplay != nullptr) {

            _gameplay->render();

            if (_gameplay->doQuit()) {
                initLevelSelectScene();
            }
            else if (_gameplay->continueNextLevel()) {
                _currentScene += 1;
                if (_currentScene <= MAX_SCENE) {
                    initGameScene(_currentScene);
                } else {
                    CULog("Did all levels!");
                    initLevelSelectScene();
                }
            }
        }
        else if (_levelSelect != nullptr) {
            _levelSelect->render();
            int scene_num = _levelSelect->sceneToLoad();
            if (scene_num != 0) {
                _currentScene = scene_num;
                initGameScene(scene_num);
            }
        }
    }
}

