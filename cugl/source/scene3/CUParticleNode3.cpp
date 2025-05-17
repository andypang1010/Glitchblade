//
//  CUParticleNode.cpp
//  Cornell University Game Library (CUGL)
//
//  This module is a node representing a 3d particle system in our scene graph.
//  It is adapted from the OpenGL tutorial series:
//
//     http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/particles-instancing/
//
//  Note that our particle system class is applicable to both 2d and 3d scenes.
//  This implementation contains the necessary information specific to a 3d
//  particle scene. That is why the particle system is separated out as its own
//  asset.
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
#include <cugl/scene3/CUParticleNode3.h>
#include <cugl/scene3/CUScene3Pipeline.h>
#include <cugl/scene3/CUParticleBatch.h>
#include <cugl/graphics/CUParticleSystem.h>

using namespace cugl::scene3;
using namespace cugl::graphics;
using namespace cugl;
using namespace std;

#pragma mark Constructors
/**
 * Creates an uninitialized node.
 *
 * You must initialize this node before use.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a node on the
 * heap, use one of the static constructors instead.
 */
ParticleNode::ParticleNode() : SceneNode() {
    _name = "";
    _classname = "ParticleNode3";
    _batchkey = ParticleBatch::BATCH_KEY;
}


/**
 * Initializes this node with the given particle system
 *
 * This node will keep a reference to particle system, but it will have its
 * own position, orientation, and texture. Therefore it is possible for
 * multiple nodes to share the same particle system.
 *
 * @param system    The particle system
 *
 * @return true if initialization was successful.
 */
bool ParticleNode::initWithSystem(const std::shared_ptr<ParticleSystem>& system) {
    if (!SceneNode::init()) {
        return false;
    }
    _system = system;
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
 *      "system":   A string with the name of a previously loaded particle system
 *      "texture":  A string with the name of a previously loaded texture asset
 *
 * All attributes are optional. However, not specifying the system means
 * that nothing is drawn. In addition, remember that JSON can provide the
 * user-defined update and allocation functions for the particle system.
 * Hence those will need to be defined for any particles to be emitted.
 *
 * @param manager   The asset manager handling this asset
 * @param json      The JSON object specifying the node
 *
 * @return true if initialization was successful.
 */
bool ParticleNode::initWithData(const AssetManager* assets,
                                 const std::shared_ptr<JsonValue>& json) {
    if (!SceneNode::initWithData(assets, json)) {
        return false;
    }
    
    if (json->has("system")) {
        _system = assets->get<ParticleSystem>(json->getString("system"));
    }

    if (json->has("texture")) {
        _texture = assets->get<Texture>(json->getString("texture"));
    }

    return _system != nullptr;
}

/**
 * Disposes all of the resources used by this node.
 *
 * A disposed node can be safely reinitialized. Any children owned by this
 * node will be released. They will be deleted if no other object owns them.
 *
 * It is unsafe to call this on a node that is still currently inside of
 * a scene graph.
 */
void ParticleNode::dispose() {
    _name = "";
    _system = nullptr;
    _texture = nullptr;
    SceneNode::dispose();
}

/**
 * Performs a shallow copy of this node into dst.
 *
 * The nodes will initially share the same sprite mesh, texture, and
 * gradient. However, the position and orientation are copied by value.
 *
 * No children from this model are copied, and no children of dst are
 * modified. In addition, the parents of both node are unchanged. However,
 * all other attributes of this node are copied.
 *
 * @param dst   The node to copy into
 *
 * @return a reference to dst for chaining.
 */
std::shared_ptr<SceneNode> ParticleNode::copy(const std::shared_ptr<SceneNode>& dst) const {
    std::shared_ptr<ParticleNode> obj = std::dynamic_pointer_cast<ParticleNode>(dst);
    if (dst == nullptr || obj == nullptr) {
        return dst;
    }
    
    SceneNode::copy(dst);
    obj->_system = _system;
    obj->_texture = _texture;
    return dst;
}

/**
 * Returns a shallow copy of this node.
 *
 * The nodes will initially share the same sprite mesh, texture, and
 * gradient. However, the position and orientation are copied by value.
 *
 * No children from this model are copied, and no children of dst are
 * modified. In addition, the parents of both node are unchanged. However,
 * all other attributes of this node are copied.
 *
 * @param dst   The node to copy into
 *
 * @return a shallow copy of this node.
 */
std::shared_ptr<SceneNode> ParticleNode::clone() const {
    std::shared_ptr<ParticleNode> result = ParticleNode::allocWithSystem(_system);
    SceneNode::copy(result);
    return result;
}
