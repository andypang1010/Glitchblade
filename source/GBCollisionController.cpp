#include "GBCollisionController.h"
#include <box2d/b2_world.h>
#include <box2d/b2_contact.h>
#include <box2d/b2_collision.h>

using namespace cugl;

/**
 * Processes the start of a collision
 *
 * This method is called when we first get a collision between two objects.  We use
 * this method to test if it is the "right" kind of collision.  In particular, we
 * use it to test if we make it to the win door.
 *
 * @param  contact  The two bodies that collided
 */
//void CollisionController::beginContact(b2Contact* contact) {
//    b2Fixture* fix1 = contact->GetFixtureA();
//    b2Fixture* fix2 = contact->GetFixtureB();
//
//    b2Body* body1 = fix1->GetBody();
//    b2Body* body2 = fix2->GetBody();
//
//    std::string* fd1 = reinterpret_cast<std::string*>(fix1->GetUserData().pointer);
//    std::string* fd2 = reinterpret_cast<std::string*>(fix2->GetUserData().pointer);
//
//    if (!fix1->GetUserData().pointer || !fix2->GetUserData().pointer) {
//        //CULog("Error: fix1 or fix2 has null user data.");//projectiles don't have names so they will error
//    }
//    else {
//        CULog("fix1 is %s and fix2 is %s", fd1->c_str(), fd2->c_str());
//    }
//
//    physics2::Obstacle* bd1 = reinterpret_cast<physics2::Obstacle*>(body1->GetUserData().pointer);
//    physics2::Obstacle* bd2 = reinterpret_cast<physics2::Obstacle*>(body2->GetUserData().pointer);
//
//    // Player-Enemy Collision
//    if (bd1->getName() == ENEMY_NAME && isPlayerBody(bd2, fd2)) {
//        if (((EnemyModel*)bd1)->isDashActive() && !((PlayerModel*)bd2)->isDashActive()) {
//            ((PlayerModel*)bd2)->damage(20);
//            ((EnemyModel*)bd1)->setDashRem(0);
//        }
//        else if (!((EnemyModel*)bd1)->isDashActive() && ((PlayerModel*)bd2)->isDashActive()) {
//            ((EnemyModel*)bd1)->damage(20);
//            ((PlayerModel*)bd2)->setDashRem(0);
//        }
//        else if (((EnemyModel*)bd1)->isDashActive() && ((PlayerModel*)bd2)->isDashActive()) {
//            ((EnemyModel*)bd1)->setDashRem(0);
//            ((PlayerModel*)bd2)->setDashRem(0);
//        }
//        ((PlayerModel*)bd2)->setKnocked(true, bd2->getPosition().subtract(bd1->getPosition()).normalize());
//        ((EnemyModel*)bd1)->setKnocked(true, bd1->getPosition().subtract(bd2->getPosition()).normalize());
//    }
//    else if (bd2->getName() == ENEMY_NAME && isPlayerBody(bd1, fd1)) {
//        if (((EnemyModel*)bd2)->isDashActive() && !((PlayerModel*)bd1)->isDashActive()) {
//            ((PlayerModel*)bd1)->damage(20);
//            ((EnemyModel*)bd2)->setDashRem(0);
//        }
//        else if (!((EnemyModel*)bd2)->isDashActive() && ((PlayerModel*)bd1)->isDashActive()) {
//            ((EnemyModel*)bd2)->damage(20);
//            ((PlayerModel*)bd1)->setDashRem(0);
//        }
//        else if (((EnemyModel*)bd2)->isDashActive() && ((PlayerModel*)bd1)->isDashActive()) {
//            ((EnemyModel*)bd2)->setDashRem(0);
//            ((PlayerModel*)bd1)->setDashRem(0);
//        }
//        ((PlayerModel*)bd1)->setKnocked(true, bd1->getPosition().subtract(bd2->getPosition()).normalize());
//        ((EnemyModel*)bd2)->setKnocked(true, bd2->getPosition().subtract(bd1->getPosition()).normalize());
//    }
//
//    // Player-Projectile Collision
//    if (isPlayerBody(bd1, fd1) && bd2->getName() == PROJECTILE_NAME) {
//        if (!((Projectile*)bd2)->getIsPlayerFired()) {
//            ((PlayerModel*)bd1)->damage(20);
//            removeProjectile((Projectile*)bd2);
//        }
//    }
//    else if (isPlayerBody(bd2, fd2) && bd1->getName() == PROJECTILE_NAME) {
//        if (!((Projectile*)bd1)->getIsPlayerFired()) {
//            ((PlayerModel*)bd2)->damage(20);
//            removeProjectile((Projectile*)bd1);
//        }
//        // TODO: REFACTOR TO NOT REPEAT CODE!!!
//    }
//
//    // Shield-Enemy Collision
//    if (bd1->getName() == ENEMY_NAME && fd2->c_str() == SHIELD_NAME) {
//        if (((EnemyModel*)bd1)->isDashActive() && _player->isParryActive()) {
//            ((EnemyModel*)bd1)->setDashRem(0);
//            ((EnemyModel*)bd1)->setStun(180);
//        }
//        else if (((EnemyModel*)bd1)->isDashActive() && _player->isGuardActive()) {
//            _player->damage(10);
//            ((EnemyModel*)bd1)->setDashRem(0);
//        }
//    }
//
//    if (bd2->getName() == ENEMY_NAME && fd1->c_str() == SHIELD_NAME) {
//        if (((EnemyModel*)bd2)->isDashActive() && _player->isParryActive()) {
//            ((EnemyModel*)bd2)->setDashRem(0);
//            ((EnemyModel*)bd2)->setStun(180);
//        }
//        else if (((EnemyModel*)bd2)->isDashActive() && _player->isGuardActive()) {
//            _player->damage(10);
//            ((EnemyModel*)bd2)->setDashRem(0);
//        }
//    }
//
//    // Shield-Projectile Collision
//    Projectile* shieldHit = getProjectileHitShield(bd1, fd1, bd2, fd2);
//    if (shieldHit) {
//
//        if (_player->isParryActive()) {
//            CULog("Parried projectile");
//
//            if (!_player->hasProjectile()) {
//                _player->setHasProjectile(true);
//            }
//        }
//        else {
//            CULog("Guarded projectile");
//
//            _player->damage(10);
//        }
//        removeProjectile(shieldHit);
//    }
//
//    // Projectile-Projectile Collision
//    if (bd1->getName() == PROJECTILE_NAME && bd2->getName() == PROJECTILE_NAME) {
//
//        // Destroy if one is fired by player and the other is not
//        if (
//            (((Projectile*)bd1)->getIsPlayerFired() && !((Projectile*)bd2)->getIsPlayerFired()) ||
//            (((Projectile*)bd2)->getIsPlayerFired() && !((Projectile*)bd1)->getIsPlayerFired())
//            ) {
//            removeProjectile((Projectile*)bd1);
//            removeProjectile((Projectile*)bd2);
//        }
//    }
//
//    // Projectile-Environment Collision
//    if (bd1->getName() == PROJECTILE_NAME && bd2->getName() == GROUND_NAME) {
//        removeProjectile((Projectile*)bd1);
//    }
//    else if (bd2->getName() == PROJECTILE_NAME && bd1->getName() == GROUND_NAME) {
//        removeProjectile((Projectile*)bd2);
//    }
//
//    // Enemy-Projectile Collision
//    if (bd1->getName() == ENEMY_NAME && bd2->getName() == PROJECTILE_NAME) {
//
//        if (((Projectile*)bd2)->getIsPlayerFired()) {
//            CULog("Enemy Damaged, remaining HP %f", ((EnemyModel*)bd1)->getHP());
//            ((EnemyModel*)bd1)->damage(20);
//            removeProjectile((Projectile*)bd2);
//        }
//    }
//    else if (bd2->getName() == ENEMY_NAME && bd1->getName() == PROJECTILE_NAME) {
//        if (((Projectile*)bd1)->getIsPlayerFired()) {
//            CULog("Enemy Damaged, remaining HP %f", ((EnemyModel*)bd2)->getHP());
//            ((EnemyModel*)bd2)->damage(20);
//            removeProjectile((Projectile*)bd1);
//        }
//    }
//
//    // Player-Ground Collision
//    if ((GROUND_SENSOR_NAME == fd2->c_str() && PLAYER_NAME != bd1->getName()) ||
//        (GROUND_SENSOR_NAME == fd1->c_str() && PLAYER_NAME != bd2->getName())) {
//        _player->setGrounded(true);
//
//        // Could have more than one ground
//        _sensorFixtures.emplace(_player.get() == bd1 ? fix2 : fix1);
//    }
//}
//
///**
// * Callback method for the start of a collision
// *
// * This method is called when two objects cease to touch.  The main use of this method
// * is to determine when the characer is NOT on the ground.  This is how we prevent
// * double jumping.
// */
//void CollisionController::endContact(b2Contact* contact) {
//    b2Fixture* fix1 = contact->GetFixtureA();
//    b2Fixture* fix2 = contact->GetFixtureB();
//
//    b2Body* body1 = fix1->GetBody();
//    b2Body* body2 = fix2->GetBody();
//
//    std::string* fd1 = reinterpret_cast<std::string*>(fix1->GetUserData().pointer);
//    std::string* fd2 = reinterpret_cast<std::string*>(fix2->GetUserData().pointer);
//
//    physics2::Obstacle* bd1 = reinterpret_cast<physics2::Obstacle*>(body1->GetUserData().pointer);
//    physics2::Obstacle* bd2 = reinterpret_cast<physics2::Obstacle*>(body2->GetUserData().pointer);
//
//    if ((GROUND_SENSOR_NAME == fd2->c_str() && PLAYER_NAME != bd1->getName()) ||
//        (GROUND_SENSOR_NAME == fd1->c_str() && PLAYER_NAME != bd2->getName())) {
//        _sensorFixtures.erase(_player.get() == bd1 ? fix2 : fix1);
//        if (_sensorFixtures.empty()) {
//            _player->setGrounded(false);
//        }
//    }
//}
//
///**Checks obstacle and fixture to see if it is an enemy body fixture.**/
//bool CollisionController::isEnemyBody(physics2::Obstacle* b) {
//    return (b->getName() == ENEMY_NAME);
//}
///**Checks obstacle and fixture to see if it the player body fixture.**/
//bool CollisionController::isPlayerBody(physics2::Obstacle* b, const std::string* f) {
//    return (b->getName() == PLAYER_NAME && f->c_str() == PLAYER_NAME);
//}
//
///**
// Checks if contact is projectile hitting player shield and returns the Projectile if so, else NULL.
// */
//Projectile* CollisionController::getProjectileHitShield(physics2::Obstacle* bd1, std::string* fd1,
//    physics2::Obstacle* bd2, std::string* fd2) const {
//    if (bd1->getName() == PROJECTILE_NAME && fd2->c_str() == SHIELD_NAME &&
//        !((Projectile*)bd1)->getIsPlayerFired() && ((PlayerModel*)bd2)->isGuardActive()) {
//            return (Projectile*)bd1;
//    }
//    if (bd2->getName() == PROJECTILE_NAME && fd1->c_str() == SHIELD_NAME &&
//        !((Projectile*)bd2)->getIsPlayerFired() && ((PlayerModel*)bd1)->isGuardActive()) {
//        return (Projectile*)bd2;
//    }
//    return nullptr;
//}
//
//Projectile* CollisionController::getEnemyHitShield(physics2::Obstacle* bd1, std::string* fd1, physics2::Obstacle* bd2, std::string* fd2) const
//{
//    return nullptr;
//}
//
//CollisionController::CollisionController()
//{
//}
//
//CollisionController::~CollisionController()
//{
//}
//
//void CollisionController::init()
//{
//}
//
//void CollisionController::fixedUpdate(float timestep)
//{
//    _world->update(timestep);
//}