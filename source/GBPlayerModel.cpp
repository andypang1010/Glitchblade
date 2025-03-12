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

#define SIGNUM(x)  ((x > 0) - (x < 0))

#pragma mark -
#pragma mark Physics Constants
/** Cooldown (in animation frames) for jumping */
#define JUMP_COOLDOWN   5
/** Cooldown (in animation frames) for shooting */
#define SHOOT_COOLDOWN  20
/** Cooldown (in frames) for guard */
#define GUARD_COOLDOWN  15
/** Cooldown (in frames) for dash */
#define DASH_COOLDOWN  30
/** Duration (in frames) for guard */
#define GUARD_DURATION  44
/** Duration (in frames) for parry */
#define PARRY_DURATION  24
/** Duration (in frames) for dash- affects friction*/
#define DASH_DURATION  20
/** The amount to shrink the body fixture (vertically) relative to the image */
#define DUDE_VSHRINK  0.95f
/** The amount to shrink the body fixture (horizontally) relative to the image */
#define DUDE_HSHRINK  0.7f
/** The amount to shrink the sensor fixture (horizontally) relative to the image */
#define DUDE_SSHRINK  0.6f
/** Height of the sensor attached to the player's feet */
#define SENSOR_HEIGHT   0.1f
/** The amount to shrink the radius of the shield relative to the image width */
#define SHIELD_RADIUS 2.0f
/** The density of the character */
#define DUDE_DENSITY    1.0f
/** The impulse for the character jump */
#define DUDE_JUMP       42.5f
/** The x SPEED for the character dash-attack */
#define DUDE_DASH       20.0f
/** The impulse for the  vertical component of the knockback */
#define DUDE_KB     15.0f
#define KB_DURATION 20
/** Debug color for the sensor */
#define DEBUG_COLOR     Color4::RED


using namespace cugl;

#pragma mark -
#pragma mark Constructors

/**
 * Initializes a new dude at the given position.
 *
 * The dude is sized according to the given drawing scale.
 *
 * The scene graph is completely decoupled from the physics system.
 * The node does not have to be the same size as the physics body. We
 * only guarantee that the scene graph node is positioned correctly
 * according to the drawing scale.
 *
 * @param pos   Initial position in world coordinates
 * @param size  The size of the dude in world units
 * @param scale The drawing scale (world to screen)
 *
 * @return  true if the obstacle is initialized properly, false otherwise.
 */
bool PlayerModel::init(const Vec2& pos, const Size& size, float scale) {
    Size nsize = size;
    nsize.width  *= DUDE_HSHRINK;
    nsize.height *= DUDE_VSHRINK;
    _drawScale = scale;
    
    if (BoxObstacle::init(pos,nsize)) {
        setDensity(DUDE_DENSITY);
        setFriction(0.0f);      // HE WILL STICK TO WALLS IF YOU FORGET
        setFixedRotation(true); // OTHERWISE, HE IS A WEEBLE WOBBLE
        
        // Gameplay attributes
        _hp = DUDE_MAXHP;
        _isGrounded = false;
        _isShootInput = false;
        _isJumpInput  = false;
        _isStrafeLeft = false;
        _isStrafeRight = false;
        _isDashLeftInput = false;
        _isDashRightInput = false;
        _isGuardInput = false;
        _hasProjectile = false;
        _faceRight  = true;
        _shootCooldownRem = 0;
        _jumpCooldownRem  = 0;
        _dashCooldownRem = 0;
        _guardCooldownRem = 0;
        _guardRem = 0;
        _parryRem= 0;

        _sceneNode = scene2::SceneNode::alloc();
        setSceneNode(_sceneNode);
        return true;
    }
    return false;
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
}

/**
 * Sets left/right movement of this character.
 *
 * This is the result of input times dude force.
 *
 * @param value left/right movement of this character.
 */
