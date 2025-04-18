//
//  GBPlayerModel.cpp
//  PlatformDemo
//
//  This encapsulates all of the information for the character avatar.  Note how this
//  class combines physics and animation.  This is a good template for models in
//  your game.
//
//  WARNING: There are a lot of shortcuts in this design that will do not adapt well
//  to data driven design.  This demo has a lot of simplifications to make it a bit
//  easier to see how everything fits together.  However, the model classes and how
//  they are initialized will need to be changed if you add dynamic level loading.
//
//  Pay close attention to how this class designed.  Subclasses of Cocos2d classes
//  (which are all subclasses of the class Ref) should never have normal public
//  constructors.  Instead, you should organize their constructors into three parts,
//  just like we have done in this class.
//
//  NORMAL CONSTRUCTOR:
//  The standard constructor should be protected (not private).  It should only
//  initialize pointers to nullptr and primitives to their defaults (pointers are
//  not always nullptr to begin with).  It should NOT take any arguments and should
//  not allocate any memory or call any methods.
//
//  STATIC CONSTRUCTOR
//  This is a static method that allocates the object and initializes it.  If
//  initialization fails, it immediately disposes of the object.  Otherwise, it
//  returns an autoreleased object, starting the garbage collection system.
//  These methods all look the same.  You can copy-and-paste them from sample code.
//  The only difference is the init method called.
//
//  INIT METHOD
//  This is a protected method that acts like what how would normally think a
//  constructor might work.  It allocates memory and initializes all values
//  according to provided arguments.  As memory allocation can fail, this method
//  needs to return a boolean indicating whether or not initialization was
//  successful.
//
//  This file is based on the CS 3152 PhysicsDemo Lab by Don Holden, 2007
//
//  Author: Walker White and Anthony Perello
//  Version:  2/9/17
//
#include "GBPlayerModel.h"
#include <cugl/scene2/CUPolygonNode.h>
#include <cugl/scene2/CUTexturedNode.h>
#include <cugl/core/assets/CUAssetManager.h>
using namespace cugl::graphics;


using namespace cugl;

#pragma mark -
#pragma mark Constructors

/**
 * Initializes a new player at the given position.
 *
 * The player is sized according to the given drawing scale.
 *
 * The scene graph is completely decoupled from the physics system.
 * The node does not have to be the same size as the physics body. We
 * only guarantee that the scene graph node is positioned correctly
 * according to the drawing scale.
 *
 * @param pos   Initial position in world coordinates
 * @param size  The size of the player in world units
 * @param scale The drawing scale (world to screen)
 *
 * @return  true if the obstacle is initialized properly, false otherwise.
 */
bool PlayerModel::init(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& constantsRef, const Vec2& pos) {
    _playerJSON = constantsRef->get("player");
    setConstants();
    resetAttributes();
    float scale = constantsRef->get("scene")->getFloat("scale");
    std::shared_ptr<graphics::Texture> image;
    image = assetRef->get<graphics::Texture>(_playerJSON->getString("texture"));
    Size nsize = image->getSize() / scale;
    nsize.width *= _playerJSON->get("fixtures")->get("body")->getFloat("h_shrink");
    nsize.height *= _playerJSON->get("fixtures")->get("body")->getFloat("v_shrink");
    _drawScale = scale;

    setDebugColor(Color4::BLACK);

    if (BoxObstacle::init(pos, nsize)) {
        setDensity(_playerJSON->getFloat("density"));
        setFriction(0.0f);      // HE WILL STICK TO WALLS IF YOU FORGET
        setFixedRotation(true); // OTHERWISE, HE IS A WEEBLE WOBBLE

        // set the scene node and attach the sprite nodes to it
        attachNodes(assetRef);
        return true;
    }
    return false;
}


