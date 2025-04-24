//
//  GBPlayerModel.cpp
//  PlatformDemo
//
//  This encapsulates all of the information for the character avatar.  Note how this
//  class combines physics and animation.  This is a good template for models in
//  your game.
//
//  WARNING: There are a lot of shortcuts in this design that will do not adapt well
//  to data driven design.  This demo has a lot of simplifications to make it a bit
//  easier to see how everything fits together.  However, the model classes and how
//  they are initialized will need to be changed if you add dynamic level loading.
//
//  Pay close attention to how this class designed.  Subclasses of Cocos2d classes
//  (which are all subclasses of the class Ref) should never have normal public
//  constructors.  Instead, you should organize their constructors into three parts,
//  just like we have done in this class.
//
//  NORMAL CONSTRUCTOR:
//  The standard constructor should be protected (not private).  It should only
//  initialize pointers to nullptr and primitives to their defaults (pointers are
//  not always nullptr to begin with).  It should NOT take any arguments and should
//  not allocate any memory or call any methods.
//
//  STATIC CONSTRUCTOR
//  This is a static method that allocates the object and initializes it.  If
//  initialization fails, it immediately disposes of the object.  Otherwise, it
//  returns an autoreleased object, starting the garbage collection system.
//  These methods all look the same.  You can copy-and-paste them from sample code.
//  The only difference is the init method called.
//
//  INIT METHOD
//  This is a protected method that acts like what how would normally think a
//  constructor might work.  It allocates memory and initializes all values
//  according to provided arguments.  As memory allocation can fail, this method
//  needs to return a boolean indicating whether or not initialization was
//  successful.
//
//  This file is based on the CS 3152 PhysicsDemo Lab by Don Holden, 2007
//
//  Author: Walker White and Anthony Perello
//  Version:  2/9/17
//
#include "GBBoss2Model.h"
#include <cugl/scene2/CUPolygonNode.h>
#include <cugl/scene2/CUTexturedNode.h>
#include <cugl/core/assets/CUAssetManager.h>
#include "../actionmodel_variants/GBRangedActionModel.h"

using namespace cugl;
using namespace graphics;
#pragma mark -
#pragma mark Constructors

/**
 * Initializes a new dude at the given position.
 *
 * The dude is sized according to the given drawing scale.
 *
 * The scene graph is completely decoupled from the physics system.
 * The node does not have to be the same size as the physics body. We
 * only guarantee that the scene graph node is positioned correctly
 * according to the drawing scale.
 *
 * @param pos   Initial position in world coordinates
 * @param size  The size of the dude in world units
 * @param scale The drawing scale (world to screen)
 *
 * @return  true if the obstacle is initialized properly, false otherwise.
 */
bool Boss2Model::init(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& enemyJSON, const Vec2& pos, std::vector<std::shared_ptr<ActionModel>> actions) {
    return EnemyModel::init(assetRef, enemyJSON, pos, actions);
};

