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

#include <ctime>
#include <string>
#include <iostream>
#include <sstream>
#include <random>

using namespace cugl;
using namespace cugl::graphics;
using namespace cugl::physics2;
using namespace cugl::audio;

// Constants moved to LevelController.cpp
// NOTE: THE SCENE WIDTH/HEIGHT, ASPECT, DEFAULT WIDTH/HEIGHT ARE CURRENTLY COPIED IN LEVELCONTROLLER WITH GAME_ PREFIX
/** This is adjusted by screen aspect ratio to get the height */
#define SCENE_WIDTH 1248
#define SCENE_HEIGHT 576
/** This is the aspect ratio for physics */
#define SCENE_ASPECT 9.0/19.5
/** Width of the game world in Box2d units */
#define DEFAULT_WIDTH   39.0f
/** Height of the game world in Box2d units */
#define DEFAULT_HEIGHT  18.0f
/** The new heavier gravity for this world (so it is not so floaty) */
#define DEFAULT_GRAVITY -28.9f

#define directions
Vec2 LEFT  = { -1.0f,  0.0f };
Vec2 RIGHT = {  1.0f,  0.0f };
Vec2 UP    = {  0.0f,  1.0f };
Vec2 DOWN  = {  0.0f, -1.0f };

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
                         _input(nullptr) {
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
bool GameScene::init(const std::shared_ptr<AssetManager>& assets) {
    if (assets == nullptr) {
        return false;
    }
    _assets = assets;

    // prepare constants
    std::shared_ptr<JsonReader> constants_reader = JsonReader::allocWithAsset("json/constants.json");
    _constantsJSON = constants_reader->readJson();
    if (_constantsJSON == nullptr) {
        return false;
    }

    std::shared_ptr<JsonValue> sceneJ = _constantsJSON->get("scene");
    if (!Scene2::initWithHint(Size(sceneJ->getInt("width"), sceneJ->getInt("height")))) {
        return false;
    }

    // Cache the camera
    _camera    = getCamera();
    _defCamPos = _camera->getPosition();

    // compute letterbox offset
    _offset = Vec2((_size.width  - sceneJ->getInt("width"))  / 2.0f,
                   (_size.height - sceneJ->getInt("height")) / 2.0f);
    Rect bounds = getBounds();
    auto boundsJ = sceneJ->get("bounds");
    boundsJ->get("origin")->get("x")->set(bounds.origin.x);
    boundsJ->get("origin")->get("y")->set(bounds.origin.y);
    boundsJ->get("size"  )->get("width")->set(bounds.size.width);
    boundsJ->get("size"  )->get("height")->set(bounds.size.height);

    // Create physics world
    Rect worldSize = Rect(0, 0,
                          sceneJ->getFloat("world_width"),
                          sceneJ->getFloat("world_height"));
    _scale = (_size.width == sceneJ->getInt("width"))
             ? _size.width /  worldSize.size.width
             : _size.height / worldSize.size.height;
    _worldPixelWidth = worldSize.size.width * _scale;
    sceneJ->get("scale")->set(_scale);

    Vec2 gravity(0.0f, -28.9f);
    _world = physics2::ObstacleWorld::alloc(worldSize, gravity);
    _world->activateCollisionCallbacks(true);
    _world->onBeginContact = [this](b2Contact* contact) {
        _collisionController->beginContact(contact);
    };
    _world->onEndContact   = [this](b2Contact* contact) {
        _collisionController->endContact(contact);
    };

    // Create scene graph nodes
    _worldnode = scene2::SceneNode::alloc();
    _worldnode->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    _worldnode->setPosition(_offset);

    _debugnode = scene2::SceneNode::alloc();
    _debugnode->setScale(_scale);
    _debugnode->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    _debugnode->setPosition(_offset);
    _debugnode->setName("game_debug_node");

    // Initialize LevelController (only once!)
    _levelController = std::make_unique<LevelController>();
    _levelController->init(_assets, _constantsJSON, _world, _debugnode, _worldnode);
    _currentLevel = _levelController->getLevelByName("Level 1");

    // Set up input and player refs
    _input  = _levelController->getInputController();
    _player = _levelController->getPlayerModel();

    // UI & state defaults
    setComplete(false);
    setFailure(false);
    _active   = true;
    _complete = false;
    setDebug(false);
    Application::get()->setClearColor(Color4f::BLACK);

    // Build and attach the level into the scene
    // populate(_currentLevel);

    // After the scene graph is built, wire up collision callbacks
    _collisionController = std::make_unique<CollisionController>(
            _player,
            _ui,
            _constantsJSON,
            [this](Projectile* p) { removeProjectile(p); },
            [this](int i,int d)  { setScreenShake(i,d); }
    );

    return true;
}

std::shared_ptr<LevelModel> GameScene::getLevelModel(std::string name) {
    return _levelController->getLevelByName(name);
}

void GameScene::setAllGameplayUIActive(bool active) {
    _ui->setVisible(active);
}

void GameScene::populateUI(const std::shared_ptr<cugl::AssetManager>& assets) {
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
        _ui->setContinueCallback([this]() {
            _shouldContinue = true;
            _ui->showWinPage2(true);
        });
    }
    auto pauseButton = _ui->getPauseButton();
    if (pauseButton) {
        pauseButton->addListener([this](const std::string& name, bool down) {
            if (down) {
                _ui->setVisible(false);
                setPaused(true);
            }
        });
    }
    setAllGameplayUIActive(_inituiactive);
}

