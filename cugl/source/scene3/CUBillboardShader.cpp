//
//  CUBillboardShader.hpp
//  Cornell University Game Library (CUGL)
//
//  This module is a lightweight subclass of Shader that caches the uniform
//  locations, making it a little quicker to update their values.
//
//  Note that this shader has been moved to the scene3 folder. Normally, we
//  put shaders in the render folder.  However, we want to restrict render to
//  our core graphic elements. Anything unique to scene graph like this module
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
#include <cugl/scene3/CUBillboardShader.h>

using namespace cugl;
using namespace cugl::scene3;
using namespace cugl::graphics;

/** Billboard fragment shader */
const std::string billShaderFrag =
#include "shaders/BillboardShader.frag"
;

/** Billboard vertex shader */
const std::string billShaderVert =
#include "shaders/BillboardShader.vert"
;

#pragma mark Constructors
/**
 * Creates an uninitialized shader with no source.
 *
 * You must initialize the shader for it to be compiled.
 */
BillboardShader::BillboardShader() : Shader(),
_cameraRightPos(-1),
_cameraUpPos(-1),
_perspectivePos(-1),
_modelMatrixPos(-1),
_texOffsetPos(-1),
_gradMatrixPos(-1),
_gradInnerPos(-1),
_gradOuterPos(-1),
_gradExtentPos(-1),
_gradRadiusPos(-1),
_gradFeathrPos(-1),
_texturePos(-1),
_stylePos(-1) {
    std::memset(_cache,0,sizeof(float)*BILL_GRAD_CACHE);
}

/**
 * Deletes the shader program and resets all attributes.
 *
 * You must reinitialize the shader to use it.
 */
void BillboardShader::dispose() {
    _cameraRightPos = -1;
    _cameraUpPos = -1;
    _perspectivePos = -1;
    _modelMatrixPos = -1;
    _texOffsetPos = -1;
    _gradMatrixPos = -1;
    _gradInnerPos = -1;
    _gradOuterPos = -1;
    _gradExtentPos = -1;
    _gradRadiusPos = -1;
    _gradFeathrPos = -1;
    _texturePos = -1;
    _stylePos = -1;
    Shader::dispose();
}

/**
 * Initializes this shader with the standard vertex and fragment source.
 *
 * The shader will compile the vertex and fragment sources and link
 * them together. When compilation is complete, the shader will be
 * bound and active. In addition, all uniforms will be validated.
 *
 * @return true if initialization was successful.
 */
bool BillboardShader::init() {
    return init(billShaderVert,billShaderFrag);
}

/**
 * Initializes this shader with the given vertex and fragment source.
 *
 * The shader will compile the vertex and fragment sources and link
 * them together. When compilation is complete, the shader will be
 * bound and active. In addition, all uniforms will be validated.
 *
 * @param vsource   The source string for the vertex shader.
 * @param fsource   The source string for the fragment shader.
 *
 * @return true if initialization was successful.
 */
bool BillboardShader::init(const std::string vsource, const std::string fsource) {
    if (!Shader::init(SHADER(vsource), SHADER(fsource))) {
        return false;
    }
    
    // Now find all the uniforms
    _perspectivePos = getUniformLocation("uPerspective");
    if (_perspectivePos == -1) {
        CUAssertLog(false,"Shader missing perspective uniform");
        return false;
    }

    _modelMatrixPos = getUniformLocation("uModelMatrix");
    if (_modelMatrixPos == -1) {
        CUAssertLog(false,"Shader missing model matrix uniform");
        return false;
    }

    _cameraRightPos = getUniformLocation("uCameraRight");
    if (_cameraRightPos == -1) {
        CUAssertLog(false,"Shader missing camera right uniform");
        return false;
    }

    _cameraUpPos = getUniformLocation("uCameraUp");
    if (_cameraUpPos == -1) {
        CUAssertLog(false,"Shader missing camera up uniform");
        return false;
    }
    
    _texOffsetPos = getUniformLocation("uTexOffset");
    if (_texOffsetPos == -1) {
        CUAssertLog(false,"Shader missing texture offset uniform");
        return false;
    }

    _gradMatrixPos = getUniformLocation("uGradientMatrix");
    if (_gradMatrixPos == -1) {
        CUAssertLog(false,"Shader missing gradient matrix uniform");
        return false;
    }

    _gradInnerPos = getUniformLocation("uGradientInner");
    if (_gradInnerPos == -1) {
        CUAssertLog(false,"Shader missing gradient inner color uniform");
        return false;
    }

    _gradOuterPos = getUniformLocation("uGradientOuter");
    if (_gradOuterPos == -1) {
        CUAssertLog(false,"Shader missing gradient outer color uniform");
        return false;
    }

    _gradExtentPos = getUniformLocation("uGradientExtent");
    if (_gradExtentPos == -1) {
        CUAssertLog(false,"Shader missing gradient extent uniform");
        return false;
    }

    _gradRadiusPos = getUniformLocation("uGradientRadius");
    if (_gradRadiusPos == -1) {
        CUAssertLog(false,"Shader missing gradient radius uniform");
        return false;
    }

    _gradFeathrPos = getUniformLocation("uGradientFeathr");
    if (_gradFeathrPos == -1) {
        CUAssertLog(false,"Shader missing gradient feather uniform");
        return false;
    }
    
    _texturePos = getUniformLocation("uTexture");
    if (_texturePos == -1) {
        CUAssertLog(false,"Shader missing texture uniform");
        return false;
    }

    _stylePos = getUniformLocation("uType");
    if (_stylePos == -1) {
        CUAssertLog(false,"Shader missing drawing style uniform");
        return false;
    }
    
    return true;
}

#pragma mark Attributes
/**
 * Sets the gradient uniforms for this shader.
 *
 * If the gradient is nullptr, this will 0 all gradient uniforms.
 *
 * This method will only succeed if the shader is actively bound.
 *
 * @param grad The gradient object
 */
void BillboardShader::setGradient(const std::shared_ptr<Gradient>& grad) {
    if (grad != nullptr) {
        grad->getComponents(_cache);
    } else {
        std::memset(_cache,0,sizeof(float)*BILL_GRAD_CACHE);
    }
    
    setUniformMatrix3fv(_gradMatrixPos, 1, _cache, false);
    setUniform4f(_gradInnerPos,  _cache[ 9],_cache[10],_cache[11],_cache[12]);
    setUniform4f(_gradOuterPos,  _cache[13],_cache[14],_cache[15],_cache[16]);
    setUniform2f(_gradExtentPos, _cache[17],_cache[18]);
    setUniform1f(_gradRadiusPos, _cache[19]);
    setUniform1f(_gradFeathrPos, _cache[20]);
}
