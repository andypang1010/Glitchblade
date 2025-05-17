//
//  CUObjShader.cpp
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
#include <cugl/scene3/CUObjShader.h>

using namespace cugl;
using namespace cugl::graphics;
using namespace cugl::scene3;

/** OBJ fragment shader */
const std::string objShaderFrag =
#include "shaders/OBJShader.frag"
;

/** OBJ vertex shader */
const std::string objShaderVert =
#include "shaders/OBJShader.vert"
;

#define AMBIENT_BIND    1
#define DIFFUSE_BIND    0
#define SPECULAR_BIND   2
#define NORMAL_BIND     3

#pragma mark Constructors
/**
 * Creates an uninitialized shader with no source.
 *
 * You must initialize the shader for it to be compiled.
 */
ObjShader::ObjShader() : Shader(),
_perspectivePos(-1),
_modelMatrixPos(-1),
_normalMatrixPos(-1),
_illumPos(-1),
_KaPos(-1),
_KdPos(-1),
_KsPos(-1),
_NsPos(-1),
_hasKaPos(-1),
_hasKdPos(-1),
_hasKsPos(-1),
_hasKnPos(-1),
_mapKdPos(-1),
_mapKaPos(-1),
_mapKsPos(-1),
_mapKnPos(-1) {
}

/**
 * Deletes the shader program and resets all attributes.
 *
 * You must reinitialize the shader to use it.
 */
void ObjShader::dispose() {
    _perspectivePos = -1;
    _modelMatrixPos = -1;
    _normalMatrixPos = -1;
    _illumPos = -1;
    _KaPos = -1;
    _KdPos = -1;
    _KsPos = -1;
    _NsPos = -1;
    _hasKaPos = -1;
    _hasKdPos = -1;
    _hasKsPos = -1;
    _hasKnPos = -1;
    _mapKdPos = -1;
    _mapKaPos = -1;
    _mapKsPos = -1;
    _mapKnPos = -1;     
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
bool ObjShader::init() {
    return init(objShaderVert,objShaderFrag);
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
bool ObjShader::init(const std::string vsource, const std::string fsource) {
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

    _normalMatrixPos = getUniformLocation("uNormalMatrix");
    if (_normalMatrixPos == -1) {
        CUAssertLog(false,"Shader missing normal matrix uniform");
        return false;
    }

    _illumPos = getUniformLocation("uIllum");
    if (_illumPos == -1) {
        CUAssertLog(false,"Shader missing illum uniform");
        return false;
    }
    
    _KaPos = getUniformLocation("uKa");
    if (_KaPos == -1) {
        CUAssertLog(false,"Shader missing ambient color uniform");
        return false;
    }

    _KdPos = getUniformLocation("uKd");
    if (_KdPos == -1) {
        CUAssertLog(false,"Shader missing diffuse color uniform");
        return false;
    }

    _KsPos = getUniformLocation("uKs");
    if (_KsPos == -1) {
        CUAssertLog(false,"Shader missing specular color uniform");
        return false;
    }

    _NsPos = getUniformLocation("uNs");
    if (_NsPos == -1) {
        CUAssertLog(false,"Shader missing specular exponent uniform");
        return false;
    }
    
    _hasKaPos = getUniformLocation("uHasKa");
    if (_hasKaPos == -1) {
        CUAssertLog(false,"Shader missing ambient flag uniform");
        return false;
    }

    _mapKaPos = getUniformLocation("uMapKa");
    if (_mapKaPos == -1) {
        CUAssertLog(false,"Shader missing ambient texture uniform");
        return false;
    }
    
    _hasKdPos = getUniformLocation("uHasKd");
    if (_hasKdPos == -1) {
        CUAssertLog(false,"Shader missing diffuse flag uniform");
        return false;
    }

    _mapKdPos = getUniformLocation("uMapKd");
    if (_mapKdPos == -1) {
        CUAssertLog(false,"Shader missing diffuse texture uniform");
        return false;
    }

    _hasKsPos = getUniformLocation("uHasKs");
    if (_hasKsPos == -1) {
        CUAssertLog(false,"Shader missing specular flag uniform");
        return false;
    }

    _mapKsPos = getUniformLocation("uMapKs");
    if (_mapKsPos == -1) {
        CUAssertLog(false,"Shader missing specular texture uniform");
        return false;
    }

    _hasKnPos = getUniformLocation("uHasKn");
    if (_hasKnPos == -1) {
        CUAssertLog(false,"Shader missing bump flag uniform");
        return false;
    }

    _mapKnPos = getUniformLocation("uMapKn");
    if (_mapKnPos == -1) {
        CUAssertLog(false,"Shader missing bump texture uniform");
        return false;
    }


    return true;
}

/**
 * Sets the ambient texture for this shader.
 *
 * This method will only succeed if the shader is actively bound.
 *
 * @param texture   The ambient texture
 */
void ObjShader::setAmbientTexture(const std::shared_ptr<Texture>& texture) {
    if (texture == nullptr) {
        setUniform1i(_hasKaPos,0);
        setSampler(_mapKaPos, 0);
    } else {
        setUniform1i(_hasKaPos,1);
        setSampler(_mapKaPos, AMBIENT_BIND);
        texture->setBindPoint(AMBIENT_BIND);
    }
}

/**
 * Sets the diffuse texture for this shader.
 *
 * This method will only succeed if the shader is actively bound.
 *
 * @param texture   The diffuse texture
 */
void ObjShader::setDiffuseTexture(const std::shared_ptr<Texture>& texture) {
    if (texture == nullptr) {
        setUniform1i(_hasKdPos,0);
        setSampler(_mapKdPos, 0);
    } else {
        setUniform1i(_hasKdPos,1);
        setSampler(_mapKdPos, DIFFUSE_BIND);
        texture->setBindPoint(DIFFUSE_BIND);
    }
}

/**
 * Sets the specular texture for this shader.
 *
 * This method will only succeed if the shader is actively bound.
 *
 * @param texture   The specular texture
 */
void ObjShader::setSpecularTexture(const std::shared_ptr<Texture>& texture) {
    if (texture == nullptr) {
        setUniform1i(_hasKsPos,0);
        setSampler(_mapKsPos, 0);
    } else {
        setUniform1i(_hasKsPos,1);
        setSampler(_mapKsPos, SPECULAR_BIND);
        texture->setBindPoint(SPECULAR_BIND);
    }
}

/**
 * Sets the normal (bump) texture for this shader.
 *
 * This method will only succeed if the shader is actively bound.
 *
 * @param texture   The normal texture
 */
void ObjShader::setNormalTexture(const std::shared_ptr<Texture>& texture) {
    if (texture == nullptr) {
        setUniform1i(_hasKnPos,0);
        setSampler(_mapKnPos, 0);
    } else {
        setUniform1i(_hasKnPos,1);
        setSampler(_mapKnPos, NORMAL_BIND);
        texture->setBindPoint(NORMAL_BIND);
    }
}