void GameScene::disableUI() {
    setInitUIActive(false);
    if (_ui) {
        setAllGameplayUIActive(false);
    }
}

void GameScene::enableUI() {
    setInitUIActive(true);
    if (_ui) {
        setAllGameplayUIActive(true);
    }
}

void GameScene::dispose() {
    if (_active) {
        _world      = nullptr;
        _worldnode  = nullptr;
        _debugnode  = nullptr;
        _complete   = false;
        _debug      = false;
        _ui         = nullptr;
        Scene2::dispose();
    }
}

void GameScene::reset() {
    removeAllChildren();
    _world->clear();
    _worldnode->removeAllChildren();
    _debugnode->removeAllChildren();
    _ui->removeAllChildren();

    _failed   = false;
    _complete = false;

    _levelController->reset();
    populate(_levelController->getCurrentLevel());
    if(_camera != nullptr)
        _camera->setPosition(_defCamPos);
    _ui->resetUI();

    Application::get()->setClearColor(Color4f::BLACK);
    _populated = false;
}

void GameScene::populate(const std::shared_ptr<LevelModel>& level) {
    if (_worldnode && _worldnode->getParent())   removeChild(_worldnode);
    if (_debugnode && _debugnode->getParent())   _debugnode->removeFromParent();

    _worldnode = _levelController->makeWorldNode(level->getLevelName());
    _worldnode->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    _worldnode->setPosition(_offset);

    addChild(_worldnode);
    addChild(_debugnode);

    populateUI(_assets);
    addChild(_ui);

    _ui->setHP(100);
    _ui->setVisible(true);

    setBG();
    _levelController->populateLevel(level);
    _player = _levelController->getPlayerModel();
    setAllGameplayUIActive(true);

    auto musicJ = _constantsJSON->get("audio")->get("music");
    auto source = _assets->get<Sound>(musicJ->getString("game"));
    AudioEngine::get()->getMusicQueue()->play(source, true, musicJ->getFloat("volume"));

    _active    = true;
    _complete  = false;
    _populated = true;
}

