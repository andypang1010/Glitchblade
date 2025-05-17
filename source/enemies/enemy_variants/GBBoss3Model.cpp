#include "GBBoss3Model.h"
#include <cugl/scene2/CUPolygonNode.h>
#include <cugl/scene2/CUTexturedNode.h>
#include <cugl/core/assets/CUAssetManager.h>
#include "../actionmodel_variants/GBRangedActionModel.h"

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
bool Boss3Model::init(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& enemyJSON, const Vec2& pos, std::vector<std::shared_ptr<ActionModel>> actions) {
    _shootCount = 0;
    _isGroundForm = true;
    _stunMeter = 0;
    return EnemyModel::init(assetRef, enemyJSON, pos, actions);
};

void Boss3Model::attachNodes(const std::shared_ptr<AssetManager>& assetRef) {
    EnemyModel::attachNodes(assetRef);
    float scale = 0.5;
    float YOffset = 65;
    //move this to new function
    _groundIdleSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss3_ground_idle"), 3, 5, 15);
    _groundIdleSprite->setPosition(0, YOffset * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _groundIdleSprite->setScale(scale*2 * 0.0004006410 * Application::get()->getDisplayWidth());
    _groundIdleSprite->setName("groundIdle");

    _airIdleSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss3_air_idle"), 3, 5, 15);
    _airIdleSprite->setPosition(0, YOffset * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _airIdleSprite->setScale(scale*2 * 0.0004006410 * Application::get()->getDisplayWidth());
    _airIdleSprite->setName("airIdle");

    _groundStunSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss3_ground_stun"), 6, 9, 53);
    _groundStunSprite->setPosition(0, YOffset * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _groundStunSprite->setScale(scale*2 * 0.0004006410 * Application::get()->getDisplayWidth());
    _groundStunSprite->setName("groundStun");
    stunFrames = 20;

    _airStunSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss3_air_stun"), 6, 9, 53);
    _airStunSprite->setPosition(0, YOffset * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _airStunSprite->setScale(scale*2 * 0.0004006410 * Application::get()->getDisplayWidth());
    _airStunSprite->setName("airStun");

    _uppercutSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss3_ground_uppercut"), 3, 4, 12);
    _uppercutSprite->setPosition(0, YOffset * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _uppercutSprite->setScale(scale*2 * 0.0004006410 * Application::get()->getDisplayWidth());
    _uppercutSprite->setName("uppercut");

    _slamSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss3_ground_slam"), 4, 5, 20);
    _slamSprite->setPosition(0, YOffset * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _slamSprite->setScale(scale*2 * 0.0004006410 * Application::get()->getDisplayWidth());
    _slamSprite->setName("slam");

    _jumpStartSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss3_ground_jump_start"), 3, 3, 9);
    _jumpStartSprite->setPosition(0, YOffset * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _jumpStartSprite->setScale(scale*2 * 0.0004006410 * Application::get()->getDisplayWidth());
    _jumpStartSprite->setName("jumpStart");

    _jumpWaitSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss3_ground_jump_wait"), 3, 3, 9);
    _jumpWaitSprite->setPosition(0, YOffset * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _jumpWaitSprite->setScale(scale * 2 * 0.0004006410 * Application::get()->getDisplayWidth());
    _jumpWaitSprite->setName("jumpWait");

    _jumpEndSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss3_ground_jump_end"), 3, 3, 9);
    _jumpEndSprite->setPosition(0, YOffset * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _jumpEndSprite->setScale(scale * 2 * 0.0004006410 * Application::get()->getDisplayWidth());
    _jumpEndSprite->setName("jumpEnd");

    _dashSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss3_dash"), 5, 9, 45);
    _dashSprite->setPosition(0, YOffset * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _dashSprite->setScale(scale*2 * 0.0004006410 * Application::get()->getDisplayWidth());
    _dashSprite->setName("dash");

    _shootStartSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss3_air_shoot_start"), 3, 5, 15);
    _shootStartSprite->setPosition(0, YOffset * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _shootStartSprite->setScale(scale*2 * 0.0004006410 * Application::get()->getDisplayWidth());
    _shootStartSprite->setName("shootStart");

    _shootAttackSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss3_air_shoot_attack"), 2, 4, 7);
    _shootAttackSprite->setPosition(0, YOffset * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _shootAttackSprite->setScale(scale*2 * 0.0004006410 * Application::get()->getDisplayWidth());
    _shootAttackSprite->setName("shootAttack");

    _shootLaserSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss3_air_shoot_laser"), 5, 9, 38);
    _shootLaserSprite->setPosition(0, YOffset * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _shootLaserSprite->setScale(scale * 2 * 0.0004006410 * Application::get()->getDisplayWidth());
    _shootLaserSprite->setName("shootLaser");

    _shootWaitSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss3_air_shoot_wait"), 2, 4, 7);
    _shootWaitSprite->setPosition(0, YOffset * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _shootWaitSprite->setScale(scale*2 * 0.0004006410 * Application::get()->getDisplayWidth());
    _shootWaitSprite->setName("shootWait");

    _laserVFXSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("laser"), 19, 2, 38);
    _laserVFXSprite->setPosition(475 * 2 * 0.0004006410 * Application::get()->getDisplayWidth(), -750 * 0.0004006410 * Application::get()->getDisplayWidth());
	_laserVFXSprite->setScale(scale * 2 * 0.0004006410 * Application::get()->getDisplayWidth());
    _laserVFXSprite->setAngle(-M_PI/4);
    _laserVFXSprite->setName("laser_vfx");

	_groundDeadSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss3_ground_dead"), 5, 9, 45);
    _groundDeadSprite->setPosition(0, YOffset * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _groundDeadSprite->setScale(scale*2 * 0.0004006410 * Application::get()->getDisplayWidth());
    _groundDeadSprite->setName("groundDead");
    _groundDeadSprite->setVisible(false);

    _airDeadSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss3_air_dead"), 5, 9, 45);
    _airDeadSprite->setPosition(0, YOffset * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
    _airDeadSprite->setScale(scale*2 * 0.0004006410 * Application::get()->getDisplayWidth());
    _airDeadSprite->setName("airDead");
	_airDeadSprite->setVisible(false);

	_groundTransformSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss3_ground_transform"), 4, 4, 16);
	_groundTransformSprite->setPosition(0, YOffset * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
	_groundTransformSprite->setScale(scale * 2 * 0.0004006410 * Application::get()->getDisplayWidth());
	_groundTransformSprite->setName("groundTransform");

	_airTransformSprite = scene2::SpriteNode::allocWithSheet(assetRef->get<Texture>("boss3_air_transform"), 4, 4, 16);
	_airTransformSprite->setPosition(0, YOffset * (2 * 0.0004006410 * Application::get()->getDisplayWidth()));
	_airTransformSprite->setScale(scale * 2 * 0.0004006410 * Application::get()->getDisplayWidth());
	_airTransformSprite->setName("airTransform");

	getSceneNode()->addChild(_groundIdleSprite);
	getSceneNode()->addChild(_airIdleSprite);
	getSceneNode()->addChild(_groundStunSprite);
	getSceneNode()->addChild(_airStunSprite);

	getSceneNode()->addChild(_uppercutSprite);
	getSceneNode()->addChild(_slamSprite);
	getSceneNode()->addChild(_dashSprite);

	getSceneNode()->addChild(_groundTransformSprite);
	getSceneNode()->addChild(_airTransformSprite);

    getSceneNode()->addChild(_jumpStartSprite);
	getSceneNode()->addChild(_jumpWaitSprite);
	getSceneNode()->addChild(_jumpEndSprite);

	getSceneNode()->addChild(_shootStartSprite);
	getSceneNode()->addChild(_shootAttackSprite);
	getSceneNode()->addChild(_shootWaitSprite);

	getSceneNode()->addChild(_groundDeadSprite);
	getSceneNode()->addChild(_airDeadSprite);

    getSceneNode()->addChild(_laserVFXSprite);
    getSceneNode()->addChild(_shootLaserSprite);
}

void Boss3Model::setActions(std::vector<std::shared_ptr<ActionModel>> actions) {
    for (auto act : actions) {
        if (act->getActionName() == "uppercut") {
            _uppercut = std::dynamic_pointer_cast<MeleeActionModel>(act);
        }
		else if (act->getActionName() == "slam") {
			_slam = std::dynamic_pointer_cast<MeleeActionModel>(act);
		}
		else if (act->getActionName() == "jump") {
			_jump = std::dynamic_pointer_cast<MeleeActionModel>(act);
		}
		else if (act->getActionName() == "dash") {
			_dash = std::dynamic_pointer_cast<MeleeActionModel>(act);
		}
		else if (act->getActionName() == "shoot") {
			_shoot = std::dynamic_pointer_cast<RangedActionModel>(act);
		}
        else if (act->getActionName() == "laser") {
            _laser = std::dynamic_pointer_cast<MeleeActionModel>(act);
        }
    }
    _closeDistance = 12;
    _farDistance = 18;
}

#pragma mark -
#pragma mark Physics Methods
/**
 * Create new fixtures for this body, defining the shape
 *
 * This is the primary method to override for custom physics objects
 */
void Boss3Model::createFixtures() {
    EnemyModel::createFixtures();
}

/**
 * Release the fixtures for this body, reseting the shape
 *
 * This is the primary method to override for custom physics objects.
 */
void Boss3Model::releaseFixtures() {
    EnemyModel::releaseFixtures();
}

/**
 * Disposes all resources and assets of this PlayerModel
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a PlayerModel may not be used until it is initialized again.
 */
void Boss3Model::dispose() {
    _geometry = nullptr;
    _node = nullptr;
    _groundSensorNode = nullptr;
    _geometry = nullptr;
    _currentSpriteNode = nullptr;
    _idleSprite = nullptr;
    _walkSprite = nullptr;
    _stunSprite = nullptr;
	_laserVFXSprite = nullptr;
    _deadSprite = nullptr;

	_groundIdleSprite = nullptr;
	_airIdleSprite = nullptr;
	_groundStunSprite = nullptr;
	_airStunSprite = nullptr;
	_uppercutSprite = nullptr;
	_slamSprite = nullptr;
	_jumpStartSprite = nullptr;
	_jumpWaitSprite = nullptr;
	_jumpEndSprite = nullptr;
	_dashSprite = nullptr;
	_shootStartSprite = nullptr;
	_shootAttackSprite = nullptr;
	_shootLaserSprite = nullptr;
	_shootWaitSprite = nullptr;
	_groundDeadSprite = nullptr;
	_airDeadSprite = nullptr;
	_groundTransformSprite = nullptr;
	_airTransformSprite = nullptr;
}

void Boss3Model::damage(float value) {
    EnemyModel::damage(value);
    if (!isStunned()) {
        setStun(value);
    }
}

void Boss3Model::setStun(int value) {
    if (isStunned()) {
        _stunRem = value;
    }
    else {
        _stunMeter = std::min(100, _stunMeter + value);
        if (_stunMeter >= 100) {
            _stunMeter = 0;
            _stunRem = actualStunFrames;
            if (!_isGroundForm) { setGravityScale(1); }
        }
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
void Boss3Model::update(float dt) {
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
void Boss3Model::nextAction() {
    int r = rand();
    AIMove();
	if (!isStunned() && !_isUppercutting && !_isSlamming && !getIsJumping() && !_isDashing && !_isGroundDashStarting && !_isGroundDashEnding
        && !_isShootStarting && !_isShootAttacking && !_isLaserAttacking && !_isShootWaiting) {
        if (_isGroundForm) {
            handleGroundAction(r);
        }
        else {
            handleAirAction(r);
        }
    }
    else {
        if (isStunned()) {
            _isUppercutting = false;
			_isSlamming = false;
			_isJumpStarting = false;
			_isJumpWaiting = false;
			_isJumpEnding = false;
			_isDashing = false;
			_isGroundDashStarting = false;
			_isGroundDashEnding = false;
			_isShootStarting = false;
			_isShootAttacking = false;
			_isLaserAttacking = false;
			_isShootWaiting = false;
			
            setMovement(0);

            if (_groundStunSprite->getFrame() >= _groundStunSprite->getCount() - 1 || _airStunSprite->getFrame() >= _airStunSprite->getCount() - 1) {
				_isGroundForm = !_isGroundForm;
                if (!_isGroundForm) { setGravityScale(0); }
            }
        }
        if (_isUppercutting && _uppercutSprite->getFrame() >= _uppercutSprite->getCount() - 1) {
            _isUppercutting = false;
            setMovement(0);
        }
		if (_isSlamming && _slamSprite->getFrame() >= _slamSprite->getCount() - 1) {
			_isSlamming = false;
			setMovement(0);
		}
        if (_isJumpStarting || _isJumpWaiting || _isJumpEnding) {
            handleJump();
        }
        if (_isDashing || _isGroundDashStarting || _isGroundDashEnding) {
            if (_isGroundForm) {
                handleGroundDash();
            }
            else {
                if (_isDashing && _dashSprite->getFrame() >= _dashSprite->getCount() - 1) {
                    _isDashing = false;
                    setMovement(0);
                }
            }
        }
        if (_isShootStarting || _isShootAttacking || _isLaserAttacking || _isShootWaiting) {
			handleShoot();
        }
    }
}

void Boss3Model::handleGroundAction(int r) {
    if (isTargetClose()) {
        if (r % 4 == 0) { // Uppercut
            uppercut();
        }
        else if (r % 4 == 1) { // Slam
            slam();
        }
        else if (r % 4 == 2) { // Jump
            jump();
        }
        else if (r % 4 == 3) { // Dash
            dash();
        }
    }
}

void Boss3Model::handleAirAction(int r) {
    if (std::abs(_targetPos.y - getPosition().y) <= 1.5 && r % 4 == 0) {
        dash();
    }
    else if (!isTargetFar()) {
        if (r % 3 == 0) {
            avoidTarget(60);
        }
        else if (r % 3 == 1) {
            approachTarget(60);
        }
        else if (_targetPos.y - getPosition().y <= -8){
            shoot(1);
        }
        
    }
    else {
        if (r % 3 == 0) {
            approachTarget(60);
        }
        else if (r % 3 == 1) {
            avoidTarget(60);
        }
        else if (_targetPos.y - getPosition().y <= -8) {
            shoot(3);
        }
    }
}

void Boss3Model::AIMove() {
    float dist = getPosition().x - _targetPos.x;
    float dir_val = dist > 0 ? -1 : 1;
    int face = _faceRight ? 1 : -1;

    if (_isGroundForm) {
        if (!getIsJumping() && !_isDashing) {
            setVerticalMovement(0);
            if (_moveDuration == 0) {
                setMovement(0);
            }
            else {
                setMovement(_moveDirection * dir_val * getForce());
                setMoveLeft(dist > 0);
                setMoveRight(dist < 0);
                _moveDuration--;
            }
        }
        else if (getIsJumping() && isJumpingUp()) {
            setMovement(dir_val * getForce() * 2);
            setVerticalMovement(getForce() * 2);
        }
        else if (_isDashing && _dashSprite->getFrame() >= _dash->getHitboxStartFrame() - 1) {
            setMovement(face * 15000);
        }
    }
    else {
        if (_moveDuration > 0) {
            if (_worldTop - getPosition().y <= 4) { // near top, quickly move down
                setVerticalMovement(-getForce()*8);
            }
            else {
                if (getPosition().y <= 10) {
                    setVerticalMovement(rand() % 10 <= 7 ? getForce()*3 : -getForce()*3);
                }
                else {
                    setVerticalMovement(rand() % 10 <= 7 ? -getForce()*3 : getForce()*3);
                }
            }

            setMovement(_moveDirection * dir_val * getForce() * 8);
            setMoveLeft(dist > 0);
            setMoveRight(dist < 0);
            _moveDuration--;
        }
        else if (_isDashing && _dashSprite->getFrame() >= _dash->getHitboxStartFrame() - 1) {
            setVerticalMovement(0);
            setMovement(face * 15000);
        }
        else if (_isLaserAttacking && _shootLaserSprite->getFrame() >= _laser->getHitboxStartFrame() - 1) {
            setVerticalMovement(0);
            setMovement(face * 5000);
        }
        else {
            setMovement(0);
        }
    }
    
}

void Boss3Model::uppercut() {
    faceTarget();
    _isUppercutting = true;
    setMovement(0);
}

void Boss3Model::slam() {
	faceTarget();
	_isSlamming = true;
	setMovement(0);
}

void Boss3Model::jump() {
    if (getPosition().y < 4.2) {
        faceTarget();
        _isJumpStarting = true;
        setMovement(0);
    }
}

void Boss3Model::handleJump() {
    if (_isJumpStarting && _jumpStartSprite->getFrame() >= _jumpStartSprite->getCount() - 1) {
        _isJumpStarting = false;
        _isJumpWaiting = true;
    }
    else if (_isJumpWaiting && getPosition().y < 4.6 && getLinearVelocity().y < 0) {
        _isJumpWaiting = false;
        _isJumpEnding = true;
    }
    else if (_isJumpEnding && _jumpEndSprite->getFrame() >= _jumpEndSprite->getCount() - 1) {
        _isJumpEnding = false;
		setMovement(0);
    }
}

bool Boss3Model::isJumpingUp() {
	return _isJumpStarting && _jumpStartSprite->getFrame() >= 6;
}

void Boss3Model::dash() {
	faceTarget();
    if (_isGroundForm) {
		_isGroundDashStarting = true;
    }
    else {
        _isDashing = true;
        setMovement(0);
        setVerticalMovement(0);
    }
}

void Boss3Model::handleGroundDash() {
	if (_isGroundDashStarting && _groundTransformSprite->getFrame() >= _groundTransformSprite->getCount() - 1) {
		_isGroundDashStarting = false;
		_isDashing = true;
	}
	else if (_isDashing && _dashSprite->getFrame() >= _dashSprite->getCount() - 1) {
		_isDashing = false;
		_isGroundDashEnding = true;
    }
    else if (_isGroundDashEnding && _airTransformSprite->getFrame() >= _airTransformSprite->getCount() - 1) {
		_isGroundDashEnding = false;
        setMovement(0);
    }
}

void Boss3Model::shoot(int repeat) {
	faceTarget();
	_isShootStarting = true;
	_shootCount = repeat;
}

void Boss3Model::handleShoot() {
	if (_isShootStarting && _shootStartSprite->getFrame() >= _shootStartSprite->getCount() - 1) {
        if (rand() % 2 == 0) { // shoot projectile
            _isShootStarting = false;
            _isShootAttacking = true;
            _shootCount--;
        }
        else {
            _isShootStarting = false;
            _isLaserAttacking = true;
            _shootCount = 0;
        }
	}
	else if (_isShootAttacking && _shootAttackSprite->getFrame() >= _shootAttackSprite->getCount() - 1) {
        // wait for a bit
		_isShootAttacking = false;
		_isShootWaiting = true;
	}
	else if (_isShootWaiting && _shootWaitSprite->getFrame() >= _shootWaitSprite->getCount() - 1) {
		if (_shootCount <= 0) { // stop repeated shooting
			_isShootWaiting = false;
		}
		else { // return to shooting
			_isShootWaiting = false;
			_isShootAttacking = true;
			_shootCount--;
		}
    }
    else if (_isLaserAttacking && _shootLaserSprite->getFrame() >= _shootLaserSprite->getCount() - 1) {
        _isLaserAttacking = false;
		_isShootWaiting = true;
    }
}

void Boss3Model::laser() {
    faceTarget();
}

std::shared_ptr<MeleeActionModel> Boss3Model::getDamagingAction() {
	if (_isUppercutting && _uppercutSprite->getFrame() == _uppercut->getHitboxStartFrame() - 1) {
		return _uppercut;
	}
    else if (_isSlamming && _slamSprite->getFrame() == _slam->getHitboxStartFrame() - 1) {
        return _slam;
	}
	else if (_isJumpWaiting && _jumpWaitSprite->getFrame() == _jump->getHitboxStartFrame() - 1) {
		return _jump;
	}
	else if (_isDashing && _dashSprite->getFrame() == _dash->getHitboxStartFrame() - 1) {
		return _dash;
    }
    else if (_isLaserAttacking && _shootLaserSprite->getFrame() == _laser->getHitboxStartFrame() - 1) {
        return _laser;
    }
    return nullptr;
}

std::shared_ptr<Projectile> Boss3Model::getProjectile() {
    std::vector<int> frames = _shoot->getProjectileSpawnFrames();
    int count = 0;
    for (int frame : frames) {
        if (_isShootAttacking && _shootAttackSprite->getFrame() == frame && frameCounter == 0) {
            return _shoot->getProjectiles()[count];
        }
        count++;
    }
    return nullptr;
}

#pragma mark -
#pragma mark Animation Methods

void Boss3Model::updateAnimation()
{
    _groundStunSprite->setVisible(isStunned() && _isGroundForm);
	_airStunSprite->setVisible(isStunned() && !_isGroundForm);

	_uppercutSprite->setVisible(!isStunned() && _isUppercutting);
	_slamSprite->setVisible(!isStunned() && _isSlamming);
	_dashSprite->setVisible(!isStunned() && _isDashing);

	_groundTransformSprite->setVisible(!isStunned() && _isGroundDashStarting);
	_airTransformSprite->setVisible(!isStunned() && _isGroundDashEnding);

    _jumpStartSprite->setVisible(!isStunned() && _isJumpStarting);
	_jumpWaitSprite->setVisible(!isStunned() && _isJumpWaiting);
	_jumpEndSprite->setVisible(!isStunned() && _isJumpEnding);

	_shootStartSprite->setVisible(!isStunned() && _isShootStarting);
	_shootAttackSprite->setVisible(!isStunned() && _isShootAttacking);
	_shootLaserSprite->setVisible(!isStunned() && _isLaserAttacking);
	_shootWaitSprite->setVisible(!isStunned() && _isShootWaiting);

    _laserVFXSprite->setVisible(_shootLaserSprite->isVisible() && _shootLaserSprite->getFrame() >= 1 && _shootLaserSprite->getFrame() <= 36);

    _groundIdleSprite->setVisible(!isStunned() && _isGroundForm && !_isUppercutting && !_isSlamming && !getIsJumping() && !_isDashing && !_isGroundDashStarting && !_isGroundDashEnding);
	_airIdleSprite->setVisible(!isStunned() && !_isGroundForm && !_isShootStarting && !_isShootAttacking && !_isLaserAttacking && !_isShootWaiting && !_isDashing);

    playAnimation(_groundIdleSprite);
    playAnimation(_airIdleSprite);
    playAnimation(_groundStunSprite);
	playAnimation(_airStunSprite);

    playAnimation(_uppercutSprite);
	playAnimation(_slamSprite);
	playAnimation(_dashSprite);

	playAnimation(_groundTransformSprite);
	playAnimation(_airTransformSprite);

	playAnimation(_jumpStartSprite);
	playAnimation(_jumpWaitSprite);
	playAnimation(_jumpEndSprite);

	playAnimation(_shootStartSprite);
	playAnimation(_shootAttackSprite);
    playAnimation(_shootLaserSprite);
	playAnimation(_shootWaitSprite);

    playVFXAnimation(_shootLaserSprite, _laserVFXSprite, 1);

    _node->setScale(Vec2(isFacingRight() ? 1 : -1, 1));
    _node->getChild(_node->getChildCount() - 2)->setScale(Vec2(isFacingRight() ? 1 : -1, 1));
    _node->getChild(_node->getChildCount() - 1)->setScale(Vec2(isFacingRight() ? 1 : -1, 1));
}

void Boss3Model::die(std::shared_ptr<scene2::SceneNode> world) {
    setVX(0);
    setVY(0);
    setGravityScale(1);

	std::shared_ptr<cugl::scene2::SpriteNode> _actualDeadSprite = _isGroundForm ? _groundDeadSprite : _airDeadSprite;

    for (NodePtr node : _node->getChildren()) {
        if ((_isGroundForm && node->getName() == "groundDead") || (!_isGroundForm && node->getName() == "airDead")) {
            continue;
        }

        node->removeFromParent();
    }

    if (_node->getChildCount() == 0) {
        _node->addChild(_actualDeadSprite);
    }
    _actualDeadSprite->setVisible(true);

    playAnimationOnce(_actualDeadSprite);

    if (_actualDeadSprite->getFrame() == _actualDeadSprite->getCount() - 1) {
        markRemoved(true);
        world->removeChild(_node);
        setSensor(true);
        getDebugNode()->removeFromParent();
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
void Boss3Model::resetDebug() {
    EnemyModel::resetDebug();
}



