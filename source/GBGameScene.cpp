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

#pragma mark -
#pragma mark Level Geography

/** This is adjusted by screen aspect ratio to get the height */
#define SCENE_WIDTH 1024
#define SCENE_HEIGHT 576

/** This is the aspect ratio for physics */
#define SCENE_ASPECT 9.0/16.0

/** Width of the game world in Box2d units */
#define DEFAULT_WIDTH   32.0f
/** Height of the game world in Box2d units */
#define DEFAULT_HEIGHT  18.0f

// Since these appear only once, we do not care about the magic numbers.
// In an actual game, this information would go in a data file.
// IMPORTANT: Note that Box2D units do not equal drawing units
/** The wall vertices */
#define WALL_VERTS 12
#define WALL_COUNT  2
#define WALL_THICKNESS 1

float WALL[WALL_COUNT][WALL_VERTS] = {
    {
        DEFAULT_WIDTH / 2, DEFAULT_HEIGHT,
        0.0f, DEFAULT_HEIGHT,
        0.0f,  0.0f,
        WALL_THICKNESS,  0.0f,
        WALL_THICKNESS, DEFAULT_HEIGHT - WALL_THICKNESS,
        DEFAULT_WIDTH / 2, DEFAULT_HEIGHT - WALL_THICKNESS
    },
    {
        DEFAULT_WIDTH, DEFAULT_HEIGHT,
        DEFAULT_WIDTH / 2, DEFAULT_HEIGHT,
        DEFAULT_WIDTH / 2, DEFAULT_HEIGHT - WALL_THICKNESS,
        DEFAULT_WIDTH - WALL_THICKNESS, DEFAULT_HEIGHT - WALL_THICKNESS,
        DEFAULT_WIDTH - WALL_THICKNESS,  0.0f,
        DEFAULT_WIDTH,  0.0f
    }
};

/** The number of ground vertices */
#define GROUND_VERTS  8
#define GROUND_THICKNESS 4

/** The outlines of the ground */
float GROUND[GROUND_VERTS]{
    0.0f, 0.0f,
    DEFAULT_WIDTH, 0.0f,
    DEFAULT_WIDTH, GROUND_THICKNESS,
    0.0f, GROUND_THICKNESS
};

/** The initial position of the dude */
float DUDE_POS[] = { 2.5f, 5.0f };
float ENEMY_POS[] = { 12.5f, 5.0f };

/** Bullet Spawn Points */
Vec2 LEFT_BULLET = { WALL_THICKNESS + 1.5f, 9.0f };
Vec2 RIGHT_BULLET = { DEFAULT_WIDTH - WALL_THICKNESS - 1.5f, 13.0f };

/** Directions */
Vec2 LEFT = { -1.0f, 0.0f };
Vec2 RIGHT = { 1.0f, 0.0f };
Vec2 UP = { 0.0f, 1.0f };
Vec2 DOWN = { 0.0f, -1.0f };

#pragma mark -
#pragma mark Physics Constants
/** The new heavier gravity for this world (so it is not so floaty) */
#define DEFAULT_GRAVITY -28.9f
/** The density for most physics objects */
#define BASIC_DENSITY   0.0f
/** The density for a projectile */
#define HEAVY_DENSITY   10.0f
/** Friction of most platforms */
#define BASIC_FRICTION  0.4f
/** The restitution for all physics objects */
#define BASIC_RESTITUTION   0.1f
/** Offset for pojectile when firing */
#define PROJECTILE_OFFSET   0.5f
/** The speed of the projectile after firing */
#define PROJECTILE_SPEED   30.0f
/** The number of frame to wait before reinitializing the game */
#define EXIT_COUNT      240

#pragma mark -
#pragma mark Testing Constants
/** The radius for enemy to initial attack */
#define ENEMY_ATTACK_RADIUS     6.0f

#pragma mark -
#pragma mark Asset Constants

