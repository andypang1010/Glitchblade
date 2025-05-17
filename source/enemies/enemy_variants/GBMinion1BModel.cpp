#include <cugl/scene2/CUPolygonNode.h>
#include <cugl/scene2/CUTexturedNode.h>
#include <cugl/core/assets/CUAssetManager.h>
#include "../actionmodel_variants/GBRangedActionModel.h"
#include "GBMinion1BModel.h"

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
bool Minion1BModel::init(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& enemyJSON, const Vec2& pos, std::vector<std::shared_ptr<ActionModel>> actions) {
    return EnemyModel::init(assetRef, enemyJSON, pos, actions);
};


void Minion1BModel::attachNodes(const std::shared_ptr<AssetManager>& assetRef) {
    EnemyModel::attachNodes(assetRef);
    //move this to new function
    _idleSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion1B_idle"), 5, 2, 10);
	_idleSprite->setScale(1.44 * 0.0004006410 * Application::get()->getDisplayWidth());
    _idleSprite->setPosition(0, 10 * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
	_idleSprite->setName("idle");

    _walkSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion1B_walk"), 5, 2, 10);
    _walkSprite->setScale(1.44 * 0.0004006410 * Application::get()->getDisplayWidth());
    _walkSprite->setPosition(0, 10 * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
	_walkSprite->setName("walk");

    _punchSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion1B_punch"), 8, 4, 30);
    _punchSprite->setScale(1.44 * 0.0004006410 * Application::get()->getDisplayWidth());
    _punchSprite->setPosition(0, 10 * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
	_punchSprite->setName("punch");

    _slamSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion1B_slam"), 8, 4, 32);
    _slamSprite->setScale(1.44 * 0.0004006410 * Application::get()->getDisplayWidth());
    _slamSprite->setPosition(0, 10 * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
	_slamSprite->setName("slam");

	_slamVFXSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("wave_enemy_1"), 1, 8, 8);
	_slamVFXSprite->setScale(0.4f * 2 * 0.0004006410 * Application::get()->getDisplayWidth());
	_slamVFXSprite->setPosition(150 * 2 * 0.0004006410 * Application::get()->getDisplayWidth(), -5 * 2 * 0.0004006410 * Application::get()->getDisplayWidth());
	_slamVFXSprite->setName("slam_vfx");

    _stunSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion1B_stun"), 5, 4, 20);
    _stunSprite->setScale(1.44 * 0.0004006410 * Application::get()->getDisplayWidth());
    _stunSprite->setPosition(0, 10 * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _stunSprite->setName("stun");
	stunFrames = _stunSprite->getCount() * 4;

    _deadSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion1B_dead"), 5, 3, 15);
    _deadSprite->setScale(1.44 * 0.0004006410 * Application::get()->getDisplayWidth());
    _deadSprite->setPosition(0, 10 * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
	_deadSprite->setName("dead");

    getSceneNode()->addChild(_idleSprite);
    getSceneNode()->addChild(_walkSprite);
    getSceneNode()->addChild(_punchSprite);
    getSceneNode()->addChild(_stunSprite);

    getSceneNode()->addChild(_slamSprite);
	getSceneNode()->addChild(_slamVFXSprite);
}

void Minion1BModel::setActions(std::vector<std::shared_ptr<ActionModel>> actions){
    for (auto act : actions) {
        if (act->getActionName() == "slam") {
            _slam = std::dynamic_pointer_cast<MeleeActionModel>(act);
        }
        else if (act->getActionName() == "punch") {
            _punch = std::dynamic_pointer_cast<MeleeActionModel>(act);
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
void Minion1BModel::createFixtures() {
    EnemyModel::createFixtures();
}


/**
 * Release the fixtures for this body, reseting the shape
 *
 * This is the primary method to override for custom physics objects.
 */
void Minion1BModel::releaseFixtures() {
    EnemyModel::releaseFixtures();
}


/**
 * Disposes all resources and assets of this PlayerModel
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a PlayerModel may not be used until it is initialized again.
 */
void Minion1BModel::dispose() {
    _geometry = nullptr;
    _node = nullptr;
    _groundSensorNode = nullptr;
    _geometry = nullptr;
    _idleSprite = nullptr;
    _walkSprite = nullptr;
    _punchSprite = nullptr;
    _slamSprite = nullptr;
    _stunSprite = nullptr;
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
void Minion1BModel::update(float dt) {
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
void Minion1BModel::nextAction() {
    int r = rand();
    AIMove();
    if (!_isSlamming && !_isPunching && _moveDuration <= 0 && !isStunned()) {
        if (isTargetClose()) {
            if (r % 2 == 0) { // Punch
                punch();
            }
            else {
                slam();
            }
        }
        else {
            approachTarget(45);
        }
    }
    else {
        if (isStunned()) {
            _isSlamming = false;
            _isPunching = false;
            setMovement(0);
        }
        if (_isSlamming && _slamSprite->getFrame() >= _slamSprite->getCount() - 1) {
            _isSlamming = false;
            setMovement(0);
        }
        if (_isPunching && _punchSprite->getFrame() >= _punchSprite->getCount() - 1) {
            _isPunching = false;
            setMovement(0);
        }
    }
}

void Minion1BModel::AIMove() {
    float dist = getPosition().x - _targetPos.x;
    float dir_val = dist > 0 ? -1 : 1;
    int face = _faceRight ? 1 : -1;

    if (_moveDuration > 0 && !_isPunching) {
        setMovement(_moveDirection * dir_val * getForce());
        setMoveLeft(dist > 0);
        setMoveRight(dist < 0);
        _moveDuration--;
    }
    else if (_isPunching && _punchSprite->getFrame() >= _punch->getHitboxStartFrame() - 1 && _punchSprite->getFrame() <= _punch->getHitboxEndFrame() - 1) {
        setMovement(face * getForce() * MINION1B_PUNCH_FORCE * _scale);
    }
    else {
        setMovement(0);
    }

}

void Minion1BModel::slam() {
    faceTarget();
    if (rand() % 100 <= _aggression) {
        _aggression -= std::max(0.0f, _aggression - 50);
        _isSlamming = true;
        setMovement(0);
    }
}

void Minion1BModel::punch() {
	faceTarget();
    if (rand() % 100 <= _aggression) {
        _aggression -= std::max(0.0f, _aggression - 50);
        _isPunching = true;
        setMovement(0);
    }
}

std::shared_ptr<MeleeActionModel> Minion1BModel::getDamagingAction() {
    if (_isPunching && _punchSprite->getFrame() == _punch->getHitboxStartFrame() - 1) {
        return _punch;
    }
    if (_isSlamming && _slamSprite->getFrame() == _slam->getHitboxStartFrame() - 1) {
        return _slam;
    }
    return nullptr;
}

#pragma mark -
#pragma mark Animation Methods

void Minion1BModel::updateAnimation()
{

    _stunSprite->setVisible(isStunned());

    _walkSprite->setVisible(!isStunned() && !_isPunching && !_isSlamming && (isMoveLeft() || isMoveRight()));

    _slamSprite->setVisible(!isStunned() && _isSlamming);

	_slamVFXSprite->setVisible(_slamSprite->isVisible() && _slamSprite->getFrame() >= 19);

    _punchSprite->setVisible(!isStunned() && _isPunching);

    _idleSprite->setVisible(!_stunSprite->isVisible() && !_slamSprite->isVisible() && !_punchSprite->isVisible() && !_walkSprite->isVisible());

    playAnimation(_walkSprite);
    playAnimation(_idleSprite);
    playAnimationOnce(_slamSprite);
    playAnimationOnce(_punchSprite);
    playAnimationOnce(_stunSprite);

	playVFXAnimation(_slamSprite, _slamVFXSprite, _slam->getHitboxStartFrame() - 1);

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
void Minion1BModel::resetDebug() {
    EnemyModel::resetDebug();
}
