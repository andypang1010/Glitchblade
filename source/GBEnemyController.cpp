#include "GBEnemyController.h"
#include "GBEnemyModel.h"
using namespace cugl;
using namespace cugl::graphics;

#define SIGNUM(x)  ((x > 0) - (x < 0))

//CONSTANTS (INITIAL POSITIONS SHOULD BE LATER DETERMINED BY LEVEL MODEL/CONTROLLER
float ENEMY_INIT_POS[] = { 12.5f, 5.0f };

EnemyController::EnemyController() {}

void EnemyController::init(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& constantsRef, std::vector<std::shared_ptr<ActionModel>> actions)
{
    _enemy = EnemyModel::alloc(assetRef, constantsRef, ENEMY_INIT_POS, actions);
    _enemyJSON = constantsRef->get("enemy");
#pragma mark hp node
    std::string enemy_debug_font = _enemyJSON->get("debug")->getString("font");
    _hpNode = scene2::Label::allocWithText("100", assetRef->get<Font>(enemy_debug_font));
    _hpNode->setAnchor(Vec2::ANCHOR_CENTER);
    _hpNode->setForeground(Color4::RED);
    _hpNode->setPosition(0, 115);

#pragma mark stun node
    _stunNode = scene2::Label::allocWithText("STUN", assetRef->get<Font>(enemy_debug_font));
    _stunNode->setAnchor(Vec2::ANCHOR_CENTER);
    _stunNode->setForeground(Color4::RED);
    _stunNode->setPosition(0, 135);

    // add indicator nodes as children to main scene node
    _enemy->getSceneNode()->addChild(_hpNode);
    _enemy->getSceneNode()->addChild(_stunNode);

}

void EnemyController::reset()
{
    _enemy->resetAttributes();
    _enemy->setPosition(ENEMY_INIT_POS);
}

void EnemyController::dispose() {
    _enemy->dispose();

}

/**
 * Applies the force to the body of this enemy
 *
 * This method should be called after the force attribute is set.
 */
void EnemyController::applyForce() {
    if (!_enemy->isEnabled()) {
        return;
    }
    b2Body* enemyBody = _enemy->getBody();

    // Don't want to be moving. Damp out player motion
    if (_enemy->getMovement() == 0.0f) {
        if (_enemy->isGrounded()) {
            // Instant friction on the ground
            b2Vec2 vel = enemyBody->GetLinearVelocity();
            vel.x = 0; // If you set y, you will stop a jump in place
            enemyBody->SetLinearVelocity(vel);
        }
        else {
            // Damping factor in the air
            b2Vec2 force(-_enemy->getDamping() * _enemy->getVX(), 0);
            enemyBody->ApplyForceToCenter(force, true);
        }
    }

    if (!_enemy->isStunned()) {
#pragma mark strafe force
        b2Vec2 force(_enemy->getMovement(), 0);
        enemyBody->ApplyForceToCenter(force, true);

#pragma mark dash force
// Dash!
        float d_force = _enemyJSON->get("physics")->get("dash")->getFloat("force");
    }
#pragma mark knockback force
    if (_enemy->isKnocked()) {
        enemyBody->SetLinearVelocity(b2Vec2(0, 0));
        Vec2 knockDirection = _enemy->getKnockDirection();
        Vec2 knockForce = knockDirection.subtract(Vec2(0, knockDirection.y)).scale(_enemy->getKnockF());
        enemyBody->ApplyLinearImpulseToCenter(b2Vec2(knockForce.x, _enemy->getKnockF()), true);
    }
    // Velocity too high, clamp it
    if (fabs(_enemy->getVX()) >= _enemy->getMaxSpeed() && !_enemy->isKnockbackActive()) {
        _enemy->setVX(SIGNUM(_enemy->getVX()) * _enemy->getMaxSpeed());
    }
    if (_enemy->isStunned()) {
        _enemy->setVX(_enemy->getVX() / 3);
    }
}

void EnemyController::fixedUpdate(float timestep)
{
    applyForce();
}

void EnemyController::preUpdate(float dt)
{
    _hpNode->setText(std::to_string((int)_enemy->getHP()));
    _stunNode->setText((_enemy->isStunned() ? "STUN" : ""));

}

void EnemyController::postUpdate(float dt)
{
}
