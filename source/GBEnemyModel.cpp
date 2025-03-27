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
#include "GBEnemyModel.h"
#include <cugl/scene2/CUPolygonNode.h>
#include <cugl/scene2/CUTexturedNode.h>
#include <cugl/core/assets/CUAssetManager.h>
#include "GBRangedActionModel.h"

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
bool EnemyModel::init(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& constantsRef, const Vec2& pos, std::vector<std::shared_ptr<ActionModel>> actions) {
    resetAttributes();

    float scale = constantsRef->get("scene")->getFloat("scale");
    _enemyJSON = constantsRef->get("enemy");
    std::shared_ptr<graphics::Texture> image;
    image = assetRef->get<graphics::Texture>(ENEMY_TEXTURE);

    Size nsize = Size(90, 180) / scale;
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

        return true;
    }

    return false;
}

void EnemyModel::attachNodes(const std::shared_ptr<AssetManager>& assetRef) {
    _node = scene2::SceneNode::alloc();
    setSceneNode(_node);
    //move this to new function
    _idleSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss1_idle"), 1, 6, 6);
    _idleSprite->setPosition(0, 50);

    _walkSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss1_walking1"), 1, 8, 8);
    _walkSprite->setPosition(0, 50);

    _slamSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss1_slam"), 4, 10, 40);
    _slamSprite->setPosition(0, 50);

    _stabSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss1_stab"), 4, 10, 40);
    _stabSprite->setPosition(0, 50);

    _stunSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss1_stun"), 3, 10, 22);
    _stunSprite->setPosition(0, 50);

    _shootSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss1_shoot"), 2, 10, 15);
    _shootSprite->setPosition(0, 50);

    _explodeSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss1_explode"), 4, 10, 40);
    _explodeSprite->setPosition(0, 50);

	_explodeVFXSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("explode_enemy_1"), 4, 8, 32);
	_explodeVFXSprite->setPosition(0, 0);

    setName(std::string(ENEMY_NAME));
    setDebugColor(ENEMY_DEBUG_COLOR);

    getSceneNode()->addChild(_idleSprite);
    getSceneNode()->addChild(_walkSprite);
    getSceneNode()->addChild(_slamSprite);
    getSceneNode()->addChild(_stabSprite);
    getSceneNode()->addChild(_stunSprite);
	getSceneNode()->addChild(_shootSprite);

	getSceneNode()->addChild(_explodeSprite);
    getSceneNode()->addChild(_explodeVFXSprite);

}

#pragma mark -
#pragma mark Attribute Properties

/**
* Reduces the health of this character.
*
* @param value the amount of hp reduction.
*/
void EnemyModel::damage(float value) {
    _hp -= value;
    _hp = _hp < 0 ? 0 : _hp;
}

/**
 * Sets left/right movement of this character.
 *
 * This is the result of input times dude force.
 *
 * @param value left/right movement of this character.
 */
void EnemyModel::setMovement(float value) {
    _movement = value;
    bool face = _targetPos > getPosition();
    if (_movement == 0 || _faceRight == face || _isStabbing) {
        return;
    }

    // Change facing
    scene2::TexturedNode* image = dynamic_cast<scene2::TexturedNode*>(_node.get());
    if (image != nullptr) {
        image->flipHorizontal(!image->isFlipHorizontal());
    }
    _faceRight = face;
}

/**
* Make the sprite face left
*/
void EnemyModel::faceLeft() {
    if (_faceRight == true) {
        _faceRight = false;
        scene2::TexturedNode* image = dynamic_cast<scene2::TexturedNode*>(_node.get());
        if (image != nullptr) {
            image->flipHorizontal(!image->isFlipHorizontal());
        }
    }
}

