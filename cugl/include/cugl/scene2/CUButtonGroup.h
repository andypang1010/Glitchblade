//
//  CUMultimodalButton.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a simple button group. It is made of
//  a list of toggle buttons, of which at most one can be selected at once.
//  The button group has its own listener, and the individual buttons can
//  have their own as will if needed.
//
//  This class uses our standard shared-pointer architecture.
//
//  1. The constructor does not perform any initialization; it just sets all
//     attributes to their defaults.
//
//  2. All initialization takes place via init methods, which can fail if an
//     object is initialized more than once.
//
//  3. All allocation takes place via static constructors which return a shared
//     pointer.
//
//  CUGL MIT License:
//      This software is provided 'as-is', without any express or implied
//      warranty.  In no event will the authors be held liable for any damages
//      arising from the use of this software.
//
//      Permission is granted to anyone to use this software for any purpose,
//      including commercial applications, and to alter it and redistribute it
//      freely, subject to the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not
//      be misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
//
//  Author: Evan Hatton
//  Version: 7/3/24 (CUGL 3.0 reorganization)
//
#ifndef __CU_BUTTON_GROUP_H__
#define __CU_BUTTON_GROUP_H__
#include <cugl/core/assets/CUJsonValue.h>
#include <cugl/core/math/CUColor4.h>
#include <cugl/core/math/CUPath2.h>
#include <cugl/scene2/CUButton.h>
#include <cugl/scene2/CUSceneNode2.h>
#include <cugl/scene2/CUPolygonNode.h>
#include <unordered_map>
#include <vector>

namespace cugl {

    /**
     * The classes to construct a 2-d scene graph.
     *
     * Even though this is an optional package, this is one of the core features
     * of CUGL. These classes provide basic UI support (including limited Figma)
     * support. Any 2-d game will make extensive use of these classes. And
     * even 3-d games may use these classes for the HUD overlay.
     */
    namespace scene2 {

#pragma mark -
#pragma mark ButtonGroup
/**
 * This class represents a button group
 *
 * A button group is a bunch of toggle buttons, of which at most one can
 * be selected at a time. If a single button is clicked that button's
 * listener will be called if it has one, as well as the button group's
 * listener. The previously toggled button will be untoggled and its
 * listener will also be called.
 *
 * Button groups can be used to make a multiple choice quiz scenes and tabs
 */
class ButtonGroup : public SceneNode {
public:
#pragma mark Listener
    /**
     * @typedef Listener
     *
     * This type represents a listener for a {@link ButtonGroup} state change.
     *
     * In CUGL, listeners are implemented as a set of callback functions, not as
     * objects. This allows each listener to implement as much or as little
     * functionality as it wants. For simplicity, ButtonGroup nodes only support
     * a single listener.  If you wish for more than one listener, then your
     * listener should handle its own dispatch.
     *
     * The function type is equivalent to
     *
     *      std::function<void(const std::string name, bool down)>
     *
     * @param name      The button name
     * @param curBtn    The currently toggled button in the group
     * @param nextBtn   The button to be toggled once the listeners have finished
     */
    typedef std::function<void(const std::string name, int curBtn, int nextBtn)> Listener;

protected:
    /**
     * The index of the current button in _buttons list.
     *
     * This value tarts as -1 (none selected)
     */
    int _curButton;
    
    /**
     * The list of buttons representing the buttons states.
     *
     * This cannot be null and must have length > 0.
     */
    std::vector<std::shared_ptr<Button>> _buttons;
    /** Keyset to access the children (may be empty) */
    std::vector<std::string> _keyset;

    /** Whether the button is actively checking for state changes */
    bool _active;
    /** Whether we are using the mouse (as opposed to the touch screen) */
    bool _mouse;
    /** The listener key when the button is checking for state changes */
    Uint32 _inputkey;
    /** The next available key for a listener */
    Uint32 _nextKey;
    /** The listener callbacks for state changes */
    std::unordered_map<Uint32, Listener> _listeners;

public:
#pragma mark Constructors
    /**
     * Creates an uninitialized button with no size or texture information.
     *
     * You must initialize this button before use.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a Node on the
     * heap, use one of the static constructors instead.
     */
    ButtonGroup();

    /**
     * Deletes this button group, disposing all resources
     */
    ~ButtonGroup() { dispose(); }

    /**
     * Disposes all of the resources used by this node.
     *
     * A disposed button can be safely reinitialized. Any children owned by
     * this node will be released.  They will be deleted if no other object
     * owns them.
     *
     * It is unsafe to call this on a button that is still currently inside
     * of a scene graph.
     */
    virtual void dispose() override;

    /**
     * Deactivates the default initializer.
     *
     * This initializer may not be used for a button group.  A button group must have a
     * child node for the up state at the very minimum.
     *
     * @return false
     */
    virtual bool init() override {
        CUAssertLog(false, "This node does not support the empty initializer");
        return false;
    }

