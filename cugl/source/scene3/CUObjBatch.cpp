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
#include <cugl/scene3/CUObjBatch.h>

using namespace cugl;
using namespace cugl::scene3;

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
bool ObjBatch::init(Uint32 key, Uint32 priority) {
    if (!Scene3Batch::init(key,priority)) {
        return false;
    }
    _shader = ObjShader::alloc();
    return _shader != nullptr;
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
void ObjBatch::append(const std::shared_ptr<SceneNode>& node, const Mat4& transform) {
    std::shared_ptr<ObjNode> obj = std::dynamic_pointer_cast<ObjNode>(node);
    if (obj == nullptr || obj->getModel() == nullptr) {
        return;
    }
    _entries.emplace_back(obj,transform);
}

/**
 * Draws all appended nodes.
 *
 * Nodes will be drawn in the order appended.
 *
 * @param camera    The camera to draw with
 */
void ObjBatch::flush(const std::shared_ptr<Camera>& camera) {
    _shader->bind();
    _shader->setPerspective(camera->getCombined());
    _shader->setUniformVec3("uLightPos",camera->getPosition());
    _shader->enableCulling(true);
    _shader->enableDepthTest(true);
    _shader->enableDepthWrite(true);

    Mat4 normalmat;
    for(auto it = _entries.begin(); it != _entries.end(); ++it) {
        Mat4::invert(*(it->transform), &normalmat);
        Mat4::transpose(normalmat, &normalmat);
        _shader->setModelMatrix(*(it->transform));
        _shader->setNormalMatrix(normalmat);
        it->node->getModel()->draw(_shader,it->node->getMaterial());
    }
    
    _shader->enableCulling(false);
    _shader->enableDepthTest(false);
    _shader->unbind();
    _entries.clear();
}
