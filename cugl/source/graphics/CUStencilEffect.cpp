//
//  CUStencilEffect.h
//  Cornell University Game Library (CUGL)
//
//  The class Spritebatch does support a basic stencil effects.  In order to
//  support SVG files, these effects became fairly elaborate, as they are
//  splitting the stencil space in half, and coordinating between the two
//  halves. Therefore, we decided to pull the functionality out of Spritebatch
//  into its own module.
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
#include <cugl/graphics/CUStencilEffect.h>

using namespace cugl;
using namespace cugl::graphics;

/**
 * Clears the stencil buffer specified
 *
 * @param buffer    The stencil buffer (lower, upper, both)
 */
void stencil::clearBuffer(GLenum buffer) {
    switch (buffer) {
        case STENCIL_NONE:
            return;
        case STENCIL_LOWER:
            glStencilMask(0xf0);
            glClear(GL_STENCIL_BUFFER_BIT);
            glStencilMask(0xff);
            return;
        case STENCIL_UPPER:
            glStencilMask(0x0f);
            glClear(GL_STENCIL_BUFFER_BIT);
            glStencilMask(0xff);
            return;
        case STENCIL_BOTH:
            glStencilMask(0xff);
            glClear(GL_STENCIL_BUFFER_BIT);
            return;
    }
}

/**
 * Configures the settings to apply the given effect.
 *
 * Note that the shader parameter is only relevant in Vulkan, as OpenGL stencil 
 * operations are applied globally.
 *
 * @param effect    The stencil effect
 * @param shader    The shader to apply the stencil operations to. 
 */