void GameScene::setComplete(bool value) {
    if (_failed) return;
    bool change = _complete != value;
    _complete = value;
    if (value && change) {
        float timeSpent = _levelController->getTimeSpentInLevel();
        int parryCount  = _levelController->getPlayerController()->getPlayer()->_parryCounter;
        _ui->updateStats(timeSpent, parryCount);

        auto musicJ = _constantsJSON->get("audio")->get("music");
        auto source = _assets->get<Sound>(musicJ->getString("win"));
        AudioEngine::get()->getMusicQueue()->play(source, false, musicJ->getFloat("volume"));

        _ui->showWinPage1(true);
        setPaused(true);
        _countdown = _constantsJSON->getInt("exit_count");
    } else if (!value) {
        setPaused(false);
        _countdown = -1;
    }
}

void GameScene::setFailure(bool value) {
    if (_complete) return;
    bool change = _failed != value;
    _failed = value;
    if (value && change) {
        auto musicJ = _constantsJSON->get("audio")->get("music");
        auto source = _assets->get<Sound>(musicJ->getString("win"));
        AudioEngine::get()->getMusicQueue()->play(source, false, musicJ->getFloat("volume"));
        if (_ui) {
            _ui->showLosePage(true);
            setPaused(true);
        }
        _countdown = _constantsJSON->getInt("exit_count");
    } else {
        if (_ui) {
            _ui->showLosePage(false);
            setPaused(false);
        }
        _countdown = -1;
    }
}

void GameScene::preUpdate(float dt) {
    if (_input->didDebug()) { setDebug(!isDebug()); }
    if (_input->didReset()) { reset(); }
    if (_input->didExit())  { Application::get()->quit(); }

    _ui->setHP(_player->getHP());
    if (_player->isJumpBegin() && _player->isGrounded()) {
        auto fxJ = _constantsJSON->get("audio")->get("effects");
        auto source = _assets->get<Sound>(fxJ->getString("jump"));
        AudioEngine::get()->play(fxJ->getString("jump"), source, false, fxJ->getFloat("volume"));
    }

    _levelController->preUpdate(dt);

    if (_shouldPause) {
        _shouldPause = false;
        _ui->showPauseMenu(true);
        _ui->showHeadsUpDisplay(false);
        setPaused(true);
    }
    if (_shouldResume) {
        _shouldResume = false;
        _ui->showPauseMenu(false);
        _ui->showHeadsUpDisplay(true);
        setPaused(false);
    }
    if (_shouldRetry) {
        _shouldRetry = false;
        _ui->showPauseMenu(false);
        _ui->showHeadsUpDisplay(true);
        setPaused(false);
        reset();
    }
    if (_shouldContinue) {
        _shouldContinue = false;
        _ui->showWinPage1(false);
        _ui->showWinPage2(true);
        setPaused(true);
    }
}

void GameScene::fixedUpdate(float step) {
    if (_isPaused) return;

    _world->update(step);
    _levelController->fixedUpdate(step);
    processScreenShake();

    float currX = _levelController->getPlayerNode()->getPositionX();
    bool isKnocked = _levelController->getPlayerModel()->isKnockbackActive();
    float velX = _levelController->getPlayerModel()->getVX();

    if (_camera == nullptr)
        CULog("SHOULDNT EVER HAPPEN!!!!");
    float camLX = _camera->getPosition().x - _camera->getViewport().size.width/2;
    float camRX = _camera->getPosition().x + _camera->getViewport().size.width/2;
    if (camLX <= 0 || camRX >= _worldPixelWidth) {
        _cameraLocked = true;
        if (camLX <= 0) {
            if (currX > getBounds().size.width * .66f) _cameraLocked = false;
        } else {
            if (currX < _worldPixelWidth - getBounds().size.width * .66f) _cameraLocked = false;
        }
    }
    if (!_cameraLocked) {
        _camera->translate(Vec2((currX - _camera->getPosition().x)*.05f, 0));
        _camera->update();
        if (velX > 0 && !isKnocked) {
            updateLayersLeft();
        } else if (velX < 0 && !isKnocked) {
            updateLayersRight();
        }
    }

    if (_ui && _camera) {
        Vec2 base = _camera->getPosition() - Vec2(_camera->getViewport().size.width/2,
                                                  _camera->getViewport().size.height/2);
        _ui->setPosition(base + _ui->_screenOffset);
    }

    setComplete(_levelController->isLevelWon());
    setFailure(_levelController->isLevelLost());

    if (!_failed && _player->getY() < 0) {
        setFailure(true);
    }
}

