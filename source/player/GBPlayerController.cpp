#include "GBPlayerController.h"
#include "../core/GBInput.h"
#include "GBPlayerModel.h"
#include "GBAudio.h"
using namespace cugl;
using namespace cugl::graphics;
using namespace cugl::audio;

#define SIGNUM(x)  ((x > 0) - (x < 0))

// CONSTANTS:
float PLAYER_INIT_POS[] = { 30.0f, 5.0f };

PlayerController::PlayerController() {}


void PlayerController::init(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& constantsRef)
{
    //    float scale = constantsRef->get("scene")->getFloat("scale");
    //    Rect bounds;
    //    std::shared_ptr<JsonValue> sceneJ = _constantsJSON->get("scene");
    //    std::shared_ptr<JsonValue> boundsJ = sceneJ->get("bounds");
    //    bounds.origin.set(boundsJ->get("origin")->getFloat("x"), boundsJ->get("origin")->getFloat("y"));
    //    bounds.size.set(boundsJ->get("size")->getFloat("width"),boundsJ->get("size")->getFloat("height"));

    _input = PlatformInput::alloc(assetRef, constantsRef);
    _player = PlayerModel::alloc(assetRef, constantsRef, PLAYER_INIT_POS);
//#pragma mark hp node
//    _hpNode = scene2::Label::allocWithText(std::to_string(_player->getMaxHP()), assetRef->get<Font>(constantsRef->get("player")->get("debug")->getString("font")));
//    _hpNode->setAnchor(Vec2::ANCHOR_CENTER);
//    _hpNode->setForeground(Color4::CYAN);
//    _hpNode->setPosition(0, 55);
//    _player->getSceneNode()->addChild(_hpNode);
#pragma mark constants
    _assets = assetRef;
    _constantsJSON = constantsRef;
}


void PlayerController::dispose() {
    _player->dispose();
    _input->dispose();
}

/**
 * Resets the player to it's initial position
 */
void PlayerController::reset() {
    _player->reset();
    _player->setPosition(PLAYER_INIT_POS);
}

/**
 * Applies the force to the player body
 *
 * This method should be called after the force attribute is set.
 */
void PlayerController::applyForce() {
    if (!_player->isEnabled()) {
        return;
    }
    b2Body* playerBody = _player->getBody();
    // Don't want to be moving.f Damp out player motion
    if (_player->getMovement() == 0.0f && !_player->isDashActive() && !_player->isKnockbackActive()) {
        if (_player->isGrounded()) {
            // Instant friction on the grounds
            b2Vec2 vel = playerBody->GetLinearVelocity();
            vel.x = 0; // If you set y, you will stop a jump in place
            playerBody->SetLinearVelocity(vel);
        }
        else {
            //             Damping factor in the air
            b2Vec2 force(_player->getDampF() * _player->getVX(), 0);
            playerBody->ApplyForceToCenter(force, true);
        }
    }

    if (!_player->isGuardActive()) {
        if (!_player->isDashActive()) {
#pragma mark strafe force
            //    b2Vec2 force(getMovement(),0);
                // Ignore stafe input if in a dash (intentional)
            if (!_player->isDashActive() && !_player->isKnockbackActive()) {
                playerBody->SetLinearVelocity(b2Vec2(_player->getMovement(), playerBody->GetLinearVelocity().y));
            }
            // _body->ApplyForceToCenter(force,true); // Old method of movement (slipper)
#pragma mark jump force
    // Jump!
            if (_player->isJumpBegin() && _player->isGrounded()) {
                b2Vec2 force(0, _player->getJumpF());
                playerBody->ApplyLinearImpulseToCenter(force, true);
            }
        }
    }

    if (!_player->isParryActive()) {
        // Dash!
#pragma mark dash force
        if (_player->isDashLeftBegin()) {
            _player->faceLeft();
            // b2Vec2 force(-_player->getDashF(),0);
            // _body->ApplyLinearImpulseToCenter(force, true); // Old method of dashing
            playerBody->SetLinearVelocity(b2Vec2(-_player->getDashF(), playerBody->GetLinearVelocity().y));
        }
        if (_player->isDashRightBegin()) {
            _player->faceRight();
            // b2Vec2 force(DASH, 0);
            // _body->ApplyLinearImpulseToCenter(force, true);
            playerBody->SetLinearVelocity(b2Vec2(_player->getDashF(), playerBody->GetLinearVelocity().y));
        }

        if (_player->isDashActive())
        {
            playerBody->SetLinearVelocity(b2Vec2(_player->getVX(), 0));
        }
    }

#pragma mark knockback force
    if (_player->isKnocked()) {
        playerBody->SetLinearVelocity(b2Vec2(0, 0));
        Vec2 knockDirection = _player->getKnockDirection();
        Vec2 knockForce = knockDirection.subtract(Vec2(0, knockDirection.y)).scale(_player->getKnockF());
        playerBody->ApplyLinearImpulseToCenter(b2Vec2(knockForce.x, _player->getKnockF()), true);
    }
 
    // Velocity too high, clamp it
    if (fabs(_player->getVX()) >= _player->getMaxSpeed() && !_player->isDashActive() && !_player->isKnockbackActive()) {
        _player->setVX(SIGNUM(_player->getVX()) * _player->getMaxSpeed());
    }
}


