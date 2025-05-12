//
//  CUBillboardShader.h
//  Cornell University Game Library (CUGL)
//
//  This module is a lightweight subclass of Shader that caches the uniform
//  locations, making it a little quicker to update their values.
//
//  Note that this shader has been moved to the scene3 folder. Normally, we
//  put shaders in the render folder.  However, we want to restrict render to
//  our core graphic elements. Anything unique to scene graph like this module
//  is pulled out of that folder.
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
#ifndef __CU_BILLBOARD_SHADER_H__
#define __CU_BILLBOARD_SHADER_H__
#include <cugl/graphics/CUShader.h>
#include <cugl/graphics/CUGradient.h>

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

#define BILL_GRAD_CACHE 21

/**
 * This class is a shader for rendering {@link BillboardNode} objects.
 *
 * This class is a very lighweight subclass of {@link Shader}. It is exists
 * mainly to verify the extistence of certain uniforms and cache their
 * program locations.
 */
class BillboardShader : public graphics::Shader {
private:
    /** The location of the camera right uniform */
    GLint _cameraRightPos;
    /** The location of the camera up uniform */
    GLint _cameraUpPos;
    /** The location of the perspective uniform */
    GLint _perspectivePos;
    /** The location of model matrix uniform  */
    GLint _modelMatrixPos;
    /** The location of offset of the tex coords (for animation) */
    GLint _texOffsetPos;
    /** The location of gradient matrix uniform  */
    GLint _gradMatrixPos;
    /** The location of gradient inner color uniform  */
    GLint _gradInnerPos;
    /** The location of gradient outer color uniform  */
    GLint _gradOuterPos;
    /** The location of gradient extent uniform  */
    GLint _gradExtentPos;
    /** The location of gradient radius uniform  */
    GLint _gradRadiusPos;
    /** The location of gradient feather uniform  */
    GLint _gradFeathrPos;
    /** The location of the texture uniform */
    GLint _texturePos;
    /** The location of the drawing style uniform */
    GLint _stylePos;
    /** A cache for extracting gradient information */
    float _cache[BILL_GRAD_CACHE];
    
public:
#pragma mark Constructors
    /**
     * Creates an uninitialized shader with no source.
     *
     * You must initialize the shader for it to be compiled.
     */
    BillboardShader();
    
    /**
     * Deletes this shader, disposing all resources.
     */
    ~BillboardShader() { dispose(); }
    
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
    static std::shared_ptr<BillboardShader> alloc() {
        std::shared_ptr<BillboardShader> result = std::make_shared<BillboardShader>();
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
    static std::shared_ptr<BillboardShader> alloc(const std::string vsource, const std::string fsource) {
        std::shared_ptr<BillboardShader> result = std::make_shared<BillboardShader>();
        return (result->init(vsource,fsource) ? result : nullptr);
    }
    
#pragma mark Attributes
    
    /**
     * Sets the perspective matrix for this shader.
     *
     * A value of 0 will omit the texture and/or gradient, and only use colors.
     * A value of 1 or 3 will include the texture. A value of 2 or 3 will
     * include the gradient.
     *
     * This method will only succeed if the shader is actively bound.
     *
     * @param style The drawing style
     */
    void setStyle(Uint32 style) {
        setUniform1i(_stylePos, style);
    }
    
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
     * Sets the texture offset for this shader.
     *
     * This value can be used for simple animations. It adjusts the texture
     * coordinates of the sprite mesh by the given amount.
     *
     * This method will only succeed if the shader is actively bound.
     *
     * @param v The texture offset
     */
    void setTextureOffset(const Vec2 v) {
        setUniformVec2(_texOffsetPos, v);
    }
    
    /**
     * Sets the texture offset for this shader.
     *
     * This value can be used for simple animations. It adjusts the texture
     * coordinates of the sprite mesh by the given amount.
     *
     * This method will only succeed if the shader is actively bound.
     *
     * @param x The x-coordinate of the texture offset
     * @param y The y-coordinate of the texture offset
     */
    void setTextureOffset(float x, float y) {
        setUniform2f(_texOffsetPos,x,y);
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
    void setTexture(const std::shared_ptr<graphics::Texture>& texture) {
        setSampler(_texturePos, texture == nullptr ? 0 : texture->getBindPoint());
    }
    
    /**
     * Sets the gradient uniforms for this shader.
     *
     * If the gradient is nullptr, this will 0 all gradient uniforms.
     *
     * This method will only succeed if the shader is actively bound.
     *
     * @param grad The gradient object
     */
    void setGradient(const std::shared_ptr<graphics::Gradient>& grad);
    
};

    }
}
#endif /* __CU_BILLBOARD_SHADER_H__ */
