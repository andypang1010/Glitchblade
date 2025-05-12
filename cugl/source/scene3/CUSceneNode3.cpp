//
//  CUSceneNode.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a 3d scene graph node. It is the base
//  for rendering 3d scenes. It essentially stores the information for
//  constructing the modelview matrix for any subclass.
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
//  Author: Walker White
//  Version: 7/3/24 (CUGL 3.0 reorganization)
//
#include <cugl/scene3/CUScene3.h>
#include <cugl/scene3/CUSceneNode3.h>
#include <cugl/scene3/CUScene3Pipeline.h>
#include <cugl/core/util/CUStringTools.h>
#include <cugl/core/math/CUCamera.h>
#include <sstream>
#include <algorithm>

using namespace cugl;
using namespace cugl::scene3;

/** A key of an unused batch */
#define UNUSED_KEY   0

#pragma mark Constructors
/**
 * Creates an uninitialized node.
 *
 * You must initialize this Node before use.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a Node on the
 * heap, use one of the static constructors instead.
 */
SceneNode::SceneNode() :
_tag(0),
_name(""),
_hashOfName(0),
_isVisible(true),
_scale(Vec3::ONE),
_parent(nullptr),
_graph(nullptr),
_childOffset(-2),
_batchkey(UNUSED_KEY) {
    _modelmat = Mat4::IDENTITY;
    _classname = "SceneNode";
}

/**
 * Initializes a node at the given position.
 *
 * The node has size (0,0,0). As a result, the position is identified with
 * the origin of the node space.
 *
 * @param pos   The origin of the node in parent space
 *
 * @return true if initialization was successful.
 */
bool SceneNode::initWithPosition(const Vec3 pos) {
    CUAssertLog(_childOffset == -2, "Attempting to reinitialize a Node");
    _position = pos;
     updateMatrices();
    _childOffset = -1;
    return true;
}

/**
 * Initializes a node with the given anchor and position.
 *
 * The anchor defines the transformation origin of the scene node. All
 * scaling and rotation happens about the anchor, not the origin. This
 * anchor is placed at position pos in parent space.
 *
 * @param pos       The node position
 * @param anchor    The node anchor
 *
 * @return true if initialization was successful.
 */
bool SceneNode::initWithAnchor(const Vec3 pos, const Vec3 anchor) {
    CUAssertLog(_childOffset == -2, "Attempting to reinitialize a Node");
    _anchor = anchor;
    _position = pos;
    updateMatrices();
    _childOffset = -1;
    return true;
}

/**
 * Initializes a node with the given JSON specificaton.
 *
 * This initializer is designed to receive the "data" object from the
 * JSON passed to {@link Scene3Loader}. This JSON format supports the
 * following attribute values:
 *
 *      "position": A three-element number array
 *      "anchor":   A three-element number array representing the anchor point
 *      "scale":    Either a three-element number array or a single number
 *      "rotation": A JSON value representing rotation about an axis
 *      "visible":  A boolean value, representing if the node is visible
 *
 * All attributes are optional. There are no required attributes. Everything
 * that is missing is assigned its default value.
 *
 * The value "rotation" can either be a four-element array or a JSON object. If
 * it is an array, it is interpreted as a quaternion. If it is an object, it
 * supports the following attributes:
 *
 *      "angle":    A number, representing the rotation in DEGREES, not radians
 *      "axis":     A three-element number array
 *
 * Once again, both are optional, though no rotation is applied if the angle is
 * missing.  If the axis is missing, rotation is about the z-axis
 *
 * @param manager   The asset manager handling this asset
 * @param json      The JSON object specifying the node
 *
 * @return true if initialization was successful.
 */
