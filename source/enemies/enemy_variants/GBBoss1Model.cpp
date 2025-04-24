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
#include "GBBoss1Model.h"
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
bool Boss1Model::init(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& enemyJSON, const Vec2& pos, std::vector<std::shared_ptr<ActionModel>> actions) {
    return EnemyModel::init(assetRef, enemyJSON, pos, actions);
};

void Boss1Model::attachNodes(const std::shared_ptr<AssetManager>& assetRef) {
    _node = scene2::SceneNode::alloc();
    setSceneNode(_node);
    //move this to new function
    _idleSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss1_idle"), 1, 5, 5);
    _idleSprite->setPosition(0, 50);
	_idleSprite->setName("idle");

    _walkSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss1_walk"), 1, 7, 7);
    _walkSprite->setPosition(0, 50);
	_walkSprite->setName("walk");

    _slamSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss1_slam"), 8, 5, 40);
    _slamSprite->setPosition(0, 50);
	_slamSprite->setName("slam");

    _stabSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss1_stab"), 8, 5, 40);
    _stabSprite->setPosition(0, 50);
	_stabSprite->setName("stab");

    _stunSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss1_stun"), 7, 6, 38);
    _stunSprite->setPosition(0, 50);
	_stunSprite->setName("stun");
    stunFrames = _stunSprite->getCount() * 4;

    _shootSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss1_shoot"), 3, 5, 15);
    _shootSprite->setPosition(0, 50);
	_shootSprite->setName("shoot");

    _explodeSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss1_explode"), 11, 5, 53);
    _explodeSprite->setPosition(0, 50);
	_explodeSprite->setName("explode");

	_explodeVFXSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("explode_enemy_1"), 4, 8, 32);
	_explodeVFXSprite->setPosition(0, 0);
	_explodeVFXSprite->setName("explode_vfx");

	_deadSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss1_dead"), 12, 5, 60);
	_deadSprite->setPosition(0, 50);
	_deadSprite->setName("dead");

    getSceneNode()->addChild(_idleSprite);
    getSceneNode()->addChild(_walkSprite);
    getSceneNode()->addChild(_slamSprite);
    getSceneNode()->addChild(_stabSprite);
    getSceneNode()->addChild(_stunSprite);
	getSceneNode()->addChild(_shootSprite);

	getSceneNode()->addChild(_explodeSprite);
    getSceneNode()->addChild(_explodeVFXSprite);

}

void Boss1Model::setActions(std::vector<std::shared_ptr<ActionModel>> actions) {
    for (auto act : actions) {
        if (act->getActionName() == "slam") {
            _slam = std::dynamic_pointer_cast<MeleeActionModel>(act);
        }
        else if (act->getActionName() == "stab") {
            _stab = std::dynamic_pointer_cast<MeleeActionModel>(act);
        }
        else if (act->getActionName() == "explode") {
            _explode = std::dynamic_pointer_cast<MeleeActionModel>(act);
        }
        else if (act->getActionName() == "shoot") {
            _shoot = std::dynamic_pointer_cast<RangedActionModel>(act);
        }
    }
}

#pragma mark -
#pragma mark Physics Methods
/**
 * Create new fixtures for this body, defining the shape
 *
 * This is the primary method to override for custom physics objects
 */
void Boss1Model::createFixtures() {
    EnemyModel::createFixtures();
}

/**
 * Release the fixtures for this body, reseting the shape
 *
 * This is the primary method to override for custom physics objects.
 */
void Boss1Model::releaseFixtures() {
    EnemyModel::releaseFixtures();
}

/**
 * Disposes all resources and assets of this PlayerModel
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a PlayerModel may not be used until it is initialized again.
 */
void Boss1Model::dispose() {
    _geometry = nullptr;
    _node = nullptr;
    _groundSensorNode = nullptr;
    _geometry = nullptr;
    _currentSpriteNode = nullptr;
    _idleSprite = nullptr;
    _walkSprite = nullptr;
    _stabSprite = nullptr;
    _slamSprite = nullptr;
    _stunSprite = nullptr;
	_shootSprite = nullptr;
	_explodeSprite = nullptr;
    _deadSprite = nullptr;
}

#pragma mark Cooldowns
/**
 * Updates the object's physics state (NOT GAME LOGIC).
 *
 * We use this method to reset cooldowns.
 *
 * @param delta Number of seconds since last animation frame
 */
