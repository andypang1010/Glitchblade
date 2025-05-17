//
//  CUTextureRendererVulkan.cpp
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
#include <cugl/graphics/CUTextureRenderer.h>
#include <cugl/graphics/CUShader.h>
#include <cugl/graphics/CUTexture.h>

const std::string fsqShaderFrag =
#include "shaders/FSQShader.frag"
;

const std::string fsqShaderVert =
#include "shaders/FSQShader.vert"
;

using namespace cugl;
using namespace cugl::graphics;

/**
 * Draws a full screen quad given a texture
 *
 * @param texture The texture to draw
 */
void TextureRenderer::draw(const std::shared_ptr<Texture>& texture) {
	_shader->bind();
	texture->bind();
	glDrawArrays(GL_TRIANGLES, 0, 3);
	texture->unbind();
	_shader->unbind();
}

/**
 * Initializes a texture renderer.
 */
bool TextureRenderer::init() {
	_shader = Shader::alloc(SHADER(fsqShaderVert), SHADER(fsqShaderFrag));
	return true;
}
