#include "GBActionModel.h"

/**
 * Default constructor.
 * Initializes the action length to 0.0f.
 */
ActionModel::ActionModel() : _actionName("") {}

/**
 * Virtual destructor.
 */
ActionModel::~ActionModel() = default;

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