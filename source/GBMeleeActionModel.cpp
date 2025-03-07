#include "GBMeleeActionModel.h"

/**
 * Default constructor.
 * Initializes all hitbox-related values to defaults.
 */
MeleeActionModel::MeleeActionModel() : _hitboxPos(0, 0), _hitboxSize(0, 0), _hitboxStartTime(0.0f), _hitboxEndTime(0.0f), _hitboxDamage(0.0f) {}

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
float MeleeActionModel::getHitboxStartTime() const {
    return _hitboxStartTime;
}

/**
 * Sets the hitbox start time.
 */
void MeleeActionModel::setHitboxStartTime(float startTime) {
    _hitboxStartTime = startTime;
}

/**
 * Returns the hitbox end time.
 */
float MeleeActionModel::getHitboxEndTime() const {
    return _hitboxEndTime;
}

/**
 * Sets the hitbox end time.
 */
void MeleeActionModel::setHitboxEndTime(float endTime) {
    _hitboxEndTime = endTime;
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
