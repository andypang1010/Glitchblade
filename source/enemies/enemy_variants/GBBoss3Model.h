#ifndef __GB_BOSS3_MODEL_H__
#define __GB_BOSS3_MODEL_H__
#include <cugl/cugl.h>
#include "../GBEnemyModel.h"
#include "../GBActionModel.h"
#include "../actionmodel_variants/GBMeleeActionModel.h"
#include "../actionmodel_variants/GBRangedActionModel.h"

using namespace cugl;

#pragma mark -
#pragma mark Action Constants // TODO: Refactor with Action parser
#define SHOOT_FRAMES    5
#define STUN_FRAMES 53

#pragma mark -
#pragma mark Enemy Model
/**
* Player avatar for the plaform game.
*
* Note that this class uses a capsule shape, not a rectangular shape.  In our
* experience, using a rectangular shape for a character will regularly snag
* on a platform.  The round shapes on the end caps lead to smoother movement.
*/
class Boss3Model : public EnemyModel {
private:
    /** This macro disables the copy constructor (not allowed on physics objects) */
    CU_DISALLOW_COPY_AND_ASSIGN(Boss3Model);
protected:
    std::string _sensorName;
    std::string _bodyName;

    float _verticalMovement;
    float _worldTop = 18.0f;

    bool _isGroundForm;
    int _stunMeter;
	int actualStunFrames = 53*4;

    bool _isShootStarting;
    bool _isShootAttacking;
    bool _isLaserAttacking;
    bool _isShootWaiting;
    int _shootCount;

    bool _isUppercutting;
    bool _isSlamming;
	bool _isJumping;
	bool _isDashing;

    // Ground form actions
    std::shared_ptr<MeleeActionModel> _uppercut;
	std::shared_ptr<MeleeActionModel> _slam;
    std::shared_ptr<MeleeActionModel> _jump;
    std::shared_ptr<MeleeActionModel> _dash;

    // Air form actions
    std::shared_ptr<RangedActionModel> _shoot;
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
    std::shared_ptr<scene2::SpriteNode> _groundIdleSprite;
    std::shared_ptr<scene2::SpriteNode> _airIdleSprite;

	std::shared_ptr<scene2::SpriteNode> _groundStunSprite;
	std::shared_ptr<scene2::SpriteNode> _airStunSprite;

	std::shared_ptr<scene2::SpriteNode> _groundDeadSprite;
	std::shared_ptr<scene2::SpriteNode> _airDeadSprite;

    std::shared_ptr<scene2::SpriteNode> _shootStartSprite;
    std::shared_ptr<scene2::SpriteNode> _shootAttackSprite;
    std::shared_ptr<scene2::SpriteNode> _shootLaserSprite;
    std::shared_ptr<scene2::SpriteNode> _shootWaitSprite;

    std::shared_ptr<scene2::SpriteNode> _uppercutSprite;
    std::shared_ptr<scene2::SpriteNode> _slamSprite;
    std::shared_ptr<scene2::SpriteNode> _jumpSprite;
    std::shared_ptr<scene2::SpriteNode> _dashSprite;

    std::shared_ptr<scene2::SpriteNode> _groundToAirSprite;
	std::shared_ptr<scene2::SpriteNode> _airToGroundSprite;

    std::shared_ptr<scene2::SpriteNode> _laserVFXSprite;

public:

#pragma mark Hidden Constructors
    /**
     * Creates a degenerate enemy object.
     *
     * This constructor does not initialize any of the enemy values beyond
     * the defaults.  To use a PlayerModel, you must call init().
     */
    Boss3Model() : EnemyModel(), _sensorName(ENEMY_SENSOR_NAME) {}

    /**
     * Destroys this PlayerModel, releasing all resources.
     */
    virtual ~Boss3Model(void) override { dispose(); }

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
    static std::shared_ptr<Boss3Model> alloc(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& enemyJSON, const Vec2& pos, std::vector<std::shared_ptr<ActionModel>> actions) {
        std::shared_ptr<Boss3Model> result = std::make_shared<Boss3Model>();
        //result->_scale = Application::get()->getDisplayWidth() / 1248;
        return (result->init(assetRef, enemyJSON, pos, actions) ? result : nullptr);
    }

#pragma mark -
#pragma mark Attribute Properties
    void damage(float value) override;

    void setStun(int value) override;

    virtual float getVerticalMovement() const { return _verticalMovement; }

    virtual void setVerticalMovement(float value) { _verticalMovement = value; }

	bool getIsGroundForm() {
		return _isGroundForm;
	}

	bool getIsJumping() {
		return _isJumping;
	}

	bool getIsDashing() {
		return _isDashing;
	}

    bool isJumpingUp();

#pragma mark -
#pragma mark Level Control and Constructor Helpers
    /** Reset all the enemy attributes to their initial values*/
    void resetAttributes() override {
        _hp = 500;
        _aggression = 0;
        _isGrounded = false;
        _isMoveLeft = false;
        _isMoveRight = false;
        _faceRight = true;
        _canKnockBack = true;
        _stunRem = 0;

		_isGroundForm = true;
		_isShootStarting = false;
		_isShootAttacking = false;
		_isLaserAttacking = false;
		_isShootWaiting = false;
		_isUppercutting = false;
		_isSlamming = false;
		_isJumping = false;
		_isDashing = false;
		_shootCount = 0;

        _moveDuration = 0;
    };

    /**Attach the scene nodes (sprite sheets) to the enemy**/
    void attachNodes(const std::shared_ptr<AssetManager>& assetRef) override;
    void setActions(std::vector<std::shared_ptr<ActionModel>> actions) override;
#pragma mark -
#pragma mark AI Methods
    void nextAction() override;
    void AIMove() override;

    void handleGroundAction(int r);
	void handleAirAction(int r);

    void uppercut();
    void slam();
    void jump();
    void dash();

    void formChange();

    void laser();
    /**
     * Performs the shoot attack of boss3
     *
     */
    void shoot(int repeat);
    void handleShoot();

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

    void die(std::shared_ptr<scene2::SceneNode> world) override;

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

#endif /* __GB_BOSS3_MODEL_H__ */
