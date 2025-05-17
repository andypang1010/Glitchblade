//
//  CUObjNode.cpp
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
#include <cugl/scene3/CUObjNode.h>
#include <cugl/scene3/CUScene3Pipeline.h>
#include <cugl/scene3/CUObjBatch.h>
#include <cugl/scene3/CUObjModel.h>

using namespace cugl::scene3;
using namespace cugl;
using namespace std;

/**
 * Creates an uninitialized node.
 *
 * You must initialize this node before use.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a node on the
 * heap, use one of the static constructors instead.
 */
ObjNode::ObjNode() : SceneNode() {
    _name = "";
    _classname = "ObjNode";
    _batchkey = ObjBatch::BATCH_KEY;
}


/**
 * Initializes this model with the data provided.
 *
 * This object will be a partial copy of the given model. It will share the
 * same meshes, but will have its own ModelView matrix. It also will not
 * have any of the children of the original model.
 *
 * @param model The model to copy
 *
 * @return true if initialization was successful.
 */
bool ObjNode::initWithModel(const std::shared_ptr<ObjModel>& model) {
    if (!SceneNode::init()) {
        return false;
    }
    _name = model->getName();
    _model = model;
    return true;
}

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
bool ObjNode::initWithData(const AssetManager* assets,
                           const std::shared_ptr<JsonValue>& json) {
    if (!SceneNode::initWithData(assets, json)) {
        return false;
    }
    
    if (json->has("model")) {
        _model = assets->get<ObjModel>(json->getString("model"));
    }

    if (json->has("material")) {
        _material = assets->get<Material>(json->getString("material"));
    }

    return true;
}

/**
 * Disposes all of the resources used by this model.
 *
 * A disposed model can be safely reinitialized. Any children owned by this
 * node will be released. They will be deleted if no other object owns them.
 *
 * It is unsafe to call this on a Node that is still currently inside of
 * a scene graph.
 */
void ObjNode::dispose() {
    _name = "";
    _model = nullptr;
    SceneNode::dispose();
}

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
std::shared_ptr<SceneNode> ObjNode::copy(const std::shared_ptr<SceneNode>& dst) const {
    std::shared_ptr<ObjNode> obj = std::dynamic_pointer_cast<ObjNode>(dst);
    if (dst == nullptr || obj == nullptr) {
        return dst;
    }
    
    SceneNode::copy(dst);
    obj->_model = _model;
    return dst;
}

/**
 * Returns a shallow copy of this node.
 *
 * The copy will share a reference to all of the meshes in this model.
 *
 * @return a shallow copy of this node.
 */
std::shared_ptr<SceneNode> ObjNode::clone() const {
    std::shared_ptr<ObjNode> result = ObjNode::allocWithModel(_model);
    SceneNode::copy(result);
    return result;
}