///////////////// TEXTURES //////////////////////////////////
/** The key for the ground texture in the asset manager */
#define GROUND_TEXTURE  "ground"
/** The key for the background texture in the asset manager */
#define BKGD_TEXTURE    "background"
/** The key for the regular projectile texture in the asset manager */
#define PROJECTILE_TEXTURE  "projectile"
/** The key for the player projectile texture in the asset manager */
#define PLAYER_PROJECTILE_TEXTURE "player-projectile"

///////////////// NAMES /////////////////////////////////////
#define ENEMY_NAME      "enemy"
#define PROJECTILE_NAME "projectile"
/** The name of a platform (for object identification) */
#define GROUND_NAME   "ground"



/** The font for victory/failure messages */
#define MESSAGE_FONT    "retro"
/** The font for debug UI */
#define DEBUG_FONT      "debug"
/** The message for winning the game */
#define WIN_MESSAGE     "VICTORY!"
/** The color of the win message */
#define WIN_COLOR       Color4::YELLOW
/** The message for losing the game */
#define LOSE_MESSAGE    "FAILURE!"
/** The color of the lose message */
#define LOSE_COLOR      Color4::RED
/** The key the basic game music */
#define GAME_MUSIC      "game"
/** The key the victory game music */
#define WIN_MUSIC       "win"
/** The key the failure game music */
#define LOSE_MUSIC      "lose"
/** The sound effect for firing a bullet */
#define PEW_EFFECT      "pew"
/** The sound effect for a bullet collision */
#define POP_EFFECT      "pop"
/** The sound effect for jumping */
#define JUMP_EFFECT     "jump"
/** The volume for the music */
#define MUSIC_VOLUME    0.7f
/** The volume for sound effects */
#define EFFECT_VOLUME   0.8f
/** The image for the left dpad/joystick */
#define LEFT_IMAGE      "dpad_left"
/** The image for the right dpad/joystick */
#define RIGHT_IMAGE     "dpad_right"
/** The fire rate for spawned bullets */
#define BULLET_SPAWN_RATE     100.0f

