#include "GBRangedActionModel.h"

/**
 * Default constructor.
 * Initializes all projectile-related values to defaults.
 */
RangedActionModel::RangedActionModel() {}

/**
 * Virtual destructor.
 */
RangedActionModel::~RangedActionModel() = default;

const std::vector<std::shared_ptr<Projectile>>& RangedActionModel::getProjectiles() const {
    return _projectiles;
}

void RangedActionModel::setProjectiles(const std::vector<std::shared_ptr<Projectile>>& projectiles) {
    _projectiles = projectiles;
}

const std::vector<Vec2>& RangedActionModel::getProjectileSpawnPositions() const {
    return _projectileSpawnPositions;
}

void RangedActionModel::setProjectileSpawnPositions(const std::vector<Vec2>& positions) {
    _projectileSpawnPositions = positions;
}

const std::vector<int>& RangedActionModel::getProjectileSpawnFrames() const {
    return _projectileSpawnFrames;
}

void RangedActionModel::setProjectileSpawnFrames(const std::vector<int>& frames) {
    _projectileSpawnFrames = frames;
}