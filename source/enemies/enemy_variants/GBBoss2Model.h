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
#ifndef __GB_BOSS2_MODEL_H__
#define __GB_BOSS2_MODEL_H__
#include <cugl/cugl.h>
#include "../GBEnemyModel.h"
#include "../GBActionModel.h"
#include "../actionmodel_variants/GBMeleeActionModel.h"
#include "../actionmodel_variants/GBRangedActionModel.h"

using namespace cugl;

#pragma mark -
#pragma mark Action Constants // TODO: Refactor with Action parser
#define SHOOT_FRAMES    5
#define STUN_FRAMES 88

#pragma mark -
#pragma mark Enemy Model
/**
* Player avatar for the plaform game.
*
* Note that this class uses a capsule shape, not a rectangular shape.  In our
* experience, using a rectangular shape for a character will regularly snag
* on a platform.  The round shapes on the end caps lead to smoother movement.
*/
class Boss2Model : public EnemyModel {
private:
    /** This macro disables the copy constructor (not allowed on physics objects) */
    CU_DISALLOW_COPY_AND_ASSIGN(Boss2Model);
protected:
    std::string _sensorName;
    std::string _bodyName;

    bool _isShortFireStarting;
    bool _isShortFireAttacking;
    bool _isShortFireWaiting;
    bool _isShortFireEnding;
    bool _isShortFiring;
    int _shortFireCount;

    bool _isHeadFireStarting;
    bool _isHeadFireAttacking;
    bool _isHeadFireWaiting;
    bool _isHeadFireEnding;
    bool _isHeadFiring;
    int _headFireCount;

	int _headFireFallCount;
    int _headFireTimer;

    bool _isTeleportStarting;
    bool _isTeleportEnding;
    int _teleportCD;

    bool _isShootingLaser;

    std::shared_ptr<RangedActionModel> _shortFire;
	std::shared_ptr<RangedActionModel> _headFire;
    std::shared_ptr<RangedActionModel> _headFireFall;
	std::shared_ptr<MeleeActionModel> _laser;

    /**
    * Redraws the outline of the physics fixtures to the debug node
    *
    * The debug node is use to outline the fixtures attached to this object.
    * This is very useful when the fixtures have a very different shape than
    * the texture (e.g. a circular shape attached to a square texture).
    */
    virtual void resetDebug() override;

public:
    std::shared_ptr<scene2::SpriteNode> _shortFireStartSprite;
    std::shared_ptr<scene2::SpriteNode> _shortFireAttackSprite;
    std::shared_ptr<scene2::SpriteNode> _shortFireWaitSprite;
    std::shared_ptr<scene2::SpriteNode> _shortFireEndSprite;

    std::shared_ptr<scene2::SpriteNode> _headFireStartSprite;
    std::shared_ptr<scene2::SpriteNode> _headFireAttackSprite;
    std::shared_ptr<scene2::SpriteNode> _headFireWaitSprite;
    std::shared_ptr<scene2::SpriteNode> _headFireEndSprite;

	std::shared_ptr<scene2::SpriteNode> _teleportStartSprite;
	std::shared_ptr<scene2::SpriteNode> _teleportEndSprite;

    std::shared_ptr<scene2::SpriteNode> _laserSprite;
    std::shared_ptr<scene2::SpriteNode> _laserVFXSprite;

public:

#pragma mark Hidden Constructors
    /**
     * Creates a degenerate enemy object.
     *
     * This constructor does not initialize any of the enemy values beyond
     * the defaults.  To use a PlayerModel, you must call init().
     */
    Boss2Model() : EnemyModel(), _sensorName(ENEMY_SENSOR_NAME) {}

    /**
     * Destroys this PlayerModel, releasing all resources.
     */
    virtual ~Boss2Model(void) override { dispose(); }

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
    virtual bool init(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& enemyJSON, const Vec2& pos, std::vector<std::shared_ptr<ActionModel>> actions) override;


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
    static std::shared_ptr<Boss2Model> alloc(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& enemyJSON, const Vec2& pos, std::vector<std::shared_ptr<ActionModel>> actions) {
        std::shared_ptr<Boss2Model> result = std::make_shared<Boss2Model>();
        //result->_scale = Application::get()->getDisplayWidth() / 1248;
        return (result->init(assetRef, enemyJSON, pos, actions) ? result : nullptr);
    }

#pragma mark -
#pragma mark Attribute Properties
    void damage(float value) override;

	void setTeleportCD(int value) {
		_teleportCD = value;
	}

	int getTeleportCD() {
		return _teleportCD;
	}

	void setHeadFireTimer(int value) {
		_headFireTimer = value;
	}

	int getHeadFireTimer() {
		return _headFireTimer;
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

        _isShortFireAttacking = false;
        _isShortFireWaiting = false;
        _isShortFireStarting = false;
        _isShortFireEnding = false;
        _isShortFiring = false;
        _shortFireCount = 0;

		_isTeleportStarting = false;
		_isTeleportEnding = false;

		_isShootingLaser = false;

        _isHeadFireAttacking = false;
        _isHeadFireWaiting = false;
        _isHeadFireStarting = false;
        _isHeadFireEnding = false;
        _isHeadFiring = false;
        _headFireCount = 0;
        _headFireFallCount = 0;

        _moveDuration = 0;
    };

    /**Attach the scene nodes (sprite sheets) to the enemy**/
    void attachNodes(const std::shared_ptr<AssetManager>& assetRef) override;
    void setActions(std::vector<std::shared_ptr<ActionModel>> actions) override;
#pragma mark -
#pragma mark AI Methods
    void nextAction() override;
    void AIMove() override;

    /**
     * Performs the shoot attack of boss1
     *
     */
    void shortFire(int repeat);

    void handleShortFire();

	void teleport();

    void headFire(int repeat);

    void handleHeadFire();

    void laser();

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
    std::shared_ptr<Projectile> getProjectile() override;

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
