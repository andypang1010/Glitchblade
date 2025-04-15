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
#ifndef __GB_BOSS1_MODEL_H__
#define __GB_BOSS1_MODEL_H__
#include <cugl/cugl.h>
#include "../GBEnemyModel.h"
#include "../GBActionModel.h"
#include "../actionmodel_variants/GBMeleeActionModel.h"
#include "../actionmodel_variants/GBRangedActionModel.h"

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
/** The impulse for the character dash-attack */
#define STAB_FORCE       200.0f
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
#define SLAM_FRAMES     40
#define STAB_FRAMES     40
#define SHOOT_FRAMES    15
#define EXPLODE_FRAMES  40

#define STUN_FRAMES 88

#pragma mark -
#pragma mark AI Constants
#define CLOSE_RADIUS     6.0f
#pragma mark -
#pragma mark Enemy Model
/**
* Player avatar for the plaform game.
*
* Note that this class uses a capsule shape, not a rectangular shape.  In our
* experience, using a rectangular shape for a character will regularly snag
* on a platform.  The round shapes on the end caps lead to smoother movement.
*/
class Boss1Model : public EnemyModel {
private:
    /** This macro disables the copy constructor (not allowed on physics objects) */
    CU_DISALLOW_COPY_AND_ASSIGN(Boss1Model);
protected:
    std::string _sensorName;
    std::string _bodyName;

    bool _isStabbing;
    bool _isSlamming;
    bool _isShooting;
    bool _isExploding;

    std::shared_ptr<MeleeActionModel> _slam;
    std::shared_ptr<MeleeActionModel> _stab;
    std::shared_ptr<RangedActionModel> _shoot;
    std::shared_ptr<MeleeActionModel> _explode;

    /**
    * Redraws the outline of the physics fixtures to the debug node
    *
    * The debug node is use to outline the fixtures attached to this object.
    * This is very useful when the fixtures have a very different shape than
    * the texture (e.g. a circular shape attached to a square texture).
    */
    virtual void resetDebug() override;

public:
    std::shared_ptr<scene2::SpriteNode> _stabSprite;
    std::shared_ptr<scene2::SpriteNode> _slamSprite;
    std::shared_ptr<scene2::SpriteNode> _shootSprite;
    std::shared_ptr<scene2::SpriteNode> _explodeSprite;
    std::shared_ptr<scene2::SpriteNode> _explodeVFXSprite;

public:

#pragma mark Hidden Constructors
    /**
     * Creates a degenerate enemy object.
     *
     * This constructor does not initialize any of the enemy values beyond
     * the defaults.  To use a PlayerModel, you must call init().
     */
    Boss1Model() : EnemyModel(), _sensorName(ENEMY_SENSOR_NAME), _bodyName(ENEMY_BODY_NAME) {}

    /**
     * Destroys this PlayerModel, releasing all resources.
     */
    virtual ~Boss1Model(void) override { dispose(); }

    /**
     * Disposes all resources and assets of this PlayerModel
     *
     * Any assets owned by this object will be immediately released.  Once
     * disposed, a PlayerModel may not be used until it is initialized again.
     */
    void dispose() override;

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
    virtual bool init(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& constantsRef, const Vec2& pos, std::vector<std::shared_ptr<ActionModel>> actions) override;


#pragma mark -
#pragma mark Static Constructors
    /**
     * Creates a new enemy at the given position.
     *
     * The enemy is sized according to the given drawing scale.
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param size  The size of the enemy in world units
     *
     * @return  A newly allocated PlayerModel at the given position with the given scale
     */
    static std::shared_ptr<Boss1Model> alloc(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& constantsRef, const Vec2& pos, std::vector<std::shared_ptr<ActionModel>> actions) {
        std::shared_ptr<Boss1Model> result = std::make_shared<Boss1Model>();
        //result->_scale = Application::get()->getDisplayWidth() / 1248;
        return (result->init(assetRef, constantsRef, pos, actions) ? result : nullptr);
    }

#pragma mark -
#pragma mark Level Control and Constructor Helpers
    /** Reset all the enemy attributes to their initial values*/
    void resetAttributes() override {
        _hp = 200;
        _isGrounded = false;
        _isMoveLeft = false;
        _isMoveRight = false;
        _faceRight = true;
        _canKnockBack = true;
        _stunRem = 0;

        _isStabbing = false;
        _isSlamming = false;
		_isShooting = false;
		_isExploding = false;

        _moveDuration = 0;
        currentFrame = 0;
    };

    /**Attach the scene nodes (sprite sheets) to the enemy**/
    void attachNodes(const std::shared_ptr<AssetManager>& assetRef) override;
#pragma mark -
#pragma mark AI Methods
    void nextAction() override;
    void AIMove() override;

    /**
     * Performs the slam attack of boss1
     *
     */
    void slam();

    /**
     * Performs the stab attack of boss1
     *
     */
    void stab();

    /**
     * Performs the shoot attack of boss1
     *
     */
    void shoot();

    /**
     * Performs the explode attack of boss1
     *
     */
    void explode();

    /**
     * Returns the action when an attack hitbox should be active, or nothing when no attack is active
     *
     * @return the action that needs hitbox, or nullptr when no hitbox is active
     */
    std::shared_ptr<MeleeActionModel> getDamagingAction() override;

    /**
     * Returns the action when a projectile is going to be shot, or nothing when no attack is active
     *
     * @return the action that needs projectile, or nullptr when no ranged attack is active
     */
    std::shared_ptr<RangedActionModel> getProjectileAction() override;

#pragma mark -
#pragma mark Animation Methods
    void updateAnimation() override;

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




};

#endif /* __GB_BOSS1_MODEL_H__ */