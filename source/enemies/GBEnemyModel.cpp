//
//  GBEnemyModel.cpp
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
#include "actionmodel_variants/GBRangedActionModel.h"
#include "../core/GBAudio.h"

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
bool EnemyModel::init(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& enemyJSON, const Vec2& pos, std::vector<std::shared_ptr<ActionModel>> actions) {
    _enemyJSON = enemyJSON;
    setConstants();
    resetAttributes();
    
    if (BoxObstacle::init(pos, _size)) {
        setDensity(_density);
        setFriction(0.0f);      // HE WILL STICK TO WALLS IF YOU FORGET
        setFixedRotation(true); // OTHERWISE, HE IS A WEEBLE WOBBLE
        attachNodes(assetRef);
        setActions(actions);
        setName(ENEMY_NAME);
        setDebugColor(Color4::BLUE);
        return true;
    }

    return false;

}

void EnemyModel::attachNodes(const std::shared_ptr<AssetManager>& assetRef) {
    _node = scene2::SceneNode::alloc();
    _node->setVisible(false);
    setSceneNode(_node);
}
void EnemyModel::setActions(std::vector<std::shared_ptr<ActionModel>> actions){
    
}
#pragma mark -
#pragma mark Attribute Properties

/**
* Reduces the health of this character.
*
* @param value the amount of hp reduction.
*/
void EnemyModel::damage(float value) {
    // placeholder sfx
    AudioHelper::playSfx("player_damage");
    _hp -= value;
    _hp = _hp < 0 ? 0 : _hp;
    _aggression += value;
    _aggression = _aggression > 100 ? 100 : _aggression;
    _lastDamagedFrame = 0;
    _node->setColor(Color4::RED);
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
    sensorDef.density = _density;
    sensorDef.isSensor = true;

    b2Filter filter = b2Filter();
    filter.maskBits = 0x0001; // collide with
    filter.categoryBits = 0x0002; // this is in group
    setFilterData(filter);

    // Sensor dimensions
    b2Vec2 corners[4];
    corners[0].x = - getWidth() / 2.0f;
    corners[0].y = (-getHeight() + _sensorHeight) / 2.0f;
    corners[1].x = - getWidth() / 2.0f;
    corners[1].y = (-getHeight() - _sensorHeight) / 2.0f;
    corners[2].x =  getWidth() / 2.0f;
    corners[2].y = (-getHeight() - _sensorHeight) / 2.0f;
    corners[3].x =  getWidth() / 2.0f;
    corners[3].y = (-getHeight() + _sensorHeight) / 2.0f;

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
 * Disposes all resources and assets of this EnemyModel
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a EnemyModel may not be used until it is initialized again.
 */
void EnemyModel::dispose() {
    _geometry = nullptr;
    _node = nullptr;
    _groundSensorNode = nullptr;
    _geometry = nullptr;
    _idleSprite = nullptr;
    _walkSprite = nullptr;
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
void EnemyModel::update(float dt) {
    if (isRemoved()) return;

    BoxObstacle::update(dt);
    if (_node != nullptr) {
        _node->setPosition(getPosition() * _drawScale);
        _node->setAngle(getAngle());
    }
}

#pragma mark -
#pragma mark AI Methods
void EnemyModel::nextAction() {

}

void EnemyModel::AIMove() {

}

bool EnemyModel::isTargetClose() {
    return (getPosition() - _targetPos).length() <= _closeDistance;
}

bool EnemyModel::isTargetFar() {
    return (getPosition() - _targetPos).length() >= _farDistance;
}

void EnemyModel::die(std::shared_ptr<scene2::SceneNode> world) {
    setVX(0);
    for (NodePtr node : _node->getChildren()) {
        if (node->getName() == "dead") {
            continue;
        }

        node->removeFromParent();
    }

    if (_node->getChildCount() == 0) {
        _node->addChild(_deadSprite);
        _deadSprite->setVisible(true);
    }

    playAnimationOnce(_deadSprite);

    if (_deadSprite->getFrame() == _deadSprite->getCount() - 1) {
        markRemoved(true);
        world->removeChild(_node);
        setSensor(true);
        getDebugNode()->removeFromParent();
    }
}

bool EnemyModel::canAvoid(){
    return (getX() - getWidth() > worldLeft && getX() + getWidth() < worldRight);
}

void EnemyModel::faceTarget() {
    bool face = _targetPos > getPosition();
    if (_faceRight == face) {
        return;
    }

    // Change facing
    scene2::TexturedNode* image = dynamic_cast<scene2::TexturedNode*>(_node.get());
    if (image != nullptr) {
        image->flipHorizontal(!image->isFlipHorizontal());
    }
    _faceRight = face;
}

std::shared_ptr<MeleeActionModel> EnemyModel::getDamagingAction() {
    return nullptr;
}

std::shared_ptr<Projectile> EnemyModel::getProjectile() {
    return nullptr;
}

#pragma mark -
#pragma mark Animation Methods
void EnemyModel::playAnimation(std::shared_ptr<scene2::SpriteNode> sprite) {
    if (sprite->isVisible()) {
        frameCounter = (frameCounter + 1) % E_ANIMATION_UPDATE_FRAME;
        if (frameCounter % E_ANIMATION_UPDATE_FRAME == 0) {
            sprite->setFrame((sprite->getFrame() + 1) % sprite->getCount());
        }
    }
    else {
        sprite->setFrame(0);
    }
}

void EnemyModel::playAnimationOnce(std::shared_ptr<scene2::SpriteNode> sprite)
{
    if (sprite->isVisible()) {
        frameCounter = (frameCounter + 1) % E_ANIMATION_UPDATE_FRAME;
        if (frameCounter % E_ANIMATION_UPDATE_FRAME == 0 && sprite->getFrame() < sprite->getCount() - 1) {
            sprite->setFrame(sprite->getFrame() + 1);
        }
    }
    else {
        sprite->setFrame(0);
    }
}

void EnemyModel::playVFXAnimation(std::shared_ptr<scene2::SpriteNode> actionSprite, std::shared_ptr<scene2::SpriteNode> vfxSprite, int startFrame)
{
    if (actionSprite->isVisible()) {
        if (actionSprite->getFrame() == startFrame) {
            vfxSprite->setFrame(0);
        }

        else if (actionSprite->getFrame() > startFrame) {
            if (frameCounter % E_ANIMATION_UPDATE_FRAME == 0 && vfxSprite->getFrame() < vfxSprite->getCount() - 1) {
                vfxSprite->setFrame(vfxSprite->getFrame() + 1);
            }
        }
	}
    else {
        vfxSprite->setFrame(0);
    }
}

void EnemyModel::updateAnimation(){}

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
    _debug->setName("enemy_debug");
    if (_groundSensorNode == nullptr){
        setDebug();
    }
    if (_debug->getChildCount() == 0){
        _groundSensorNode->setRelativeColor(false);
        _groundSensorNode->setColor(Color4::RED);
        _debug->addChild(_groundSensorNode);
    }
    
    // necessary during reset, set debug scene node, since BoxObstacle::resetDebug() doesn't handle it correctly.
    if (_debug->getScene() == nullptr){
        _scene->addChild(_debug);
    }
}

void EnemyModel::setConstants(){
    _drawScale = _enemyJSON->_parent->get("world_info")->getFloat("scale");
    worldLeft = _enemyJSON->_parent->get("world_info")->getFloat("worldLeft");
    worldRight = _enemyJSON->_parent->get("world_info")->getFloat("worldRight");
    
    _size = Size(_enemyJSON->get("size")->get(0)->asFloat(), _enemyJSON->get("size")->get(1)->asFloat()) / _drawScale;
    _size.width *= _hShrink;
    _size.height *= _hShrink;    
    
    _maxHP = _enemyJSON->getInt("max_hp");
}

void EnemyModel::setDebug(){
    // Sensor dimensions
    float w = _dimension.width;
    float h = _sensorHeight;
    Poly2 groundPoly(Rect(-w / 2.0f, -h / 2.0f, w, h));
    _groundSensorNode = scene2::WireNode::allocWithTraversal(groundPoly, poly2::Traversal::INTERIOR);
    _groundSensorNode->setPosition(Vec2(_debug->getContentSize().width / 2.0f, 0.0f));
}