void Boss2Model::attachNodes(const std::shared_ptr<AssetManager>& assetRef) {
    _node = scene2::SceneNode::alloc();
    setSceneNode(_node);
    //move this to new function
    _idleSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss2_idle"), 3, 5, 15);
    _idleSprite->setPosition(0, 60);
	_idleSprite->setName("idle");

    _stunSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss2_stun_short"), 3, 5, 15);
    _stunSprite->setPosition(0, 60);
	_stunSprite->setName("stun");

    _shortFireStartSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss2_shortFire_start"), 3, 5, 15);
    _shortFireStartSprite->setPosition(0, 60);
    _shortFireStartSprite->setName("shortFireStart");

    _shortFireAttackSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss2_shortFire_attack"), 1, 5, 5);
    _shortFireAttackSprite->setPosition(0, 60);
    _shortFireAttackSprite->setName("shortFireAttack");

    _shortFireWaitSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss2_shortFire_wait"), 1, 5, 5);
    _shortFireWaitSprite->setPosition(0, 60);
    _shortFireWaitSprite->setName("shortFireWait");

    _shortFireEndSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss2_shortFire_end"), 2, 4, 8);
    _shortFireEndSprite->setPosition(0, 60);
    _shortFireEndSprite->setName("shortFireEnd");

    _headFireStartSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss2_headFire_start"), 3, 5, 15);
    _headFireStartSprite->setPosition(0, 60);
    _headFireStartSprite->setName("headFireStart");

    _headFireAttackSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss2_headFire_attack"), 3, 5, 15);
    _headFireAttackSprite->setPosition(0, 60);
    _headFireAttackSprite->setName("headFireAttack");

    _headFireWaitSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss2_headFire_wait"), 2, 5, 10);
    _headFireWaitSprite->setPosition(0, 60);
    _headFireWaitSprite->setName("headFireWait");

    _headFireEndSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss2_headFire_end"), 3, 5, 15);
    _headFireEndSprite->setPosition(0, 60);
    _headFireEndSprite->setName("headFireEnd");

    _teleportStartSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss2_teleport_start"), 3, 5, 13);
    _teleportStartSprite->setPosition(0, 60);
    _teleportStartSprite->setName("teleportStart");

    _teleportEndSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss2_teleport_end"), 3, 5, 15);
    _teleportEndSprite->setPosition(0, 60);
    _teleportEndSprite->setName("teleportEnd");

    _laserSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss2_laser"), 15, 4, 60);
    _laserSprite->setPosition(0, 60);
    _laserSprite->setName("laser");

    _laserVFXSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("laser"), 19, 2, 38);
    _laserVFXSprite->setPosition(1150, 0);
    _laserVFXSprite->setName("laser_vfx");

	_deadSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss2_dead"), 15, 4, 58);
	_deadSprite->setPosition(0, 60);
	_deadSprite->setName("dead");

    getSceneNode()->addChild(_idleSprite);
    getSceneNode()->addChild(_stunSprite);
	getSceneNode()->addChild(_shortFireStartSprite);
    getSceneNode()->addChild(_shortFireAttackSprite);
    getSceneNode()->addChild(_shortFireWaitSprite);
    getSceneNode()->addChild(_shortFireEndSprite);
    getSceneNode()->addChild(_headFireStartSprite);
    getSceneNode()->addChild(_headFireAttackSprite);
    getSceneNode()->addChild(_headFireWaitSprite);
    getSceneNode()->addChild(_headFireEndSprite);
	getSceneNode()->addChild(_teleportStartSprite);
	getSceneNode()->addChild(_teleportEndSprite);

    getSceneNode()->addChild(_laserVFXSprite);
	getSceneNode()->addChild(_laserSprite);
}

void Boss2Model::setActions(std::vector<std::shared_ptr<ActionModel>> actions) {
    for (auto act : actions) {
        if (act->getActionName() == "shortFire") {
            _shortFire = std::dynamic_pointer_cast<RangedActionModel>(act);
        }
        if (act->getActionName() == "laser") {
			_laser = std::dynamic_pointer_cast<MeleeActionModel>(act);
        }
		if (act->getActionName() == "headFire") {
			_headFire = std::dynamic_pointer_cast<RangedActionModel>(act);
		}
    }
    _closeDistance = 12;
    _farDistance = 19;
}

#pragma mark -
#pragma mark Physics Methods
/**
 * Create new fixtures for this body, defining the shape
 *
 * This is the primary method to override for custom physics objects
 */
void Boss2Model::createFixtures() {
    EnemyModel::createFixtures();
}

/**
 * Release the fixtures for this body, reseting the shape
 *
 * This is the primary method to override for custom physics objects.
 */
void Boss2Model::releaseFixtures() {
    EnemyModel::releaseFixtures();
}

/**
 * Disposes all resources and assets of this PlayerModel
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a PlayerModel may not be used until it is initialized again.
 */
