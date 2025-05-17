//
//  CUSceneNode.h
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
//      warranty. In no event will the authors be held liable for any damages
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
#ifndef __CU_SCENE_NODE_3_H__
#define __CU_SCENE_NODE_3_H__
#include <cugl/core/math/cu_math.h>
#include <cugl/graphics/cu_graphics.h>
#include <cugl/core/assets/CUAssetManager.h>

namespace cugl {

    /**
     * The classes to construct a 3-d scene graph.
     *
     * Unlike the scene2 package, the 3-d scene graph classes are quite limited.
     * We only have support for OBJ/MTL files, as well as simple billboarding.
     * There is no support for bone animation or physically based rendering.
     *
     * The reason for this limitation is because this is a student training
     * engine, and we often like to task students at adding those features.
     * In addition, unlike our 2-d scene graph with Figma support, there are
     * a lot of third party libraries out there that handle rendering better
     * for 3-d scenes.
     */
    namespace scene3 {

// Forward reference to the scene
class Scene3;
class Scene3Pipeline;

/**
 * This class provides a 3d scene graph node.
 *
 * A base node is a cubic that can contain other (transformed) nodes. Each
 * node forms a its own coordinate space. All rendering takes place inside of
 * this coordinate space. By default, a mode does not render anything. Instead,
 * all drawing is handled via {@link Scene3Pipeline}, which traverses the
 * scene graph tree before drawing.
 *
 * Unlike the 2d scene graph nodes, we do not associate a bounding box (or
 * volume with the node). While that is important for UI layout code, it is
 * less important for 3d scene graphs. However, we do still have the concept
 * of the anchor. All transforms (scaling and rotation) are applied relative
 * to the anchor, not the node origin. But because there is no bounding box,
 * this anchor is specified as a point in node space, and not a percentage.
 */
class SceneNode {
protected:
    /**
     * An identifying tag.
     *
     * This value is used to quickly identify the child of a node. To work
     * properly, a tag should be unique within a scene graph. It is 0 if
     * undefined.
     */
    unsigned int _tag;
    
    /**
     * A descriptive identifying tag.
     *
     * Like tag, this value is used to quickly identify the child of a node.
     * However, it is more descriptive, and so is useful in debugging.
     */
    std::string _name;
    
    /**
     * A cached has value of _name.
     *
     * This value is used to speed up look-ups by string.
     */
    size_t _hashOfName;
    
    /** The class name for the specific subclass */
    std::string _classname;
    
    /** Whether this node is visible */
    bool  _isVisible;
    
    /** The position of this node in the parent coordinate space. */
    cugl::Vec3 _position;
    
    /**
     * The anchor point of the node.
     *
     * The anchor point is a position in node space. All transforms applied
     * to this node occur about the anchor, not the origin.
     */
    cugl::Vec3 _anchor;
    
    /**
     * The scaling factor to transform this node.
     *
     * This scaling happens in the parent coordinate space. Scaling happens
     * before any rotation. The node coordinate space remains unchanged.
     */
    cugl::Vec3 _scale;
    
    /**
     * The rotation to transform this node.
     *
     * Rotation is applied after any scaling, but before the position
     * translation. The node coordinate space remains unchanged.
     */
    cugl::Quaternion _rotate;
    
    /** The model matrix for this node */
    Mat4 _modelmat;
    
    /** The array of children nodes */
    std::vector<std::shared_ptr<SceneNode>> _children;
    
    /** A weaker pointer to the parent (or null if root) */
    SceneNode* _parent;
    /** A weaker pointer to the scene (or null if not in a scene) */
    Scene3* _graph;

    /** The (current) child offset of this node (-1 if root) */
    int _childOffset;
    
    /** The batch key */
    CUEnum _batchkey;
        
    /** The defining JSON data for this node (if any) */
    std::shared_ptr<JsonValue> _json;
    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates an uninitialized node.
     *
     * You must initialize this node before use.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a node on the
     * heap, use one of the static constructors instead.
     */
    SceneNode();
    
    /**
     * Deletes this node, disposing all resources
     */
    ~SceneNode() { dispose(); }
    
