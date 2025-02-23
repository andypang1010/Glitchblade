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
#include "GBDudeModel.h"
#include "GBSpinner.h"
#include "GBRopeBridge.h"
#include "GBBullet.h"

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

float WALL[WALL_COUNT][WALL_VERTS] = {
	{16.0f, 18.0f,  0.0f, 18.0f,  0.0f,  0.0f,
      1.0f,  0.0f,  1.0f, 17.0f, 16.0f, 17.0f },
	{32.0f, 18.0f, 16.0f, 18.0f, 16.0f, 17.0f,
     31.0f, 17.0f, 31.0f,  0.0f, 32.0f,  0.0f }
};

/** The number of platforms */
#define PLATFORM_VERTS  8
#define PLATFORM_COUNT  10

/** The outlines of all of the platforms */
float PLATFORMS[PLATFORM_COUNT][PLATFORM_VERTS] = {
	{ 1.0f, 3.0f, 1.0f, 2.5f, 6.0f, 2.5f, 6.0f, 3.0f},
	{ 6.0f, 4.0f, 6.0f, 2.5f, 9.0f, 2.5f, 9.0f, 4.0f},
	{23.0f, 4.0f,23.0f, 2.5f,31.0f, 2.5f,31.0f, 4.0f},
	{26.0f, 5.5f,26.0f, 5.0f,28.0f, 5.0f,28.0f, 5.5f},
	{29.0f, 7.0f,29.0f, 6.5f,31.0f, 6.5f,31.0f, 7.0f},
	{24.0f, 8.5f,24.0f, 8.0f,27.0f, 8.0f,27.0f, 8.5f},
	{29.0f,10.0f,29.0f, 9.5f,31.0f, 9.5f,31.0f,10.0f},
	{23.0f,11.5f,23.0f,11.0f,27.0f,11.0f,27.0f,11.5f},
	{19.0f,12.5f,19.0f,12.0f,23.0f,12.0f,23.0f,12.5f},
	{ 1.0f,12.5f, 1.0f,12.0f, 7.0f,12.0f, 7.0f,12.5f}
};

/** The goal door position */
float GOAL_POS[] = { 4.0f,14.0f};
/** The position of the spinning barrier */
float SPIN_POS[] = {13.0f,12.5f};
/** The initial position of the dude */
float DUDE_POS[] = { 2.5f, 5.0f};
/** The position of the rope bridge */
float BRIDGE_POS[] = {9.0f, 3.8f};

#pragma mark -
#pragma mark Physics Constants
/** The new heavier gravity for this world (so it is not so floaty) */
#define DEFAULT_GRAVITY -28.9f
/** The density for most physics objects */
#define BASIC_DENSITY   0.0f
/** The density for a bullet */
#define HEAVY_DENSITY   10.0f
/** Friction of most platforms */
#define BASIC_FRICTION  0.4f
/** The restitution for all physics objects */
#define BASIC_RESTITUTION   0.1f
/** The width of the rope bridge */
#define BRIDGE_WIDTH    14.0f
/** Offset for bullet when firing */
#define BULLET_OFFSET   0.5f
/** The speed of the bullet after firing */
#define BULLET_SPEED   20.0f
/** The number of frame to wait before reinitializing the game */
#define EXIT_COUNT      240


