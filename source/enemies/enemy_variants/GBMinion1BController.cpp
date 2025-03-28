#include "GBMinion1BController.h"

#define SIGNUM(x)  ((x > 0) - (x < 0))

Minion1BController::Minion1BController() : EnemyController() {}

void Minion1BController::init(const std::shared_ptr<AssetManager>& assetRef,
    const std::shared_ptr<JsonValue>& constantsRef,
    std::vector<std::shared_ptr<ActionModel>> actions) {

    _enemy = Minion1BModel::alloc(assetRef, constantsRef, ENEMY_INIT_POS, actions);
    _enemyJSON = constantsRef->get("enemy");
    EnemyController::init(assetRef, constantsRef, actions);
}

void Minion1BController::applyForce() {
    if (!_enemy || !_enemy->isEnabled()) return;

    b2Body* enemyBody = _enemy->getBody();

    if (_enemy->getMovement() == 0.0f) {
        if (_enemy->isGrounded()) {
            b2Vec2 vel = enemyBody->GetLinearVelocity();
            vel.x = 0;
            enemyBody->SetLinearVelocity(vel);
        }
        else {
            b2Vec2 force(-_enemy->getDamping() * _enemy->getVX(), 0);
            enemyBody->ApplyForceToCenter(force, true);
        }
    }

    if (!_enemy->isStunned()) {
        b2Vec2 force(_enemy->getMovement(), 0);
        enemyBody->ApplyForceToCenter(force, true);

        float d_force = _enemyJSON->get("physics")->get("dash")->getFloat("force");
        // Dash force fetched but unused ?this is where you'd apply it if needed
    }

    if (_enemy->isKnocked()) {
        enemyBody->SetLinearVelocity(b2Vec2(0, 0));
        Vec2 knockDirection = _enemy->getKnockDirection();
        Vec2 knockForce = knockDirection.subtract(Vec2(0, knockDirection.y)).scale(_enemy->getKnockF());
        enemyBody->ApplyLinearImpulseToCenter(b2Vec2(knockForce.x, _enemy->getKnockF()), true);
    }

    if (fabs(_enemy->getVX()) >= _enemy->getMaxSpeed() && !_enemy->isKnockbackActive()) {
        _enemy->setVX(SIGNUM(_enemy->getVX()) * _enemy->getMaxSpeed());
    }

    if (_enemy->isStunned()) {
        _enemy->setVX(_enemy->getVX() / 3);
    }
}

void Minion1BController::fixedUpdate(float timestep) {
    applyForce();
}

void Minion1BController::preUpdate(float dt) {
    if (_hpNode) _hpNode->setText(std::to_string((int)_enemy->getHP()));
    if (_stunNode) _stunNode->setText((_enemy->isStunned() ? "STUN" : ""));
}

void Minion1BController::postUpdate(float dt) {
    // No post-update behavior needed yet
}
