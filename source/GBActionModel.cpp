#include "GBActionModel.h"

/**
 * Default constructor.
 * Initializes the action length to 0.0f.
 */
ActionModel::ActionModel() : _actionLength(0.0f), _actionName("") {}

/**
 * Virtual destructor.
 */
ActionModel::~ActionModel() = default;

/**
 * Returns the action animation.
 *
 * @return The shared pointer to the SpriteSheet.
 */
std::shared_ptr<cugl::graphics::SpriteSheet> ActionModel::getActionAnimation() const {
    return _actionAnimation;
}

/**
 * Sets the action animation.
 *
 * @param animation The shared pointer to the new SpriteSheet.
 */
void ActionModel::setActionAnimation(const std::shared_ptr<cugl::graphics::SpriteSheet>& animation) {
    _actionAnimation = animation;
}

/**
 * Returns the action length.
 *
 * @return The length of the action.
 */
float ActionModel::getActionLength() const {
    return _actionLength;
}

/**
 * Sets the action length.
 *
 * @param length The new action length.
 */
void ActionModel::setActionLength(float length) {
    _actionLength = length;
}

/**
 * Returns the action name.
 *
 * @return The name of the action.
 */
std::string ActionModel::getActionName() const {
    return _actionName;
}

/**
 * Sets the action name.
 *
 * @param name The new action name.
 */
void ActionModel::setActionName(const std::string& name) {
    _actionName = name;
}