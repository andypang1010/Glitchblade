//
//  GBGameScene.cpp
//  PlatformDemo
//
//  This is the most important class in this demo.  This class manages the gameplay
//  for this demo.  It also handles collision detection. There is not much to do for
//  collisions; our ObstacleWorld class takes care of all of that for us.  This
//  controller mainly transforms input into gameplay.
//
//  WARNING: There are a lot of shortcuts in this design that will do not adapt
//  well to data driven design. This demo has a lot of simplifications to make
//  it a bit easier to see how everything fits together. However, the model
//  classes and how they are initialized will need to be changed if you add
//  dynamic level loading.
//
//  This file is based on the CS 3152 PhysicsDemo Lab by Don Holden (2007).
//  This file has been refactored to support the physics changes in CUGL 2.5.
//
//  Author: Walker White and Anthony Perello
//  Version:  2/9/24
//
#include "GBGameScene.h"
#include <box2d/b2_world.h>
#include <box2d/b2_contact.h>
#include <box2d/b2_collision.h>
#include "../player/GBPlayerModel.h"
#include "../enemies/GBEnemyModel.h"
#include "objects/GBProjectile.h"
#include "../enemies/actionmodel_variants/GBMeleeActionModel.h"
#include "ui/GBIngameUI.h"
#include "../core/GBAudio.h"

#include <ctime>
#include <string>
#include <iostream>
#include <sstream>
#include <random>

using namespace cugl;
using namespace cugl::graphics;
using namespace cugl::physics2;
using namespace cugl::audio;



#define directions
Vec2 LEFT = { -1.0f, 0.0f };
Vec2 RIGHT = { 1.0f, 0.0f };
Vec2 UP = { 0.0f, 1.0f };
Vec2 DOWN = { 0.0f, -1.0f };
#pragma mark -
#pragma mark Constructors
/**
 * Creates a new game world with the default values.
 *
 * This constructor does not allocate any objects or start the controller.
 * This allows us to use a controller without a heap pointer.
 */
GameScene::GameScene() : Scene2(),
_worldnode(nullptr),
_debugnode(nullptr),
_world(nullptr),
_levelController(nullptr),
_complete(false),
_debug(false),
_input(nullptr)
{
}

/**
 * Initializes the controller contents, and starts the game
 *
 * The constructor does not allocate any objects or memory.  This allows
 * us to have a non-pointer reference to this controller, reducing our
 * memory allocation.  Instead, allocation happens in this method.
 *
 * The game world is scaled so that the screen coordinates do not agree
 * with the Box2d coordinates.  The bounds are in terms of the Box2d
 * world, not the screen.
 *
 * @param assets    The (loaded) assets for this game mode
 * @return  true if the controller is initialized properly, false otherwise.
 */
