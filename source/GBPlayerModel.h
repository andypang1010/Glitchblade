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

using namespace cugl;

#pragma mark -
#pragma mark Drawing Constants
/** The texture for the character avatar */
#define TEXTURE    "player"
#define ENEMY_TEXTURE   "enemy"
/** Identifier to allow us to track the player sensor in ContactListener */
#define BODY_NAME      "body"
#define SENSOR_NAME     "playersensor"
#define SHIELD_SENSOR_NAME      "shield"

#pragma mark -
#pragma mark Physics Constants
/** The factor to multiply by the input */
#define FORCE      50.0f
/** The amount to slow the character down */
#define DAMPING    30.0f
/** The maximum character speed */
#define MAXSPEED   10.0f
/** The maximum character hp */
#define MAXHP   100.0f
/** Cooldown (in animation frames) for jumping */
#define JUMP_COOLDOWN   5
/** Cooldown (in animation frames) for shooting */
#define SHOOT_COOLDOWN  20
/** Cooldown (in frames) for guard */
#define GUARD_COOLDOWN  15
/** Cooldown (in frames) for dash */
#define DASH_COOLDOWN  30
/** Duration (in frames) for guard */
#define GUARD_DURATION  60
/** Duration (in frames) for parry */
#define PARRY_DURATION  24
/** Duration (in frames) for dash- affects friction*/
#define DASH_DURATION  5
/** The amount to shrink the body fixture (vertically) relative to the image */
#define VSHRINK  0.95f
/** The amount to shrink the body fixture (horizontally) relative to the image */
#define HSHRINK  0.7f
/** The amount to shrink the sensor fixture (horizontally) relative to the image */
#define SSHRINK  0.6f
/** Height of the sensor attached to the player's feet */
#define SENSOR_HEIGHT   0.1f
/** The amount to shrink the radius of the shield relative to the image width */
#define SHIELD_RADIUS 2.0f
/** The density of the character */
#define DENSITY    1.0f
/** The impulse for the character jump */
#define JUMP       42.5f
/** The x SPEED for the character dash-attack */
#define DASH       75.0f
/** The impulse for the  vertical component of the knockback */
#define KB     15.0f
#define KB_DURATION 20
/** Debug color for the sensor */
#define SENSOR_DEBUG_COLOR     Color4::RED
#define DEBUG_COLOR     Color4::YELLOW


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
    /** This character's remaining health */
    float _hp;
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
    int  _guardCooldownRem;
    /** How many frames remaining in the guard (0 when guard is not active) */
    int  _guardRem;
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
    int _knockbackRem;
    Vec2 _knockDirection;
	/** How long until we can shoot again in animation frames*/
	int  _shootCooldownRem;
	/** Whether our feet are on the ground */
	bool _isGrounded;
    /** Whether the dash has been released (reset) */
    bool _dashReset = true;

    std::string _bodyName;
	/** Ground sensor to represent our feet */
	b2Fixture*  _sensorFixture;
	/** Reference to the sensor name (since a constant cannot have a pointer) */
	std::string _sensorName;
	/** The node for debugging the ground sensor */
	std::shared_ptr<scene2::WireNode> _sensorNode;
    /** Ground sensor to represent our feet */
    b2Fixture*  _shieldFixture;
    /** Reference to the sensor name (since a constant cannot have a pointer) */
    std::string _shieldName;
    /** The node for debugging the ground sensor */
    std::shared_ptr<scene2::WireNode> _shieldNode;
    /** The guard shield when guard is active */
    
	/** The scene graph node for the player. */
	std::shared_ptr<scene2::SceneNode> _node;
	/** The scale between the physics world and the screen (MUST BE UNIFORM) */
	float _drawScale;

	/**
	* Redraws the outline of the physics fixtures to the debug node
	*
	* The debug node is use to outline the fixtures attached to this object.
	* This is very useful when the fixtures have a very different shape than
	* the texture (e.g. a circular shape attached to a square texture).
	*/
	virtual void resetDebug() override;

public:
    
