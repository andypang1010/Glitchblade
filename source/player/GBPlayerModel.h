//
//  GBPlayerModel.h
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
//  Author:  Walker White and Anthony Perello
//  Version: 2/9/21
//
#ifndef __GB_PLAYER_MODEL_H__
#define __GB_PLAYER_MODEL_H__
#include <cugl/cugl.h>

#define PLAYER_HIT_COLOR_DURATION 16

using namespace cugl;

#pragma mark -
#pragma mark player Model
/**
* Player avatar for the plaform game.
*
* Note that this class uses a capsule shape, not a rectangular shape.  In our
* experience, using a rectangular shape for a character will regularly snag
* on a platform.  The round shapes on the end caps lead to smoother movement.
*/
class PlayerModel : public physics2::BoxObstacle {
private:
    /** This macro disables the copy constructor (not allowed on physics objects) */
    CU_DISALLOW_COPY_AND_ASSIGN(PlayerModel);
protected:
    /**JsonValue storing all necessary player constants for fixtures and physics*/
    std::shared_ptr<JsonValue> _playerJSON;
    /** This character's remaining health */
    float _hp;
    /** how much damage player deals*/
    int _damage;
    /** The current horizontal movement of the character */
    float _movement;
    /** Which direction is the character facing */
    bool _faceRight;
    /** How long until we can jump again in animation frames */
    int  _jumpCooldownRem;
    /** How long until we can dash again in frames*/
    int  _dashCooldownRem;
    /** How many frames remaining in the dash animation (affects friciton)*/
    int  _dashRem;
    /** How long until we can guard again in frames */
    int  _guardCooldownRem = 0;
    /** How many frames remaining in the guard release animation */
    int  _guardReleaseRem = 0;
    /** How many frames remaining in the guard (0 when guard is not active) */
    int  _guardRem;
    /** The state of the guard: 1 = is guarding, 2 = parry release, 3 = normal release, 0 = not guarding */
    int  _guardState;
    /** How many frames remaining in the parry (0 when parry is not active) */
    int  _parryRem;
    /** Whether we are actively inputting jumping */
    bool _isJumpInput;
    /** Whether we are actively inputting strafe left*/
    bool _isStrafeLeft;
    /** Whether we are actively inputting strafe right*/
    bool _isStrafeRight;
    /** Whether we are actively inputting dash left*/
    bool _isDashLeftInput;
    /** Whether we are actively inputting dash right*/
    bool _isDashRightInput;
    /** Whether we are actively inputting the guard*/
    bool _isGuardInput;
    /** Whether we have a (swallowed) projectile*/
    bool _hasProjectile;
    /** Whether we are actively inputting shoot */
    bool _isShootInput;
    /** Whether we are knocked-back (sets input cd) */
    bool _isKnocked;
    /** Whether we are knocked-back (sets input cd) */
    int _knockRem;
    Vec2 _knockDirection;
    /** How long until we can shoot again in animation frames*/
    int  _shootCooldownRem;
    /** Whether our feet are on the ground */
    bool _isGrounded;
    /** Whether the dash has been released (reset) */
    bool _dashReset = true;
    int _lastDamagedFrame;
    /** Remaining time for damaged animation */
    int _damageRem;
    
#pragma mark fixture constants
    /** The amount to shrink the body fixture (horizontally) relative to the image */
    float _hShrink = 0.7f;
    /** Height of the sensor attached to the Enemy's feet */
    float _sensorHeight = 0.1f;
    Size _size;


    std::string _name;
    std::string _bodyName;
    /** Ground sensor to represent our feet */
    b2Fixture* _groundSensorFixture;
    /** Reference to the sensor name (since a constant cannot have a pointer) */
    std::string _groundSensorName;
    /** The node for debugging the ground sensor */
    std::shared_ptr<scene2::WireNode> _groundSensorNode;
    /** The player scene node**/
    std::shared_ptr<scene2::SceneNode> _sceneNode;
    /** The scale between the physics world and the screen (MUST BE UNIFORM) */
    float _drawScale;
    int frameCounter = 0;