    /**
     * Disposes all of the resources used by this node.
     *
     * A disposed node can be safely reinitialized. Any children owned by this
     * node will be released. They will be deleted if no other object owns them.
     *
     * It is unsafe to call this on a node that is still currently inside of
     * a scene graph.
     */
    virtual void dispose();
    
    /**
     * Initializes a node at the world origin.
     *
     * The node has both position and anchor (0,0,0).
     *
     * @return true if initialization was successful.
     */
    virtual bool init() {
        return initWithPosition(0, 0, 0);
    }
    
    /**
     * Initializes a node at the given position.
     *
     * The node has anchor (0,0,0). As a result, the position is identified with
     * the origin of the node space.
     *
     * @param pos   The origin of the node in parent space
     *
     * @return true if initialization was successful.
     */
    virtual bool initWithPosition(const Vec3 pos);
    
    /**
     * Initializes a node at the given position.
     *
     * The node has anchor (0,0,0). As a result, the position is identified with
     * the origin of the node space.
     *
     * @param x     The x-coordinate of the node in parent space
     * @param y     The y-coordinate of the node in parent space
     * @param z     The z-coordinate of the node in parent space
     *
     * @return true if initialization was successful.
     */
    bool initWithPosition(float x, float y, float z) {
        return initWithPosition(Vec3(x,y,z));
    }
    
    /**
     * Initializes a node with the given anchor.
     *
     * The anchor defines the transformation origin of the scene node. All
     * scaling and rotation happens about the anchor, not the origin. This
     * anchor is placed at (0,0,0) in parent space.
     *
     * @param anchor    The node anchor
     *
     * @return true if initialization was successful.
     */
    virtual bool initWithAnchor(const Vec3 anchor) {
        return initWithAnchor(Vec3::ZERO,anchor);
    }
    
