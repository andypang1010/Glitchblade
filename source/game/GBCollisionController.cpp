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
#include "../core/GBAudio.h"

using namespace cugl::physics2;
// Constructor
CollisionController::CollisionController(
     std::shared_ptr<PlayerModel> player,
     std::shared_ptr<GBIngameUI> ui,
     std::shared_ptr<JsonValue> constantsJSON,
     RemoveProjectileCallback removeProjectile,
     ScreenShakeCallback screenShake
) :
    _player(player),
    _ui(ui),
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

    Obstacle* o1 = reinterpret_cast<Obstacle*>(body1->GetUserData().pointer);
    Obstacle* o2 = reinterpret_cast<Obstacle*>(body2->GetUserData().pointer);


    // Player-Enemy Collision
    if (o1->getName() == enemy_name && isPlayerBody(o2, fd2)) {
        playerEnemyCollision(o1);
    } else if (o2->getName() == enemy_name && isPlayerBody(o1, fd1)) {
        playerEnemyCollision(o2);
    }
    
    // Aoe-Enemy Collision
    else if (o1->getName() == enemy_name && isPlayerAoe(o2, fd2)) {
        aoeEnemyCollision(o1);
    } else if (o2->getName() == enemy_name && isPlayerAoe(o1, fd1)) {
        aoeEnemyCollision(o2);
    }

    // Test: player-hitbox collision
    else if (o1->getName() == "hitbox" && isPlayerBody(o2, fd2)) {
        playerHitboxCollision(o1);
    }
    else if (o2->getName() == "hitbox" && isPlayerBody(o1, fd1)) {
        playerHitboxCollision(o2);
    }


    // Player-Projectile Collision
    else if (isPlayerBody(o1, fd1) && o2->getName() == proj_name) {
        playerProjectileCollision(o2);
    }
    else if (isPlayerBody(o2, fd2) && o1->getName() == proj_name) {
        playerProjectileCollision(o1);
    }

    // Projectile-Projectile Collision
    else if (o1->getName() == proj_name && o2->getName() == proj_name) {

        // Destroy if one is fired by player and the other is not
        if (
            (((Projectile*)o1)->getIsPlayerFired() && !((Projectile*)o2)->getIsPlayerFired()) ||
            (((Projectile*)o2)->getIsPlayerFired() && !((Projectile*)o1)->getIsPlayerFired())
            ) {
            _removeProjectile((Projectile*)o1);
            _removeProjectile((Projectile*)o2);
        }
    }

    // Projectile-Environment Collision
    else if (o1->getName() == proj_name && (o2->getName() == ground_name || o2->getName() == wall_name)) {
        _removeProjectile((Projectile*)o1);
    }
    else if (o2->getName() == proj_name && (o1->getName() == ground_name || o1->getName() == wall_name)) {
        _removeProjectile((Projectile*)o2);
    }

    // Enemy-Projectile Collision
    else if (o1->getName() == enemy_name && o2->getName() == proj_name) {
        enemyProjectileCollision(o1, o2);
    }
    else if (o2->getName() == enemy_name && o1->getName() == proj_name) {
        enemyProjectileCollision(o2, o1);
    }


    // Player-Ground Collision
    else if ((_player->getGroundSensorName() == fd2 && _player.get() != o1) ||
        (_player->getGroundSensorName() == fd1 && _player.get() != o2)) {
        _player->setGrounded(true);

        // Could have more than one ground
        _sensorFixtures.emplace(_player.get() == o1 ? fix2 : fix1);
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

    physics2::Obstacle* o1 = reinterpret_cast<physics2::Obstacle*>(body1->GetUserData().pointer);
    physics2::Obstacle* o2 = reinterpret_cast<physics2::Obstacle*>(body2->GetUserData().pointer);

    if ((_player->getGroundSensorName() == fd2 && _player.get() != o1) ||
        (_player->getGroundSensorName() == fd1 && _player.get() != o2)) {
        _sensorFixtures.erase(_player.get() == o1 ? fix2 : fix1);
        if (_sensorFixtures.empty()) {
            _player->setGrounded(false);
        }
    }
}

#pragma mark collision case helpers

void CollisionController::playerEnemyCollision(Obstacle* enemyObstacle) {
    EnemyModel* enemy = (EnemyModel*) enemyObstacle;
    if (_player->isDashActive() && !_player->isGuardActive()) {
        enemy->damage(_player->getDamage());
        _screenShake(_player->_isNextAttackEnhanced ? 30 : 3, 5);

        if (_player->_isNextAttackEnhanced) {
            _player->_isNextAttackEnhanced = false;
        }

        else {
            _player->incrementComboCounterByAttack();
        }

        _player->setDashRem(0);

    }
    _player->setKnocked(true, _player->getPosition().subtract(enemy->getPosition()).normalize());
    enemy->setKnocked(true, enemy->getPosition().subtract(_player->getPosition()).normalize());
}