/** Color to outline the physics nodes */
#define DEBUG_COLOR     Color4::YELLOW
/** Opacity of the physics outlines */
#define DEBUG_OPACITY   192


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
_player(nullptr),
_complete(false),
_debug(false)
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
    else if (!Scene2::initWithHint(Size(SCENE_WIDTH, SCENE_HEIGHT))) {
        return false;
    }

    std::shared_ptr<JsonReader> reader = JsonReader::allocWithAsset("json/enemies.json");

    _enemiesJSON = reader->readJson();
    if (_enemiesJSON == nullptr) {
        CULog("Failed to load enemies.json");
        return false;
    }

    // Start up the input handler
    _assets = assets;
    _input.init(getBounds());

    // Create the world and attach the listeners.
    _world = physics2::ObstacleWorld::alloc(rect, gravity);
    _world->activateCollisionCallbacks(true);
    _world->onBeginContact = [this](b2Contact* contact) {
        beginContact(contact);
        };
    _world->onEndContact = [this](b2Contact* contact) {
        endContact(contact);
        };

    // IMPORTANT: SCALING MUST BE UNIFORM
    // This means that we cannot change the aspect ratio of the physics world
    // Shift to center if a bad fit
    _scale = _size.width == SCENE_WIDTH ? _size.width / rect.size.width : _size.height / rect.size.height;
    Vec2 offset((_size.width - SCENE_WIDTH) / 2.0f, (_size.height - SCENE_HEIGHT) / 2.0f);

    // Create the scene graph
    std::shared_ptr<Texture> image = _assets->get<Texture>(BKGD_TEXTURE);
    _worldnode = scene2::PolygonNode::allocWithTexture(image);
    _worldnode->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    _worldnode->setPosition(offset);

    _debugnode = scene2::SceneNode::alloc();
    _debugnode->setScale(_scale); // Debug node draws in PHYSICS coordinates
    _debugnode->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    _debugnode->setPosition(offset);

    _winnode = scene2::Label::allocWithText(WIN_MESSAGE, _assets->get<Font>(MESSAGE_FONT));
    _winnode->setAnchor(Vec2::ANCHOR_CENTER);
    _winnode->setPosition(_size.width / 2.0f, _size.height / 2.0f);
    _winnode->setForeground(WIN_COLOR);
    setComplete(false);

    _losenode = scene2::Label::allocWithText(LOSE_MESSAGE, _assets->get<Font>(MESSAGE_FONT));
    _losenode->setAnchor(Vec2::ANCHOR_CENTER);
    _losenode->setPosition(_size.width / 2.0f, _size.height / 2.0f);
    _losenode->setForeground(LOSE_COLOR);
    setFailure(false);

    _playerHPNode = scene2::Label::allocWithText("100", _assets->get<Font>(DEBUG_FONT));
    _playerHPNode->setAnchor(Vec2::ANCHOR_CENTER);
    _playerHPNode->setForeground(Color4::CYAN);
    _playerHPNode->setPosition(0, 55);

    _enemyHPNode = scene2::Label::allocWithText("100", _assets->get<Font>(DEBUG_FONT));
    _enemyHPNode->setAnchor(Vec2::ANCHOR_CENTER);
    _enemyHPNode->setForeground(Color4::RED);
    _enemyHPNode->setPosition(0, 80);

    _enemyStunNode = scene2::Label::allocWithText("STUN", _assets->get<Font>(DEBUG_FONT));
    _enemyStunNode->setAnchor(Vec2::ANCHOR_CENTER);
    _enemyStunNode->setForeground(Color4::RED);
    _enemyStunNode->setPosition(0, 100);

    _leftnode = scene2::PolygonNode::allocWithTexture(_assets->get<Texture>(LEFT_IMAGE));
    _leftnode->SceneNode::setAnchor(Vec2::ANCHOR_MIDDLE_RIGHT);
    _leftnode->setScale(0.35f);
    _leftnode->setVisible(false);

    _rightnode = scene2::PolygonNode::allocWithTexture(_assets->get<Texture>(RIGHT_IMAGE));
    _rightnode->SceneNode::setAnchor(Vec2::ANCHOR_MIDDLE_LEFT);
    _rightnode->setScale(0.35f);
    _rightnode->setVisible(false);

    addChild(_worldnode);
    addChild(_debugnode);
    addChild(_winnode);
    addChild(_losenode);
    addChild(_leftnode);
    addChild(_rightnode);

    populate();
    _active = true;
    _complete = false;
    setDebug(false); // Debug on by default

    // XNA nostalgia
    Application::get()->setClearColor(Color4f::GRAY);

    CULog("Setting up LevelController");
    _levelController = std::make_shared<LevelController>();
    _levelController->init(); // Initialize the LevelController

    return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameScene::dispose() {
    if (_active) {
        _input.dispose();
        _world = nullptr;
        _worldnode = nullptr;
        _debugnode = nullptr;
        _winnode = nullptr;
        _losenode = nullptr;
        _playerHPNode = nullptr;
        _enemyHPNode = nullptr;
        _enemyStunNode = nullptr;
        _leftnode = nullptr;
        _rightnode = nullptr;
        _complete = false;
        _debug = false;
        Scene2::dispose();
    }
}


#pragma mark -
#pragma mark Level Layout

/**
 * Resets the status of the game so that we can play again.
 *
 * This method disposes of the world and creates a new one.
 */