    /**
    * Redraws the outline of the physics fixtures to the debug node
    *
    * The debug node is use to outline the fixtures attached to this object.
    * This is very useful when the fixtures have a very different shape than
    * the texture (e.g. a circular shape attached to a square texture).
    */
    virtual void resetDebug() override;

public:
    int iframe = 0;
    int _parryCounter = 0;
    float _comboMeter = 0;
    float _lastComboElapsedTime = 0;

    std::shared_ptr<scene2::SpriteNode> _idleSprite;
    std::shared_ptr<scene2::SpriteNode> _walkSprite;
    std::shared_ptr<scene2::SpriteNode> _jumpUpSprite;
    std::shared_ptr<scene2::SpriteNode> _jumpDownSprite;
    std::shared_ptr<scene2::SpriteNode> _attackSprite;
    std::shared_ptr<scene2::SpriteNode> _damagedSprite;

    std::shared_ptr<scene2::SpriteNode> _guardSprite;
    std::shared_ptr<scene2::SpriteNode> _guardReleaseSprite;
    std::shared_ptr<scene2::SpriteNode> _parryReleaseSprite;

#pragma mark Hidden Constructors
    /**
     * Creates a degenerate player object.
     *
     * This constructor does not initialize any of the player values beyond
     * the defaults.  To use a PlayerModel, you must call init().
     */
    PlayerModel() : BoxObstacle() {}

    /**
     * Destroys this PlayerModel, releasing all resources.
     */
    virtual ~PlayerModel(void) { dispose(); }

    /**
     * Disposes all resources and assets of this PlayerModel
     *
     * Any assets owned by this object will be immediately released.  Once
     * disposed, a PlayerModel may not be used until it is initialized again.
     */
    void dispose();
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
     * @param constantsJSON reference to JsonValue with constants
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    virtual bool init(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& constantsRef, const Vec2& pos);


#pragma mark -
#pragma mark Static Constructors
    /**
     * Creates a new player that is uninitialized.
     * @return  A newly allocated, unitialized PlayerModel
     */
    static std::shared_ptr<PlayerModel> alloc() {
        std::shared_ptr<PlayerModel> result = std::make_shared<PlayerModel>();
        return (result ? result : nullptr);
    }

    /**
     * Creates a new player at the given position.
     *
     * The player is sized according to the given drawing scale.
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param pos   Initial position in world coordinates
     * @param constantsJSON reference to JsonValue with constants
     *
     * @return  A newly allocated PlayerModel at the given position with the given scale
     */
    static std::shared_ptr<PlayerModel> alloc(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& constantsRef, const Vec2& pos) {
        std::shared_ptr<PlayerModel> result = std::make_shared<PlayerModel>();
        return (result->init(assetRef, constantsRef, pos) ? result : nullptr);
    }

#pragma mark -
#pragma mark Level Control and Constructor Helpers
    /** Reset all the player attributes to their initial values*/
    void resetAttributes() {
        _hp = _maxhp;
        _isGrounded = false;
        _isShootInput = false;
        _isJumpInput = false;
        _isStrafeLeft = false;
        _isStrafeRight = false;
        _isDashLeftInput = false;
        _isDashRightInput = false;
        _isGuardInput = false;
        _hasProjectile = false;
        _faceRight = true;
        _dashReset = true; //must init to true to be able to dash?
        _shootCooldownRem = 0;
        _jumpCooldownRem = 0;
        _dashCooldownRem = 0;
        _dashRem = 0;
        _guardCooldownRem = 0;
        _guardRem = 0;
        _guardState = 0;
        _parryRem = 0;
        _damageRem = 0;
        _damage = 10;// default player dmg
        
        _parryCounter = 0;
		_comboMeter = 0;
        _lastComboElapsedTime = 0;
    };
    
    void setConstants();
    void setDebug();
    void debugHelper();
    void reset();

