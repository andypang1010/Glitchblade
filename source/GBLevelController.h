#ifndef LEVEL_CONTROLLER_H
#define LEVEL_CONTROLLER_H

#include <string>
#include <vector>
#include <cugl/cugl.h>
#include <iostream>

#include "GBActionModel.h"
#include "GBMeleeActionModel.h"
#include "GBRangedActionModel.h"
#include "GBMovementActionModel.h"

/**
 * A class that parses a JSON level file and generates corresponding action models.
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
    /** Parses the JSON file and returns a vector of parsed actions. */
    static std::vector<std::shared_ptr<ActionModel>> parseActions(const std::shared_ptr<JsonValue>& json, const std::string enemyName);
};

#endif // LEVEL_CONTROLLER_H