void GameScene::reset() {
    _world->clear();
    _worldnode->removeAllChildren();
    _debugnode->removeAllChildren();
    _player = nullptr;

    setFailure(false);
    setComplete(false);
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

    std::shared_ptr<Texture> image;
    std::shared_ptr<scene2::PolygonNode> sprite;
    std::shared_ptr<scene2::WireNode> draw;
    std::shared_ptr<scene2::SceneNode> node = scene2::SceneNode::alloc();

#pragma mark : Walls
    // All walls and platforms share the same texture
    image = Texture::alloc(1, 1, Texture::PixelFormat::RGBA);
    for (int ii = 0; ii < WALL_COUNT; ii++) {
        std::shared_ptr<physics2::PolygonObstacle> wallobj;

        Poly2 wall(reinterpret_cast<Vec2*>(WALL[ii]), WALL_VERTS / 2);
        // Call this on a polygon to get a solid shape
        EarclipTriangulator triangulator;
        triangulator.set(wall.vertices);
        triangulator.calculate();
        wall.setIndices(triangulator.getTriangulation());
        triangulator.clear();

        wallobj = physics2::PolygonObstacle::allocWithAnchor(wall, Vec2::ANCHOR_CENTER);
        // You cannot add constant "".  Must stringify
        wallobj->setName(std::string(GROUND_NAME));

        // Set the physics attributes
        wallobj->setBodyType(b2_staticBody);
        wallobj->setDensity(BASIC_DENSITY);
        wallobj->setFriction(BASIC_FRICTION);
        wallobj->setRestitution(BASIC_RESTITUTION);
        wallobj->setDebugColor(DEBUG_COLOR);

        wall *= _scale;
        sprite = scene2::PolygonNode::allocWithTexture(image, wall);
        addObstacle(wallobj, sprite, 1);  // All walls share the same texture
    }

#pragma mark : GROUND
    image = _assets->get<Texture>(GROUND_TEXTURE);

    std::shared_ptr<physics2::PolygonObstacle> groundObj;
    Poly2 ground(reinterpret_cast<Vec2*>(GROUND), 4);

    EarclipTriangulator triangulator;
    triangulator.set(ground.vertices);
    triangulator.calculate();
    ground.setIndices(triangulator.getTriangulation());
    triangulator.clear();

    groundObj = physics2::PolygonObstacle::allocWithAnchor(ground, Vec2::ANCHOR_CENTER);
    // You cannot add constant "".  Must stringify
    groundObj->setName(std::string(GROUND_NAME));

    // Set the physics attributes
    groundObj->setBodyType(b2_staticBody);
    groundObj->setDensity(BASIC_DENSITY);
    groundObj->setFriction(BASIC_FRICTION);
    groundObj->setRestitution(BASIC_RESTITUTION);
    groundObj->setDebugColor(DEBUG_COLOR);

    ground *= _scale;
    sprite = scene2::PolygonNode::allocWithTexture(image, ground);
    addObstacle(groundObj, sprite, 1);

#pragma mark : Dude
    Vec2 dudePos = DUDE_POS;
    node = scene2::SceneNode::alloc();
    image = _assets->get<Texture>(DUDE_TEXTURE);
    _player = PlayerModel::alloc(dudePos, image->getSize() / _scale, _scale);
    
    _player->_idleSprite = scene2::SpriteNode::allocWithSheet(_assets->get<Texture>("player_idle"), 1, 6, 6);
    _player->_idleSprite->setPosition(0, 40);

    _player->_walkSprite = scene2::SpriteNode::allocWithSheet(_assets->get<Texture>("player_walk"), 1, 6, 6);
    _player->_walkSprite->setPosition(0, 40);

    _player->_jumpUpSprite = scene2::SpriteNode::allocWithSheet(_assets->get<Texture>("player_jumpUp"), 1, 8, 8);
    _player->_jumpUpSprite->setPosition(0, 40);

    _player->_jumpDownSprite = scene2::SpriteNode::allocWithSheet(_assets->get<Texture>("player_jumpDown"), 1, 8, 8);
    _player->_jumpDownSprite->setPosition(0, 40);

    _player->_guardSprite = scene2::SpriteNode::allocWithSheet(_assets->get<Texture>("player_guard"), 1, 6, 6);
    _player->_guardSprite->setPosition(0, 40);

    _player->_attackSprite = scene2::SpriteNode::allocWithSheet(_assets->get<Texture>("player_attack"), 1, 8, 8);
    _player->_attackSprite->setPosition(0, 40);

    _player->getSceneNode()->addChild(_player->_idleSprite);
    _player->getSceneNode()->addChild(_player->_walkSprite);
    _player->getSceneNode()->addChild(_player->_jumpUpSprite);
    _player->getSceneNode()->addChild(_player->_jumpDownSprite);
    _player->getSceneNode()->addChild(_player->_guardSprite);
    _player->getSceneNode()->addChild(_player->_attackSprite);

    _player->setDebugColor(DEBUG_COLOR);
    addObstacle(_player, _player->getSceneNode()); // Put this at the very front

#pragma mark : Test Enemy
    Vec2 enemyPos = ENEMY_POS;
    std::vector<std::shared_ptr<ActionModel>> actions = LevelController::parseActions(_enemiesJSON, "boss1");
    _testEnemy = EnemyModel::alloc(enemyPos, Size(90, 130) / _scale, _scale, actions);

    _testEnemy->_idleSprite = scene2::SpriteNode::allocWithSheet(_assets->get<Texture>("boss1_idle"), 1, 6, 6);
    _testEnemy->_idleSprite->setPosition(0, 40);

    _testEnemy->_walkSprite = scene2::SpriteNode::allocWithSheet(_assets->get<Texture>("boss1_walking1"), 1, 8, 8);
    _testEnemy->_walkSprite->setPosition(0, 40);

    _testEnemy->_slamSprite = scene2::SpriteNode::allocWithSheet(_assets->get<Texture>("boss1_slam"), 4, 10, 40);
    _testEnemy->_slamSprite->setPosition(0, 40);

    _testEnemy->_stabSprite = scene2::SpriteNode::allocWithSheet(_assets->get<Texture>("boss1_stab"), 4, 10, 40);
    _testEnemy->_stabSprite->setPosition(0, 40);

    _testEnemy->_stunSprite = scene2::SpriteNode::allocWithSheet(_assets->get<Texture>("boss1_stun"), 3, 10, 22);
    _testEnemy->_stunSprite->setPosition(0, 40);

    _testEnemy->setName(std::string(ENEMY_NAME));
    _testEnemy->setDebugColor(DEBUG_COLOR);
    addObstacle(_testEnemy, _testEnemy->getSceneNode()); // Put this at the very front
    
    _testEnemy->getSceneNode()->addChild(_testEnemy->_idleSprite);
    _testEnemy->getSceneNode()->addChild(_testEnemy->_walkSprite);
    _testEnemy->getSceneNode()->addChild(_testEnemy->_slamSprite);
    _testEnemy->getSceneNode()->addChild(_testEnemy->_stabSprite);
    _testEnemy->getSceneNode()->addChild(_testEnemy->_stunSprite);

    // Add UI elements
    _player->getSceneNode()->addChild(_playerHPNode);
    _testEnemy->getSceneNode()->addChild(_enemyStunNode);
    _testEnemy->getSceneNode()->addChild(_enemyHPNode);

	// Play the background music on a loop.
	std::shared_ptr<Sound> source = _assets->get<Sound>(GAME_MUSIC);
    AudioEngine::get()->getMusicQueue()->play(source, true, MUSIC_VOLUME);
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

    // Dynamic objects need constant updating
    if (obj->getBodyType() == b2_dynamicBody) {
        scene2::SceneNode* weak = node.get(); // No need for smart pointer in callback
        obj->setListener([=, this](physics2::Obstacle* obs) {
            weak->setPosition(obs->getPosition() * _scale);
            weak->setAngle(obs->getAngle());
            });
    }
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
        std::shared_ptr<Sound> source = _assets->get<Sound>(WIN_MUSIC);
        AudioEngine::get()->getMusicQueue()->play(source, false, MUSIC_VOLUME);
        _winnode->setVisible(true);
        _countdown = EXIT_COUNT;
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
    _failed = value;
    if (value) {
        std::shared_ptr<Sound> source = _assets->get<Sound>(LOSE_MUSIC);
        AudioEngine::get()->getMusicQueue()->play(source, false, MUSIC_VOLUME);
        _losenode->setVisible(true);
        _countdown = EXIT_COUNT;
    }
    else {
        _losenode->setVisible(false);
        _countdown = -1;
    }
}


