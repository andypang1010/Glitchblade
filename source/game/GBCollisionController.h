//
//  GBCollisionController.h
//  gb-mac
//
//  Created by Sasha Loayza on 4/9/25.
//

#ifndef GBCollision_h
#define GBCollision_h

#include <stdio.h>
#include "../player/GBPlayerModel.h"
#include "../enemies/GBEnemyModel.h"
#include "ui/GBIngameUI.h"
#include "ui/GBPauseMenu.h"
#include <box2d/b2_contact.h>
#include <box2d/b2_collision.h>
#include "../CUHaptics.h"

using namespace cugl;
using namespace cugl::physics2;

class CollisionController {
public:
    // Callback types
    using RemoveProjectileCallback = std::function<void(Projectile*)>;
    using ScreenShakeCallback = std::function<void(int intensity, int duration)>;
private:
    std::shared_ptr<PlayerModel> _player;
    std::shared_ptr<GBIngameUI> _ui;
    std::shared_ptr<GBPauseMenu> _pauseMenu;
    std::shared_ptr<JsonValue> _constantsJSON;
    // captured lambdase from GameScene
    RemoveProjectileCallback _removeProjectile;
    ScreenShakeCallback _screenShake;
    /** Mark set to handle more sophisticated collision callbacks */
    std::unordered_set<b2Fixture*> _sensorFixtures;
    
#pragma mark collision case helpers
    void playerEnemyCollision(Obstacle* enemyObstacle);
    void playerHitboxCollision(Obstacle* hitboxObstacle);
    void playerProjectileCollision(Obstacle* projectileObstacle);
    void enemyProjectileCollision(Obstacle* enemyObstacle, Obstacle* projectileObstacle);
#pragma mark collision helpers
    /**
     *
     * This function returns true if the provided Obstacle b is an enemy body
     * and if the fixture name `f` matches the enemy's body name.
     *
     * @param b Pointer to the Obstacle  being checked.
     * @param f Pointer to the fixture's user data (string).
     * @return True if the fixture and body correspond to an enemy body, false otherwise.
     */
    bool isEnemyBody(physics2::Obstacle* b, std::string f);
    /**
     *
     * This function returns true if the provided Obstacle b is the player body
     * and if the fixture name `f` matches the player's body name.
     *
     * @param b Pointer to the Obstacle  being checked.
     * @param f Pointer to the fixture's user data (string).
     * @return True if the fixture and body correspond to the player body, false otherwise.
     */
    bool isPlayerBody(physics2::Obstacle* b,const std::string* f);
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


public:
    // Constructor
    CollisionController(
        std::shared_ptr<PlayerModel> player,
        std::shared_ptr<GBIngameUI> ui,
        std::shared_ptr<GBPauseMenu> pauseMenu,
        std::shared_ptr<JsonValue> constantsJSON,
        RemoveProjectileCallback removeProjectile,
        ScreenShakeCallback screenShake
    );
    
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
    
    /** Reset the collision controller (eg for a new level) */
    void reset();


};









#endif /* GBCollisionController_h */
