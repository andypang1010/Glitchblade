#include "GBPlayerController.h"
#include "GBInput.h"
#include "GBPlayerModel.h"
using namespace cugl;
using namespace cugl::graphics;


// CONSTANTS:
float DUDE_POS[] = { 2.5f, 5.0f };

PlayerController::PlayerController()
{
}


void PlayerController::init(cugl::Rect bounds, const std::shared_ptr<AssetManager>& assetRef, float scale)
{
    std::shared_ptr<Texture> image;
    std::shared_ptr<scene2::PolygonNode> sprite;

    _input.init(bounds);
	Vec2 dudePos = DUDE_POS;
    image = assetRef->get<Texture>(DUDE_TEXTURE);
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

#pragma mark fixedUpdate
void PlayerController::fixedUpdate(float timestep)
{
	// CULog("updated playercontroller");
}

#pragma mark preUpdate
void PlayerController::preUpdate(float dt)
{
    // guard cooldown first for most recent movement inputs
    if (_player->isGuardActive() && !_player->isGuardBegin()){
        int guardRem = _player->getGuardRem();
        _player->setGuardRem(guardRem > 0 ? guardRem - 1 : 0);
        int parryRem = _player->getParryRem();
        _player->setParryRem(parryRem > 0 ? parryRem - 1 : 0);
    }
    
    // guard not active, update cooldown
    else {
        int guardCD = _player->getGuardCDRem();
        _player->setGuardCDRem(guardCD > 0 ? guardCD - 1 : 0);
        
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
        _dashCooldownRem = DASH_COOLDOWN;
        _player->setGuardCDRem();
        _player->setJumpCDRem();
        _player->setShootCDRem();
        _knockbackRem = KB_DURATION;
    } else {
        _knockbackRem = (_knockbackRem > 0 ? _knockbackRem-1 : 0);
    }
    
    if (_player->isDashBegin()) {
        _dashRem = DASH_DURATION;
        _dashCooldownRem = DASH_COOLDOWN;
    }
    else {
        _dashRem = (_dashRem > 0 ? _dashRem-1 : 0);
        if (_dashRem == 0){
            _dashCooldownRem = (_dashCooldownRem > 0 ? _dashCooldownRem-1 : 0);
        }
    }

    // Reset the dash if ready (requires user to stop holding dash key(s) for at least one frame)
    if (_dashReset == false && _dashCooldownRem == 0 && !(_isDashLeftInput || _isDashRightInput)) {
        _dashReset = true; // ready to dash again
    }

    // player inputs guard and cooldown is ready
    if (_player->isGuardBegin()) {
        CULog("Beginning guard\n");
        _shieldNode->setColor(Color4::GREEN);
        _guardCooldownRem = GUARD_COOLDOWN;
        _guardRem = GUARD_DURATION;
        // begin parry
        CULog("Beginning parry\n");
        _parryRem = PARRY_DURATION;
    }
}

#pragma mark postUpdate
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

