//
//  GBCollisionController.cpp
//  gb-mac
//
//  Created by Sasha Loayza on 4/9/25.
//

#include "GBCollisionController.h"
#include <box2d/b2_contact.h>
#include <box2d/b2_collision.h>
#include "../core/GBHitbox.h"

// Constructor
CollisionController::CollisionController(
     std::shared_ptr<PlayerModel> player,
     std::shared_ptr<GBIngameUI> ui,
     std::shared_ptr<GBPauseMenu> pauseMenu,
     std::shared_ptr<JsonValue> constantsJSON,
     RemoveProjectileCallback removeProjectile,
     ScreenShakeCallback screenShake
) :
    _player(player),
    _ui(ui),
    _pauseMenu(pauseMenu),
    _constantsJSON(constantsJSON),
    _removeProjectile(removeProjectile),
    _screenShake(screenShake){}


/**
 * Processes the start of a collision
 *
 * This method is called when we first get a collision between two objects.  We use
 * this method to test if it is the "right" kind of collision.  In particular, we
 * use it to test if we make it to the win door.
 *
 * @param  contact  The two bodies that collided
 */
void CollisionController::beginContact(b2Contact* contact) {
    std::string proj_name = _constantsJSON->get("projectile")->getString("name");
    std::string enemy_name = _constantsJSON->get("enemy")->getString("name");
    std::string ground_name = _constantsJSON->get("ground")->getString("name");
    std::string wall_name = _constantsJSON->get("walls")->getString("name");
    b2Fixture* fix1 = contact->GetFixtureA();
    b2Fixture* fix2 = contact->GetFixtureB();



    b2Body* body1 = fix1->GetBody();
    b2Body* body2 = fix2->GetBody();

    std::string* fd1 = reinterpret_cast<std::string*>(fix1->GetUserData().pointer);
    std::string* fd2 = reinterpret_cast<std::string*>(fix2->GetUserData().pointer);

    // No idea what this if block is doing; previously was only used for CULogs
    if (!fix1->GetUserData().pointer || !fix2->GetUserData().pointer) {
    }
    else {
    }

    physics2::Obstacle* bd1 = reinterpret_cast<physics2::Obstacle*>(body1->GetUserData().pointer);
    physics2::Obstacle* bd2 = reinterpret_cast<physics2::Obstacle*>(body2->GetUserData().pointer);

    // Player-Enemy Collision
    if (bd1->getName() == enemy_name && isPlayerBody(bd2, fd2)) {
        if (_player->isDashActive() && !_player->isGuardActive()) {
            ((EnemyModel*)bd1)->damage(100);
            _player->setDashRem(0);
            _screenShake(3, 5);
        }
        _player->setKnocked(true, _player->getPosition().subtract(bd1->getPosition()).normalize());
        ((EnemyModel*)bd1)->setKnocked(true, bd1->getPosition().subtract(_player->getPosition()).normalize());
    }
    else if (bd2->getName() == enemy_name && isPlayerBody(bd1, fd1)) {
        if (_player->isDashActive() && !_player->isGuardActive()) {
            ((EnemyModel*)bd2)->damage(100);
            _player->setDashRem(0);
            _screenShake(3, 5);
        }
        _player->setKnocked(true, _player->getPosition().subtract(bd1->getPosition()).normalize());
        ((EnemyModel*)bd2)->setKnocked(true, bd2->getPosition().subtract(_player->getPosition()).normalize());
    }

    // Test: plyaer-hitbox collision
    if (bd1->getName() == "hitbox" && isPlayerBody(bd2, fd2)) {
        if (_player->iframe <= 0) {
            _player->iframe = 60;
            if (!_player->isGuardActive() && !_player->isParryActive()) {
                _player->damage(((Hitbox*)bd1)->getDamage());
                _player->setKnocked(true, _player->getPosition().subtract(((Hitbox*)bd1)->getEnemy()->getPosition()).normalize());
                _screenShake(((Hitbox*)bd1)->getDamage(), 3);
            }
            else if (_player->isParryActive()) {
                ((Hitbox*)bd1)->getEnemy()->setStun(88);
            }
            else if (_player->isGuardActive()) {
                _player->damage(((Hitbox*)bd1)->getDamage() / 2);
                _screenShake(((Hitbox*)bd1)->getDamage() / 2, 3);
            }
        }
    }
    else if (bd2->getName() == "hitbox" && isPlayerBody(bd1, fd1)) {
        if (_player->iframe <= 0) {
            _player->iframe = 60;
            if (!_player->isGuardActive() && !_player->isParryActive()) {
                _player->damage(((Hitbox*)bd2)->getDamage());
                _player->setKnocked(true, _player->getPosition().subtract(((Hitbox*)bd2)->getEnemy()->getPosition()).normalize());
                _screenShake(((Hitbox*)bd2)->getDamage(), 3);
            }
            else if (_player->isParryActive()) {
                ((Hitbox*)bd2)->getEnemy()->setStun(88);
            }
            else if (_player->isGuardActive()) {
                _player->damage(((Hitbox*)bd2)->getDamage() / 2);
                _screenShake(((Hitbox*)bd2)->getDamage() / 2, 3);
            }
        }
    }

    // Player-Projectile Collision
    if (isPlayerBody(bd1, fd1) && bd2->getName() == proj_name) {
        if (!((Projectile*)bd2)->getIsPlayerFired()) {
            if (_player->iframe <= 0) {
                _player->iframe = 60;
                _player->damage(10);
            }
            _removeProjectile((Projectile*)bd2);
            _ui->setHP(_player->getHP());
            _pauseMenu->setHP(_player->getHP());
        }
    }
    else if (isPlayerBody(bd2, fd2) && bd1->getName() == proj_name) {
        if (!((Projectile*)bd1)->getIsPlayerFired()) {
            if (_player->iframe <= 0) {
                _player->iframe = 60;
                _player->damage(10);
            }
            _removeProjectile((Projectile*)bd1);
            _ui->setHP(_player->getHP());
            _pauseMenu->setHP(_player->getHP());
        }
    }

    // Shield-Projectile Collision
    Projectile* shieldHit = getProjectileHitShield(bd1, fd1, bd2, fd2);
    if (shieldHit) {

        if (_player->isParryActive()) {
            if (!_player->hasProjectile()) {
                _player->setHasProjectile(true);
            }
        }
        else {
            _player->damage(10);
            _ui->setHP(_player->getHP());
            _pauseMenu->setHP(_player->getHP());
        }
        _removeProjectile(shieldHit);
    }

    // Projectile-Projectile Collision
    if (bd1->getName() == proj_name && bd2->getName() == proj_name) {

        // Destroy if one is fired by player and the other is not
        if (
            (((Projectile*)bd1)->getIsPlayerFired() && !((Projectile*)bd2)->getIsPlayerFired()) ||
            (((Projectile*)bd2)->getIsPlayerFired() && !((Projectile*)bd1)->getIsPlayerFired())
            ) {
            _removeProjectile((Projectile*)bd1);
            _removeProjectile((Projectile*)bd2);
        }
    }

    // Projectile-Environment Collision
    if (bd1->getName() == proj_name && (bd2->getName() == ground_name || bd2->getName() == wall_name)) {
        _removeProjectile((Projectile*)bd1);
    }
    else if (bd2->getName() == proj_name && (bd1->getName() == ground_name || bd1->getName() == wall_name)) {
        _removeProjectile((Projectile*)bd2);
    }

    // Enemy-Projectile Collision
    if (bd1->getName() == enemy_name && bd2->getName() == proj_name) {

        if (((Projectile*)bd2)->getIsPlayerFired()) {
            ((EnemyModel*)bd1)->damage(20);
            _removeProjectile((Projectile*)bd2);
        }
    }
    else if (bd2->getName() == enemy_name && bd1->getName() == proj_name) {
        if (((Projectile*)bd1)->getIsPlayerFired()) {
            ((EnemyModel*)bd2)->damage(20);
            _removeProjectile((Projectile*)bd1);
        }
    }

    // Player-Ground Collision
    if ((_player->getGroundSensorName() == fd2 && _player.get() != bd1) ||
        (_player->getGroundSensorName() == fd1 && _player.get() != bd2)) {
        _player->setGrounded(true);

        // Could have more than one ground
        _sensorFixtures.emplace(_player.get() == bd1 ? fix2 : fix1);
    }
}

