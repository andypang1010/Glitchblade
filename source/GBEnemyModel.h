//
//  GBDudeModel.h
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
#ifndef __GB_ENEMY_MODEL_H__
#define __GB_ENEMY_MODEL_H__
#include <cugl/cugl.h>

using namespace cugl;

#pragma mark -
#pragma mark Drawing Constants
/** The texture for the character avatar */
#define ENEMY_TEXTURE   "enemy"
/** Identifier to allow us to track the player sensor in ContactListener */
#define ENEMY_BODY_NAME      "enemybody"
#define ENEMY_SENSOR_NAME     "enemysensor"
#define ENEMY_SHIELD_SENSOR_NAME      "shield"



#pragma mark -
#pragma mark Physics Constants
/** The factor to multiply by the input */
#define ENEMY_FORCE      20.0f
/** The amount to slow the character down */
#define ENEMY_DAMPING    30.0f
/** The maximum character speed */
#define ENEMY_MAXSPEED   10.0f
/** The maximum character hp */
#define ENEMY_MAXHP   100.0f


#pragma mark -
#pragma mark Enemy Model
/**
* Player avatar for the plaform game.
*
* Note that this class uses a capsule shape, not a rectangular shape.  In our
* experience, using a rectangular shape for a character will regularly snag
* on a platform.  The round shapes on the end caps lead to smoother movement.
*/
class EnemyModel : public physics2::BoxObstacle {
private:
	/** This macro disables the copy constructor (not allowed on physics objects) */
	CU_DISALLOW_COPY_AND_ASSIGN(EnemyModel);
protected:
    /** This character's remaining health */
    float _hp;
	/** The current horizontal movement of the character */
	float _movement;
	/** Which direction is the character facing */
	bool _faceRight;
	/** How long until we can jump again in animation frames */
	int  _jumpCooldownRem;
    /** How long until we can jump again in frames*/
    int  _dashCooldownRem;
    /** How many frames remaining in the dash animation (affects friciton)*/
    int  _dashRem;
    /** How long until we can guard again in frames */
    int  _guardCooldownRem;
    /** How many frames remaining in the guard (0 when guard is not active) */
    int  _guardRem;
    /** How many frames remaining in the parry (0 when parry is not active) */
    int  _parryRem;
    /** How many frames remaining in enemy stun */
    int _stunRem;
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
	/** How long until we can shoot again in animation frames*/
	int  _shootCooldownRem;
	/** Whether our feet are on the ground */
	bool _isGrounded;

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
    
	/** The scene graph node for the Dude. */
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
     * Creates a degenerate Dude object.
     *
     * This constructor does not initialize any of the dude values beyond
     * the defaults.  To use a DudeModel, you must call init().
     */
    EnemyModel() : BoxObstacle(), _sensorName(ENEMY_SENSOR_NAME), _shieldName(ENEMY_SHIELD_SENSOR_NAME), _bodyName(ENEMY_BODY_NAME) { }
    
    /**
     * Destroys this DudeModel, releasing all resources.
     */
    virtual ~EnemyModel(void) { dispose(); }
    
    /**
     * Disposes all resources and assets of this DudeModel
     *
     * Any assets owned by this object will be immediately released.  Once
     * disposed, a DudeModel may not be used until it is initialized again.
     */
    void dispose();
    
    /**
     * Initializes a new dude at the origin.
     *
     * The dude is a unit square scaled so that 1 pixel = 1 Box2d unit
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
     * Initializes a new dude at the given position.
     *
     * The dude is unit square scaled so that 1 pixel = 1 Box2d unit
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
     * Initializes a new dude at the given position.
     *
     * The dude has the given size, scaled so that 1 pixel = 1 Box2d unit
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param pos   Initial position in world coordinates
     * @param size  The size of the dude in world units
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    virtual bool init(const Vec2 pos, const Size size) override {
        return init(pos, size, 1.0f);
    }
    
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
    virtual bool init(const Vec2& pos, const Size& size, float scale);

    
#pragma mark -
#pragma mark Static Constructors
	/**
	 * Creates a new dude at the origin.
	 *
	 * The dude is a unit square scaled so that 1 pixel = 1 Box2d unit
	 *
	 * The scene graph is completely decoupled from the physics system.
	 * The node does not have to be the same size as the physics body. We
	 * only guarantee that the scene graph node is positioned correctly
	 * according to the drawing scale.
	 *
	 * @return  A newly allocated DudeModel at the origin
	 */
	static std::shared_ptr<EnemyModel> alloc() {
		std::shared_ptr<EnemyModel> result = std::make_shared<EnemyModel>();
		return (result->init() ? result : nullptr);
	}