/** doc comments generated by Microsoft Copilot for these collision case helpers*/
void CollisionController::playerHitboxCollision(Obstacle* hitboxObstacle) {
    Hitbox* hitbox = (Hitbox*)hitboxObstacle; // Cast to the proper type
    std::shared_ptr<EnemyModel> enemy = hitbox->getEnemy();
    // Process collision only if the player's invulnerability frames have expired.
    if (_player->iframe <= 0) {
        _player->iframe = 15;  // Reset the iframe counter

        // If neither guard nor parry is active, apply full damage and knockback.
        if (!_player->isGuardActive() && !_player->isParryActive()) {
            int damage = hitbox->getDamage();

            _player->damage(damage);
            _player->resetCombo();

            _player->setKnocked(true, _player->getPosition().subtract(enemy->getPosition()).normalize());
            _screenShake(damage, 3);
        }
        // If parry is active, stun the enemy.
        else if (_player->isParryActive()) {
            AudioHelper::playSfx("parry");
            if (!_player->_isNextAttackEnhanced)
            {
                _player->incrementComboCounterByParry();
            }

			_player->_parryCounter++;
            _player->damage(0);

            // Reset guard and parry
            _player->setParryRem(0);
            _player->setGuardRem(0);
            _player->setGuardCDRem(0);

            _screenShake(40, 1);

            if (hitbox->getIsParriable()) {
                enemy->setStun(enemy->stunFrames);
            }
        }
        // If guard is active, deal half damage with corresponding screen shake.
        else if (_player->isGuardActive()) {
            int halfDamage = hitbox->getDamage() / 2;
            _player->damage(halfDamage);

            _screenShake(halfDamage, 3);
        }
    }
}

void CollisionController::aoeEnemyCollision(Obstacle *enemyObstacle) {
    EnemyModel* enemy = (EnemyModel*) enemyObstacle;
    
    if (enemy->isKnockbackActive()){
        // don't want to do double damage
        return;
    }
    enemy->damage(_player->getDamage());
    _screenShake(_player->_isNextAttackEnhanced ? 30 : 3, 5);
    if (_player->_isNextAttackEnhanced) {
        _player->_isNextAttackEnhanced = false;
    }

    else {
        _player->incrementComboCounterByAttack();
    }
    enemy->setKnocked(true, enemy->getPosition().subtract(_player->getPosition()).normalize());
}

void CollisionController::playerProjectileCollision(Obstacle* projectileObstacle) {
    Projectile* projectile = (Projectile*)projectileObstacle; // Cast to the proper type

    // Process collision only if the projectile was not fired by the player.
    if (!projectile->getIsPlayerFired()) {
        // If the player's invulnerability frames have expired, reset and deal damage.
        bool deflected = false;
        if (_player->iframe <= 0) {
            if (_player->isParryActive()) {
                AudioHelper::playSfx("parry");
                if (!_player->hasProjectile()) {
                    _player->setHasProjectile(true);
                }

                if (!_player->_isNextAttackEnhanced)
                {
                    _player->incrementComboCounterByParry();
                }

                _player->_parryCounter++;
                _player->damage(0);

                // Reset guard and parry
                _player->setParryRem(0);
                _player->setGuardRem(0);
                _player->setGuardCDRem(0);

                _screenShake(40, 1);
				
                deflected = true;
				projectile->setIsPlayerFired(true);
				projectile->setVX(-projectile->getVX());
                projectile->setVY(-projectile->getVY());
                //projectile->getSceneNode()->flipHorizontal(!projectile->getSceneNode()->isFlipHorizontal());
				projectile->getSceneNode()->setColor(cugl::Color4(0, 255, 150, 255));
            }
            else if (_player->isGuardActive()) {
                _player->damage(projectile->getDamage() / 2);
                _ui->setHP(_player->getHP());
            }
            else {
                _player->damage(projectile->getDamage());
                _player->resetCombo();

                _player->setKnocked(true, _player->getPosition().subtract(projectileObstacle->getPosition()).normalize());
                _ui->setHP(_player->getHP());
            }
            _player->iframe = 15;
        }
        // Remove the projectile and update the UI and pause menu with the current HP.
        if (!deflected) _removeProjectile(projectile);
        _ui->setHP(_player->getHP());
    }
}

void CollisionController::enemyProjectileCollision(Obstacle* enemyObstacle, Obstacle* projectileObstacle) {
    // Cast obstacles to their proper types.
    EnemyModel* enemy = static_cast<EnemyModel*>(enemyObstacle);
    Projectile* projectile = static_cast<Projectile*>(projectileObstacle);
    
    // Process collision only if the projectile was fired by the player.
    if (projectile->getIsPlayerFired()) {
        enemy->damage(10);
        _removeProjectile(projectile);
        _screenShake(10, 3);
    }
}


#pragma mark collision helpers

/**Checks obstacle and fixture to see if it is an enemy body fixture.**/
bool CollisionController::isEnemyBody(physics2::Obstacle* b, std::string f ) {
    // This depends on enemies having their name set to enemy. This is probably dumb
    return (f == "enemy");
}
/**Checks obstacle and fixture to see if it the player body fixture.**/
bool CollisionController::isPlayerBody(physics2::Obstacle* b, const std::string* f ) {
    return (f == _player->getBodyName());
}
               
/**Checks obstacle and fixture to see if it the player aoe fixture.**/
bool CollisionController::isPlayerAoe(physics2::Obstacle* b, const std::string* f ) {
   return (f == _player->getAoeSensorName());
}
               


void CollisionController::reset() {
    _sensorFixtures.clear();
}


