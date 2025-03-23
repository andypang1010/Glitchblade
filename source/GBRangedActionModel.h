#ifndef RANGED_ACTION_MODEL_H
#define RANGED_ACTION_MODEL_H

#include <string>
#include <cugl/cugl.h>
#include "GBActionModel.h"
#include "GBProjectile.h"

using namespace cugl;

/**
 * A class representing a ranged action model, inheriting from ActionModel.
 */
class RangedActionModel : public ActionModel {
protected:
	std::vector<std::shared_ptr<Projectile>> _projectiles;
    std::vector<Vec2> _projectileSpawnPositions;
    std::vector<int> _projectileSpawnFrames;

public:
    /** Default constructor. */
    RangedActionModel();

    /** Virtual destructor. */
    virtual ~RangedActionModel();

    /** Getters and Setters **/
    const std::vector<std::shared_ptr<Projectile>>& getProjectiles() const;
    void setProjectiles(const std::vector<std::shared_ptr<Projectile>>& projectiles);
    const std::vector<Vec2>& getProjectileSpawnPositions() const;
    void setProjectileSpawnPositions(const std::vector<Vec2>& positions);
    const std::vector<int>& getProjectileSpawnFrames() const;
    void setProjectileSpawnFrames(const std::vector<int>& frames);
};

#endif // RANGED_ACTION_MODEL_H