bool SceneNode::initWithData(const AssetManager* assets,
                              const std::shared_ptr<JsonValue>& json) {
    CUAssertLog(_childOffset == -2, "Attempting to reinitialize a Node");
    if (!json) {
        return initWithPosition(0, 0, 0);
    }
    _childOffset = -1;
    
    _name = json->key();
    
    // It is VERY important to do this first
    Vec3 value;
    if (json->has("anchor")) {
        JsonValue* pos = json->get("anchor").get();
        CUAssertLog(pos->size() >= 2, "'anchor' must be at least a two element number array");
        _anchor.x = pos->get(0)->asFloat(0.0f);
        _anchor.y = pos->get(1)->asFloat(0.0f);
        _anchor.z = (pos->size() >= 3 ? pos->get(2)->asFloat(0.0f) : 0.0f);
    }

    if (json->has("position")) {
        JsonValue* pos = json->get("position").get();
        CUAssertLog(pos->size() >= 2, "'position' must be at least a two element number array");
        _position.x = pos->get(0)->asFloat(0.0f);
        _position.y = pos->get(1)->asFloat(0.0f);
        _position.z = (pos->size() >= 3 ? pos->get(2)->asFloat(0.0f) : 0.0f);
    }
    
    if (json->has("scale")) {
        JsonValue* scale = json->get("scale").get();
        if (scale->size() > 0) {
            _scale.x = scale->get(0)->asFloat(1.0f);
            _scale.y = (scale->size() >= 2 ? scale->get(1)->asFloat(1.0f) : 1.0f);
            _scale.z = (scale->size() >= 3 ? scale->get(2)->asFloat(1.0f) : 1.0f);
        } else {
            _scale.x = scale->asFloat(1.0f);
            _scale.y = _scale.x;
        }
    }
    
    if (json->has("rotation")) {
        JsonValue* rot = json->get("rotation").get();
        if (rot->isObject()) {
            float angle = rot->getFloat("angle",0.0f);
            angle *= M_PI/180.0f;

            Vec3 axis = Vec3::UNIT_Z;
            if (rot->has("axis")) {
                JsonValue* adata = rot->get("axis").get();
                CUAssertLog(adata->size() >= 3, "'axis' must be a three element number array");
                axis.x = adata->get(0)->asFloat(0.0f);
                axis.y = adata->get(1)->asFloat(0.0f);
                axis.z = adata->get(2)->asFloat(0.0f);
            }
            _rotate.set(axis, angle);
        } else if (rot->size() > 0) {
            _rotate.x = rot->get(0)->asFloat(0.0f);
            _rotate.y = (rot->size() >= 2 ? rot->get(1)->asFloat(0.0f) : 0.0f);
            _rotate.z = (rot->size() >= 3 ? rot->get(2)->asFloat(0.0f) : 0.0f);
            _rotate.w = (rot->size() >= 4 ? rot->get(3)->asFloat(0.0f) : 0.0f);
        } else {
            float angle = rot->asFloat(0.0f);
            angle *= M_PI/180.0f;
            _rotate.set(Vec3::UNIT_Z, angle);
        }
    }

    updateMatrices();
    _isVisible = json->getBool("visible",true);
    
    // Store the data for later
    _json = json;
    return true;
}

/**
 * Disposes all of the resouces used by this node.
 *
 * A disposed Node can be safely reinitialized. Any children owned by this
 * node will be released.  They will be deleted if no other object owns them.
 *
 * It is unsafe to call this on a Node that is still currently inside of
 * a scene graph.
 */
void SceneNode::dispose() {
    if (_childOffset >= 0) {
        removeFromParent();
    }
    removeAllChildren();
    _anchor = Vec2::ZERO;
    _position = Vec2::ZERO;
    _isVisible = true;
    _scale = Vec2::ONE;
    _rotate = Quaternion::ZERO;
    _modelmat  = Mat4::IDENTITY;
    _parent = nullptr;
    _graph = nullptr;
    _childOffset = -2;
    _tag = 0;
    _name = "";
    _hashOfName = 0;
}

/**
 * Performs a shallow copy of this node into dst.
 *
 * No children from this node are copied, and no children of dst are
 * modified. In addition, the parents of both node are unchanged. However,
 * all other attributes of this node are copied.
 *
 * @param dst   The node to copy into
 *
 * @return A reference to dst for chaining.
 */
std::shared_ptr<SceneNode> SceneNode::copy(const std::shared_ptr<SceneNode>& dst) const {
    dst->_isVisible = _isVisible;
    dst->_position = _position;
    dst->_anchor = _anchor;
    dst->_scale = _scale;
    dst->_rotate = _rotate;
    dst->_modelmat = _modelmat;
    dst->_tag = _tag;
    dst->_name = _name;
    dst->_hashOfName = _hashOfName;
    return dst;
}

#pragma mark -
#pragma mark Attributes
/**
 * Sets the position of the node in its parent's coordinate system.
 *
 * The node position is not necessarily the origin of the Node coordinate
 * system.  The relationship between the position and Node space is
 * determined by the anchor point.  See {@link getAnchor()} for more
 * details.
 *
 * @param position  The position of the node in its parent's coordinate system.
 */
void SceneNode::setPosition(const Vec3 position) {
    _position = position;
    updateMatrices();
}

