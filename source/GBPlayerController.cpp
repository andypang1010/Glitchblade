#include "GBPlayerController.h"
#include "GBInput.h"
#include "GBPlayerModel.h"
using namespace cugl;
using namespace cugl::graphics;

#define DEBUG_COLOR     Color4::YELLOW
#define SIGNUM(x)  ((x > 0) - (x < 0))

// CONSTANTS:
float POS[] = { 2.5f, 5.0f };

PlayerController::PlayerController()
{
}


void PlayerController::init(cugl::Rect bounds, const std::shared_ptr<AssetManager>& assetRef, float scale)
{
    std::shared_ptr<Texture> image;
    std::shared_ptr<scene2::PolygonNode> sprite;

    _input.init(bounds);
	Vec2 dudePos = POS;
    image = assetRef->get<Texture>(TEXTURE);
    _player = PlayerModel::alloc(dudePos, image->getSize() / scale, scale);
    sprite = scene2::PolygonNode::allocWithTexture(image);
    _player->setSceneNode(sprite);
    _player->setDebugColor(DEBUG_COLOR);

	CULog("Inited playercontoller");
}

void PlayerController::dispose() {
    _input.dispose();
    _playerHPNode = nullptr;
}

/**
 * Resets the status of the game so that we can play again.
 *
 * This method disposes of the world and creates a new one.
 */
void PlayerController::reset() {
    _player = nullptr;
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

void PlayerController::fixedUpdate(float timestep)
{
	// CULog("updated playercontroller");
}

void PlayerController::preUpdate(float dt)
{
}

void PlayerController::postUpdate(float dt)
{
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

