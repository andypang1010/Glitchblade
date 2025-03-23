//
//  PlayerController.h
//

#ifndef __PLAYER_CONTROLLER_H__
#define __PLAYER_CONTROLLER_H__

#include <cugl/cugl.h>
#include "GBPlayerModel.h"
#include "GBProjectile.h"
#include "GBInput.h"

using namespace cugl;

/**
 * This class defines the logic that manipulates the PlayerModel.
 *
 * It translates input signals into actual player actions such as movement, jumping, guarding, dashing, and projectile interactions.
 */
class PlayerController {
private:
    /** Reference to the player model */
    std::shared_ptr<PlayerModel> _player;
    std::shared_ptr<scene2::Label> _hpNode;
    /** Controller for abstracting out input across multiple platforms */
    std::shared_ptr<PlatformInput> _input;
    /** Data */
    std::shared_ptr<AssetManager> _assets;
    std::shared_ptr<JsonValue> _constantsJSON;
    
public:
    
    /** Constructor */
    PlayerController();

    /** Destructor */
    ~PlayerController() { dispose(); }

    /**
     * Initializes the player controller.
     */
    void init(const std::shared_ptr<AssetManager>& assetRef,  const std::shared_ptr<JsonValue>& constantsRef);

    /**
     * Disposes of all (non-static) resources allocated to this controller.
     */
    void dispose();

    /**
     * Resets the status of the PlayerController so that we can play again.
     */
    void reset();
    
    /** Apply force to the player model*/
    void applyForce();

    /**
     * Updates the player's state based on inputs.
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
        Helper for preUpdate
     */
    void updateCooldowns();
    /**
     * The method called to indicate the end of a deterministic loop.
     *
     * @param remain    The amount of time (in seconds) last fixedUpdate
     */
    void postUpdate(float dt);

    /**
     * Activates the player’s shield.
     */
    void activateShield();

    /**
     * @brief 
     * 
     */
    void deactivateShield();

    /**
     * Fires a projectile in the direction the player is facing.
     */
    void fireProjectile();

    /**
     * Deflects a projectile using the shield.
     */
    void deflectProjectile();
    
    #pragma mark Getters
    std::shared_ptr<PlayerModel> getPlayer() {return _player;};
    //can remove this once no longer need input for debugging.
    std::shared_ptr<PlatformInput> getInputController() {return _input;};
};


#endif /* __PLAYER_CONTROLLER_H__ */
