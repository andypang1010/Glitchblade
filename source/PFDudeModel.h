//
//  PFDudeModel.h
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
#ifndef __PF_DUDE_MODEL_H__
#define __PF_DUDE_MODEL_H__
#include <cugl/cugl.h>

using namespace cugl;

#pragma mark -
#pragma mark Drawing Constants
/** The texture for the character avatar */
#define DUDE_TEXTURE    "dude"
/** Identifier to allow us to track the sensor in ContactListener */
#define SENSOR_NAME     "dudesensor"


#pragma mark -
#pragma mark Physics Constants
/** The factor to multiply by the input */
#define DUDE_FORCE      20.0f
/** The amount to slow the character down */
#define DUDE_DAMPING    10.0f
/** The maximum character speed */
#define DUDE_MAXSPEED   5.0f


#pragma mark -
#pragma mark Dude Model
/**
* Player avatar for the plaform game.
*
* Note that this class uses a capsule shape, not a rectangular shape.  In our
* experience, using a rectangular shape for a character will regularly snag
* on a platform.  The round shapes on the end caps lead to smoother movement.
*/
class DudeModel : public physics2::CapsuleObstacle {
private:
	/** This macro disables the copy constructor (not allowed on physics objects) */
	CU_DISALLOW_COPY_AND_ASSIGN(DudeModel);
protected:
	/** The current horizontal movement of the character */
	float _movement;
	/** Which direction is the character facing */
	bool _faceRight;
	/** How long until we can jump again in animation frames */
	int  _jumpCooldown;
    /** How long until we can jump again in frames*/
    int  _dashCooldown;
    /** How long until we can guard again in frames */
    int  _guardCooldown;
	/** Whether we are actively jumping */
	bool _isJumping;
    /** Whether we are actively dashing left*/
    bool _isDashingLeft;
    /** Whether we are actively dashing right*/
    bool _isDashingRight;
    /** Whether we are actively guarding*/
    bool _isGuarding;
    /** Whether we are actively parrying*/
    bool _isParrying;
    /** Whether we have a (swallowed) projectile*/
    bool _hasProjectile;
	/** How long until we can shoot again */
	int  _shootCooldown;
	/** Whether our feet are on the ground */
	bool _isGrounded;
	/** Whether we are actively shooting */
	bool _isShooting;
	/** Ground sensor to represent our feet */
	b2Fixture*  _sensorFixture;
	/** Reference to the sensor name (since a constant cannot have a pointer) */
	std::string _sensorName;
	/** The node for debugging the sensor */
	std::shared_ptr<scene2::WireNode> _sensorNode;
    /** The guard field when guard is active */
    std::shared_ptr<physics2::WheelObstacle> _guardField;
    
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
    DudeModel() : CapsuleObstacle(), _sensorName(SENSOR_NAME) { }
    
    /**
     * Destroys this DudeModel, releasing all resources.
     */
    virtual ~DudeModel(void) { dispose(); }
    
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
	static std::shared_ptr<DudeModel> alloc() {
		std::shared_ptr<DudeModel> result = std::make_shared<DudeModel>();
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
	static std::shared_ptr<DudeModel> alloc(const Vec2& pos) {
		std::shared_ptr<DudeModel> result = std::make_shared<DudeModel>();
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
	static std::shared_ptr<DudeModel> alloc(const Vec2& pos, const Size& size) {
		std::shared_ptr<DudeModel> result = std::make_shared<DudeModel>();
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
	static std::shared_ptr<DudeModel> alloc(const Vec2& pos, const Size& size, float scale) {
		std::shared_ptr<DudeModel> result = std::make_shared<DudeModel>();
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
     * Returns true if the dude is actively firing.
     *
     * @return true if the dude is actively firing.
     */
    bool isShooting() const { return _isShooting && _shootCooldown <= 0; }
    
    /**
     * Sets whether the dude is actively firing.
     *
     * @param value whether the dude is actively firing.
     */
    void setShooting(bool value) { _isShooting = value; }
    
    /**
     * Returns true if the dude is actively jumping.
     *
     * @return true if the dude is actively jumping.
     */
    bool isJumping() const { return _isJumping && _jumpCooldown <= 0; }
    
    /**
     * Sets whether the dude is actively jumping.
     *
     * @param value whether the dude is actively jumping.
     */
    /**
     * Sets whether the dude is actively dashing left.
     *
     * @param value whether the dude is actively dashing left.
     */
    bool isDashingLeft() { return _isDashingLeft && _dashCooldown <= 0; };
    /**
     * Sets whether the dude is actively dashing right.
     *
     * @param value whether the dude is actively dashing right.
     */
    bool isDashingRight() { return _isDashingRight && _dashCooldown <= 0; };
    /**
     * Sets whether the dude is actively guarding.
     *
     * @param value whether the dude is actively guarding.
     */
    bool isGuarding() { return _isGuarding && _guardCooldown <= 0; };
    /**
     * Sets whether the dude is actively parrying.
     *
     * @param value whether the dude is actively dashing left.
     */
    bool isParrying() { return _isJumping && _jumpCooldown <= 0; };
    /**
     * Sets whether the dude has a swallowed projectile.
     *
     * @param value whether the dude has a swallowed projectile..
     */
    bool hasProjectile() { return _hasProjectile; };
    
    void setHasProjectile(bool value) { _hasProjectile = value; }
    void setGuarding(bool value) { _isGuarding = value; }
    void setDashingLeft(bool value) { _isDashingLeft = value; }
    void setDashingRight(bool value) { _isDashingRight = value; }
    void setJumping(bool value) { _isJumping = value; }
    
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
    float getForce() const { return DUDE_FORCE; }
    
    /**
     * Returns ow hard the brakes are applied to get a dude to stop moving
     *
     * @return ow hard the brakes are applied to get a dude to stop moving
     */
    float getDamping() const { return DUDE_DAMPING; }
    
    /**
     * Returns the upper limit on dude left-right movement.
     *
     * This does NOT apply to vertical movement.
     *
     * @return the upper limit on dude left-right movement.
     */
    float getMaxSpeed() const { return DUDE_MAXSPEED; }
    
    /**
     * Returns the name of the ground sensor
     *
     * This is used by ContactListener
     *
     * @return the name of the ground sensor
     */
    std::string* getSensorName() { return &_sensorName; }
    
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
     * Applies the force to the body of this dude
     *
     * This method should be called after the force attribute is set.
     */
    void applyForce();


	
};

#endif /* __PF_DUDE_MODEL_H__ */
