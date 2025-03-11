#include "GBRangedActionModel.h"

/**
 * Default constructor.
 * Initializes all projectile-related values to defaults.
 */
RangedActionModel::RangedActionModel() : _projectileSpeed(0.0f), _projectileDirection(0, 0), _projectileDamage(0.0f) {}

/**
 * Virtual destructor.
 */
RangedActionModel::~RangedActionModel() = default;

/**
 * Returns the projectile speed.
 */
float RangedActionModel::getProjectileSpeed() const {
    return _projectileSpeed;
}

/**
 * Sets the projectile speed.
 */
void RangedActionModel::setProjectileSpeed(float speed) {
    _projectileSpeed = speed;
}

/**
 * Returns the projectile direction.
 */
cugl::Vec2 RangedActionModel::getProjectileDirection() const {
    return _projectileDirection;
}

/**
 * Sets the projectile direction.
 */
void RangedActionModel::setProjectileDirection(const cugl::Vec2& direction) {
    _projectileDirection = direction;
}

/**
 * Returns the projectile texture.
 */
std::shared_ptr<cugl::graphics::Texture> RangedActionModel::getProjectileTexture() const {
    return _projectileTexture;
}

/**
 * Sets the projectile texture.
 */
void RangedActionModel::setProjectileTexture(const std::shared_ptr<cugl::graphics::Texture>& texture) {
    _projectileTexture = texture;
}

/**
 * Returns the projectile damage.
 */
float RangedActionModel::getProjectileDamage() const {
    return _projectileDamage;
}

/**
 * Sets the projectile damage.
 */
void RangedActionModel::setProjectileDamage(float damage) {
    _projectileDamage = damage;
}
