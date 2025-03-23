#include "GBPlayerController.h"
#include "GBInput.h"
#include "GBPlayerModel.h"
using namespace cugl;
using namespace cugl::graphics;

#define SIGNUM(x)  ((x > 0) - (x < 0))

// CONSTANTS:
float INIT_POS[] = { 2.5f, 5.0f };

PlayerController::PlayerController()
{
}


void PlayerController::init(const cugl::Rect bounds, const std::shared_ptr<AssetManager>& assetRef, float scale)
{
    std::shared_ptr<Texture> image;
    std::shared_ptr<scene2::PolygonNode> sprite;

    _input = PlatformInput::alloc(assetRef, bounds);
	Vec2 pos = INIT_POS;
    image = assetRef->get<Texture>(PLAYER_TEXTURE);
    _player = PlayerModel::alloc(pos, image->getSize() / scale, scale);
    _player->_idleSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("player_idle"), 3, 4, 11);
    _player->_idleSprite->setPosition(0, -25);
	_player->_idleSprite->setScale(0.5f);

    _player->_walkSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("player_walk"), 2, 4, 5);
    _player->_walkSprite->setPosition(0, -25);
    _player->_walkSprite->setScale(0.5f);

    _player->_jumpUpSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("player_jumpUp"), 2, 4, 5);
    _player->_jumpUpSprite->setPosition(0, -25);
    _player->_jumpUpSprite->setScale(0.5f);

    _player->_jumpDownSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("player_jumpDown"), 2, 4, 7);
    _player->_jumpDownSprite->setPosition(0, -25);
    _player->_jumpDownSprite->setScale(0.5f);

    _player->_guardSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("player_guard"), 3, 4, 12);
    _player->_guardSprite->setPosition(0, -25);
    _player->_guardSprite->setScale(0.5f);

    _player->_attackSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("player_attack"), 2, 4, 5);
    _player->_attackSprite->setPosition(0, -25);
    _player->_attackSprite->setScale(0.5f);
   
    _player->getSceneNode()->addChild(_player->_idleSprite);
    _player->getSceneNode()->addChild(_player->_walkSprite);
    _player->getSceneNode()->addChild(_player->_jumpUpSprite);
    _player->getSceneNode()->addChild(_player->_jumpDownSprite);
    _player->getSceneNode()->addChild(_player->_guardSprite);
    _player->getSceneNode()->addChild(_player->_attackSprite);
    
    #pragma mark hp node
    _hpNode = scene2::Label::allocWithText("100", assetRef->get<Font>(DEBUG_FONT));
    _hpNode->setAnchor(Vec2::ANCHOR_CENTER);
    _hpNode->setForeground(Color4::CYAN);
    _hpNode->setPosition(0, 55);
    _player->getSceneNode()->addChild(_hpNode);
    
    _player->setDebugColor(DEBUG_COLOR);
    
	CULog("Inited playercontoller");
}


void PlayerController::dispose() {
    _input->dispose();
}

/**
 * Resets the player to it's initial position
 */
void PlayerController::reset() {
    _player->resetAttributes();
    _player->setPosition(INIT_POS);
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
        CULog("Applying jump impulse to player");
        b2Vec2 force(0, _player->getJumpF());
        playerBody->ApplyLinearImpulseToCenter(force,true);
    }
#pragma mark dash force
    // Dash!
    if (_player->isDashLeftBegin()){
        CULog("player dashing left begin");
        _player->faceLeft();
        // b2Vec2 force(-_player->getDashF(),0);
        // _body->ApplyLinearImpulseToCenter(force, true); // Old method of dashing
        playerBody->SetLinearVelocity(b2Vec2(-_player->getDashF(), playerBody->GetLinearVelocity().y));
    }
    if (_player->isDashRightBegin()){
        CULog("player dashing right begin");
        _player->faceRight();
        // b2Vec2 force(DASH, 0);
        // _body->ApplyLinearImpulseToCenter(force, true);
        playerBody->SetLinearVelocity(b2Vec2(_player->getDashF(), playerBody->GetLinearVelocity().y));
    }
