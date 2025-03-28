#ifndef MELEE_ACTION_MODEL_H
#define MELEE_ACTION_MODEL_H

#include <string>
#include <cugl/cugl.h>
#include "../GBActionModel.h"

using namespace cugl;

/**
 * An abstract class representing an action model with an animation and duration.
 */
class MeleeActionModel : public ActionModel {
protected:
    /** The position of the hitbox. */
    cugl::Vec2 _hitboxPos;

    /** The size of the hitbox. */
    cugl::Vec2 _hitboxSize;

    /** The start time of the hitbox. */
    float _hitboxStartTime;

    /** The end time of the hitbox. */
    float _hitboxEndTime;

    /** The damage dealt by the hitbox. */
    float _hitboxDamage;

public:
    /** Default constructor. */
    MeleeActionModel();

    /** Virtual destructor. */
    virtual ~MeleeActionModel();

    /** Getters and Setters **/
    cugl::Vec2 getHitboxPos() const;
    void setHitboxPos(const cugl::Vec2& pos);

    cugl::Vec2 getHitboxSize() const;
    void setHitboxSize(const cugl::Vec2& size);

    float getHitboxStartTime() const;
    void setHitboxStartTime(float startTime);

    float getHitboxEndTime() const;
    void setHitboxEndTime(float endTime);

    float getHitboxDamage() const;
    void setHitboxDamage(float damage);
};

#endif // ACTION_MODEL_H