#pragma mark Hidden Constructors
    /**
     * Creates a degenerate player object.
     *
     * This constructor does not initialize any of the player values beyond
     * the defaults.  To use a PlayerModel, you must call init().
     */
    PlayerModel() : BoxObstacle(), _sensorName(SENSOR_NAME), _shieldName(SHIELD_SENSOR_NAME), _bodyName(BODY_NAME) { }
    
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
     * Initializes a new player at the origin.
     *
     * The player is a unit square scaled so that 1 pixel = 1 Box2d unit
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    virtual bool init() override { return init(Vec2::ZERO, Size(1,1), 1.0f); }
    
    /**
     * Initializes a new player at the given position.
     *
     * The player is unit square scaled so that 1 pixel = 1 Box2d unit
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param pos   Initial position in world coordinates
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    virtual bool init(const Vec2 pos) override { return init(pos, Size(1,1), 1.0f); }
    
    /**
     * Initializes a new player at the given position.
     *
     * The player has the given size, scaled so that 1 pixel = 1 Box2d unit
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param pos   Initial position in world coordinates
     * @param size  The size of the player in world units
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    virtual bool init(const Vec2 pos, const Size size) override {
        return init(pos, size, 1.0f);
    }
    
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
    virtual bool init(const Vec2& pos, const Size& size, float scale);

    
#pragma mark -
#pragma mark Static Constructors
	/**
	 * Creates a new player at the origin.
	 *
	 * The player is a unit square scaled so that 1 pixel = 1 Box2d unit
	 *
	 * The scene graph is completely decoupled from the physics system.
	 * The node does not have to be the same size as the physics body. We
	 * only guarantee that the scene graph node is positioned correctly
	 * according to the drawing scale.
	 *
	 * @return  A newly allocated PlayerModel at the origin
	 */
	static std::shared_ptr<PlayerModel> alloc() {
		std::shared_ptr<PlayerModel> result = std::make_shared<PlayerModel>();
		return (result->init() ? result : nullptr);
	}

	/**
	 * Creates a new player at the given position.
	 *
	 * The player is a unit square scaled so that 1 pixel = 1 Box2d unit
	 *
	 * The scene graph is completely decoupled from the physics system.
	 * The node does not have to be the same size as the physics body. We
	 * only guarantee that the scene graph node is positioned correctly
	 * according to the drawing scale.
	 *
     * @param pos   Initial position in world coordinates
	 *
	 * @return  A newly allocated PlayerModel at the given position
	 */
	static std::shared_ptr<PlayerModel> alloc(const Vec2& pos) {
		std::shared_ptr<PlayerModel> result = std::make_shared<PlayerModel>();
		return (result->init(pos) ? result : nullptr);
	}

    /**
	 * Creates a new player at the given position.
	 *
     * The player has the given size, scaled so that 1 pixel = 1 Box2d unit
	 *
 	 * The scene graph is completely decoupled from the physics system.
	 * The node does not have to be the same size as the physics body. We
	 * only guarantee that the scene graph node is positioned correctly
	 * according to the drawing scale.
	 *
	 * @param pos   Initial position in world coordinates
     * @param size  The size of the player in world units
	 *
	 * @return  A newly allocated PlayerModel at the given position with the given scale
	 */
	static std::shared_ptr<PlayerModel> alloc(const Vec2& pos, const Size& size) {
		std::shared_ptr<PlayerModel> result = std::make_shared<PlayerModel>();
		return (result->init(pos, size) ? result : nullptr);
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
     * @param size  The size of the player in world units
	 * @param scale The drawing scale (world to screen)
	 *
	 * @return  A newly allocated PlayerModel at the given position with the given scale
	 */
	static std::shared_ptr<PlayerModel> alloc(const Vec2& pos, const Size& size, float scale) {
		std::shared_ptr<PlayerModel> result = std::make_shared<PlayerModel>();
		return (result->init(pos, size, scale) ? result : nullptr);
	}
    

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
	const std::shared_ptr<scene2::SceneNode>& getSceneNode() const { return _node; }

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
        _node = node;
        _node->setPosition(getPosition() * _drawScale);
    }

    
