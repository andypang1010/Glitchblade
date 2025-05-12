//
//  CUSpriteMesh.hpp
//  Cornell University Game Library (CUGL)
//
//  This module an alternative to SpriteBatch that implements a sprite via a
//  static mesh. It has its own vertex buffer, and is therefore meant to be
//  used directly with a shader instead of a SpriteBatch. With that said, the
//  vertices are all instances of SpriteVertex and thus it can be used with the
//  default SpriteBatch shader.
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
#include <cugl/graphics/CUSpriteMesh.h>

using namespace cugl;
using namespace cugl::graphics;


#pragma mark Constructors

/**
 * Initializes a sprite mesh with the given vertex mesh.
 *
 * The sprite mesh will only create a graphics buffer if buffer is true.
 * This is to handle cases where the sprite mesh is created in a separate
 * thread (as OpenGL only allows graphics buffers to be made on the main
 * thread).
 *
 * @param mesh      The sprite vertex mesh
 * @param buffer    Whether to create a graphics buffer
 *
 * @return true if initialization was successful
 */
bool SpriteMesh::initWithMesh(const Mesh<SpriteVertex>& mesh, bool buffer) {
    _mesh = mesh;
    if (buffer && _mesh.vertices.size()) {
        createVertexBuffer();
    }
    return true;
}

/**
 * Initializes a sprite mesh with the given vertex mesh.
 *
 * The sprite mesh will only create a graphics buffer if buffer is true.
 * This is to handle cases where the sprite mesh is created in a separate
 * thread (as OpenGL only allows graphics buffers to be made on the main
 * thread).
 *
 * @param mesh      The sprite vertex mesh
 * @param buffer    Whether to create a graphics buffer
 *
 * @return true if initialization was successful
 */
bool SpriteMesh::initWithMesh(Mesh<SpriteVertex>&& mesh, bool buffer) {
    _mesh = std::move(mesh);
    if (buffer && _mesh.vertices.size()) {
        createVertexBuffer();
    }
    return true;
}

/**
 * Initializes a sprite mesh from the given JsonValue
 *
 * This JSON object can either be an array or an object. If it is an array,
 * it should contain JSON values compatible with the {@link SpriteVertex}
 * constructor. If it is a JSON object, then it supports the following
 * attributes:
 *
 *     "vertices":      An array {@link SpriteVertex} descriptions
 *     "indices":       An intenger list of triangle indices (in multiples of 3)
 *     "triangulator":  One of 'monotone', 'earclip', 'delaunay', 'fan', or 'strip'
 *
 * All attributes are optional. If "vertices" are missing, the mesh will
 * be empty. If both "indices" and "triangulator" are missing, the mesh
 * will use a triangle fan. The "triangulator" choice will only be applied
 * if the "indices" are missing.
 *
 * The sprite mesh will only create a graphics buffer if buffer is true.
 * This is to handle cases where the sprite mesh is created in a separate
 * thread (as OpenGL only allows graphics buffers to be made on the main
 * thread).
 *
 * @param data      The JSON object specifying the sprite mesh
 * @param buffer    Whether to create a graphics buffer
 *
 * @return true if initialization was successful
 */
bool SpriteMesh::initWithData(const std::shared_ptr<JsonValue>& data,
                              bool buffer) {
    parse_mesh<SpriteVertex>(_mesh, data);
    if (buffer && _mesh.vertices.size()) {
        createVertexBuffer();
    }
    return true;
}

/**
 * Disposes the mesh and vertex buffer for this sprite mesh.
 *
 * You must reinitialize the sprite mesh to use it.
 */
void SpriteMesh::dispose() {
    _mesh.clear();
    _buffer = nullptr;
}

#pragma mark Drawing Methods
/**
 * Draws the sprite mesh with the given shader.
 *
 * Note that sprite meshes to do not necessarily come with an texture or
 * a gradient; they are just a mesh. Those values should be passed to the
 * shader separately as uniforms.
 *
 * @param shader    The shader to draw with
 */
void SpriteMesh::draw(const std::shared_ptr<Shader>& shader) {
    if (_buffer == nullptr) {
        return;
    }
    
    _buffer->attach(shader);
    _buffer->draw(_mesh.command, (GLsizei)_mesh.indices.size());
    _buffer->unbind();
}

/**
 * Allocates the vertex buffer for this billboard.
 *
 * @return true if the buffer was successfully created
 */
bool SpriteMesh::createVertexBuffer() {
    _buffer = VertexBuffer::alloc((GLsizei)_mesh.vertices.size(),
                                  (GLsizei)sizeof(SpriteVertex));
    if (_buffer == nullptr) {
        return false;
    }
    
    // Set up attributes
    _buffer->setupAttribute("aPosition", 2, GL_FLOAT, GL_FALSE,
                            offsetof(SpriteVertex,position));
    _buffer->setupAttribute("aColor",    4, GL_UNSIGNED_BYTE, GL_TRUE,
                            offsetof(SpriteVertex,color));
    _buffer->setupAttribute("aTexCoord", 2, GL_FLOAT, GL_FALSE,
                            offsetof(SpriteVertex,texcoord));
    _buffer->setupAttribute("aGradCoord",2, GL_FLOAT, GL_FALSE,
                            offsetof(SpriteVertex,gradcoord));

    
    // Load the static data
    _buffer->loadVertexData(_mesh.vertices.data(),
                            (GLsizei)_mesh.vertices.size(),
                            GL_STATIC_DRAW);
    _buffer->loadIndexData( _mesh.indices.data(),
                           (GLsizei)_mesh.indices.size(),
                           GL_STATIC_DRAW);
    _buffer->unbind();
    return true;
}