void stencil::applyEffect(StencilEffect effect,
                          std::shared_ptr<cugl::graphics::Shader> shader) {
    //CULog("Applying %d",effect);
    switch(effect) {
        case StencilEffect::NATIVE:
            // Nothing more to do
            break;
        case StencilEffect::NONE:
            glDisable(GL_STENCIL_TEST);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            break;
        case StencilEffect::CLIP:
        case StencilEffect::CLIP_JOIN:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xff);
            glStencilFunc(GL_NOTEQUAL, 0x00, 0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            break;
        case StencilEffect::MASK:
        case StencilEffect::MASK_JOIN:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xff);
            glStencilFunc(GL_EQUAL, 0x00, 0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            break;
        case StencilEffect::FILL:
        case StencilEffect::FILL_JOIN:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xff);
            glStencilFunc(GL_NOTEQUAL, 0x00, 0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            break;
        case StencilEffect::WIPE:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xff);
            glStencilFunc(GL_ALWAYS, 0x00, 0xff);
            glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            break;
        case StencilEffect::STAMP:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xff);
            glStencilFunc(GL_ALWAYS, 0x00, 0xff);
            glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
            glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR_WRAP);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            break;
        case StencilEffect::CARVE:
        case StencilEffect::CARVE_NONE:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xf0);
            glStencilFunc(GL_EQUAL, 0x00, 0xf0);
            glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            break;
        case StencilEffect::CLAMP:
        case StencilEffect::CLAMP_NONE:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xf0);
            glStencilFunc(GL_EQUAL, 0x00, 0xf0);
            glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            break;
        case StencilEffect::NONE_CLIP:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0x0f);
            glStencilFunc(GL_NOTEQUAL, 0x00, 0x0f);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            break;
        case StencilEffect::NONE_MASK:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0x0f);
            glStencilFunc(GL_EQUAL, 0x00, 0x0f);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            break;
        case StencilEffect::NONE_FILL:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0x0f);
            glStencilFunc(GL_NOTEQUAL, 0x00, 0x0f);
            glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            break;
        case StencilEffect::NONE_WIPE:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0x0f);
            glStencilFunc(GL_ALWAYS, 0x00, 0x0f);
            glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            break;
        case StencilEffect::NONE_STAMP:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0x0f);
            glStencilFunc(GL_ALWAYS, 0x00, 0x0f);
            glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INVERT);
            glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_INVERT);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            break;
        case StencilEffect::NONE_CARVE:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0x0f);
            glStencilFunc(GL_EQUAL, 0x00, 0x0f);
            glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            break;
        case StencilEffect::NONE_CLAMP:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0x0f);
            glStencilFunc(GL_EQUAL, 0x00, 0x0f);
            glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            break;
        case StencilEffect::CLIP_NONE:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xff);
            glStencilFunc(GL_NOTEQUAL, 0x00, 0xf0);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            break;
        case StencilEffect::CLIP_MEET:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xff);
            glStencilFunc(GL_EQUAL, 0xff, 0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            break;
        case StencilEffect::CLIP_MASK:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xff);
            glStencilFunc(GL_EQUAL, 0xf0, 0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            break;
        case StencilEffect::CLIP_FILL:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0x0f);
            glStencilFunc(GL_NOTEQUAL, 0x00, 0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            break;
        case StencilEffect::CLIP_WIPE:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0x0f);
            glStencilFunc(GL_NOTEQUAL, 0x00, 0xf0);
            glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            break;
        case StencilEffect::CLIP_STAMP:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0x0f);
            glStencilFunc(GL_NOTEQUAL, 0x00, 0xf0);
            glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INVERT);
            glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_INVERT);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            break;
        case StencilEffect::CLIP_CARVE:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0x0f);
            glStencilFunc(GL_EQUAL, 0xf0, 0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            break;
        case StencilEffect::CLIP_CLAMP:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0x0f);
            glStencilFunc(GL_EQUAL, 0xf0, 0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            break;
        case StencilEffect::MASK_NONE:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xff);
            glStencilFunc(GL_EQUAL, 0x00, 0xf0);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            break;
        case StencilEffect::MASK_MEET:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xff);
            glStencilFunc(GL_NOTEQUAL, 0xff, 0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            break;
        case StencilEffect::MASK_CLIP:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xff);
            glStencilFunc(GL_EQUAL, 0x0f, 0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            break;
        case StencilEffect::MASK_FILL:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0x0f);
            glStencilFunc(GL_EQUAL, 0x0f, 0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            break;
        case StencilEffect::MASK_WIPE:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0x0f);
            glStencilFunc(GL_EQUAL, 0x00, 0xf0);
            glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            break;
        case StencilEffect::MASK_STAMP:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0x0f);
            glStencilFunc(GL_EQUAL, 0x00, 0xf0);
            glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INVERT);
            glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_INVERT);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            break;
        case StencilEffect::MASK_CARVE:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0x0f);
            glStencilFunc(GL_EQUAL, 0x0, 0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            break;
        case StencilEffect::MASK_CLAMP:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0x0f);
            glStencilFunc(GL_EQUAL, 0x00, 0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            break;
        case StencilEffect::FILL_NONE:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xf0);
            glStencilFunc(GL_NOTEQUAL, 0x00, 0xf0);
            glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            break;
        case StencilEffect::FILL_MEET:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xf0);
            glStencilFunc(GL_EQUAL, 0xff, 0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            break;
        case StencilEffect::FILL_CLIP:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xf0);
            glStencilFunc(GL_EQUAL, 0xff, 0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            break;
        case StencilEffect::FILL_MASK:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xf0);
            glStencilFunc(GL_EQUAL, 0xf0, 0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            break;
        case StencilEffect::WIPE_NONE:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xf0);
            glStencilFunc(GL_ALWAYS, 0x00, 0xf0);
            glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            break;
        case StencilEffect::WIPE_CLIP:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xf0);
            glStencilFunc(GL_NOTEQUAL, 0x00, 0x0f);
            glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            break;
        case StencilEffect::WIPE_MASK:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xf0);
            glStencilFunc(GL_EQUAL, 0x00, 0x0f);
            glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            break;
        case StencilEffect::STAMP_NONE:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xf0);
            glStencilFunc(GL_ALWAYS, 0x00, 0x0f);
            glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INVERT);
            glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_INVERT);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            break;
        case StencilEffect::STAMP_CLIP:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xf0);
            glStencilFunc(GL_NOTEQUAL, 0x00, 0x0f);
            glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INVERT);
            glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_INVERT);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            break;
        case StencilEffect::STAMP_MASK:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xf0);
            glStencilFunc(GL_EQUAL, 0x00, 0x0f);
            glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INVERT);
            glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_INVERT);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            break;
        case StencilEffect::STAMP_BOTH:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xff);
            glStencilFunc(GL_ALWAYS, 0x00, 0xff);
            glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INVERT);
            glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_INVERT);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            break;
        case StencilEffect::CARVE_CLIP:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xf0);
            glStencilFunc(GL_NOTEQUAL, 0x0f, 0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            break;
        case StencilEffect::CARVE_MASK:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xf0);
            glStencilFunc(GL_EQUAL, 0x0f, 0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            break;
        case StencilEffect::CARVE_BOTH:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xff);
            glStencilFunc(GL_EQUAL, 0x00, 0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            break;
        case StencilEffect::CLAMP_CLIP:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xf0);
            glStencilFunc(GL_EQUAL, 0x0f, 0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            break;
        case StencilEffect::CLAMP_MASK:
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0xf0);
            glStencilFunc(GL_EQUAL, 0x00, 0xff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            break;
    }
}
