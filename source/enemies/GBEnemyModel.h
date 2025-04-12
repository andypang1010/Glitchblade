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
#ifndef __GB_ENEMY_MODEL_H__
#define __GB_ENEMY_MODEL_H__
#include <cugl/cugl.h>
#include "GBActionModel.h"
#include "actionmodel_variants/GBMeleeActionModel.h"
#include "actionmodel_variants/GBRangedActionModel.h"

using namespace cugl;

#pragma mark -
#pragma mark Drawing Constants
/** The texture for the character avatar */
#define ENEMY_TEXTURE   "enemy"
/** Identifier to allow us to track the player sensor in ContactListener */
#define ENEMY_BODY_NAME      "enemybody"
#define ENEMY_SENSOR_NAME     "enemysensor"

#define E_ANIMATION_UPDATE_FRAME 4

#define ENEMY_HIT_COLOR_DURATION 8

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

/** The amount to shrink the body fixture (vertically) relative to the image */
#define ENEMY_VSHRINK  0.95f
/** The amount to shrink the body fixture (horizontally) relative to the image */
#define ENEMY_HSHRINK  0.7f
/** The amount to shrink the sensor fixture (horizontally) relative to the image */
#define ENEMY_SSHRINK  0.6f
/** Height of the sensor attached to the player's feet */
#define ENEMY_SENSOR_HEIGHT   0.1f
/** The density of the character */
#define ENEMY_DENSITY    1.0f
/** The implulse fot the character knockback */
#define ENEMY_KB       1.0f
#define ENEMY_KB_DURATION 20
/** Debug color for the sensor */
#define ENEMY_DEBUG_COLOR     Color4::RED
/** enemy obstacle name*/
#define ENEMY_NAME      "enemy"
#define ENEMY_DEBUG_FONT      "debug"

#pragma mark -
#pragma mark Action Constants // TODO: Refactor with Action parser

#pragma mark -
#pragma mark AI Constants
#define CLOSE_RADIUS 6.0f
#define FAR_RADIUS 12.0f

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
    /** How many frames remaining in enemy stun */
    int _stunRem;
    /** Whether we are actively inputting strafe left*/
    bool _isMoveLeft;
    /** Whether we are actively inputting strafe right*/
    bool _isMoveRight;
    /** Whether enemy can be knocked-back */
    bool _canKnockBack;
    /** Whether enemy is knocked-back (sets  cd) */
    bool _isKnocked;
    /** Whether we are knocked-back (sets input cd) */
    int _knockbackRem;
    Vec2 _knockDirection;
    /** Whether our feet are on the ground */
    bool _isGrounded;
    int _lastDamagedFrame;

    Vec2 _targetPos;
    float _aggression = 0.0f;

    int _moveDuration;
    int _moveDirection;

    std::string _bodyName;
    /** Ground sensor to represent our feet */
    b2Fixture* _sensorFixture;
    /** Reference to the sensor name (since a constant cannot have a pointer) */
    std::string _sensorName;
    /** The node for debugging the ground sensor */
    std::shared_ptr<scene2::WireNode> _sensorNode;

    /** The scene graph node for the enemy. */
    std::shared_ptr<scene2::SceneNode> _node;
    /** The scale between the physics world and the screen (MUST BE UNIFORM) */
    float _drawScale;

    float _scale;
    std::shared_ptr<JsonValue> _enemyJSON;

    /**
    * Redraws the outline of the physics fixtures to the debug node
    *
    * The debug node is use to outline the fixtures attached to this object.
    * This is very useful when the fixtures have a very different shape than
    * the texture (e.g. a circular shape attached to a square texture).
    */
    virtual void resetDebug() override;

public:
    int currentFrame = 0;
    int frameCounter = 0;
    int stunFrame;

    std::shared_ptr<scene2::SpriteNode> _currentSpriteNode;

    std::shared_ptr<scene2::SpriteNode> _idleSprite;
    std::shared_ptr<scene2::SpriteNode> _walkSprite;
    std::shared_ptr<scene2::SpriteNode> _stunSprite;

public:

