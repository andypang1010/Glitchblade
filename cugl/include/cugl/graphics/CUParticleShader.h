//
//  CUParticleShader.h
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
#ifndef __CU_PARTICLE_SHADER_H__
#define __CU_PARTICLE_SHADER_H__
#include <cugl/graphics/CUShader.h>
#include <cugl/graphics/CUTexture.h>

namespace cugl {

    /**
     * The classes and functions needed to construct a graphics pipeline.
     *
     * Initially these were part of the core CUGL module (everyone wants graphics,
     * right). However, after student demand for a headless option that did not
     * have so many OpenGL dependencies, this was factored out.
     */
    namespace graphics {

/**
 * This class is a shader for rendering {@link ParticleSystem} objects.
 *
 * This class is a very lighweight subclass of {@link Shader}. It is exists
 * mainly to verify the extistence of certain uniforms and cache their
 * program locations.
 */
class ParticleShader : public Shader {
private:
    /** The location of the camera right uniform */
    GLint _cameraRightPos;
    /** The location of the camera up uniform */
    GLint _cameraUpPos;
    /** The location of the perspective uniform */
    GLint _perspectivePos;
    /** The location of model matrix uniform  */
    GLint _modelMatrixPos;
    /** The location of the texture uniform */
    GLint _texturePos;
    
public:
#pragma mark Constructors
    /**
     * Creates an uninitialized shader with no source.
     *
     * You must initialize the shader for it to be compiled.
     */
    ParticleShader();
    
    /**
     * Deletes this shader, disposing all resources.
     */
    ~ParticleShader() { dispose(); }
    
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
    static std::shared_ptr<ParticleShader> alloc() {
        std::shared_ptr<ParticleShader> result = std::make_shared<ParticleShader>();
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
    static std::shared_ptr<ParticleShader> alloc(const std::string vsource, const std::string fsource) {
        std::shared_ptr<ParticleShader> result = std::make_shared<ParticleShader>();
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
     * Sets the right direction of the camera for this shader.
     *
     * This method will only succeed if the shader is actively bound.
     *
     * @param v The camera right direction
     */
    void setCameraRight(const Vec3 v) {
        setUniformVec3(_cameraRightPos,v);
    }
    
    /**
     * Sets the up direction of the camera for this shader.
     *
     * This method will only succeed if the shader is actively bound.
     *
     * @param v The camera up direction
     */
    void setCameraUp(const Vec3 v) {
        setUniformVec3(_cameraUpPos,v);
    }
    
    /**
     * Sets the texture for this shader.
     *
     * This method will only succeed if the shader is actively bound. In
     * addition, it does not actually bind the texture. That must be done
     * separately.
     *
     * @param texture   The texture to bind
     */
    void setTexture(const std::shared_ptr<Texture>& texture) {
        setSampler(_texturePos, texture == nullptr ? 0 : texture->getBindPoint());
    }
    
};

    }
}
#endif /* __CU_BILLBOARD_SHADER_H__ */