void PlayerModel::attachNodes(const std::shared_ptr<AssetManager>& assetRef) {
    _sceneNode = scene2::SceneNode::alloc();
    setSceneNode(_sceneNode);

    _idleSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("player_idle"), 3, 4, 11);
    _idleSprite->setPosition(0, -25);
    _idleSprite->setScale(0.5f);

    _walkSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("player_walk"), 2, 4, 5);
    _walkSprite->setPosition(0, -25);
    _walkSprite->setScale(0.5f);

    _jumpUpSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("player_jumpUp"), 2, 4, 5);
    _jumpUpSprite->setPosition(0, -25);
    _jumpUpSprite->setScale(0.5f);

    _jumpDownSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("player_jumpDown"), 2, 4, 7);
    _jumpDownSprite->setPosition(0, -25);
    _jumpDownSprite->setScale(0.5f);

    _guardSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("player_guard"), 9, 4, 36);
    _guardSprite->setPosition(0, -25);
    _guardSprite->setScale(0.5f);

    _guardReleaseSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("player_guard_release"), 2, 4, 5);
    _guardReleaseSprite->setPosition(0, -25);
    _guardReleaseSprite->setScale(0.5f);

    _parryReleaseSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("player_parry_release"), 2, 4, 5);
    _parryReleaseSprite->setPosition(0, -25);
    _parryReleaseSprite->setScale(0.5f);

    _damagedSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("player_damaged"), 2, 4, 5);
    _damagedSprite->setPosition(0, -25);
    _damagedSprite->setScale(0.5f);

    _attackSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("player_attack"), 2, 4, 5);
    _attackSprite->setPosition(0, -25);
    _attackSprite->setScale(0.5f);

    getSceneNode()->addChild(_idleSprite);
    getSceneNode()->addChild(_walkSprite);
    getSceneNode()->addChild(_jumpUpSprite);
    getSceneNode()->addChild(_jumpDownSprite);
    getSceneNode()->addChild(_guardSprite);
    getSceneNode()->addChild(_guardReleaseSprite);
	getSceneNode()->addChild(_parryReleaseSprite);
    getSceneNode()->addChild(_attackSprite);
    getSceneNode()->addChild(_damagedSprite);
    
    setName(_name);
}

#pragma mark -
#pragma mark Attribute Properties

/**
* Reduces the health of this character.
*
* @param value the amount of hp reduction.
*/
void PlayerModel::damage(float value) {
    _hp -= value;
    _hp = _hp < 0 ? 0 : _hp;
	//_sceneNode->setColor(Color4::RED);
    _lastDamagedFrame = 0;
    if (!isGuardActive()) {
        _damageRem = PLAYER_HIT_COLOR_DURATION;
    }
    else {
        if (isParryActive()) {
            _guardState = 2;
		}
        else {
            _guardState = 3;
        }
        _parryRem = 0;
        _guardRem = 0;
        _guardCooldownRem = 0;
        _guardReleaseRem = PLAYER_HIT_COLOR_DURATION;
    }
}

/**
 * Sets left/right movement of this character.
 *
 * This is the result of input times player force.
 *
 * @param value left/right movement of this character.
 */
void PlayerModel::setMovement(float value) {
    _movement = value;
    bool face = _movement > 0;
    if (_movement == 0 || _faceRight == face) {
        return;
    }

    // Change facing
    scene2::TexturedNode* image = dynamic_cast<scene2::TexturedNode*>(_sceneNode.get());
    if (image != nullptr) {
        // Don't flip if it means overriding a dash direction
        if (!isDashActive()) {
            image->flipHorizontal(!image->isFlipHorizontal());
        }
    }

    if (!isDashActive()) {
        _faceRight = face;
    }
}

/**
* Make the sprite face left
*/
void PlayerModel::faceLeft() {
    if (_faceRight == true) {
        _faceRight = false;
        scene2::TexturedNode* image = dynamic_cast<scene2::TexturedNode*>(_sceneNode.get());
        if (image != nullptr) {
            image->flipHorizontal(!image->isFlipHorizontal());
        }
    }
}