    /**
     * Initializes a button group with the given button and initial index
     *
     * The new button group will have a single button, the one given, and
     * will initially be set down if initialButton is 0. If initialButton is
     * -1, no button is set down.
     *
     * @param button        The button in the button group
     * @param initialButton The index of the button to set down initially
     *
     * @return true if the button group is initialized properly, false otherwise.
     */
    bool init(const std::shared_ptr<Button>& button, int initialButton = -1);

    /**
     * Initializes a button group with the given buttons and initial index
     *
     * The new button group will have all of the buttons in the given list.
     * If initialButton is a positive number the button at that index in the list
     * will be set down initially.  If initialButton is -1, no button is set down.
     *
     * @param buttons        The buttons in the button group
     * @param initialButton The index of the button to set down initially
     *
     * @return true if the button group is initialized properly, false otherwise.
     */
    bool init(const std::vector<std::shared_ptr<Button>>& buttons, int initialButton = -1);

    /**
     * Initializes a node with the given JSON specificaton.
     *
     * This initializer is designed to receive the "data" object from the
     * JSON passed to {@link Scene2Loader}. This JSON format supports all
     * of the attribute values of its parent class. In addition, it supports
     * the following additional attributes:
     *
     *      "buttons":    A list of strings referencing the name of children nodes (must be buttons)
     *      "initial":    A number representing the index of the button to initially be set down
     *
     * The attribute 'buttons' is REQUIRED.  All other attributes are optional.
     *
     * @param manager   The asset manager handling this asset
     * @param data      The JSON object specifying the node
     *
     * @return true if initialization was successful.
     */
    bool initWithData(const AssetManager* manager,
                      const std::shared_ptr<JsonValue>& data) override;

#pragma mark Static Constructors
    /**
     * Returns a newly allocated button group with the given button and index
     *
     * The new button group will have a single button, the one given, and
     * will initially be set down if initialButton is 0. If initialButton is
     * -1, no button is set down.
     *
     * @param button        The button in the button group
     * @param initialButton The index of the button to set down initially
     *
     * @return a newly allocated button group with the given button and index
     */
    static std::shared_ptr<ButtonGroup> alloc(const std::shared_ptr<Button>& button,
                                              int initialButton = -1) {
        std::shared_ptr<ButtonGroup> node = std::make_shared<ButtonGroup>();
        return (node->init(button,initialButton) ? node : nullptr);
    }

    /**
     * Returns a newly allocated button group with the given buttons and index
     *
     * The new button group will have all of the buttons in the given list.
     * If initialButton is a positive number the button at that index in the list
     * will be set down initially.  If initialButton is -1, no button is set down.
     *
     * @param buttons        The buttons in the button group
     * @param initialButton The index of the button to set down initially
     *
     * @return a newly allocated button group with the given buttons and index
     */
    static std::shared_ptr<ButtonGroup> alloc(const std::vector<std::shared_ptr<Button>>& buttons,
                                              int initialButton = -1) {
        std::shared_ptr<ButtonGroup> node = std::make_shared<ButtonGroup>();
        return (node->init(buttons) ? node : nullptr);
    }

    /**
     * Returns a newly allocated a node with the given JSON specificaton.
     *
     * This initializer is designed to receive the "data" object from the
     * JSON passed to {@link Scene2Loader}. This JSON format supports all
     * of the attribute values of its parent class. In addition, it supports
     * the following additional attributes:
     *
     *      "buttons":    A list of strings referencing the name of children nodes (must be buttons)
     *      "initial":    A number representing the index of the button to initially be set down
     *
     * The attribute 'buttons' is REQUIRED.  All other attributes are optional.
     *
     * @param manager   The asset manager handling this asset
     * @param data      The JSON object specifying the node
     *
     * @return a newly allocated a node with the given JSON specificaton.
     */
    static std::shared_ptr<SceneNode> allocWithData(const AssetManager* manager,
                                                    const std::shared_ptr<JsonValue>& data) {
        std::shared_ptr<ButtonGroup> result = std::make_shared<ButtonGroup>();
        if (!result->initWithData(manager, data)) {
          result = nullptr;
        }
        return std::dynamic_pointer_cast<SceneNode>(result);
    }

#pragma mark ButtonGroup State
    /**
     * Returns the index of the currently toggled button.
     *
     * This method returns -1 if no button is toggled.
     *
     * @return the index of the currently toggled button.
     */
    const int getCurrent() const { return _curButton; }

    /**
     * Add a button to the button group
     *
     * @param btn   The button to add to the group
     */
    void addButton(const std::shared_ptr<Button>& btn);

    /**
     * Remove a button from the button group
     *
     * @param btn   The button to remove from the group
     */
    void removeButton(const std::shared_ptr<Button>& btn);