#pragma mark -
#pragma mark Asset Constants
/** The key for the earth texture in the asset manager */
#define EARTH_TEXTURE   "earth"
/** The key for the win door texture in the asset manager */
#define GOAL_TEXTURE    "goal"
/** The key for the win door texture in the asset manager */
#define BULLET_TEXTURE  "bullet"
/** The name of a bullet (for object identification) */
#define BULLET_NAME     "bullet"
/** The name of a wall (for object identification) */
#define WALL_NAME       "wall"
/** The name of a platform (for object identification) */
#define PLATFORM_NAME   "platform"
/** The font for victory/failure messages */
#define MESSAGE_FONT    "retro"
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
	_avatar(nullptr),
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
    return init(assets,Rect(0,0,DEFAULT_WIDTH,DEFAULT_HEIGHT),Vec2(0,DEFAULT_GRAVITY));
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
    return init(assets,rect,Vec2(0,DEFAULT_GRAVITY));
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
    } else if (!Scene2::initWithHint(Size(SCENE_WIDTH,SCENE_HEIGHT))) {
        return false;
    }
    
    // Start up the input handler
    _assets = assets;
    _input.init(getBounds());
    
    // Create the world and attach the listeners.
    _world = physics2::ObstacleWorld::alloc(rect,gravity);
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
    _scale = _size.width == SCENE_WIDTH ? _size.width/rect.size.width : _size.height/rect.size.height;
    Vec2 offset((_size.width-SCENE_WIDTH)/2.0f,(_size.height-SCENE_HEIGHT)/2.0f);

    // Create the scene graph
    std::shared_ptr<Texture> image;
    _worldnode = scene2::SceneNode::alloc();
    _worldnode->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    _worldnode->setPosition(offset);

    _debugnode = scene2::SceneNode::alloc();
    _debugnode->setScale(_scale); // Debug node draws in PHYSICS coordinates
    _debugnode->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    _debugnode->setPosition(offset);

    _winnode = scene2::Label::allocWithText(WIN_MESSAGE, _assets->get<Font>(MESSAGE_FONT));
    _winnode->setAnchor(Vec2::ANCHOR_CENTER);
    _winnode->setPosition(_size.width/2.0f,_size.height/2.0f);
    _winnode->setForeground(WIN_COLOR);
    setComplete(false);

    _losenode = scene2::Label::allocWithText(LOSE_MESSAGE, _assets->get<Font>(MESSAGE_FONT));
    _losenode->setAnchor(Vec2::ANCHOR_CENTER);
    _losenode->setPosition(_size.width/2.0f,_size.height/2.0f);
    _losenode->setForeground(LOSE_COLOR);
    setFailure(false);

    
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
    setDebug(false);
    
    // XNA nostalgia
    Application::get()->setClearColor(Color4f::CORNFLOWER);
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
    _avatar = nullptr;
    _goalDoor = nullptr;
    _spinner = nullptr;
    _ropebridge = nullptr;
      
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
    
    
#pragma mark : Goal door
	std::shared_ptr<Texture> image = _assets->get<Texture>(GOAL_TEXTURE);
	std::shared_ptr<scene2::PolygonNode> sprite;
	std::shared_ptr<scene2::WireNode> draw;

	// Create obstacle
	Vec2 goalPos = GOAL_POS;
	Size goalSize(image->getSize().width/_scale,
	image->getSize().height/_scale);
	_goalDoor = physics2::BoxObstacle::alloc(goalPos,goalSize);
	
	// Set the physics attributes
	_goalDoor->setBodyType(b2_staticBody);
	_goalDoor->setDensity(0.0f);
	_goalDoor->setFriction(0.0f);
	_goalDoor->setRestitution(0.0f);
	_goalDoor->setSensor(true);

	// Add the scene graph nodes to this object
	sprite = scene2::PolygonNode::allocWithTexture(image);
	_goalDoor->setDebugColor(DEBUG_COLOR);
	addObstacle(_goalDoor, sprite);

#pragma mark : Walls
	// All walls and platforms share the same texture
	image  = _assets->get<Texture>(EARTH_TEXTURE);
	std::string wname = "wall";
	for (int ii = 0; ii < WALL_COUNT; ii++) {
		std::shared_ptr<physics2::PolygonObstacle> wallobj;

		Poly2 wall(reinterpret_cast<Vec2*>(WALL[ii]),WALL_VERTS/2);
		// Call this on a polygon to get a solid shape
		EarclipTriangulator triangulator;
		triangulator.set(wall.vertices);
		triangulator.calculate();
		wall.setIndices(triangulator.getTriangulation());
        triangulator.clear();

		wallobj = physics2::PolygonObstacle::allocWithAnchor(wall,Vec2::ANCHOR_CENTER);
		// You cannot add constant "".  Must stringify
		wallobj->setName(std::string(WALL_NAME)+strtool::to_string(ii));
		wallobj->setName(wname);

		// Set the physics attributes
		wallobj->setBodyType(b2_staticBody);
		wallobj->setDensity(BASIC_DENSITY);
		wallobj->setFriction(BASIC_FRICTION);
		wallobj->setRestitution(BASIC_RESTITUTION);
		wallobj->setDebugColor(DEBUG_COLOR);

		wall *= _scale;
		sprite = scene2::PolygonNode::allocWithTexture(image,wall);
		addObstacle(wallobj,sprite,1);  // All walls share the same texture
	}

