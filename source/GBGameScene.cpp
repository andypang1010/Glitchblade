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
#include "GBPlayerModel.h"
#include "GBEnemyModel.h"
#include "GBProjectile.h"
#include "GBMeleeActionModel.h"
#include "GBIngameUI.h"
#include "GBPauseMenu.h"

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
 * with the Box2d coordinates.  This initializer uses the default scale.
 *
 * @param assets    The (loaded) assets for this game mode
 *
 * @return true if the controller is initialized properly, false otherwise.
 */
bool GameScene::init(const std::shared_ptr<AssetManager>& assets) {
    return init(assets, Rect(0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT), Vec2(0, DEFAULT_GRAVITY));
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
 * @param rect      The game bounds in Box2d coordinates
 *
 * @return  true if the controller is initialized properly, false otherwise.
 */
bool GameScene::init(const std::shared_ptr<AssetManager>& assets, const Rect& rect) {
    return init(assets, rect, Vec2(0, DEFAULT_GRAVITY));
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
 * @param rect      The game bounds in Box2d coordinates
 * @param gravity   The gravitational force on this Box2d world
 *
 * @return  true if the controller is initialized properly, false otherwise.
 */
bool GameScene::init(const std::shared_ptr<AssetManager>& assets,
    const Rect& rect, const Vec2& gravity) {
    if (assets == nullptr) {
        return false;
    }
    _assets = assets;
    // prepare constants
    std::shared_ptr<JsonReader> constants_reader = JsonReader::allocWithAsset("json/constants.json");
    _constantsJSON = constants_reader->readJson();
    if (_constantsJSON == nullptr) {
        CULog("Failed to load constants.json");
        return false;
    }
    std::shared_ptr<JsonValue> sceneJ = _constantsJSON->get("scene");
    if (!Scene2::initWithHint(Size(sceneJ->getInt("width"), sceneJ->getInt("height")))) {
        return false;
    }
    
    _offset = Vec2((_size.width - sceneJ->getInt("width")) / 2.0f, (_size.height - sceneJ->getInt("height")) / 2.0f);

    Rect bounds = getBounds();
    std::shared_ptr<JsonValue> boundsJ = sceneJ->get("bounds");
    boundsJ->get("origin")->get("x")->set(bounds.origin.x);
    boundsJ->get("origin")->get("y")->set(bounds.origin.y);
    boundsJ->get("size")->get("width")->set(bounds.size.width);
    boundsJ->get("size")->get("height")->set(bounds.size.height);
    
    
    // IMPORTANT: SCALING MUST BE UNIFORM
    // This means that we cannot change the aspect ratio of the physics world
    // Shift to center if a bad fit
    _scale = _size.width == sceneJ->getInt("width") ? _size.width / rect.size.width : _size.height / rect.size.height;
    sceneJ->get("scale")->set(_scale);
    
    
    // Create the world and attach the listeners.
    _world = physics2::ObstacleWorld::alloc(rect, gravity);
    _world->activateCollisionCallbacks(true);
    _world->onBeginContact = [this](b2Contact* contact) {
        beginContact(contact);
        };
    _world->onEndContact = [this](b2Contact* contact) {
        endContact(contact);
        };
    _debugnode = scene2::SceneNode::alloc();
    _debugnode->setScale(_scale); // Debug node draws in PHYSICS coordinates
    _debugnode->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    _debugnode->setPosition(_offset);
    std::shared_ptr<JsonValue> messagesJ = _constantsJSON->get("messages");
    _winnode = scene2::Label::allocWithText(messagesJ->get("win")->getString("text", "win msg json fail"), _assets->get<Font>(messagesJ->getString("font", "retro")));
    _winnode->setAnchor(Vec2::ANCHOR_CENTER);
    _winnode->setPosition(_size.width / 2.0f, _size.height / 2.0f);
    _winnode->setForeground(messagesJ->get("win")->getString("color"));
    setComplete(false);

    _losenode = scene2::Label::allocWithText(messagesJ->get("lose")->getString("text", "lose msg json fail"), _assets->get<Font>(messagesJ->getString("font", "retro")));
    _losenode->setAnchor(Vec2::ANCHOR_CENTER);
    _losenode->setPosition(_size.width / 2.0f, _size.height / 2.0f);
    _losenode->setForeground(messagesJ->get("lose")->getString("color"));
    setFailure(false);
    
    addChild(_debugnode);
    addChild(_winnode);
    addChild(_losenode);
    
    _levelController = std::make_shared<LevelController>();
    _levelController->init(_assets, _constantsJSON, _world, _debugnode); // Initialize the LevelController
    // LevelController needs to be initialized before populate() or else we will get nullptr related issues


    #pragma mark : Input (can delete and remove the code using _input in preupdate- only for easier setting of debugging node)
        _input =  _levelController->getInputController();
        if (!_input){
            CULog("input is null in populate");
        }
    #pragma mark : Player
        _player = _levelController->getPlayerModel();
        if (!_player){
            CULog("player is null in populate");
        }
        _player->setDebugColor(Color4::YELLOW);
    
    populate(_levelController->getLevelByName("Level 1"));

    // === Initialize in-game UI ===
    populateUI(assets);
    
    _active = true;
    _complete = false;
    setDebug(false); // Debug on by default

    // XNA nostalgia
    Application::get()->setClearColor(Color4f::BLACK);

    return true;
}

void GameScene::populateUI(const std::shared_ptr<cugl::AssetManager>& assets)
{
    _ui = GBIngameUI::alloc(_assets);
    if (_ui != nullptr) {
        addChild(_ui);
    }
    _pauseMenu = GBPauseMenu::alloc(assets);
    if (_pauseMenu != nullptr) {
        _pauseMenu->setVisible(false);
        addChild(_pauseMenu);
    }

    auto pauseButton = _ui->getPauseButton();
    if (pauseButton) {
        pauseButton->addListener([this](const std::string& name, bool down) {
            if (down) {
                _ui->setVisible(false);
                _pauseMenu->setVisible(true);
                setPaused(true);
            }
            });
    }

    auto resumeButton = _pauseMenu->getResumeButton();
    if (resumeButton) {
        resumeButton->addListener([this](const std::string& name, bool down) {
            if (down) {
                _pauseMenu->setVisible(false);
                _ui->setVisible(true);
                setPaused(false);
            }
            });
    }

    auto restartButton = _pauseMenu->getRestartButton();
    if (restartButton) {
        restartButton->addListener([this](const std::string& name, bool down) {
            if (down) {
                _pauseMenu->setVisible(false);
                _ui->setVisible(true);
                setPaused(false);
                this->reset();
            }
            });
    }
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameScene::dispose() {
    if (_active) {
        _world = nullptr;
        _worldnode = nullptr;
        _debugnode = nullptr;
        _winnode = nullptr;
        _losenode = nullptr;
        _complete = false;
        _debug = false;
		_ui = nullptr;
		_pauseMenu = nullptr;
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
    _pauseMenu->removeAllChildren();
    _ui->removeAllChildren();
    setFailure(false);
    setComplete(false);
    _levelController->reset();
    populate(_levelController->getCurrentLevel());
	populateUI(_assets);
    _ui->setHP(100);
    _pauseMenu->setHP(100);
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
    _worldnode = _levelController->makeWorldNode("Level 1");
    _worldnode->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    _worldnode->setPosition(_offset);
    addChild(_worldnode);
    _levelController->populateLevel(level); // Sets the level we want to populate here
    _player = _levelController->getPlayerModel(); // DELETE!


    std::vector<std::shared_ptr<EnemyController>> enemyControllers = _levelController->getEnemyControllers();

    // Add UI elements

	// Play the background music on a loop.
    std::shared_ptr<JsonValue> musicJ = _constantsJSON->get("audio")->get("music");
	std::shared_ptr<Sound> source = _assets->get<Sound>(musicJ->getString("game"));
    AudioEngine::get()->getMusicQueue()->play(source, true, musicJ->getFloat("volume"));
    
    // Spawn the first wave
    _levelController->spawnWave(0);
    
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
        std::shared_ptr<JsonValue> musicJ = _constantsJSON->get("audio")->get("music");
        std::shared_ptr<Sound> source = _assets->get<Sound>(musicJ->getString("win"));
        AudioEngine::get()->getMusicQueue()->play(source, false, musicJ->getFloat("volume"));
        _winnode->setVisible(true);
        _countdown = _constantsJSON->getInt("exit_count");
    }
    else if (!value) {
        _winnode->setVisible(false);
        _countdown = -1;
    }
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
        _losenode->setVisible(true);
        _countdown = _constantsJSON->getInt("exit_count");
    }
    else {
        _losenode->setVisible(false);
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
    if (_isPaused) return;
    
    // Process the toggled key commands
    if (_input->didDebug()) { setDebug(!isDebug()); }
    if (_input->didReset()) { reset(); }
    if (_input->didExit()) {
        Application::get()->quit();
    }


	_ui->setHP(_player->getHP());


    if (_player->isJumpBegin() && _player->isGrounded()) {
        std::shared_ptr<JsonValue> fxJ = _constantsJSON->get("audio")->get("effects");
        std::shared_ptr<Sound> source = _assets->get<Sound>(fxJ->getString("jump"));
        AudioEngine::get()->play(fxJ->getString("jump"),source,false,fxJ->getFloat("volume"));
    }

    // Call preUpdate on the LevelController
    _levelController->preUpdate(dt);
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


    setComplete(_levelController->isCurrentLevelComplete());
    setFailure(_player->getHP() <= 0);

    // Record failure if necessary.
    if (!_failed && _player->getY() < 0) {
        setFailure(true);
    }

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
    
    std::shared_ptr<JsonValue> fxJ = _constantsJSON->get("audio")->get("effects");
    std::shared_ptr<Sound> source = _assets->get<Sound>(fxJ->getString("pop"));
    AudioEngine::get()->play(fxJ->getString("pop"), source, false, fxJ->getFloat("volume"), true);
}

#pragma mark -
#pragma mark Collision Handling

/**Checks obstacle and fixture to see if it is an enemy body fixture.**/
bool GameScene::isEnemyBody(physics2::Obstacle* b, std::string f ) {
    std::shared_ptr<JsonValue> enemyJ = _constantsJSON->get("enemy");
    // This depends on enemies having their name set to enemy. This is probably dumb
    return (b->getName() == enemyJ->getString("name") && f == "enemy");
}
/**Checks obstacle and fixture to see if it the player body fixture.**/
bool GameScene::isPlayerBody(physics2::Obstacle* b, const std::string* f ) {
    std::shared_ptr<JsonValue> playerJ = _constantsJSON->get("player");
    return (b->getName() == playerJ->getString("name") && f == _player->getBodyName());
}

/**
 Checks if contact is projectile hitting player shield and returns the Projectile if so, else NULL.
 */
Projectile* GameScene::getProjectileHitShield(physics2::Obstacle* bd1, std::string* fd1,
                                              physics2::Obstacle* bd2, std::string* fd2) const {
    std::string proj_name = _constantsJSON->get("projectile")->getString("name");
    if (bd1->getName() == proj_name && fd2 == _player->getShieldName() &&
        !((Projectile*)bd1)->getIsPlayerFired() && _player->isGuardActive()) {
        return (Projectile*)bd1;
    }
    if (bd2->getName() == proj_name && fd1 == _player->getShieldName() &&
        !((Projectile*)bd2)->getIsPlayerFired() && _player->isGuardActive()) {
        return (Projectile*)bd2;
    }
    return nullptr;
}
/**
 * Processes the start of a collision
 *
 * This method is called when we first get a collision between two objects.  We use
 * this method to test if it is the "right" kind of collision.  In particular, we
 * use it to test if we make it to the win door.
 *
 * @param  contact  The two bodies that collided
 */
void GameScene::beginContact(b2Contact* contact) {
    std::string proj_name = _constantsJSON->get("projectile")->getString("name");
    std::string enemy_name = _constantsJSON->get("enemy")->getString("name");
    std::string ground_name = _constantsJSON->get("ground")->getString("name");
    std::string wall_name = _constantsJSON->get("walls")->getString("name");
    b2Fixture* fix1 = contact->GetFixtureA();
    b2Fixture* fix2 = contact->GetFixtureB();



    b2Body* body1 = fix1->GetBody();
    b2Body* body2 = fix2->GetBody();

    std::string* fd1 = reinterpret_cast<std::string*>(fix1->GetUserData().pointer);
    std::string* fd2 = reinterpret_cast<std::string*>(fix2->GetUserData().pointer);

    // No idea what this if block is doing; previously was only used for CULogs
    if (!fix1->GetUserData().pointer || !fix2->GetUserData().pointer) {
    }
    else {
    }

    physics2::Obstacle* bd1 = reinterpret_cast<physics2::Obstacle*>(body1->GetUserData().pointer);
    physics2::Obstacle* bd2 = reinterpret_cast<physics2::Obstacle*>(body2->GetUserData().pointer);

    // Player-Enemy Collision
    if (bd1->getName() == enemy_name && isPlayerBody(bd2, fd2)) {
        if (_player->isDashActive() && !_player->isGuardActive()) {
            ((EnemyModel*)bd1)->damage(5);
            _player->setDashRem(0);
        }
        _player->setKnocked(true, _player->getPosition().subtract(bd1->getPosition()).normalize());
        ((EnemyModel*)bd1)->setKnocked(true, bd1->getPosition().subtract(_player->getPosition()).normalize());
    }
    else if (bd2->getName() == enemy_name && isPlayerBody(bd1, fd1)) {
        if (_player->isDashActive() && !_player->isGuardActive()) {
            ((EnemyModel*)bd2)->damage(5);
            _player->setDashRem(0);
        }
        _player->setKnocked(true, _player->getPosition().subtract(bd1->getPosition()).normalize());
        ((EnemyModel*)bd2)->setKnocked(true, bd2->getPosition().subtract(_player->getPosition()).normalize());
    }

    // Test: plyaer-hitbox collision
    if (bd1->getName() == "hitbox" && isPlayerBody(bd2, fd2)) {
        if (_player->iframe <= 0) {
            _player->iframe = 60;
            if (!_player->isGuardActive() && !_player->isParryActive()) {
                _player->damage(((Hitbox*)bd1)->getDamage());
                _player->setKnocked(true, _player->getPosition().subtract(((Hitbox*)bd1)->getEnemy()->getPosition()).normalize());
            }
            else if (_player->isParryActive()) {
                ((Hitbox*)bd1)->getEnemy()->setStun(88);
            }
            else if (_player->isGuardActive()) {
                _player->damage(((Hitbox*)bd1)->getDamage() / 2);
            }
        }
    }
    else if (bd2->getName() == "hitbox" && isPlayerBody(bd1, fd1)) {
        if (_player->iframe <= 0) {
            _player->iframe = 60;
            if (!_player->isGuardActive() && !_player->isParryActive()) {
                _player->damage(((Hitbox*)bd2)->getDamage());
                _player->setKnocked(true, _player->getPosition().subtract(((Hitbox*)bd2)->getEnemy()->getPosition()).normalize());
            }
            else if (_player->isParryActive()) {
                ((Hitbox*)bd2)->getEnemy()->setStun(88);
            }
            else if (_player->isGuardActive()) {
                _player->damage(((Hitbox*)bd2)->getDamage() / 2);
            }
        }
    }

    // Player-Projectile Collision
    if (isPlayerBody(bd1, fd1) && bd2->getName() == proj_name) {
        if (!((Projectile*)bd2)->getIsPlayerFired()) {
            _player->damage(20);
            removeProjectile((Projectile*)bd2);
            _ui->setHP(_player->getHP());
            _pauseMenu->setHP(_player->getHP());
        }
    }
    else if (isPlayerBody(bd2, fd2) && bd1->getName() == proj_name) {
        if (!((Projectile*)bd1)->getIsPlayerFired()) {
            _player->damage(20);
            removeProjectile((Projectile*)bd1);
            _ui->setHP(_player->getHP());
            _pauseMenu->setHP(_player->getHP());
        }
    }

    // Shield-Projectile Collision
    Projectile* shieldHit = getProjectileHitShield(bd1, fd1, bd2, fd2);
    if (shieldHit) {

        if (_player->isParryActive()) {
            if (!_player->hasProjectile()) {
                _player->setHasProjectile(true);
            }
        }
        else {
            _player->damage(10);
            _ui->setHP(_player->getHP());
            _pauseMenu->setHP(_player->getHP());
        }
        removeProjectile(shieldHit);
    }

    // Projectile-Projectile Collision
    if (bd1->getName() == proj_name && bd2->getName() == proj_name) {

        // Destroy if one is fired by player and the other is not
        if (
            (((Projectile*)bd1)->getIsPlayerFired() && !((Projectile*)bd2)->getIsPlayerFired()) ||
            (((Projectile*)bd2)->getIsPlayerFired() && !((Projectile*)bd1)->getIsPlayerFired())
            ) {
            removeProjectile((Projectile*)bd1);
            removeProjectile((Projectile*)bd2);
        }
    }

    // Projectile-Environment Collision
    if (bd1->getName() == proj_name && (bd2->getName() == ground_name || bd2->getName() == wall_name)) {
        removeProjectile((Projectile*)bd1);
    }
    else if (bd2->getName() == proj_name && (bd1->getName() == ground_name || bd1->getName() == wall_name)) {
        removeProjectile((Projectile*)bd2);
    }

    // Enemy-Projectile Collision
    if (bd1->getName() == enemy_name && bd2->getName() == proj_name) {

        if (((Projectile*)bd2)->getIsPlayerFired()) {
            ((EnemyModel*)bd1)->damage(20);
            removeProjectile((Projectile*)bd2);
        }
    }
    else if (bd2->getName() == enemy_name && bd1->getName() == proj_name) {
        if (((Projectile*)bd1)->getIsPlayerFired()) {
            ((EnemyModel*)bd2)->damage(20);
            removeProjectile((Projectile*)bd1);
        }
    }

    // Player-Ground Collision
    if ((_player->getGroundSensorName() == fd2 && _player.get() != bd1) ||
        (_player->getGroundSensorName() == fd1 && _player.get() != bd2)) {
        _player->setGrounded(true);

        // Could have more than one ground
        _sensorFixtures.emplace(_player.get() == bd1 ? fix2 : fix1);
    }
}

/**
 * Callback method for the start of a collision
 *
 * This method is called when two objects cease to touch.  The main use of this method
 * is to determine when the characer is NOT on the ground.  This is how we prevent
 * double jumping.
 */
void GameScene::endContact(b2Contact* contact) {
    b2Fixture* fix1 = contact->GetFixtureA();
    b2Fixture* fix2 = contact->GetFixtureB();

    b2Body* body1 = fix1->GetBody();
    b2Body* body2 = fix2->GetBody();

    std::string* fd1 = reinterpret_cast<std::string*>(fix1->GetUserData().pointer);
    std::string* fd2 = reinterpret_cast<std::string*>(fix2->GetUserData().pointer);

    physics2::Obstacle* bd1 = reinterpret_cast<physics2::Obstacle*>(body1->GetUserData().pointer);
    physics2::Obstacle* bd2 = reinterpret_cast<physics2::Obstacle*>(body2->GetUserData().pointer);

    if ((_player->getGroundSensorName() == fd2 && _player.get() != bd1) ||
        (_player->getGroundSensorName() == fd1 && _player.get() != bd2)) {
        _sensorFixtures.erase(_player.get() == bd1 ? fix2 : fix1);
        if (_sensorFixtures.empty()) {
            _player->setGrounded(false);
        }
    }
}