    /**
     * Initializes a node with the given anchor.
     *
     * The anchor defines the transformation origin of the scene node. All
     * scaling and rotation happens about the anchor, not the origin. This
     * anchor is placed at (0,0,0) in parent space.
     *
     * @param x     The x-coordinate of the anchor in node space
     * @param y     The y-coordinate of the anchor in node space
     * @param z     The z-coordinate of the anchor in node space
     *
     * @return true if initialization was successful.
     */
    bool initWithAnchor(float x, float y, float z) {
        return initWithAnchor(Vec3(x,y,z));
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
    virtual bool initWithAnchor(const Vec3 pos, const Vec3 anchor);
    
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
     * missing. If the axis is missing, rotation is about the z-axis
     *
     * @param manager   The asset manager handling this asset
     * @param json      The JSON object specifying the node
     *
     * @return true if initialization was successful.
     */
    virtual bool initWithData(const AssetManager* manager,
                              const std::shared_ptr<JsonValue>& json);
    

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
    virtual std::shared_ptr<SceneNode> copy(const std::shared_ptr<SceneNode>& dst) const;

#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a newly allocated node at the world origin.
     *
     * The node has both position and size (0,0,0).
     *
     * @return a newly allocated node at the world origin.
     */
    static std::shared_ptr<SceneNode> alloc() {
        std::shared_ptr<SceneNode> result = std::make_shared<SceneNode>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated node at the given position.
     *
     * The node has size (0,0,0). As a result, the position is identified with
     * the origin of the node space.
     *
     * @param pos   The origin of the node in parent space
     *
     * @return a newly allocated node at the given position.
     */
    static std::shared_ptr<SceneNode> allocWithPosition(const Vec3 pos) {
        std::shared_ptr<SceneNode> result = std::make_shared<SceneNode>();
        return (result->initWithPosition(pos) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated node at the given position.
     *
     * The node has size (0,0,0). As a result, the position is identified with
     * the origin of the node space.
     *
     * @param x     The x-coordinate of the node in parent space
     * @param y     The y-coordinate of the node in parent space
     * @param z     The z-coordinate of the node in parent space
     *
     * @return a newly allocated node at the given position.
     */
    static std::shared_ptr<SceneNode> allocWithPosition(float x, float y, float z) {
        std::shared_ptr<SceneNode> result = std::make_shared<SceneNode>();
        return (result->initWithPosition(x,y,z) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated node with the given anchor.
     *
     * The anchor defines the transformation origin of the scene node. All
     * scaling and rotation happens about the anchor, not the origin. This
     * anchor is placed at (0,0,0) in parent space.
     *
     * @param anchor    The anchor of this node
     *
     * @return a newly allocated node with the given anchor.
     */
    static std::shared_ptr<SceneNode> allocWithAnchor(const Vec3 anchor) {
        std::shared_ptr<SceneNode> result = std::make_shared<SceneNode>();
        return (result->initWithAnchor(anchor) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated node with the given anchor.
     *
     * The anchor defines the transformation origin of the scene node. All
     * scaling and rotation happens about the anchor, not the origin. This
     * anchor is placed at (0,0,0) in parent space.
     *
     * @param x     The x-coordinate of the anchor in node space
     * @param y     The y-coordinate of the anchor in node space
     * @param z     The z-coordinate of the anchor in node space
     *
     * @return a newly allocated node with the given anchor.
     */
    static std::shared_ptr<SceneNode> allocWithAnchor(float x, float y, float z) {
        std::shared_ptr<SceneNode> result = std::make_shared<SceneNode>();
        return (result->initWithAnchor(x,y,z) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated node with the given anchor and position.
     *
     * The anchor defines the transformation origin of the scene node. All
     * scaling and rotation happens about the anchor, not the origin. This
     * anchor is placed at position pos in parent space.
     *
     * @param pos       The node position
     * @param anchor    The node anchor
     *
     * @return a newly allocated node with the given anchor and position.
     */
    static std::shared_ptr<SceneNode> allocWithAnchor(const Vec3 pos, const Vec3 anchor) {
        std::shared_ptr<SceneNode> result = std::make_shared<SceneNode>();
        return (result->initWithAnchor(pos,anchor) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated node with the given JSON specificaton.
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
     * missing. If the axis is missing, rotation is about the z-axis
     *
     * @param manager   The asset manager handling this asset
     * @param json      The JSON object specifying the node
     *
     * @return a newly allocated node with the given JSON specificaton.
     */
    static std::shared_ptr<SceneNode> allocWithData(const AssetManager* manager,
                                                    const std::shared_ptr<JsonValue>& json) {
        std::shared_ptr<SceneNode> result = std::make_shared<SceneNode>();
        return (result->initWithData(manager,json) ? result : nullptr);
    }
    
#pragma mark -
#pragma mark Identifiers
    /**
     * Returns a tag that is used to identify the node easily.
     *
     * This tag is used to quickly access a child node, since child position
     * may change. To work properly, a tag should be unique within a scene
     * graph. It is 0 if undefined.
     *
     * @return a tag that is used to identify the node easily.
     */
    unsigned int getTag() const { return _tag; }
    
    /**
     * Sets a tag that is used to identify the node easily.
     *
     * This tag is used to quickly access a child node, since child position
     * may change. To work properly, a tag should be unique within a scene
     * graph. It is 0 if undefined.
     *
     * @param tag   A tag that is used to identify the node easily.
     */
    void setTag(unsigned int tag) { _tag = tag; }
    
    /**
     * Returns a string that is used to identify the node.
     *
     * This name is used to access a child node, since child position may
     * change. In addition, the name is useful for debugging. To work properly,
     * a name should be unique within a scene graph. It is empty if undefined.
     *
     * @return a string that is used to identify the node.
     */
    const std::string getName() const { return _name; }
    
    /**
     * Sets a string that is used to identify the node.
     *
     * This name is used to access a child node, since child position may
     * change. In addition, the name is useful for debugging. To work properly,
     * a name should be unique within a scene graph. It is empty if undefined.
     *
     * @param name  A string that is used to identify the node.
     */
    void setName(const std::string name) {
        _name = name;
        _hashOfName = std::hash<std::string>()(_name);
    }
    
    /**
     * Returns the class name of this node.
     *
     * This method is to help us speed up subclass-based polymorphism
     * on the scene graphs.
     *
     * @return the class name of this node.
     */
    const std::string getClassName() const { return _classname; }
    
    /**
     * Returns the batch key for this node type.
     *
     * Batch keys are uses to identify the correct {@link Scene3Batch} to use
     * in {@link Scene3Pipeline}. Each batch type has its own key. See
     * {@link Scene3Batch#getBatchKey}.
     *
     * @return the batch key for this node type.
     */
    CUEnum getBatchKey() const { return _batchkey; }
    
    /**
     * Sets the batch key for this node type.
     *
     * Batch keys are uses to identify the correct {@link Scene3Batch} to use
     * in {@link Scene3Pipeline}. Each batch type has its own key. See
     * {@link Scene3Batch#getBatchKey}.
     *
     * @param key   The batch key for this node type.
     */
    void setBatchKey(CUEnum key) { _batchkey = key; }
    
    /**
     * Returns a string representation of this node for debugging purposes.
     *
     * If verbose is true, the string will include class information. This
     * allows us to unambiguously identify the class.
     *
     * @param verbose Whether to include class information
     *
     * @return a string representation of this node for debuggging purposes.
     */
    virtual std::string toString(bool verbose = false) const;
    
    /** Cast from a Node to a string. */
    operator std::string() const { return toString(); }

#pragma mark -
#pragma mark Position and Anchor
    /**
     * Returns the position of the node in its parent's coordinate system.
     *
     * The node position is not necessarily the origin of the Node coordinate
     * system. The relationship between the position and Node space is
     * determined by the anchor point. See {@link getAnchor()} for more
     * details.
     *
     * @return the position of the node in its parent's coordinate system.
     */
    const Vec3 getPosition() const { return _position; }
    
    /**
     * Sets the position of the node in its parent's coordinate system.
     *
     * The node position is not necessarily the origin of the Node coordinate
     * system. The relationship between the position and Node space is
     * determined by the anchor point. See {@link getAnchor()} for more
     * details.
     *
     * @param position  The position of the node in its parent's coordinate system.
     */
    void setPosition(const Vec3 position);
    
    /**
     * Sets the position of the node in its parent's coordinate system.
     *
     * The node position is not necessarily the origin of the Node coordinate
     * system. The relationship between the position and Node space is
     * determined by the anchor point. See {@link getAnchor()} for more
     * details.
     *
     * @param x The x-coordinate of the node in its parent's coordinate system.
     * @param y The x-coordinate of the node in its parent's coordinate system.
     * @param z The z-coordinate of the node in its parent's coordinate system.
     */
    void setPosition(float x, float y, float z);
    
    /**
     * Returns the position of the anchor point node in world space.
     *
     * Note that this is the position of the anchor point. This is not the
     * same as the location of the node origin in world space.
     *
     * @return the position of the anchor point node in world space.
     */
    Vec3 getWorldPosition() const {
        return nodeToWorldCoords(_anchor);
    }
    
    /**
     * Sets the anchor point.
     *
     * The anchor point defines the relative origin of Node with respect to
     * its parent. It is a "pin" where the Node is attached to its parent. In
     * effect, the translation of a Node is defined by its position plus
     * anchor point.
     *
     * The anchor point is defined as a position in node space. It is the
     * origin by default. Note that changing the anchor will not move the
     * contents of the node in the parent space, but it will change the value
     * of the node position.
     *
     * @param anchor    The anchor point of node.
     */
    void setAnchor(const Vec3 anchor);
    
    /**
     * Sets the anchor point.
     *
     * The anchor point defines the relative origin of Node with respect to its
     * parent. It is a "pin" where the Node is attached to its parent. In
     * effect, the translation of a Node is defined by its position plus
     * anchor point.
     *
     * The anchor point is defined as a position in node space. It it the
     * origin by default. Note that changing the anchor will not move the
     * contents of the node in the parent space, but it will change the value
     * of the node position.
     *
     * @param x     The x-coordinate of the anchor.
     * @param y     The y-coordinate of the anchor.
     * @param z     The z-coordinate of the anchor.
     */
    void setAnchor(float x, float y, float z);
    
    /**
     * Returns the anchor point.
     *
     * The anchor point defines the relative origin of Node with respect to its
     * parent. It is a "pin" where the Node is attached to its parent. In
     * effect, the translation of a Node is defined by its position plus
     * anchor point.
     *
     * The anchor point is defined as a position in node space. It is the
     * origin by default. Note that changing the anchor will not move the
     * contents of the node in the parent space, but it will change the value
     * of the node position.
     *
     * @return The anchor point.
     */
    const Vec3 getAnchor() const { return _anchor; }
    
    /**
     * Returns true if the node is visible.
     *
     * If a node is not visible, then it is not drawn. This means that its
     * children are not visible as well, regardless of their visibility settings.
     * The default value is true, making the node visible.
     *
     * @return true if the node is visible.
     */
    bool isVisible() const { return _isVisible; }
    
    /**
     * Sets whether the node is visible.
     *
     * If a node is not visible, then it is not drawn. This means that its
     * children are not visible as well, regardless of their visibility settings.
     * The default value is true, making the node visible.
     *
     * @param visible   true if the node is visible.
     */
    void setVisible(bool visible) { _isVisible = visible; }

#pragma mark -
#pragma mark Transforms
    /**
     * Returns the non-uniform scaling factor for this node about the anchor.
     *
     * This factor scales the node about the anchor (with the anchor unmoved).
     * Hence this is not the same as a scale applied to Node space, as the
     * origin may be different. Scaling is first, before any other transforms.
     *
     * @return the non-uniform scaling factor for this node about the anchor
     */
    cugl::Vec3 getScale() const { return _scale; }
    
    /**
     * Sets the uniform scaling factor for this node about the anchor.
     *
     * This factor scales the node about the anchor (with the anchor unmoved).
     * Hence this is not the same as a scale applied to Node space, as the
     * origin may be different. Scaling is first, before any other transforms.
     *
     * @param scale The uniform scaling factor for this node about the anchor
     */
    void setScale(float scale) {
        setScale(Vec3(scale,scale,scale));
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
    void setScale(const cugl::Vec3 scale);

    /**
     * Returns the rotation quaterion of this node about the anchor.
     *
     * This value rotates the node about the anchor, with the anchor unmoved.
     * Rotations are represented by quaternions, which store both an axis and
     * and angle of rotation (in radians).
     *
     * @return the rotation quaterion of this node about the anchor.
     */
    cugl::Quaternion getRotation() const { return _rotate; }
    
    /**
     * Sets the rotation quaterion of this node about the anchor.
     *
     * This value rotates the node about the anchor, with the anchor unmoved.
     * Rotations are represented by quaternions, which store both an axis and
     * and angle of rotation (in radians).
     *
     * @param rot   The rotation quaterion of this node about the anchor.
     */
    void setRotation(cugl::Quaternion rot);
    
    /**
     * Returns the model matrix of this node.
     *
     * The model matrix defines the transform of this node in parent space.
     *
     * @return the model matrix of this node.
     */
    const Mat4& getModelMatrix() const { return _modelmat; }
    
    /**
     * Returns the matrix transforming node space to parent space.
     *
     * This value is the node's transform. It is computed from the anchor,
     * scale, rotation, and position.
     *
     * @return the matrix transforming node space to parent space.
     */
    const Mat4& getNodeToParentTransform() const { return _modelmat; }
    
    /**
     * Returns the matrix transforming parent space to node space.
     *
     * This value is the node's transform. It is computed from the anchor,
     * scale, rotation, and position.
     *
     * @return the matrix transforming parent space to node space.
     */
    Mat4 getParentToNodeTransform() const { return _modelmat.getInverse(); }

     /**
      * Returns the matrix transforming node space to world space.
      *
      * This matrix is used to convert node coordinates into OpenGL coordinates.
      * It is the recursive (left-multiplied) node-to-parent transforms of all
      * of its ancestors.
      *
      * @return the matrix transforming node space to world space.
      */
     virtual Mat4 getNodeToWorldTransform() const;
     
     /**
      * Returns the matrix transforming node space to world space.
      *
      * This matrix is used to convert OpenGL coordinates into node coordinates.
      * This method is useful for converting global positions like touches
      * or mouse clicks. It is the recursive (right-multiplied) parent-to-node
      * transforms of all of its ancestors.
      *
      * @return the matrix transforming node space to world space.
      */
    Mat4 getWorldToNodeTransform() const {
         return getNodeToWorldTransform().getInverse();
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
    Vec3 screenToNodeCoords(const Vec2 screenPoint) const;

    
    /**
     * Converts an OpenGL position to node (local) space coordinates.
     *
     * See getWorldtoNodeTransform() for how this conversion takes place.
     * That method should be used instead if there are many points to convert,
     * as this method will recompute the transform matrix each time.
     *
     * @param worldPoint    An OpenGL position.
     *
     * @return A point in node (local) space coordinates.
     */
    Vec3 worldToNodeCoords(const Vec3 worldPoint) const {
        return getWorldToNodeTransform().transform(worldPoint);
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
     * This method returns the original point if there is no active scene.
     *
     * @param nodePoint     A local position.
     *
     * @return A point in screen coordinates.
     */
    Vec2 nodeToScreenCoords(const Vec3 nodePoint) const;
    
    /**
     * Converts an node (local) position to OpenGL coordinates.
     *
     * See getNodeToWorldTransform() for how this conversion takes place.
     * That method should be used instead if there are many points to convert,
     * as this method will recompute the transform matrix each time.
     *
     * @param nodePoint     A local position.
     *
     * @return A point in OpenGL coordinates.
     */
    Vec3 nodeToWorldCoords(const Vec3 nodePoint) const {
        return getNodeToWorldTransform().transform(nodePoint);
    }
    
    /**
     * Converts an parent space position to node (local) space coordinates.
     *
     * See getParentToNodeTransform() for how this conversion takes place.
     * That method should be used instead if there are many points to convert,
     * as this method will recompute the transform matrix each time.
     *
     * @param parentPoint   A parent position.
     *
     * @return A point in node (local) space coordinates.
     */
    Vec3 parentToNodeCoords(const Vec3 parentPoint) const {
        return getParentToNodeTransform().transform(parentPoint);
    }
    
    /**
     * Converts an node (local) space position to parent coordinates.
     *
     * See getNodeToParentTransform() for how this conversion takes place.
     * That method should be used instead if there are many points to convert,
     * as this method will recompute the transform matrix each time.
     *
     * @param nodePoint     A local position.
     *
     * @return A point in parent space coordinates.
     */
    Vec3 nodeToParentCoords(const Vec3 nodePoint) const {
        return getNodeToParentTransform().transform(nodePoint);
    }
    
#pragma mark -
#pragma mark Scene Graph
    /**
     * Returns the number of children of this node.
     *
     * @return The number of children of this node.
     */
    size_t getChildCount() const { return _children.size(); }

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
    std::shared_ptr<SceneNode> getChild(unsigned int pos);

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
    const std::shared_ptr<SceneNode>& getChild(unsigned int pos) const;
    
    /**
     * Returns the child at the given position, typecast to a shared T pointer.
     *
     * This method is provided to simplify the polymorphism of a scene graph.
     * While all children are a subclass of type Node, you may want to access
     * them by their specific subclass. If the child is not an instance of
     * type T (or a subclass), this method returns nullptr.
     *
     * For the base SceneNode class, children are always enumerated in the
     * order that they are added. However, this is not guaranteed for all
     * subclasses of SceneNode. Hence it is generally best to retrieve a child
     * using either a tag or a name instead.
     *
     * @param pos   The child position.
     *
     * @return the child at the given position, typecast to a shared T pointer.
     */
    template <typename T>
    inline std::shared_ptr<T> getChild(unsigned int pos) const {
        return std::dynamic_pointer_cast<T>(getChild(pos));
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
    std::shared_ptr<SceneNode> getChildByTag(unsigned int tag) const;
    
    /**
     * Returns the (first) child with the given tag, typecast to a shared T pointer.
     *
     * This method is provided to simplify the polymorphism of a scene graph.
     * While all children are a subclass of type Node, you may want to access
     * them by their specific subclass. If the child is not an instance of
     * type T (or a subclass), this method returns nullptr.
     *
     * If there is more than one child of the given tag, it returns the first
     * one that is found. For the base SceneNode class, children are always
     * enumerated in the order that they are added. However, this is not
     * guaranteed for all subclasses of SceneNode. Hence it is very important
     * that tags be unique.
     *
     * @param tag   An identifier to find the child node.
     *
     * @return the (first) child with the given tag, typecast to a shared T pointer.
     */
    template <typename T>
    inline std::shared_ptr<T> getChildByTag(unsigned int tag) const {
        return std::dynamic_pointer_cast<T>(getChildByTag(tag));
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
    std::shared_ptr<SceneNode> getChildByName(const std::string name) const;
    
    /**
     * Returns the (first) child with the given name, typecast to a shared T pointer.
     *
     * This method is provided to simplify the polymorphism of a scene graph.
     * While all children are a subclass of type Node, you may want to access
     * them by their specific subclass. If the child is not an instance of
     * type T (or a subclass), this method returns nullptr.
     *
     * If there is more than one child of the given name, it returns the first
     * one that is found. For the base SceneNode class, children are always
     * enumerated in the order that they are added. However, this is not
     * guaranteed for all subclasses of SceneNode. Hence it is very important
     * that names be unique.
     *
     * @param name  An identifier to find the child node.
     *
     * @return the (first) child with the given name, typecast to a shared T pointer.
     */
    template <typename T>
    inline std::shared_ptr<T> getChildByName(const std::string name) const {
        return std::dynamic_pointer_cast<T>(getChildByName(name));
    }

    /**
     * Returns the list of the node's children.
     *
     * @return the list of the node's children.
     */
    std::vector<std::shared_ptr<SceneNode>> getChildren() { return _children; }

    /**
     * Returns the list of the node's children.
     *
     * @return the list of the node's children.
     */
    const std::vector<std::shared_ptr<SceneNode>>& getChildren() const { return _children; }
    
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
    void addChild(const std::shared_ptr<SceneNode>& child);
    
    
    /**
     * Adds a child to this node with the given tag.
     *
     * For the base SceneNode class, children are always enumerated in the
     * order that they are added. However, this is not guaranteed for all
     * subclasses of SceneNode. Hence it is generally best to retrieve a child
     * using either a tag or a name instead.
     *
     * @param child A child node.
     * @param tag   An integer to identify the node easily.
     */
    void addChildWithTag(const std::shared_ptr<SceneNode>& child, unsigned int tag) {
        addChild(child);
        child->setTag(tag);
    }

    /**
     * Adds a child to this node with the given name.
     *
     * For the base SceneNode class, children are always enumerated in the
     * order that they are added. However, this is not guaranteed for all
     * subclasses of SceneNode. Hence it is generally best to retrieve a child
     * using either a tag or a name instead.
     *
     * @param child A child node.
     * @param name  A string to identify the node.
     */
    void addChildWithName(const std::shared_ptr<SceneNode>& child, const std::string name) {
        addChild(child);
        child->setName(name);
    }
    
    /**
     * Swaps the current child child1 with the new child child2.
     *
     * If inherit is true, the children of child1 are assigned to child2 after
     * the swap; this value is false by default. The purpose of this value is
     * to allow transition nodes in the middle of the scene graph.
     *
     * This method is undefined if child1 is not a child of this node.
     *
     * @param child1    The current child of this node
     * @param child2    The child to swap it with.
     * @param inherit   Whether the new child should inherit the children of child1.
     */
    void swapChild(const std::shared_ptr<SceneNode>& child1,
                   const std::shared_ptr<SceneNode>& child2, bool inherit=false);
    
    /**
     * Returns a (weak) pointer to the parent node.
     *
     * The purpose of this pointer is to climb back up the scene graph tree.
     * No child asserts ownership of its parent.
     *
     * @return a (weak) pointer to the parent node.
     */
    SceneNode* getParent() { return _parent; }
    
    /**
     * Returns a (weak) pointer to the parent node.
     *
     * The purpose of this pointer is to climb back up the scene graph tree.
     * No child asserts ownership of its parent.
     *
     * @return a (weak) pointer to the parent node.
     */
    const SceneNode* getParent() const { return _parent; }
    
    /**
     * Returns a (weak) pointer to the scene graph.
     *
     * The purpose of this pointer is to climb back up to the root of the
     * scene graph tree. No node asserts ownership of its scene.
     *
     * @return a (weak) pointer to the scene graph.
     */
    Scene3* getScene() { return _graph; }
    
    /**
     * Returns a (weak) pointer to the scene graph.
     *
     * The purpose of this pointer is to climb back up to the root of the
     * scene graph tree. No node asserts ownership of its scene.
     *
     * @return a (weak) pointer to the scene graph.
     */
    const Scene3* getScene() const { return _graph; }

    /**
     * Removes this node from its parent node.
     *
     * If the node has no parent, nothing happens.
     */
    void removeFromParent() { if (_parent) { _parent->removeChild(_childOffset); } }
    
    /**
     * Removes the child at the given position from this Node.
     *
     * Removing a child alters the position of every child after it. Hence
     * it is unsafe to cache child positions.
     *
     * @param pos   The position of the child node which will be removed.
     */
    virtual void removeChild(unsigned int pos);
    
    /**
     * Removes a child from this Node.
     *
     * Removing a child alters the position of every child after it. Hence
     * it is unsafe to cache child positions.
     *
     * If the child is not in this node, nothing happens.
     *
     * @param child The child node which will be removed.
     */
    void removeChild(const std::shared_ptr<SceneNode>& child);
    
    /**
     * Removes a child from the Node by tag value.
     *
     * If there is more than one child of the given tag, it removes the first
     * one that is found. For the base SceneNode class, children are always
     * enumerated in the order that they are added. However, this is not
     * guaranteed for subclasses of SceneNode. Hence it is very important
     * that tags be unique.
     *
     * @param tag   An integer to identify the node easily.
     */
    void removeChildByTag(unsigned int tag);
    
    /**
     * Removes a child from the Node by name.
     *
     * If there is more than one child of the given name, it removes the first
     * one that is found. For the base SceneNode class, children are always
     * enumerated in the order that they are added. However, this is not
     * guaranteed for subclasses of SceneNode. Hence it is very important
     * that names be unique.
     *
     * @param name  A string to identify the node.
     */
    void removeChildByName(const std::string name);
    
    /**
     * Removes all children from this Node.
     */
    virtual void removeAllChildren();
    
private:
#pragma mark -
#pragma mark Internal Helpers
    /**
     * Sets the parent node.
     *
     * The purpose of this pointer is to climb back up the scene graph tree.
     * No child asserts ownership of its parent.
     *
     * @param parent    A pointer to the parent node.
     */
    void setParent(SceneNode* parent) { _parent = parent; }

    /**
     * Sets the scene graph.
     *
     * The purpose of this pointer is to climb back up to the root of the
     * scene graph tree. No node asserts ownership of its scene.
     *
     * @param scene    A pointer to the scene graph.
     */
    void setScene(Scene3* scene) { _graph = scene; }

    /**
     * Recursively sets the scene graph for this node and all its children.
     *
     * The purpose of this pointer is to climb back up to the root of the
     * scene graph tree. No node asserts ownership of its scene.
     *
     * @param scene    A pointer to the scene graph.
     */
    void pushScene(Scene3* scene);

    /**
     * Updates the model and normal matrices
     *
     * This transform is defined by scaling, rotation, and positional
     * translation, in that order.
     */
    virtual void updateMatrices();
    
    // Copying is only allowed via shared pointer.
    CU_DISALLOW_COPY_AND_ASSIGN(SceneNode);
    
    friend class Scene3;

};

    }
}


#endif /* __CU_SCENE_3_NODE_H__ */