    /**Attach the scene nodes (sprite sheets) to the player**/
    void attachNodes(const std::shared_ptr<AssetManager>& assetRef);

#pragma mark -
#pragma mark Animation
    /**
     * Returns the scene graph node representing this PlayerModel.
     *
     * By storing a reference to the scene graph node, the model can update
     * the node to be in sync with the physics info. It does this via the
     * {@link Obstacle#update(float)} method.
     *
     * @return the scene graph node representing this PlayerModel.
     */
    const std::shared_ptr<scene2::SceneNode>& getSceneNode() const { return _sceneNode; }

    /**
     * Sets the scene graph node representing this PlayerModel.
     *
     * Note that this method also handles creating the nodes for the body parts
     * of this PlayerModel. Since the obstacles are decoupled from the scene graph,
     * initialization (which creates the obstacles) occurs prior to the call to
     * this method. Therefore, to be drawn to the screen, the nodes of the attached
     * bodies must be added here.
     *
     * The bubbler also uses the world node when adding bubbles to the scene, so
     * the input node must be added to the world BEFORE this method is called.
     *
     * By storing a reference to the scene graph node, the model can update
     * the node to be in sync with the physics info. It does this via the
     * {@link Obstacle#update(float)} method.
     *
     * @param node  The scene graph node representing this PlayerModel, which has been added to the world node already.
     */
    void setSceneNode(const std::shared_ptr<scene2::SceneNode>& node) {
        _sceneNode = node;
        _sceneNode->setPosition(getPosition() * _drawScale);
    }



// Miscellaneous
Vec2 getKnockDirection() { return _knockDirection; }
    std::string* getBodyName() { return &_bodyName; }
    /**
     * Returns the name of the ground sensor
     *
     * This is used by ContactListener
     *
     * @return the name of the ground sensor
     */
    std::string* getGroundSensorName() { return &_groundSensorName; }

    /**
     * Returns true if this character is facing right
     *
     * @return true if this character is facing right
     */
    bool isFacingRight() const { return _faceRight; }

#pragma mark -
#pragma mark Animation Methods
    void playAnimation(std::shared_ptr<scene2::SpriteNode> sprite);
    void playAnimationOnce(std::shared_ptr<scene2::SpriteNode> sprite);
    void updateAnimation();

#pragma mark -
#pragma mark Physics Methods
    /**
     * Creates the physics Body(s) for this object, adding them to the world.
     *
     * This method overrides the base method to keep your ship from spinning.
     *
     * @param world Box2D world to store body
     *
     * @return true if object allocation succeeded
     */
    void createFixtures() override;

    /**
     * Release the fixtures for this body, reseting the shape
     *
     * This is the primary method to override for custom physics objects.
     */
    void releaseFixtures() override;

    /**
     * Updates the object's physics state (NOT GAME LOGIC).
     *
     * We use this method to reset cooldowns.
     *
     * @param delta Number of seconds since last animation frame
     */
    void update(float dt) override;


#pragma mark constants
private:
    static int _animation_update_frame;
    static float _maxspeed;
    static float _maxhp;
    static int _jump_cooldown;
    static int _shoot_cooldown;
    static int _guard_cooldown;
    static int _dash_cooldown;
    
    static int _guard_duration;
    static int _parry_duration;
    static int _dash_duration;
    static int _knock_duration;
    
    static float _strafe_force;
    static float _jump_force;
    static float _damp_force;
    static float _dash_force;
    static float _knock_force;

public:
    #pragma mark -
    #pragma mark - Attribute Properties

    // Damage
    int getDamage() const { return _damage;}
    void setDamage(int value) { _damage = value; }
	bool isDamaged() const { return _damageRem > 0; }
	void setDamagedRem(int value) { _damageRem = value; }
	int getDamagedRem() { return _damageRem; }
    // Health
    int getMaxHP() const { return _maxhp;}
    float getHP() const { return _hp; }
    void setHP(float value) { _hp = value; }
    void damage(float value);

    // Movement
    float getMovement() const { return _movement; }
    void setMovement(float value);

    // Direction
    void faceLeft();
    void faceRight();

    // Shooting
    bool isShooting() const { return _isShootInput && _shootCooldownRem <= 0; }
    void setShootInput(bool value) { _isShootInput = value; }
    int getShootCDRem() { return _shootCooldownRem; }
    void setShootCDRem(int value = _shoot_cooldown) { _shootCooldownRem = _shoot_cooldown; }

