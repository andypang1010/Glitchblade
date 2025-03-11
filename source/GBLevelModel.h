//
//  GBLevelModel.h
//

#ifndef __GB_LEVEL_MODEL_H__
#define __GB_LEVEL_MODEL_H__
#include <cugl/cugl.h>
#include "GBPlayerModel.h"
#include "GBEnemyModel.h"

using namespace cugl;

/**
 * This model holds all states of the current level. This includes the current wave number, the total number of waves, references to 
 * the enemies in the level, and sprites of all environmental objects.
 */
class LevelModel {
private:
    /** The current wave number in the level */
    int _currentWave;

    /** The total number of waves in the level */
    int _totalWaves;

    /** The player's initial state */
    std::shared_ptr<PlayerModel> _playerState;

    /** A list of enemy entities present in the level */
    std::vector<std::shared_ptr<EnemyModel>> _enemies;

    /** Whether the level is complete */
    bool _isComplete;

public:
    /**
     * Constructor.
     */
    LevelModel();

    /**
     * Destructor.
     */
    ~LevelModel();

    /**
     * Initializes the level model with the given parameters.
     */
    void init();

    /**
     * Updates the level model each frame.
     *
     * @param timestep The time elapsed since the last update.
     */
    void update(float timestep);

    /** Getters and Setters **/

    /**
     * Returns the current wave number.
     * @return The current wave.
     */
    int getCurrentWave() const { return _currentWave; }

    /**
     * Sets the current wave number.
     * @param wave The new wave number.
     */
    void setCurrentWave(int wave) { _currentWave = wave; }

    /**
     * Returns the total number of waves.
     * @return The total waves in the level.
     */
    int getTotalWaves() const { return _totalWaves; }

    /**
     * Sets the total number of waves.
     * @param waves The total number of waves.
     */
    void setTotalWaves(int waves) { _totalWaves = waves; }

    /**
     * Returns the player's initial state.
     * @return The player model.
     */
    std::shared_ptr<PlayerModel> getPlayerState() const { return _playerState; }

    /**
     * Sets the player's initial state.
     * @param playerState The player model.
     */
    void setPlayerState(const std::shared_ptr<PlayerModel>& playerState) { _playerState = playerState; }

    /**
     * Returns a reference to the list of enemies in the level.
     * @return The vector of enemy entities.
     */
    const std::vector<std::shared_ptr<EnemyModel>>& getEnemies() const { return _enemies; }

    /**
     * Adds an enemy to the level.
     * @param enemy The enemy entity to add.
     */
    void addEnemy(const std::shared_ptr<EnemyModel>& enemy) { _enemies.push_back(enemy); }

    /**
     * Removes an enemy from the level.
     * @param enemy The enemy entity to remove.
     */
    void removeEnemy(const std::shared_ptr<EnemyModel>& enemy);

    /**
     * Checks if the level is complete.
     * @return true if the level is complete, false otherwise.
     */
    bool isLevelComplete() const { return _isComplete; }

    /**
     * Sets whether the level is complete.
     * @param complete True if the level is complete.
     */
    void setLevelComplete(bool complete) { _isComplete = complete; }
};

#endif /* __GB_LEVEL_MODEL_H__ */