/**
* Make the sprite face right
*/
void PlayerModel::faceRight() {
    if (_faceRight == false) {
        _faceRight = true;
        scene2::TexturedNode* image = dynamic_cast<scene2::TexturedNode*>(_sceneNode.get());
        if (image != nullptr) {
            image->flipHorizontal(!image->isFlipHorizontal());
        }
    }
}

#pragma mark -
#pragma mark Physics Methods
/**
 * Create new fixtures for this body, defining the shape
 *
 * This is the primary method to override for custom physics objects
 */
void PlayerModel::createFixtures() {
    if (_body == nullptr) {
        return;
    }

    BoxObstacle::createFixtures();

    // give name to initial body fixture for collision handling
    b2Fixture* bodyFix = _body->GetFixtureList();
    if (bodyFix) {
        bodyFix->GetUserData().pointer = reinterpret_cast<uintptr_t>(getBodyName());
    }

    b2FixtureDef sensorDef;
    sensorDef.density = _playerJSON->getFloat("density");
    sensorDef.isSensor = true;

    // Sensor dimensions
    float sensorHShrink = _playerJSON->get("fixtures")->get("sensor")->getFloat("h_shrink");
    float sensorHeight = _playerJSON->get("fixtures")->get("sensor")->getFloat("height");

    b2Vec2 corners[4];
    corners[0].x = -sensorHShrink * getWidth() / 2.0f;
    corners[0].y = (-getHeight() + sensorHeight) / 2.0f;
    corners[1].x = -sensorHShrink * getWidth() / 2.0f;
    corners[1].y = (-getHeight() - sensorHeight) / 2.0f;
    corners[2].x = sensorHShrink * getWidth() / 2.0f;
    corners[2].y = (-getHeight() - sensorHeight) / 2.0f;
    corners[3].x = sensorHShrink * getWidth() / 2.0f;
    corners[3].y = (-getHeight() + sensorHeight) / 2.0f;

    b2PolygonShape sensorShape;
    sensorShape.Set(corners, 4);

    sensorDef.shape = &sensorShape;
    sensorDef.userData.pointer = reinterpret_cast<uintptr_t>(getGroundSensorName());
    _groundSensorFixture = _body->CreateFixture(&sensorDef);
}

/**
 * Release the fixtures for this body, reseting the shape
 *
 * This is the primary method to override for custom physics objects.
 */
void PlayerModel::releaseFixtures() {
    if (_body != nullptr) {
        return;
    }

    BoxObstacle::releaseFixtures();
    if (_groundSensorFixture != nullptr) {
        _body->DestroyFixture(_groundSensorFixture);
        _groundSensorFixture = nullptr;
    }
}

/**
 * Disposes all resources and assets of this PlayerModel
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a PlayerModel may not be used until it is initialized again.
 */
void PlayerModel::dispose() {
    _geometry = nullptr;
    _sceneNode = nullptr;
    _groundSensorNode = nullptr;
    _idleSprite = nullptr;
    _guardSprite = nullptr;
    _walkSprite = nullptr;
    _jumpUpSprite = nullptr;
    _jumpDownSprite = nullptr;
    _attackSprite = nullptr;
}

#pragma mark Cooldowns
/**
 * Updates the object's physics state (NOT GAME LOGIC).
 *
 * We use this method to reset cooldowns.
 *
 * @param delta Number of seconds since last animation frame
 */
void PlayerModel::update(float dt) {

    if (_isGuardInput)
    {
        CULog("PLAYER GUARD INPUT: %s", _isGuardInput ? "TRUE" : "FALSE");
        CULog("PLAYER GUARD CD REMAINING: %i", _guardCooldownRem);
        CULog("");
    }

    BoxObstacle::update(dt);
    if (_sceneNode != nullptr) {
        _sceneNode->setPosition(getPosition() * _drawScale);
        _sceneNode->setAngle(getAngle());
    }

    if (_lastDamagedFrame < PLAYER_HIT_COLOR_DURATION) {
        _lastDamagedFrame++;
    }

    if (_lastDamagedFrame == PLAYER_HIT_COLOR_DURATION) {
        _sceneNode->setColor(Color4::WHITE);
    }
}

