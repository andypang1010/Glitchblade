//
//  CUInstanceBuffer.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a vertex buffer that supports instancing. It is used
//  heavily by our particle system. It separates the buffer data into two
//  groups: the template and the instance data. The template is the mesh data
//  repeated every instance. The instance data is the unique data for each
//  instance. By design, template data is designed to be static, while instance
//  data is designed to be streamed.
//
//  For simplicity, we do not support instance depths (e.g glVertexAttribDivisor)
//  greater than 1. If you need that level of control, you should create your own
//  abstraction.
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
#ifndef __CU_INSTANCE_BUFFER_H__
#define __CU_INSTANCE_BUFFER_H__
#include <string>
#include <unordered_map>
#include <cugl/graphics/CUVertexBuffer.h>

namespace cugl {

    /**
     * The classes and functions needed to construct a graphics pipeline.
     *
     * Initially these were part of the core CUGL module (everyone wants graphics,
     * right). However, after student demand for a headless option that did not
     * have so many OpenGL dependencies, this was factored out.
     */
    namespace graphics {

// Forward reference to the associated shader
class Shader;

/**
 * This class defines a vertex buffer for drawing instanced shapes.
 *
 * This class is an alternative to {@link VertexBuffer} for those cases in
 * which you need to instance a simple mesh (e.g. particle systems). It
 * separates the buffer data into two groups: the template and the instance
 * data. The template is the mesh data repeated every instance. The instance
 * data is the unique data for each instance. By design, template data is
 * designed to be static, while instance data is designed to be streamed.
 *
 * The template mesh uses these methods inherited from {@link VertexBuffer}.
 * Indeed, if you restrict yourself to only the inherited methods, this class
 * behaves exactly like {@link VertexBuffer}. Instancing is provided by the
 * new methods.
 *
 * For simplicity, we do not support instance depths (e.g glVertexAttribDivisor)
 * greater than 1. If you need that level of control, you should create your own
 * abstraction.
 *
 * A instance buffer must be attached to a shader to be used. However, an
 * instance buffer can swap shaders at any time, which is why this class is
 * separated out. Unlike {@link Texture} and {@link UniformBuffer}, an instance
 * buffer does not have a true many one relationship with a {@link Shader}
 * object. An instance buffer can only be connected to one shader at a time and
 * vice versa. So we model this as a direct connection. As instance buffers push
 * data to a shader, the dependency requires that a shader be linked to an
 * instance buffer object.
 *
 * This class tries to remain loosely coupled with its shader. If the instance
 * buffer has attributes lacking in the shader, they will be ignored. If it is
 * missing attributes that the shader expects, the shader will use the default
 * value for the type.
 *
 * As with {@link VertexBuffer}, we allow the mesh data to be indexed, though
 * that is not required. The index data is applied to the template, not the
 * instance data.
 */
class InstanceBuffer : public VertexBuffer {
protected:
    
    /** The maximums size of the instance layer */
    GLsizei _instanceSize;
    /** The data stride of the instance layer (0 if there is only one attribute) */
    GLsizei _instanceStride;
    
    /** The buffer for the instance data */
    GLuint _instanceBuffer;
    
    /** The instance attributes */
    std::unordered_map<std::string, AttribData> _instAttribs;
    
public:
#pragma mark Constructors
    /**
     * Creates an uninitialized instance buffer.
     *
     * You must initialize the instance buffer to allocate buffer memory.
     */
    InstanceBuffer() : VertexBuffer(), _instanceSize(0), _instanceStride(0), _instanceBuffer(0) {}
    
    /**
     * Deletes this instance buffer, disposing all resources.
     */
    ~InstanceBuffer() {dispose(); }
    
    /**
     * Deletes the instance buffer, freeing all resources.
     *
     * You must reinitialize the instance buffer to use it.
     */
    void dispose() override;
    
    /**
     * Initializes this instance buffer to support the given stride
     *
     * The stride is the size of a single piece of vertex data. The instance
     * buffer needs this value to set attribute locations for both the template
     * and the instance buffers. In this initializer, these are assumed to be
     * the same value.
     *
     * Since changing these values fundamentally changes the type of data that
     * can be sent to this instance buffer, they are set at buffer creation and
     * cannot be changed. It is possible for a stride to be 0, but only if that
     * layer consists of a single attribute. Using stride 0 is not recommended.
     *
     * For performance reasons, we also require that the instance buffer specify
     * a maximum size. This size is applied to everything: vertex, index, and
     * instance data. So it should be the maximum of all of them. Size is
     * specified in terms of maximum elements, not bytes.
     *
     * @param size      The maximum number of elements in this buffer
     * @param stride    The size of a single piece of data (template or instance)
     *
     * @return true if initialization was successful.
     */
    bool init(GLsizei size, GLsizei stride) override {
        return init(size,stride,size,stride);
    }
    
