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
bool Minion1BModel::init(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& constantsRef, const Vec2& pos, std::vector<std::shared_ptr<ActionModel>> actions) {
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
            if (act->getActionName() == "slam") {
                _slam = std::dynamic_pointer_cast<MeleeActionModel>(act);
            }
            else if (act->getActionName() == "punch") {
                _punch = std::dynamic_pointer_cast<MeleeActionModel>(act);
            }
        }

        return true;
    }

    return false;
}

void Minion1BModel::attachNodes(const std::shared_ptr<AssetManager>& assetRef) {
    _node = scene2::SceneNode::alloc();
    setSceneNode(_node);
    //move this to new function
    _idleSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion1B_idle"), 3, 4, 10);
	_idleSprite->setScale(0.5f);
    _idleSprite->setPosition(0, 10);

    _walkSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion1B_walk"), 3, 4, 10);
    _walkSprite->setScale(0.5f);
    _walkSprite->setPosition(0, 10);

    _punchSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion1B_punch"), 8, 4, 30);
    _punchSprite->setScale(0.5f);
    _punchSprite->setPosition(0, 10);

    _slamSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion1B_slam"), 8, 4, 32);
    _slamSprite->setScale(0.5f);
    _slamSprite->setPosition(0, 10);

	_slamVFXSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("wave_enemy_1"), 2, 8, 12);
	_slamVFXSprite->setScale(0.4f);
	_slamVFXSprite->setPosition(150, -5);

    _stunSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion1B_stun"), 1, 4, 4);
    _stunSprite->setScale(0.5f);
    _stunSprite->setPosition(0, 10);

    _deadSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("minion1B_dead"), 4, 4, 15);
    _deadSprite->setScale(0.5f);
    _deadSprite->setPosition(0, 10);

    setName(std::string(ENEMY_NAME));
    setDebugColor(ENEMY_DEBUG_COLOR);

    getSceneNode()->addChild(_idleSprite);
    getSceneNode()->addChild(_walkSprite);
    getSceneNode()->addChild(_punchSprite);
    getSceneNode()->addChild(_stunSprite);

    getSceneNode()->addChild(_slamSprite);
	getSceneNode()->addChild(_slamVFXSprite);

    getSceneNode()->addChild(_deadSprite);
    _deadSprite->setVisible(false);
}

#pragma mark -
#pragma mark Physics Methods
/**
 * Create new fixtures for this body, defining the shape
 *
 * This is the primary method to override for custom physics objects
 */
void Minion1BModel::createFixtures() {
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
void Minion1BModel::releaseFixtures() {
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
void Minion1BModel::dispose() {
    _geometry = nullptr;
    _node = nullptr;
    _sensorNode = nullptr;
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
        if (_isSlamming && _slamSprite->getFrame() >= MINION1B_SLAM_FRAMES - 1) {
            _isSlamming = false;
            setMovement(0);
        }
        if (_isPunching && _punchSprite->getFrame() >= MINION1B_PUNCH_FRAMES - 1) {
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
    playAnimation(_slamSprite);
    playAnimation(_punchSprite);
    playAnimation(_stunSprite);

	playVFXAnimation(_slamSprite, _slamVFXSprite, 19);

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
void Minion1BModel::resetDebug() {
    BoxObstacle::resetDebug();
    float w = ENEMY_SSHRINK * _dimension.width;
    float h = ENEMY_SENSOR_HEIGHT;
    Poly2 dudePoly(Rect(-w / 0.1f, -h / 2.0f, w, h));
    _sensorNode = scene2::WireNode::allocWithTraversal(dudePoly, poly2::Traversal::INTERIOR);
    _sensorNode->setColor(ENEMY_DEBUG_COLOR);
    _sensorNode->setPosition(Vec2(_debug->getContentSize().width / 2.0f, 0.0f));
}
