//
//  CUScene3.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a 3d scene graph node. It is the top level
//  class for the scene. It contains the camera and is the root of the scene
//  graph tree.
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
#include <sstream>
#include <algorithm>

/** Default near value for the camera */
#define DEFAULT_NEAR 0.1f
/** Default distance of initial camera on Z-axis */
#define DEFAULT_POS  2.0f

using namespace cugl;
using namespace cugl::scene3;

/**
 * Creates a new degenerate scene on the stack.
 *
 * The scene has no camera and must be initialized.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
 * the heap, use one of the static constructors instead.
 */
Scene3::Scene3() :
_camera(nullptr),
_name(""),
_active(false)
{}

/**
 * Disposes all of the resources used by this scene.
 *
 * A disposed scene can be safely reinitialized. Any children owned by this
 * scene will be released.  They will be deleted if no other object owns them.
 */
void Scene3::dispose() {
    removeAllChildren();
    _camera = nullptr;
    _name = "";
    _active = false;
}

/**
 * Initializes a scene with the given viewport.
 *
 * Offseting the viewport origin has little affect on the Scene in general.
 * It only affects the coordinate conversion methods {@link Camera#project()}
 * and {@link Camera#unproject()}. It is supposed to represent the offset
 * of the viewport in a larger canvas.
 *
 * @param x         The viewport x offset
 * @param y         The viewport y offset
 * @param width     The viewport width
 * @param height    The viewport height
 *
 * @return true if initialization was successful.
 */
bool Scene3::init(float x, float y, float width, float height) {
    // First make the camera
    _camera = PerspectiveCamera::alloc(x, y, width, height);
    _camera->update();
    _camera->setNear(DEFAULT_NEAR);
    _camera->update();
    _camera->setPosition(Vec3::UNIT_Z*DEFAULT_POS);
    _camera->update();
    _camera->lookAt(Vec3::ZERO,Vec3::UNIT_Y);
    _camera->update();
    _active = _camera != nullptr;
    
    // Then the pipeline
    _pipeline = Scene3Pipeline::allocWithStandard();
    
    return _active && _pipeline != nullptr;
}


#pragma mark -
#pragma mark Attributes

/**
 * Returns the camera for this scene.
 *
 * @return the camera for this scene.
 */
std::shared_ptr<Camera> Scene3::getCamera() {
    return std::dynamic_pointer_cast<Camera>(_camera);
}

/**
 * Returns the camera for this scene.
 *
 * @return the camera for this scene.
 */
const std::shared_ptr<Camera> Scene3::getCamera() const {
    return std::dynamic_pointer_cast<Camera>(_camera);
}

/**
 * Returns a string representation of this scene for debugging purposes.
 *
 * If verbose is true, the string will include class information.  This
 * allows us to unambiguously identify the class.
 *
 * @param verbose Whether to include class information
 *
 * @return a string representation of this scene for debuggging purposes.
 */
std::string Scene3::toString(bool verbose) const {
    std::stringstream ss;
    ss << (verbose ? "cugl::Scene3(name:" : "(name:");
    ss << _name << ")";
    return ss.str();
}


#pragma mark -
#pragma mark Scene Graph

/**
 * Returns the child at the given position.
 *
 * Children are not necessarily enumerated in the order that they are added.
 * Hence you should generally attempt to retrieve a child by tag or by name
 * instead.
 *
 * @param pos   The child position.
 *
 * @return the child at the given position.
 */
std::shared_ptr<SceneNode> Scene3::getChild(unsigned int pos) {
    CUAssertLog(pos < _children.size(), "Position index out of bounds");
    return _children[pos];
}

/**
 * Returns the child at the given position.
 *
 * Children are not necessarily enumerated in the order that they are added.
 * Hence you should generally attempt to retrieve a child by tag or by name
 * instead.
 *
 * @param pos   The child position.
 *
 * @return the child at the given position.
 */
const std::shared_ptr<SceneNode>& Scene3::getChild(unsigned int pos) const {
    CUAssertLog(pos < _children.size(), "Position index out of bounds");
    return _children[pos];
}

/**
 * Returns the (first) child with the given tag.
 *
 * If there is more than one child of the given tag, it returns the first
 * one that is found. Children are not necessarily enumerated in the order
 * that they are added. Hence it is very important that tags be unique.
 *
 * @param tag   An identifier to find the child node.
 *
 * @return the (first) child with the given tag.
 */