	/**
	 * Creates a new dude at the given position.
	 *
	 * The dude is a unit square scaled so that 1 pixel = 1 Box2d unit
	 *
	 * The scene graph is completely decoupled from the physics system.
	 * The node does not have to be the same size as the physics body. We
	 * only guarantee that the scene graph node is positioned correctly
	 * according to the drawing scale.
	 *
     * @param pos   Initial position in world coordinates
	 *
	 * @return  A newly allocated DudeModel at the given position
	 */
	static std::shared_ptr<EnemyModel> alloc(const Vec2& pos) {
		std::shared_ptr<EnemyModel> result = std::make_shared<EnemyModel>();
		return (result->init(pos) ? result : nullptr);
	}

    /**
	 * Creates a new dude at the given position.
	 *
     * The dude has the given size, scaled so that 1 pixel = 1 Box2d unit
	 *
 	 * The scene graph is completely decoupled from the physics system.
	 * The node does not have to be the same size as the physics body. We
	 * only guarantee that the scene graph node is positioned correctly
	 * according to the drawing scale.
	 *
	 * @param pos   Initial position in world coordinates
     * @param size  The size of the dude in world units
	 *
	 * @return  A newly allocated DudeModel at the given position with the given scale
	 */
	static std::shared_ptr<EnemyModel> alloc(const Vec2& pos, const Size& size) {
		std::shared_ptr<EnemyModel> result = std::make_shared<EnemyModel>();
		return (result->init(pos, size) ? result : nullptr);
	}

	/**
	 * Creates a new dude at the given position.
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
	 * @return  A newly allocated DudeModel at the given position with the given scale
	 */
	static std::shared_ptr<EnemyModel> alloc(const Vec2& pos, const Size& size, float scale) {
		std::shared_ptr<EnemyModel> result = std::make_shared<EnemyModel>();
		return (result->init(pos, size, scale) ? result : nullptr);
	}
    

#pragma mark -
#pragma mark Animation
    /**
     * Returns the scene graph node representing this DudeModel.
     *
     * By storing a reference to the scene graph node, the model can update
     * the node to be in sync with the physics info. It does this via the
     * {@link Obstacle#update(float)} method.
     *
     * @return the scene graph node representing this DudeModel.
     */
	const std::shared_ptr<scene2::SceneNode>& getSceneNode() const { return _node; }

    /**
     * Sets the scene graph node representing this DudeModel.
     *
     * Note that this method also handles creating the nodes for the body parts
     * of this DudeModel. Since the obstacles are decoupled from the scene graph,
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
     * @param node  The scene graph node representing this DudeModel, which has been added to the world node already.
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
     * This is the result of input times dude force.
     *
     * @return left/right movement of this character.
     */
    float getMovement() const { return _movement; }
    
    /**
     * Sets left/right movement of this character.
     *
     * This is the result of input times dude force.
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
     * Returns true if the dude is actively firing.
     *
     * @return true if the dude is actively firing.
     */
    bool isShooting() const { return _isShootInput && _shootCooldownRem <= 0; }
    /**
     * Returns true if the dude is actively strafing left.
     *
     * @return true if the dude is actively strafing left.
     */
    void setStrafeLeft(bool value) {_isStrafeLeft = value;};

    /**
     * Sets whether the dude is actively strafing right.
     *
     * @param value whether the dude is actively strafing right.
     */
    void setStrafeRight(bool value) {_isStrafeRight = value;};

    /**
     * Sets whether the dude has a swallowed projectile.
     *
     * @param value whether the dude has a swallowed projectile.
     */
    void setHasProjectile(bool value) { _hasProjectile = value; }

    /**
     * Sets whether the dude is beginning dash left
     *
     * @param value whether the dude is beginning dash left.
     */
    void setDashLeftInput(bool value) { _isDashLeftInput = value; }

    /**
     * Sets whether the dude is beginning dash right.
     *
     * @param value whether the dude is beginning dash right.
     */
    void setDashRightInput(bool value) { _isDashRightInput = value; }

    /**
     * Sets whether the dude is actively trying to jump.
     *
     * @param value whether the dude is actively trying to jump.
     */
    void setJumpInput(bool value) { _isJumpInput = value; }