#pragma mark Hidden Constructors
    /**
     * Creates a degenerate enemy object.
     *
     * This constructor does not initialize any of the enemy values beyond
     * the defaults.  To use a PlayerModel, you must call init().
     */
    EnemyModel() : BoxObstacle(), _sensorName(ENEMY_SENSOR_NAME), _bodyName(ENEMY_BODY_NAME), _scale(Application::get()->getDisplayWidth() / 1248) {}

    /**
     * Destroys this PlayerModel, releasing all resources.
     */
    virtual ~EnemyModel(void) { dispose(); }

    /**
     * Disposes all resources and assets of this PlayerModel
     *
     * Any assets owned by this object will be immediately released.  Once
     * disposed, a PlayerModel may not be used until it is initialized again.
     */
    virtual void dispose();

    /**
     * Initializes a new enemy at the given position.
     *
     * The enemy is sized according to the given drawing scale.
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param pos   Initial position in world coordinates
     * @param size  The size of the enemy in world units
     * @param scale The drawing scale (world to screen)
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    virtual bool init(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& constantsRef, const Vec2& pos, std::vector<std::shared_ptr<ActionModel>> actions);

#pragma mark -
#pragma mark Level Control and Constructor Helpers
    /** Reset all the enemy attributes to their initial values*/
    virtual void resetAttributes() {
        _hp = ENEMY_MAXHP;
        _isGrounded = false;
        _isMoveLeft = false;
        _isMoveRight = false;
        _faceRight = true;
        _canKnockBack = true;
        _stunRem = 0;
        _aggression = 0;

        _moveDuration = 0;
        currentFrame = 0;
    };

    /**Attach the scene nodes (sprite sheets) to the enemy**/
    virtual void attachNodes(const std::shared_ptr<AssetManager>& assetRef);
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
    virtual const std::shared_ptr<scene2::SceneNode>& getSceneNode() const { return _node; }

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
    virtual void setSceneNode(const std::shared_ptr<scene2::SceneNode>& node) {
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
    virtual float getHP() const { return _hp; }

    /**
     * Sets the remaining health of this character.
     *
     * @param value the new hp.
     */
    virtual void setHP(float value) { _hp = value; }

    /**
     * Reduces the health of this character.
     *
     * @param value the amount of hp reduction.
     */
    virtual void damage(float value);

    /**
     * Returns left/right movement of this character.
     *
     * This is the result of input times enemy force.
     *
     * @return left/right movement of this character.
     */
    virtual float getMovement() const { return _movement; }

    /**
     * Sets left/right movement of this character.
     *
     * This is the result of input times enemy force.
     *
     * @param value left/right movement of this character.
     */
    virtual void setMovement(float value);

    /**
    * Sets the character to face left
    */
    virtual void faceLeft();

    /**
    * Sets the character to face right
    */
    virtual void faceRight();

    /**
     * Returns true if the enemy is actively strafing left.
     *
     * @return true if the enemy is actively strafing left.
     */
    virtual void setMoveLeft(bool value) { _isMoveLeft = value; };

    /**
     * Sets whether the enemy is actively strafing right.
     *
     * @param value whether the enemy is actively strafing right.
     */
    virtual void setMoveRight(bool value) { _isMoveRight = value; };

    /**
     * Sets whether the enemy is actively strafing left.
     *
     * @param value whether the enemy is actively strafing left.
     */
    virtual bool isMoveLeft() { return _isMoveLeft; };
    /**
     * Sets whether the enemy is actively strafing right.
     *
     * @return true if whether the enemy is actively strafing right.
     */
    virtual bool isMoveRight() { return _isMoveRight; };

    /**
     * Returns true if the enemy is inputting a movement action/
     *
     * @return value whether the enemy is performing a movement action.
     */
    virtual bool isMoveBegin() { return isMoveLeft() || isMoveRight() || isKnocked(); };

    /**
    * Returns true if the enemy is being knocked back.
    *
    * @return true if the enemy is being knocked back.
    */
    virtual bool isKnocked() const { return _isKnocked; }
    /**
     * Sets whether the enemy is being knocked back
     *
     * @param value whether the enemy is being knocked back
     * @param knockDirection direction that the enemy will move toward
     */
     /**
      * Returns true if the enemy is in a knockback animation.
      *
      * @return value whether the enemy is in a knockback animation.
      */
    virtual bool isKnockbackActive() { return _knockbackRem > 0 || isKnocked(); };
    virtual float getKnockF() { return ENEMY_KB; }
    virtual Vec2 getKnockDirection() { return _knockDirection; }
    /**
     * Sets whether the player is being knocked back
     *
     * @param value whether the player is being knocked back
     * @param knockDirection direction that the player will move toward
     */
    virtual void setKnocked(bool value, Vec2 knockDirection) { _isKnocked = value; _knockDirection = knockDirection; }
    /**
     * Resets knock status - do this after applying force.
     */
    virtual void resetKnocked() { _isKnocked = false; }
    /**
     * Returns the amount of knockback frames remaining
     *
     * @return the amount of knockback frames remaining
     */
    virtual int getKnockbackRem() { return _knockbackRem; };

    /**
     * Used to set the amount of knockback frames remaining
     *
     * @param the value that remaining knockback frames should be set to
     */
    virtual void setKnockbackRem(int value = ENEMY_KB_DURATION) { _knockbackRem = value; };

    /**
     * Sets the stun duration of this enemy.
     *
     * @param value new stun duration.
     */
    virtual void setStun(int value) { _stunRem = value; };
    /**
     * Returns the stun duration remaining.
     *
     * @return the remaining stun duration
     */
    virtual int getStunRem() { return _stunRem; };
    /**
     * Checks if enemy is stunned.
     *
     * @return whether the enemy is stunned.
     */
    virtual bool isStunned() { return _stunRem > 0; };
    /**
     * Returns true if the enemy is on the ground.
     *
     * @return true if the enemy is on the ground.
     */
    virtual bool isGrounded() const { return _isGrounded; }

    /**
     * Sets whether the enemy is on the ground.
     *
     * @param value whether the enemy is on the ground.
     */
    virtual void setGrounded(bool value) { _isGrounded = value; }

    /**
     * Returns how much force to apply to get the enemy moving
     *
     * Multiply this by the input to get the movement value.
     *
     * @return how much force to apply to get the enemy moving
     */
    virtual float getForce() const { return ENEMY_FORCE; }

    /**
     * Returns How hard the brakes are applied to get a enemy to stop moving
     *
     * @return How hard the brakes are applied to get a enemy to stop moving
     */
    virtual float getDamping() const { return ENEMY_DAMPING; }

    /**
     * Returns the upper limit on enemy left-right movement.
     *
     * This does NOT apply to vertical movement.
     *
     * @return the upper limit on enemy left-right movement.
     */
    virtual float getMaxSpeed() const { return ENEMY_MAXSPEED; }

    /**
     * Returns the name of the ground sensor
     *
     * This is used by ContactListener
     *
     * @return the name of the ground sensor
     */
    virtual std::string* getSensorName() { return &_sensorName; }

    /**
     * Returns true if this character is facing right
     *
     * @return true if this character is facing right
     */
    virtual bool isFacingRight() const { return _faceRight; }

    /**
     * Returns the name of the body fixture
     *
     * This is used by ContactListener
     *
     * @return the name of the body fixture
     */
    virtual std::string* getBodyName() { return &_bodyName; }

#pragma mark -
#pragma mark AI Methods
    /**
     * Sets the target position of this enemy
     *
     * @param pos the position of target(player)
     */
    virtual void setTargetPos(Vec2 pos) { _targetPos = pos; }

    /**
     * Make the enemy face the target
     *
     */
    virtual void faceTarget();

    /**
     * Make the enemy move toward the target
     *
	 * @param duration the duration of the movement
     */
    virtual void approachTarget(int duration) { faceTarget(); _moveDirection = 1; _moveDuration = duration; };
    /**
   * Make the enemy move away from the target
   *
   * @param duration the duration of the movement
   */
    virtual void avoidTarget(int duration) { faceTarget(); _moveDirection = -1; _moveDuration = duration; };

    virtual bool isTargetClose();
    virtual bool isTargetFar();
    virtual void nextAction();
    virtual void AIMove();

    virtual void setAggression(float value) { _aggression = value; }
	virtual float getAggression() { return _aggression; }

    /**
     * Returns the action when an attack hitbox should be active, or nothing when no attack is active
     *
     * @return the action that needs hitbox, or nullptr when no hitbox is active
     */
    virtual std::shared_ptr<MeleeActionModel> getDamagingAction();

    /**
     * Returns the action when a projectile is going to be shot, or nothing when no attack is active
     *
     * @return the action that needs projectile, or nullptr when no ranged attack is active
     */
    virtual std::shared_ptr<RangedActionModel> getProjectileAction();

#pragma mark -
#pragma mark Animation Methods
    void playAnimation(std::shared_ptr<scene2::SpriteNode> sprite);
    void playVFXAnimation(std::shared_ptr<scene2::SpriteNode> actionSprite, std::shared_ptr<scene2::SpriteNode> vfxSprite, int startFrame);
    virtual void updateAnimation();

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
    virtual void createFixtures() override;

    /**
     * Release the fixtures for this body, reseting the shape
     *
     * This is the primary method to override for custom physics objects.
     */
    virtual void releaseFixtures() override;

    /**
     * Updates the object's physics state (NOT GAME LOGIC).
     *
     * We use this method to reset cooldowns.
     *
     * @param delta Number of seconds since last animation frame
     */
    virtual void update(float dt) override;




};

#endif /* __GB_ENEMY_MODEL_H__ */