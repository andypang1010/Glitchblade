#include <cugl/scene2/CUPolygonNode.h>
#include <cugl/scene2/CUTexturedNode.h>
#include <cugl/core/assets/CUAssetManager.h>
#include "../actionmodel_variants/GBRangedActionModel.h"
#include "GBMinion2AModel.h"

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
bool Minion2AModel::init(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& enemyJSON, const Vec2& pos, std::vector<std::shared_ptr<ActionModel>> actions) {
    return EnemyModel::init(assetRef, enemyJSON, pos, actions);
};


void Minion2AModel::attachNodes(const std::shared_ptr<AssetManager>& assetRef) {
    _node = scene2::SceneNode::alloc();
    setSceneNode(_node);
    // need new idle animation for this minion
    _idleSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion2A_idle"), 5, 2, 10);
    _idleSprite->setScale(1.44 * 0.3f * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _idleSprite->setPosition(0, -30 * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _idleSprite->setName("idle");

    _shootSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion2A_shoot"), 2, 3, 6);
    _shootSprite->setScale(1.44 * 0.3f * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _shootSprite->setPosition(0, -30 * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
	_shootSprite->setName("shoot");

    _deadSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion2A_dead"), 10, 3, 30);
	_deadSprite->setScale(1.44 * 0.3f * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _deadSprite->setPosition(0, -30 * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
	_deadSprite->setName("dead");

    _spawnSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("enemy_spawn"), 4, 7, 28);
    _spawnSprite->setPosition(0, 30 * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _spawnSprite->setScale(0.5 * 0.0004006410 * Application::get()->getDisplayWidth());
    _spawnSprite->setName("spawn");

    stunFrames = 0;

    setName(std::string(ENEMY_NAME));

    getSceneNode()->addChild(_idleSprite);
    getSceneNode()->addChild(_shootSprite);
	getSceneNode()->addChild(_spawnSprite);
}

void Minion2AModel::setActions(std::vector<std::shared_ptr<ActionModel>> actions){
    for (auto act : actions) {
        if (act->getActionName() == "shoot") {
            _shoot = std::dynamic_pointer_cast<RangedActionModel>(act);
        }
    }
    _force = 10;
}
#pragma mark -
#pragma mark Physics Methods
/**
 * Create new fixtures for this body, defining the shape
 *
 * This is the primary method to override for custom physics objects
 */
void Minion2AModel::createFixtures() {
    EnemyModel::createFixtures();
	setGravityScale(0.0f);
}

/**
 * Release the fixtures for this body, reseting the shape
 *
 * This is the primary method to override for custom physics objects.
 */
void Minion2AModel::releaseFixtures() {
    EnemyModel::releaseFixtures();
}

/**
 * Disposes all resources and assets of this PlayerModel
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a PlayerModel may not be used until it is initialized again.
 */
void Minion2AModel::dispose() {
    _geometry = nullptr;
    _node = nullptr;
    _groundSensorNode = nullptr;
    _geometry = nullptr;
    _walkSprite = nullptr;
    _shootSprite = nullptr;
    _deadSprite = nullptr;
}

void Minion2AModel::damage(float value) {
    EnemyModel::damage(value);
    if (_hp <= 0) setGravityScale(1);
}

#pragma mark Cooldowns
/**
 * Updates the object's physics state (NOT GAME LOGIC).
 *
 * We use this method to reset cooldowns.
 *
 * @param delta Number of seconds since last animation frame
 */
void Minion2AModel::update(float dt) {
    EnemyModel::update(dt);
}

#pragma mark -
#pragma mark AI Methods
void Minion2AModel::nextAction() {
    int r = rand();
    AIMove();
    
    //maintain mid-distance and shoot player
    if (!_isShooting && !isStunned()) {
        if (!isTargetFar()) {
            avoidTarget(30);
        }
        else {
            approachTarget(30);
            if (isInRange()) {
                shoot();
            }
        }
    }
    else {
        if (isStunned()) {
            _isShooting = false;
            setMovement(0);
        }
        if (_isShooting && _shootSprite->getFrame() >= _shootSprite->getCount() - 1) {
            _isShooting = false;
            setMovement(0);
        }
    }
}

void Minion2AModel::AIMove() {
    float dist = getPosition().x - _targetPos.x;
    float dir_val = dist > 0 ? -1 : 1;
    int face = _faceRight ? 1 : -1;

    if (_moveDuration > 0) {
        if (_worldTop - getPosition().y <= 4) { // near top, quickly move down
            setVerticalMovement(-getForce());
        }
        else {
            if (getPosition().y <= 8) {
                setVerticalMovement(rand() % 10 <= 7 ? getForce() : -getForce());
            }
            else {
                setVerticalMovement(rand() % 10 <= 7 ? -getForce() : getForce());
            }
        }

        setMovement(_moveDirection * dir_val * getForce());
        setMoveLeft(dist > 0);
        setMoveRight(dist < 0);
        _moveDuration--;
    }
    else {
        setMovement(0);
    }

}

bool Minion2AModel::isInRange() {
    float targetAngle = Vec2::angle(_targetPos - this->getPosition(), Vec2(1, 0));
    return ((targetAngle >= 0 && targetAngle <= M_PI / 6) || (targetAngle >= M_PI * 5 / 6 && targetAngle <= M_PI));
}

void Minion2AModel::shoot() {
    faceTarget();
    if (_aggression > 12) {
        _aggression = 0;
        _isShooting = true;
        setMovement(0);
    }
}

std::shared_ptr<MeleeActionModel> Minion2AModel::getDamagingAction() {
    return nullptr;
}

std::shared_ptr<Projectile> Minion2AModel::getProjectile() {
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

void Minion2AModel::updateAnimation()
{

    _idleSprite->setVisible(!isStunned() && !_isShooting);

    _shootSprite->setVisible(!isStunned() && _isShooting);

	_spawnSprite->setVisible(isSpawning);
    
    playAnimation(_idleSprite);
    playAnimationOnce(_shootSprite);
	playAnimationOnce(_spawnSprite);
   
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
void Minion2AModel::resetDebug() {
    EnemyModel::resetDebug();
}