/**
 * Sets the position of the node in its parent's coordinate system.
 *
 * The node position is not necessarily the origin of the Node coordinate
 * system.  The relationship between the position and Node space is
 * determined by the anchor point.  See {@link getAnchor()} for more
 * details.
 *
 * @param x The x-coordinate of the node in its parent's coordinate system.
 * @param y The x-coordinate of the node in its parent's coordinate system.
 * @param z The z-coordinate of the node in its parent's coordinate system.
 */
void SceneNode::setPosition(float x, float y, float z) {
    _position.set(x,y,z);
    updateMatrices();
}

/**
 * Sets the anchor point.
 *
 * The anchor point defines the relative origin of Node with respect to its
 * parent. It is a "pin" where the Node is attached to its parent.  In
 * effect, the translation of a Node is defined by its position plus
 * anchor point.
 *
 * The anchor point is defined as a position in node space.  It it the
 * origin by default. Note that changing the anchor will not move the
 * contents of the node in the parent space, but it will change the value
 * of the node position.
 *
 * @param anchor    The anchor point of node.
 */
void SceneNode::setAnchor(const Vec3 anchor) {
    if (anchor != _anchor) {
        Mat4::transform(_modelmat, anchor, &_position);
        _anchor = anchor;
        updateMatrices();
    }
}

/**
 * Sets the anchor point.
 *
 * The anchor point defines the relative origin of Node with respect to its
 * parent. It is a "pin" where the Node is attached to its parent.  In
 * effect, the translation of a Node is defined by its position plus
 * anchor point.
 *
 * The anchor point is defined as a position in node space.  It it the
 * origin by default. Note that changing the anchor will not move the
 * contents of the node in the parent space, but it will change the value
 * of the node position.
 *
 * @param x     The x-coordinate of the anchor.
 * @param y     The y-coordinate of the anchor.
 * @param z     The z-coordinate of the anchor.
 */
void SceneNode::setAnchor(float x, float y, float z) {
    setAnchor(Vec3(x,y,y));
}

/**
 * Sets the non-uniform scaling factor for this node about the anchor.
 *
 * This factor scales the node about the anchor (with the anchor unmoved).
 * Hence this is not the same as a scale applied to Node space, as the
 * origin may be different. Scaling is first, before any other transforms.
 *
 * @param scale The non-uniform scaling factor for this node about the anchor
 */
void SceneNode::setScale(const cugl::Vec3 scale) {
    _scale = scale;
    updateMatrices();
}

/**
 * Sets the rotation quaterion of this node about the anchor.
 *
 * This value rotates the node about the anchor, with the anchor unmoved.
 * Rotations are represented by quaternions, which store both an axis and
 * and angle of rotation (in radians).
 *
 * @param rot   The rotation quaterion of this node about the anchor.
 */
void SceneNode::setRotation(cugl::Quaternion rot) {
    _rotate = rot;
    updateMatrices();
}

/**
 * Returns a string representation of this vector for debugging purposes.
 *
 * If verbose is true, the string will include class information.  This
 * allows us to unambiguously identify the class.
 *
 * @param verbose Whether to include class information
 *
 * @return a string representation of this vector for debuggging purposes.
 */
std::string SceneNode::toString(bool verbose) const {
    std::stringstream ss;
    ss << (verbose ? "cugl::"+_classname+"(tag:" : "(tag:");
    ss <<  cugl::strtool::to_string(_tag);
    ss << ", name:" << _name;
    ss << ", children:" << cugl::strtool::to_string((Uint64)_children.size());
    ss << ")";
    if (verbose) {
        ss << "\n";
        for(auto it = _children.begin(); it != _children.end(); ++it) {
            ss << "  " << (*it)->toString(verbose);
        }
    }

    return ss.str();
}

#pragma mark -
#pragma mark Transforms
/**
 * Returns the matrix transforming node space to world space.
 *
 * This matrix is used to convert node coordinates into OpenGL coordinates.
 * It is the recursive (left-multiplied) transforms of all of its descendents.
 *
 * @return the matrix transforming node space to world space.
 */
Mat4 SceneNode::getNodeToWorldTransform() const {
    Mat4 result = _modelmat;
    if (_parent) {
        // Multiply on left
        Mat4::multiply(result,_parent->getNodeToWorldTransform(),&result);
    }
    return result;
}

/**
 * Converts a screen position to node (local) space coordinates.
 *
 * This method is useful for converting global positions like touches
 * or mouse clicks, which are represented in screen coordinates. Screen
 * coordinates typically have the origin in the top left.
 *
 * The screen coordinate system is defined by the scene's camera. The method
 * uses the camera to convert into world space, and then converts from world
 * space into not (local) space.
 *
 * This method returns the original point if there is no active scene.
 *
 * @param screenPoint   An position on the screen
 *
 * @return A point in node (local) space coordinates.
 */
