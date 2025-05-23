#include <cugl/scene2/CUPolygonNode.h>
#include <cugl/scene2/CUTexturedNode.h>
#include <cugl/core/assets/CUAssetManager.h>
#include "../actionmodel_variants/GBRangedActionModel.h"
#include "../../core/GBAudio.h"
#include "GBMinion1AModel.h"

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
bool Minion1AModel::init(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& enemyJSON, const Vec2& pos, std::vector<std::shared_ptr<ActionModel>> actions) {
    return EnemyModel::init(assetRef, enemyJSON, pos, actions);
};


void Minion1AModel::attachNodes(const std::shared_ptr<AssetManager>& assetRef) {
    EnemyModel::attachNodes(assetRef);
    // need new idle animation for this minion
    _idleSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion1A_walk"), 1, 8, 1);
    _idleSprite->setScale(0.0004006410 * Application::get()->getDisplayWidth());
    _idleSprite->setPosition(0, 5 * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _idleSprite->setName("idle");
    
    _walkSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion1A_walk"), 1, 8, 8);
    _walkSprite->setScale(0.0004006410 * Application::get()->getDisplayWidth());
    _walkSprite->setPosition(0, 5 * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _walkSprite->setName("walk");

    _explodeSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion1A_explode"), 9, 5, 45);
    _explodeSprite->setScale(0.0004006410 * Application::get()->getDisplayWidth());
    _explodeSprite->setPosition(0, 5 * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _explodeSprite->setName("explode");

    _shootSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion1A_shoot"), 1, 8, 8);
    _shootSprite->setScale(0.0004006410 * Application::get()->getDisplayWidth());
    _shootSprite->setPosition(0, 5 * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
	_shootSprite->setName("shoot");
    
    _explodeVFXSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("explode_enemy_1"), 4, 8, 28);
    _explodeVFXSprite->setPosition(0, 0);
	_explodeVFXSprite->setScale(2 * 0.0004006410 * Application::get()->getDisplayWidth());
    _explodeVFXSprite->setName("explode_vfx");

    _deadSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion1A_dead"), 3, 5, 15);
	_deadSprite->setScale(0.0004006410 * Application::get()->getDisplayWidth());
    _deadSprite->setPosition(0, 0);
	_deadSprite->setName("dead");

    _spawnSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("enemy_spawn"), 4, 7, 28);
    _spawnSprite->setPosition(0, 30 * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _spawnSprite->setScale(0.5 * 0.0004006410 * Application::get()->getDisplayWidth());
    _spawnSprite->setName("spawn");

    _damagedSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion_damaged"), 3, 3, 7);
    _damagedSprite->setPosition(0, 30 * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _damagedSprite->setScale(0.6 * 0.0004006410 * Application::get()->getDisplayWidth());
    _damagedSprite->setName("damaged");
    _damagedSprite->setVisible(false);

    stunFrames = 0;

    setName(std::string(ENEMY_NAME));

    getSceneNode()->addChild(_idleSprite);
    getSceneNode()->addChild(_walkSprite);
    getSceneNode()->addChild(_explodeSprite);
    getSceneNode()->addChild(_shootSprite);
    getSceneNode()->addChild(_explodeVFXSprite);

	getSceneNode()->addChild(_spawnSprite);
	getSceneNode()->addChild(_damagedSprite);
}

void Minion1AModel::setActions(std::vector<std::shared_ptr<ActionModel>> actions){
    for (auto act : actions) {
        if (act->getActionName() == "shoot") {
            _shoot = std::dynamic_pointer_cast<RangedActionModel>(act);
        }
        else if (act->getActionName() == "explode") {
            _explode = std::dynamic_pointer_cast<MeleeActionModel>(act);
        }
    }
    // TODO: init constants on construction
    _closeDistance = 12.0f;
    _farDistance = 18.0f;
    _maxSpeed = 5.0f;
}
#pragma mark -
#pragma mark Physics Methods
/**
 * Create new fixtures for this body, defining the shape
 *
 * This is the primary method to override for custom physics objects
 */
void Minion1AModel::createFixtures() {
    EnemyModel::createFixtures();
}

/**
 * Release the fixtures for this body, reseting the shape
 *
 * This is the primary method to override for custom physics objects.
 */
void Minion1AModel::releaseFixtures() {
    EnemyModel::releaseFixtures();
}

/**
 * Disposes all resources and assets of this PlayerModel
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a PlayerModel may not be used until it is initialized again.
 */
void Minion1AModel::dispose() {
    _geometry = nullptr;
    _node = nullptr;
    _groundSensorNode = nullptr;
    _geometry = nullptr;
    _walkSprite = nullptr;
    _explodeSprite = nullptr;
    _shootSprite = nullptr;
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
void Minion1AModel::update(float dt) {
	EnemyModel::update(dt);
}

#pragma mark -
#pragma mark AI Methods
void Minion1AModel::nextAction() {
    int r = rand();
    AIMove();
    
    //maintain mid-distance and shoot player
    if (!_isShooting && !_isExploding && _moveDuration <= 0 && !isStunned()) {
        if (isTargetClose()) {
            if (_hp < 25){
                AudioHelper::playSfx("explode");
                explode();
            }
            else {
                avoidTarget(45);
            }
        }
        else if (isTargetFar()){
            approachTarget(45);
        }
        
        else{
            faceTarget();
            shoot();
        }
    }
    else {
        if (isStunned()) {
            _isShooting = false;
            _isExploding = false;
            setMovement(0);
        }
        if (_isShooting && _shootSprite->getFrame() >= _shootSprite->getCount() - 1) {
            _isShooting = false;
            setMovement(0);
        }
        if (_isExploding && _explodeVFXSprite->getFrame() >= _explodeVFXSprite->getCount() - 1) {
            CULog("Setting minion1a hp to 0");
            _isExploding = false;
            _hp = 0;
            setMovement(0);
        }
    }
}

void Minion1AModel::AIMove() {
    float dist = getPosition().x - _targetPos.x;
    float dir_val = dist > 0 ? -1 : 1;
    int face = _faceRight ? 1 : -1;

    if (_moveDuration > 0 && !_isShooting) {
        setMovement(_moveDirection * dir_val * getForce());
        setMoveLeft(dist > 0);
        setMoveRight(dist < 0);
        _moveDuration--;
    }
    else {
        setMovement(0);
    }

}

void Minion1AModel::shoot() {
    faceTarget();
    if (_aggression > 8) {
        AudioHelper::playSfx("projectile");
        _aggression = 0;
        _isShooting = true;
        setMovement(0);
    }
}

void Minion1AModel::explode() {
	faceTarget();
    if (rand() % 100 <= _aggression) {
        _aggression -= std::max(0.0f, _aggression - 50);
        _isExploding = true;
        setMovement(0);
    }
}

std::shared_ptr<MeleeActionModel> Minion1AModel::getDamagingAction() {
    if (_isExploding && _explodeSprite->getFrame() == _explode->getHitboxStartFrame() - 1) {
        setEnabled(false);
        return _explode;
    }
    return nullptr;
}

std::shared_ptr<Projectile> Minion1AModel::getProjectile() {
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

void Minion1AModel::updateAnimation()
{
    int framenum = 0;
    framenum = _explodeVFXSprite->getFrame();
    if (framenum >0){
        CULog("ExplodeVfx frame #%d before update",framenum);
    }

    _idleSprite->setVisible(!isStunned() && !_isExploding && !_isShooting && !(isMoveLeft() || isMoveRight()));
    
    _walkSprite->setVisible(!isStunned() && !_isExploding && !_isShooting && (isMoveLeft() || isMoveRight()));

    _shootSprite->setVisible(!isStunned() && _isShooting);

    _explodeSprite->setVisible(!isStunned() && _isExploding);
    
    _explodeVFXSprite->setVisible(_isExploding && _explodeSprite->getFrame() >= _explode->getHitboxStartFrame() - 1);
    
	_spawnSprite->setVisible(isSpawning);
    
    playAnimation(_idleSprite);
    playAnimation(_walkSprite);
    playAnimationOnce(_shootSprite);
    playAnimationOnce(_explodeSprite);
	playAnimationOnce(_spawnSprite);
    playVFXAnimation(_explodeSprite, _explodeVFXSprite, _explode->getHitboxStartFrame() - 1);
    playDamagedEffect();
   
    if (framenum > 0){
        CULog("ExplodeVfx frame #%d after update",_explodeVFXSprite->getFrame());
    }
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
void Minion1AModel::resetDebug() {
    EnemyModel::resetDebug();
}