#pragma mark -
#pragma mark Attribute Properties
    /**
     * Returns the remaining health of this character.
     *
     * @return HP of this character.
     */
    float getHP() const { return _hp; }

    /**
     * Sets the remaining health of this character.
     * 
     * @param value the new hp.
     */
    void setHP(float value) { _hp = value; }

    /**
     * Reduces the health of this character.
     * 
     * @param value the amount of hp reduction.
     */
    void damage(float value);

    /**
     * Returns left/right movement of this character.
     *
     * This is the result of input times player force.
     *
     * @return left/right movement of this character.
     */
    float getMovement() const { return _movement; }
    
    /**
     * Sets left/right movement of this character.
     *
     * This is the result of input times player force.
     *
     * @param value left/right movement of this character.
     */
    void setMovement(float value);

    /**
    * Sets the character to face left
    */
    void faceLeft();

    /**
    * Sets the character to face right
    */
    void faceRight();
    
    /**
     * Returns true if the player is actively firing.
     *
     * @return true if the player is actively firing.
     */
    bool isShooting() const { return _isShootInput && _shootCooldownRem <= 0; }
    /**
     * Returns true if the player is actively strafing left.
     *
     * @return true if the player is actively strafing left.
     */
    void setStrafeLeft(bool value) {_isStrafeLeft = value;};

    /**
     * Sets whether the player is actively strafing right.
     *
     * @param value whether the player is actively strafing right.
     */
    void setStrafeRight(bool value) {_isStrafeRight = value;};

    /**
     * Sets whether the player has a swallowed projectile.
     *
     * @param value whether the player has a swallowed projectile.
     */
    void setHasProjectile(bool value) { _hasProjectile = value; }

    /**
     * Sets whether the player is beginning dash left
     *
     * @param value whether the player is beginning dash left.
     */
    void setDashLeftInput(bool value) { _isDashLeftInput = value; }

    /**
     * Sets whether the player is beginning dash right.
     *
     * @param value whether the player is beginning dash right.
     */
    void setDashRightInput(bool value) { _isDashRightInput = value; }

    /**
     * Sets whether the player is actively trying to jump.
     *
     * @param value whether the player is actively trying to jump.
     */
    void setJumpInput(bool value) { _isJumpInput = value; }

    /**
     * Sets whether the player is actively firing.
     *
     * @param value whether the player is actively firing.
     */
    void setShootInput(bool value) { _isShootInput = value; }
    /**
     * Sets whether the player is inputting guard
     *
     * @param value whether the player is inputting guard
     */
    void setGuardInput(bool value) { _isGuardInput = value; }
    /**
     * Sets whether the player is actively strafing left.
     *
     * @param value whether the player is actively strafing left.
     */
    bool isStrafeLeft() { return _isStrafeLeft; };
    /**
     * Sets whether the player is actively strafing right.
     *
     * @return true if whether the player is actively strafing right.
     */
    bool isStrafeRight() { return _isStrafeRight; };
    /**
     * Returns true if if the player is actively trying to jump and jump cooldown is ready (regardless if on the ground).
     *
     * @return true if the player is actively trying to jump and jump cooldown is ready (regardless if on the ground or not).
     */
    bool isJumpBegin() const { return _isJumpInput && _jumpCooldownRem <= 0; }
    
    /**
     * Returns true if the player is actively dashing left.
     *
     * @param value whether the player is actively dashing left.
     */
    bool isDashLeftBegin() { return _isDashLeftInput && _dashCooldownRem <= 0; };
    /**
     * Returns true if the player is actively dashing right.
     *
     * @param value whether the player is actively dashing right.
     */
    bool isDashRightBegin() { return _isDashRightInput && _dashCooldownRem <= 0; };
    /**
     * Returns true if the player is dashing
     *
     * @return value whether the player is dashing either direction.
     */
    bool isDashBegin() { return isDashLeftBegin() || isDashRightBegin(); };
    /**
     * Returns true if the player is inputting a movement action/
     *
     * @return value whether the player is performing a movement action.
     */
    bool isMoveBegin() {return isDashBegin() || isStrafeLeft() || isStrafeRight() || (isJumpBegin() && isGrounded()) || isKnocked(); };
    
    /**
     *  Returns true if the player is currently beginning guard action.
     *
     * @return value whether the player is beginning guard action.
     */
    bool isGuardBegin() { return _isGuardInput && _guardCooldownRem <= 0; };
      /**
     * Returns true if the player has a swallowed projectile.
     *
     * @return value whether the player has a swallowed projectile.
     */
    /**
     * Returns true ifrthe player is actively guarding.
     *
     * @return value whether the player is actively guarding.
     */
    bool isGuardActive() { return  _guardRem > 0 || isGuardBegin(); };
    
    /**
     * Returns the amount of guard frames remaining
     *
     * @return the amount of guard frames remaining
     */
    int getGuardRem() {return _guardRem; };
    
    /**
     * Used to set the amount of guard frames remaining
     *
     * @param the value that remaining guard frames should be set to
     */
    void setGuardRem(int value = GUARD_DURATION) {_guardRem = value; };
    /**
     * Returns the amount of frames remaining before the player can guard again
     *
     * @returns the amount of frames remaining before the player can guard again
     */
    int getGuardCDRem() {return _guardCooldownRem; };
    
    /**
     * Used to set the amount of frames remaining before the player can guard again
     *
     * @param the amount of frames remaining before the player can guard again should be set to
     */
    void setGuardCDRem(int value = GUARD_COOLDOWN) {_guardCooldownRem = value; };
    
    /**
     * Returns the amount of dash frames remaining
     *
     * @return the amount of dash frames remaining
     */
    int getDashRem() {return _dashRem; };
    
    /**
     * Used to set the amount of dash frames remaining
     *
     * @param the value that remaining dash frames should be set to
     */
    void setDashRem(int value = DASH_DURATION) {_dashRem = value; };
    /**
     * Returns the amount of frames remaining before the player can dash again
     *
     * @returns the amount of frames remaining before the player can dash again
     */
    int getDashCDRem() {return _dashCooldownRem; };
    
    /**
     * Used to set the amount of frames remaining before the player can dash again
     *
     * @param the amount of frames remaining before the player can dash again should be set to
     */
    void setDashCDRem(int value = DASH_COOLDOWN) {_dashCooldownRem = value; };

    /**
     * Returns true if the player is actively parrying.
     *
     * @return value whether the player is actively parrying.
     */
    bool isParryActive() { return _parryRem > 0 || isGuardBegin(); };
    /**
     * Returns the amount of parry frames remaining
     *
     * @return the amount of parry frames remaining
     */
    int getParryRem() {return _parryRem; };
    
    /**
     * Used to set the amount of parry frames remaining
     *
     * @param the value that remaining parry frames should be set to
     */
    void setParryRem(int value = PARRY_DURATION) {_parryRem = value; };
    /**
     * Returns the amount of knockback frames remaining
     *
     * @return the amount of knockback frames remaining
     */
    int getKnockbackRem() {return _knockbackRem; };
    
    /**
     * Used to set the amount of knockback frames remaining
     *
     * @param the value that remaining knockback frames should be set to
     */
    void setKnockbackRem(int value = KB_DURATION) {_knockbackRem = value; };
    /**
     * Returns true if the player has a swallowed projectile.
     *
     * @return value whether the player has a swallowed projectile.
     */
    bool hasProjectile() { return _hasProjectile; };
    /**
     * Returns true if the player is in a dash animation.
     *
     * @return value whether the player is in a dash animation.
     */
    bool isDashActive() { return _dashRem > 0 || isDashBegin(); };
    /**
     * Returns the amount of frames remaining before the player can jump again
     *
     * @returns the amount of frames remaining before the player can jump again
     */
    int getJumpCDRem() {return _jumpCooldownRem; };
    
    /**
     * Used to set the amount of frames remaining before the player can jump again
     *
     * @param the value that the amount of frames remaining before the player can jump again should be set to
     */
    void setJumpCDRem(int value = JUMP_COOLDOWN) {_jumpCooldownRem = value; };
    /**
     * Returns the amount of frames remaining before the player can shoot again
     *
     * @returns the amount of frames remaining before the player can shoot again
     */
    int getShootCDRem() {return _shootCooldownRem; };
    
    /**
     * Used to set the amount of frames remaining before the player can shoot again
     *
     * @param the value that the amount of frames remaining before the player can shoot again should be set to
     */
    void setShootCDRem(int value = SHOOT_COOLDOWN) {_shootCooldownRem = value; };
    /**
     * Returns true if the player is being knocked back.
     *
     * @return true if the player is being knocked back.
     */
    bool isKnocked() const { return _isKnocked;}
    /**
     * Returns true if the player is in a knockback animation.
     *
     * @return value whether the player is in a knockback animation.
     */
    bool isKnockbackActive() { return _knockbackRem > 0 || isKnocked(); };
    /**
     * Returns true if the player is on the ground.
     *
     * @return true if the player is on the ground.
     */
    bool isGrounded() const { return _isGrounded; }
    
    /**
     * Sets whether the player is on the ground.
     *
     * @param value whether the player is on the ground.
     */
    void setGrounded(bool value) { _isGrounded = value; }
    float getKnockF() {return KB;}
    Vec2 getKnockDirection() {return _knockDirection;}
    /**
     * Sets whether the player is being knocked back
     *
     * @param value whether the player is being knocked back
     * @param knockDirection direction that the player will move toward
     */
    void setKnocked(bool value, Vec2 knockDirection) { _isKnocked = value; _knockDirection = knockDirection;  }
    /**
     * Resets knock status and direction
     */
    void resetKnocked() { _isKnocked = false; _knockDirection = Vec2(0,0);  }
    /**
     * Returns how much force to apply to get the player moving
     *
     * Multiply this by the input to get the movement value.
     *
     * @return how much force to apply to get the player moving
     */
    float getForce() const { return FORCE; }
    /**
     * @return how much jump force to apply
     */
    float getJumpF() const { return JUMP; }
    /**
     * @return how much dash force to apply
     */
    float getDashF() const { return DASH; }
    /** @return is dash left input */
    bool isDashLeftInput() const { return _isDashLeftInput; }
    /** @return is dash right input */
    bool isDashRightInput() const { return _isDashRightInput; }
    /**
     * Returns How hard the brakes are applied to get a player to stop moving
     *
     * @return How hard the brakes are applied to get a player to stop moving
     */
    float getDamping() const { return DAMPING; }
    
    /** @return Whether the dash has been released (reset). only for keyboard controls*/
    bool getDashReset() const { return _dashReset; };
    /** For keyboard dash controls*/
    void setDashReset(bool r){_dashReset = r;}
    /**
     * Returns the upper limit on player left-right movement.
     *
     * This does NOT apply to vertical movement.
     *
     * @return the upper limit on player left-right movement.
     */
    float getMaxSpeed() const { return MAXSPEED; }
    /**
     * Returns the name of the body fixture
     *
     * This is used by ContactListener
     *
     * @return the name of the body fixture
     */
    std::string* getBodyName() { return &_bodyName; }
    /**
     * Returns the name of the ground sensor
     *
     * This is used by ContactListener
     *
     * @return the name of the ground sensor
     */
    std::string* getGroundSensorName() { return &_sensorName; }
    /**
     * Returns the name of the shield sensor
     *
     * This is used by ContactListener
     *
     * @return the name of the shield sensor
     */
    std::string* getShieldName() { return &_shieldName; }
    
    /**
     * Returns true if this character is facing right
     *
     * @return true if this character is facing right
     */
    bool isFacingRight() const { return _faceRight; }

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
    
    /**
     * Applies the force to the body of this player
     *
     * This method should be called after the force attribute is set.
     */
    void applyForce();


	
};

#endif /* __GB_MODEL_H__ */
