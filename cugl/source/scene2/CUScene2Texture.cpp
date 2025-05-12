//
//  CUScene2Texture.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides support for writing the results of a scene graph to
//  a texture. This is very helpful for doing simple multi-pass rendering.
//  You can render to a texture, and then post-process that texture
//  a second pass.
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
#include <cugl/scene2/CUScene2Texture.h>
#include <cugl/core/CUDisplay.h>

using namespace cugl;
using namespace cugl::scene2;
using namespace cugl::graphics;

/**
 * Creates a new degenerate Scene2Texture on the stack.
 *
 * The scene has no camera and must be initialized.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
 * the heap, use one of the static constructors instead.
 */
Scene2Texture::Scene2Texture() : Scene2() {
}
  
/**
 * Disposes all of the resources used by this scene.
 *
 * A disposed Scene2Texture can be safely reinitialized. Any children owned
 * by this scene will be released. They will be deleted if no other object
 * owns them.
 */
void Scene2Texture::dispose() {
    Scene2::dispose();
    _target = nullptr;
    _texture = nullptr;
}
    
/**
 * Initializes a Scene to fill the entire screen.
 *
 * @return true if initialization was successful.
 */
bool Scene2Texture::init() {
    if (Scene2::init()) {
        // We need to account for high DPI screens
        float scale = Display::get()->getPixelDensity();
        float width = _size.width*scale;
        float height = _size.height*scale;

        _target = RenderTarget::alloc(width, height);
        _texture = (_target == nullptr ? nullptr : _target->getTexture());
        return _texture != nullptr;
    }
    return false;
}

/**
 * Initializes a Scene2Texture with the given size hint.
 *
 * Scenes are designed to fill the entire screen. If you want a scene that
 * is only part of the screen, that should be implemented with a specific
 * scene graph. However, the size of that screen can vary from device to
 * device. To make scene design easier, designs are typically locked to a
 * dimension: width or height.
 *
 * This is the purpose of the size hint. If either of the values of hint
 * are non-zero, then the scene will lock that dimension to that particular
 * size. If both are non-zero, it will choose its dimension according to the
 * device orientation. Landscape will be height, while portrait will pick
 * width. Devices with no orientation will always priortize height over
 * width.
 *
 * @param hint      The size hint
 *
 * @return a newly allocated Scene with the given size hint.
 */
bool Scene2Texture::initWithHint(const Size hint) {
    if (Scene2::initWithHint(hint)) {
        // We need to account for high DPI screens
        float scale = Display::get()->getPixelDensity();
        float width = _size.width*scale;
        float height = _size.height*scale;

        _target = RenderTarget::alloc(width, height);
        _texture = (_target == nullptr ? nullptr : _target->getTexture());
        return _texture != nullptr;
    }
    return false;
}
  
#pragma mark -
#pragma mark Scene Logic

/**
 * Draws all of the children in this scene with the given SpriteBatch.
 *
 * This method with draw using {@link #getSpriteBatch}. If not sprite batch
 * has been assigned, nothing will be drawn.
 *
 * Rendering happens by traversing the the scene graph using an "Pre-Order"
 * tree traversal algorithm ( https://en.wikipedia.org/wiki/Tree_traversal#Pre-order ).
 * That means that parents are always draw before (and behind children).
 * To override this draw order, you should place an {@link OrderedNode}
 * in the scene graph to specify an alternative order.
 *
 * This will render to the offscreen texture associated with this scene.
 * That texture can then be used in subsequent render passes.
 *
 * @param batch     The SpriteBatch to draw with.
 */
void Scene2Texture::render() {
    if (_batch == nullptr) {
        return;
    } else if (_batch->isDrawing()) {
        _batch->end();
    }
    
    Affine2 matrix = _camera->getCombined();
    matrix.scale(1, -1); // Flip the y axis for texture write
    
    _target->begin();
    _batch->begin(matrix);
    _batch->setSrcBlendFunc(_srcFactor);
    _batch->setDstBlendFunc(_dstFactor);
    _batch->setBlendEquation(_blendEquation);

    for(auto it = _children.begin(); it != _children.end(); ++it) {
        (*it)->render(_batch, Affine2::IDENTITY, _color);
    }

    _batch->end();
    _target->end();
}
