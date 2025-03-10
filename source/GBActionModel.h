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

    /** The animation for the action. */
    std::shared_ptr<cugl::graphics::SpriteSheet> _actionSprite;

    /** The length/duration of the action. */
    float _actionLength;

public:
    /** Default constructor. */
    ActionModel();

    /** Virtual destructor. */
    virtual ~ActionModel();;

    /**
     * Returns the action animation.
     *
     * @return The shared pointer to the SpriteSheet.
     */
    std::shared_ptr<cugl::graphics::SpriteSheet> getActionAnimation() const;

    /**
     * Sets the action animation.
     *
     * @param animation The shared pointer to the new SpriteSheet.
     */
    void setActionAnimation(const std::shared_ptr<cugl::graphics::SpriteSheet>& animation);

    /**
     * Returns the action length.
     *
     * @return The length of the action.
     */
    float getActionLength() const;

    /**
     * Sets the action length.
     *
     * @param length The new action length.
     */
    void setActionLength(float length);

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
