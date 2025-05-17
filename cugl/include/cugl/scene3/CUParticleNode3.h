//
//  CUParticle3Node.h
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
#ifndef __CU_PARTICLE3_NODE_H__
#define __CU_PARTICLE3_NODE_H__
#include <cugl/graphics/CUParticleSystem.h>
#include <cugl/scene3/CUSceneNode3.h>

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
 * This class represents an particle system in a 3d scene graph.
 *
 * Particle systems contain the logic for creating particles, as well as the
 * underlying mesh. However, they do not have a position/orientation, or even
 * a texture. That information is factored out so that we can reuse particle
 * systems, as they are fairly heavy weight.
 *
 * Depending on the choice of texture, systems do support limited animation.
 * However, that animation is manage through the {@link graphics::ParticleInstance}
 * objects. It is not managed in this node.
 */
class ParticleNode : public SceneNode {
private:
    /** The particle system associated with this node */
    std::shared_ptr<graphics::ParticleSystem> _system;
    /** The particle texture (optional) */
    std::shared_ptr<graphics::Texture> _texture;


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
    ParticleNode();
    
    /**
     * Deletes this node, disposing all resources
     */
    ~ParticleNode() { dispose(); }
    
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
    bool initWithSystem(const std::shared_ptr<graphics::ParticleSystem>& system);

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
    bool initWithData(const AssetManager* manager,
                      const std::shared_ptr<JsonValue>& json) override;

    /**
     * Performs a shallow copy of this node into dst.
     *
     * The nodes will initially share the same particle system and texture.
     * However, the position and orientation are copied by value.
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
     * The nodes will initially share the same particle system and texture.
     * However, the position and orientation are copied by value.
     *
     * No children from this model are copied, and no children of dst are
     * modified. In addition, the parents of both node are unchanged. However,
     * all other attributes of this node are copied.
     *
     * @return a shallow copy of this node.
     */
    std::shared_ptr<SceneNode> clone() const;
    
    /**
     * Returns a newly allocated node with the given particle system
     *
     * This node will keep a reference to particle system, but it will have its
     * own position, orientation, and texture. Therefore it is possible for
     * multiple nodes to share the same particle system.
     *
     * @param system    The particle system
     *
     * @return a newly allocated node with the given particle system
     */
    static std::shared_ptr<ParticleNode> allocWithSystem(const std::shared_ptr<graphics::ParticleSystem>& system) {
        std::shared_ptr<ParticleNode> result = std::make_shared<ParticleNode>();
        return (result->initWithSystem(system) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated node with the given JSON specification.
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
     * @return a newly allocated node with the given JSON specification.
     */
    static std::shared_ptr<ParticleNode> allocWithData(const AssetManager* manager,
                                                       const std::shared_ptr<JsonValue>& json) {
        std::shared_ptr<ParticleNode> result = std::make_shared<ParticleNode>();
        return (result->initWithData(manager,json) ? result : nullptr);
    }
    
#pragma mark Attributes
    /**
     * Returns the particle system associated with this node.
     *
     * If this value is nullptr, nothing will be drawn for this node. If this
     * value was initialized via JSON, you must provide this system with a
     * user-defined update and allocation function.
     *
     * @return the particle system associated with this node.
     */
    std::shared_ptr<graphics::ParticleSystem> getParticleSystem() const {
        return _system;
    }
    
    /**
     * Sets the particle system associated with this node.
     *
     * If this value is nullptr, nothing will be drawn for this node. If this
     * value was initialized via JSON, you must provide this system with a
     * user-defined update and allocation function.
     *
     * @param system    The particle system mesh associated with this node.
     */
    void setParticleSystem(const std::shared_ptr<graphics::ParticleSystem>& system) {
        _system = system;
    }

    /**
     * Returns the texture associated with this particle system.
     *
     * If this value is nullptr, the particles will be rendered as a single color.
     *
     * @return the texture associated with this particle system.
     */
    std::shared_ptr<graphics::Texture> getTexture() const { return _texture; }

    /**
     * Sets the texture associated with this particle system.
     *
     * If this value is nullptr, the particles will be rendered as a single color.
     *
     * @param value The texture associated with this particle system.
     */
    void setTexture(const std::shared_ptr<graphics::Texture>& value) {
        _texture = value;
    }
    
};
    }
}


#endif /* __CU_PARTICLE3_NODE_H__ */