bool GameScene::init(const std::shared_ptr<AssetManager>& assets, int levelNum) {
    if (assets == nullptr) {
        return false;
    }

    _levelNum = levelNum;
    std::string levelName = "Level " + std::to_string(levelNum);
    _assets = assets;
    // prepare constants
    std::shared_ptr<JsonReader> constants_reader = JsonReader::allocWithAsset("json/constants.json");
    _constantsJSON = constants_reader->readJson();
    if (_constantsJSON == nullptr) {
        CULog("Failed to load constants.json");
        return false;
    }
    
    std::shared_ptr<JsonValue> fxJ = _constantsJSON->get("audio")->get("effects");
    AudioHelper::init(fxJ, assets);
    std::shared_ptr<JsonValue> sceneJ = _constantsJSON->get("scene");
    
    // Uncomment this part to rollback.
//    if (!Scene2::initWithHint(Size(sceneJ->getInt("width"), sceneJ->getInt("height")))) {
//        return false;
//    }
    // Uncomment this part to rollback. // initWithHint will scale the scene based on height by default, which usually is not correct.
    
    // Comment this part to rollback.
    if (!Scene2::init()) {
        return false;
    }
    // Comment this part to rollback.
    
    _offset = Vec2((_size.width - Application::get()->getDisplayWidth()) / 2.0f, (_size.height - Application::get()->getDisplayHeight()) / 2.0f);
    Rect bounds = getBounds();
    std::shared_ptr<JsonValue> boundsJ = sceneJ->get("bounds");
    boundsJ->get("origin")->get("x")->set(bounds.origin.x);
    boundsJ->get("origin")->get("y")->set(bounds.origin.y);
    boundsJ->get("size")->get("width")->set(bounds.size.width);
    boundsJ->get("size")->get("height")->set(bounds.size.height);
    
    // Create the world and attach the listeners.
    Rect worldSize = Rect(0, 0, sceneJ->getFloat("world_width"), sceneJ->getFloat("world_height"));
    Rect screenSize = worldSize;
    screenSize.size.width /= 3*(worldSize.size.width/117);
    Vec2 gravity = Vec2(0.0,-28.9);
    
    
    // IMPORTANT: SCALING MUST BE UNIFORM
    // This means that we cannot change the aspect ratio of the physics world
    // Shift to center if a bad fit
    //_scale = _size.width == sceneJ->getInt("width") ? _size.width / screenSize.size.width : _size.height / screenSize.size.height;
    // use the smaller scale to ensure the whole scene is in the screen.
    float scaleX = _size.width / screenSize.size.width;
    float scaleY = _size.height / screenSize.size.height;
    _scale = std::min(scaleX, scaleY);


    _worldPixelWidth = worldSize.size.width * _scale;
    sceneJ->get("scale")->set(_scale);
    
    
    _world = physics2::ObstacleWorld::alloc(worldSize, gravity);
    _world->activateCollisionCallbacks(true);
    _world->onBeginContact = [this](b2Contact* contact) {
        _collisionController->beginContact(contact);
        };
    _world->onEndContact = [this](b2Contact* contact) {
        _collisionController->endContact(contact);
        };
    
    
    // Create the scene graph
    _worldnode = scene2::SceneNode::alloc();
    _worldnode->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    _worldnode->setPosition(_offset);
//    _worldnode->setScale(_scale); //Cannot run properly with this line added. Therefore, background and phyisics cannot scale properly.
    
    _camera = getCamera();
    _defCamPos = _camera->getPosition();

    _levelController = std::make_unique<LevelController>();
    _debugnode = scene2::SceneNode::alloc();
    _debugnode->setScale(_scale); // Debug node draws in PHYSICS coordinates
    _debugnode->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    _debugnode->setPosition(_offset);
    _debugnode->setName("game_debug_node");
    
    _levelController->init(_assets,_constantsJSON, _world, _debugnode, _worldnode); // Initialize the LevelController
    _currentLevel = _levelController->getLevelByName(levelName);

    std::shared_ptr<JsonValue> messagesJ = _constantsJSON->get("messages");
    setComplete(false);
    setFailure(false);
    
    #pragma mark : Input (can delete and remove the code using _input in preupdate- only for easier setting of debugging node)
    _input =  _levelController->getInputController();
    #pragma mark : Player
        _player = _levelController->getPlayerModel();
    
    populate(_levelController->getLevelByName(levelName));

    // === Initialize in-game UI ===
    populateUI(assets);
    
    _active = true;
    _complete = false;
    setDebug(false); // Debug on by default

    Application::get()->setClearColor(Color4f::BLACK);
    
    _collisionController = std::make_unique<CollisionController>(
        _player,
        _ui,
        _constantsJSON,
        [this](Projectile* p) { this->removeProjectile(p); },  // Callback for projectile removal
        [this](int i, int d) { this->setScreenShake(i, d); }   // Callback for screen shake
    );

    return true;
}

