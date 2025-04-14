#include <cugl/scene2/CUPolygonNode.h>
#include <cugl/scene2/CUTexturedNode.h>
#include <cugl/core/assets/CUAssetManager.h>
#include "../actionmodel_variants/GBRangedActionModel.h"
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
bool Minion1AModel::init(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& constantsRef, const Vec2& pos, std::vector<std::shared_ptr<ActionModel>> actions) {
    resetAttributes();

    float scale = constantsRef->get("scene")->getFloat("scale");
    _enemyJSON = constantsRef->get("enemy");
    std::shared_ptr<graphics::Texture> image;
    image = assetRef->get<graphics::Texture>(ENEMY_TEXTURE);

    stunFrame = 16;

    Size nsize = Size(90, 90) / scale;
    nsize.width *= _enemyJSON->get("fixtures")->get("body")->getFloat("h_shrink");
    nsize.height *= _enemyJSON->get("fixtures")->get("body")->getFloat("h_shrink");
    _drawScale = scale;

    setDebugColor(_enemyJSON->get("debug")->getString("color"));
    if (BoxObstacle::init(pos, nsize)) {
        setDensity(ENEMY_DENSITY);
        setFriction(0.0f);      // HE WILL STICK TO WALLS IF YOU FORGET
        setFixedRotation(true); // OTHERWISE, HE IS A WEEBLE WOBBLE
        attachNodes(assetRef);

        for (auto act : actions) {
            if (act->getActionName() == "shoot") {
                _shoot = std::dynamic_pointer_cast<RangedActionModel>(act);
            }
            else if (act->getActionName() == "explode") {
                _explode = std::dynamic_pointer_cast<MeleeActionModel>(act);
            }
        }

        return true;
    }

    return false;
}

void Minion1AModel::attachNodes(const std::shared_ptr<AssetManager>& assetRef) {
    _node = scene2::SceneNode::alloc();
    setSceneNode(_node);
    
    _walkSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion1A_walk"), 1, 8, 8);
    _walkSprite->setScale(0.5f);
    _walkSprite->setPosition(0, 10);

    _explodeSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion1A_explode"), 6, 8, 45);
    _explodeSprite->setScale(0.5f);
    _explodeSprite->setPosition(0, 10);

    _shootSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion1A_shoot"), 1, 5, 5);
    _shootSprite->setScale(0.5f);
    _shootSprite->setPosition(0, 10);
    
    _explodeVFXSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("explode_enemy_1"), 4, 8, 28);
    _explodeVFXSprite->setPosition(0, 0);
 
    setName(std::string(ENEMY_NAME));
    setDebugColor(ENEMY_DEBUG_COLOR);

    getSceneNode()->addChild(_walkSprite);
    getSceneNode()->addChild(_explodeSprite);
    getSceneNode()->addChild(_shootSprite);
    getSceneNode()->addChild(_explodeVFXSprite);


}

#pragma mark -
#pragma mark Physics Methods
/**
 * Create new fixtures for this body, defining the shape
 *
 * This is the primary method to override for custom physics objects
 */
void Minion1AModel::createFixtures() {
    if (_body == nullptr) {
        return;
    }

    BoxObstacle::createFixtures();
    b2FixtureDef sensorDef;
    sensorDef.density = ENEMY_DENSITY;
    sensorDef.isSensor = true;

    b2Filter filter = b2Filter();
    filter.maskBits = 0x0001;
    filter.categoryBits = 0x0002;
    setFilterData(filter);

    // Sensor dimensions
    b2Vec2 corners[4];
    corners[0].x = -ENEMY_SSHRINK * getWidth() / 2.0f;
    corners[0].y = (-getHeight() + ENEMY_SENSOR_HEIGHT) / 2.0f;
    corners[1].x = -ENEMY_SSHRINK * getWidth() / 2.0f;
    corners[1].y = (-getHeight() - ENEMY_SENSOR_HEIGHT) / 2.0f;
    corners[2].x = ENEMY_SSHRINK * getWidth() / 2.0f;
    corners[2].y = (-getHeight() - ENEMY_SENSOR_HEIGHT) / 2.0f;
    corners[3].x = ENEMY_SSHRINK * getWidth() / 2.0f;
    corners[3].y = (-getHeight() + ENEMY_SENSOR_HEIGHT) / 2.0f;

    b2PolygonShape sensorShape;
    sensorShape.Set(corners, 4);

    sensorDef.shape = &sensorShape;
    sensorDef.userData.pointer = reinterpret_cast<uintptr_t>(getSensorName());
    _sensorFixture = _body->CreateFixture(&sensorDef);
}

/**
 * Release the fixtures for this body, reseting the shape
 *
 * This is the primary method to override for custom physics objects.
 */
void Minion1AModel::releaseFixtures() {
    if (_body != nullptr) {
        return;
    }

    BoxObstacle::releaseFixtures();
    if (_sensorFixture != nullptr) {
        _body->DestroyFixture(_sensorFixture);
        _sensorFixture = nullptr;
    }
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
    _sensorNode = nullptr;
    _geometry = nullptr;
    _currentSpriteNode = nullptr;
    _walkSprite = nullptr;
    _explodeSprite = nullptr;
    _shootSprite = nullptr;
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
void Minion1AModel::nextAction() {
    int r = rand();
    AIMove();
    
    //maintain mid-distance and shoot player
    if (!_isShooting && !_isExploding && _moveDuration <= 0 && !isStunned()) {
        if (isTargetClose()) {
            if (_hp < 25){
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
        if (_isShooting && _shootSprite->getFrame() >= MINION1A_SHOOT_FRAMES - 1) {
            _isShooting = false;
            setMovement(0);
        }
        if (_isExploding && _explodeVFXSprite->getFrame() >= MINION1A_EXPLODEVFX_FRAMES - 1) {
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
    if (_isExploding && _explodeSprite->getFrame() == _explode->getHitboxStartTime() - 1) {
        setEnabled(false);
        return _explode;
    }
    return nullptr;
}

std::shared_ptr<RangedActionModel> Minion1AModel::getProjectileAction() {
    std::vector<int> frames = _shoot->getProjectileSpawnFrames();
    for (int frame : frames) {
        if (_isShooting && _shootSprite->getFrame() == frame && frameCounter == 0) {
            return _shoot;
        }
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


    _walkSprite->setVisible(!isStunned() && !_isExploding && !_isShooting && (isMoveLeft() || isMoveRight()));

    _shootSprite->setVisible(!isStunned() && _isShooting);

    _explodeSprite->setVisible(!isStunned() && _isExploding);
    
    _explodeVFXSprite->setVisible(_isExploding && _explodeSprite->getFrame() >= _explode->getHitboxStartTime() - 1);
    

    playAnimation(_walkSprite);
    playAnimation(_shootSprite);
    playAnimation(_explodeSprite);
    playVFXAnimation(_explodeSprite, _explodeVFXSprite, _explode->getHitboxStartTime() - 1);
   
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
    BoxObstacle::resetDebug();
    float w = ENEMY_SSHRINK * _dimension.width;
    float h = ENEMY_SENSOR_HEIGHT;
    Poly2 dudePoly(Rect(-w / 0.1f, -h / 2.0f, w, h));
    _sensorNode = scene2::WireNode::allocWithTraversal(dudePoly, poly2::Traversal::INTERIOR);
    _sensorNode->setColor(ENEMY_DEBUG_COLOR);
    _sensorNode->setPosition(Vec2(_debug->getContentSize().width / 2.0f, 0.0f));
}
