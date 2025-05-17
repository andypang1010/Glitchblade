//
//  CUTextureRenderer.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a way to render a texture to the screen. It is a simple 
//  fullscreen quad pass.
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
//  Author: Zachary Schecter
//  Version: 7/3/24 (CUGL 3.0 reorganization)
//
#ifndef __CU_TEXTURE_RENDERER_H__
#define __CU_TEXTURE_RENDERER_H__
#include <cugl/graphics/CUGraphicsBase.h>
#include <cugl/graphics/CUMesh.h>
#include <cugl/core/math/CUMathBase.h>
#include <cugl/core/math/CUMat4.h>
#include <cugl/core/math/CUColor4.h>


namespace cugl {

    /**
     * The classes and functions needed to construct a graphics pipeline.
     *
     * Initially these were part of the core CUGL module (everyone wants graphics,
     * right). However, after student demand for a headless option that did not
     * have so many OpenGL dependencies, this was factored out.
     */
    namespace graphics {

/** Forward references */
class Shader;
class Texture;

/**
 * A class for drawing a full-screen texture.
 *
 * This class is an alternative to {@link SpriteBatch} for the case when all
 * you want to do is draw a single texture as a full-screen quad. There is no
 * support for compositing or other features. It is primarily used to test out
 * custom shaders.
 */
class TextureRenderer {
#pragma mark Values
private:
    /** The shader for this renderer  */
    std::shared_ptr<Shader> _shader;
    
public:
    /**
     * Draws a full screen quad given a texture
     *
     * @param texture The texture to draw
     */
    void draw(const std::shared_ptr<Texture>& texture);
    
    /**
     * Initializes a texture renderer.
     */
    bool init();
    
    /**
     * Returns a new texture renderer with the default vertex capacity
     *
     * @return a new texture renderer
     */
    static std::shared_ptr<TextureRenderer> alloc() {
        std::shared_ptr<TextureRenderer> result = std::make_shared<TextureRenderer>();
        return (result->init() ? result : nullptr);
    }
};

    }

}
#endif /* __CU_TEXTURE_RENDERER_H__ */
