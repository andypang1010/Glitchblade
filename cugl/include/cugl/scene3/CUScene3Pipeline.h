//
//  CUScene3Pipeline.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a pipeline for rendering 3d scene graphs. Eeach node
//  type in a 3d scene graph is likely to have its own batch/shader. While
//  occasional shader switching is okay, switching for each object is too
//  expensive. The solution is split the scene graph into multiple passes, one
//  for each batch/shader type. The purpose of this pipeline is to manage and
//  coordinate these different batches.
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
#ifndef __CU_SCENE3_PIPELINE_H__
#define __CU_SCENE3_PIPELINE_H__
#include <cugl/graphics/cu_graphics.h>
#include <cugl/core/math/CUMathBase.h>
#include <unordered_map>
#include <deque>


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

// Forward pointer to nodes
class SceneNode;
class Scene3Batch;
    
/**
 * This class manages the batches used to process a 3d scene graph.
 *
 * Unlike 2d scene graphs, each node type in a 3d scene graph is likely to
 * have its own batch/shader. While occasional shader switching is okay,
 * switching for each object is too expensive. The solution is split the scene
 * graph into multiple passes, one for each batch/shader type. As 3d scene
 * graphs leverage depth testing, this is generally okay.
 *
 * A render queue does not use the standard begin()/end() pattern that we have
 * in {@link graphics::SpriteBatch} and other general purpose shaders. That is
 * because a pipeline is a collection of batches, each with their own queue.
 * To draw with the pipeline, we populate the individual queues with the method
 * {@link #append}. We then render these queue contents with {@link #flush}.
 *
 * The piplein has predefined shaders for the built-in 3d scene graph types
 * like {@link ObjNode} and {@link ParticleNode}. However, it is extensible
 * enough to include extra shaders as well. You should create a fresh CUEnum
 * (unsigned int) for your new shader before adding it via {@link #attach}.
 * Make sure that this enum does not conflict with existing values.
 *
 * All shaders (including the built-in ones) have a priority. Shaders are
 * applied in order of priority from lowest value to highest. Priorities can
 * be reassigned at any time with {@link #setPriority}. As a general rule,
 * particles and shaders with transparency effects should be processed last.
 */
class Scene3Pipeline {
private:
    /** The attached drawing batches */
    std::unordered_map<CUEnum, std::shared_ptr<Scene3Batch>> _batches;
    /** The batch priorities */
    std::unordered_map<CUEnum, Uint32> _priorities;
    /** The batch keys */
    std::vector<CUEnum> _keys;

public:
#pragma mark Constructors
    /**
     * Creates a new degenerate pipeline on the stack.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    Scene3Pipeline() {}
    
    /**
     * Deletes this pipeline, disposing all resources
     */
    ~Scene3Pipeline() { dispose(); }
    
    /**
     * Disposes all of the resources used by this pipeline.
     *
     * A disposed pipeline can be safely reinitialized. Any batches owned by
     * this pipeline will be released. They will be deleted if no other object
     * owns them.
     */
    void dispose();
    
    /**
     * Initializes a new empty pipeline.
     *
     * The initialized pipeline starts with no batches. They must be attached
     * via {@link #attach}.
     *
     * @return true if initialization was successful.
     */
    bool init();

    /**
     * Initializes a new default pipeline.
     *
     * The pipeline starts with the all the default batches: {@link ObjBatch},
     * {@link BillboardBatch}, and {@link ParticleBatch}. Additional batches
     * may be added via {@link #attach}.
     *
     * @return true if initialization was successful.
     */
    bool initWithStandard();

    /**
     * Returns a newly allocated empty pipeline.
     *
     * The initialized pipeline starts with no batches. They must be attached
     * via {@link #attach}.
     *
     * @return a newly allocated empty pipeline.
     */
    static std::shared_ptr<Scene3Pipeline> alloc() {
        std::shared_ptr<Scene3Pipeline> result = std::make_shared<Scene3Pipeline>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated default pipeline.
     *
     * The pipeline starts with the all the default batches: {@link ObjBatch},
     * {@link BillboardBatch}, and {@link ParticleBatch}. Additional batches
     * may be added via {@link #attach}.
     *
     * @return a newly allocated default pipeline.
     */
    static std::shared_ptr<Scene3Pipeline> allocWithStandard() {
        std::shared_ptr<Scene3Pipeline> result = std::make_shared<Scene3Pipeline>();
        return (result->initWithStandard() ? result : nullptr);
    }
    
#pragma mark Batch Management
    /**
     * Attaches the batch to this pipeline.
     *
     * The batch will use the values {@link Scene3Batch#getBatchKey} and
     * {@link Scene3Batch#getPriority} to set the batch key and priority,
     * respectively. This method will fail if the pipeline already has a
     * batch with the specified key.
     *
     * @param batch The batch to attach to this pipeline.
     */
    bool attach(const std::shared_ptr<Scene3Batch>& batch);

    /**
     * Attaches the batch to this pipeline with the given priority.
     *
     * The batch will use the value {@link Scene3Batch#getBatchKey} to set
     * batch key. This method will fail if the pipeline already has a batch
     * with the specified key.
     *
     * @param batch     The batch to attach to this pipeline.
     * @param priority  The batch priority.
     */
    bool attach(const std::shared_ptr<Scene3Batch>& batch, Uint32 priority);

    /**
     * Sets the priority for the given batch key.
     *
     * This method will do nothing if key does not reference an attached batch.
     *
     * @param key       The batch key
     * @param priority  The batch priority.
     */
    void setPriority(CUEnum key, Uint32 priority);
    
    /**
     * Returns the priority for the given batch key.
     *
     * This method will return -1 if key does not reference an attached batch.
     *
     * @param key       The batch key
     *
     * @return the priority for the given batch key.
     */
    Sint32 getPriority(CUEnum key) const;

    /**
     * Adds the node and transform to this pipeline for drawing.
     *
     * No drawing will occur until {@link #flush} is called. Drawing will
     * be processed by batch, according to priority. Drawing order for each
     * batch depends upon the specification for that batch type.
     *
     * @param node      The node to draw
     * @param transform The global transform for the node
     */
    void append(const std::shared_ptr<SceneNode>& node, const Mat4& transform);
    
    /**
     * Draws all appended nodes.
     *
     * Drawing will be processed by batch, according to priority. Drawing order
     * for each batch depends upon the specification for that batch type.
     *
     * @param camera    The camera to draw with
     */
    void flush(const std::shared_ptr<Camera>& camera);

    /**
     * Removes all appended nodes without drawing them.
     *
     * Calling {@link #flush} after this method will draw nothing.
     */
    void clear();

    
};
    }
}

#endif /* __CU_SCENE3_PIPELINE_H__ */
