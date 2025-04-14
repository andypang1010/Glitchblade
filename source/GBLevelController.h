#ifndef __GB__LEVEL_CONTROLLER_H
#define __GB__LEVEL_CONTROLLER_H

#include <string>
#include <vector>
#include <cugl/cugl.h>
#include <iostream>

#include "GBActionModel.h"
#include "GBMeleeActionModel.h"
#include "GBRangedActionModel.h"
#include "GBMovementActionModel.h"
#include "GBLevelModel.h"
#include "GBEnemyController.h"
#include "GBPlayerController.h"
#include "GBTypes.h"

#define MAX_NUM_ENEMIES 2

using namespace cugl;
/**
 * A class that parses a JSON level file and generates corresponding action models.
 */
class LevelController {
private:
    /** A reference to the level model */
    std::shared_ptr<LevelModel> _levelModel;

	std::vector<std::shared_ptr<LevelModel>> _levels;
	int _currentLevelIndex;
	int _currentWaveIndex;
    int _currentEnemyIndex;
    int _numEnemiesActive;
    float _lastSpawnedInterval;

    // TODO: Need to add/store HUD node somewhere

    /* Data */
    std::shared_ptr<AssetManager> _assets;
    std::shared_ptr<JsonValue> _enemiesJSON;
    std::shared_ptr<JsonValue> _constantsJSON;
    std::shared_ptr<JsonValue> _levelsJSON;
    cugl::Rect* _bounds;
    float _scale;
    /* Controllers */

    /** One enemy controller for this level controller: It will likely need to be a vector for future levels*/
    std::shared_ptr<EnemyController> _testEnemyController;

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
     * Initializes the level controller. Return false on failure
     */
    bool init(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& constantsRef);
    
    /**
     * Resets the state of the LevelController.
     */
    
    /**
     Initializes static level obstacles with scene nodes to be added to the Game Scene
     @returns a vector of all the obstacle shared pointers.
     */
    ObstacleNodePairs createStaticObstacles(const std::shared_ptr<AssetManager>& assetRef);
    
    ObstacleNodePairs populateLevel(std::string levelName);

    /**
     * Resets the state of the LevelController.
     */
    void reset();

    /**
     * Updates the level controller each frame.
     *
     * @param timestep The time elapsed since the last update.
     */
    void fixedUpdate(float timestep);

    /**
     * Starts the level.
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

    /** Parses the JSON file and returns a vector of parsed actions. */
    static std::vector<std::shared_ptr<ActionModel>> parseActions(const std::shared_ptr<JsonValue>& json, const std::string enemyName);
    /** Parses the JSON file and returns a vector of parsed actions. */
    static std::vector<std::shared_ptr<LevelModel>> parseLevels(const std::shared_ptr<JsonValue>& json);
    /** Parses the JSON file and returns a vector of parsed actions. */
    static std::shared_ptr<LevelModel> parseLevel(const std::shared_ptr<JsonValue>& json);
    
    std::vector<std::shared_ptr<LevelModel>> getLevels() const { return _levels; }

#pragma mark Getters
    // this is a test method because we will need to access all enemies in the level not just one
    std::shared_ptr<EnemyModel> getTestEnemyModel() { return _testEnemyController->getEnemy(); };
    std::shared_ptr<cugl::scene2::SceneNode> getTestEnemyNode() { return _testEnemyController->getEnemy()->getSceneNode(); };
    std::shared_ptr<PlayerModel> getPlayerModel() { return _playerController->getPlayer(); };
    std::shared_ptr<cugl::scene2::SceneNode> getPlayerNode() { return _playerController->getPlayer()->getSceneNode(); };
    std::shared_ptr<PlatformInput> getInputController() { return _playerController->getInputController(); };
#pragma mark level obstacle data
    void setStaticPhysics(const std::shared_ptr<physics2::Obstacle>& obj);
    std::vector<std::vector<Vec2>>  calculateWallVertices();
    std::vector<Vec2> calculateGroundVertices();
    Vec2 calculateLeftBulletPosition();
    Vec2 calculateRightBulletPosition();
};


#endif /* LEVEL_CONTROLLER_H */
