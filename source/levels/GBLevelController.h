#ifndef __GB__LEVEL_CONTROLLER_H
#define __GB__LEVEL_CONTROLLER_H

#include <string>
#include <vector>
#include <cugl/cugl.h>
#include <iostream>

#include "../enemies/GBActionModel.h"
#include "../enemies/actionmodel_variants/GBMeleeActionModel.h"
#include "../enemies/actionmodel_variants/GBRangedActionModel.h"
#include "../enemies/actionmodel_variants/GBMovementActionModel.h"
#include "GBLevelModel.h"
#include "../enemies/GBEnemyController.h"
#include "../player/GBPlayerController.h"
#include "../core/GBTypes.h"
#include "../core/GBHitbox.h"
#include "../game/objects/GBProjectile.h"

#define MAX_NUM_ENEMIES 2

using namespace cugl;
/**
 * A class that parses a JSON level file and generates corresponding action models.
 */
class LevelController {
private:
    /** A reference to the level model */
    std::shared_ptr<LevelModel> _currentLevel;

    // Set the current level to be incomplete by default
    bool _isCurrentLevelComplete = false;

    std::unordered_map<std::string, std::shared_ptr<LevelModel >> _levels;
    std::shared_ptr<cugl::scene2::SceneNode> _worldNode;
    std::shared_ptr<cugl::scene2::SceneNode> _debugNodeRef;
    std::shared_ptr<cugl::physics2::ObstacleWorld> _worldRef;

    int _numEnemiesActive = 0;

	int _currentLevelIndex = 0;
	int _currentWaveIndex = 0;
	int _currentEnemyIndex = 0;
    float _lastSpawnedTime = 0;

    /* Data */
    std::shared_ptr<AssetManager> _assets;
    std::shared_ptr<JsonValue> _enemiesJSON;
    std::shared_ptr<JsonValue> _constantsJSON;
    std::shared_ptr<JsonValue> _levelsJSON;
    cugl::Rect* _bounds;
    float _scale;
    /* Controllers */

	std::vector<std::vector<std::shared_ptr<EnemyController>>> _enemyWaves;

    /** The player controller for this level controller */
    std::shared_ptr<PlayerController> _playerController;
    
public:
	float timeElapsed = 0.0f;


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

    bool isLevelWon() {
        bool levelWon = true;

        if (_currentWaveIndex == _currentLevel->getWaves().size() - 1) {
            for (auto enemyController : _enemyWaves[_currentWaveIndex]) {
                levelWon &= enemyController->getEnemy()->isRemoved() && enemyController->getEnemy()->getHP() <= 0;
            }
        }

        else {
            return false;
        }

        return levelWon;
    }

    bool isLevelLost() {
        return _playerController->getPlayer()->getHP() <= 0;
    }
    
    /**Return a new enemy controller from the enemy name*/
    std::shared_ptr<EnemyController> createEnemy(std::string enemy_name);
    void addEnemy(const std::shared_ptr<EnemyController>& cont);

    void updateLevel();
    void updateWave();
    void spawnLevel();
    bool waveComplete();

    std::shared_ptr<LevelModel> getLevelByName(std::string name) {
        auto it = _levels.find(name);
        if (it != _levels.end()) {
            return it->second;
        }
        else {
            throw std::runtime_error("The level specified: " + name + " does not exist!");
        }
    }

    /**
     * Initializes the level controller. Return false on failure
     */
    bool init(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& constantsRef, const std::shared_ptr<cugl::physics2::ObstacleWorld>& worldRef, const std::shared_ptr<cugl::scene2::SceneNode>& debugNodeRef, const std::shared_ptr<cugl::scene2::SceneNode>& worldNodeRef);

    /**
    * Creates and returns a worldNode for GameScene to use
    */
    std::shared_ptr<cugl::scene2::PolygonNode> makeWorldNode(std::string levelName);
    
    /**
     * Resets the state of the LevelController.
     */
    
    /**
     Initializes static level obstacles with scene nodes to be added to the Game Scene
     @returns a vector of all the obstacle shared pointers.
     */
    void createStaticObstacles(const std::shared_ptr<LevelModel>& levelRef);
    
    /**
     * Adds the physics object to the physics world and loosely couples it to the scene graph
     *
     * @param obstacle_pair the obstacle and it's node
     */
    void addObstacle(ObstacleNodePair obstacle_pair);
    
    
    /**
    * Populates the level.
    */
    void populateLevel(const std::shared_ptr<LevelModel>& level);

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


    /**
    * Adds a new hitbox to the world.
    */
    void createHitbox(std::shared_ptr<EnemyModel> enemy, Vec2 pos, Size size, int damage, float duration);

    /** Parses the JSON file and returns a vector of parsed actions. */
    static std::vector<std::shared_ptr<ActionModel>> parseActions(const std::shared_ptr<JsonValue>& json, const std::string enemyName);
    /** Parses the JSON file and returns a vector of parsed actions. */
    static std::unordered_map<std::string, std::shared_ptr<LevelModel>> parseLevels(const std::shared_ptr<JsonValue>& json, const std::shared_ptr<AssetManager>& assetRef);
    /** Parses the JSON file and returns a vector of parsed actions. */
    static std::shared_ptr<LevelModel> parseLevel(const std::shared_ptr<JsonValue>& json, const std::shared_ptr<AssetManager>& assetRef);

#pragma mark Getters
    // this is a test method because we will need to access all enemies in the level not just one
    // std::shared_ptr<EnemyModel> getTestEnemyModel() { return _testEnemyController->getEnemy(); };
    // std::shared_ptr<cugl::scene2::SceneNode> getTestEnemyNode() { return _testEnemyController->getEnemy()->getSceneNode(); };
    std::shared_ptr<PlayerModel> getPlayerModel() { return _playerController->getPlayer(); };
    std::shared_ptr<cugl::scene2::SceneNode> getPlayerNode() { return _playerController->getPlayer()->getSceneNode(); };
    std::shared_ptr<PlatformInput> getInputController() { return _playerController->getInputController(); };
    std::shared_ptr<LevelModel> getCurrentLevel() {if (_currentLevel == nullptr){throw std::runtime_error("current level is null!");}; return _currentLevel;};
#pragma mark level obstacle data
    void setStaticPhysics(const std::shared_ptr<physics2::Obstacle>& obj);
    std::vector<std::vector<Vec2>>  calculateWallVertices();
    std::vector<Vec2> calculateGroundVertices();
    Vec2 calculateLeftBulletPosition();
    Vec2 calculateRightBulletPosition();
    
    static bool canUpdate(std::shared_ptr<EnemyController> enemyCtrlr){
        return (
            enemyCtrlr != nullptr && 
            enemyCtrlr->getEnemy()->getBody() != nullptr && 
            !enemyCtrlr->getEnemy()->isRemoved() &&
            enemyCtrlr->getEnemy()->getHP() > 0);
        }
    };


#endif /* LEVEL_CONTROLLER_H */