#pragma mark -
#pragma mark Physics Handling
/**
 * The method called to update the game mode.
 *
 * This is the nondeterministic version of a physics simulation. It is
 * provided for comparison purposes only.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void GameScene::update(float timestep) {
    _input.update(timestep);

    // Process the toggled key commands
    if (_input.didDebug()) { setDebug(!isDebug()); }
    if (_input.didReset()) { reset(); }
    if (_input.didExit()) {
        Application::get()->quit();
    }

    // Process the movement
    if (_input.withJoystick()) {
        if (_input.getHorizontal() < 0) {
            _leftnode->setVisible(true);
            _rightnode->setVisible(false);
        }
        else if (_input.getHorizontal() > 0) {
            _leftnode->setVisible(false);
            _rightnode->setVisible(true);
        }
        else {
            _leftnode->setVisible(false);
            _rightnode->setVisible(false);
        }
        _leftnode->setPosition(_input.getJoystick());
        _rightnode->setPosition(_input.getJoystick());
    }
    else {
        _leftnode->setVisible(false);
        _rightnode->setVisible(false);
    }
    
	_player->setMovement(_input.getHorizontal()*_player->getForce());
	_player->setJumpInput( _input.didJump());
    _player->setGuardInput(_input.didGuard());
	_player->applyForce();

	if (_player->isJumpBegin() && _player->isGrounded()) {
		std::shared_ptr<Sound> source = _assets->get<Sound>(JUMP_EFFECT);
		AudioEngine::get()->play(JUMP_EFFECT,source,false,EFFECT_VOLUME);
	}

    	
	// Turn the physics engine crank.
    _world->update(timestep);
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
void GameScene::preUpdate(float dt) {
    _input.update(dt);

    // Process the toggled key commands
    if (_input.didDebug()) { setDebug(!isDebug()); }
    if (_input.didReset()) { reset(); }
    if (_input.didExit()) {
        CULog("Shutting down");
        Application::get()->quit();
    }

    // Process the movement
    if (_input.withJoystick()) {
        if (_input.getHorizontal() < 0) {
            _leftnode->setVisible(true);
            _rightnode->setVisible(false);
        }
        else if (_input.getHorizontal() > 0) {
            _leftnode->setVisible(false);
            _rightnode->setVisible(true);
        }
        else {
            _leftnode->setVisible(false);
            _rightnode->setVisible(false);
        }
        _leftnode->setPosition(_input.getJoystick());
        _rightnode->setPosition(_input.getJoystick());
    }
    else {
        _leftnode->setVisible(false);
        _rightnode->setVisible(false);
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
    
    _player->setMovement(_input.getHorizontal()*_player->getForce());
    _player->setStrafeLeft(_input.didStrafeLeft());
    _player->setStrafeRight(_input.didStrafeRight());
	_player->setJumpInput( _input.didJump());
    _player->setDashLeftInput(_input.didDashLeft());
    _player->setDashRightInput(_input.didDashRight());
    _player->setGuardInput(_input.didGuard());
    _player->applyForce();

    _testEnemy->setTargetPos(_player->getPosition());
    //_testEnemy->setDashLeftInput(_input.didDashLeft());
    //_testEnemy->setDashRightInput(dist < 0 && dist > -ENEMY_ATTACK_RADIUS);
    _testEnemy->applyForce();
    
    _playerHPNode->setText(std::to_string((int)_player->getHP()));
    _enemyHPNode->setText(std::to_string((int)_testEnemy->getHP()));
    _enemyStunNode->setText((_testEnemy->isStunned() ? "STUN" : ""));
    

    if (_player->isJumpBegin() && _player->isGrounded()) {
      std::shared_ptr<Sound> source = _assets->get<Sound>(JUMP_EFFECT);
      AudioEngine::get()->play(JUMP_EFFECT,source,false,EFFECT_VOLUME);
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

    // Add a bullet AFTER physics allows it to hang in front
    // Otherwise, it looks like bullet appears far away
    _player->setShootInput(_input.didFire());
    if (_player->isShooting() && _player->hasProjectile()) {
        createProjectile(_player->getPosition(), _player->isFacingRight() ? Vec2(1, 0) : Vec2(-1, 0), true);
        _player->setHasProjectile(false);
    }

//    if (_bulletTimer <= 0) {
//        int r = rand() % 2;
//        if (r == 0) {
//            createProjectile(LEFT_BULLET, RIGHT, false);
//        }
//        else if(r == 1){
//            createProjectile(RIGHT_BULLET, LEFT, false);
//        }
//
//        _bulletTimer = BULLET_SPAWN_RATE;
//    }
//    else {
//        _bulletTimer -= 1;
//    }

    setComplete(_testEnemy->getHP() <= 0);
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
 * Add a new projectile to the world and send it in the right direction.
 */
