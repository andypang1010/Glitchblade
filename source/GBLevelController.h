//
//  GBLevelController.h
//

#ifndef __GB_LEVEL_CONTROLLER_H__
#define __GB_LEVEL_CONTROLLER_H__
#include "GBLevelModel.h"
#include "GBEnemyController.h"
#include "GBPlayerController.h"
#include <cugl/cugl.h>

using namespace cugl;

/**
 * This class controls the levels for Glitchblade.
 *
 * This controller manages the level progress of the current game. It handles the game win/lose conditions by reading the LevelModel.
 * It also instantiates the HUD nodes and communicates information to them for real-time updates.
 */
class LevelController {
private:
    /** A reference to the level model */
    std::shared_ptr<LevelModel> _levelModel;

    // TODO: Need to add/store HUD node somewhere

    /* Controllers */

    /** The enemy controller for this level controller */
    std::shared_ptr<EnemyController> _enemyController;

    /** The player controller for this level controller */
    std::shared_ptr<PlayerController> _playerController;

protected:
    /**
     * Check if the game win conditions have been met.
     */
    void checkWinCondition();

    /**
     * Check if the game lose conditions have been met.
     */
    void checkLoseCondition();

public:
    /**
     * Constructor.
     */
    LevelController();

    /**
     * Destructor.
     */
    ~LevelController();

    /**
     * Initializes the level controller.
     */
    void init();

    /**
     * Updates the level controller each frame.
     *
     * @param timestep The time elapsed since the last update.
     */
    void update(float timestep);

    /**
     * Starts the level.
     */
    void start();
};

#endif /* __GB_LEVEL_CONTROLLER_H__ */