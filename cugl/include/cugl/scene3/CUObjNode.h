//
//  CUObjNode.h
//  Cornell University Game Library (CUGL)
//
//  This module represents a node for an OBJ file in a 3d scene graph. We
//  separate this from the ObjModel, as we want to make a distinction between
//  the static asset, and an instance of the asset in the game.
//
//  This class has our standard shared-pointer architecture.
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
#ifndef __CU_OBJ_NODE_H__
#define __CU_OBJ_NODE_H__
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cugl/graphics/cu_graphics.h>
#include <cugl/scene3/CUSceneNode3.h>
#include <cugl/scene3/CUObjModel.h>
#include <cugl/scene3/CUMaterial.h>
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

/**
 * This class represents an OBJ model in a 3d scene graph.
 *
 * OBJ models are assets, like {@link graphics::Texture} objects. By themselves, 
 * they do not have enough information to display them on the screen. They need
 * a position and orientation. That is the purpose of this class. It combines an
 * OBJ model with position/orientation information so that it can be drawn on
 * the screen.
 *
 * If you wish to break up an OBJ model into multiple nodes (for the purpose of
 * animation), use {@link ObjModel#getSubModel} to extract the individual
 * components before making the nodes.
 */
class ObjNode : public SceneNode {
public:
    /** The model associated with this node */
    std::shared_ptr<ObjModel> _model;
    /** A material to apply as a default */
    std::shared_ptr<Material> _material;

public:
#pragma mark Constructors
    /**
     * Creates an uninitialized node.
     *
     * You must initialize this model before use.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a model on the
     * heap, use one of the static constructors instead.
     */
    ObjNode();
    
    /**
     * Deletes this node, disposing all resources
     */
    ~ObjNode() { dispose(); }
    
    /**
     * Disposes all of the resources used by this node.
     *
     * A disposed node can be safely reinitialized. Any children owned by this
     * node will be released. They will be deleted if no other object owns them.
     *
     * It is unsafe to call this on a node that is still currently inside of
     * a scene graph.
     */
    virtual void dispose() override;
    
    /**
     * Initializes this node with the given model
     *
     * This node will keep a reference to the given model, but it will have its
     * own model matrix. Therefore it is possible for multiple nodes to share
     * the same model.
     *
     * @param model The OBJ model
     *
     * @return true if initialization was successful.
     */
    bool initWithModel(const std::shared_ptr<ObjModel>& model);

    /**
     * Initializes a node with the given JSON specification.
     *
     * This initializer is designed to receive the "data" object from the
     * JSON passed to {@link Scene3Loader}. This JSON format supports all
     * of the attribute values of its parent class. In addition, it supports
     * the following additional attribute:
     *
     *      "model":     A string with the name of a previously loaded OBJ asset
     *
     * While this attribute is technically optional, not specifying means that
     * nothing is drawn.
     *
     * @param manager   The asset manager handling this asset
     * @param json      The JSON object specifying the node
     *
     * @return true if initialization was successful.
     */
    bool initWithData(const AssetManager* manager,
                      const std::shared_ptr<JsonValue>& json) override;

    /**
     * Performs a shallow copy of this node into dst.
     *
     * The nodes will share the OBJ models, but each have their own model
     * matrix.
     *
     * No children from this model are copied, and no children of dst are
     * modified. In addition, the parents of both node are unchanged. However,
     * all other attributes of this node are copied.
     *
     * @param dst   The node to copy into
     *
     * @return a reference to dst for chaining.
     */
    virtual std::shared_ptr<SceneNode> copy(const std::shared_ptr<SceneNode>& dst) const override;

    /**
     * Returns a shallow copy of this node.
     *
     * The nodes will share the OBJ models, but each have their own model
     * matrix.
     *
     * No children from this model are copied, and no children of dst are
     * modified. In addition, the parents of both node are unchanged. However,
     * all other attributes of this node are copied.
     *
     * @return a shallow copy of this node.
     */
    std::shared_ptr<SceneNode> clone() const;
    
    /**
     * Returns a newly allocated node with the given model
     *
     * This node will keep a reference to the given model, but it will have its
     * own model matrix. Therefore it is possible for multiple nodes to share
     * the same model.
     *
     * @param model The OBJ model
     *
     * @return a newly allocated node with the given model
     */
    static std::shared_ptr<ObjNode> allocWithModel(const std::shared_ptr<ObjModel>& model) {
        std::shared_ptr<ObjNode> result = std::make_shared<ObjNode>();
        return (result->initWithModel(model) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated node with the given JSON specification.
     *
     * This initializer is designed to receive the "data" object from the
     * JSON passed to {@link Scene3Loader}. This JSON format supports all
     * of the attribute values of its parent class. In addition, it supports
     * the following additional attribute:
     *
     *      "model":     A string with the name of a previously loaded OBJ asset
     *
     * While this attribute is technically optional, not specifying means that
     * nothing is drawn.
     *
     * @param manager   The asset manager handling this asset
     * @param json      The JSON object specifying the node
     *
     * @return a newly allocated node with the given JSON specification.
     */
    static std::shared_ptr<ObjNode> allocWithData(const AssetManager* manager,
                                                  const std::shared_ptr<JsonValue>& json) {
        std::shared_ptr<ObjNode> result = std::make_shared<ObjNode>();
        return (result->initWithData(manager,json) ? result : nullptr);
    }
    
#pragma mark Mesh Access
    /**
     * Returns the model associated with this node.
     *
     * If this value is nullptr, nothing will be drawn for this node.
     *
     * @return the model associated with this node.
     */
    std::shared_ptr<ObjModel> getModel() const {
        return _model;
    }
    
    /**
     * Sets the model associated with this node.
     *
     * If this value is nullptr, nothing will be drawn for this node.
     *
     * @param model The model associated with this node.
     */
    void setModel(const std::shared_ptr<ObjModel>& model) {
        _model = model;
    }
    
    /**
     * Returns the default material for this model reference.
     *
     * This material will be applied to any surface that does not
     * already have a material. Otherwise, it is ignored.
     *
     * @return the default material for this model reference.
     */
    std::shared_ptr<Material> getMaterial() const {
        return _material;
    }

    /**
     * Sets the default material for this model reference.
     * <p>
     * This material will be applied to any surface that does not
     * already have a material. Otherwise, it is ignored.
     *
     * @param material    The default material for this model reference.
     */
    void setMaterial(const std::shared_ptr<Material>& material) {
        _material = material;
    }
        
};
    }
}

#endif /* __CU_OBJ_NODE_H__ */