void GameScene::postUpdate(float remain) {
    if (_isPaused) return;

    _world->garbageCollect();
    _levelController->postUpdate(remain);

    if (_countdown > 0) {
        --_countdown;
    } else if (_countdown == 0) {
        reset();
    }

    _levelController->postUpdate(remain);
}

void GameScene::removeProjectile(Projectile* projectile) {
    if (projectile->isRemoved()) return;

    _worldnode->removeChild(projectile->getSceneNode());
    projectile->setDebugScene(nullptr);
    projectile->markRemoved(true);

    auto fxJ = _constantsJSON->get("audio")->get("effects");
    auto source = _assets->get<Sound>(fxJ->getString("pop"));
    AudioEngine::get()->play(fxJ->getString("pop"), source, false, fxJ->getFloat("volume"), true);
}

void GameScene::setBG() {
    for (auto pair : _currentLevel->getLayers()) {
        auto node = scene2::PolygonNode::allocWithTexture(pair.first);
        node->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
        node->setScale(_currentLevel->getScale());
        node->setPosition(Vec2(node->getPositionX() - node->getSize().width,
                               node->getPositionY()));
        node->setTag(pair.second);
        _worldnode->addChild(node);

        auto node2 = scene2::PolygonNode::allocWithTexture(pair.first);
        node2->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
        node2->setScale(_currentLevel->getScale());
        node2->setTag(pair.second);
        _worldnode->addChild(node2);

        auto node3 = scene2::PolygonNode::allocWithTexture(pair.first);
        node3->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
        node3->setScale(_currentLevel->getScale());
        node3->setPosition(Vec2(node3->getPositionX() + node->getSize().width,
                                node3->getPositionY()));
        node3->setTag(pair.second);
        _worldnode->addChild(node3);

        auto node4 = scene2::PolygonNode::allocWithTexture(pair.first);
        node4->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
        node4->setScale(_currentLevel->getScale());
        node4->setPosition(Vec2(node4->getPositionX() + 2 * node->getSize().width,
                                node4->getPositionY()));
        node4->setTag(pair.second);
        _worldnode->addChild(node4);
    }
}

void GameScene::updateLayersLeft() {
    for (auto node : _worldnode->getChildren()) {
        if (node->getTag() > 0) {
            node->setPosition(Vec2(node->getPositionX() - static_cast<float>(node->getTag())/3,
                                   node->getPositionY()));
        }
    }
}

void GameScene::updateLayersRight() {
    for (auto node : _worldnode->getChildren()) {
        if (node->getTag() > 0) {
            node->setPosition(Vec2(node->getPositionX() + static_cast<float>(node->getTag())/3,
                                   node->getPositionY()));
        }
    }
}

void GameScene::setScreenShake(float intensity, int duration) {
    if (_shakeIntensity > intensity) return;
    _shakeIntensity = intensity;
    _shakeDuration = duration;
}

void GameScene::processScreenShake() {
    Vec2 target = Vec2::ZERO;
    if (_shakeDuration > 0) {
        --_shakeDuration;
        int sx = (rand() % 2 ? _shakeIntensity : -_shakeIntensity);
        int sy = (rand() % 2 ? _shakeIntensity : -_shakeIntensity);
        target = Vec2(sx, sy);
    } else {
        _shakeIntensity = 0;
    }
    _worldnode->setPosition(_worldnode->getPosition() + (target - _worldnode->getPosition()) / 2);
}