/**
* Make the sprite face right
*/
void EnemyModel::faceRight() {
    if (_faceRight == false) {
        _faceRight = true;
        scene2::TexturedNode* image = dynamic_cast<scene2::TexturedNode*>(_node.get());
        if (image != nullptr) {
            image->flipHorizontal(!image->isFlipHorizontal());
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
void EnemyModel::createFixtures() {
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
void EnemyModel::releaseFixtures() {
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
void EnemyModel::dispose() {
    _geometry = nullptr;
    _node = nullptr;
    _sensorNode = nullptr;
    _geometry = nullptr;
    _currentSpriteNode = nullptr;
    _idleSprite = nullptr;
    _walkSprite = nullptr;
    _stabSprite = nullptr;
    _slamSprite = nullptr;
    _stunSprite = nullptr;
	_shootSprite = nullptr;
	_explodeSprite = nullptr;
}

#pragma mark Cooldowns
/**
 * Updates the object's physics state (NOT GAME LOGIC).
 *
 * We use this method to reset cooldowns.
 *
 * @param delta Number of seconds since last animation frame
 */
void EnemyModel::update(float dt) {
    if (isRemoved()) return;

    updateAnimation();
    nextAction();

    // Apply cooldowns

    if (isKnocked()) {
        resetKnocked();
    }

    if (isStunned()) {
        _stunRem--;
    }

    BoxObstacle::update(dt);
    if (_node != nullptr) {
        _node->setPosition(getPosition() * _drawScale);
        _node->setAngle(getAngle());
    }
}

#pragma mark -
#pragma mark AI Methods
bool EnemyModel::isTargetClose(Vec2 targetPos) {
    return (getPosition() - targetPos).length() <= CLOSE_RADIUS;
}

void EnemyModel::nextAction() {
    int r = rand();
    AIMove();
    if (!_isSlamming && !_isStabbing && !_isShooting && !_isExploding && _moveDuration <= 0 && isTargetClose(_targetPos) && !isStunned()) {
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
            _moveDuration = 45;
            _moveDirection = -1;
        }
    }
    else if (!_isSlamming && !_isStabbing && !_isShooting && !_isExploding && _moveDuration <= 0 && !isStunned()) {
        if (r % 3 == 0) { // Stab
            stab();
        }
		else if (r % 3 == 1) { // Shoot
			shoot();
		}
        else { // Move closer
            _moveDuration = 45;
            _moveDirection = 1;
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
        if (_isSlamming && _slamSprite->getFrame() >= SLAM_FRAMES - 1) {
            _isSlamming = false;
            setMovement(0);
        }
        if (_isStabbing && _stabSprite->getFrame() >= STAB_FRAMES - 1) {
            _isStabbing = false;
            setMovement(getMovement());
        }
        if (_isShooting && _shootSprite->getFrame() >= SHOOT_FRAMES - 1) {
            _isShooting = false;
            setMovement(0);
        }
        if (_isExploding && _explodeSprite->getFrame() >= EXPLODE_FRAMES - 1) {
            _isExploding = false;
            setMovement(0);
        }
    }
}

void EnemyModel::AIMove() {
    float dist = getPosition().x - _targetPos.x;
    float dir_val = dist > 0 ? -1 : 1;
    int face = _faceRight ? 1 : -1;

    if (_moveDuration > 0 && !_isStabbing) {
        setMovement(_moveDirection * dir_val * getForce());
        setMoveLeft(dist > 0);
        setMoveRight(dist < 0);
        _moveDuration--;
    }

    else if (_isStabbing && _stabSprite->getFrame() >= _stab->getHitboxStartTime() - 1 && _stabSprite->getFrame() <= _stab->getHitboxEndTime() - 1) {
        setMovement(face * getForce() * STAB_FORCE);
    }

}

void EnemyModel::slam() {
    if (getPosition().x - _targetPos.x < 0) {
        faceRight();
    }
    else {
        faceLeft();
    }
    _isSlamming = true;
    setMovement(0);
}

void EnemyModel::stab() {
    if (getPosition().x - _targetPos.x < 0) {
        faceRight();
    }
    else {
        faceLeft();
    }
    _isStabbing = true;
    setMovement(0);
}

void EnemyModel::shoot() {
    if (getPosition().x - _targetPos.x < 0) {
        faceRight();
    }
    else {
        faceLeft();
    }
    _isShooting = true;
    setMovement(0);
}

void EnemyModel::explode() {
    if (getPosition().x - _targetPos.x < 0) {
        faceRight();
    }
    else {
        faceLeft();
    }
    _isExploding = true;
    setMovement(0);
}

std::shared_ptr<MeleeActionModel> EnemyModel::getDamagingAction() {
    if (_isStabbing && _stabSprite->getFrame() == _stab->getHitboxStartTime() - 1) {
        return _stab;
    }
    else if (_isSlamming && _slamSprite->getFrame() == _slam->getHitboxStartTime() - 1) {
        return _slam;
	}
	else if (_isExploding && _explodeSprite->getFrame() == _explode->getHitboxStartTime() - 1) {
		return _explode;
	}
    return nullptr;
}

std::shared_ptr<RangedActionModel> EnemyModel::getProjectileAction() {
	std::vector<int> frames = _shoot->getProjectileSpawnFrames();
    if (_isShooting && std::find(frames.begin(), frames.end(), _shootSprite->getFrame()+1) != frames.end()) {
        return _shoot;
    }
    return nullptr;
}

#pragma mark -
#pragma mark Animation Methods
void EnemyModel::playAnimation(std::shared_ptr<scene2::SpriteNode> sprite) {
    if (sprite->isVisible()) {
        currentFrame++;
        if (currentFrame > sprite->getCount() * E_ANIMATION_UPDATE_FRAME) {
            currentFrame = 0;
        }

        if (currentFrame % E_ANIMATION_UPDATE_FRAME == 0) {
            sprite->setFrame((sprite->getFrame() + 1) % sprite->getCount());
        }
    }
    else {
        sprite->setFrame(0);
    }
}

void EnemyModel::updateAnimation()
{

    _stunSprite->setVisible(isStunned());

    _walkSprite->setVisible(!isStunned() && !_isStabbing && !_isSlamming && isGrounded() && (isMoveLeft() || isMoveRight()));

    _slamSprite->setVisible(!isStunned() && _isSlamming);

    _stabSprite->setVisible(!isStunned() && _isStabbing);

	_shootSprite->setVisible(!isStunned() && _isShooting);

	_explodeSprite->setVisible(!isStunned() && _isExploding);

    _explodeVFXSprite->setVisible(_explodeSprite->isVisible() && currentFrame >= 96);

    if (_stunRem == STUN_FRAMES) {
        currentFrame = 0;
    }

    _idleSprite->setVisible(!isStunned() && !_slamSprite->isVisible() && !_stabSprite->isVisible() && !_shootSprite->isVisible() && !_explodeSprite->isVisible() && !_walkSprite->isVisible());

    playAnimation(_walkSprite);
    playAnimation(_idleSprite);
    playAnimation(_slamSprite);
    playAnimation(_stabSprite);
    playAnimation(_stunSprite);
	playAnimation(_shootSprite);
	playAnimation(_explodeSprite);

    playVFXAnimation(_explodeSprite, _explodeVFXSprite, 24);

    _node->setScale(Vec2(isFacingRight() ? 1 : -1, 1));
    _node->getChild(_node->getChildCount() - 2)->setScale(Vec2(isFacingRight() ? 1 : -1, 1));
    _node->getChild(_node->getChildCount() - 1)->setScale(Vec2(isFacingRight() ? 1 : -1, 1));
}

void EnemyModel::playVFXAnimation(std::shared_ptr<scene2::SpriteNode> actionSprite, std::shared_ptr<scene2::SpriteNode> vfxSprite, int startFrame)
{
    if (actionSprite->isVisible()) {
        if (currentFrame == startFrame * E_ANIMATION_UPDATE_FRAME) {
            vfxSprite->setFrame(0);
        }

        if (currentFrame > startFrame * E_ANIMATION_UPDATE_FRAME && currentFrame % E_ANIMATION_UPDATE_FRAME == 0) {
            vfxSprite->setFrame((vfxSprite->getFrame() + 1) % vfxSprite->getCount());
        }
    }
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
void EnemyModel::resetDebug() {
    BoxObstacle::resetDebug();
    float w = ENEMY_SSHRINK * _dimension.width;
    float h = ENEMY_SENSOR_HEIGHT;
    Poly2 dudePoly(Rect(-w / 0.1f, -h / 2.0f, w, h));
    _sensorNode = scene2::WireNode::allocWithTraversal(dudePoly, poly2::Traversal::INTERIOR);
    _sensorNode->setColor(ENEMY_DEBUG_COLOR);
    _sensorNode->setPosition(Vec2(_debug->getContentSize().width / 2.0f, 0.0f));
}
