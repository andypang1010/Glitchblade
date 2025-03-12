#include "GBPlayerController.h"
#include "GBInput.h"
#include "GBPlayerModel.h"
using namespace cugl;
using namespace cugl::graphics;

#define SIGNUM(x)  ((x > 0) - (x < 0))

// CONSTANTS:
float POS[] = { 2.5f, 5.0f };

PlayerController::PlayerController()
{
}


void PlayerController::init(const cugl::Rect bounds, const std::shared_ptr<AssetManager>& assetRef, float scale)
{
    std::shared_ptr<Texture> image;
    std::shared_ptr<scene2::PolygonNode> sprite;

    _input = PlatformInput::alloc(assetRef, bounds);
	Vec2 pos = POS;
    image = assetRef->get<Texture>(PLAYER_TEXTURE);
    _player = PlayerModel::alloc(pos, image->getSize() / scale, scale);
    sprite = scene2::PolygonNode::allocWithTexture(image);
    _player->setSceneNode(sprite);
    _player->setDebugColor(DEBUG_COLOR);

	CULog("Inited playercontoller");
}


void PlayerController::dispose() {
    _input->dispose();
}

/**
 * Resets the status of the game so that we can play again.
 *
 * This method disposes of the world and creates a new one.
 */
void PlayerController::reset() {
    _player = nullptr;
    _input = nullptr;
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
            // CULog("Setting x vel to 0");
            // Instant friction on the grounds
            b2Vec2 vel = playerBody->GetLinearVelocity();
            vel.x = 0; // If you set y, you will stop a jump in place
            playerBody->SetLinearVelocity(vel);
        } else {
            //             Damping factor in the air
            b2Vec2 force(_player->getDamping()*_player->getVX(),0);
            playerBody->ApplyForceToCenter(force,true);
        }
    }
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
        playerBody->ApplyLinearImpulseToCenter(force,true);
    }
#pragma mark dash force
    // Dash!
    if (_player->isDashLeftBegin() && _player->getDashReset()){
        CULog("dashing left begin");
        _player->faceLeft();
        // b2Vec2 force(-_player->getDashF(),0);
        // _body->ApplyLinearImpulseToCenter(force, true); // Old method of dashing
        playerBody->SetLinearVelocity(b2Vec2(-_player->getDashF(), playerBody->GetLinearVelocity().y));
        _player->setDashReset(false); //only needed (and is it really needed?) for keyboard
    }
    
    else if (_player->isDashRightBegin() && _player->getDashReset()){
        CULog("dashing right begin");
        _player->faceRight();
        // b2Vec2 force(DASH, 0);
        // _body->ApplyLinearImpulseToCenter(force, true);
        playerBody->SetLinearVelocity(b2Vec2(_player->getDashF(), playerBody->GetLinearVelocity().y));
        _player->setDashReset(false); //only needed (and is it really needed?) for keyboard
    }
#pragma mark knockback force
    if (_player->isKnocked()) {
        playerBody->SetLinearVelocity(b2Vec2(0,0));
        Vec2 knockDirection = _player->getKnockDirection();
        Vec2 knockForce = knockDirection.subtract(Vec2(0,knockDirection.y)).scale(_player->getKnockF());
        playerBody->ApplyLinearImpulseToCenter(b2Vec2(knockForce.x, _player->getKnockF()), true);
        _player->resetKnocked();
    }
    // Velocity too high, clamp it
    if (fabs(_player->getVX()) >= _player->getMaxSpeed() && !_player->isDashActive() && !_player->isKnockbackActive()) {
        //CULog("clamping velocity");
        _player->setVX(SIGNUM(_player->getVX())*_player->getMaxSpeed());
        
    }
}

#pragma mark fixedUpdate
void PlayerController::fixedUpdate(float timestep)
{
	// CULog("updated playercontroller");
}

#pragma mark preUpdate
void PlayerController::preUpdate(float dt)
{    _input->update(dt);
    // can't reset or set debug for the whole scene from player controller- we should use buttons for reset, debug , exit instead of keyboard (or gesture) inputs
//    // Process the toggled key commands
//    if (_input->didDebug()) { setDebug(!isDebug()); }
//    if (_input->didReset()) { reset(); }
//    if (_input->didExit()) {
//        CULog("Shutting down");
//        Application::get()->quit();
//    }

    // Process the movement

    _player->setMovement(_input->getHorizontal()*_player->getForce());
    _player->setStrafeLeft(_input->didStrafeLeft());
    _player->setStrafeRight(_input->didStrafeRight());
    _player->setJumpInput( _input->didJump());
    _player->setDashLeftInput(_input->didDashLeft());
    _player->setDashRightInput(_input->didDashRight());
    _player->setGuardInput(_input->didGuard());
    applyForce();

    // guard cooldown first for most recent movement inputs
    if (_player->isGuardActive() && !_player->isGuardBegin()){
        CULog("Beginning guard");
        int guardRem = _player->getGuardRem();
        _player->setGuardRem(guardRem > 0 ? guardRem - 1 : 0);
        int parryRem = _player->getParryRem();
        _player->setParryRem(parryRem > 0 ? parryRem - 1 : 0);
    } else if (_player->getGuardRem()>0) {
        CULog("Updating guard cooldown");
        // guard not active, update cooldown
        int guardCD = _player->getGuardCDRem();
        _player->setGuardCDRem(guardCD > 0 ? guardCD - 1 : 0);
    }
        
    if (_player->isJumpBegin() && _player->isGrounded()) {
        _player->setJumpCDRem();
    } else {
        int jumpCD = _player->getJumpCDRem();
        _player->setJumpCDRem(jumpCD > 0 ? jumpCD - 1 :0);
    }
    
    if (_player->isShooting()) {
        _player->setShootCDRem();
    } else {
        int shootCD = _player->getShootCDRem();
        _player->setShootCDRem(shootCD > 0 ? shootCD - 1 : 0);
    }
    
    if (_player->isKnocked()) {
        CULog("Player is knocked");
        _player->setDashCDRem();
        _player->setGuardCDRem();
        _player->setJumpCDRem();
        _player->setShootCDRem();
        _player->setKnockbackRem();
    } else {
        int kbREM = _player->getKnockbackRem();
        _player->setKnockbackRem(kbREM > 0 ? kbREM - 1 : 0);
    }
    
    if (_player->isDashBegin()) {
        _player->setDashRem();
        _player->setDashCDRem();
    }
    else {
        int dashRem = _player->getDashRem();
        _player->setDashRem(dashRem > 0 ? dashRem - 1 : 0);
        int dashCDRem = _player->getDashCDRem();
        _player->setDashCDRem(dashCDRem > 0 ? dashCDRem - 1 : 0);
    }
    
    // Reset the dash if ready (requires user to stop holding dash key(s) for at least one frame)
    if (!_player->getDashReset() && _player->getDashCDRem() == 0 && !(_player->isDashLeftInput() || !_player->isDashRightInput())) {
        _player->setDashReset(true); // ready to dash again
    }
    
    // player inputs guard and cooldown is ready
    if (_player->isGuardBegin()) {
        _player->setGuardCDRem();
        _player->setGuardRem();
        // begin parry
        CULog("Beginning parry\n");
        _player->setParryRem();
    }
}

#pragma mark postUpdate
void PlayerController::postUpdate(float dt)
{
    _player->setShootInput(_input->didFire());
}

void PlayerController::activateShield()
{
}

void PlayerController::deactivateShield()
{
}

void PlayerController::fireProjectile()
{
}

void PlayerController::deflectProjectile()
{
}

