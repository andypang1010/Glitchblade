#ifndef __GB_MINION1A_MODEL_H__
#define __GB_MINION1A_MODEL_H__
#include <cugl/cugl.h>
#include "../GBEnemyModel.h"
#include "../GBActionModel.h"
#include "../actionmodel_variants/GBMeleeActionModel.h"
#include "../actionmodel_variants/GBRangedActionModel.h"

using namespace cugl;

#pragma mark -
#pragma mark Enemy Model
/**
* Player avatar for the plaform game.
*
* Note that this class uses a capsule shape, not a rectangular shape.  In our
* experience, using a rectangular shape for a character will regularly snag
* on a platform.  The round shapes on the end caps lead to smoother movement.
*/
class Minion1AModel : public EnemyModel {
private:
    /** This macro disables the copy constructor (not allowed on physics objects) */
    CU_DISALLOW_COPY_AND_ASSIGN(Minion1AModel);
protected:
    std::string _sensorName;
    std::string _bodyName;

    bool _isExploding;
    bool _isShooting;

    std::shared_ptr<MeleeActionModel> _explode;
    std::shared_ptr<RangedActionModel> _shoot;

    /**
    * Redraws the outline of the physics fixtures to the debug node
    *
    * The debug node is use to outline the fixtures attached to this object.
    * This is very useful when the fixtures have a very different shape than
    * the texture (e.g. a circular shape attached to a square texture).
    */
    virtual void resetDebug() override;

public:
    std::shared_ptr<scene2::SpriteNode> _explodeSprite;
    std::shared_ptr<scene2::SpriteNode> _explodeVFXSprite;
    std::shared_ptr<scene2::SpriteNode> _shootSprite;

public:

#pragma mark Hidden Constructors
    /**
     * Creates a degenerate enemy object.
     *
     * This constructor does not initialize any of the enemy values beyond
     * the defaults.  To use a PlayerModel, you must call init().
     */
    Minion1AModel() : EnemyModel(), _sensorName(ENEMY_SENSOR_NAME), _bodyName(ENEMY_BODY_NAME) {}

    /**
     * Destroys this PlayerModel, releasing all resources.
     */
    virtual ~Minion1AModel(void) override { dispose(); }

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
    static std::shared_ptr<Minion1AModel> alloc(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& enemiesJSON, const Vec2& pos, std::vector<std::shared_ptr<ActionModel>> actions) {
        std::shared_ptr<Minion1AModel> result = std::make_shared<Minion1AModel>();
        return (result->init(assetRef, enemiesJSON, pos, actions) ? result : nullptr);
    }

#pragma mark -
#pragma mark Level Control and Constructor Helpers
    /** Reset all the enemy attributes to their initial values*/
    void resetAttributes() override {
        EnemyModel::resetAttributes();
        _isExploding = false;
        _isShooting = false;
    };

    /**Attach the scene nodes (sprite sheets) to the enemy**/
    void attachNodes(const std::shared_ptr<AssetManager>& assetRef) override;
    void setActions(std::vector<std::shared_ptr<ActionModel>> actions) override;
#pragma mark -
#pragma mark AI Methods
    void nextAction() override;
    void AIMove() override;

    /**
     * Performs the shoot attack of minion1A
     *
     */
    void shoot();

    /**
     * Performs the explode attack of minion1A
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
