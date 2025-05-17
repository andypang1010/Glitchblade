//
//  CUObjBatch.h
//  Cornell University Game Library (CUGL)
//
//  This module provides an implementation of Scene3Batch for drawing (and
//  batching) ObjNode objects. It is only designed for those types of nodes.
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
#ifndef __CU_OBJ_BATCH_H__
#define __CU_OBJ_BATCH_H__
#include <cugl/scene3/CUScene3Batch.h>
#include <cugl/scene3/CUObjNode.h>
#include <cugl/scene3/CUObjShader.h>

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
 * This class is a batch for drawing {@link ObjNode} objects.
 *
 * This class is only designed for {@link ObjNode} objects. Attempts to apply
 * it to any other {@link SceneNode} will be ignored. Objects are drawn in
 * the order they are appended to the batch.
 */
class ObjBatch : public Scene3Batch {
private:
    /**
     * The queue entry for an ObjBatch.
     *
     * We we add an element to the queue, it has to be added with its global
     * transform as that value is computed recursively from the scene graph.
     */
    class Entry {
    public:
        /** The node to draw */
        std::shared_ptr<ObjNode> node;
        /** The global transform */
        std::shared_ptr<Mat4> transform;
        
        /**
         * Creates an entry with the given node and transform
         *
         * @param node  The node to draw
         * @param mat   The global transform
         */
        Entry(const std::shared_ptr<ObjNode>& n, const Mat4& mat) {
            node = n;
            transform = std::make_shared<Mat4>(mat);
        }
    };
    
    /** The shader for this batch */
    std::shared_ptr<ObjShader> _shader;
    /** The batch queue */
    std::vector<Entry> _entries;
    
public:
    /** The key for this batch type */
    static const Uint32 BATCH_KEY = 1;
    
    /**
     * Creates a new degenerate batch on the stack.
     *
     * The batch has no shader, and therefore cannot draw anything.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    ObjBatch() : Scene3Batch() {}
    
    /**
     * Deletes this batch, disposing all resources
     */
    ~ObjBatch() { dispose(); }
    

    /**
     * Disposes all of the resources used by this batch.
     *
     * A disposed batch can be safely reinitialized. Any shader owned by this
     * batch will be released. It will be deleted if no other object owns it.
     */
    void dispose() override {
        _entries.clear();
        _shader = nullptr;
        Scene3Batch::dispose();
    }
    
    /**
     * Initializes a new OBJ batch with the default key and priority.
     *
     * This method can fail is the shader fails to compile.
     *
     * @return true if initialization was successful.
     */
    bool init() {
        return init(BATCH_KEY,0);
    }
    
    /**
     * Initializes a new OBJ batch with the given key and priority.
     *
     * This method can fail is the shader fails to compile.
     *
     * @param key       The batch key
     * @param priority  The batch priority
     *
     * @return true if initialization was successful.
     */
    bool init(Uint32 key, Uint32 priority) override;
    
    /**
     * Returns a newly allocated OBJ batch with the default key and priority.
     *
     * This method can fail is the shader fails to compile.
     *
     * @return a newly allocated OBJ batch with the default key and priority.
     */
    static std::shared_ptr<ObjBatch> alloc() {
        std::shared_ptr<ObjBatch> result = std::make_shared<ObjBatch>();
        return (result->init() ? result : nullptr);        
    }

    /**
     * Returns a newly allocated OBJ batch with the given key and priority.
     *
     * This method can fail is the shader fails to compile.
     *
     * @param key       The batch key
     * @param priority  The batch priority
     *
     * @return a newly allocated OBJ batch with the given key and priority.
     */
    static std::shared_ptr<ObjBatch> alloc(Uint32 key, Uint32 priority) {
        std::shared_ptr<ObjBatch> result = std::make_shared<ObjBatch>();
        return (result->init(key,priority) ? result : nullptr);
    }
    
    /**
     * Appends a scene node to this batch for drawing.
     *
     * The scene node will be ignored if it not an instance of {@link ObjNode}.
     * Nodes will be drawn in the order appended.
     *
     * @param node      The node to draw
     * @param transform The global transform
     */
    void append(const std::shared_ptr<SceneNode>& node, const Mat4& transform) override;
    
    /**
     * Draws all appended nodes.
     *
     * Nodes will be drawn in the order appended.
     *
     * @param camera    The camera to draw with
     */
    void flush(const std::shared_ptr<Camera>& camera) override;
    
    /**
     * Removes all appended nodes without drawing them.
     *
     * Calling {@link #flush} after this method will draw nothing.
     */
    void clear() override {
        _entries.clear();
    }

};
    
    }
}

#endif /* __CU_OBJ_BATCH_H__ */
