#include "GBMovementActionModel.h"

/**
 * Default constructor.
 * Initializes movement-related values to defaults.
 */
MovementActionModel::MovementActionModel() : _moveDirection(0, 0), _moveDistance(0.0f), _moveSpeed(0.0f), _moveToPlayer(false) {}

/**
 * Virtual destructor.
 */
MovementActionModel::~MovementActionModel() = default;

/**
 * Returns the movement direction.
 */
cugl::Vec2 MovementActionModel::getMoveDirection() const {
    return _moveDirection;
}

/**
 * Sets the movement direction.
 */
void MovementActionModel::setMoveDirection(const cugl::Vec2& direction) {
    _moveDirection = direction;
}

/**
 * Returns the movement distance.
 */
float MovementActionModel::getMoveDistance() const {
    return _moveDistance;
}

/**
 * Sets the movement distance.
 */
void MovementActionModel::setMoveDistance(float distance) {
    _moveDistance = distance;
}

/**
 * Returns the movement speed.
 */
float MovementActionModel::getMoveSpeed() const {
    return _moveSpeed;
}

/**
 * Sets the movement speed.
 */
void MovementActionModel::setMoveSpeed(float speed) {
    _moveSpeed = speed;
}

/**
 * Returns whether the movement is directed toward the player.
 */
bool MovementActionModel::getMoveToPlayer() const {
    return _moveToPlayer;
}

/**
 * Sets whether the movement is directed toward the player.
 */
void MovementActionModel::setMoveToPlayer(bool moveToPlayer) {
    _moveToPlayer = moveToPlayer;
}