#pragma mark -
#pragma mark Animation
void PlayerModel::playAnimation(std::shared_ptr<scene2::SpriteNode> sprite) {
    if (sprite->isVisible()) {
        frameCounter = (frameCounter + 1) % _animation_update_frame;
        if (frameCounter % _animation_update_frame == 0) {
            sprite->setFrame((sprite->getFrame() + 1) % sprite->getCount());
        }
    }
    else {
        sprite->setFrame(0);
    }
}

void PlayerModel::playAnimationOnce(std::shared_ptr<scene2::SpriteNode> sprite) {
    if (sprite->isVisible()) {
        frameCounter = (frameCounter + 1) % _animation_update_frame;
        if (frameCounter % _animation_update_frame == 0 && sprite->getFrame() < sprite->getCount() - 1) {
            sprite->setFrame(sprite->getFrame() + 1);
        }
    }
    else {
        sprite->setFrame(0);
    }
}


void PlayerModel::updateAnimation()
{
    if (isDamaged()) {
        _guardSprite->setVisible(false);
		_guardReleaseSprite->setVisible(false);
		_parryReleaseSprite->setVisible(false);
        _attackSprite->setVisible(false);
        _jumpUpSprite->setVisible(false);
        _jumpDownSprite->setVisible(false);
        _walkSprite->setVisible(false);
        _idleSprite->setVisible(false);
        _damagedSprite->setVisible(true);

        if (_damageRem == PLAYER_HIT_COLOR_DURATION) {
            frameCounter = 0;
            _damagedSprite->setFrame(0);
        }

        playAnimationOnce(_damagedSprite);
    }
    else if (isGuardActive()) {
        _guardSprite->setVisible(true);
        _guardReleaseSprite->setVisible(false);
        _parryReleaseSprite->setVisible(false);
        _attackSprite->setVisible(false);
        _jumpUpSprite->setVisible(false);
        _jumpDownSprite->setVisible(false);
        _walkSprite->setVisible(false);
        _idleSprite->setVisible(false);
        _damagedSprite->setVisible(false);

        if (isGuardBegin()) {
            frameCounter = 0;
            _guardSprite->setFrame(0);
        }

        playAnimationOnce(_guardSprite);
    }

    else if (getGuardReleaseRem() > 0) {
        _guardSprite->setVisible(false);
        _guardReleaseSprite->setVisible(false);
        _parryReleaseSprite->setVisible(false);
        _attackSprite->setVisible(false);
        _jumpUpSprite->setVisible(false);
        _jumpDownSprite->setVisible(false);
        _walkSprite->setVisible(false);
        _idleSprite->setVisible(false);
        _damagedSprite->setVisible(false);

        if (getGuardReleaseRem() == PLAYER_HIT_COLOR_DURATION) {
            frameCounter = 0;
            _guardReleaseSprite->setFrame(0);
            _parryReleaseSprite->setFrame(0);
        }

        if (_guardState == 2) {
            _parryReleaseSprite->setVisible(true);
            playAnimationOnce(_parryReleaseSprite);
        }
        else if (_guardState == 3) {
            _guardReleaseSprite->setVisible(true);
            playAnimationOnce(_guardReleaseSprite);
        }
    }

    else if (isDashActive()) {
        _guardSprite->setVisible(false);
        _guardReleaseSprite->setVisible(false);
        _parryReleaseSprite->setVisible(false);
        _attackSprite->setVisible(true);
        _jumpUpSprite->setVisible(false);
        _jumpDownSprite->setVisible(false);
        _walkSprite->setVisible(false);
        _idleSprite->setVisible(false);
        _damagedSprite->setVisible(false);

        if (isDashInput()) {
            frameCounter = 0;
            _attackSprite->setFrame(0);
        }

        playAnimationOnce(_attackSprite);
    }

    else if (isJumpBegin()) {
        _guardSprite->setVisible(false);
        _guardReleaseSprite->setVisible(false);
        _parryReleaseSprite->setVisible(false);
        _attackSprite->setVisible(false);
        _jumpUpSprite->setVisible(true);
        _jumpDownSprite->setVisible(false);
        _walkSprite->setVisible(false);
        _idleSprite->setVisible(false);
        _damagedSprite->setVisible(false);

        frameCounter = 0;
        _jumpUpSprite->setFrame(0);
    }
    else if (getVY() > 0 && !isGrounded()) {
        _guardSprite->setVisible(false);
        _guardReleaseSprite->setVisible(false);
        _parryReleaseSprite->setVisible(false);
        _attackSprite->setVisible(false);
        _jumpUpSprite->setVisible(true);
        _jumpDownSprite->setVisible(false);
        _walkSprite->setVisible(false);
        _idleSprite->setVisible(false);
        _damagedSprite->setVisible(false);

        playAnimationOnce(_jumpUpSprite);
    }
    else if (getVY() == 0 && !isGrounded()) {
        _guardSprite->setVisible(false);
        _guardReleaseSprite->setVisible(false);
        _parryReleaseSprite->setVisible(false);
        _attackSprite->setVisible(false);
        _jumpUpSprite->setVisible(false);
        _jumpDownSprite->setVisible(true);
        _walkSprite->setVisible(false);
        _idleSprite->setVisible(false);
        _damagedSprite->setVisible(false);

        frameCounter = 0;
        _jumpDownSprite->setFrame(0);
    }
    else if (getVY() < 0 && !isGrounded()) {
        _guardSprite->setVisible(false);
        _guardReleaseSprite->setVisible(false);
        _parryReleaseSprite->setVisible(false);
        _attackSprite->setVisible(false);
        _jumpUpSprite->setVisible(false);
        _jumpDownSprite->setVisible(true);
        _walkSprite->setVisible(false);
        _idleSprite->setVisible(false);
        _damagedSprite->setVisible(false);

        playAnimationOnce(_jumpDownSprite);
    }

    else if (isStrafeLeft() || isStrafeRight()) {
        _guardSprite->setVisible(false);
        _guardReleaseSprite->setVisible(false);
        _parryReleaseSprite->setVisible(false);
        _attackSprite->setVisible(false);
        _jumpUpSprite->setVisible(false);
        _jumpDownSprite->setVisible(false);
        _walkSprite->setVisible(true);
        _idleSprite->setVisible(false);
        _damagedSprite->setVisible(false);

        playAnimation(_walkSprite);
    }

    else {
        _guardSprite->setVisible(false);
        _guardReleaseSprite->setVisible(false);
        _parryReleaseSprite->setVisible(false);
        _attackSprite->setVisible(false);
        _jumpUpSprite->setVisible(false);
        _jumpDownSprite->setVisible(false);
        _walkSprite->setVisible(false);
        _idleSprite->setVisible(true);
        _damagedSprite->setVisible(false);

        playAnimation(_idleSprite);
    }

    _sceneNode->setScale(Vec2(isFacingRight() ? 1 : -1, 1));
    //_sceneNode->getChild(_sceneNode->getChildCount() - 1)->setScale(Vec2(isFacingRight() ? 1 : -1, 1));
}


