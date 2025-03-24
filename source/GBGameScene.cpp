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
    // prepare constants
    std::shared_ptr<JsonReader> constants_reader = JsonReader::allocWithAsset("json/constants.json");
    _constantsJSON = constants_reader->readJson();
    std::shared_ptr<JsonValue> sceneJ = _constantsJSON->get("scene");
    if (_constantsJSON == nullptr) {
        CULog("Failed to load constants.json");
        return false;
    }
    
    if (assets == nullptr) {
        return false;
    }
    else if (!Scene2::initWithHint(Size(sceneJ->getInt("width"), sceneJ->getInt("height")))) {
        return false;
    }
    _assets = assets;
    
    Rect bounds = getBounds();
    CULog("bounds is (%f, %f), (%f, %f)",bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height);
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
    CULog("in game scene scale is %f", _scale);
    
    
    // Create the world and attach the listeners.
    _world = physics2::ObstacleWorld::alloc(rect, gravity);
    _world->activateCollisionCallbacks(true);
    _world->onBeginContact = [this](b2Contact* contact) {
        beginContact(contact);
        };
    _world->onEndContact = [this](b2Contact* contact) {
        endContact(contact);
        };


    // Create the scene graph
    Vec2 offset((_size.width - sceneJ->getInt("width")) / 2.0f, (_size.height - sceneJ->getInt("height")) / 2.0f);
    std::shared_ptr<Texture> image = _assets->get<Texture>(sceneJ->getString("texture"));
    _worldnode = scene2::PolygonNode::allocWithTexture(image);
    _worldnode->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    _worldnode->setPosition(offset);

    _debugnode = scene2::SceneNode::alloc();
    _debugnode->setScale(_scale); // Debug node draws in PHYSICS coordinates
    _debugnode->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    _debugnode->setPosition(offset);
    
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

    addChild(_worldnode);
    addChild(_debugnode);
    addChild(_winnode);
    addChild(_losenode);

    CULog("Creating empty level controller in gamescene init");
    _levelController = std::make_shared<LevelController>();
    CULog("initializing level controller in gamescene init");
    _levelController->init(_assets,_constantsJSON); // Initialize the LevelController
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
    populate();
    _active = true;
    _complete = false;
    setDebug(false); // Debug on by default

    // XNA nostalgia
    Application::get()->setClearColor(Color4f::BLACK);



    return true;
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
        Scene2::dispose();
    }
}


#pragma mark -
#pragma mark Level Layout

/**
 * Resets the status of the game by resetting player and enemy positions so that we can play again.
 */