    // Strafing
    void setStrafeLeft(bool value) { _isStrafeLeft = value; }
    void setStrafeRight(bool value) { _isStrafeRight = value; }
    bool isStrafeLeft() { return _isStrafeLeft; }
    bool isStrafeRight() { return _isStrafeRight; }

    // Jumping
    bool isJumpBegin() const { return _isJumpInput && _jumpCooldownRem <= 0; }
    int getJumpCDRem() { return _jumpCooldownRem; }
    void setJumpCDRem(int value = _jump_cooldown) { _jumpCooldownRem = value; }
    void setJumpInput(bool value) { _isJumpInput = value; }

    // Dashing
    bool isDashLeftBegin() { return _isDashLeftInput && _dashCooldownRem <= 0 && _dashReset; }
    bool isDashRightBegin() { return _isDashRightInput && _dashCooldownRem <= 0 && _dashReset; }
    bool isDashBegin() { return isDashLeftBegin() || isDashRightBegin(); }
    bool isDashActive() { return _dashRem > 0 || isDashBegin(); }
    int getDashRem() { return _dashRem; }
    void setDashRem(int value = _dash_duration) { _dashRem = value; }
    int getDashCDRem() { return _dashCooldownRem; }
    void setDashCDRem(int value = _dash_cooldown) { _dashCooldownRem = value; }
    bool isDashLeftInput() const { return _isDashLeftInput; }
    bool isDashRightInput() const { return _isDashRightInput; }
    bool isDashInput() const { return isDashRightInput() || isDashLeftInput(); }
    void setDashInput(bool value) { _isDashLeftInput = value; }
        
    // Dash
    bool getDashReset() const { return _dashReset; }
    void setDashReset(bool r) { _dashReset = r; }
    void setDashRightInput(bool value) { _isDashRightInput = value; }
    void setDashLeftInput(bool value) { _isDashLeftInput = value; }

    // Guarding
    bool isGuardBegin() { return _isGuardInput && _guardCooldownRem == 0; }
    bool isGuardActive() { return _guardRem > 0 || isGuardBegin(); }
    int getGuardRem() { return _guardRem; }
    void setGuardRem(int value = _guard_duration) { _guardRem = value; }
    int getGuardCDRem() { return _guardCooldownRem; }
    void setGuardCDRem(int value = _guard_cooldown) { _guardCooldownRem = value; }
    void setGuardInput(bool value) { _isGuardInput = value; }

	int getGuardState() { return _guardState; }
	void setGuardState(int value) { _guardState = value; }
    int getGuardReleaseRem() { return _guardReleaseRem; }
	void setGuardReleaseRem(int value) { _guardReleaseRem = value; }

    // Parrying
    bool isParryActive() { return _parryRem > 0 || isGuardBegin(); }
    int getParryRem() { return _parryRem; }
    void setParryRem(int value = _parry_duration) { _parryRem = value; }

    // Knockback
    int getKnockbackRem() { return _knockRem; }
    void setKnockbackRem(int value = _knock_duration) { _knockRem = value; }
    bool isKnocked() const { return _isKnocked; }
    bool isKnockbackActive() { return _knockRem > 0 || isKnocked(); }
    void setKnocked(bool value, Vec2 knockDirection) { _isKnocked = value; _knockDirection = knockDirection; }
    void resetKnocked() { _isKnocked = false; }

    // Projectile
    bool hasProjectile() { return _hasProjectile; }
    void setHasProjectile(bool value) { _hasProjectile = value; }

    // Grounded
    bool isGrounded() const { return _isGrounded; }
    void setGrounded(bool value) { _isGrounded = value; }

    // Forces
    float getStrafeF() const { return _strafe_force; }
    float getJumpF() const { return _jump_force; }
    float getDashF() const { return _dash_force; }
    float getKnockF() { return _knock_force; }
    float getDampF() const { return _damp_force; }

    // Speed
    float getMaxSpeed() const { return _maxspeed; }

    // Debug



    
};

#endif /* __GB_MODEL_H__ */