#pragma mark : Platforms
	for (int ii = 0; ii < PLATFORM_COUNT; ii++) {
		std::shared_ptr<physics2::PolygonObstacle> platobj;
		Poly2 platform(reinterpret_cast<Vec2*>(PLATFORMS[ii]),4);

		EarclipTriangulator triangulator;
		triangulator.set(platform.vertices);
		triangulator.calculate();
		platform.setIndices(triangulator.getTriangulation());
        triangulator.clear();

        platobj = physics2::PolygonObstacle::allocWithAnchor(platform,Vec2::ANCHOR_CENTER);
		// You cannot add constant "".  Must stringify
		platobj->setName(std::string(PLATFORM_NAME)+strtool::to_string(ii));

		// Set the physics attributes
		platobj->setBodyType(b2_staticBody);
		platobj->setDensity(BASIC_DENSITY);
		platobj->setFriction(BASIC_FRICTION);
		platobj->setRestitution(BASIC_RESTITUTION);
		platobj->setDebugColor(DEBUG_COLOR);

		platform *= _scale;
		sprite = scene2::PolygonNode::allocWithTexture(image,platform);
		addObstacle(platobj,sprite,1);
	}

#pragma mark : Spinner
	Vec2 spinPos = SPIN_POS;
    image = _assets->get<Texture>(SPINNER_TEXTURE);
	_spinner = Spinner::alloc(spinPos,image->getSize()/_scale,_scale);
    _spinner->setTexture(image);
	std::shared_ptr<scene2::SceneNode> node = scene2::SceneNode::alloc();
    
    // With refactor, must be added manually
    // Add the node to the world before calling setSceneNode,
    _worldnode->addChild(node);
    _spinner->setSceneNode(node);

    _spinner->setDrawScale(_scale);
    _spinner->setDebugColor(DEBUG_COLOR);
    _spinner->setDebugScene(_debugnode);
    _spinner->activate(_world);

#pragma mark : Rope Bridge
	Vec2 bridgeStart = BRIDGE_POS;
	Vec2 bridgeEnd   = bridgeStart;
	bridgeEnd.x += BRIDGE_WIDTH;
    image = _assets->get<Texture>(BRIDGE_TEXTURE);
    
	_ropebridge = RopeBridge::alloc(bridgeStart,bridgeEnd,image->getSize()/_scale,_scale);
    _ropebridge->setTexture(image);
	node = scene2::SceneNode::alloc();

    // With refactor, must be added manually
    // Add the node to the world before calling setSceneNode,
    _worldnode->addChild(node);
    _ropebridge->setSceneNode(node);
    
    _ropebridge->setDrawScale(_scale);
    _ropebridge->setDebugColor(DEBUG_COLOR);
    _ropebridge->setDebugScene(_debugnode);
    _ropebridge->activate(_world);