    /**
     * Returns the index of the button that containing the point in screen space
     *
     * This method is used to manually check for mouse presses/touches. It
     * converts a point in screen coordinates to the node coordinates and
     * checks if it is in the bounds of the button.
     *
     * If the click is not within a button in the group this function will
     * return -1.
     *
     * @param point The point in screen coordinates
     *
     * @return the index of the button that containing the point in screen space
     */
    int screenToIndex(const Vec2 point);

    /**
     * Returns the index of the button that containing the point in screen space
     *
     * This method is used to manually check for mouse presses/touches. It
     * converts a point in screen coordinates to the node coordinates and
     * checks if it is in the bounds of the button.
     *
     * If the click is not within a button in the group this function will
     * return -1.
     *
     * @param x The x-value in screen coordinates
     * @param y The y-value in screen coordinates
     *
     * @return index of button clicked, or -1 if no button is clicked
     */
    int screenToIndex(float x, float y) {
        return screenToIndex(Vec2(x, y));
    }

    /**
     * Invokes setDown on the button at index btn
     *
     * This method invokes setDown on the button at index btn with the
     * appropriate parameter. If btn is -1, no button is down.
     *
     * @param btn  The index of the button to be set down
     */
    void setDown(int btn);

#pragma mark -
#pragma mark ButtonGroup Layout
    /**
     * Arranges the child of this node using the layout manager.
     *
     * This process occurs recursively and top-down. A layout manager may end
     * up resizing the children.  That is why the parent must finish its layout
     * before we can apply a layout manager to the children.
     */
    virtual void doLayout() override;

#pragma mark Listeners
    /**
     * Returns true if this button group has a listener
     *
     * This listener is invoked when the toggled button changes.
     *
     * @return true if this button has a listener
     */
    bool hasListener() const { return !_listeners.empty(); }

    /**
     * Returns the listener for the given key
     *
     * This listener is invoked when the toggled button changes.
     *
     * If there is no listener for the given key, it returns nullptr.
     *
     * @param key   The identifier for the listener
     *
     * @return the listener for the given key
     */
    const Listener getListener(Uint32 key) const;

    /**
     * Returns all listeners for this button group
     *
     * This listener is invoked when the toggled button changes.
     *
     * @return the listener (if any) for this button
     */
    const std::vector<Listener> getListeners() const;

    /**
     * Returns a key for a listener after adding it to this button group.
     *
     * This listener is invoked when the button group state changes (up or down).
     *
     * C++ cannot hash functions types. Therefore, the listener will be
     * identified by a unique key, returned by this function.  You should
     * remember this key to remove the listener if necessary.
     *
     * This also means that adding a listener twice will add it for an additional
     * key, causing the listener to be called multiple times on a state change.
     *
     * @param listener  The listener to add
     *
     * @return the key for the listener
     */
    Uint32 addListener(Listener listener);

    /**
     * Removes a listener from this button group.
     *
     * This listener is invoked when the button group state changes (up or down).
     *
     * Listeners must be identified by the key returned by the {@link #addListener}
     * method. If this button does not have a listener for the given key,
     * this method will fail.
     *
     * @param key  The key of the listener to remove
     *
     * @return true if the listener was succesfully removed
     */
    bool removeListener(Uint32 key);

    /**
     * Clears all listeners for this button group.
     *
     * These listeners are invoked when the button state changes (up or down).
     * This method does not require you to remember the keys assigned to the
     * individual listeners.
     */
    void clearListeners();

    /**
     * Activates this button group to listen for mouse/touch events.
     *
     * This method attaches a listener to either the {@link Mouse} or
     * {@link Touchscreen} inputs to monitor when the button is pressed and/or
     * released. The button group will favor the mouse, but will use the touch
     * screen if no mouse input is active. If neither input is active, this
     * method will fail.
     *
     * When active, the button group will change the state of the appropriate
     * button on its own, without requiring the user to use {@link setDown(int)}.
     * If there is a {@link Listener} attached, it will call that function upon
     * any state changes.
     *
     * @return true if the button was successfully activated
     */
    bool activate();

    /**
     * Deactivates this button group, ignoring future mouse/touch events.
     *
     * This method removes its internal listener from either the {@link Mouse}
     * or {@link Touchscreen} inputs to monitor when the button is pressed and/or
     * released.  The input affected is the one that received the listener
     * upon activation.
     *
     * When deactivated, the buttons will no longer change their state on their own.
     * However, the user can still change the state manually with the
     * {@link setDown(int)} method.  In addition, any {@link Listener}
     * attached will still respond to manual state changes.
     *
     * @return true if the button was successfully deactivated
     */
    bool deactivate();

    /**
     * Returns true if this button group has been activated.
     *
     * @return true if this button group has been activated.
     */
    bool isActive() const { return _active; }
    
};
    }
}

#endif /* __CU_BUTTON_GROUP_H__ */
