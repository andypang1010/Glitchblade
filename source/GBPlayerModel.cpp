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
#include "GBPlayerModel.h"
#include <cugl/scene2/CUPolygonNode.h>
#include <cugl/scene2/CUTexturedNode.h>
#include <cugl/core/assets/CUAssetManager.h>
using namespace cugl::graphics;


using namespace cugl;

#pragma mark -
#pragma mark Constructors

/**
 * Initializes a new player at the given position.
 *
 * The player is sized according to the given drawing scale.
 *
 * The scene graph is completely decoupled from the physics system.
 * The node does not have to be the same size as the physics body. We
 * only guarantee that the scene graph node is positioned correctly
 * according to the drawing scale.
 *
 * @param pos   Initial position in world coordinates
 * @param size  The size of the player in world units
 * @param scale The drawing scale (world to screen)
 *
 * @return  true if the obstacle is initialized properly, false otherwise.
 */
bool PlayerModel::init(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& constantsRef, const Vec2& pos) {
    resetAttributes();
    float scale = constantsRef->get("scene")->getFloat("scale");
    std::shared_ptr<graphics::Texture> image;
    image = assetRef->get<graphics::Texture>(PLAYER_TEXTURE);
    Size nsize = image->getSize() / scale;
    nsize.width *= PLAYER_HSHRINK;
    nsize.height *= PLAYER_VSHRINK;
    _drawScale = scale;

    setDebugColor(PLAYER_DEBUG_COLOR);

    if (BoxObstacle::init(pos, nsize)) {
        setDensity(PLAYER_DENSITY);
        setFriction(0.0f);      // HE WILL STICK TO WALLS IF YOU FORGET
        setFixedRotation(true); // OTHERWISE, HE IS A WEEBLE WOBBLE

        // set the scene node and attach the sprite nodes to it
        attachNodes(assetRef);
        return true;
    }
    return false;
}


void PlayerModel::attachNodes(const std::shared_ptr<AssetManager>& assetRef) {
    _sceneNode = scene2::SceneNode::alloc();
    setSceneNode(_sceneNode);

    _idleSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("player_idle"), 3, 4, 11);
    _idleSprite->setPosition(0, -25);
    _idleSprite->setScale(0.5f);

    _walkSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("player_walk"), 2, 4, 5);
    _walkSprite->setPosition(0, -25);
    _walkSprite->setScale(0.5f);

    _jumpUpSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("player_jumpUp"), 2, 4, 5);
    _jumpUpSprite->setPosition(0, -25);
    _jumpUpSprite->setScale(0.5f);

    _jumpDownSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("player_jumpDown"), 2, 4, 7);
    _jumpDownSprite->setPosition(0, -25);
    _jumpDownSprite->setScale(0.5f);

    _guardSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("player_guard"), 3, 4, 12);
    _guardSprite->setPosition(0, -25);
    _guardSprite->setScale(0.5f);

    _attackSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("player_attack"), 2, 4, 5);
    _attackSprite->setPosition(0, -25);
    _attackSprite->setScale(0.5f);

    getSceneNode()->addChild(_idleSprite);
    getSceneNode()->addChild(_walkSprite);
    getSceneNode()->addChild(_jumpUpSprite);
    getSceneNode()->addChild(_jumpDownSprite);
    getSceneNode()->addChild(_guardSprite);
    getSceneNode()->addChild(_attackSprite);
}

#pragma mark -
#pragma mark Attribute Properties

/**
* Reduces the health of this character.
*
* @param value the amount of hp reduction.
*/
void PlayerModel::damage(float value) {
    _hp -= value;
    _hp = _hp < 0 ? 0 : _hp;
}

/**
 * Sets left/right movement of this character.
 *
 * This is the result of input times player force.
 *
 * @param value left/right movement of this character.
 */
void PlayerModel::setMovement(float value) {
    _movement = value;
    bool face = _movement > 0;
    if (_movement == 0 || _faceRight == face) {
        return;
    }

    // Change facing
    scene2::TexturedNode* image = dynamic_cast<scene2::TexturedNode*>(_sceneNode.get());
    if (image != nullptr) {
        // Don't flip if it means overriding a dash direction
        if (!isDashActive()) {
            image->flipHorizontal(!image->isFlipHorizontal());
        }
    }

    if (!isDashActive()) {
        _faceRight = face;
    }
}