#pragma mark : Dude
	Vec2 dudePos = DUDE_POS;
	node = scene2::SceneNode::alloc();
    image = _assets->get<Texture>(DUDE_TEXTURE);
	_avatar = DudeModel::alloc(dudePos,image->getSize()/_scale,_scale);
	sprite = scene2::PolygonNode::allocWithTexture(image);
	_avatar->setSceneNode(sprite);
	_avatar->setDebugColor(DEBUG_COLOR);
	addObstacle(_avatar,sprite); // Put this at the very front

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
	  	node->setPosition(obj->getPosition()*_scale);
	  }
	  _worldnode->addChild(node);
    
    // Dynamic objects need constant updating
    if (obj->getBodyType() == b2_dynamicBody) {
        scene2::SceneNode* weak = node.get(); // No need for smart pointer in callback
        obj->setListener([=,this](physics2::Obstacle* obs){
            weak->setPosition(obs->getPosition()*_scale);
            weak->setAngle(obs->getAngle());
        });
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
	if (_input.didExit())  {
		Application::get()->quit();
	}

	// Process the movement
    if (_input.withJoystick()) {
        if (_input.getHorizontal() < 0) {
            _leftnode->setVisible(true);
            _rightnode->setVisible(false);
        } else if (_input.getHorizontal() > 0) {
            _leftnode->setVisible(false);
            _rightnode->setVisible(true);
        } else {
            _leftnode->setVisible(false);
            _rightnode->setVisible(false);
        }
        _leftnode->setPosition(_input.getJoystick());
        _rightnode->setPosition(_input.getJoystick());
    } else {
        _leftnode->setVisible(false);
        _rightnode->setVisible(false);
    }
    
	_avatar->setMovement(_input.getHorizontal()*_avatar->getForce());
	_avatar->setJumpInput( _input.didJump());
    _avatar->setGuardInput(_input.didGuard());
	_avatar->applyForce();

	if (_avatar->isJumpBegin() && _avatar->isGrounded()) {
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
	if (_input.didExit())  {
		CULog("Shutting down");
		Application::get()->quit();
	}

	// Process the movement
    if (_input.withJoystick()) {
        if (_input.getHorizontal() < 0) {
            _leftnode->setVisible(true);
            _rightnode->setVisible(false);
        } else if (_input.getHorizontal() > 0) {
            _leftnode->setVisible(false);
            _rightnode->setVisible(true);
        } else {
            _leftnode->setVisible(false);
            _rightnode->setVisible(false);
        }
        _leftnode->setPosition(_input.getJoystick());
        _rightnode->setPosition(_input.getJoystick());
    } else {
        _leftnode->setVisible(false);
        _rightnode->setVisible(false);
    }
    
	_avatar->setMovement(_input.getHorizontal()*_avatar->getForce());
    _avatar->setStrafeLeft(_input.didStrafeLeft());
    _avatar->setStrafeRight(_input.didStrafeRight());
	_avatar->setJumpInput( _input.didJump());
    _avatar->setDashLeftInput(_input.didDashLeft());
    _avatar->setDashRightInput(_input.didDashRight());
    _avatar->setGuardInput(_input.didGuard());
    _avatar->applyForce();

	if (_avatar->isJumpBegin() && _avatar->isGrounded()) {
		std::shared_ptr<Sound> source = _assets->get<Sound>(JUMP_EFFECT);
		AudioEngine::get()->play(JUMP_EFFECT,source,false,EFFECT_VOLUME);
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
    // Turn the physics engine crank.
    _world->update(step);
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
    
    // TODO: Update this demo to support interpolation
    // We can interpolate the rope bridge and spinner as we have the data structures
    _spinner->update(remain);
    _ropebridge->update(remain);

    // Add a bullet AFTER physics allows it to hang in front
    // Otherwise, it looks like bullet appears far away
    _avatar->setShootInput(_input.didFire());
    if (_avatar->isShooting()) {
        createBullet();
    }

    // Record failure if necessary.
    if (!_failed && _avatar->getY() < 0) {
        setFailure(true);
    }

    // Reset the game if we win or lose.
    if (_countdown > 0) {
        _countdown--;
    } else if (_countdown == 0) {
        reset();
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
    bool change = _complete != value;
	_complete = value;
	if (value && change) {
		std::shared_ptr<Sound> source = _assets->get<Sound>(WIN_MUSIC);
		AudioEngine::get()->getMusicQueue()->play(source, false, MUSIC_VOLUME);
		_winnode->setVisible(true);
		_countdown = EXIT_COUNT;
	} else if (!value) {
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
	_failed = value;
	if (value) {
		std::shared_ptr<Sound> source = _assets->get<Sound>(LOSE_MUSIC);
        AudioEngine::get()->getMusicQueue()->play(source, false, MUSIC_VOLUME);
		_losenode->setVisible(true);
		_countdown = EXIT_COUNT;
	} else {
		_losenode->setVisible(false);
		_countdown = -1;
	}
}


/**
 * Add a new bullet to the world and send it in the right direction.
 */
void GameScene::createBullet() {
	float offset = BULLET_OFFSET;
	Vec2 pos = _avatar->getPosition();
	pos.x += (_avatar->isFacingRight() ? offset : -offset);

	std::shared_ptr<Texture> image = _assets->get<Texture>(BULLET_TEXTURE);
	float radius = 0.5f*image->getSize().width/_scale;

	std::shared_ptr<Bullet> bullet = Bullet::alloc(pos, radius);
	bullet->setName(BULLET_NAME);
	bullet->setDensity(HEAVY_DENSITY);
	bullet->setBullet(true);
	bullet->setGravityScale(0);
	bullet->setDebugColor(DEBUG_COLOR);
	bullet->setDrawScale(_scale);

	std::shared_ptr<scene2::PolygonNode> sprite = scene2::PolygonNode::allocWithTexture(image);
	bullet->setSceneNode(sprite);

	// Compute position and velocity
	float speed  = (_avatar->isFacingRight() ? BULLET_SPEED : -BULLET_SPEED);
	bullet->setVX(speed);
	addObstacle(bullet, sprite, 5);

	std::shared_ptr<Sound> source = _assets->get<Sound>(PEW_EFFECT);
	AudioEngine::get()->play(PEW_EFFECT,source, false, EFFECT_VOLUME, true);
}

/**
 * Removes a new bullet from the world.
 *
 * @param  bullet   the bullet to remove
 */
void GameScene::removeBullet(Bullet* bullet) {
  // do not attempt to remove a bullet that has already been removed
	if (bullet->isRemoved()) {
		return;
	}
	_worldnode->removeChild(bullet->getSceneNode());
	bullet->setDebugScene(nullptr);
	bullet->markRemoved(true);

	std::shared_ptr<Sound> source = _assets->get<Sound>(POP_EFFECT);
	AudioEngine::get()->play(POP_EFFECT,source,false,EFFECT_VOLUME, true);
}


#pragma mark -
#pragma mark Collision Handling
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

    physics2::Obstacle* bd1 = reinterpret_cast<physics2::Obstacle*>(body1->GetUserData().pointer);
    physics2::Obstacle* bd2 = reinterpret_cast<physics2::Obstacle*>(body2->GetUserData().pointer);
    
	// Test bullet collision with world
	if (bd1->getName() == BULLET_NAME && bd2 != _avatar.get()) {
		removeBullet((Bullet*)bd1);
	} else if (bd2->getName() == BULLET_NAME && bd1 != _avatar.get()) {
		removeBullet((Bullet*)bd2);
	}

	// See if we have landed on the ground.
	if ((_avatar->getSensorName() == fd2 && _avatar.get() != bd1) ||
		(_avatar->getSensorName() == fd1 && _avatar.get() != bd2)) {
		_avatar->setGrounded(true);
		// Could have more than one ground
		_sensorFixtures.emplace(_avatar.get() == bd1 ? fix2 : fix1);
	}

	// If we hit the "win" door, we are done
	if((bd1 == _avatar.get()   && bd2 == _goalDoor.get()) ||
		(bd1 == _goalDoor.get() && bd2 == _avatar.get())) {
		setComplete(true);
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

	if ((_avatar->getSensorName() == fd2 && _avatar.get() != bd1) ||
		(_avatar->getSensorName() == fd1 && _avatar.get() != bd2)) {
		_sensorFixtures.erase(_avatar.get() == bd1 ? fix2 : fix1);
		if (_sensorFixtures.empty()) {
			_avatar->setGrounded(false);
		}
	}
}
