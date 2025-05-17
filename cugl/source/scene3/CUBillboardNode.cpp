//
//  CUBillboardNode.cpp
//  Cornell University Game Library (CUGL)
//
//  This module an implementation of a billboard in 3d space. It is adapted
//  from the OpenGL tutorial series:
//
//     http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/billboards/
//
//  Some changes have been made from that tutorial. We do not fix the size of
//  the billboard, so it is possible to zoom in and out. In addition, the
//  billboard is always positioned at the origin, but it can be translated
//  (and rotated or scaled) by a model matrix.
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
#include <cugl/scene3/CUBillboardNode.h>
#include <cugl/scene3/CUScene3Pipeline.h>
#include <cugl/scene3/CUBillboardBatch.h>
#include <cugl/graphics/CUSpriteMesh.h>
#include <cugl/graphics/CUTexture.h>
#include <cugl/graphics/CUGradient.h>

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
BillboardNode::BillboardNode() : SceneNode(),
_size(1),
_rows(1),
_cols(1),
_frame(0) {
    _name = "";
    _classname = "BillboardNode";
    _batchkey = BillboardBatch::BATCH_KEY;
}

/**
 * Initializes this node with the given sprite mesh
 *
 * This node will keep a reference to sprite mesh, but it will have its own
 * position, orientation, texture, and gradient. Therefore it is possible
 * for multiple nodes to share the same sprite mesh.
 *
 * The billboard will assume that the sprite mesh is to represent a static
 * image. Therefore, there is only one animation frame in the texture
 * (e.g. rows and cols are 1).
 *
 * @param mesh  The sprite mesh
 *
 * @return true if initialization was successful.
 */
bool BillboardNode::initWithSpriteMesh(const std::shared_ptr<SpriteMesh>& mesh) {
    if (!SceneNode::init()) {
        return false;
    }
    _mesh = mesh;
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
 *      "mesh":     A string with the name of a previously loaded sprite mesh asset
 *      "texture":  A string with the name of a previously loaded texture asset
 *      "gradient": A JSON object defining a gradient. See {@link Gradient}.
 *      "span":     The number of frames in the texture sprite sheet
 *      "rows":     An int specifying the number of rows in the sprite sheet
 *      "cols":     An int specifying the number of columns in the sprite sheet
 *      "frame":    The initial starting frame.
 *
 * All attributes are optional. However, not specifying the mesh means that
 * nothing is drawn.
 *
 * @param manager   The asset manager handling this asset
 * @param json      The JSON object specifying the node
 *
 * @return true if initialization was successful.
 */
bool BillboardNode::initWithData(const AssetManager* assets,
                                 const std::shared_ptr<JsonValue>& json) {
    if (!SceneNode::initWithData(assets, json)) {
        return false;
    }
    
    if (json->has("mesh")) {
        _mesh = assets->get<SpriteMesh>(json->getString("mesh"));
    }

    if (json->has("texture")) {
        _texture = assets->get<Texture>(json->getString("texture"));
    }

    if (json->has("gradient")) {
        auto child = json->get("gradient");
        if (child->isString()) {
            _gradient = assets->get<Gradient>(child->asString());
        } else {
            _gradient = Gradient::allocWithData(json->get("gradient"));
        }
    }

    if (json->has("span")) {
        _size = json->getInt("span",1);
        _cols = json->getInt("cols",_size);
        _rows = json->getInt("rows",_size/_cols + (_size % _cols == 0 ? 0 : 1));
    } else {
        _cols = json->getInt("cols",1);
        _rows = json->getInt("rows",1);
        _size = _cols*_rows;
    }
    _frame = json->getInt("frame",0);
    computeSpriteBounds();
    return _mesh != nullptr;
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
void BillboardNode::dispose() {
    _name = "";
    _mesh = nullptr;
    _texture = nullptr;
    _gradient = nullptr;
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
std::shared_ptr<SceneNode> BillboardNode::copy(const std::shared_ptr<SceneNode>& dst) const {
    std::shared_ptr<BillboardNode> obj = std::dynamic_pointer_cast<BillboardNode>(dst);
    if (dst == nullptr || obj == nullptr) {
        return dst;
    }
    
    SceneNode::copy(dst);
    obj->_mesh = _mesh;
    obj->_texture = _texture;
    obj->_gradient = _gradient;
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
std::shared_ptr<SceneNode> BillboardNode::clone() const {
    std::shared_ptr<BillboardNode> result = BillboardNode::allocWithSpriteMesh(_mesh);
    SceneNode::copy(result);
    return result;
}

#pragma mark -
#pragma mark Animation

/**
 * Computes the sprite bounds for the initial frame in the sprite sheet
 */
void BillboardNode::computeSpriteBounds() {
    _bounds.size.width  = 1.0f/_cols;
    _bounds.size.height = 1.0f/_rows;
    _bounds.origin.x = (_frame % _cols)*_bounds.size.width;
    _bounds.origin.y = (_frame / _cols)*_bounds.size.height;
}

/**
 * Sets the sprite dimensions of the associated texture
 *
 * This method is used to divide the texture up into a sprite sheet for
 * animation. If it size is set to a value > 1, then calls to
 * {@link #setFrame} will adjust the current animation frame. This has
 * no effect if the billboard has no texture.
 *
 * Note that the sprite mesh already has its texture coordinates assigned.
 * The only way to animate the sprite is by applying an offset to the
 * texture coordinates. Therefore, for animation to work correctly, the
 * texture coordinates of the sprite mesh must all fit in a single frame
 * (the initial one). If these texture coordinate fit in a frame other than
 * frame 0, that should be specified.
 *
 * @param rows  The number of rows in the sprite sheet
 * @param cols  The number of columns in the sprite sheet
 * @param size  The number of frames in the sprite sheet
 * @param frame The initial frame represented by the sprite sheet
 */
void BillboardNode::setSpriteSheet(int rows, int cols, int size, int frame) {
    CUAssertLog(rows > 0 && cols > 0 && size >= rows*cols, "A %dx%d sprite sheet cannot support %d frames",rows,cols,size);
    if (size <= 1) {
        clearSpriteSheet();
    }
    CUAssertLog(frame >= 0 && frame < size, "Invalid animation frame %d", frame);
    _rows = rows;
    _cols = cols;
    _size = size;
    _frame = frame;
    computeSpriteBounds();
}

/**
 * Removes all animation information, setting the billboard to a static image
 */
void BillboardNode::clearSpriteSheet() {
    _bounds.set(0,0,0,0);
    _texoffset.setZero();
    _rows = 1;
    _cols = 1;
    _size = 1;
    _frame = 0;
}

/**
 * Sets the active frame as the given index.
 *
 * If the frame index is invalid, an error is raised.
 *
 * @param frame the index to make the active frame
 */
void BillboardNode::setFrame(int frame) {
    CUAssertLog(frame >= 0 && frame < _size, "Invalid animation frame %d", frame);
    
    _frame = frame;
    float x = (frame % _cols)*_bounds.size.width;
    float y = (frame / _cols)*_bounds.size.height;
    float dx = x-_bounds.origin.x;
    float dy = y-_bounds.origin.y;
    _texoffset.set(dx,dy);
}