/**
 * Callback method for the start of a collision
 *
 * This method is called when two objects cease to touch.  The main use of this method
 * is to determine when the characer is NOT on the ground.  This is how we prevent
 * double jumping.
 */
void CollisionController::endContact(b2Contact* contact) {
    b2Fixture* fix1 = contact->GetFixtureA();
    b2Fixture* fix2 = contact->GetFixtureB();

    b2Body* body1 = fix1->GetBody();
    b2Body* body2 = fix2->GetBody();

    std::string* fd1 = reinterpret_cast<std::string*>(fix1->GetUserData().pointer);
    std::string* fd2 = reinterpret_cast<std::string*>(fix2->GetUserData().pointer);

    physics2::Obstacle* bd1 = reinterpret_cast<physics2::Obstacle*>(body1->GetUserData().pointer);
    physics2::Obstacle* bd2 = reinterpret_cast<physics2::Obstacle*>(body2->GetUserData().pointer);

    if ((_player->getGroundSensorName() == fd2 && _player.get() != bd1) ||
        (_player->getGroundSensorName() == fd1 && _player.get() != bd2)) {
        _sensorFixtures.erase(_player.get() == bd1 ? fix2 : fix1);
        if (_sensorFixtures.empty()) {
            _player->setGrounded(false);
        }
    }
}



/**Checks obstacle and fixture to see if it is an enemy body fixture.**/
bool CollisionController::isEnemyBody(physics2::Obstacle* b, std::string f ) {
    // This depends on enemies having their name set to enemy. This is probably dumb
    return (f == "enemy");
}
/**Checks obstacle and fixture to see if it the player body fixture.**/
bool CollisionController::isPlayerBody(physics2::Obstacle* b, const std::string* f ) {
    return (f == _player->getBodyName());
}

/**
 Checks if contact is projectile hitting player shield and returns the Projectile if so, else NULL.
 */
Projectile* CollisionController::getProjectileHitShield(physics2::Obstacle* bd1, std::string* fd1,
                                              physics2::Obstacle* bd2, std::string* fd2) const {
    std::string proj_name = _constantsJSON->get("projectile")->getString("name");
    if (bd1->getName() == proj_name && fd2 == _player->getShieldName() &&
        !((Projectile*)bd1)->getIsPlayerFired() && _player->isGuardActive()) {
        return (Projectile*)bd1;
    }
    if (bd2->getName() == proj_name && fd1 == _player->getShieldName() &&
        !((Projectile*)bd2)->getIsPlayerFired() && _player->isGuardActive()) {
        return (Projectile*)bd2;
    }
    return nullptr;
}

void CollisionController::reset() {
    _sensorFixtures.clear();
}
