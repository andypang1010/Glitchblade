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
#include <cugl/core/input/cu_input.h>
#include <cugl/scene2/CUScene2.h>
#include <cugl/scene2/CUButtonGroup.h>
#include <cugl/scene2/layout/CULayout.h>
#include <cugl/scene2/CUScene2Loader.h>
#include <vector>

using namespace cugl;
using namespace cugl::scene2;
using namespace cugl::graphics;

/** To define the size of an empty button */
#define DEFAULT_SIZE 50

#pragma mark -
#pragma mark Constructors
/**
 * Creates an uninitialized button with no size or texture information.
 *
 * You must initialize this button before use.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a Node on the
 * heap, use one of the static constructors instead.
 */
ButtonGroup::ButtonGroup() : SceneNode(),
_mouse(false),
_active(false),
_curButton(-1),
_inputkey(0),
_nextKey(1)
{
  _classname = "ButtonGroup";
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
bool ButtonGroup::init(const std::shared_ptr<Button>& button, int initialButton) {
    // Force toggle or change it if it isn't
    CUAssertLog(button->isToggle(), "Button must be a toggle button");
    CUAssertLog(initialButton < 1 && initialButton >= -1,
                "initialButton must be in range [-1,1)");
    
    if (!SceneNode::init()) {
        return false;
    }

    // Deactivate the button, as we will assume control
    button->deactivate();
    button->setDown(false);
    _buttons.push_back(button);

    Size size = button->getContentSize();
    button->setAnchor(Vec2::ANCHOR_CENTER);
    button->setPosition(size.width / 2.0f, size.height / 2.0f);

    _curButton = initialButton;
    if (_curButton >= 0) {
        _buttons[_curButton]->setDown(true);
    }

    setContentSize(size);
    return true;
}

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
bool ButtonGroup::init(const std::vector<std::shared_ptr<Button>>& buttons,
                       const int initialButton) {
    CUAssertLog(initialButton < (int) buttons.size() && initialButton >= -1,
                "initialButton must be in range [-1,%zu)",buttons.size());
    if (!SceneNode::init()) {
        return false;
    }

    Size size = Size();
    for (auto it = buttons.begin(); it != buttons.end(); ++it) {
        CUAssertLog((*it)->isToggle(), "All buttons must be toggle buttons");
        _buttons.push_back(*it);
        if (*it) {
            (*it)->setAnchor(Vec2::ANCHOR_CENTER);
            (*it)->setVisible(true);
            (*it)->deactivate();
            (*it)->setDown(false);

            Size curSize = (*it)->getSize();
            size.width = curSize.width > size.width ? curSize.width : size.width;
            size.height = curSize.height > size.height ? curSize.height : size.height;
        }
    }

    _curButton = initialButton;
    if (_curButton >= 0) {
        _buttons[_curButton]->setDown(true);
    }

    for (auto it = _buttons.begin(); it != _buttons.end(); ++it) {
        if (*it) {
            (*it)->setPosition(size.width / 2.0f, size.height / 2.0f);
        }
    }

    setContentSize(size);
    return true;
}

/**
 * Initializes a node with the given JSON specificaton.
 *
 * This initializer is designed to receive the "data" object from the
 * JSON passed to {@link SceneLoader}.  This JSON format supports all
 * of the attribute values of its parent class.  In addition, it supports
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
bool ButtonGroup::initWithData(const AssetManager* manager,
                               const std::shared_ptr<JsonValue>& data) {
    if (!data) {
        return init();
    } else if (!SceneNode::initWithData(manager, data)) {
        return false;
    }

    if (getContentSize() == Size::ZERO) {
        setContentSize(Size(DEFAULT_SIZE, DEFAULT_SIZE));
    }

    std::shared_ptr<JsonValue> buttons = data->get("buttons");

    CUAssertLog(buttons->size() >= 1, "The list buttons must have at least one entry");
    for (int i = 0; i < buttons->size(); i++) {
        _keyset.push_back(buttons->get(i)->asString());
    }


    int initial = data->getInt("initial", -1);
    CUAssertLog(initial < (int) _children.size() && initial >= -1,
                "initial must be in range [-1,%zu)",_children.size());
    _curButton = initial;
    return true;
}

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
void ButtonGroup::dispose() {
    if (_active) {
        deactivate();
    }

    for (auto it = _buttons.begin(); it != _buttons.end(); ++it) {
        (*it)->dispose();
    }

    _buttons.clear();
    _listeners.clear();
    _children.clear();
    _curButton = -1;
    _nextKey = 1;
    _inputkey = 0;
    SceneNode::dispose();
}

#pragma mark -
#pragma mark Listeners
/**
 * Activates this button group to listen for mouse/touch events.
 *
 * This method attaches a listener to either the {@link Mouse} or
 * {@link Touchscreen} inputs to monitor when the button is pressed and/or
 * released. The button group will favor the mouse, but will use the touch
 * screen if no mouse input is active.  If neither input is active, this
 * method will fail.
 *
 * When active, the button group will change the state of the appropriate
 * button on its own, without requiring the user to use {@link setDown(bool)}.
 * If there is a {@link Listener} attached, it will call that function upon
 * any state changes.
 *
 * @return true if the button was successfully activated
 */
bool ButtonGroup::activate() {
    if (_active) {
        return false;
    }

    Mouse* mouse = Input::get<Mouse>();
    Touchscreen* touch = Input::get<Touchscreen>();
    CUAssertLog(mouse || touch, "Neither mouse nor touch input is enabled");

    if (mouse) {
        _mouse = true;
        if (!_inputkey) {
            _inputkey = mouse->acquireKey();
        }

        // Add the mouse listeners
        bool down = mouse->addPressListener(_inputkey, [=,this](const MouseEvent& event, Uint8 clicks, bool focus) {
            int btnPressed = this->screenToIndex(event.position);
            if (btnPressed != -1) {
                this->setDown(btnPressed);
            }
        });

        _active = down;
    } else {
        _mouse = false;
        if (!_inputkey) {
            _inputkey = touch->acquireKey();
        }

        // Add the mouse listeners
        bool down = touch->addBeginListener(_inputkey, [=,this](const TouchEvent& event, bool focus) {
            int btnPressed = this->screenToIndex(event.position);
            if (btnPressed != -1) {
                this->setDown(btnPressed);
            }
        });

        _active = down;
    }

    return _active;
}

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
 * {@link setDown(bool)} method.  In addition, any {@link Listener}
 * attached will still respond to manual state changes.
 *
 * @return true if the button was successfully deactivated
 */
bool ButtonGroup::deactivate() {
    if (!_active) {
        return false;
    }

    bool success = false;
    if (_mouse) {
        Mouse* mouse = Input::get<Mouse>();
        CUAssertLog(mouse, "Mouse input is no longer enabled");
        success = mouse->removePressListener(_inputkey);
    } else {
        Touchscreen* touch = Input::get<Touchscreen>();
        CUAssertLog(touch, "Touch input is no longer enabled");
        success = touch->removeBeginListener(_inputkey);
    }

    _active = false;
    _mouse = false;

    return success;
}

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
const ButtonGroup::Listener ButtonGroup::getListener(Uint32 key) const {
    auto item = _listeners.find(key);
    if (item == _listeners.end()) {
        return nullptr;
    }
    return item->second;
}

/**
 * Returns all listeners for this button group
 *
 * This listener is invoked when the toggled button changes.
 *
 * @return the listener (if any) for this button
 */
const std::vector<ButtonGroup::Listener> ButtonGroup::getListeners() const {
    std::vector<Listener> result;
    result.reserve(_listeners.size());
    for (auto kv : _listeners) {
        result.push_back(kv.second);
    }
    return result;
}

/**
 * Returns a key for a listener after adding it to this button group.
 *
 * This listener is invoked when the toggled button changes.
 *
 * C++ cannot hash functions types. Therefore, the listener will be
 * identified by a unique key, returned by this function. You should
 * remember this key to remove the listener if necessary.
 *
 * This also means that adding a listener twice will add it for an additional
 * key, causing the listener to be called multiple times on a state change.
 *
 * @param listener  The listener to add
 *
 * @return the key for the listener
 */
Uint32 ButtonGroup::addListener(Listener listener) {
    CUAssertLog(_nextKey < (Uint32)-1, "No more available listener slots");
    _listeners[++_nextKey] = listener;
    return _nextKey;
}

/**
 * Removes a listener from this button group.
 *
 * This listener is invoked when the toggled button changes.
 *
 * Listeners must be identified by the key returned by the {@link #addListener}
 * method. If this button does not have a listener for the given key,
 * this method will fail.
 *
 * @param key  The key of the listener to remove
 *
 * @return true if the listener was succesfully removed
 */
bool ButtonGroup::removeListener(Uint32 key) {
    if (_listeners.find(key) == _listeners.end()) {
        return false;
    }
    _listeners.erase(key);
    return true;
}

/**
 * Clears all listeners for this button group.
 *
 * These listeners are invoked when the button state changes (up or down).
 * This method does not require you to remember the keys assigned to the
 * individual listeners.
 */
void ButtonGroup::clearListeners() {
    _listeners.clear();
}

#pragma mark -
#pragma mark ButtonGroup State
/**
 * Add a button to the button group
 *
 * @param btn   The button to add to the group
 */
void ButtonGroup::addButton(const std::shared_ptr<Button>& btn) {
    _buttons.push_back(btn);
}

/**
 * Remove a button from the button group
 *
 * @param btn   The button to remove from the group
 */
void ButtonGroup::removeButton(const std::shared_ptr<Button>& btn) {
    for(auto it = _buttons.begin(); it != _buttons.end(); ++it) {
        if ((*it).get() == btn.get()) {
            _buttons.erase(it);
            it = _buttons.end();
        }
    }
}

/**
 * Calls setDown on the button at index btn
 *
 * Calls setDown on the button at index btn with the appropriate 
 * parameter.
 *
 * @param btn  The index of the button to be set down
 */
void ButtonGroup::setDown(int btn) {
    if (btn == -1 || btn >= _buttons.size()) {
        return;
    }

    for (auto it = _listeners.begin(); it != _listeners.end(); ++it) {
        it->second(getName(), _curButton, btn);
    }

    if (btn == _curButton) {
        _buttons[btn]->setDown(!_buttons[btn]->isDown());
        _curButton = -1;
    } else {
        _buttons[btn]->setDown(true);
        if (_curButton >= 0) {
            _buttons[_curButton]->setDown(false);
        }
        _curButton = btn;
    }
}

/**
 * Returns the index of the button that is clicked
 *
 * This method is used to manually check for mouse presses/touches.  It
 * converts a point in screen coordinates to the node coordinates and
 * checks if it is in the bounds of the button.
 * If the click is not within a button in the group this function will return -1.
 *
 * @param point The point in screen coordinates
 *
 * @return index of button clicked, or -1 if no button is clicked
 */
int ButtonGroup::screenToIndex(const Vec2 point) {
    Vec2 local = screenToNodeCoords(point);
    if (Rect(Vec2::ZERO, getContentSize()).contains(local)) {
        for (auto it = _buttons.begin(); it != _buttons.end(); ++it) {
            if ((*it)->inContentBounds(point)) {
                return (int)distance(_buttons.begin(), it);
            }
        }
    }
    return -1;
}

#pragma mark -
#pragma mark ButtonGroup Layout
/**
 * Arranges the child of this node using the layout manager.
 *
 * This process occurs recursively and top-down. A layout manager may end
 * up resizing the children.  That is why the parent must finish its layout
 * before we can apply a layout manager to the children.
 */
void ButtonGroup::doLayout() {
    // Revision for 2019: Lazy attachment of up and down nodes.
    if (_buttons.size() == 0) {
        // All of the code that follows can corrupt the position.
        Vec2 coord = getPosition();
        Size osize = getContentSize();
        Size size = osize;

        for (auto it = _keyset.begin(); it != _keyset.end(); ++it) {
            if (*it != "") {
                std::shared_ptr<SceneNode> btn = getChildByName(*it);
                if (btn == nullptr) {
                    btn = PolygonNode::allocWithTexture(Texture::getBlank());
                    Size curr = btn->getContentSize();
                    if (_json == nullptr || !_json->has("size")) {
                        size.width = DEFAULT_SIZE > size.width ? DEFAULT_SIZE : size.width;
                        size.height = DEFAULT_SIZE > size.height ? DEFAULT_SIZE : size.height;
                    }
                    btn->setScale(size.width / curr.width, size.height / curr.height);
                } else {
                    Size curSize = btn->getSize();
                    if (_json == nullptr || !_json->has("size")) {
                        size.width = curSize.width > size.width ? curSize.width : size.width;
                        size.height = curSize.height > size.height ? curSize.height : size.height;
                    }
                }
                _buttons.push_back(std::dynamic_pointer_cast<Button>(btn));
                _buttons.back()->deactivate();
                _buttons.back()->setDown(false);
            }
        }

        osize = size;
        setContentSize(size);

        // Now position them
        for (auto it = _buttons.begin(); it != _buttons.end(); ++it) {
            if (*it != nullptr) {
                bool anchor = false;
                bool position = false;
                if (_json->has("children") && _json->get("children")->has((*it)->getName())) {
                    std::shared_ptr<JsonValue> data = _json->get("children")->get((*it)->getName());
                    if (data->has("data")) {
                        data = data->get("data");
                        anchor = data->has("anchor");
                        position = data->has("position");
                    }
                }
                if (!anchor) {
                    (*it)->setAnchor(Vec2::ANCHOR_CENTER);
                }
                if (!position) {
                    (*it)->setPosition(size.width / 2.0f, size.height / 2.0f);
                }
                (*it)->setVisible(true);
            }
        }

        if (_curButton > -1) {
            _buttons[_curButton]->setDown(true);
        }

        // Now redo the position
        setPosition(coord);
    }
    SceneNode::doLayout();
}
