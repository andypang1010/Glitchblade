//
//  EnemyController.h
//

#ifndef __ENEMY_CONTROLLER_H__
#define __ENEMY_CONTROLLER_H__

#include <cugl/cugl.h>
#include "GBEnemyModel.h"
#include "GBProjectile.h"

using namespace cugl;

/**
 * This class controls all logical updates to EnemyModels.
 *
 * It manages AI decision-making, enemy movement, attacks, and removal of defeated enemies from the game world.
 */
class EnemyController {
private:
    /** The enemy corresponding to this controller  */
    std::shared_ptr<EnemyModel> _enemy;
    std::shared_ptr<scene2::Label> _hpNode;
    std::shared_ptr<scene2::Label> _stunNode;

    std::shared_ptr<JsonValue> _enemyJSON;

public:
    /** Constructor */
    EnemyController();

    /** Destructor */
    ~EnemyController() { dispose(); }

    /**
     * Initializes the enemy controller.
     */
    void init(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& constantsRef, std::vector<std::shared_ptr<ActionModel>> actions);


    /**
     * Disposes of all (non-static) resources allocated to this controller.
     */
    void dispose();

    /**
     * Resets the EnemyController.
     */
    void reset();

    /** Apply force to the enemy model*/
    void applyForce();

    /**
     * Updates all enemy behaviors.
     * @param timestep The time elapsed since the last update.
     */
    void fixedUpdate(float timestep);

    /**
     * The method called to indicate the start of a deterministic loop.
     *
     * @param dt    The amount of time (in seconds) since the last frame
     */
    void preUpdate(float dt);

    /**
     * The method called to indicate the end of a deterministic loop.
     *
     * @param remain    The amount of time (in seconds) last fixedUpdate
     */
    void postUpdate(float dt);

    std::shared_ptr<EnemyModel> getEnemy() { return _enemy; };

    //  Maybe for the level controller, this will be useful:
    //    /**
    //     * Adds an enemy to the controller for management.
    //     * @param enemy The enemy to add.
    //     */
    //    void addEnemy(const std::shared_ptr<EnemyModel>& enemy) { _enemies.push_back(enemy); }
    //
    //    /**
    //     * Removes a defeated enemy from the game.
    //     * @param enemy The enemy to remove.
    //     */
    //    void removeEnemy(const std::shared_ptr<EnemyModel>& enemy);
    //
    //    /**
    //     * Clears all enemies from the controller.
    //     */
    //    void clearEnemies() { _enemies.clear(); }
};

#endif /* __ENEMY_CONTROLLER_H__ */