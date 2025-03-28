#ifndef MOVEMENT_ACTION_MODEL_H
#define MOVEMENT_ACTION_MODEL_H

#include <string>
#include <cugl/cugl.h>
#include "../GBActionModel.h"

using namespace cugl;

/**
 * A class representing a movement action model, inheriting from ActionModel.
 */
class MovementActionModel : public ActionModel {
protected:
    /** The direction of movement. */
    cugl::Vec2 _moveDirection;

    /** The distance to move. */
    float _moveDistance;

    /** The speed to move. */
    float _moveSpeed;

    /** Whether to move toward the player. */
    bool _moveToPlayer;

public:
    /** Default constructor. */
    MovementActionModel();

    /** Virtual destructor. */
    virtual ~MovementActionModel();

    /** Getters and Setters **/
    cugl::Vec2 getMoveDirection() const;
    void setMoveDirection(const cugl::Vec2& direction);

    float getMoveDistance() const;
    void setMoveDistance(float distance);

    float getMoveSpeed() const;
    void setMoveSpeed(float speed);

    bool getMoveToPlayer() const;
    void setMoveToPlayer(bool moveToPlayer);
};

#endif // MOVEMENT_ACTION_MODEL_H