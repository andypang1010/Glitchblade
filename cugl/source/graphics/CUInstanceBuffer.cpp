//
//  CUInstanceBuffer.cpp
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
#include <cugl/graphics/CUInstanceBuffer.h>
#include <cugl/graphics/CUShader.h>
#include <cugl/core/util/CUDebug.h>

using namespace cugl;
using namespace cugl::graphics;

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
bool InstanceBuffer::init(GLsizei tsize, GLsizei tstride, GLsizei isize, GLsizei istride) {
    if (!VertexBuffer::init(tsize,tstride)) {
        return false;
    }
    
    _instanceSize = isize;
    _instanceStride = istride;
    glGenBuffers(1, &_instanceBuffer);
    if (!_instanceBuffer) {
        GLenum error = glGetError();
        CULogError("Could not create index buffer. %s", gl_error_name(error).c_str());
        VertexBuffer::dispose();
        return false;
    }
    
    return true;
}

/**
 * Deletes the instance buffer, freeing all resources.
 *
 * You must reinitialize the instance buffer to use it.
 */
void InstanceBuffer::dispose() {
    if (_instanceBuffer) {
        glDeleteBuffers(1,&_instanceBuffer);
        _instanceBuffer = 0;
    }
    VertexBuffer::dispose();
}


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
void InstanceBuffer::attach(const std::shared_ptr<Shader>& shader) {
    bool activate = (_shader != shader);
    VertexBuffer::attach(shader);
    if (activate) {
        // Add on the instance attributes
        glBindBuffer( GL_ARRAY_BUFFER, _instanceBuffer );

        // Link up attributes on the first time
        for(auto it = _instAttribs.begin(); it != _instAttribs.end(); ++it) {
            std::string name = it->first;
            GLint pos = glGetAttribLocation(_shader->getProgram(), name.c_str());
            if (pos == -1) {
                CUWarn("Active shader has no attribute %s", name.c_str());
            } else if (_enabled[name]) {
                glEnableVertexAttribArray(pos);
                glVertexAttribPointer(pos,it->second.size,it->second.type,
                                      it->second.norm,_instanceStride,
                                      reinterpret_cast<void*>(it->second.offset));
                glVertexAttribDivisor(pos,1);
            } else {
                glDisableVertexAttribArray(pos);
            }
        }

        GLenum error = glGetError();
        CUAssertLog(error == GL_NO_ERROR, "InstanceBuffer: %s", gl_error_name(error).c_str());
    }
}

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
void InstanceBuffer::loadInstanceData(const void * data, GLsizei size, GLenum usage) {
    // Assert causes problems on android emulator for now
    //CUAssertLog(isBound(), "Instance buffer is not bound");
    CUAssertLog(size <= _instanceSize, "Data exceeds maximum capacity: %d > %d",size,_instanceSize);
    glBindBuffer( GL_ARRAY_BUFFER, _instanceBuffer );
    if (usage == GL_STATIC_DRAW) {
        glBufferData( GL_ARRAY_BUFFER, _instanceStride * size, data, usage );
    } else {
        // Buffer orphaning
        glBufferData(GL_ARRAY_BUFFER, _instanceStride*_instanceSize, NULL, usage);
        glBufferSubData(GL_ARRAY_BUFFER, 0, _instanceStride*size, data);
    }
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    GLenum error = glGetError();
    CUAssertLog(error == GL_NO_ERROR, "InstanceBuffer: %s", gl_error_name(error).c_str());

}

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
void InstanceBuffer::drawInstanced(GLenum mode, GLsizei count, GLsizei instances, GLint offset) {
    // Assert causes problems on android emulator for now
    //CUAssertLog(isBound(), "Vertex buffer is not bound");
    glDrawElementsInstanced(mode, count, GL_UNSIGNED_INT, (void*)(offset * sizeof(GLuint)), instances);
}
    
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
void InstanceBuffer::drawInstancedDirect(GLenum mode, GLint first, GLsizei count, GLsizei instances) {
    // Assert causes problems on android emulator for now
    //CUAssertLog(isBound(), "Vertex buffer is not bound");
    glDrawArraysInstanced(mode, first, count, instances);
}



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
void InstanceBuffer::setupInstanceAttribute(const std::string name, GLint size, GLenum type,
                                            GLboolean norm, GLsizei offset) {
    AttribData data;
    data.size = size;
    data.norm = norm;
    data.type = type;
    data.offset = offset;
    _instAttribs[name] = data;
    _enabled[name] = true;
    
    if (_shader != nullptr) {
        _shader->bind();
        GLint pos = glGetAttribLocation(_shader->getProgram(), name.c_str());
        if (pos == -1) {
            CUWarn("Active shader has no attribute %s", name.c_str());
        } else {
            glEnableVertexAttribArray(pos);
            glVertexAttribPointer(pos,data.size,data.type,data.norm,_instanceStride,
                                  reinterpret_cast<GLvoid*>(data.offset));
            glVertexAttribDivisor(pos,1);
        }
        
        GLenum error = glGetError();
        CUAssertLog(error == GL_NO_ERROR, "VertexBuffer: %s", gl_error_name(error).c_str());
    }
}
