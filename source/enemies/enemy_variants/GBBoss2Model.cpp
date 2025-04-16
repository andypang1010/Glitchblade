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
    _idleSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss2_idle"), 2, 8, 15);
    _idleSprite->setPosition(0, 50);
	_idleSprite->setName("idle");

    _stunSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss2_stun_short"), 2, 8, 15);
    _stunSprite->setPosition(0, 50);
	_stunSprite->setName("stun");

    _shortFireStartSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss2_shortFire_start"), 2, 8, 15);
    _shortFireStartSprite->setPosition(0, 50);
    _shortFireStartSprite->setName("shortFireStart");

    _shortFireAttackSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss2_shortFire_attack"), 1, 8, 5);
    _shortFireAttackSprite->setPosition(0, 50);
    _shortFireAttackSprite->setName("shortFireAttack");

    _shortFireWaitSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss2_shortFire_wait"), 1, 8, 5);
    _shortFireWaitSprite->setPosition(0, 50);
    _shortFireWaitSprite->setName("shortFireWait");

    _shortFireEndSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss2_shortFire_end"), 1, 9, 8);
    _shortFireEndSprite->setPosition(0, 50);
    _shortFireEndSprite->setName("shortFireEnd");

	_deadSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss2_dead"), 5, 10, 45);
	_deadSprite->setPosition(0, 50);
	_deadSprite->setName("dead");

    getSceneNode()->addChild(_idleSprite);
    getSceneNode()->addChild(_stunSprite);
	getSceneNode()->addChild(_shortFireStartSprite);
    getSceneNode()->addChild(_shortFireAttackSprite);
    getSceneNode()->addChild(_shortFireWaitSprite);
    getSceneNode()->addChild(_shortFireEndSprite);
}

void Boss2Model::setActions(std::vector<std::shared_ptr<ActionModel>> actions) {
    for (auto act : actions) {
        if (act->getActionName() == "shortFire") {
            _shortFire = std::dynamic_pointer_cast<RangedActionModel>(act);
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
    _sensorNode = nullptr;
    _geometry = nullptr;
    _currentSpriteNode = nullptr;
    _idleSprite = nullptr;
    _walkSprite = nullptr;
    _stunSprite = nullptr;
	_shortFireStartSprite = nullptr;
    _shortFireAttackSprite = nullptr;
    _shortFireWaitSprite = nullptr;
    _shortFireEndSprite = nullptr;
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
    if (_moveDuration <= 0 && !isStunned()) {
        
    }
    else {
        if (isStunned()) {
            _isShortFiring = false;
            setMovement(0);
        }
        if (_isShortFiring && _shortFireAttackSprite->getFrame() >= SHOOT_FRAMES - 1) {
            _isShortFiring = false;
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
        setMovement(getMovement() / 3);
    }
}

void Boss2Model::shortFire() {
	faceTarget();
}

std::shared_ptr<MeleeActionModel> Boss2Model::getDamagingAction() {
    return nullptr;
}

std::shared_ptr<RangedActionModel> Boss2Model::getProjectileAction() {
	std::vector<int> frames = _shortFire->getProjectileSpawnFrames();
    for (int frame : frames) {
		if (_isShortFiring && _shortFireAttackSprite->getFrame() == frame && frameCounter == 0) {
			return _shortFire;
		}
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

    _idleSprite->setVisible(!isStunned() && !(isMoveLeft() || isMoveRight()));

    playAnimation(_idleSprite);
    playAnimation(_stunSprite);

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