#pragma mark preUpdate
void PlayerController::preUpdate(float dt)
{
    _input->update(dt);
    // can't reset or set debug for the whole scene from player controller- we should use buttons for reset, debug , exit instead of keyboard (or gesture) inputs
    //    // Process the toggled key commands
    //    if (_input->didDebug()) { setDebug(!isDebug()); }
    //    if (_input->didReset()) { reset(); }
    //    if (_input->didExit()) {
    //        Application::get()->quit();
    //    }

    // Process the movement inputs
    _player->setMovement(_player->isGuardActive() ? 0 : _input->getHorizontal() * _player->getStrafeF());
    _player->setStrafeLeft(_input->didStrafeLeft());
    _player->setStrafeRight(_input->didStrafeRight());
    _player->setJumpInput(_input->didJump());
    _player->setDashLeftInput(_input->didDashLeft());
    _player->setDashRightInput(_input->didDashRight());
    _player->setGuardInput(_input->didGuard());
    _player->setShootInput(_input->didFire());
    //_hpNode->setText(std::to_string((int)_player->getHP()));

	_player->setDebugColor(_player->isParryActive() ? Color4::RED : _player->isGuardActive() ? Color4::YELLOW : Color4::BLACK);


#pragma mark sfx
    if (_player->isGuardBegin()){
        AudioHelper::play_sfx("guard");
    }

}
#pragma mark fixedUpdate
void PlayerController::fixedUpdate(float timestep)
{
    _player->updateAnimation();
    applyForce();
    updateCooldowns();

	_player->_lastComboElapsedTime += timestep;

	if (_player->_lastComboElapsedTime >= 5 && _player->_comboMeter > 0) {
		_player->_comboMeter = std::max(_player->_comboMeter - timestep * 10, 0.0f);
	}

    if (_player->_isNextAttackEnhanced) {
        CULog("NEXT ATTACK ENHANCED");
    }

    if (_player->_comboMeter >= 100)
    {
        if (_player->getHP() < _player->getMaxHP()) {
            _player->setHP(std::min(_player->getHP() + 20.0f, 100.0f));
        }

        else {
			_player->_isNextAttackEnhanced = true;
        }

		_player->_comboMeter = 0;
    }

    _player->getSceneNode()->setColor(_player->_isNextAttackEnhanced ? Color4::YELLOW : Color4::WHITE);


  //  if (_player->_parryCounter == 5) {
		//_player->_parryCounter = 0;
  //      _player->setHP(std::min(_player->getHP() + 20.0f, 100.0f));
  //  }
}

#pragma mark postUpdate
void PlayerController::postUpdate(float dt)
{
}

void PlayerController::updateCooldowns()
{
    if (_player->iframe > 0) _player->iframe--;
    if (_player->isDamaged()) _player->setDamagedRem(_player->getDamagedRem() - 1);
#pragma mark Guard cooldown
    // Update guard release time
    if (_player->getGuardReleaseRem() > 0) {
        int newRem = _player->getGuardReleaseRem() - 1;
        _player->setGuardReleaseRem(newRem);
    }
    // player inputs guard and cooldown is ready
    if (_player->isGuardBegin()) {
        _player->setGuardCDRem();
        _player->setGuardRem();
        _player->setParryRem();
        _player->setGuardState(1);
    }
    if (_player->isGuardActive() && !_player->isGuardBegin()) {
        int guardRem = _player->getGuardRem();
        _player->setGuardRem(guardRem - 1);
        if (guardRem == 1) {
            // Parry ending on this frame
            _player->setGuardReleaseRem(PLAYER_HIT_COLOR_DURATION);
            _player->setGuardState(3);
        }
        int parryRem = _player->getParryRem();
        _player->setParryRem(parryRem > 0 ? parryRem - 1 : 0);
    }
    // guard not active, update cooldown
    else if (_player->getGuardCDRem() >= 0) {
            int newCD = _player->getGuardCDRem() - 1;
            _player->setGuardCDRem((newCD < 0) ? 0 : newCD);
    }

#pragma mark Jump cooldown
    if (_player->isJumpBegin() && _player->isGrounded()) {
        _player->setJumpCDRem();
    }
    else {
        int jumpCD = _player->getJumpCDRem();
        _player->setJumpCDRem(jumpCD > 0 ? jumpCD - 1 : 0);
    }
#pragma mark Shoot cooldown
    if (_player->isShooting()) {
        _player->setShootCDRem();
    }
    else {
        int shootCD = _player->getShootCDRem();
        _player->setShootCDRem(shootCD > 0 ? shootCD - 1 : 0);
    }
#pragma mark Knockback cooldown
    if (_player->isKnocked()) {
        //_player->setDashCDRem();
        _player->setGuardCDRem();
        _player->setJumpCDRem();
        _player->setShootCDRem();
        _player->setKnockbackRem();
        _player->resetKnocked();
    }
    else {
        int kbREM = _player->getKnockbackRem();
        _player->setKnockbackRem(kbREM > 0 ? kbREM - 1 : 0);
    }
#pragma mark dash cooldowns
    if (_player->isDashBegin()) {
        _player->setDashRem();
        _player->setDashCDRem();
        _player->setDashReset(false); //only needed (and is it really needed?) for keyboard
    }
    else if (_player->getDashCDRem() > 0) {
        int dashRem = _player->getDashRem();
        _player->setDashRem(dashRem > 0 ? dashRem - 1 : 0);
        int dashCDRem = _player->getDashCDRem();
        _player->setDashCDRem(dashCDRem > 0 ? dashCDRem - 1 : 0);
    }

    // Reset the dash if ready (requires user to stop holding dash key(s) for at least one frame)
    if (!_player->getDashReset() && _player->getDashCDRem() <= 0 && !(_player->isDashInput())) {
        _player->setDashReset(true); // ready to dash again
    }

}

void PlayerController::fireProjectile()
{
}

void PlayerController::deflectProjectile()
{
}

