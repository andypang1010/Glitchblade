#include "GBMinion2AController.h"

#define SIGNUM(x)  ((x > 0) - (x < 0))

Minion2AController::Minion2AController() : EnemyController() {}

void Minion2AController::init(const std::shared_ptr<AssetManager>& assetRef,
    const std::shared_ptr<JsonValue>& enemiesJSON,
    std::vector<std::shared_ptr<ActionModel>> actions) {
    _enemyJSON = enemiesJSON->get("minion_2A");
    _enemy = Minion2AModel::alloc(assetRef, _enemyJSON, ENEMY_DEFAULT_INIT_POS, actions);
    EnemyController::init(assetRef, _enemyJSON, actions);
}

void Minion2AController::applyForce() {
    if (!_enemy || !_enemy->isEnabled()) return;

    b2Body* enemyBody = _enemy->getBody();

    if (_enemy->getMovement() == 0.0f) {
        if (_enemy->isGrounded()) {
            b2Vec2 vel = enemyBody->GetLinearVelocity();
            vel.x = 0;
            enemyBody->SetLinearVelocity(vel);
        }
        else {
            b2Vec2 dampForce(-_enemy->getDamping() * _enemy->getVX(), -_enemy->getDamping() * _enemy->getVY());
            enemyBody->ApplyForceToCenter(dampForce, true);
        }
    }

    if (!_enemy->isStunned()) {
        b2Vec2 force(_enemy->getMovement(), std::dynamic_pointer_cast<Minion2AModel>(_enemy)->getVerticalMovement());
        enemyBody->ApplyForceToCenter(force, true);

//        float d_force = _enemyJSON->get("physics")->get("dash")->getFloat("force");
//        // Dash force fetched but unused ?this is where you'd apply it if needed
    }

    if (_enemy->isKnocked()) {
        enemyBody->SetLinearVelocity(b2Vec2(0, 0));
        Vec2 knockDirection = _enemy->getKnockDirection();
        Vec2 knockForce = knockDirection.subtract(Vec2(0, knockDirection.y)).scale(_enemy->getKnockF());
        enemyBody->ApplyLinearImpulseToCenter(b2Vec2(knockForce.x, _enemy->getKnockF()), true);
    }

    if (fabs(_enemy->getVX()) >= _enemy->getMaxSpeed()) {
        _enemy->setVX(SIGNUM(_enemy->getVX()) * _enemy->getMaxSpeed());
    }

    if (fabs(_enemy->getVY()) >= _enemy->getMaxSpeed()) {
        _enemy->setVY(SIGNUM(_enemy->getVY()) * _enemy->getMaxSpeed());
    }

    if (_enemy->isStunned()) {
        _enemy->setVX(_enemy->getVX() / 3);
    }
}

void Minion2AController::preUpdate(float dt) {
    if (_hpNode) _hpNode->setText(std::to_string((int)_enemy->getHP()));
    //if (_hpNode) _hpNode->setText(std::to_string((int)_enemy->getAggression()));
    if (_stunNode) _stunNode->setText((_enemy->isStunned() ? "STUN" : ""));
    

    // Apply cooldowns
    _enemy->setAggression(std::min(100.0f, _enemy->getAggression() + dt * 5));

    if (_enemy->isKnocked()) {
        _enemy->resetKnocked();
    }

    if (_enemy->isStunned()) {
        _enemy->setStun(_enemy->getStunRem() - 1);
    }
}

void Minion2AController::fixedUpdate(float timestep) {
    applyForce();
    _enemy->updateAnimation();
    _enemy->nextAction();
    //don't put code here! it will get called twice in certain cases, between pre and post update (and 0 times other cases).
    // the fixedUpdate for the obstacle world is already handled by the physics world itself, applyForce must stay in preUpdate!
}


void Minion2AController::postUpdate(float dt) {
    // No post-update behavior needed yet
}