#pragma mark -
#pragma mark Scene Graph Methods
/**
 * Redraws the outline of the physics fixtures to the debug node
 *
 * The debug node is use to outline the fixtures attached to this object.
 * This is very useful when the fixtures have a very different shape than
 * the texture (e.g. a circular shape attached to a square texture).
 */
void PlayerModel::resetDebug() {
    BoxObstacle::resetDebug();
    _debug->setName("player_debug");
    if (_groundSensorNode == nullptr){
        setDebug();
    }
    if (_debug->getChildCount() == 0){
        _debug->addChild(_groundSensorNode);
    }
    
    // necessary during reset, set debug scene node, since BoxObstacle::resetDebug() doesn't handle it correctly.
    if (_debug->getScene() == nullptr){
        _scene->addChild(_debug);
    }
}

void PlayerModel::setDebug(){
    // Sensor dimensions
    float sensor_shrink = _playerJSON->get("fixtures")->get("sensor")->getFloat("s_shrink");
    float sensor_height = _playerJSON->get("fixtures")->get("sensor")->getFloat("height");
    float w = sensor_shrink * _dimension.width;
    float h = sensor_height;
    Poly2 playerPoly(Rect(-w / 0.1f, -h / 2.0f, w, h));
    _groundSensorNode = scene2::WireNode::allocWithTraversal(playerPoly, poly2::Traversal::INTERIOR);
    _groundSensorNode->setColor(Color4(_playerJSON->get("debug")->getString("ground_sensor_color")));
    _groundSensorNode->setPosition(Vec2(_debug->getContentSize().width / 2.0f, 0.0f));
}