Vec3 SceneNode::screenToNodeCoords(const Vec2 screenPoint) const {
    if (_graph == nullptr) {
        return screenPoint;
    }
    return worldToNodeCoords(_graph->getCamera()->screenToWorldCoords(screenPoint));
}

/**
 * Converts an node (local) position to screen coordinates.
 *
 * This method is useful for converting back to global positions like
 * touches or mouse clicks, which are represented in screen coordinates.
 * Screen coordinates typically have the origin in the top left.
 *
 * The screen coordinate system is defined by the scene's camera. The method
 * converts the node point into world space, and then uses the camera to
 * convert into screen space.
 *
 * @param nodePoint     A local position.
 *
 * @return A point in screen coordinates.
 */
Vec2 SceneNode::nodeToScreenCoords(const Vec3 nodePoint) const {
    if (_graph == nullptr) {
        return nodePoint;
    }
    return (_graph->getCamera()->worldToScreenCoords(nodeToWorldCoords(nodePoint)));
}


/**
 * Updates the model and normal matrices
 *
 * This transform is defined by scaling, rotation, and positional
 * translation, in that order.
 */
void SceneNode::updateMatrices() {
    _modelmat.setIdentity();
    Mat4::translate(_modelmat, -_anchor, &_modelmat);
    Mat4::scale(_modelmat, _scale, &_modelmat);
    Mat4::rotate(_modelmat, _rotate, &_modelmat);
    Mat4::translate(_modelmat, _position, &_modelmat);
}

#pragma mark -
#pragma mark Scene Graph
/**
 * Returns the child at the given position.
 *
 * For the base SceneNode class, children are always enumerated in the
 * order that they are added. However, this is not guaranteed for all
 * subclasses of SceneNode. Hence it is generally best to retrieve a child
 * using either a tag or a name instead.
 *
 * @param pos   The child position.
 *
 * @return the child at the given position.
 */
std::shared_ptr<SceneNode> SceneNode::getChild(unsigned int pos) {
    CUAssertLog(pos < _children.size(), "Position index out of bounds");
    return _children[pos];
}

/**
 * Returns the child at the given position.
 *
 * For the base SceneNode class, children are always enumerated in the
 * order that they are added. However, this is not guaranteed for all
 * subclasses of SceneNode. Hence it is generally best to retrieve a child
 * using either a tag or a name instead.
 *
 * @param pos   The child position.
 *
 * @return the child at the given position.
 */
const std::shared_ptr<SceneNode>& SceneNode::getChild(unsigned int pos) const {
    CUAssertLog(pos < _children.size(), "Position index out of bounds");
    return _children[pos];
}

/**
 * Returns the (first) child with the given tag.
 *
 * If there is more than one child of the given tag, it returns the first
 * one that is found. For the base SceneNode class, children are always
 * enumerated in the order that they are added. However, this is not
 * guaranteed for all subclasses of SceneNode. Hence it is very important
 * that tags be unique.
 *
 * @param tag   An identifier to find the child node.
 *
 * @return the (first) child with the given tag.
 */
std::shared_ptr<SceneNode> SceneNode::getChildByTag(unsigned int tag) const {
    for(auto it = _children.begin(); it != _children.end(); ++it) {
        if ((*it)->getTag() == tag) {
            return *it;
        }
    }
    return nullptr;
}

/**
 * Returns the (first) child with the given name.
 *
 * If there is more than one child of the given name, it returns the first
 * one that is found. For the base SceneNode class, children are always
 * enumerated in the order that they are added. However, this is not
 * guaranteed for all subclasses of SceneNode. Hence it is very important
 * that names be unique.
 *
 * @param name  An identifier to find the child node.
 *
 * @return the (first) child with the given name.
 */
std::shared_ptr<SceneNode> SceneNode::getChildByName(const std::string name) const {
    for(auto it = _children.begin(); it != _children.end(); ++it) {
        if ((*it)->getName() == name) {
            return *it;
        }
    }
    return nullptr;
}

/**
 * Adds a child to this node.
 *
 * Children are enumerated in the order that they are added.
 * However, it is still best to
 * For example, they may be resorted by their z-order. Hence you should
 * generally attempt to retrieve a child by tag or by name instead.
 *
 * @param child A child node.
 */