void GameScene::populateUI(const std::shared_ptr<cugl::AssetManager>& assets)
{
    _ui = GBIngameUI::alloc(_assets);
    if (_ui != nullptr) {
        _ui->setPauseCallback([this]() {
            _shouldPause = true;
        });
        _ui->setResumeCallback([this]() {
            _shouldResume = true;
        });
        _ui->setRetryCallback([this]() {
            _shouldRetry = true;
        });
        _ui->setQuitCallback([this]() {
            CULog("Want to quit from gamescene!");
            _doQuit = true;
        });
        _ui->setSettingCallback([this]() {
            _shouldSetting = true;
        });
        _ui->setWinContinueCallback([this]() {
            _continueNextLevel = true;
            CULog("Want to winContinue from gamescene!");
        });

        addChild(_ui);
    }
    
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameScene::dispose() {

    _world->onEndContact = nullptr;
    _world->onBeginContact = nullptr;

    _input->clear();
    _input->clearListeners();

    removeAllChildren();
    if (_active) {
        _world = nullptr;
        _worldnode = nullptr;
        _debugnode = nullptr;
        _complete = false;
        _debug = false;
		_ui = nullptr;
        Scene2::dispose();
    }
}


#pragma mark -
#pragma mark Level Layout

/**
 * Resets the status of the game by resetting player and enemy positions so that we can play again.
 */
void GameScene::reset() {
	removeAllChildren();
    _world->clear();
    _worldnode->removeAllChildren();
    _debugnode->removeAllChildren();
    setFailure(false);
    setComplete(false);
    _levelController->reset();
    populate(_levelController->getCurrentLevel());
    _cameraLocked = false;
    _camera->setPosition(_defCamPos);
    addChild(_ui);
    _ui->resetUI();

    Application::get()->setClearColor(Color4f::BLACK);
}

/**
 * Lays out the game geography.
 *
 * Pay close attention to how we attach physics objects to a scene graph.
 * The simplest way is to make a subclass, like we do for the Dude.  However,
 * for simple objects you can just use a callback function to lightly couple
 * them.  This is what we do with the crates.
 *
 * This method is really, really long.  In practice, you would replace this
 * with your serialization loader, which would process a level file.
 */
void GameScene::populate(const std::shared_ptr<LevelModel>& level) {
    _currentLevel = level;
    addChild(_worldnode);
    addChild(_debugnode);
    setBG();

    _levelController->populateLevel(level); // Sets the level we want to populate here
    _levelController->updateRightZone(0);
    _levelController->updateLeftZone(0);
    _player = _levelController->getPlayerModel();
    
    _active = true;
    _complete = false;
}

/**
* Sets whether the level is completed.
*
* If true, the level will advance after a countdown
*
* @param value whether the level is completed.
*/
void GameScene::setComplete(bool value) {
    if (_failed)
        return; // Do not win if lost

    bool change = _complete != value;
    _complete = value;
    if (value && change) {

        if (_levelCompleteCallback != nullptr) {
            _levelCompleteCallback(); // Let GBApp know to save the progress
        }

        float timeSpent = _levelController->getTimeSpentInLevel();
        int parryCount = _levelController->getPlayerController()->getPlayer()->_parryCounter;
        int spawnedCount = _levelController->getSpawnedEnemyCount();
        int totalCount = _levelController->getTotalEnemyCount();
        _ui->updateStats(timeSpent, parryCount, spawnedCount, totalCount);
        std::shared_ptr<JsonValue> musicJ = _constantsJSON->get("audio")->get("music");
        std::shared_ptr<Sound> source = _assets->get<Sound>(musicJ->getString("win"));
        AudioEngine::get()->getMusicQueue()->play(source, false, musicJ->getFloat("volume"));
        _ui->showHeadsUpDisplay(false, false);
        _ui->showWinPage(true);
        setPaused(true);
        _countdown = _constantsJSON->getInt("exit_count");
    }
    else if (!value) {
        setPaused(false);
        _countdown = -1;
    }
}

void GameScene::setLevelCompleteCallback(const std::function<void()>& cb) {
    _levelCompleteCallback = cb;
}

/**
 * Sets whether the level is failed.
 *
 * If true, the level will reset after a countdown
 *
 * @param value whether the level is failed.
 */
void GameScene::setFailure(bool value) {
    if (_complete)
        return; // Do not fail if won
    bool change = _complete != value;
    _failed = value;
    if (value && change) {
        std::shared_ptr<JsonValue> musicJ = _constantsJSON->get("audio")->get("music");
        std::shared_ptr<Sound> source = _assets->get<Sound>(musicJ->getString("win"));
        AudioEngine::get()->getMusicQueue()->play(source, false, musicJ->getFloat("volume"));
        
        if (_ui) {
            float timeSpent = _levelController->getTimeSpentInLevel();
            int parryCount = _levelController->getPlayerController()->getPlayer()->_parryCounter;
            int spawnedCount = _levelController->getSpawnedEnemyCount();
            int totalCount = _levelController->getTotalEnemyCount();
            _ui->updateStats(timeSpent, parryCount, spawnedCount, totalCount);
            _ui->showHeadsUpDisplay(false, false);
            _ui->showLosePage(true);
            setPaused(true);
        }
        
        _countdown = _constantsJSON->getInt("exit_count");
    }
    else {
        if (_ui) {
            _ui->showHeadsUpDisplay(true, true);
            _ui->showLosePage(false);
            setPaused(false);
        }
        _countdown = -1;
    }
}


#pragma mark -
#pragma mark Physics Handling
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
void GameScene::preUpdate(float dt) {
//    if (_isPaused) return;
    
    // Process the toggled key commands
    if (_input->didDebug()) { setDebug(!isDebug()); }
    if (_input->didReset()) { reset(); }
    if (_input->didExit()) {
        Application::get()->quit();
    }
    
	_ui->updateHP(_player->getHP());
    _ui->updateComboBar(_player->_comboMeter);

    // Call preUpdate on the LevelController
    _levelController->preUpdate(dt);
    
    if (_shouldPause) {
        _shouldPause = false;
        _ui->showPauseMenu(true);
        _ui->showSettingMenu(false);
        _ui->showHeadsUpDisplay(true, false);
        setPaused(true);
    }
    
    if (_shouldSetting) {
        _shouldSetting = false;
        _ui->showPauseMenu(false);
        _ui->showSettingMenu(true);
        setPaused(true);
    }
    
    if (_shouldResume) {
        _shouldResume = false;
        _ui->showPauseMenu(false);
        _ui->showSettingMenu(false);
        _ui->showHeadsUpDisplay(true, true);
        setPaused(false);
    }
    
    if (_shouldRetry) {
        _shouldRetry = false;
        _ui->showPauseMenu(false);
        _ui->showHeadsUpDisplay(true, true);
        setPaused(false);
        reset();
    }
    
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
 *
 * The time passed to this method is NOT the same as the one passed to
 * {@link #preUpdate}. It will always be exactly the same value.
 *
 * @param step  The number of fixed seconds for this step
 */
void GameScene::fixedUpdate(float step) {
    
    if (_isPaused) return;
    
    // Turn the physics engine crank.
    _world->update(step);

    // Update the level controller
    _levelController->fixedUpdate(step);
    
    processScreenShake();

    auto currPlayerPosX = _levelController->getPlayerNode()->getPositionX();
    bool isKnocked = _levelController->getPlayerModel()->isKnockbackActive();
    auto currPlayerVel = _levelController->getPlayerModel()->getVX();
    auto cameraPosLX = _camera->getPosition().x-_camera->getViewport().size.width / 2;
    auto cameraPosRX = _camera->getPosition().x + _camera->getViewport().size.width / 2;
    float leftBound = _levelController->getLeftWall()->xPosition;
    float rightBound = _levelController->getRightWall()->xPosition;
    
    if (_levelController->getZoneUpdate()) {
        _cameraLocked = false;
        if (cameraPosLX >= _levelController->getNextTrigger()) {
            _levelController->setInNextZone(true);
        }
    }
    
    CULog("PlayerPos: %f", currPlayerPosX);
    CULog("CamLPos: %f", cameraPosLX);
    CULog("CamLPos: %f", cameraPosRX);
    CULog("CamLocked?: %d", _cameraLocked);
    for (auto obs : _world->getObstacles()) {
        
    }
    if (_levelController->getZoneUpdate()) {
        CULog("NextTriggerPos: %f", _levelController->getNextTrigger());
    }

    if (cameraPosLX < leftBound || cameraPosRX >= rightBound) {
        _cameraLocked = true;
        if (cameraPosLX <= leftBound) {
            if (currPlayerPosX > leftBound + _camera->getViewport().size.width*.66) {
                _cameraLocked = false;
            }
        } else {
            if (currPlayerPosX < rightBound - _camera->getViewport().size.width*.66) {
                _cameraLocked = false;
            }
        }
    }
    if (!_cameraLocked) {
        _camera->translate(Vec2((currPlayerPosX-_camera->getPosition().x)*.05,0));
        _camera->update();
        if (currPlayerVel > 0 && !isKnocked) {
            updateLayersLeft();
        }   else if (currPlayerVel < 0 && !isKnocked) {
            updateLayersRight();
        }
    }

    if (_ui != nullptr && _camera != nullptr) {
        Vec2 camPos = _camera->getPosition();
        Size viewSize = _camera->getViewport().size;
        
        float offsetX = (viewSize.width - 1248 * _ui->getScaleX()) / 2.0f;
        float offsetY = (viewSize.height - 576 * _ui->getScaleY()) / 2.0f;
        Vec2 base = camPos - Vec2(viewSize.width / 2, viewSize.height / 2);
        _ui->setPosition(base + Vec2(offsetX, offsetY));
        _ui->updateTime(_levelController->getTimeSpentInLevel());
        _ui->updateProgression(_levelController->getSpawnedEnemyCount(),_levelController->getTotalEnemyCount(), _levelController->getCurrentWaveIndex());
    }
    
    setComplete(_levelController->isLevelWon());
    setFailure(_levelController->isLevelLost());

    // Record failure if necessary.
    if (!_failed && _player->getHP() <= 0) {
        setFailure(_levelController->isLevelLost());
    }
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
 * last call to {@link #fixedUpdate}. That is because this method is used
 * to interpolate object position for animation.
 *
 * @param remain    The amount of time (in seconds) last fixedUpdate
 */
void GameScene::postUpdate(float remain) {
    
    if (_isPaused) return;
    
    // Since items may be deleted, garbage collect
    _world->garbageCollect();
    _levelController->postUpdate(remain);
    // Add a bullet AFTER physics allows it to hang in front
    // Otherwise, it looks like bullet appears far away

    // Reset the game if we win or lose.
    if (_countdown > 0) {
        _countdown--;
    }
    else if (_countdown == 0) {
        reset();
    }

    // postUpdate the LevelController
    _levelController->postUpdate(remain);
}


#pragma mark collision helpers
/**
 * Removes a new projectile from the world.
 *
 * @param  projectile   the projectile to remove
 */
void GameScene::removeProjectile(Projectile* projectile) {
    // do not attempt to remove a projectile that has already been removed
    if (projectile->isRemoved()) {
        return;
    }
    _worldnode->removeChild(projectile->getSceneNode());
    projectile->setDebugScene(nullptr);
    projectile->markRemoved(true);
}

void GameScene::setBG() {
    int bgCount = _currentLevel->getBGN();
    float scale = _currentLevel->getScale();
    for (const auto& layerPair : _currentLevel->getLayers()) {
        for (int i = 0; i < bgCount; ++i) {
            std::shared_ptr<scene2::SceneNode> node = scene2::PolygonNode::allocWithTexture(layerPair.first);
            node->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
            node->setScale(scale);
            float textureWidth = node->getSize().width;

            // Position nodes starting from -1 * textureWidth (to provide buffer for parallax)
            float xPos = (i - 1) * textureWidth;
            node->setPosition(Vec2(xPos, 0));
            
            node->setTag(layerPair.second);
            _worldnode->addChild(node);
        }
    }
}

void GameScene::updateLayersLeft() {
    for (std::shared_ptr<scene2::SceneNode> node : _worldnode->getChildren()) {
        if (node->getTag() > 0) {
            node->setPosition(Vec2(node->getPositionX()-static_cast<float>(node->getTag())/4, node->getPositionY()));
        }
    }
}

void GameScene::updateLayersRight() {
    for (std::shared_ptr<scene2::SceneNode> node : _worldnode->getChildren()) {
        if (node->getTag() > 0) {
            node->setPosition(Vec2(node->getPositionX()+static_cast<float>(node->getTag())/4, node->getPositionY()));
        }
    }
}



void GameScene::setScreenShake(float intensity, int duration) {
    if (_shakeIntensity > intensity) {
        return;
    }
    _shakeIntensity = intensity;
    _shakeDuration = duration;
}

void GameScene::processScreenShake() {
    Vec2 target = Vec2::ZERO;
    if (_shakeDuration > 0) {
        _shakeDuration--;
        int shakeX = rand() % 2 ? _shakeIntensity : -_shakeIntensity;
        int shakeY = rand() % 2 ? _shakeIntensity : -_shakeIntensity;
        target = Vec2(shakeX, shakeY);
    }
    else {
        _shakeIntensity = 0;
    }
    _worldnode->setPosition(_worldnode->getPosition() + (target - _worldnode->getPosition()) / 2);
}
