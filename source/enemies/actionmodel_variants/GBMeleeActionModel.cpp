#include "GBMeleeActionModel.h"

/**
 * Default constructor.
 * Initializes all hitbox-related values to defaults.
 */
MeleeActionModel::MeleeActionModel() : _hitboxPos(0, 0), _hitboxSize(0, 0), _hitboxStartFrame(0.0f), _hitboxEndFrame(0.0f), _hitboxDamage(0.0f), _isParriable(true) {}

/**
 * Virtual destructor.
 */
MeleeActionModel::~MeleeActionModel() = default;

/**
 * Returns the hitbox position.
 */
cugl::Vec2 MeleeActionModel::getHitboxPos() const {
    return _hitboxPos;
}

/**
 * Sets the hitbox position.
 */
void MeleeActionModel::setHitboxPos(const cugl::Vec2& pos) {
    _hitboxPos = pos;
}

/**
 * Returns the hitbox size.
 */
cugl::Vec2 MeleeActionModel::getHitboxSize() const {
    return _hitboxSize;
}

/**
 * Sets the hitbox size.
 */
void MeleeActionModel::setHitboxSize(const cugl::Vec2& size) {
    _hitboxSize = size;
}

/**
 * Returns the hitbox start time.
 */
int MeleeActionModel::getHitboxStartFrame() const {
    return _hitboxStartFrame;
}

/**
 * Sets the hitbox start time.
 */
void MeleeActionModel::setHitboxStartFrame(int StartFrame) {
    _hitboxStartFrame = StartFrame;
}

/**
 * Returns the hitbox end time.
 */
int MeleeActionModel::getHitboxEndFrame() const {
    return _hitboxEndFrame;
}

/**
 * Sets the hitbox end time.
 */
void MeleeActionModel::setHitboxEndFrame(int EndFrame) {
    _hitboxEndFrame = EndFrame;
}

/**
 * Returns the hitbox damage.
 */
float MeleeActionModel::getHitboxDamage() const {
    return _hitboxDamage;
}

/**
 * Sets the hitbox damage.
 */
void MeleeActionModel::setHitboxDamage(float damage) {
    _hitboxDamage = damage;
}

/**
 * Returns the hitbox knockback.
 */ 
float MeleeActionModel::getHitboxKnockBack() const {
	return _hitboxKnockBack;
}

/**
 * Sets the hitbox knockback.
 */
void MeleeActionModel::setHitboxKnockBack(float knockback) {
	_hitboxKnockBack = knockback;
}

/**
 * Returns whether the hitbox can be parried.
 */
bool MeleeActionModel::getIsParriable() const {
	return _isParriable;
}

/**
 * Sets whether the hitbox can be parried.
 */
void MeleeActionModel::setIsParriable(bool parriable) {
	_isParriable = parriable;
}
