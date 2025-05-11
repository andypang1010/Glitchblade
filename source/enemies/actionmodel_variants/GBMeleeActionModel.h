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
    int _hitboxStartFrame;

    /** The end time of the hitbox. */
    int _hitboxEndFrame;

    /** The damage dealt by the hitbox. */
    float _hitboxDamage;

    /** Knockback force of the hitbox. */
    float _hitboxKnockBack;

    /** Whether the attack can be parried */
    bool _isParriable;

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

    int getHitboxStartFrame() const;
    void setHitboxStartFrame(int StartFrame);

    int getHitboxEndFrame() const;
    void setHitboxEndFrame(int EndFrame);

    float getHitboxDamage() const;
    void setHitboxDamage(float damage);

	float getHitboxKnockBack() const;
	void setHitboxKnockBack(float knockback);

	bool getIsParriable() const;
    void setIsParriable(bool parriable);
};

#endif // ACTION_MODEL_H