void Boss2Model::dispose() {
    _geometry = nullptr;
    _node = nullptr;
    _groundSensorNode = nullptr;
    _geometry = nullptr;
    _currentSpriteNode = nullptr;
    _idleSprite = nullptr;
    _walkSprite = nullptr;
    _stunSprite = nullptr;
	_shortFireStartSprite = nullptr;
    _shortFireAttackSprite = nullptr;
    _shortFireWaitSprite = nullptr;
    _shortFireEndSprite = nullptr;
	_headFireStartSprite = nullptr;
	_headFireAttackSprite = nullptr;
	_headFireWaitSprite = nullptr;
	_headFireEndSprite = nullptr;
	_teleportStartSprite = nullptr;
	_teleportEndSprite = nullptr;
	_laserSprite = nullptr;
	_laserVFXSprite = nullptr;
    _deadSprite = nullptr;
}

void Boss2Model::damage(float value) {
    EnemyModel::damage(value);
    if (_isShortFireAttacking || _isShortFireWaiting || _isHeadFireAttacking || _isHeadFireWaiting) {
        setStun(60);
    }
}

#pragma mark Cooldowns
/**
 * Updates the object's physics state (NOT GAME LOGIC).
 *
 * We use this method to reset cooldowns.
 *
 * @param delta Number of seconds since last animation frame
 */
void Boss2Model::update(float dt) {
    if (isRemoved()) return;

    BoxObstacle::update(dt);
    if (_node != nullptr) {
        _node->setPosition(getPosition() * _drawScale);
        _node->setAngle(getAngle());
    }

    if (_lastDamagedFrame < ENEMY_HIT_COLOR_DURATION) {
		_lastDamagedFrame++;
    }

    if (_lastDamagedFrame == ENEMY_HIT_COLOR_DURATION) {
        _node->setColor(Color4::WHITE);
    }
}

#pragma mark -
#pragma mark AI Methods
void Boss2Model::nextAction() {
    int r = rand();
    AIMove();
    if (_moveDuration <= 0 && !isStunned() && !_isShootingLaser && !_isShortFiring && !_isHeadFiring && !_isTeleportStarting && !_isTeleportEnding) {
        if (isTargetClose()) {
            if (r % 2 == 0) {
                shortFire();
            }
            else {
                teleport();
            }
        }
        else if (isTargetFar()) {
            if (r % 3 == 0) {
                teleport();
            }
            else if (r % 3 == 1){
                shortFire();
            }
            else {
				headFire();
            }
        }
        else {
            if (r % 3 == 0) {
                shortFire();
            }
            else if (r % 3 == 1) {
                laser();
            }
            else {
                teleport();
            }
        }
    }
    else {
        if (isStunned()) {
            _isShortFiring = false;
			_isShortFireStarting = false;
			_isShortFireAttacking = false;
			_isShortFireWaiting = false;
			_isShortFireEnding = false;

            _isHeadFiring = false;
            _isHeadFireStarting = false;
            _isHeadFireAttacking = false;
            _isHeadFireWaiting = false;
            _isHeadFireEnding = false;

			_isTeleportStarting = false;
			_isTeleportEnding = false;
            _isShootingLaser = false;
			
            setMovement(0);
        }
        if (_isShortFiring) {
			handleShortFire();
        }
        if (_isHeadFiring) {
            handleHeadFire();
        }
        if (_isTeleportStarting && _teleportStartSprite->getFrame() >= _teleportStartSprite->getCount() - 1) {
            _isTeleportStarting = false;
			_isTeleportEnding = true;
            float newPos;
            if (_targetPos.x < getPosition().x) {
                newPos = _targetPos.x - 16 < worldLeft ? _targetPos.x + 20 : _targetPos.x - 16;
            }
            else {
				newPos = _targetPos.x + 16 > worldRight ? _targetPos.x - 20 : _targetPos.x + 16;
            }
            setPosition(newPos, getPosition().y);
            setMovement(0);
        }
        if (_isTeleportEnding && _teleportEndSprite->getFrame() >= _teleportEndSprite->getCount() - 1) {
            faceTarget();
			_isTeleportEnding = false;
			setEnabled(true);
            setMovement(0);
        }
        if (_isShootingLaser && _laserSprite->getFrame() >= _laserSprite->getCount() - 1) {
			_isShootingLaser = false;
            setMovement(0);
        }
    }
}