void GameScene::reset() {
    _world->clear();
    _worldnode->removeAllChildren();
    _debugnode->removeAllChildren();
    setFailure(false);
    setComplete(false);
    _levelController->reset();
    populate();
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
void GameScene::populate() {
    // DO NOT KEEP THIS IN THE CODE YOU DEGEN
    CULog("TODO: Get rid of this reference to player in gamescene.");
    ObstacleNodePairs static_obstacles = _levelController->populateLevel("Level 1"); // Will want to set the level we want to populate here
    _player = _levelController->getPlayerModel(); // DELETE!
    _testEnemy = _levelController->getTestEnemyModel();
    for (const auto& pair : static_obstacles) {
        ObstaclePtr obstacle = pair.first;
        NodePtr node = pair.second;
        // add obstacle and set node position
        addObstacle(obstacle, node, 1);
    }
    
    addObstacle(_levelController->getPlayerModel(),_levelController->getPlayerNode());
    addObstacle(_levelController->getTestEnemyModel(),_levelController->getTestEnemyNode());

    // Add UI elements

	// Play the background music on a loop.
    std::shared_ptr<JsonValue> musicJ = _constantsJSON->get("audio")->get("music");
	std::shared_ptr<Sound> source = _assets->get<Sound>(musicJ->getString("game"));
    AudioEngine::get()->getMusicQueue()->play(source, true, musicJ->getFloat("volume"));
}

/**
 * Adds the physics object to the physics world and loosely couples it to the scene graph
 *
 * There are two ways to link a physics object to a scene graph node on the
 * screen.  One way is to make a subclass of a physics object, like we did
 * with dude.  The other is to use callback functions to loosely couple
 * the two.  This function is an example of the latter.
 *
 * @param obj             The physics object to add
 * @param node            The scene graph node to attach it to
 * @param zOrder          The drawing order
 * @param useObjPosition  Whether to update the node's position to be at the object's position
 */
void GameScene::addObstacle(const std::shared_ptr<physics2::Obstacle>& obj,
    const std::shared_ptr<scene2::SceneNode>& node,
    bool useObjPosition) {
    _world->addObstacle(obj);
    obj->setDebugScene(_debugnode);

    // Position the scene graph node (enough for static objects)
    if (useObjPosition) {
        node->setPosition(obj->getPosition() * _scale);
    }
    _worldnode->addChild(node);

//    // Dynamic objects need constant updating
//    if (obj->getBodyType() == b2_dynamicBody) {
//        scene2::SceneNode* weak = node.get(); // No need for smart pointer in callback
//        obj->setListener([=, this](physics2::Obstacle* obs) {
//            weak->setPosition(obs->getPosition() * _scale);
//            weak->setAngle(obs->getAngle());
//            });
//    }
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

    // Process the toggled key commands
    if (_input->didDebug()) { setDebug(!isDebug()); }
    if (_input->didReset()) { reset(); }
    if (_input->didExit()) {
        CULog("Shutting down");
        Application::get()->quit();
    }

    // TODO: refactor using Box2d
    Vec2 dist = _testEnemy->getPosition() - _player->getPosition();
    bool hit = false;
    if(_player->iframe > 0) _player->iframe--;
    if (_testEnemy->isDamaging() && _player->iframe <= 0) {
        if (_testEnemy->_isSlamming) {
            if (dist.x > 0 && dist.x <= 6 && !_testEnemy->isFacingRight() && std::abs(dist.y) <= 6) {
                hit = true;
            }
            else if (dist.x < 0 && dist.x >= -6 && _testEnemy->isFacingRight() && std::abs(dist.y) <= 6) {
                hit = true;
            }
        }
        else if (_testEnemy->_isStabbing) {
            if (dist.x > 0 && dist.x <= 6 && !_testEnemy->isFacingRight() && std::abs(dist.y) <= 2) {
                hit = true;
            }
            else if (dist.x < 0 && dist.x >= -6 && _testEnemy->isFacingRight() && std::abs(dist.y) <= 2) {
                hit = true;
            }
        }
    }

    if (hit) {
        _player->setKnocked(true, _player->getPosition().subtract(_testEnemy->getPosition()).normalize());
        if (_player->iframe <= 0 && !_player->isParryActive() && !_player->isGuardActive()) {
            _player->damage(20);
        }
        else if (_player->iframe <= 0 && _player->isParryActive()) {
            _testEnemy->setStun(120);
        }
        else if (_player->iframe <= 0 && _player->isGuardActive()) {
            _player->damage(10);
        }
        _player->iframe = 60;
    }
    _testEnemy->setTargetPos(_player->getPosition());


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
    // Since items may be deleted, garbage collect
    _world->garbageCollect();
    _levelController->postUpdate(remain);
    // Add a bullet AFTER physics allows it to hang in front
    // Otherwise, it looks like bullet appears far away

    if (_player->isShooting() && _player->hasProjectile()) {
        ObstacleNodePair p = Projectile::createProjectile(_assets, _constantsJSON, _player->getPosition(), _player->isFacingRight() ? Vec2(1, 0) : Vec2(-1, 0), true, _player->isFacingRight());
        addObstacle(p.first, p.second);
        _player->setHasProjectile(false);
    }

    if (_bulletTimer <= 0) {
        int r = rand() % 2;
        if (r == 0) {
            Vec2 left_spawn = { _constantsJSON->get("walls")->getFloat("thickness") + 1.5f, 9.0f };
            ObstacleNodePair p = Projectile::createProjectile(_assets, _constantsJSON, left_spawn, RIGHT, false, false);
            addObstacle(p.first, p.second, 1);
        }
        else if(r == 1){
            Vec2 right_spawn = {_constantsJSON->get("scene")->getFloat("default_width") - _constantsJSON->get("walls")->getFloat("thickness") - 1.5f, 13.0f};
            ObstacleNodePair p = Projectile::createProjectile(_assets, _constantsJSON, right_spawn, LEFT, false, false);
            addObstacle(p.first, p.second, 1);
        }

        _bulletTimer = _constantsJSON->get("projectile")->getInt("spawn_rate");
    }
    else {
        _bulletTimer -= 1;
    }
    setComplete(_testEnemy->getHP() <= 0);
    setFailure(_player->getHP() <= 0);

    // Record failure if necessary.
    if (!_failed && _player->getY() < 0) {
        setFailure(true);
    }

    // Reset the game if we win or lose.
    if (_countdown > 0) {
        CULog( " countdown is %d",_countdown);
        _countdown--;
    }
    else if (_countdown == 0) {
        CULog( "resetting countdown is 0");
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
bool GameScene::isEnemyBody(physics2::Obstacle* b, const std::string* f ) {
    std::shared_ptr<JsonValue> enemyJ = _constantsJSON->get("enemy");
    return (b->getName() == enemyJ->getString("name") && f == _testEnemy->getBodyName());
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
    
    if (!fix1->GetUserData().pointer || !fix2->GetUserData().pointer) {
        //CULog("Error: fix1 or fix2 has null user data.");//projectiles don't have names so they will error
    }
        else{
            CULog("fix1 is %s and fix2 is %s", fd1->c_str(), fd2->c_str());
        }

    physics2::Obstacle* bd1 = reinterpret_cast<physics2::Obstacle*>(body1->GetUserData().pointer);
    physics2::Obstacle* bd2 = reinterpret_cast<physics2::Obstacle*>(body2->GetUserData().pointer);
    
    // Player-Enemy Collision
    if (bd1->getName() == enemy_name && isPlayerBody(bd2, fd2)) {
        if (((EnemyModel*)bd1)->isDashActive() && !_player->isDashActive()) {
            _player->damage(20);
            ((EnemyModel*)bd1)->setDashRem(0);
            CULog("Player damaged by enemy, remaining HP %f", _player->getHP());
        }
        else if (!((EnemyModel*)bd1)->isDashActive() && _player->isDashActive() && !_player->isGuardActive()) {
            ((EnemyModel*)bd1)->damage(5);
            _player->setDashRem(0);
            CULog("Enemy damaged by player, remaining HP %f", _testEnemy->getHP());
        }
        else if (((EnemyModel*)bd1)->isDashActive() && _player->isDashActive()) {
            ((EnemyModel*)bd1)->setDashRem(0);
            _player->setDashRem(0);
            CULog("Attacks canceled");
        }
        //CULog("Applying knockback");
        _player->setKnocked(true, _player->getPosition().subtract(bd1->getPosition()).normalize());
        ((EnemyModel*)bd1)->setKnocked(true, bd1->getPosition().subtract(_player-> getPosition()).normalize());
    }
    else if (bd2->getName() == enemy_name && isPlayerBody(bd1, fd1)) {
        if (((EnemyModel*)bd2)->isDashActive() && !_player->isDashActive()) {
            _player->damage(20);
            ((EnemyModel*)bd2)->setDashRem(0);
            CULog("Player damaged by enemy, remaining HP %f", _player->getHP());
        }
        else if (!((EnemyModel*)bd2)->isDashActive() && _player->isDashActive() && !_player->isGuardActive()) {
            ((EnemyModel*)bd2)->damage(5);
            _player->setDashRem(0);
            CULog("Enemy damaged by player, remaining HP %f", _testEnemy->getHP());
        }
        else if (((EnemyModel*)bd2)->isDashActive() && _player->isDashActive()) {
            ((EnemyModel*)bd2)->setDashRem(0);
            _player->setDashRem(0);
            CULog("Attacks canceled");
        }
        _player->setKnocked(true, _player->getPosition().subtract(bd2->getPosition()).normalize());
        ((EnemyModel*)bd2)->setKnocked(true, bd2->getPosition().subtract(_player->getPosition()).normalize());
        //CULog("Applying knockback");
    }

    // Player-Projectile Collision
    if (isPlayerBody(bd1, fd1) && bd2->getName() == proj_name) {
        if (!((Projectile*)bd2)->getIsPlayerFired()) {
            _player->damage(20);
            removeProjectile((Projectile*)bd2);
            CULog("Player Damaged, remaining HP %f", _player->getHP());
        }
    }
    else if (isPlayerBody(bd2, fd2) && bd1->getName() == proj_name) {
        if (!((Projectile*)bd1)->getIsPlayerFired()) {
            _player->damage(20);
            removeProjectile((Projectile*)bd1);
            CULog("Player Damaged, remaining HP %f", _player->getHP());
        }
        // TODO: REFACTOR TO NOT REPEAT CODE!!!
    }

    // Shield-Enemy Collision
    if (bd1->getName() == enemy_name && fd2 == _player->getShieldName()) {
        if (((EnemyModel*)bd1)->isDashActive() && _player->isParryActive()) {
            ((EnemyModel*)bd1)->setDashRem(0);
            ((EnemyModel*)bd1)->setStun(120);
        }
        else if (((EnemyModel*)bd1)->isDashActive() && _player->isGuardActive()) {
            _player->damage(10);
            ((EnemyModel*)bd1)->setDashRem(0);
        }
    }

    if (bd2->getName() == enemy_name && fd1 == _player->getShieldName()) {
        if (((EnemyModel*)bd2)->isDashActive() && _player->isParryActive()) {
            ((EnemyModel*)bd2)->setDashRem(0);
            ((EnemyModel*)bd2)->setStun(120);
        }
        else if (((EnemyModel*)bd2)->isDashActive() && _player->isGuardActive()) {
            _player->damage(10);
            ((EnemyModel*)bd2)->setDashRem(0);
        }
    }

    // Shield-Projectile Collision
    Projectile* shieldHit = getProjectileHitShield(bd1, fd1, bd2, fd2);
    if (shieldHit){

        if (_player->isParryActive()) {
            CULog("Parried projectile");

            if (!_player->hasProjectile()) {
                _player->setHasProjectile(true);
            }
        }
        else {
            CULog("Guarded projectile");

            _player->damage(10);
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
    if (bd1->getName() == proj_name && (bd2->getName() == ground_name||bd2->getName() == wall_name)) {
        removeProjectile((Projectile*)bd1);
    }
    else if (bd2->getName() == proj_name && (bd1->getName() == ground_name||bd1->getName() == wall_name)) {
        removeProjectile((Projectile*)bd2);
    }

    // Enemy-Projectile Collision
    if (bd1->getName() == enemy_name && bd2->getName() == proj_name) {

        if (((Projectile*)bd2)->getIsPlayerFired()) {
            CULog("Enemy Damaged, remaining HP %f", ((EnemyModel*)bd1)->getHP());
            ((EnemyModel*)bd1)->damage(20);
            removeProjectile((Projectile*)bd2);
        }
    }
    else if (bd2->getName() == enemy_name && bd1->getName() == proj_name) {
        if (((Projectile*)bd1)->getIsPlayerFired()) {
            CULog("Enemy Damaged, remaining HP %f", ((EnemyModel*)bd2)->getHP());
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