void GameScene::createProjectile(Vec2 pos, Vec2 direction, bool isPlayerFired) {
	float offset = PROJECTILE_OFFSET;
    if (isPlayerFired) {
        pos.x += (_player->isFacingRight() ? offset : -offset);
        pos.y += 0.5f;
    }

    std::shared_ptr<Texture> image = _assets->get<Texture>(isPlayerFired ? PLAYER_PROJECTILE_TEXTURE : PROJECTILE_TEXTURE);
    float radius = 0.5f * image->getSize().width / _scale;

    // Change last parameter to test player-fired or regular projectile
    std::shared_ptr<Projectile> projectile = Projectile::alloc(pos, radius, isPlayerFired);
    projectile->setName(PROJECTILE_NAME);
    projectile->setDensity(HEAVY_DENSITY);
    projectile->setBullet(true);
    projectile->setGravityScale(0);
    projectile->setDebugColor(DEBUG_COLOR);
    projectile->setDrawScale(_scale);
    projectile->setSensor(true);
    projectile->setIsPlayerFired(isPlayerFired);

    std::shared_ptr<scene2::PolygonNode> sprite = scene2::PolygonNode::allocWithTexture(image);
    projectile->setSceneNode(sprite);

    sprite->flipHorizontal(direction.x < 0);

    // Compute position and velocity
    Vec2 speed = isPlayerFired ? direction.getNormalization()*PROJECTILE_SPEED : direction.getNormalization() * PROJECTILE_SPEED / 2;
    projectile->setLinearVelocity(speed);
    addObstacle(projectile, sprite, 5);

    std::shared_ptr<Sound> source = _assets->get<Sound>(PEW_EFFECT);
    AudioEngine::get()->play(PEW_EFFECT, source, false, EFFECT_VOLUME, true);
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

    std::shared_ptr<Sound> source = _assets->get<Sound>(POP_EFFECT);
    AudioEngine::get()->play(POP_EFFECT, source, false, EFFECT_VOLUME, true);
}

