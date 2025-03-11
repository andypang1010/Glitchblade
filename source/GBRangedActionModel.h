#ifndef RANGED_ACTION_MODEL_H
#define RANGED_ACTION_MODEL_H

#include <string>
#include <cugl/cugl.h>
#include "GBActionModel.h"

using namespace cugl;

/**
 * A class representing a ranged action model, inheriting from ActionModel.
 */
class RangedActionModel : public ActionModel {
protected:
    /** The speed of the projectile. */
    float _projectileSpeed;

    /** The direction of the projectile. */
    cugl::Vec2 _projectileDirection;

    /** The texture of the projectile. */
    std::shared_ptr<cugl::graphics::Texture> _projectileTexture;

    /** The damage dealt by the projectile. */
    float _projectileDamage;

public:
    /** Default constructor. */
    RangedActionModel();

    /** Virtual destructor. */
    virtual ~RangedActionModel();

    /** Getters and Setters **/
    float getProjectileSpeed() const;
    void setProjectileSpeed(float speed);

    cugl::Vec2 getProjectileDirection() const;
    void setProjectileDirection(const cugl::Vec2& direction);

    std::shared_ptr<cugl::graphics::Texture> getProjectileTexture() const;
    void setProjectileTexture(const std::shared_ptr<cugl::graphics::Texture>& texture);

    float getProjectileDamage() const;
    void setProjectileDamage(float damage);
};

#endif // RANGED_ACTION_MODEL_H