void Boss2Model::AIMove() {
    float dist = getPosition().x - _targetPos.x;
    float dir_val = dist > 0 ? -1 : 1;
    int face = _faceRight ? 1 : -1;

    if (_moveDuration > 0) {
        setMovement(_moveDirection * dir_val * getForce());
        setMoveLeft(dist > 0);
        setMoveRight(dist < 0);
        _moveDuration--;
    }
    else {
        setMovement(0);
    }
}

void Boss2Model::shortFire() {
	faceTarget();
    _isShortFiring = true;
}

void Boss2Model::headFire() {
    faceTarget();
    _isHeadFiring = true;
}

void Boss2Model::handleShortFire() {
    setMovement(0);
    if (!_isShortFireStarting && !_isShortFireAttacking && !_isShortFireWaiting && !_isShortFireEnding) {
        _isShortFireStarting = true;
        _shortFireCount = 0;
    }
    else if (_isShortFireStarting && _shortFireStartSprite->getFrame() >= _shortFireStartSprite->getCount() - 1) {
        _isShortFireStarting = false;
        _isShortFireAttacking = true;
        _shortFireCount++;
    }
    else if (_isShortFireAttacking && _shortFireAttackSprite->getFrame() >= _shortFireAttackSprite->getCount() - 1) {
        if (_shortFireCount >= 3) { // stop repeated shooting
            _isShortFireAttacking = false;
            _isShortFireEnding = true;
        }
        else { // wait for a bit
            _isShortFireAttacking = false;
            _isShortFireWaiting = true;
        }
    }
    else if (_isShortFireWaiting && _shortFireWaitSprite->getFrame() >= _shortFireWaitSprite->getCount() - 1) {
        if (_shortFireCount >= 3) { // stop repeated shooting
            _isShortFireWaiting = false;
            _isShortFireEnding = true;
        }
        else { // return to shooting
            _isShortFireWaiting = false;
            _isShortFireAttacking = true;
            _shortFireCount++;
        }
    }
    else if (_isShortFireEnding && _shortFireEndSprite->getFrame() >= _shortFireEndSprite->getCount() - 1) {
        _isShortFireEnding = false;
        _isShortFiring = false;
    }
}

void Boss2Model::handleHeadFire() {
    setMovement(0);
    if (!_isHeadFireStarting && !_isHeadFireAttacking && !_isHeadFireWaiting && !_isHeadFireEnding) {
        _isHeadFireStarting = true;
        _headFireCount = 0;
    }
    else if (_isHeadFireStarting && _headFireStartSprite->getFrame() >= _headFireStartSprite->getCount() - 1) {
        _isHeadFireStarting = false;
        _isHeadFireAttacking = true;
        _headFireCount++;
    }
    else if (_isHeadFireAttacking && _headFireAttackSprite->getFrame() >= _headFireAttackSprite->getCount() - 1) {
        if (_headFireCount >= 3) { // stop repeated shooting
            _isHeadFireAttacking = false;
            _isHeadFireEnding = true;
        }
        else { // wait for a bit
            _isHeadFireAttacking = false;
            _isHeadFireWaiting = true;
        }
    }
    else if (_isHeadFireWaiting && _headFireWaitSprite->getFrame() >= _headFireWaitSprite->getCount() - 1) {
        if (_headFireCount >= 3) { // stop repeated shooting
            _isHeadFireWaiting = false;
            _isHeadFireEnding = true;
        }
        else { // return to shooting
            _isHeadFireWaiting = false;
            _isHeadFireAttacking = true;
            _headFireCount++;
        }
    }
    else if (_isHeadFireEnding && _headFireEndSprite->getFrame() >= _headFireEndSprite->getCount() - 1) {
        _isHeadFireEnding = false;
        _isHeadFiring = false;
    }
}

void Boss2Model::teleport() {
    faceTarget();
	setEnabled(false);
	_isTeleportStarting = true;
}

void Boss2Model::laser() {
    faceTarget();
	_isShootingLaser = true;
}

