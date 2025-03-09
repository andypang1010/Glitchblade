//
//  PlayerController.h
//

#ifndef __PLAYER_CONTROLLER_H__
#define __PLAYER_CONTROLLER_H__ller

#include <cugl/cugl.h>
#include "GBPlayerModel.h"
#include "GBProjectile.h"

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

public:
    /** Constructor */
    PlayerController();

    /** Destructor */
    ~PlayerController();

    /**
     * Initializes the player controller.
     */
    void init();

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
     * Deactivates the player’s shield.
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
};

#endif /* __PLAYER_CONTROLLER_H__ */
