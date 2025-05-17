//
//  CUParticleShader.cpp
//  Cornell University Game Library (CUGL)
//
//  This module is a lightweight subclass of Shader that caches the uniform
//  locations, making it a little quicker to update their values.
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
#include <cugl/graphics/CUParticleShader.h>

using namespace cugl;
using namespace cugl::graphics;


/** Billboard fragment shader */
const std::string partShaderFrag =
#include "shaders/ParticleShader.frag"
;

/** Billboard vertex shader */
const std::string partShaderVert =
#include "shaders/ParticleShader.vert"
;

#pragma mark Constructors
/**
 * Creates an uninitialized shader with no source.
 *
 * You must initialize the shader for it to be compiled.
 */
ParticleShader::ParticleShader() : Shader(),
_cameraRightPos(-1),
_cameraUpPos(-1),
_perspectivePos(-1),
_modelMatrixPos(-1),
_texturePos(-1) {
}

/**
 * Deletes the shader program and resets all attributes.
 *
 * You must reinitialize the shader to use it.
 */
void ParticleShader::dispose() {
    _cameraRightPos = -1;
    _cameraUpPos = -1;
    _perspectivePos = -1;
    _modelMatrixPos = -1;
    _texturePos = -1;
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
bool ParticleShader::init() {
    return init(partShaderVert,partShaderFrag);
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
bool ParticleShader::init(const std::string vsource, const std::string fsource) {
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
    
    _texturePos = getUniformLocation("uTexture");
    if (_texturePos == -1) {
        CUAssertLog(false,"Shader missing texture uniform");
        return false;
    }
    
    return true;
}
