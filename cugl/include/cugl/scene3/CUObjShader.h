//
//  CUObjShader.h
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
#ifndef __CU_OBJ_SHADER_H__
#define __CU_OBJ_SHADER_H__
#include <cugl/graphics/CUShader.h>
#include <cugl/graphics/CUTexture.h>

namespace  cugl {

    /**
     * The classes to construct a 3-d scene graph.
     *
     * Unlike the scene2 package, the 3-d scene graph classes are quite limited.
     * We only have support for OBJ/MTL files, as well as simple billboarding.
     * There is no support for bone animation or physically based rendering.
     *
     * The reason for this limitation is because this is a student training
     * engine, and we often like to task students at adding those features.
     * In addition, unlike our 2-d scene graph with Figma support, there are
     * a lot of third party libraries out there that handle rendering better
     * for 3-d scenes.
     */
    namespace scene3 {

/**
 * This class is a shader for rendering {@link ObjModel} objects.
 *
 * This class is a very lighweight subclass of {@link Shader}. It is exists
 * mainly to verify the extistence of certain uniforms and cache their
 * program locations.
 */
class ObjShader : public graphics::Shader {
private:
    /** The location of the perspective uniform */
    GLint _perspectivePos;
    /** The location of model matrix uniform  */
    GLint _modelMatrixPos;
    /** The location of model matrix uniform  */
    GLint _normalMatrixPos;
    /** The locationm of the illumination uniform */
    GLint _illumPos;
    
    /** The location of the ambient color pos */
    GLint _KaPos;
    /** The location of the diffuse color pos */
    GLint _KdPos;
    /** The location of the specular color pos */
    GLint _KsPos;
    /** The location of the specular exponent pos */
    GLint _NsPos;
    
    // The textures settings
    /** The location of the ambient texture flag */
    GLint _hasKaPos;
    /** The location of the diffuse texture flag */
    GLint _hasKdPos;
    /** The location of the specular texture flag */
    GLint _hasKsPos;
    /** The location of the bump texture flag */
    GLint _hasKnPos;
    
    /** The location of the diffuse texture uniform */
    GLint _mapKdPos;
    /** The location of the ambient texture uniform */
    GLint _mapKaPos;
    /** The location of the specular texture uniform */
    GLint _mapKsPos;
    /** The location of the bump texture uniform */
    GLint _mapKnPos;
    
public:
#pragma mark Constructors
    /**
     * Creates an uninitialized shader with no source.
     *
     * You must initialize the shader for it to be compiled.
     */
    ObjShader();
    
    /**
     * Deletes this shader, disposing all resources.
     */
    ~ObjShader() { dispose(); }
    
    /**
     * Deletes the shader program and resets all attributes.
     *
     * You must reinitialize the shader to use it.
     */
    void dispose() override;
    
    /**
     * Initializes this shader with the standard vertex and fragment source.
     *
     * The shader will compile the vertex and fragment sources and link
     * them together. When compilation is complete, the shader will be
     * bound and active. In addition, all uniforms will be validated.
     *
     * @return true if initialization was successful.
     */
    bool init();
    
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
    bool init(const std::string vsource, const std::string fsource) override;
    
    /**
     * Returns a newly allocated shader with the standard vertex and fragment source.
     *
     * The shader will compile the vertex and fragment sources and link
     * them together. When compilation is complete, the shader will be
     * bound and active. In addition, all uniforms will be validated.
     *
     * @return a newly allocated shader with the standard vertex and fragment source.
     */
    static std::shared_ptr<ObjShader> alloc() {
        std::shared_ptr<ObjShader> result = std::make_shared<ObjShader>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated shader with the given vertex and fragment source.
     *
     * The shader will compile the vertex and fragment sources and link
     * them together. When compilation is complete, the shader will be
     * bound and active. In addition, all uniforms will be validated.
     *
     * @param vsource   The source string for the vertex shader.
     * @param fsource   The source string for the fragment shader.
     *
     * @return a newly allocated shader with the given vertex and fragment source.
     */
    static std::shared_ptr<ObjShader> alloc(const std::string vsource, const std::string fsource) {
        std::shared_ptr<ObjShader> result = std::make_shared<ObjShader>();
        return (result->init(vsource,fsource) ? result : nullptr);
    }
    
#pragma mark Attributes
    /**
     * Sets the perspective matrix for this shader.
     *
     * This method will only succeed if the shader is actively bound.
     *
     * @param matrix    The perspective matrix
     */
    void setPerspective(const Mat4& matrix) {
        setUniformMat4(_perspectivePos, matrix);
    }
    
    /**
     * Sets the model matrix for this shader.
     *
     * This method will only succeed if the shader is actively bound.
     *
     * @param matrix    The model matrix
     */
    void setModelMatrix(const Mat4& matrix) {
        setUniformMat4(_modelMatrixPos, matrix);
    }
    
    /**
     * Sets the normal matrix for this shader.
     *
     * This method will only succeed if the shader is actively bound.
     *
     * @param matrix    The normal matrix
     */
    void setNormalMatrix(const Mat4& matrix) {
        setUniformMat4(_normalMatrixPos, matrix);
    }
    
    /**
     * Sets the illumination level of this shader.
     *
     * The values are as follows:
     *
     *     0: Color with no reflection (ambient only)
     *     1: Diffuse reflection
     *     2: Specular reflection
     *
     * @param value The illumination value
     */
    void setIllum(GLuint value) {
        setUniform1i(_illumPos,value);
    }
    
    /**
     * Sets the ambient color for this shader.
     *
     * This method will only succeed if the shader is actively bound.
     *
     * @param color The ambient color
     */
    void setAmbientColor(const Color4f color) {
        setUniformVec4(_KaPos,color);
    }
    
    /**
     * Sets the diffuse color for this shader.
     *
     * This method will only succeed if the shader is actively bound.
     *
     * @param color The diffuse color
     */
    void setDiffuseColor(const Color4f color) {
        setUniformVec4(_KdPos,color);
    }
    
    /**
     * Sets the specular color for this shader.
     *
     * This method will only succeed if the shader is actively bound.
     *
     * @param color The specular color
     */
    void setSpecularColor(const Color4f color) {
        setUniformVec4(_KsPos,color);
    }
    
    /**
     * Sets the specular exponent for this shader.
     *
     * This method will only succeed if the shader is actively bound.
     *
     * @param value The specular exponent
     */
    void setSpecularExponent(float value) {
        setUniform1f(_NsPos,value);
    }
    
    /**
     * Sets the ambient texture for this shader.
     *
     * This method will only succeed if the shader is actively bound.
     *
     * @param texture   The ambient texture
     */
    void setAmbientTexture(const std::shared_ptr<graphics::Texture>& texture);
    
    /**
     * Sets the diffuse texture for this shader.
     *
     * This method will only succeed if the shader is actively bound.
     *
     * @param texture   The diffuse texture
     */
    void setDiffuseTexture(const std::shared_ptr<graphics::Texture>& texture);
    
    /**
     * Sets the specular texture for this shader.
     *
     * This method will only succeed if the shader is actively bound.
     *
     * @param texture   The specular texture
     */
    void setSpecularTexture(const std::shared_ptr<graphics::Texture>& texture);
    
    /**
     * Sets the normal (bump) texture for this shader.
     *
     * This method will only succeed if the shader is actively bound.
     *
     * @param texture   The normal texture
     */
    void setNormalTexture(const std::shared_ptr<graphics::Texture>& texture);
    
};

    }
}
#endif /* __CU_OBJ_SHADER_H__ */
