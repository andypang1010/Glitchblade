//
//  CUScene3Batch.hpp
//  Cornell University Game Library (CUGL)
//
//  This module provides an abstract class for batching together 3d scene graph
//  nodes for drawing. This is an abstract class for polymorphism. It should
//  never be instantied directly.
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
#ifndef __CU_SCENE3_BATCH_H__
#define __CU_SCENE3_BATCH_H__
#include <cugl/core/math/cu_math.h>
#include <memory>

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

// Forward reference to a scene node
class SceneNode;
    
/**
 * This class is used to batch together {@link SceneNode} objects for drawing.
 *
 * {@link Scene3} is different from {@link scene2::Scene2} in that there is no
 * single pipeline for drawing all node types. Each node type has its own shader
 * with its own uniforms. To minimize shader switching (which is expensive), we
 * use batches to gather together all nodes of a single type before drawing.
 *
 * This batching allows us to do any necessary pre-drawing computation such as
 * sorting nodes by their z-depth. This is important for transparency affects,
 * as depth buffers are incompatible with alpha blending. However, not all
 * batches sort. This optimization is handled on a type-by-type basis.
 *
 * This is an abstract class, and should not be instantiated by itself.
 * Implementing subclasses must define {@link #append}, {@link #flush} and
 * {@link #clear}.
*/
class Scene3Batch {
protected:
    /** The batch key for identifying the batch */
    Uint32 _batchkey;
    /** The default priority for this batch */
    Uint32 _priority;
    
public:
    /**
     * Creates a new degenerate batch on the stack.
     *
     * The batch has no shader, and therefore cannot draw anything.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    Scene3Batch() : _batchkey(0), _priority(0) {}

    /**
     * Disposes all of the resources used by this batch.
     *
     * A disposed batch can be safely reinitialized. Any shader owned by this
     * batch will be released. It will be deleted if no other object owns it.
     */
    virtual void dispose() {
        _batchkey = 0;
        _priority = -1;
    }
    
    /**
     * Initializes a batch with the given key and priority.
     *
     * Key value 0 is reserved and should not be used.
     *
     * @param key       The batch key
     * @param priority  The batch priority
     *
     * @return true if initialization was successful
     */
    virtual bool init(Uint32 key, Uint32 priority) {
        _batchkey = key;
        _priority = priority;
        return true;
    }
    
    /**
     * Returns the batch key for this batch.
     *
     * The batch key is used by the {@link Scene3Pipeline} to distinguish
     * individual batches from each other. These keys may not be reassigned
     * once the batch is instantiated.
     *
     * Key value 0 is reserved and should not be used.
     *
     * @return the batch key for this batch.
     */
    Uint32 getBatchKey() const { return _batchkey; }
    
    /**
     * Returns the default priority for this batch.
     *
     * The priority is used by the {@link Scene3Pipeline} to determine drawing
     * order across multiple batches.
     *
     * @return the default priority for this batch.
     */
    Uint32 getPriority() const { return _priority; }

    /**
     * Appends the given node and transform for drawing.
     *
     * Drawing order within a batch depends on the implementation. Some batches
     * draw nodes in the order that they are appended. Some sort the nodes
     * before drawing, in order to manage transparency or color blending.
     *
     * @param node      The node to draw
     * @param transform The global transform
     */
    virtual void append(const std::shared_ptr<SceneNode>& node, const Mat4& transform) = 0;
    
    /**
     * Draws all appended nodes.
     *
     * Drawing order of the nodes is implementation dependent.
     *
     * @param camera    The camera to draw with
     */
    virtual void flush(const std::shared_ptr<Camera>& camera) = 0;

    /**
     * Removes all appended nodes without drawing them.
     *
     * Calling {@link #flush} after this method will draw nothing.
     */
    virtual void clear() = 0;

};
    
    }
}

#endif /* __CU_SCENE3_BATCH_H__ */
