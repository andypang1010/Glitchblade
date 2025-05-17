//
//  CUScene3Pipeline.cpp
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
#include <cugl/scene3/CUScene3Pipeline.h>
#include <cugl/scene3/CUScene3.h>
#include <cugl/scene3/CUSceneNode3.h>
#include <cugl/scene3/CUObjBatch.h>
#include <cugl/scene3/CUBillboardBatch.h>
#include <cugl/scene3/CUParticleBatch.h>

using namespace cugl::scene3;
using namespace cugl;


#pragma mark Constructors
/**
 * Initializes a new empty pipeline.
 *
 * The initialized pipeline starts with no batches. They must be attached
 * via {@link #attach}.
 *
 * @return true if initialization was successful.
 */
bool Scene3Pipeline::init() {
    return true;
}

/**
 * Initializes a new default pipeline.
 *
 * The pipeline starts with the all the default batches: {@link ObjBatch},
 * {@link BillboardBatch}, and {@link ParticleBatch}. Additional batches
 * may be added via {@link #attach}.
 *
 * @return true if initialization was successful.
 */
bool Scene3Pipeline::initWithStandard() {
    
    // Add the standard batches
    if (!attach(ObjBatch::alloc())) {
        return false;
    }

    if (!attach(BillboardBatch::alloc())) {
        return false;
    }

    if (!attach(ParticleBatch::alloc())) {
        return false;
    }

    return true;
}

/**
 * Disposes all of the resources used by this pipeline.
 *
 * A disposed pipeline can be safely reinitialized. Any batches owned by
 * this pipeline will be released. They will be deleted if no other object
 * owns them.
 */
void Scene3Pipeline::dispose() {
    clear();
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
bool Scene3Pipeline::attach(const std::shared_ptr<Scene3Batch>& batch) {
    return attach(batch,batch->getPriority());
}

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
bool Scene3Pipeline::attach(const std::shared_ptr<Scene3Batch>& batch, Uint32 priority) {
    CUEnum key = batch->getBatchKey();
    auto it = _batches.find(key);
    if (it != _batches.end()) {
        return false;
    }
    _batches[key] = batch;
    _priorities[key] = priority;
    _keys.push_back(key);

    // Resort the keys
    std::sort(_keys.begin(),_keys.end(),
              [this](int m,int n)-> bool { return _priorities[m] < _priorities[n]; });
    return true;

}

/**
 * Sets the priority for the given batch key.
 *
 * This method will do nothing if key does not reference an attached batch.
 *
 * @param key       The batch key
 * @param priority  The batch priority.
 */
void Scene3Pipeline::setPriority(CUEnum queue, Uint32 priority) {
    auto it = _priorities.find(queue);
    if (it == _priorities.end()) {
        return;
    }
    it->second = priority;
    // Resort the keys
    std::sort(_keys.begin(),_keys.end(),
              [this](int m,int n)-> bool { return _priorities[m] < _priorities[n]; });
}

/**
 * Returns the priority for the given batch key.
 *
 * This method will return -1 if key does not reference an attached batch.
 *
 * @param key       The batch key
 *
 * @return the priority for the given batch key.
 */
Sint32 Scene3Pipeline::getPriority(CUEnum queue) const {
    auto it = _priorities.find(queue);
    if (it == _priorities.end()) {
        return -1;
    }
    return it->second;
}

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
void Scene3Pipeline::append(const std::shared_ptr<SceneNode>& node, const Mat4& transform) {
    CUEnum key = node->getBatchKey();
    auto it = _batches.find(key);
    if (it != _batches.end()) {
        it->second->append(node, transform);
    }
}

/**
 * Draws all appended nodes.
 *
 * Drawing will be processed by batch, according to priority. Drawing order
 * for each batch depends upon the specification for that batch type.
 *
 * @param camera    The camera to draw with
 */
void Scene3Pipeline::flush(const std::shared_ptr<Camera>& camera) {
    for(auto it = _keys.begin(); it != _keys.end(); ++it) {
        auto jt = _batches.find(*it);
        jt->second->flush(camera);
    }
}

/**
 * Removes all appended nodes without drawing them.
 *
 * Calling {@link #flush} after this method will draw nothing.
 */
void Scene3Pipeline::clear() {
    for(auto it = _batches.begin(); it != _batches.end(); ++it) {
        it->second->clear();
    }
}