/**
* Make the sprite face left
*/
void PlayerModel::faceLeft() {
    if (_faceRight == true) {
        _faceRight = false;
        scene2::TexturedNode* image = dynamic_cast<scene2::TexturedNode*>(_sceneNode.get());
        if (image != nullptr) {
            image->flipHorizontal(!image->isFlipHorizontal());
        }
    }
}

/**
* Make the sprite face right
*/
void PlayerModel::faceRight() {
    if (_faceRight == false) {
        _faceRight = true;
        scene2::TexturedNode* image = dynamic_cast<scene2::TexturedNode*>(_sceneNode.get());
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
void PlayerModel::createFixtures() {
    if (_body == nullptr) {
        return;
    }

    BoxObstacle::createFixtures();

    // give name to initial body fixture for collision handling
    b2Fixture* bodyFix = _body->GetFixtureList();
    if (bodyFix) {
        bodyFix->GetUserData().pointer = reinterpret_cast<uintptr_t>(getBodyName());
    }

    b2FixtureDef sensorDef;
    sensorDef.density = PLAYER_DENSITY;
    sensorDef.isSensor = true;

    // Sensor dimensions
    b2Vec2 corners[4];
    corners[0].x = -PLAYER_SSHRINK * getWidth() / 2.0f;
    corners[0].y = (-getHeight() + PLAYER_SENSOR_HEIGHT) / 2.0f;
    corners[1].x = -PLAYER_SSHRINK * getWidth() / 2.0f;
    corners[1].y = (-getHeight() - PLAYER_SENSOR_HEIGHT) / 2.0f;
    corners[2].x = PLAYER_SSHRINK * getWidth() / 2.0f;
    corners[2].y = (-getHeight() - PLAYER_SENSOR_HEIGHT) / 2.0f;
    corners[3].x = PLAYER_SSHRINK * getWidth() / 2.0f;
    corners[3].y = (-getHeight() + PLAYER_SENSOR_HEIGHT) / 2.0f;

    b2PolygonShape sensorShape;
    sensorShape.Set(corners, 4);

    sensorDef.shape = &sensorShape;
    sensorDef.userData.pointer = reinterpret_cast<uintptr_t>(getGroundSensorName());
    _sensorFixture = _body->CreateFixture(&sensorDef);

    // create shield circle fixture
    b2FixtureDef shieldDef;
    b2CircleShape shieldShape;
    shieldShape.m_radius = PLAYER_SHIELD_RADIUS;
    shieldShape.m_p.Set(0, 0);//center of body
    shieldDef.isSensor = true;
    shieldDef.shape = &shieldShape;
    shieldDef.userData.pointer = reinterpret_cast<uintptr_t>(getShieldName());
    _shieldFixture = _body->CreateFixture(&shieldDef);

}

/**
 * Release the fixtures for this body, reseting the shape
 *
 * This is the primary method to override for custom physics objects.
 */
void PlayerModel::releaseFixtures() {
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
void PlayerModel::dispose() {
    _geometry = nullptr;
    _sceneNode = nullptr;
    _sensorNode = nullptr;
    _shieldNode = nullptr;
    _currentSpriteNode = nullptr;
    _idleSprite = nullptr;
    _guardSprite = nullptr;
    _walkSprite = nullptr;
    _jumpUpSprite = nullptr;
    _jumpDownSprite = nullptr;
    _attackSprite = nullptr;
}

#pragma mark Cooldowns
/**
 * Updates the object's physics state (NOT GAME LOGIC).
 *
 * We use this method to reset cooldowns.
 *
 * @param delta Number of seconds since last animation frame
 */
void PlayerModel::update(float dt) {
    updateAnimation();
    BoxObstacle::update(dt);
    if (_sceneNode != nullptr) {
        _sceneNode->setPosition(getPosition() * _drawScale);
        _sceneNode->setAngle(getAngle());
    }
}

#pragma mark -
#pragma mark Animation
void PlayerModel::playAnimation(std::shared_ptr<scene2::SpriteNode> sprite) {
    currentFrame += 1;

    if (currentFrame % ANIMATION_UPDATE_FRAME == 0) {
        sprite->setFrame((sprite->getFrame() + 1) % sprite->getCount());
    }
}

void PlayerModel::playAnimationOnce(std::shared_ptr<scene2::SpriteNode> sprite) {
    currentFrame += 1;

    if (currentFrame % ANIMATION_UPDATE_FRAME == 0
        && sprite->getFrame() < sprite->getCount() - 1) {
        sprite->setFrame(sprite->getFrame() + 1);
    }
}


void PlayerModel::updateAnimation()
{
    if (isGuardActive()) {
        _guardSprite->setVisible(true);
        _attackSprite->setVisible(false);
        _jumpUpSprite->setVisible(false);
        _jumpDownSprite->setVisible(false);
        _walkSprite->setVisible(false);
        _idleSprite->setVisible(false);

        if (_isGuardInput) {
            currentFrame = 0;
            _guardSprite->setFrame(0);
        }

        playAnimation(_guardSprite);
    }

    else if (isDashActive()) {
        _guardSprite->setVisible(false);
        _attackSprite->setVisible(true);
        _jumpUpSprite->setVisible(false);
        _jumpDownSprite->setVisible(false);
        _walkSprite->setVisible(false);
        _idleSprite->setVisible(false);

        if (isDashInput()) {
            currentFrame = 0;
            _attackSprite->setFrame(0);
        }

        playAnimation(_attackSprite);
    }

    else if (isJumpBegin()) {
        _guardSprite->setVisible(false);
        _attackSprite->setVisible(false);
        _jumpUpSprite->setVisible(true);
        _jumpDownSprite->setVisible(false);
        _walkSprite->setVisible(false);
        _idleSprite->setVisible(false);

        currentFrame = 0;
        _jumpUpSprite->setFrame(0);
    }
    else if (getVY() > 0 && !isGrounded()) {
        _guardSprite->setVisible(false);
        _attackSprite->setVisible(false);
        _jumpUpSprite->setVisible(true);
        _jumpDownSprite->setVisible(false);
        _walkSprite->setVisible(false);
        _idleSprite->setVisible(false);

        playAnimationOnce(_jumpUpSprite);
    }
    else if (getVY() == 0 && !isGrounded()) {
        _guardSprite->setVisible(false);
        _attackSprite->setVisible(false);
        _jumpUpSprite->setVisible(false);
        _jumpDownSprite->setVisible(true);
        _walkSprite->setVisible(false);
        _idleSprite->setVisible(false);

        currentFrame = 0;
        _jumpDownSprite->setFrame(0);
    }
    else if (getVY() < 0 && !isGrounded()) {
        _guardSprite->setVisible(false);
        _attackSprite->setVisible(false);
        _jumpUpSprite->setVisible(false);
        _jumpDownSprite->setVisible(true);
        _walkSprite->setVisible(false);
        _idleSprite->setVisible(false);

        playAnimationOnce(_jumpDownSprite);
    }

    else if (isStrafeLeft() || isStrafeRight()) {
        _guardSprite->setVisible(false);
        _attackSprite->setVisible(false);
        _jumpUpSprite->setVisible(false);
        _jumpDownSprite->setVisible(false);
        _walkSprite->setVisible(true);
        _idleSprite->setVisible(false);

        playAnimation(_walkSprite);
    }

    else {
        _guardSprite->setVisible(false);
        _attackSprite->setVisible(false);
        _jumpUpSprite->setVisible(false);
        _jumpDownSprite->setVisible(false);
        _walkSprite->setVisible(false);
        _idleSprite->setVisible(true);

        playAnimation(_idleSprite);
    }

    _sceneNode->setScale(Vec2(isFacingRight() ? 1 : -1, 1));
    _sceneNode->getChild(_sceneNode->getChildCount() - 1)->setScale(Vec2(isFacingRight() ? 1 : -1, 1));
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
void PlayerModel::resetDebug() {
    BoxObstacle::resetDebug();
    float w = PLAYER_SSHRINK * _dimension.width;
    float h = PLAYER_SENSOR_HEIGHT;
    Poly2 playerPoly(Rect(-w / 0.1f, -h / 2.0f, w, h));
    _sensorNode = scene2::WireNode::allocWithTraversal(playerPoly, poly2::Traversal::INTERIOR);
    _sensorNode->setColor(PLAYER_SENSOR_DEBUG_COLOR);
    _sensorNode->setPosition(Vec2(_debug->getContentSize().width / 2.0f, 0.0f));

    Poly2 shieldPoly;
    shieldPoly = PolyFactory().makeNgon(10000, 0, PLAYER_SHIELD_RADIUS, 20);
    _shieldNode = scene2::WireNode::allocWithPoly(shieldPoly);
    _shieldNode->setPosition(Vec2(_debug->getContentSize() / 2));
    _shieldNode->setColor(PLAYER_DEBUG_COLOR);

    _debug->addChild(_sensorNode);
    _debug->addChild(_shieldNode);
}



