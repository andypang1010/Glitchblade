//
//  CUSpriteMesh.h
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
#ifndef __CU_SPRITE_MESH_H__
#define __CU_SPRITE_MESH_H__
#include <cugl/graphics/CUMesh.h>
#include <cugl/graphics/CUSpriteVertex.h>
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

/**
 * This class is represents a sprite with a static mesh.
 *
 * Sprites are typically drawn with a {@link SpriteBatch}. Sprite batches work
 * best when geometric complexity is low, as they are constantly updating their
 * {@link VertexBuffer}. However, if a sprite does not change much, it sometimes
 * makes sense to assign it its own {@link VertexBuffer}. That is the purpose of
 * this class.
 *
 * Because a sprite mesh has its own buffer, it is intended to be used directly
 * with a {@link Shader}. With that said, the vertices are all instances of
 * {@link SpriteVertex}, and thus this class can be used with the default
 * {@link SpriteBatch#getShader}.
 */
class SpriteMesh {
private:
    /** The sprite mesh */
    Mesh<SpriteVertex> _mesh;
    /** The internal billboard vertex buffer */
    std::shared_ptr<VertexBuffer> _buffer;
    
public:
#pragma mark Constructors
    /**
     * Creates a degenerate sprite mesh
     *
     * This sprite mesh has degenerate values for all attributes. Nothing will
     * rendered to the screen until it is initialized.
     */
    SpriteMesh() {}
    
    /**
     * Deletes this sprite mesh, disposing all of the resources.
     */
    ~SpriteMesh() { dispose(); }
    
    /**
     * Disposes the mesh and vertex buffer for this sprite mesh.
     *
     * You must reinitialize the sprite mesh to use it.
     */
    void dispose();
    
    /**
     * Initializes an empty sprite mesh.
     *
     * The sprite mesh will have an empty mesh and therefore not create an
     * a graphics buffer. It will need a mesh provided to {@link #setMesh}.
     *
     * @return true if initialization was successful
     */
    bool init() {
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
    bool initWithMesh(const Mesh<SpriteVertex>& mesh, bool buffer=true);
    
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
    bool initWithMesh(Mesh<SpriteVertex>&& mesh, bool buffer=true);
    
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
    bool initWithData(const std::shared_ptr<JsonValue>& data,
                      bool buffer=true);
    
    /**
     * Returns a newly allocated empty sprite mesh.
     *
     * The sprite mesh will have an empty mesh and therefore not create an
     * a graphics buffer. It will need a mesh provided to {@link #setMesh}.
     *
     * @return a newly allocated empty sprite mesh.
     */
    static std::shared_ptr<SpriteMesh> alloc() {
        std::shared_ptr<SpriteMesh> result = std::make_shared<SpriteMesh>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated sprite mesh with the given vertex mesh.
     *
     * The sprite mesh will only create a graphics buffer if buffer is true.
     * This is to handle cases where the sprite mesh is created in a separate
     * thread (as OpenGL only allows graphics buffers to be made on the main
     * thread).
     *
     * @param mesh      The sprite vertex mesh
     * @param buffer    Whether to create a graphics buffer
     *
     * @return a newly allocated sprite mesh with the given vertex mesh.
     */
    static std::shared_ptr<SpriteMesh> allocWithMesh(const Mesh<SpriteVertex>& mesh,
                                                     bool buffer=true) {
        std::shared_ptr<SpriteMesh> result = std::make_shared<SpriteMesh>();
        return (result->initWithMesh(mesh,buffer) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated sprite mesh with the given vertex mesh.
     *
     * The sprite mesh will only create a graphics buffer if buffer is true.
     * This is to handle cases where the sprite mesh is created in a separate
     * thread (as OpenGL only allows graphics buffers to be made on the main
     * thread).
     *
     * @param mesh      The sprite vertex mesh
     * @param buffer    Whether to create a graphics buffer
     *
     * @return a newly allocated sprite mesh with the given vertex mesh.
     */
    static std::shared_ptr<SpriteMesh> allocWithMesh(Mesh<SpriteVertex>&& mesh,
                                                     bool buffer=true) {
        std::shared_ptr<SpriteMesh> result = std::make_shared<SpriteMesh>();
        return (result->initWithMesh(mesh,buffer) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated sprite mesh from the given JsonValue
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
     * @return a newly allocated sprite mesh from the given JsonValue
     */
    static std::shared_ptr<SpriteMesh> allocWithData(const std::shared_ptr<JsonValue>& data,
                                                     bool buffer=true) {
        std::shared_ptr<SpriteMesh> result = std::make_shared<SpriteMesh>();
        return (result->initWithData(data,buffer) ? result : nullptr);
    }
    
#pragma mark Attributes
    /**
     * Returns the mesh associated with sprite mesh.
     *
     * This mesh can be safely changed at any time. However, changing the mesh
     * affects the {@link #getVertexBuffer}. Therefore, the mesh should
     * never be modified directly. Changes should go through {@link #setMesh}.
     *
     * @return the mesh associated with sprite mesh.
     */
    const Mesh<SpriteVertex> getMesh() const { return _mesh; }
    
    /**
     * Sets the mesh associated with sprite mesh.
     *
     * This mesh can be safely changed at any time. However, changing the mesh
     * affects the {@link #getVertexBuffer}. Therefore, the mesh should
     * never be modified directly. Changes should go through {@link #setMesh}.
     *
     * This method will always create a graphics buffer, even if one did not
     * previously exist. It should not be called outside the main thread.
     *
     * @param mesh  The mesh associated with sprite mesh.
     */
    void setMesh(const Mesh<SpriteVertex>& mesh) {
        _mesh = mesh;
        createVertexBuffer();
    }
    
    /**
     * Sets the mesh associated with sprite mesh.
     *
     * This mesh can be safely changed at any time. However, changing the mesh
     * affects the {@link #getVertexBuffer}. Therefore, the mesh should
     * never be modified directly. Changes should go through {@link #setMesh}.
     *
     * This method will always create a graphics buffer, even if one did not
     * previously exist. It should not be called outside the main thread.
     *
     * @param mesh  The mesh associated with sprite mesh.
     */
    void setMesh(Mesh<SpriteVertex>&& mesh) {
        _mesh = std::move(mesh);
        createVertexBuffer();
    }
    
#pragma mark Rendering
    /**
     * Returns the vertex buffer for this sprite mesh.
     *
     * This buffer is used to render the mesh. It should be combined with a
     * {@link Shader}, preferably one compatible with {@link SpriteBatch}.
     *
     * @return the vertex buffer for this billboard.
     */
    std::shared_ptr<VertexBuffer> getVertexBuffer() const { return _buffer; }
    
    /**
     * Draws the sprite mesh with the given shader.
     *
     * Note that sprite meshes to do not necessarily come with an texture or
     * a gradient; they are just a mesh. Those values should be passed to the
     * shader separately as uniforms.
     *
     * @param shader    The shader to draw with
     */
    void draw(const std::shared_ptr<Shader>& shader);
    
private:
    /**
     * Allocates the vertex buffer for this billboard.
     *
     * @return true if the buffer was successfully created
     */
    bool createVertexBuffer();
    
    friend class SpriteMeshLoader;
    
};
    }
}

#endif /* __CU_SPRITE_MESH_H__ */
