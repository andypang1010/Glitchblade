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
    
    Size nsize = Size(90, 130) / scale;
    nsize.width  *= _enemyJSON->get("fixtures")->get("body")->getFloat("h_shrink");
    nsize.height *= _enemyJSON->get("fixtures")->get("body")->getFloat("h_shrink");
    _drawScale = scale;
    
    setDebugColor(_enemyJSON->get("debug")->getString("color"));
    if (BoxObstacle::init(pos,nsize)) {
        setDensity(ENEMY_DENSITY);
        setFriction(0.0f);      // HE WILL STICK TO WALLS IF YOU FORGET
        setFixedRotation(true); // OTHERWISE, HE IS A WEEBLE WOBBLE
        attachNodes(assetRef);
        return true;
    }
    return false;
}

void EnemyModel::attachNodes(const std::shared_ptr<AssetManager>& assetRef){
    _node = scene2::SceneNode::alloc();
    setSceneNode(_node);
    //move this to new function
    _idleSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss1_idle"), 1, 6, 6);
    _idleSprite->setPosition(0, 40);

    _walkSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss1_walking1"), 1, 8, 8);
    _walkSprite->setPosition(0, 40);

    _slamSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss1_slam"), 4, 10, 40);
    _slamSprite->setPosition(0, 40);

    _stabSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss1_stab"), 4, 10, 40);
    _stabSprite->setPosition(0, 40);

    _stunSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss1_stun"), 3, 10, 22);
    _stunSprite->setPosition(0, 40);

    setName(std::string(ENEMY_NAME));
    setDebugColor(ENEMY_DEBUG_COLOR);
    
    getSceneNode()->addChild(_idleSprite);
    getSceneNode()->addChild(_walkSprite);
    getSceneNode()->addChild(_slamSprite);
    getSceneNode()->addChild(_stabSprite);
    getSceneNode()->addChild(_stunSprite);
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
    
    // Sensor dimensions
    b2Vec2 corners[4];
    corners[0].x = -ENEMY_SSHRINK*getWidth()/2.0f;
    corners[0].y = (-getHeight()+ENEMY_SENSOR_HEIGHT)/2.0f;
    corners[1].x = -ENEMY_SSHRINK*getWidth()/2.0f;
    corners[1].y = (-getHeight()-ENEMY_SENSOR_HEIGHT)/2.0f;
    corners[2].x = ENEMY_SSHRINK*getWidth()/2.0f;
    corners[2].y = (-getHeight()-ENEMY_SENSOR_HEIGHT)/2.0f;
    corners[3].x = ENEMY_SSHRINK*getWidth()/2.0f;
    corners[3].y = (-getHeight()+ENEMY_SENSOR_HEIGHT)/2.0f;
    
    b2PolygonShape sensorShape;
    sensorShape.Set(corners,4);
    
    sensorDef.shape = &sensorShape;
    sensorDef.userData.pointer = reinterpret_cast<uintptr_t>(getSensorName());
    _sensorFixture = _body->CreateFixture(&sensorDef);
    
    // create shield circle fixture
    b2FixtureDef shieldDef;
    b2CircleShape shieldShape;
    shieldShape.m_radius = ENEMY_SHIELD_RADIUS;
    shieldShape.m_p.Set(getWidth()/2, getHeight()/2);//center of body
    shieldDef.isSensor = true;
    shieldDef.shape = &sensorShape;
    shieldDef.userData.pointer = reinterpret_cast<uintptr_t>(getShieldName());
    _shieldFixture = _body->CreateFixture(&shieldDef);

    // create attack fixtures
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
    _shieldNode = nullptr;
    _geometry = nullptr;
    _sensorNode = nullptr;
    _shieldNode = nullptr;
    _currentSpriteNode = nullptr;
    _idleSprite = nullptr;
    _walkSprite = nullptr;

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
    updateAnimation();
    nextAction();

    // Apply cooldowns
    
#pragma mark Guard and Parry timing
    // guard cooldown first for most recent movement inputs
    if (isGuardActive() && !isGuardBegin()){
        //just for logging end of parry
        bool parry_active = isParryActive();
        // If robot moves, guard is cancelled
        if (isMoveBegin()){
            _guardRem = 0;
            _parryRem = 0;
            CULog("Parry and Guard ended due to movement\n");
        }
        else{
            _guardRem = (_guardRem > 0 ? _guardRem -1 : 0);
            _parryRem= (_parryRem > 0 ? _parryRem -1 : 0);
            
            //The rest of this block is for logging end of guard&parry
            if (parry_active && (_parryRem == 0)){
                CULog("Parry completed during guard\n");
            }
            if (_guardRem == 0){
                CULog("Enemy Guard completed full duration\n");
            }
        }
    }
    // guard not active, update cooldown
    else {
        _guardCooldownRem = (_guardCooldownRem > 0 ? _guardCooldownRem -1 : 0);
    }
    if (isJumpBegin() && isGrounded()) {
        CULog("isJumping is true");
        _jumpCooldownRem = ENEMY_JUMP_COOLDOWN;
    } else {
        _jumpCooldownRem = (_jumpCooldownRem > 0 ? _jumpCooldownRem-1 : 0);
    }
    
    if (isKnocked()) {
        CULog("enmey knockback applied");
        _dashCooldownRem = ENEMY_DASH_COOLDOWN;
        _jumpCooldownRem = ENEMY_JUMP_COOLDOWN;
        _shootCooldownRem = ENEMY_SHOOT_COOLDOWN;
    }
    
    if (isShooting()) {
        _shootCooldownRem = ENEMY_SHOOT_COOLDOWN;
    } else {
        _shootCooldownRem = (_shootCooldownRem > 0 ? _shootCooldownRem-1 : 0);
    }
    
    if (isDashBegin()) {
        _dashRem = ENEMY_DASH_DURATION;
        _dashCooldownRem = ENEMY_DASH_COOLDOWN;
    }
    else {
        _dashRem = (_dashRem > 0 ? _dashRem-1 : 0);
        if (_dashRem == 0){
            _dashCooldownRem = (_dashCooldownRem > 0 ? _dashCooldownRem-1 : 0);
        }
    }
    // player inputs guard and cooldown is ready
    if (isGuardBegin()) {
        CULog("Beginning guard\n");
        _guardCooldownRem = ENEMY_GUARD_COOLDOWN;
        _guardRem = ENEMY_GUARD_DURATION;
    }

    if (isStunned()) {
        CULog("Enemy stunned\n");
        _stunRem--;
    }
    
    BoxObstacle::update(dt);
    if (_node != nullptr) {
        _node->setPosition(getPosition()*_drawScale);
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
    if (!_isSlamming && !_isStabbing && _moveDuration <= 0 && isTargetClose(_targetPos) && !isStunned()) {
        if (r%3 == 0) { //Slam
            _isSlamming = true;
            setMovement(0);
        }
        else if(r % 3 == 1){ // Stab
            _isStabbing = true;
            setMovement(0);
        }
        else { // Move away
            _moveDuration = 45;
            _moveDirection = -1;
        }
    }
    else if (!_isSlamming && !_isStabbing && _moveDuration <= 0 && !isStunned()) {
        if (r % 2 == 0) { // Stab
            _isStabbing = true;
            setMovement(0);
        }
        else{ // Move closer
            _moveDuration = 45;
            _moveDirection = 1;
        }
    }
    else {
        if (isStunned()) {
            _isSlamming = false;
            _isStabbing = false;
            setMovement(0);
        }
        if (_isSlamming && _slamSprite->getFrame() >= SLAM_FRAMES-1) {
            _isSlamming = false;
            setMovement(0);
        }
        if (_isStabbing && _stabSprite->getFrame() >= STAB_FRAMES-1) {
            _isStabbing = false;
            setMovement(getMovement());
        }
    }
}

void EnemyModel::AIMove() {
    float dist = getPosition().x - _targetPos.x;
    float dir_val = dist > 0 ? -1 : 1;
    int face = _faceRight ? 1 : -1;
    
    if (_moveDuration > 0 && !_isStabbing) {
        setMovement(_moveDirection * dir_val* getForce());
        setStrafeLeft(dist > 0);
        setStrafeRight(dist < 0);
        _moveDuration--;
    }
    //else if (_moveDuration > 0) {
    //    _moveDuration--;
    //}
    else if (_isStabbing && _stabSprite->getFrame() >= STAB_DAMAGE_START_FRAME - 1 && _stabSprite->getFrame() <= STAB_DAMAGE_END_FRAME - 1) {
        /*_faceRight ? setDashRightInput(true) : setDashLeftInput(true);*/
        /*b2Vec2 force(face * ENEMY_DASH, 0);
        _body->ApplyLinearImpulseToCenter(force, true);*/
        /*_moveDuration = STAB_DAMAGE_END_FRAME - STAB_DAMAGE_START_FRAME;*/
        setMovement(face * getForce() * ENEMY_DASH);
    }
    
}

bool EnemyModel::isDamaging() {
    if (_isSlamming && _slamSprite->getFrame() >= SLAM_DAMAGE_START_FRAME-1 && _slamSprite->getFrame() <= SLAM_DAMAGE_END_FRAME - 1) {
        return true;
    }
    else if (_isStabbing && _stabSprite->getFrame() >= STAB_DAMAGE_START_FRAME - 1 && _stabSprite->getFrame() <= STAB_DAMAGE_END_FRAME - 1) {
        return true;
    }
    else {
        return false;
    }
}

#pragma mark -
#pragma mark Animation Methods
void EnemyModel::playAnimation(std::shared_ptr<scene2::SpriteNode> sprite) {
    if (sprite->isVisible()) {
        currentFrame++;
        if (currentFrame > sprite->getCount()*E_ANIMATION_UPDATE_FRAME) {
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
    
    _walkSprite->setVisible(!isStunned() && !_isStabbing && !_isSlamming && isGrounded() && !isDashActive() && (isStrafeLeft() || isStrafeRight()));

    _slamSprite->setVisible(!isStunned() && _isSlamming);

    _stabSprite->setVisible(!isStunned() && _isStabbing);

    if (_stunRem == STUN_FRAMES) {
        currentFrame = 0;
    }

    _idleSprite->setVisible(!isStunned() && !_slamSprite->isVisible() && !_stabSprite->isVisible() && !_walkSprite->isVisible());

    playAnimation(_walkSprite);
    playAnimation(_idleSprite);
    playAnimation(_slamSprite);
    playAnimation(_stabSprite);
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
void EnemyModel::resetDebug() {
    BoxObstacle::resetDebug();
    float w = ENEMY_SSHRINK*_dimension.width;
    float h = ENEMY_SENSOR_HEIGHT;
    Poly2 dudePoly(Rect(-w/0.1f,-h/2.0f,w,h));
    _sensorNode = scene2::WireNode::allocWithTraversal(dudePoly, poly2::Traversal::INTERIOR);
    _sensorNode->setColor(ENEMY_DEBUG_COLOR);
    _sensorNode->setPosition(Vec2(_debug->getContentSize().width/2.0f, 0.0f));

    Poly2 shieldPoly;
    shieldPoly = PolyFactory().makeCircle(_debug->getContentWidth()/2,_debug->getContentHeight()/2, ENEMY_SHIELD_RADIUS);
    _shieldNode = scene2::WireNode::allocWithPoly(shieldPoly);
    _shieldNode->setColor(ENEMY_DEBUG_COLOR);
    
}