#pragma mark -
#pragma mark Collision Handling

/**Checks obstacle and fixture to see if it is an enemy body fixture.**/
bool GameScene::isEnemyBody(physics2::Obstacle* b, const std::string* f ) {
    return (b->getName() == ENEMY_NAME && f == _testEnemy->getBodyName());
}
/**Checks obstacle and fixture to see if it the player body fixture.**/
bool GameScene::isPlayerBody(physics2::Obstacle* b, const std::string* f ) {
    return (b == _player.get() && f == _player->getBodyName());
}

/**
 Checks if contact is projectile hitting player shield and returns the Projectile if so, else NULL.
 */
Projectile* GameScene::getProjectileHitShield(physics2::Obstacle* bd1, std::string* fd1,
                                              physics2::Obstacle* bd2, std::string* fd2) const {
    if (bd1->getName() == PROJECTILE_NAME && fd2 == _player->getShieldName() &&
        !((Projectile*)bd1)->getIsPlayerFired() && _player->isGuardActive()) {
        return (Projectile*)bd1;
    }
    if (bd2->getName() == PROJECTILE_NAME && fd1 == _player->getShieldName() &&
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
    if (bd1->getName() == ENEMY_NAME && isPlayerBody(bd2, fd2)) {
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
        CULog("Applying knockback");
        _player->setKnocked(true, _player->getPosition().subtract(bd1->getPosition()).normalize());
        ((EnemyModel*)bd1)->setKnocked(true, bd1->getPosition().subtract(_player->getPosition()).normalize());
    }
    else if (bd2->getName() == ENEMY_NAME && isPlayerBody(bd1, fd1)) {
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
        CULog("Applying knockback");
    }

    // Player-Projectile Collision
    if (isPlayerBody(bd1, fd1) && bd2->getName() == PROJECTILE_NAME) {
        if (!((Projectile*)bd2)->getIsPlayerFired()) {
            _player->damage(20);
            removeProjectile((Projectile*)bd2);
            CULog("Player Damaged, remaining HP %f", _player->getHP());
        }
    }
    else if (isPlayerBody(bd2, fd2) && bd1->getName() == PROJECTILE_NAME) {
        if (!((Projectile*)bd1)->getIsPlayerFired()) {
            _player->damage(20);
            removeProjectile((Projectile*)bd1);
            CULog("Player Damaged, remaining HP %f", _player->getHP());
        }
        // TODO: REFACTOR TO NOT REPEAT CODE!!!
    }

    // Shield-Enemy Collision
    if (bd1->getName() == ENEMY_NAME && fd2 == _player->getShieldName()) {
        if (((EnemyModel*)bd1)->isDashActive() && _player->isParryActive()) {
            ((EnemyModel*)bd1)->setDashRem(0);
            ((EnemyModel*)bd1)->setStun(120);
        }
        else if (((EnemyModel*)bd1)->isDashActive() && _player->isGuardActive()) {
            _player->damage(10);
            ((EnemyModel*)bd1)->setDashRem(0);
        }
    }

    if (bd2->getName() == ENEMY_NAME && fd1 == _player->getShieldName()) {
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
    if (bd1->getName() == PROJECTILE_NAME && bd2->getName() == PROJECTILE_NAME) {

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
    if (bd1->getName() == PROJECTILE_NAME && bd2->getName() == GROUND_NAME) {
        removeProjectile((Projectile*)bd1);
    }
    else if (bd2->getName() == PROJECTILE_NAME && bd1->getName() == GROUND_NAME) {
        removeProjectile((Projectile*)bd2);
    }

    // Enemy-Projectile Collision
    if (bd1->getName() == ENEMY_NAME && bd2->getName() == PROJECTILE_NAME) {

        if (((Projectile*)bd2)->getIsPlayerFired()) {
            CULog("Enemy Damaged, remaining HP %f", ((EnemyModel*)bd1)->getHP());
            ((EnemyModel*)bd1)->damage(20);
            removeProjectile((Projectile*)bd2);
        }
    }
    else if (bd2->getName() == ENEMY_NAME && bd1->getName() == PROJECTILE_NAME) {
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