    /**
     * Initializes this instance buffer to support the given strides
     *
     * The stride is the size of a single piece of vertex data. The instance
     * buffer needs this value to set attribute locations for both the template
     * and the instance buffers. Note that these values do not have to be the
     * same as these buffers are separate.
     *
     * Since changing these values fundamentally changes the type of data that
     * can be sent to this instance buffer, they are set at buffer creation and
     * cannot be changed. It is possible for a stride to be 0, but only if that
     * layer consists of a single attribute. Using stride 0 is not recommended.
     *
     * For performance reasons, we also require that the instance buffer specify
     * a maximum size. The template size is applied both the template vertices
     * and the indices. So it should be the maximum of both. Size is specified
     * in terms of maximum elements, not bytes.
     *
     * @param tsize     The maximum number of elements in the template later
     * @param tstride   The size of a single piece of data in the template layer
     * @param isize     The maximum number of elements in the instance later
     * @param istride   The size of a single piece of data in the instance layer
     *
     * @return true if initialization was successful.
     */
    bool init(GLsizei tsize, GLsizei tstride, GLsizei isize, GLsizei istride);
    
    /**
     * Returns a new instance buffer to support the given strides.
     *
     * The stride is the size of a single piece of vertex data. The instance
     * buffer needs this value to set attribute locations for both the template
     * and the instance buffers. In this initializer, these are assumed to be
     * the same value.
     *
     * Since changing these values fundamentally changes the type of data that
     * can be sent to this instance buffer, they are set at buffer creation and
     * cannot be changed. It is possible for a stride to be 0, but only if that
     * layer consists of a single attribute. Using stride 0 is not recommended.
     *
     * For performance reasons, we also require that the instance buffer specify
     * a maximum size. This size is applied to everything: vertex, index, and
     * instance data. So it should be the maximum of all of them. Size is
     * specified in terms of maximum elements, not bytes.
     *
     * @param size      The maximum number of elements in this buffer
     * @param stride    The size of a single piece of data (template or instance)
     *
     * @return a new instance buffer to support the given strides.
     */
    static std::shared_ptr<InstanceBuffer> alloc(GLsizei size, GLsizei stride) {
        std::shared_ptr<InstanceBuffer> result = std::make_shared<InstanceBuffer>();
        return (result->init(size, stride) ? result : nullptr);
    }
    
    /**
     * Returns a new instance buffer to support the given strides.
     *
     * The stride is the size of a single piece of vertex data. The instance
     * buffer needs this value to set attribute locations for both the template
     * and the instance buffers. Note that these values do not have to be the
     * same as these buffers are separate.
     *
     * Since changing these values fundamentally changes the type of data that
     * can be sent to this instance buffer, they are set at buffer creation and
     * cannot be changed. It is possible for a stride to be 0, but only if that
     * layer consists of a single attribute. Using stride 0 is not recommended.
     *
     * For performance reasons, we also require that the instance buffer specify
     * a maximum size. The template size is applied both the template vertices
     * and the indices. So it should be the maximum of both. Size is specified
     * in terms of maximum elements, not bytes.
     *
     * @param tsize     The maximum number of elements in the template later
     * @param tstride   The size of a single piece of data in the template layer
     * @param isize     The maximum number of elements in the instance later
     * @param istride   The size of a single piece of data in the instance layer
     *
     * @return a new instance buffer to support the given strides.
     */
    static std::shared_ptr<InstanceBuffer> alloc(GLsizei tsize, GLsizei tstride,
                                                 GLsizei isize, GLsizei istride) {
        std::shared_ptr<InstanceBuffer> result = std::make_shared<InstanceBuffer>();
        return (result->init(tsize, tstride, isize, istride) ? result : nullptr);
    }
    
    
#pragma mark -
#pragma mark Binding
    /**
     * Attaches the given shader to this instance buffer.
     *
     * This method will link all enabled attributes in this instance buffer
     * (warning about any attributes that are missing from the shader). It will
     * also immediately bind both the instance buffer and the shader, making
     * them ready to use.
     *
     * @param shader    The shader to attach
     */
    void attach(const std::shared_ptr<Shader>& shader) override;
    
    
    
#pragma mark -
#pragma mark Vertex Processing
    /**
     * Returns the maximum capacity of the instance layer.
     *
     * The size determines the number of elements that can be loaded with
     * {@link #loadInstanceData}.
     *
     * @return the maximum capacity of the instance layer.
     */
    GLsizei getInstanceCapacity() const { return _instanceSize; }
    /**
     * Returns the stride of the instance layer.
     *
     * The data loaded into the instance layer is expected to have the size of
     * this stride. If it does not, strange things will happen.
     *
     * @return the stride of of the instance layer.
     */
    GLsizei getInstanceStride() const { return _instanceStride; }
    
