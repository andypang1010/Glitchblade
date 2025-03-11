#ifndef COLLISION_CONTROLLER_H
#define COLLISION_CONTROLLER_H

#include <string>
#include <vector>
#include <cugl/cugl.h>
#include <box2d/b2_world_callbacks.h>
#include <box2d/b2_fixture.h>
#include "GBProjectile.h"
#include "GBPlayerModel.h"
#include "GBEnemyModel.h"

using namespace cugl;

#define PLAYER_NAME     "player"
#define ENEMY_NAME      "enemy"
#define PROJECTILE_NAME "projectile"
#define SHIELD_NAME     "shield"
#define GROUND_NAME     "ground"
#define GROUND_SENSOR_NAME     "sensor"

/**
 * A class that parses a JSON level file and generates corresponding action models.
 */
class CollisionController {
protected:

    /**
    * Processes the start of a collision
    *
    * This method is called when we first get a collision between two objects.  We use
    * this method to test if it is the "right" kind of collision.  In particular, we
    * use it to test if we make it to the win door.  We also us it to eliminate bullets.
    *
    * @param  contact  The two bodies that collided
    */
    void beginContact(b2Contact* contact);

    /**
    * Processes the end of a collision
    *
    * This method is called when we no longer have a collision between two objects.
    * We use this method allow the character to jump again.
    *
    * @param  contact  The two bodies that collided
    */
    void endContact(b2Contact* contact);

    /**
     *
     * This function returns true if the provided Obstacle b is an enemy body
     * and if the fixture name `f` matches the enemy's body name.
     *
     * @param b Pointer to the Obstacle  being checked.
     * @return True if the body correspond to an enemy body, false otherwise.
     */
    bool isEnemyBody(physics2::Obstacle* b);
    /**
     *
     * This function returns true if the provided Obstacle b is the player body
     * and if the fixture name `f` matches the player's body name.
     *
     * @param b Pointer to the Obstacle  being checked.
     * @param f Pointer to the fixture's user data (string).
     * @return True if the fixture and body correspond to the player body, false otherwise.
     */
    bool isPlayerBody(physics2::Obstacle* b, const std::string* f);
    /**
     * @brief Checks if a projectile is hitting the player's shield.
     *
     * This function examines two possible (body, fixture) pairs to determine if a projectile
     * has collided with the player's shield. It returns a pointer to the projectile if a valid
     * collision is detected, otherwise returns nullptr.
     *
     * @param bd1 The first body involved in the contact.
     * @param fd1 The user data of the first fixture.
     * @param bd2 The second body involved in the contact.
     * @param fd2 The user data of the second fixture.
     * @return A pointer to the projectile that hit the shield, or nullptr if no such collision occurred.
     *
     * @note The function ensures that the projectile was not fired by the player and that the player's
     * shield is active before confirming a valid collision.
     */
    Projectile* getProjectileHitShield(physics2::Obstacle* bd1, std::string* fd1,
        physics2::Obstacle* bd2, std::string* fd2) const;

    /**
 Checks if contact is projectile hitting player shield and returns the Projectile if so, else NULL.
 */
    Projectile* getEnemyHitShield(physics2::Obstacle* bd1, std::string* fd1,
        physics2::Obstacle* bd2, std::string* fd2) const;

    /** Mark set to handle more sophisticated collision callbacks */
    std::unordered_set<b2Fixture*> _sensorFixtures;

public:
    /** The Box2D world */
    std::shared_ptr<physics2::ObstacleWorld> _world;

    /**
     * Constructor.
     */
    CollisionController();

    /**
     * Destructor.
     */
    ~CollisionController();

    /**
     * Initializes the collision controller.
     */
    void init();

    /**
     * Updates the level controller each frame.
     *
     * @param timestep The time elapsed since the last update.
     */
    void fixedUpdate(float timestep);
};

#endif // COLLISION_CONTROLLER_H