void SceneNode::addChild(const std::shared_ptr<SceneNode>& child) {
    CUAssertLog(child->_childOffset == -1, "The child is already in a scene graph");
    CUAssertLog(child->_graph == nullptr,  "The child is already in a scene graph");
    child->_childOffset = (unsigned int)_children.size();
    
    // Add the child
    _children.push_back(child);
    child->setParent(this);
    child->pushScene(_graph);
    
}

/**
 * Swaps the current child child1 with the new child child2.
 *
 * If inherit is true, the children of child1 are assigned to child2 after
 * the swap; this value is false by default.  The purpose of this value is
 * to allow transition nodes in the middle of the scene graph.
 *
 * This method is undefined if child1 is not a child of this node.
 *
 * @param child1    The current child of this node
 * @param child2    The child to swap it with.
 * @param inherit   Whether the new child should inherit the children of child1.
 */
void SceneNode::swapChild(const std::shared_ptr<SceneNode>& child1,
                           const std::shared_ptr<SceneNode>& child2, bool inherit) {
    _children[child1->_childOffset] = child2;
    child2->_childOffset = child1->_childOffset;
    child2->setParent(this);
    child1->setParent(nullptr);
    child2->pushScene(_graph);
    child1->pushScene(nullptr);
    
    // Check if we are dirty and/or inherit children
    if (inherit) {
        std::vector<std::shared_ptr<SceneNode>> grands = child1->getChildren();
        child1->removeAllChildren();
        for(auto it = grands.begin(); it != grands.end(); ++it) {
            child2->addChild(*it);
        }
    }
}

/**
 * Removes the child at the given position from this Node.
 *
 * Removing a child alters the position of every child after it.  Hence
 * it is unsafe to cache child positions.
 *
 * @param pos   The position of the child node which will be removed.
 */
void SceneNode::removeChild(unsigned int pos) {
    CUAssertLog(pos < _children.size(), "Position index out of bounds");
    std::shared_ptr<SceneNode> child = _children[pos];
    child->setParent(nullptr);
    child->pushScene(nullptr);
    child->_childOffset = -1;
    for(int ii = pos; ii < _children.size()-1; ii++) {
        _children[ii] = _children[ii+1];
        _children[ii]->_childOffset = ii;
    }
    _children.resize(_children.size()-1);
}

/**
 * Removes a child from this Node.
 *
 * Removing a child alters the position of every child after it.  Hence
 * it is unsafe to cache child positions.
 *
 * If the child is not in this node, nothing happens.
 *
 * @param child The child node which will be removed.
 */
void SceneNode::removeChild(const std::shared_ptr<SceneNode>& child) {
    CUAssertLog(_children[child->_childOffset] == child, "The child is not in this scene graph");
    removeChild(child->_childOffset);
}

/**
 * Removes a child from the Node by tag value.
 *
 * If there is more than one child of the given tag, it removes the first
 * one that is found. For the base SceneNode class, children are always
 * enumerated in the order that they are added.  However, this is not
 * guaranteed for subclasses of SceneNode. Hence it is very important
 * that tags be unique.
 *
 * @param tag   An integer to identify the node easily.
 */
void SceneNode::removeChildByTag(unsigned int tag) {
    std::shared_ptr<SceneNode> child = getChildByTag(tag);
    if (child != nullptr) {
        removeChild(child->_childOffset);
    }
}

/**
 * Removes a child from the Node by name.
 *
 * If there is more than one child of the given name, it removes the first
 * one that is found. For the base SceneNode class, children are always
 * enumerated in the order that they are added.  However, this is not
 * guaranteed for subclasses of SceneNode. Hence it is very important
 * that names be unique.
 *
 * @param name  A string to identify the node.
 */
void SceneNode::removeChildByName(const std::string name) {
    std::shared_ptr<SceneNode> child = getChildByName(name);
    if (child != nullptr) {
        removeChild(child->_childOffset);
    }
}

/**
 * Removes all children from this Node.
 */
void SceneNode::removeAllChildren() {
    for(auto it = _children.begin(); it != _children.end(); ++it) {
        (*it)->setParent(nullptr);
        (*it)->_childOffset = -1;
        (*it)->pushScene(nullptr);
    }
    _children.clear();
}

/**
 * Recursively sets the scene graph for this node and all its children.
 *
 * The purpose of this pointer is to climb back up to the root of the
 * scene graph tree. No node asserts ownership of its scene.
 *
 * @param scene A pointer to the scene graph.
 */
void SceneNode::pushScene(Scene3* scene) {
    setScene(scene);
    for(auto it = _children.begin(); it != _children.end(); ++it) {
        (*it)->pushScene(scene);
    }
}