    /**
     * Sets whether the dude is actively firing.
     *
     * @param value whether the dude is actively firing.
     */
    void setShootInput(bool value) { _isShootInput = value; }
    /**
     * Sets whether the dude is inputting guard
     *
     * @param value whether the dude is inputting guard
     */
    void setGuardInput(bool value) { _isGuardInput = value; }
    /**
     * Sets whether the dude is actively strafing left.
     *
     * @param value whether the dude is actively strafing left.
     */
    bool isStrafeLeft() { return _isStrafeLeft; };
    /**
     * Sets whether the dude is actively strafing right.
     *
     * @return true if whether the dude is actively strafing right.
     */
    bool isStrafeRight() { return _isStrafeRight; };
    /**
     * Returns true if if the dude is actively trying to jump and jump cooldown is ready (regardless if on the ground).
     *
     * @return true if the dude is actively trying to jump and jump cooldown is ready (regardless if on the ground or not).
     */
    bool isJumpBegin() const { return _isJumpInput && _jumpCooldownRem <= 0; }
    
    /**
     * Returns true if the dude is actively dashing left.
     *
     * @param value whether the dude is actively dashing left.
     */
    bool isDashLeftBegin() { return _isDashLeftInput && _dashCooldownRem <= 0; };
    /**
     * Returns true if the dude is actively dashing right.
     *
     * @param value whether the dude is actively dashing right.
     */
    bool isDashRightBegin() { return _isDashRightInput && _dashCooldownRem <= 0; };
    /**
     * Returns true if the dude is dashing
     *
     * @return value whether the dude is dashing either direction.
     */
    bool isDashBegin() { return isDashLeftBegin() || isDashRightBegin(); };
    /**
     * Returns true if the dude is inputting a movement action/
     *
     * @return value whether the dude is performing a movement action.
     */
    bool isMoveBegin() {return isDashBegin() || isStrafeLeft() || isStrafeRight() || (isJumpBegin() && isGrounded()); };
    
    /**
     *  Returns true if the dude is currently beginning guard action.
     *
     * @return value whether the dude is beginning guard action.
     */
    bool isGuardBegin() { return _isGuardInput && _guardCooldownRem <= 0; };
      /**
     * Returns true if the dude has a swallowed projectile.
     *
     * @return value whether the dude has a swallowed projectile.
     */
    /**
     * Returns true ifrthe dude is actively guarding.
     *
     * @return value whether the dude is actively guarding.
     */
    bool isGuardActive() { return  _guardRem > 0 || isGuardBegin(); };

    /**
     * Returns true if the dude is actively parrying.
     *
     * @return value whether the dude is actively parrying.
     */
    bool isParryActive() { return _parryRem > 0 || isGuardBegin(); };
    /**
     * Returns true if the dude has a swallowed projectile.
     *
     * @return value whether the dude has a swallowed projectile.
     */
    bool hasProjectile() { return _hasProjectile; };
    /**
     * Returns true if the dude is in a dash animation.
     *
     * @return value whether the dude is in a dash animation.
     */
    bool isDashActive() { return _dashRem > 0 || isDashBegin(); };
    /**
     * Sets the dash duration of this enemy.
     *
     * @param value new dash duration.
     */
    void setDashRem(int value) { _dashRem = value; };
    /**
     * Sets the stun duration of this enemy.
     *
     * @param value new stun duration.
     */
    void setStun(int value) { _stunRem = value; };
    /**
     * Checks if enemy is stunned.
     *
     * @return whether the enemy is stunned.
     */
    bool isStunned() { return _stunRem>0; };
    /**
     * Returns true if the dude is on the ground.
     *
     * @return true if the dude is on the ground.
     */
    bool isGrounded() const { return _isGrounded; }
    
    /**
     * Sets whether the dude is on the ground.
     *
     * @param value whether the dude is on the ground.
     */
    void setGrounded(bool value) { _isGrounded = value; }
    
    /**
     * Returns how much force to apply to get the dude moving
     *
     * Multiply this by the input to get the movement value.
     *
     * @return how much force to apply to get the dude moving
     */
    float getForce() const { return ENEMY_FORCE; }
    
    /**
     * Returns How hard the brakes are applied to get a dude to stop moving
     *
     * @return How hard the brakes are applied to get a dude to stop moving
     */
    float getDamping() const { return ENEMY_DAMPING; }
    
    /**
     * Returns the upper limit on dude left-right movement.
     *
     * This does NOT apply to vertical movement.
     *
     * @return the upper limit on dude left-right movement.
     */
    float getMaxSpeed() const { return ENEMY_MAXSPEED; }
    
    /**
     * Returns the name of the ground sensor
     *
     * This is used by ContactListener
     *
     * @return the name of the ground sensor
     */
    std::string* getSensorName() { return &_sensorName; }
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

    /**
     * Returns the name of the body fixture
     *
     * This is used by ContactListener
     *
     * @return the name of the body fixture
     */
    std::string* getBodyName() { return &_bodyName; }

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
     * Applies the force to the body of this dude
     *
     * This method should be called after the force attribute is set.
     */
    void applyForce();


	
};

#endif /* __GB_ENEMY_MODEL_H__ */
