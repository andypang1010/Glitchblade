#include <cugl/scene2/CUPolygonNode.h>
#include <cugl/scene2/CUTexturedNode.h>
#include <cugl/core/assets/CUAssetManager.h>
#include "../actionmodel_variants/GBRangedActionModel.h"
#include "GBMinion2BModel.h"

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
bool Minion2BModel::init(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& enemyJSON, const Vec2& pos, std::vector<std::shared_ptr<ActionModel>> actions) {
    return EnemyModel::init(assetRef, enemyJSON, pos, actions);
};


void Minion2BModel::attachNodes(const std::shared_ptr<AssetManager>& assetRef) {
    EnemyModel::attachNodes(assetRef);
    float scale = 0.4;
    float YOffset = 20;
    //move this to new function
    _idleSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion2B_idle"), 3, 4, 10);
	_idleSprite->setScale(1.44 * scale * 0.0004006410 * Application::get()->getDisplayWidth());
    _idleSprite->setPosition(0, YOffset * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
	_idleSprite->setName("idle");

    _walkSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion2B_walk"), 3, 4, 10);
    _walkSprite->setScale(1.44 * scale * 0.0004006410 * Application::get()->getDisplayWidth());
    _walkSprite->setPosition(0, YOffset * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
	_walkSprite->setName("walk");

    _guardSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion2B_guard"), 1, 3, 3);
    _guardSprite->setScale(1.44 * scale * 0.0004006410 * Application::get()->getDisplayWidth());
    _guardSprite->setPosition(0, YOffset * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _guardSprite->setName("guard");

    _attackSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion2B_attack"), 1, 5, 5);
    _attackSprite->setScale(1.44 * scale * 0.0004006410 * Application::get()->getDisplayWidth());
    _attackSprite->setPosition(0, YOffset * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _attackSprite->setName("attack");

    _stunSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion2B_stun"), 5, 4, 20);
    _stunSprite->setScale(1.44 * scale * 0.0004006410 * Application::get()->getDisplayWidth());
    _stunSprite->setPosition(0, YOffset * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _stunSprite->setName("stun");
	stunFrames = _stunSprite->getCount() * 4;

    _deadSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion2B_dead"), 4, 4, 14);
    _deadSprite->setScale(1.44 * scale * 0.0004006410 * Application::get()->getDisplayWidth());
    _deadSprite->setPosition(0, YOffset * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
	_deadSprite->setName("dead");

    getSceneNode()->addChild(_idleSprite);
    getSceneNode()->addChild(_walkSprite);
    getSceneNode()->addChild(_guardSprite);
    getSceneNode()->addChild(_attackSprite);
    getSceneNode()->addChild(_stunSprite);
}

void Minion2BModel::setActions(std::vector<std::shared_ptr<ActionModel>> actions){
    for (auto act : actions) {
        if (act->getActionName() == "guard") {
            _guard = std::dynamic_pointer_cast<MeleeActionModel>(act);
        }
        else if (act->getActionName() == "attack") {
            _attack = std::dynamic_pointer_cast<MeleeActionModel>(act);
        }
    }
    _closeDistance = 6;
	_farDistance = 12;
}

#pragma mark -
#pragma mark Physics Methods
/**
 * Create new fixtures for this body, defining the shape
 *
 * This is the primary method to override for custom physics objects
 */
void Minion2BModel::createFixtures() {
    EnemyModel::createFixtures();
}


/**
 * Release the fixtures for this body, reseting the shape
 *
 * This is the primary method to override for custom physics objects.
 */
void Minion2BModel::releaseFixtures() {
    EnemyModel::releaseFixtures();
}


/**
 * Disposes all resources and assets of this PlayerModel
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a PlayerModel may not be used until it is initialized again.
 */
void Minion2BModel::dispose() {
    _geometry = nullptr;
    _node = nullptr;
    _groundSensorNode = nullptr;
    _geometry = nullptr;
    _idleSprite = nullptr;
    _walkSprite = nullptr;
    _guardSprite = nullptr;
    _attackSprite = nullptr;
    _stunSprite = nullptr;
    _deadSprite = nullptr;

}

void Minion2BModel::damage(float value) {
	bool targetAtFront = ((_targetPos - getPosition()).x > 0 && isFacingRight()) || ((_targetPos - getPosition()).x < 0 && !isFacingRight());
    if(targetAtFront && _targetPos.y <= getPosition().y + 2.1 && !isStunned()) {
        if (value <= 10) {
            _attackGuarded = true;
            _moveDuration = 0;
        }
        else {
            setStun(stunFrames);
        }
    }
    else {
        EnemyModel::damage(value);
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
void Minion2BModel::update(float dt) {
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
void Minion2BModel::nextAction() {
    int r = rand();
    AIMove();
    if (_moveDuration <= 0 && !isStunned() && !_isGuarding && !_isAttacking) {
        if (_attackGuarded) {
            guard();
        }
        else if (isTargetClose() && r % 100 == 0) {
            attack(180);
        }
        else if (!isTargetClose()) {
            approachTarget(45);
        }
    }
    else {
        if (isStunned()) {
            _isGuarding = false;
            _isAttacking = false;
            setMovement(0);
        }
        if (_isGuarding && _guardSprite->getFrame() >= _guardSprite->getCount() - 1) {
            _isGuarding = false;
            setMovement(0);
        }
        if (_isAttacking && _attackDuration <= 0) {
            _isAttacking = false;
            setMovement(0);
        }
    }
}

void Minion2BModel::AIMove() {
    float dist = getPosition().x - _targetPos.x;
    float dir_val = dist > 0 ? -1 : 1;
    int face = _faceRight ? 1 : -1;

    if (_moveDuration > 0) {
        setMovement(_moveDirection * dir_val * getForce());
        setMoveLeft(dist > 0);
        setMoveRight(dist < 0);
        _moveDuration--;
    }
    else if (_attackDuration > 0) {
        setMovement(face * getForce() * 3);
        setMoveLeft(dist > 0);
        setMoveRight(dist < 0);
        _attackDuration--;
    }
    else {
        setMovement(0);
    }
}

void Minion2BModel::guard() {
    faceTarget();
    _isGuarding = true;
    _attackGuarded = false;
    setMovement(0);
}

void Minion2BModel::attack(int duration) {
	faceTarget();
    _isAttacking = true;
	_attackDuration = duration;
    setMovement(0);
}

std::shared_ptr<MeleeActionModel> Minion2BModel::getDamagingAction() {
    if (_isGuarding && _guardSprite->getFrame() == _guard->getHitboxStartFrame() - 1) {
        return _guard;
    }
    if (_isAttacking && _attackSprite->getFrame() == _attack->getHitboxStartFrame() - 1) {
        return _attack;
    }
    return nullptr;
}

#pragma mark -
#pragma mark Animation Methods

void Minion2BModel::updateAnimation()
{

    _stunSprite->setVisible(isStunned());

    _walkSprite->setVisible(!isStunned() && !_isGuarding && !_isAttacking && _moveDuration > 0);

    _guardSprite->setVisible(!isStunned() && _isGuarding);

    _attackSprite->setVisible(!isStunned() && _isAttacking);

    _idleSprite->setVisible(!isStunned() && !_isGuarding && !_isAttacking && _moveDuration <= 0);

    playAnimation(_walkSprite);
    playAnimation(_idleSprite);
    playAnimationOnce(_guardSprite);
    playAnimation(_attackSprite);
    playAnimationOnce(_stunSprite);

    _node->setScale(1.44 * Vec2(isFacingRight() ? 1 : -1, 1));
    _node->getChild(_node->getChildCount() - 2)->setScale(1.44 * Vec2(isFacingRight() ? 1 : -1, 1));
    _node->getChild(_node->getChildCount() - 1)->setScale(1.44 * Vec2(isFacingRight() ? 1 : -1, 1));
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
void Minion2BModel::resetDebug() {
    EnemyModel::resetDebug();
}
