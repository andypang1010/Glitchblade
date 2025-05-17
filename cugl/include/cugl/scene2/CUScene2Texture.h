//
//  CUScene2Texture.h
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
#ifndef __CU_SCENE_2_TEXTURE_H__
#define __CU_SCENE_2_TEXTURE_H__
#include <cugl/scene2/CUScene2.h>
#include <cugl/graphics/CUTexture.h>
#include <cugl/graphics/CURenderTarget.h>
#include <memory>

namespace cugl {

    /**
     * The classes to construct a 2-d scene graph.
     *
     * Even though this is an optional package, this is one of the core features
     * of CUGL. These classes provide basic UI support (including limited Figma)
     * support. Any 2-d game will make extensive use of these classes. And
     * even 3-d games may use these classes for the HUD overlay.
     */
    namespace scene2 {

/**
 * This class provides the root node of an offscreen scene graph.
 *
 * This subclass of {@link Scene2} supports offscreen rendering to a texture.
 * It has its own {@link graphics::RenderTarget}, which is what it uses to
 * render to. You can then access the result of this with {@link #getTexture()}.
 * The rendering process ensures that the origin of the scene is rendered
 * to the bottom left corner of the texture (and not the top right, as is
 * the default in OpenGL), making it consist with sprite-based images used
 * by the scene graph.
 *
 * As a result, this class provides support for simple multi-pass rendering.
 * Simply render to a scene to a texture in one pass, and then use that
 * texture in future passes.
 */
class Scene2Texture : public Scene2 {
protected:
    /** The texture created by this scene */
    std::shared_ptr<graphics::Texture> _texture;
    
    /** The offscreen buffer for rendering the texture. */
    std::shared_ptr<graphics::RenderTarget> _target;
    
#pragma mark Constructors
public:
    /**
     * Creates a new degenerate Scene2Texture on the stack.
     *
     * The scene has no camera and must be initialized.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    Scene2Texture();
    
    /**
     * Deletes this scene, disposing all resources
     */
    ~Scene2Texture() { dispose(); }
    
    /**
     * Disposes all of the resources used by this scene.
     *
     * A disposed Scene2Texture can be safely reinitialized. Any children owned
     * by this scene will be released. They will be deleted if no other object
     * owns them.
     */
    virtual void dispose() override;
    
    /**
     * Initializes a Scene to fill the entire screen.
     *
     * @return true if initialization was successful.
     */
    virtual bool init() override;
    
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
    bool initWithHint(const Size hint) override;
    
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
     * @param width     The width size hint
     * @param height    The height size hint
     *
     * @return true if initialization was successful.
     */
    bool initWithHint(float width, float height) override {
        return initWithHint(Size(width,height));
    }
    
#pragma mark Static Constructors
    /**
     * Returns a newly allocated Scene to fill the entire screen.
     *
     * @return a newly allocated Scene to fill the entire screen..
     */
    static std::shared_ptr<Scene2Texture> alloc() {
        std::shared_ptr<Scene2Texture> result = std::make_shared<Scene2Texture>();
        return (result->init() ? result : nullptr);
    }

    /**
     * Returns a newly allocated Scene2Texture with the given size hint.
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
     * @return a newly allocated Scene2Texture with the given size hint.
     */
    static std::shared_ptr<Scene2Texture> allocWithHint(const Size hint) {
        std::shared_ptr<Scene2Texture> result = std::make_shared<Scene2Texture>();
        return (result->initWithHint(hint) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated Scene2Texture with the given size hint.
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
     * @param width     The width size hint
     * @param height    The height size hint
     *
     * @return a newly allocated Scene2Texture with the given size hint.
     */
    static std::shared_ptr<Scene2Texture> allocWithHint(float width, float height) {
        std::shared_ptr<Scene2Texture> result = std::make_shared<Scene2Texture>();
        return (result->initWithHint(width,height) ? result : nullptr);
    }
    
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
     */
    virtual void render() override;
    
    /**
     * Returns the texture associated with this scene graph.
     *
     * Rendering this scene graph will draw to the offscreen texture. This method
     * returns that texture so that it can be used in subsequent passes.
     *
     * @return the texture associated with this scene graph.
     */
    std::shared_ptr<graphics::Texture> getTexture() const {
        return _texture;
    }
};

    }
}

#endif /* __CU_SCENE_2_TEXTURE_H__ */