    /**
     * Loads the given instance buffer with data.
     *
     * The data loaded is the data that will be used at the next call to either
     * {@link #drawInstanced} or {@link #drawInstancedDirect}. It will be
     * combined with the vertex/index data the render the final image.
     *
     * The data loaded is expected to have the size of the instance buffer
     * stride. If it does not, strange things will happen.
     *
     * The usage is one of GL_STATIC_DRAW, GL_STREAM_DRAW, or GL_DYNAMIC_DRAW.
     * Static drawing should be reserved for vertices and/or indices that do not
     * change (so all animation happens in uniforms). Instance data is typically
     * streaming (as is with the case of particle systems), so it is generally
     * best to choose GL_STREAM_DRAW.
     *
     * This method will only succeed if this buffer is actively bound.
     *
     * @param data  The data to load
     * @param size  The number of instances to load
     * @param usage The type of data load
     */
    void loadInstanceData(const void * data, GLsizei size, GLenum usage=GL_STREAM_DRAW);
    
    /**
     * Draws to the active framebuffer using this instance buffer
     *
     * This draw command will instance the mesh defined by both the vertex
     * and index data. The count parameter determines the number of indices
     * to use in the base mesh, while the instances parameter determines the
     * number of indices. The optional paramter offset is for the offset into
     * the indices, not the instances.
     *
     * Any call to this command will use the current texture and uniforms. If
     * the texture and/or uniforms need to be changed, then this draw command
     * will need to be broken up into chunks. Use the optional parameter offset
     * to chunk up the draw calls without having to reload data.
     *
     * The drawing mode can be any of  GL_POINTS, GL_LINE_STRIP, GL_LINE_LOOP,
     * GL_LINES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN or GL_TRIANGLES.  These
     * are the only modes accepted by both OpenGL and OpenGLES. See the OpenGL
     * documentation for the number of indices required for each type.  In
     * practice the {@link Poly2} class is designed to support GL_POINTS,
     * GL_LINES, and GL_TRIANGLES only.
     *
     * This method will only succeed if this buffer is actively bound.
     *
     * @param mode      The OpenGLES drawing mode
     * @param count     The number of vertices in the base mesh
     * @param instances The number of mesh instances
     * @param offset    The initial index to start the mesh
     */
    void drawInstanced(GLenum mode, GLsizei count, GLsizei instances, GLint offset=0);
    
    /**
     * Draws to the active framebuffer using this instance buffer
     *
     * This draw command will instance the mesh defined in the vertex buffer
     * the given number of times. In defining the mesh, it will bypass the
     * index buffer, and draw the vertices in order starting from the first
     * specified vertex.
     *
     * Any call to this command will use the current texture and uniforms. If
     * the texture and/or uniforms need to be changed, then this draw command
     * will need to be broken up into chunks. Use the initial offset parameter
     * to chunk up the draw calls without having to reload data.
     *
     * The drawing mode can be any of  GL_POINTS, GL_LINE_STRIP, GL_LINE_LOOP,
     * GL_LINES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN or GL_TRIANGLES.  These
     * are the only modes accepted by both OpenGL and OpenGLES. See the OpenGL
     * documentation for the number of indices required for each type.  In
     * practice the {@link Poly2} class is designed to support GL_POINTS,
     * GL_LINES, and GL_TRIANGLES only.
     *
     * This method will only succeed if this buffer is actively bound.
     *
     * @param mode      The OpenGLES drawing mode
     * @param first     The initial vertex in the base mesh
     * @param count     The number of vertices in the base mesh
     * @param instances The number of mesh instances
     */
    void drawInstancedDirect(GLenum mode, GLint first, GLsizei count, GLsizei instances);
    
    
    
#pragma mark -
#pragma mark Attributes
    /**
     * Initializes an instance attribute, assigning is a size, type and offset.
     *
     * This method is necessary for the instance buffer to convey data to the
     * shader. Without it, the shader will used default values for the attribute
     * rather than data from the instance buffer.
     *
     * It is safe to call this method even when the shader is not attached.
     * The values will be cached and will be used to link this buffer to the
     * shader when the shader is attached.  This also means that a instance
     * buffer can swap shaders without having to reinitialize attributes.
     * If a shader is attached, the attribute will be enabled immediately.
     *
     * If the attribute does not refer to one supported by the active
     * shader, then it will be ignored (e.g. the effect is the same as
     * disabling the attribute).
     *
     * The attribute type can be one of GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT,
     * GL_UNSIGNED_SHORT, GL_INT, GL_UNSIGNED_INT, GL_HALF_FLOAT, GL_FLOAT,
     * GL_FIXED, or GL_INT_2_10_10_10_REV.  Doubles are not supported by
     * OpenGLES.
     *
     * The attribute offset is measured in bytes from the start of the
     * instance data structure (for a single instance).
     *
     * @param name      The attribute name
     * @param size      The attribute size in byte.
     * @param type      The attribute type
     * @param norm      Whether to normalize the value (floating point only)
     * @param offset    The attribute offset in the vertex data structure
     */
    void setupInstanceAttribute(const std::string name, GLint size, GLenum type,
                                GLboolean norm, GLsizei offset);
    
    
};

    }
}


#endif /* __CU_INSTANCE_BUFFER_H__ */
