#ifndef __GB_MINION1B_MODEL_H__
#define __GB_MINION1B_MODEL_H__
#include <cugl/cugl.h>
#include "../GBEnemyModel.h"
#include "../GBActionModel.h"
#include "../actionmodel_variants/GBMeleeActionModel.h"
#include "../actionmodel_variants/GBRangedActionModel.h"

using namespace cugl;

#define MINION1B_PUNCH_FORCE       35.0f

#pragma mark -
#pragma mark Enemy Model
/**
* Player avatar for the plaform game.
*
* Note that this class uses a capsule shape, not a rectangular shape.  In our
* experience, using a rectangular shape for a character will regularly snag
* on a platform.  The round shapes on the end caps lead to smoother movement.
*/
class Minion1BModel : public EnemyModel {
private:
    /** This macro disables the copy constructor (not allowed on physics objects) */
    CU_DISALLOW_COPY_AND_ASSIGN(Minion1BModel);
protected:
    std::string _sensorName;
    std::string _bodyName;

    bool _isPunching;
    bool _isSlamming;

    std::shared_ptr<MeleeActionModel> _punch;
    std::shared_ptr<MeleeActionModel> _slam;

    /**
    * Redraws the outline of the physics fixtures to the debug node
    *
    * The debug node is use to outline the fixtures attached to this object.
    * This is very useful when the fixtures have a very different shape than
    * the texture (e.g. a circular shape attached to a square texture).
    */
    virtual void resetDebug() override;

public:
    std::shared_ptr<scene2::SpriteNode> _punchSprite;
    std::shared_ptr<scene2::SpriteNode> _slamSprite;
	std::shared_ptr<scene2::SpriteNode> _slamVFXSprite;

public:

#pragma mark Hidden Constructors
    /**
     * Creates a degenerate enemy object.
     *
     * This constructor does not initialize any of the enemy values beyond
     * the defaults.  To use a PlayerModel, you must call init().
     */
    Minion1BModel() : EnemyModel(), _sensorName(ENEMY_SENSOR_NAME), _bodyName(ENEMY_BODY_NAME) {}

    /**
     * Destroys this PlayerModel, releasing all resources.
     */
    virtual ~Minion1BModel(void) override { dispose(); }

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
    static std::shared_ptr<Minion1BModel> alloc(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& minion1BJSON, const Vec2& pos, std::vector<std::shared_ptr<ActionModel>> actions) {
        std::shared_ptr<Minion1BModel> result = std::make_shared<Minion1BModel>();
        return (result->init(assetRef, minion1BJSON, pos, actions) ? result : nullptr);
    }

#pragma mark -
#pragma mark Level Control and Constructor Helpers
    /** Reset all the enemy attributes to their initial values*/
    void resetAttributes() override {
        _hp = 50;
        _isGrounded = false;
        _isMoveLeft = false;
        _isMoveRight = false;
        _faceRight = true;
        _canKnockBack = true;
        _stunRem = 0;

        _isPunching = false;
        _isSlamming = false;

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
     * Performs the slam attack of minion1B
     *
     */
    void slam();

    /**
     * Performs the punch attack of minion1B
     *
     */
    void punch();

    /**
     * Returns the action when an attack hitbox should be active, or nothing when no attack is active
     *
     * @return the action that needs hitbox, or nullptr when no hitbox is active
     */
    std::shared_ptr<MeleeActionModel> getDamagingAction() override;

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

#endif /* __GB_MINION1B_MODEL_H__ */