#pragma mark knockback force
    if (_player->isKnocked()) {
        playerBody->SetLinearVelocity(b2Vec2(0,0));
        Vec2 knockDirection = _player->getKnockDirection();
        Vec2 knockForce = knockDirection.subtract(Vec2(0,knockDirection.y)).scale(_player->getKnockF());
        playerBody->ApplyLinearImpulseToCenter(b2Vec2(knockForce.x, _player->getKnockF()), true);
    }
    // Velocity too high, clamp it
    if (fabs(_player->getVX()) >= _player->getMaxSpeed() && !_player->isDashActive() && !_player->isKnockbackActive()) {
        //CULog("clamping velocity");
        _player->setVX(SIGNUM(_player->getVX())*_player->getMaxSpeed());
    }
    else if (fabs(_player->getVX()) >= _player->getMaxSpeed()){
        CULog("NOT CLAMPING");
    }
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

    // Process the movement inputs
    _player->setMovement(_player->isGuardActive() ? 0 : _input->getHorizontal()*_player->getForce());
    _player->setStrafeLeft(_input->didStrafeLeft());
    _player->setStrafeRight(_input->didStrafeRight());
    _player->setJumpInput( _input->didJump());
    _player->setDashLeftInput(_input->didDashLeft());
    _player->setDashRightInput(_input->didDashRight());
    _player->setGuardInput(_input->didGuard());
    _player->setShootInput(_input->didFire());
    
    _hpNode->setText(std::to_string((int)_player->getHP()));
    
    applyForce();
    updateCooldowns();
}
#pragma mark fixedUpdate
void PlayerController::fixedUpdate(float timestep)
{
    // CULog("updated playercontroller");
}

#pragma mark postUpdate
void PlayerController::postUpdate(float dt)
{
}

void PlayerController::updateCooldowns()
{
#pragma mark Guard cooldown
    // player inputs guard and cooldown is ready
    if (_player->isGuardBegin()) {
        _player->setGuardCDRem();
        _player->setGuardRem();
        _player->setParryRem();
        _player->setShieldDebugColor(Color4::GREEN);
        CULog("Beginning guard and parry");
    }
    if (_player->isGuardActive() && !_player->isGuardBegin()){
        CULog("Guard is active");
        int guardRem = _player->getGuardRem();
        CULog("Updating guard duration from %d", guardRem);
        _player->setGuardRem(guardRem - 1);
        int parryRem = _player->getParryRem();
        _player->setParryRem(parryRem > 0 ? parryRem - 1 : 0);
        if (parryRem == 0){
            // Parry ending on this frame
            _player->setShieldDebugColor(Color4::BLUE);
            }
        }
    // guard not active, update cooldown
    else if (_player->getGuardCDRem()>0) {
        CULog("Updating guard cooldown from %d", _player->getGuardCDRem());
        int guardCD = _player->getGuardCDRem();
        _player->setGuardCDRem(guardCD - 1);
        if (_player->getGuardCDRem() == 0){
            //end guard
            _player->setShieldDebugColor(DEBUG_COLOR);
        }
    }
    
#pragma mark Jump cooldown
    if (_player->isJumpBegin() && _player->isGrounded()) {
        _player->setJumpCDRem();
    } else {
        int jumpCD = _player->getJumpCDRem();
        _player->setJumpCDRem(jumpCD > 0 ? jumpCD - 1 :0);
    }
#pragma mark Shoot cooldown
    if (_player->isShooting()) {
        _player->setShootCDRem();
    } else {
        int shootCD = _player->getShootCDRem();
        _player->setShootCDRem(shootCD > 0 ? shootCD - 1 : 0);
    }
#pragma mark Knockback cooldown
    if (_player->isKnocked()) {
        CULog("Player is knocked");
        _player->setDashCDRem();
        _player->setGuardCDRem();
        _player->setJumpCDRem();
        _player->setShootCDRem();
        _player->setKnockbackRem();
        _player->resetKnocked();
    } else {
        int kbREM = _player->getKnockbackRem();
        _player->setKnockbackRem(kbREM > 0 ? kbREM - 1 : 0);
    }
#pragma mark dash cooldowns
    if (_player->isDashBegin()) {
//        CULog("Setting player dash cooldowns");
        _player->setDashRem();
        _player->setDashCDRem();
        _player->setDashReset(false); //only needed (and is it really needed?) for keyboard
    }
    else if (_player->getDashCDRem()>0){
//        CULog("Decrementing Dash cooldowns, frames rem from %d and cdrem from %d", _player->getDashRem(), _player->getDashCDRem());
        int dashRem = _player->getDashRem();
        _player->setDashRem(dashRem > 0 ? dashRem - 1 : 0);
        int dashCDRem = _player->getDashCDRem();
        _player->setDashCDRem(dashCDRem > 0 ? dashCDRem - 1 : 0);
    }
    
    // Reset the dash if ready (requires user to stop holding dash key(s) for at least one frame)
    if (!_player->getDashReset() && _player->getDashCDRem() == 0 && !(_player->isDashInput())) {
//        CULog("Resetting dash");
        _player->setDashReset(true); // ready to dash again
    }
    
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