std::shared_ptr<MeleeActionModel> Boss2Model::getDamagingAction() {
    if (_isShootingLaser && _laserSprite->getFrame() == _laser->getHitboxStartFrame() - 1) {
        return _laser;
    }
    return nullptr;
}

std::shared_ptr<Projectile> Boss2Model::getProjectile() {
	std::vector<int> frames = _shortFire->getProjectileSpawnFrames();
    int count = 0;
    for (int frame : frames) {
		if (_isShortFireAttacking && _shortFireAttackSprite->getFrame() == frame && frameCounter == 0) {
			return _shortFire->getProjectiles()[count];
		}
        count++;
    }
	frames = _headFire->getProjectileSpawnFrames();
    count = 0;
	for (int frame : frames) {
		if (_isHeadFireAttacking && _headFireAttackSprite->getFrame() == frame && frameCounter == 0) {
            return _headFire->getProjectiles()[count];
		}
        count++;
	}
    
    return nullptr;
}

#pragma mark -
#pragma mark Animation Methods

void Boss2Model::updateAnimation()
{

    _stunSprite->setVisible(isStunned());

	_shortFireStartSprite->setVisible(!isStunned() && _isShortFireStarting);
    _shortFireAttackSprite->setVisible(!isStunned() && _isShortFireAttacking);
    _shortFireWaitSprite->setVisible(!isStunned() && _isShortFireWaiting);
    _shortFireEndSprite->setVisible(!isStunned() && _isShortFireEnding);

	_headFireStartSprite->setVisible(!isStunned() && _isHeadFireStarting);
	_headFireAttackSprite->setVisible(!isStunned() && _isHeadFireAttacking);
	_headFireWaitSprite->setVisible(!isStunned() && _isHeadFireWaiting);
	_headFireEndSprite->setVisible(!isStunned() && _isHeadFireEnding);

	_teleportStartSprite->setVisible(!isStunned() && _isTeleportStarting);
	_teleportEndSprite->setVisible(!isStunned() && _isTeleportEnding);
	_laserSprite->setVisible(!isStunned() && _isShootingLaser);

    _laserVFXSprite->setVisible(_laserSprite->isVisible() && _laserSprite->getFrame() >= 17 && _laserSprite->getFrame() <= 55);

    _idleSprite->setVisible(!isStunned() && !_isShootingLaser && !_isTeleportStarting && !_isTeleportEnding 
        && !_isShortFireStarting && !_isShortFireAttacking && !_isShortFireWaiting && !_isShortFireEnding 
        && !_isHeadFireStarting && !_isHeadFireAttacking && !_isHeadFireWaiting && !_isHeadFireEnding
        && !(isMoveLeft() || isMoveRight()));

    playAnimation(_idleSprite);
    playAnimation(_stunSprite);

    playAnimation(_shortFireStartSprite);
    playAnimation(_shortFireAttackSprite);
    playAnimation(_shortFireWaitSprite);
    playAnimation(_shortFireEndSprite);

	playAnimation(_headFireStartSprite);
	playAnimation(_headFireAttackSprite);
	playAnimation(_headFireWaitSprite);
	playAnimation(_headFireEndSprite);

    playAnimation(_laserSprite);
    playVFXAnimation(_laserSprite, _laserVFXSprite, 17);

	playAnimation(_teleportStartSprite);
	playAnimation(_teleportEndSprite);

    _node->setScale(Vec2(isFacingRight() ? 1 : -1, 1));
    _node->getChild(_node->getChildCount() - 2)->setScale(Vec2(isFacingRight() ? 1 : -1, 1));
    _node->getChild(_node->getChildCount() - 1)->setScale(Vec2(isFacingRight() ? 1 : -1, 1));
}

#pragma mark -
#pragma mark Scene Graph Methods
/**
 * Redraws the outline of the physics fixtures to the debug node
 *
 * The debug node is use to outline the fixtures attached to this object.
 * This is very useful when the fixtures have a very different shape than
 * the texture (e.g. a circular shape attached to a square texture).
 */
void Boss2Model::resetDebug() {
    EnemyModel::resetDebug();
}