void Boss1Model::update(float dt) {
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
void Boss1Model::nextAction() {
    int r = rand();
    AIMove();
    if (!_isSlamming && !_isStabbing && !_isShooting && !_isExploding && _moveDuration <= 0 && !isStunned()) {
        if (isTargetClose()) {
            if (r % 4 == 0) { //Slam
                slam();
            }
            else if (r % 4 == 1) { // Stab
                stab();
            }
            else if (r % 4 == 2) { // Explode
                explode();
            }
            else { // Move away
                avoidTarget(45);
            }
        }
        else if (isTargetFar()) {
            if (r % 2 == 0) { // Shoot
                shoot();
            }
            else { // Move closer
                approachTarget(45);
            }
        }
        else {
            if (r % 2 == 0) { // Stab
                stab();
            }
            else { // Move closer
                approachTarget(45);
            }
        }
    }
    else {
        if (isStunned()) {
            _isSlamming = false;
            _isStabbing = false;
			_isShooting = false;
			_isExploding = false;
            setMovement(0);
        }
        if (_isSlamming && _slamSprite->getFrame() >= _slamSprite->getCount() - 1) {
            _isSlamming = false;
            setMovement(0);
        }
        if (_isStabbing && _stabSprite->getFrame() >= _stabSprite->getCount() - 1) {
            _isStabbing = false;
            setMovement(getMovement());
        }
        if (_isShooting && _shootSprite->getFrame() >= _shootSprite->getCount() - 1) {
            _isShooting = false;
            setMovement(0);
        }
        if (_isExploding && _explodeSprite->getFrame() >= _explodeSprite->getCount() - 1) {
            _isExploding = false;
            setMovement(0);
        }
    }
}

void Boss1Model::AIMove() {
    float dist = getPosition().x - _targetPos.x;
    float dir_val = dist > 0 ? -1 : 1;
    int face = _faceRight ? 1 : -1;

    if (_moveDuration > 0 && !_isStabbing) {
        setMovement(_moveDirection * dir_val * getForce());
        setMoveLeft(dist > 0);
        setMoveRight(dist < 0);
        _moveDuration--;
    }

    else if (_isStabbing && _stabSprite->getFrame() >= _stab->getHitboxStartFrame() - 1 && _stabSprite->getFrame() <= _stab->getHitboxEndFrame() - 1) {
        setMovement(face * getForce() * STAB_FORCE * _scale);
    }
    else {
        setMovement(getMovement() / 3);
    }
}

void Boss1Model::slam() {
    faceTarget();
    if (rand() % 100 <= _aggression) {
        _aggression -= std::max(0.0f, _aggression - 25);
        _isSlamming = true;
        setMovement(0);
    }
}

void Boss1Model::stab() {
	faceTarget();
    if (rand() % 100 <= _aggression) {
        _aggression -= std::max(0.0f, _aggression - 25);
        _isStabbing = true;
        setMovement(0);
    }
}

void Boss1Model::shoot() {
	faceTarget();
    if (_aggression >= 75) {
        _aggression -= std::max(0.0f, _aggression - 10);
        _isShooting = true;
        setMovement(0);
    }
}

void Boss1Model::explode() {
	faceTarget();
    if (rand() % 200 <= _aggression) {
        _aggression -= std::max(0.0f, _aggression - 50);
        _isExploding = true;
        setMovement(0);
    }
}

std::shared_ptr<MeleeActionModel> Boss1Model::getDamagingAction() {
    if (_isStabbing && _stabSprite->getFrame() == _stab->getHitboxStartFrame() - 1) {
        return _stab;
    }
    else if (_isSlamming && _slamSprite->getFrame() == _slam->getHitboxStartFrame() - 1) {
        return _slam;
	}
	else if (_isExploding && _explodeSprite->getFrame() == _explode->getHitboxStartFrame() - 1) {
		return _explode;
	}
    return nullptr;
}

std::shared_ptr<Projectile> Boss1Model::getProjectile() {
	std::vector<int> frames = _shoot->getProjectileSpawnFrames();
    int count = 0;
    for (int frame : frames) {
		if (_isShooting && _shootSprite->getFrame() == frame && frameCounter == 0) {
			return _shoot->getProjectiles()[count];
		}
		count++;
    }
    
    return nullptr;
}

#pragma mark -
#pragma mark Animation Methods

void Boss1Model::updateAnimation()
{

    _stunSprite->setVisible(isStunned());

    _walkSprite->setVisible(!isStunned() && !_isStabbing && !_isSlamming && !_isShooting && !_isExploding && (isMoveLeft() || isMoveRight()));

    _slamSprite->setVisible(!isStunned() && _isSlamming);

    _stabSprite->setVisible(!isStunned() && _isStabbing);

	_shootSprite->setVisible(!isStunned() && _isShooting);

	_explodeSprite->setVisible(!isStunned() && _isExploding);

    _explodeVFXSprite->setVisible(_explodeSprite->isVisible() && _explodeSprite->getFrame() >= _explodeSprite->getCount() - _explodeVFXSprite->getCount());

    _idleSprite->setVisible(!isStunned() && !_isStabbing && !_isSlamming && !_isShooting && !_isExploding && !(isMoveLeft() || isMoveRight()));

    playAnimation(_walkSprite);
    playAnimation(_idleSprite);
    playAnimationOnce(_slamSprite);
    playAnimationOnce(_stabSprite);
    playAnimationOnce(_stunSprite);
	playAnimationOnce(_shootSprite);
	playAnimationOnce(_explodeSprite);

    playVFXAnimation(_explodeSprite, _explodeVFXSprite, 24);

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
void Boss1Model::resetDebug() {
    EnemyModel::resetDebug();
}