void PlayerModel::setMovement(float value) {
    _movement = value;
    bool face = _movement > 0;
    if (_movement == 0 || _faceRight == face) {
        // CULog("RETURNING!");
        return;
    }
    
    // Change facing
    scene2::TexturedNode* image = dynamic_cast<scene2::TexturedNode*>(_sceneNode.get());
    if (image != nullptr) {
        // Don't flip if it means overriding a dash direction
        if (!isDashActive()) {
            image->flipHorizontal(!image->isFlipHorizontal());
            CULog("DID FLIP!");
        }
        else {
            CULog("IGNORING FLIP!");
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
    CULog("Doing faceLeft");
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
    CULog("Doing faceRight");
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
    sensorDef.density = DUDE_DENSITY;
    sensorDef.isSensor = true;
    
    // Sensor dimensions
    b2Vec2 corners[4];
    corners[0].x = -DUDE_SSHRINK*getWidth()/2.0f;
    corners[0].y = (-getHeight()+SENSOR_HEIGHT)/2.0f;
    corners[1].x = -DUDE_SSHRINK*getWidth()/2.0f;
    corners[1].y = (-getHeight()-SENSOR_HEIGHT)/2.0f;
    corners[2].x =  DUDE_SSHRINK*getWidth()/2.0f;
    corners[2].y = (-getHeight()-SENSOR_HEIGHT)/2.0f;
    corners[3].x =  DUDE_SSHRINK*getWidth()/2.0f;
    corners[3].y = (-getHeight()+SENSOR_HEIGHT)/2.0f;
    
    b2PolygonShape sensorShape;
    sensorShape.Set(corners,4);
    
    sensorDef.shape = &sensorShape;
    sensorDef.userData.pointer = reinterpret_cast<uintptr_t>(getGroundSensorName());
    _sensorFixture = _body->CreateFixture(&sensorDef);
    
    // create shield circle fixture
    b2FixtureDef shieldDef;
    b2CircleShape shieldShape;
    shieldShape.m_radius = SHIELD_RADIUS;
    shieldShape.m_p.Set(0,0);//center of body
    shieldDef.isSensor = true;
    shieldDef.shape = &shieldShape;
    shieldDef.userData.pointer = reinterpret_cast<uintptr_t>(getShieldName());
    _shieldFixture = _body->CreateFixture(&shieldDef);
    
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
    if (_sensorFixture != nullptr) {
        _body->DestroyFixture(_sensorFixture);
        _sensorFixture = nullptr;
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
    _sensorNode = nullptr;
    _shieldNode = nullptr;
}

/**
 * Applies the force to the body of this dude
 *
 * This method should be called after the force attribute is set.
 */
void PlayerModel::applyForce() {
    if (!isEnabled()) {
        return;
    }
    
    // Don't want to be moving.f Damp out player motion
    if (getMovement() == 0.0f && !isDashActive() && !isKnockbackActive()) {
        if (isGrounded()) {
            // CULog("Setting x vel to 0");
            // Instant friction on the ground
            b2Vec2 vel = _body->GetLinearVelocity();
            vel.x = 0; // If you set y, you will stop a jump in place
            _body->SetLinearVelocity(vel);
        } else {
            //             Damping factor in the air
            b2Vec2 force(-getDamping()*getVX(),0);
            _body->ApplyForceToCenter(force,true);
        }
    }
#pragma mark strafe force
//    b2Vec2 force(getMovement(),0);
    // Ignore stafe input if in a dash (intentional)
    if (!(_dashRem > 0) && !isKnockbackActive()) {
        _body->SetLinearVelocity(b2Vec2(getMovement(), _body->GetLinearVelocity().y));
    }
    // _body->ApplyForceToCenter(force,true); // Old method of movement (slipper)
#pragma mark jump force
    // Jump!
    if (isJumpBegin() && isGrounded()) {
        b2Vec2 force(0, DUDE_JUMP);
        _body->ApplyLinearImpulseToCenter(force,true);
    }
#pragma mark dash force
    // Dash!
    if (isDashLeftBegin() && _dashReset == true){
        CULog("dashing left begin");
        faceLeft();
        // b2Vec2 force(-DUDE_DASH,0);
        // _body->ApplyLinearImpulseToCenter(force, true); // Old method of dashing
        _body->SetLinearVelocity(b2Vec2(-DUDE_DASH, _body->GetLinearVelocity().y));
        _dashReset = false;
    }
    
    if (isDashRightBegin() && _dashReset == true){
        CULog("dashing right begin");
        faceRight();
        // b2Vec2 force(DUDE_DASH, 0);
        // _body->ApplyLinearImpulseToCenter(force, true);
        _body->SetLinearVelocity(b2Vec2(DUDE_DASH, _body->GetLinearVelocity().y));
        _dashReset = false;
    }
#pragma mark knockback force
    if (isKnocked()) {
        _body->SetLinearVelocity(b2Vec2(0,0));

        Vec2 knockForce = _knockDirection.subtract(Vec2(0,_knockDirection.y)).scale(DUDE_KB);
        _body->ApplyLinearImpulseToCenter(b2Vec2(knockForce.x, DUDE_KB), true);
        _isKnocked = false;
    }
    
    // Velocity too high, clamp it
    if (fabs(getVX()) >= getMaxSpeed() && !isDashActive() && !isKnockbackActive()) {
        //CULog("clamping velocity");
        setVX(SIGNUM(getVX())*getMaxSpeed());
        
    }
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

    updateAnimation();

    // Apply cooldowns
    #pragma mark Guard and Parry timing
    // guard cooldown first for most recent movement inputs
    if (isGuardActive() && !isGuardBegin()){
        //just for logging end of parry
        bool parry_active = isParryActive();
        _guardRem = (_guardRem > 0 ? _guardRem -1 : 0);
        _parryRem= (_parryRem > 0 ? _parryRem -1 : 0);
            
        //The rest of this block is for logging end of guard&parry
        if (parry_active && (_parryRem == 0)){
            _shieldNode->setColor(Color4::BLUE);
            CULog("Parry completed during guard\n");
        }
        if (_guardRem == 0){
            CULog("Guard completed full duration\n");
            _shieldNode->setColor(DEBUG_COLOR);
        }
    }
    // guard not active, update cooldown
    else {
        _guardCooldownRem = (_guardCooldownRem > 0 ? _guardCooldownRem -1 : 0);
    }
    if (isJumpBegin() && isGrounded()) {
        CULog("isJumping is true");
        _jumpCooldownRem = JUMP_COOLDOWN;
    } else {
        _jumpCooldownRem = (_jumpCooldownRem > 0 ? _jumpCooldownRem-1 : 0);
    }
    
    if (isShooting()) {
        _shootCooldownRem = SHOOT_COOLDOWN;
    } else {
        _shootCooldownRem = (_shootCooldownRem > 0 ? _shootCooldownRem-1 : 0);
    }
    
    if (isKnocked()) {
        CULog("knockback applied");
        _dashCooldownRem = DASH_COOLDOWN;
        _guardCooldownRem = GUARD_COOLDOWN;
        _jumpCooldownRem = JUMP_COOLDOWN;
        _shootCooldownRem = SHOOT_COOLDOWN;
        _knockbackRem = KB_DURATION;
    } else {
        _knockbackRem = (_knockbackRem > 0 ? _knockbackRem-1 : 0);
    }
    
    if (isDashBegin()) {
        _dashRem = DASH_DURATION;
        _dashCooldownRem = DASH_COOLDOWN;
    }
    else {
        _dashRem = (_dashRem > 0 ? _dashRem-1 : 0);
        if (_dashRem == 0){
            _dashCooldownRem = (_dashCooldownRem > 0 ? _dashCooldownRem-1 : 0);
        }
    }

    // Reset the dash if ready (requires user to stop holding dash key(s) for at least one frame)
    if (_dashReset == false && _dashCooldownRem == 0 && !(_isDashLeftInput || _isDashRightInput)) {
        _dashReset = true; // ready to dash again
    }

    // player inputs guard and cooldown is ready
    if (isGuardBegin()) {
        CULog("Beginning guard\n");
        _shieldNode->setColor(Color4::GREEN);
        _guardCooldownRem = GUARD_COOLDOWN;
        _guardRem = GUARD_DURATION;
        // begin parry
        CULog("Beginning parry\n");
        _parryRem = PARRY_DURATION;
    }
    
    BoxObstacle::update(dt);
    if (_sceneNode != nullptr) {
        _sceneNode->setPosition(getPosition()*_drawScale);
        _sceneNode->setAngle(getAngle());
    }
}

#pragma mark -
#pragma mark Animation
void PlayerModel::playAnimation(std::shared_ptr<scene2::SpriteNode> sprite) {
    if (sprite->isVisible()) {
        currentFrame++;

        if (currentFrame % ANIMATION_UPDATE_FRAME == 0) {
            sprite->setFrame((sprite->getFrame() + 1) % sprite->getCount());
        }
    }
}

void PlayerModel::updateAnimation()
{
    _walkSprite->setVisible(!isGuardActive() && isGrounded() && !isDashActive() && (isStrafeLeft() || isStrafeRight()));
    _jumpUpSprite->setVisible(!isGuardActive() && !isGrounded() && getBody()->GetLinearVelocity().y > 0);
    _jumpDownSprite->setVisible(!isGuardActive() && !isGrounded() && getBody()->GetLinearVelocity().y < 0);
    _guardSprite->setVisible(isGuardActive());
    _attackSprite->setVisible(!isGuardActive() && isDashActive());

    _idleSprite->setVisible(
        !_walkSprite->isVisible() &&
        !_jumpUpSprite->isVisible() &&
        !_jumpDownSprite->isVisible() &&
        !_guardSprite->isVisible() &&
        !_attackSprite->isVisible());

    if (isDashBegin()) {
        currentFrame = 0;
        _attackSprite->setFrame(0);
    }

    if (isJumpBegin()) {
        currentFrame = 0;
        _jumpUpSprite->setFrame(0);
        _jumpDownSprite->setFrame(0);
    }

    if (isParryActive()) {
        currentFrame = 0;
        _guardSprite->setFrame(0);
    }

    else {
        playAnimation(_guardSprite);
    }

    playAnimation(_attackSprite);
    playAnimation(_walkSprite);
    playAnimation(_idleSprite);

    _sceneNode->setScale(Vec2(isFacingRight() ? 1 : -1, 1));
    _sceneNode->getChild(_sceneNode->getChildCount() - 1)->setScale(Vec2(isFacingRight() ? 1 : -1, 1));
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
    float w = DUDE_SSHRINK*_dimension.width;
    float h = SENSOR_HEIGHT;
    Poly2 dudePoly(Rect(-w/0.1f,-h/2.0f,w,h));
    _sensorNode = scene2::WireNode::allocWithTraversal(dudePoly, poly2::Traversal::INTERIOR);
    _sensorNode->setColor(DEBUG_COLOR);
    _sensorNode->setPosition(Vec2(_debug->getContentSize().width/2.0f, 0.0f));
    
    Poly2 shieldPoly;
    shieldPoly = PolyFactory().makeNgon(10000,0, SHIELD_RADIUS, 20);
    _shieldNode = scene2::WireNode::allocWithPoly(shieldPoly);
    _shieldNode->setPosition(Vec2(_debug->getContentSize()/2));
    _shieldNode->setColor(DEBUG_COLOR);

    _debug->addChild(_sensorNode);
    _debug->addChild(_shieldNode);
    
}