void PlayerModel::setConstants(){

    // Health
    _maxhp = _playerJSON->getInt("max_hp");
    // Animation
    _animation_update_frame = _playerJSON->get("animation")->getInt("update_frame");

    // Movement
    _strafe_force = _playerJSON->get("strafe")->getFloat("force");
    _damp_force = _playerJSON->get("movement")->getFloat("damping");
    _maxspeed = _playerJSON->get("movement")->getFloat("max_speed");

    // Jump
    _jump_cooldown = _playerJSON->get("jump")->getInt("cooldown");
    _jump_force = _playerJSON->get("jump")->getFloat("force");

    // Dash
    _dash_cooldown = _playerJSON->get("dash")->getInt("cooldown");
    _dash_duration = _playerJSON->get("dash")->getInt("duration");
    _dash_force = _playerJSON->get("dash")->getFloat("speed");

    // Knockback
    _knock_force = _playerJSON->get("knockback")->getFloat("force");
    _knock_duration = _playerJSON->get("knockback")->getInt("duration");

    // Combat
    _shoot_cooldown = _playerJSON->getInt("shoot_cooldown");
    _guard_cooldown = _playerJSON->get("guard")->getInt("cooldown");
    _guard_duration = _playerJSON->get("guard")->getInt("duration");
    _parry_duration = _playerJSON->get("parry")->getInt("duration");
    
    _name = _playerJSON->getString("name");
    _bodyName = _playerJSON->get("fixture_names")->getString("body");
    _groundSensorName = _playerJSON->get("fixture_names")->getString("ground");

}

void PlayerModel::reset(){
    CULog("resetting player model");
    if (getDebugScene()){
        CULog("In reset player model, debug scene name is %s", getDebugScene()->getName().c_str());
    }
    //setDebugScene(nullptr);
    _scene = nullptr; // set debug scene to nullptr
    resetAttributes();
}

#pragma mark static constants
int PlayerModel::_animation_update_frame = 4;
float PlayerModel::_maxspeed = 5.0f;
float PlayerModel::_maxhp = 100.0f;
int PlayerModel::_jump_cooldown = 5;
int PlayerModel::_shoot_cooldown = 20;
int PlayerModel::_guard_cooldown = 30;
int PlayerModel::_dash_cooldown = 45;

int PlayerModel::_guard_duration = 30;
int PlayerModel::_parry_duration = 5;
int PlayerModel::_dash_duration = 20;
int PlayerModel::_knock_duration = 20;

float PlayerModel::_strafe_force = 50.0f;
float PlayerModel::_jump_force = 45.0f;
float PlayerModel::_damp_force = 30.0f;
float PlayerModel::_dash_force = 30.0f;
float PlayerModel::_knock_force = 15.0f;


