//
//  CUSlider.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a slider, which allows the user to drag
//  a knob to select a value.  The slider can be spartan (a circle on a line),
//  or it can have custom images.
//
//  The slider can track its own state, relieving you of having to manually
//  check mouse presses. However, it can only do this when the slider is part
//  of a scene graph, as the scene graph maps mouse coordinates to screen
//  coordinates.
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
//  Authors: Walker White and Enze Zhou
//  Version: 7/3/24 (CUGL 3.0 reorganization)
//
#include <cugl/core/input/cu_input.h>
#include <cugl/scene2/CUSlider.h>
#include <cugl/scene2/CUPathNode.h>
#include <cugl/scene2/CUScene2Loader.h>
#include <cugl/core/assets/CUAssetManager.h>

using namespace cugl;
using namespace cugl::scene2;
using namespace cugl::graphics;

/** String for managing unknown JSON values */
#define UNKNOWN_STR "<unknown>"
/** The line weight of the default path node */
#define LINE_WEIGHT 2.0f
/** The number of line segments in the knob "circle" */
#define KNOB_SEGS   32

#pragma mark -
#pragma mark Constructors
/**
 * Creates an uninitialized slider. You must initialize it before use.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a Node on the
 * heap, use one of the static constructors instead.
 */
Slider::Slider() :
_range(Vec2::ZERO),
_bounds(Rect::ZERO),
_tick(0),
_value(0),
_snap(false),
_active(false),
_mouse(false),
_knob(nullptr),
_path(nullptr),
_inputkey(0),
_nextKey(1),
_knobChild(""),
_pathChild("") {
    _classname  = "Slider";
    _range.x = DEFAULT_MIN;
    _range.y = DEFAULT_MAX;
}

/**
 * Initializes a slider with given bounds.
 *
 * The slider visuals will be interpretted from bounds.  The knob will be
 * a circle whose radius is the maximum of x and y, where (x,y) is the
 * bounds origin.  The path will be a simple line, but it will be surrounded
 * by a transparent "track" which tightly fits the knob.
 *
 * The range is the slider value range.  The x value is the minimum value
 * (corresponding to the bottom left corner of bounds) and the y value is
 * the maximum value (corresponding to the top right corner of bounds).
 * The slider will start at the middle value.
 *
 * @param range     The slider value range
 * @param bounds    The slider path
 *
 * @return true if the slider is initialized properly, false otherwise.
 */
bool Slider::init(const Vec2 range, const Rect bounds) {
    if (SceneNode::initWithBounds(bounds)) {
        return false;
    }

    _range  = range;
    _bounds = bounds;

    setPath(nullptr);
    placeKnob(nullptr);
    
    _value = (_range.y+_range.x)/2.0f;
    reconfigure();
    return true;
}

/**
 * Initializes a slider with given scene graph nodes.
 *
 * The slider visuals will be taken from the scene graph nodes knob and path.
 * The rectangle bounds should define an interior region of path.  The
 * knob graph node can be slid from the origing of bounds to the top right
 * corner.
 *
 * The range is the slider value range.  The x value is the minimum value
 * (corresponding to the bottom left corner of bounds) and the y value is
 * the maximum value (corresponding to the top right corner of bounds).
 * The slider will start at the middle value.
 *
 * @param range     The slider value range
 * @param bounds    The slider path
 * @param path      The scene graph node for the path
 * @param knob      The scene graph node for the knob
 *
 * @return true if the slider is initialized properly, false otherwise.
 */
