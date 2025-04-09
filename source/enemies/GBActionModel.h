#ifndef ACTION_MODEL_H
#define ACTION_MODEL_H

#include <string>
#include <cugl/cugl.h>

using namespace cugl;

/**
 * An abstract class representing an action model with an animation and duration.
 */
class ActionModel {
protected:
    /** The name of the action */
    std::string _actionName;

public:
    /** Default constructor. */
    ActionModel();

    /** Virtual destructor. */
    virtual ~ActionModel();;

    /**
     * Returns the action name.
     *
     * @return The name of the action.
     */
    std::string getActionName() const;

    /**
     * Sets the action name.
     *
     * @param name The new action name.
     */
    void setActionName(const std::string& name);
};

#endif // ACTION_MODEL_H