std::shared_ptr<SceneNode> Scene3::getChildByTag(unsigned int tag) const  {
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
 * one that is found. Children are not necessarily enumerated in the order
 * that they are added. Hence it is very important that names be unique.
 *
 * @param name  An identifier to find the child node.
 *
 * @return the (first) child with the given name.
 */
std::shared_ptr<SceneNode> Scene3::getChildByName(const std::string name) const {
    for(auto it = _children.begin(); it != _children.end(); ++it) {
        if ((*it)->getName() == name) {
            return *it;
        }
    }
    return nullptr;
}

/**
 * Adds a child to this scene.
 *
 * Children are not necessarily enumerated in the order that they are added.
 * Hence you should generally attempt to retrieve a child by tag or by name
 * instead.
 *
 * @param child A child node.
 */
void Scene3::addChild(const std::shared_ptr<SceneNode>& child) {
    CUAssertLog(child->_childOffset == -1, "The child is already in a scene graph");
    CUAssertLog(child->_graph == nullptr,  "The child is already in a scene graph");
    child->_childOffset = (unsigned int)_children.size();
    
    // Add the child
    _children.push_back(child);
    child->setParent(nullptr);
    child->pushScene(this);
}

/**
 * Swaps the current child child1 with the new child child2.
 *
 * If inherit is true, the children of child1 are assigned to child2 after
 * the swap; this value is false by default.  The purpose of this value is
 * to allow transitions in the scene graph.
 *
 * This method is undefined if child1 is not a child of this scene.
 *
 * @param child1    The current child of this node
 * @param child2    The child to swap it with.
 * @param inherit   Whether the new child should inherit the children of child1.
 */
void Scene3::swapChild(const std::shared_ptr<SceneNode>& child1,
                       const std::shared_ptr<SceneNode>& child2,
                       bool inherit) {
    _children[child1->_childOffset] = child2;
    child2->_childOffset = child1->_childOffset;
    child2->setParent(nullptr);
    child1->setParent(nullptr);
    child2->pushScene(this);
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
void Scene3::removeChild(unsigned int pos) {
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
void Scene3::removeChild(const std::shared_ptr<SceneNode>& child) {
    CUAssertLog(_children[child->_childOffset] == child, "The child is not in this scene graph");
    removeChild(child->_childOffset);
}

/**
 * Removes a child from the Scene by tag value.
 *
 * If there is more than one child of the given tag, it removes the first
 * one that is found. Children are not necessarily enumerated in the order
 * that they are added. Hence it is very important that tags be unique.
 *
 * @param tag   An integer to identify the node easily.
 */
void Scene3::removeChildByTag(unsigned int tag) {
    std::shared_ptr<SceneNode> child = getChildByTag(tag);
    if (child != nullptr) {
        removeChild(child->_childOffset);
    }
}


/**
 * Removes a child from the Scene by name.
 *
 * If there is more than one child of the given name, it removes the first
 * one that is found. Children are not necessarily enumerated in the order
 * that they are added. Hence it is very important that names be unique.
 *
 * @param name  A string to identify the node.
 */
void Scene3::removeChildByName(const std::string name) {
    std::shared_ptr<SceneNode> child = getChildByName(name);
    if (child != nullptr) {
        removeChild(child->_childOffset);
    }
}


/**
 * Removes all children from this Node.
 */
void Scene3::removeAllChildren() {
    for(auto it = _children.begin(); it != _children.end(); ++it) {
        (*it)->setParent(nullptr);
        (*it)->_childOffset = -1;
        (*it)->pushScene(nullptr);
    }
    _children.clear();
}

#pragma mark -
#pragma mark Rendering
/**
 * Draws all of the children in this scene with the given pipeline.
 *
 * Rendering happens by traversing the the scene graph using an "Pre-Order"
 * tree traversal algorithm ( https://en.wikipedia.org/wiki/Tree_traversal#Pre-order ).
 * That means that parents are always draw before children.
 *
 * @param pipeline  The pipeline to draw with.
 */
void Scene3::render() {
    if (_pipeline == nullptr) {
        return;
    }
    for(auto it = _children.begin(); it != _children.end(); ++it) {
        traverse(*it, Mat4::IDENTITY);
    }

    _pipeline->flush(_camera);
}

/**
 * Recursively adds the given scene graph node to the pipeline.
 *
 * This method is uses to add nodes to the {@link Scene3Pipeline} by
 * recursively traversing the tree.
 *
 * @param node      The current node in the traversal.
 * @param transform The global transformation matrix.
 */
void Scene3::traverse(const std::shared_ptr<SceneNode>& node,
                      const Mat4& transform) {
    Mat4 matrix;
    Mat4::multiply(transform, node->getModelMatrix(), &matrix);
    _pipeline->append(node,matrix);
    auto kids = node->getChildren();
    for(auto it = kids.begin(); it != kids.end(); ++it) {
        traverse(*it, matrix);
    }
}
