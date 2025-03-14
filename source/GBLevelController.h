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
#include "GBLevelModel.h"
#include "GBEnemyController.h"
#include "GBPlayerController.h"

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
    void init(cugl::Rect bounds, const std::shared_ptr<AssetManager>& assetRef, float scale);
    
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
    
#pragma mark Getters
    std::shared_ptr<PlayerModel> getPlayerModel(){return _playerController->getPlayer();};
    std::shared_ptr<PlatformInput> getInputController(){return _playerController->getInputController();};
    std::shared_ptr<cugl::scene2::SceneNode> getPlayerNode(){ return _playerController->getPlayer()->getSceneNode();};
    
};

#endif // LEVEL_CONTROLLER_H