bool Slider::initWithUI(const Vec2 range, const Rect bounds,
                        const std::shared_ptr<SceneNode>& path, 
                        const std::shared_ptr<Button>& knob) {
    if (SceneNode::initWithBounds(bounds)) {
        return false;
    }
    
    _range  = range;
    _bounds = bounds;

    setPath(path);
    placeKnob(knob);
    
    _value = (_range.y+_range.x)/2.0f;
    reconfigure();
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
 *      "bounds":   A 4-element array of numbers (x,y,width,height)
 *      "range":    A 2-element array of numbers (min,max)
 *      "value':    A number representing the initial value
 *      "tick':     A number greater than 0, representing the tick period
 *      "snap":     A boolean indicating whether to snap to a nearest tick
 *      "knob":     A string referencing the name of a child node
 *      "path":     A string referencing the name of a child node
 *
 * The attribute 'bounds' is REQUIRED.  All other attributes are optional.
 *
 * @param manager   The asset manager handling this asset
 * @param data      The JSON object specifying the node
 *
 * @return true if initialization was successful.
 */
bool Slider::initWithData(const AssetManager* manager,
                          const std::shared_ptr<JsonValue>& data) {
    if (!data) {
        return init();
    } else if (!SceneNode::initWithData(manager,data)) {
        return false;
    }

    if (data->has("bounds")) {
        JsonValue* bound = data->get("bounds").get();
        CUAssertLog(bound->size() == 4, "Attribute 'bounds' must be a four element array");
        _bounds.origin.x = bound->get(0)->asFloat(0.0f);
        _bounds.origin.y = bound->get(1)->asFloat(0.0f);
        _bounds.size.width = bound->get(2)->asFloat(0.0f);
        _bounds.size.height= bound->get(3)->asFloat(0.0f);
    } else {
        CUAssertLog(false, "JSON is missing a required 'bounds' rectangle");
        return false;
    }
    
    
    if (data->has("path")) {
        _pathChild = data->get("path")->asString();
    }

    if (data->has("knob")) {
        _knobChild = data->get("knob")->asString();
    }

    if (data->has("range")) {
        JsonValue* range = data->get("range").get();
        CUAssertLog(range->size() == 2, "Attribute 'range' must be a two element array");
        _range.x = range->get(0)->asFloat(DEFAULT_MIN);
        _range.y = range->get(1)->asFloat(DEFAULT_MAX);
    }
    
    _value = data->getFloat("value",(_range.y+_range.x)/2.0f);
    _tick  = data->getFloat("tick",0);
    _snap  = data->getBool("snap",false);
    
    if (_contentSize == Size::ZERO) {
        _contentSize = _bounds.size;
    }
    
    return true;
}

/**
 * Disposes all of the resources used.
 *
 * A disposed slider can be safely reinitialized. Any child will
 * be released. They will be deleted if no other object owns them.
 *
 * It is unsafe to call this on a slider that is still currently
 * inside of a scene graph.
 */
void Slider::dispose() {
    if (_active) {
		deactivate();
    }
    _value = 0;
    _tick = 0;
    _snap = false;
    _range  = Vec2::ZERO;
    _bounds = Rect::ZERO;
    _active = false;
    _mouse  = false;
    _knob   = nullptr;
    _path   = nullptr;
    _knobChild = "";
    _pathChild = "";
    _listeners.clear();
    _nextKey = 1;
    _inputkey = 0;
    SceneNode::dispose();
}

#pragma mark -
#pragma mark Appearance
/**
 * Sets the scene graph node for the knob.
 *
 * If this value is nullptr, the method will construct a default knob scene
 * graph consisting of a simple circle.
 *
 * Changing the knob may resize the bounding box of the slider. The slider
 * tries to ensure that the knob remains inside of the bounding box no
 * matter its position.
 *
 * @param knob  The new scene graph node for the knob.
 */
void Slider::setKnob(const std::shared_ptr<Button>& knob) {
    placeKnob(knob);
    reconfigure();
}

/**
 * Sets the scene graph node for the knob.
 *
 * If this value is nullptr, the method will construct a default knob
 * scene graph consisting of a simple circle.
 *
 * Unlike {@link setKnob()}, this does not resize the bounding box.
 *
 * @param knob  The new scene graph node for the knob.
 */
void Slider::placeKnob(const std::shared_ptr<Button>& knob) {
    if (_knob) {
        removeChild(_knob);
    }
    if (knob == nullptr) {
        float radius = std::max(_bounds.origin.x,_bounds.origin.y);
        PolyFactory factory;
        Poly2 poly = factory.makeEllipse(Vec2(radius,radius), 
                                         Size(2*radius,2*radius));
        std::shared_ptr<PolygonNode> circ = PolygonNode::allocWithPoly(poly);
        circ->setColor(Color4::GRAY);
        // Just in case
        circ->setContentSize(Size(2*radius,2*radius));
        _knob = Button::alloc(circ);
    } else {
        _knob = knob;
    }
    
    addChild(_knob);
}

/**
 * Sets the scene graph node for the path.
 *
 * If this value is nullptr, the method will construct a default path
 * scene graph consisting of a simple line and a semi-transparent track.
 *
 * Changing the path will not resize the bounding box of the slider. It
 * is up to the user to ensure the path aligns with the bounds.
 *
 * @param path  The new scene graph node for the path.
 */
void Slider::setPath(const std::shared_ptr<SceneNode>& path) {
    // Always need knob on top
    if (_knob) {
        removeChild(_knob);
    }
    if (_path) {
        removeChild(_path);
    }
    if (path == nullptr) {
        Size psize;
        psize.width  = (_bounds.size.width > 0 ? _bounds.size.width  :
                        -_bounds.size.width)+_bounds.origin.x;
        psize.height = (_bounds.size.height > 0 ? _bounds.size.height :
                        -_bounds.size.height)+_bounds.origin.y;
        float radius = std::max(_bounds.origin.x,_bounds.origin.y);
        
        _path = SceneNode::allocWithBounds(psize);
        
        Path2 path;
        path.vertices.push_back(_bounds.origin);
        path.vertices.push_back(_bounds.origin+_bounds.size);
        auto track = PathNode::allocWithPath(path,2*radius,
                                             poly2::Joint::SQUARE,
                                             poly2::EndCap::ROUND);
        track->setColor(Color4(255,255,255,32));
        track->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
        track->setPosition(_bounds.origin);
        _path->addChild(track);
        
        auto line = PathNode::allocWithPath(path,LINE_WEIGHT,
                                            poly2::Joint::SQUARE,
                                            poly2::EndCap::ROUND);
        line->setColor(Color4::BLACK);
        line->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
        line->setPosition(_bounds.origin);
        _path->addChild(line);
    } else {
        _path = path;
    }
    addChild(_path);
    if (_knob) {
        addChild(_knob);
    }
}

/**
 * Sets the sliding bounds
 *
 * This rectangle defines the slideable region inside of the path node. The
 * bottom left corner of the bounds rectangle is the minimum value while
 * the top right is the maximum. While the origin should have positive
 * values either the width or height may be negative.
 *
 * The bounds should be inside of the bounding box of the path node.
 * However, this is not enforced.
 *
 * @param value The new sliding bounds
 */
void Slider::setBounds(const Rect value) {
    _bounds = value;
    reposition();
}

#pragma mark -
#pragma mark Listeners
/**
 * Activates this slider to enable dragging.
 *
 * This method attaches a listener to either the {@link Mouse} or
 * {@link Touchscreen} inputs to monitor when the slider is dragged.
 * The slider will favor the mouse, but will use the touch screen
 * if no mouse input is active. If neither input is active, this method
 * will fail.
 *
 * When active, the slider will change its value on its own, without
 * requiring the user to use {@link setValue(float)}. If there is a
 * {@link Listener} attached, it will call that function upon any
 * state changes.
 *
 * @return true if the slider was successfully activated
 */
bool Slider::activate() {
    if (_active) {
        return false;
    }

    Mouse* mouse = Input::get<Mouse>();
    Touchscreen* touch = Input::get<Touchscreen>();
    CUAssertLog(mouse || touch,  "Neither mouse nor touch input is enabled");
    
    if (mouse) {
        _mouse = true;
        if (!_inputkey) { _inputkey = mouse->acquireKey(); }

        // Add the mouse listeners
        bool down = mouse->addPressListener(_inputkey, [=,this](const MouseEvent& event, Uint8 clicks, bool focus) {
            if (_knob->inContentBounds(event.position)) {
                _dragpos = this->screenToNodeCoords(event.position);
                _knob->setDown(true);
            }
        });
        
        bool up = false;
        if (down) {
            up = mouse->addReleaseListener(_inputkey, [=,this](const MouseEvent& event, Uint8 clicks, bool focus) {
                if (_knob->isDown()) {
                    _knob->setDown(false);
                }
            });
            if (!up) {
                mouse->removePressListener(_inputkey);
            }
        }
        
        bool drag = false;
        if (up && down) {
            drag = mouse->addDragListener(_inputkey, [=,this](const MouseEvent& event, const Vec2 previous, bool focus) {
                if (_knob->isDown()) {
                    dragKnob(event.position);
                }
            });
            if (!drag) {
                mouse->removePressListener(_inputkey);
                mouse->removeReleaseListener(_inputkey);
            }
        }
        _active = up && down && drag;
    } else {
        _mouse = false;
        if (!_inputkey) { _inputkey = touch->acquireKey(); }

        // Add the mouse listeners
        bool down = touch->addBeginListener(_inputkey, [=,this](const TouchEvent& event, bool focus) {
            if (_knob->inContentBounds(event.position)) {
                _dragpos = this->screenToNodeCoords(event.position);
                _knob->setDown(true);
            }
        });
        
        bool up = false;
        if (down) {
            up = touch->addEndListener(_inputkey, [=,this](const TouchEvent& event, bool focus) {
                if (_knob->isDown()) {
                    _knob->setDown(false);
                }
            });
            if (!up) {
                touch->removeBeginListener(_inputkey);
            }
        }
        
        bool drag = false;
        if (up && down) {
            drag = touch->addMotionListener(_inputkey, [=,this](const TouchEvent& event, const Vec2 previous, bool focus) {
                if (_knob->isDown()) {
                    dragKnob(event.position);
                }
            });
            if (!drag) {
                mouse->removePressListener(_inputkey);
                mouse->removeReleaseListener(_inputkey);
            }
        }
        _active = up && down && drag;
        touch->requestFocus(_inputkey);
    }
    
    return _active;
}

/**
 * Deactivates this slider, unable to drag from then on.
 *
 * This method removes its internal listener from either the {@link Mouse}
 * or {@link Touchscreen}.
 *
 * When deactivated, the slider will no longer change value on its own.
 * However, the user can still change manually with the {@link setValue(float)}
 * method. In addition, any {@link Listener} attached will still
 * respond to manual state changes.
 *
 * @return true if the slider was successfully deactivated
 */
bool Slider::deactivate() {
    if (!_active) {
        return false;
    }

    bool success = false;
    if (_mouse) {
        Mouse* mouse = Input::get<Mouse>();
        CUAssertLog(mouse,  "Mouse input is no longer enabled");
        success = mouse->removePressListener(_inputkey);
        success = mouse->removeReleaseListener(_inputkey) && success;
        success = mouse->removeDragListener(_inputkey) && success;
    } else {
        Touchscreen* touch = Input::get<Touchscreen>();
        CUAssertLog(touch,  "Touch input is no longer enabled");
        success = touch->removeBeginListener(_inputkey);
        success = touch->removeEndListener(_inputkey) && success;
        success = touch->removeMotionListener(_inputkey) && success;
    }
    
    _active = false;
    _mouse = false;
    
    return success;
}

/**
 * Returns the listener for the given key
 *
 * This listener is invoked when the slider value changes.
 *
 * If there is no listener for the given key, it returns nullptr.
 *
 * @param key   The identifier for the listener
 *
 * @return the listener for the given key
 */
const Slider::Listener Slider::getListener(Uint32 key) const {
    auto item = _listeners.find(key);
    if (item == _listeners.end()) {
        return nullptr;
    }
    return item->second;
}

/**
 * Returns all listeners for this slider
 *
 * These listeners are invoked when the slider value changes.
 *
 * @return the listeners for this slider
 */
const std::vector<Slider::Listener> Slider::getListeners() const {
    std::vector<Listener> result;
    result.reserve(_listeners.size());
    for(auto kv : _listeners) {
        result.push_back(kv.second);
    }
    return result;
}

/**
 * Returns a key for a listener after adding it to this button.
 *
 * This listener is invoked when the slider value changes.
 *
 * C++ cannot hash functions types.  Therefore, the listener will
 * be identified by a unique key, returned by this function.  You should
 * remember this key to remove the listener if necessary.
 *
 * @param listener  The listener to add
 *
 * @return the key for the listener
 */
Uint32 Slider::addListener(Listener listener) {
    CUAssertLog(_nextKey < (Uint32)-1, "No more available listener slots");
    _listeners[++_nextKey] = listener;
    return _nextKey;
}

/**
 * Removes a listener from this slider.
 *
 * This listener is invoked when the button state changes (up or down).
 *
 * Listeners must be identified by the key returned by the {@link #addListener}
 * method. If this slider does not have a listener for the given key,
 * this method will fail.
 *
 * @param key  The key of the listener to remove
 *
 * @return true if the listener was succesfully removed
 */
bool Slider::removeListener(Uint32 key) {
    if (_listeners.find(key) == _listeners.end()) {
        return false;
    }
    _listeners.erase(key);
    return true;
}

/**
 * Clears all listeners for this slider.
 *
 * These listeners are invoked when the slider value changes. This
 * method does not require you to remember the keys assigned to the
 * individual listeners.
 */
void Slider::clearListeners() {
    _listeners.clear();
}


#pragma mark -
#pragma mark Internal Helpers
/**
 * Returns the correct value nearest the given one.
 *
 * This method is used to snap values to the grid of ticks, as well as
 * keep the value in range.
 *
 * @param value The candidate value
 *
 * @return the nearest correct value.
 */
float Slider::validate(float value) const {
    float result = value;
    if (_snap && _tick > 0) {
        float actual = (result - _range.x)/_tick;
        actual = std::round(actual);
        result = actual*_tick+_range.x;
    }
    return std::max(std::min(result,_range.y),_range.x);
}

/**
 * Resizes the node to fit the knob and path.
 *
 * This will adjust the bounds to match the resizing
 */
void Slider::reconfigure() {
    // Need the knob to fit in the bounding box at all times
    Size ksize = _knob->getSize();
    
    // Compute the left and right padding necessary.
    Vec2 left, rght;
    if (ksize.width/2.0f > _bounds.origin.x) {
        left.x = ksize.width/2.0f - _bounds.origin.x;
    }
    if (ksize.height/2.0f > _bounds.origin.y) {
        left.y = ksize.height/2.0f - _bounds.origin.y;
    }
    if (ksize.width/2.0f > _contentSize.width-_bounds.size.width-_bounds.origin.x) {
        rght.x = ksize.width/2.0f-_contentSize.width+_bounds.size.width+_bounds.origin.x;
    }
    if (ksize.height/2.0f > _contentSize.height-_bounds.size.height-_bounds.origin.y) {
        rght.y = ksize.height/2.0f-_contentSize.height+_bounds.size.height+_bounds.origin.y;
    }
    
    // Resize and reposition
    Size size = _contentSize+left+rght;
    setContentSize(size);

    _bounds.origin += left;
    Vec2 pos= _path->getPosition();
    _path->setPosition(pos+left);
    reposition();
}

/**
 * Repositions the knob to reflect a change in value.
 *
 * This method is called whenever the value or its range changes.
 */
void Slider::reposition() {
    Vec2 pos = _bounds.origin + _bounds.size * ((_value-_range.x) / (_range.y-_range.x));
    _knob->setAnchor(Vec2::ANCHOR_CENTER);
    _knob->setPosition(pos);
    
    for(auto it = _listeners.begin(); it != _listeners.end(); ++it) {
        it->second(getName(),_value);
    }
}

/**
 * Drags the knob to the given position.
 *
 * This method is called by the touch listeners and assumes that an
 * initial drag anchor has been set.  The position defines a drag
 * vector that is projected on to the sliding bounds.
 *
 * @param pos   The position to drag to (if in the sliding bounds)
 */
void Slider::dragKnob(const Vec2 pos) {
    Vec2 off = screenToNodeCoords(pos);
    off -= _bounds.origin;
    
    Vec2 line = (Vec2)_bounds.size;
    Vec2 drag = off-_dragpos;
    Vec2 prog = drag.getProjection(line);
    
    float param  = (line.x != 0 ? prog.x/line.x : prog.y/line.y);
    if (param == 0) { return; }
    
    // Compute how much we can move.
    float result = validate(_value+param*(_range.y-_range.x));

    // Remember the remainder and restore the drag.
    double remain = (result-_value)/(_range.y-_range.x);
    drag *= (float)(remain/param);

    _dragpos += drag;
    _value = result;
    reposition();
}


/**
 * Arranges the child of this node using the layout manager.
 *
 * This process occurs recursively and top-down. A layout manager may end
 * up resizing the children.  That is why the parent must finish its layout
 * before we can apply a layout manager to the children.
 */
void Slider::doLayout() {
    // Revision for 2024: Lazy attachment similar to that of a button
    if (_knob == nullptr) {
        if (_knobChild != "") {
            std::shared_ptr<SceneNode> node = getChildByName(_knobChild);
            if (node != nullptr) {
                _knob = std::dynamic_pointer_cast<Button>(node);
                if (_knob == nullptr) {
                    removeChild(node);
                    _knob = Button::alloc(node);
                    addChild(_knob);
                }
            }
        }
        if (_pathChild != "") {
            _path = getChildByName(_pathChild);
        } else {
            setPath(nullptr);
        }
        
        if (_knob == nullptr) {
            placeKnob(nullptr);
        }
        reconfigure();
    }
    SceneNode::doLayout();
}
