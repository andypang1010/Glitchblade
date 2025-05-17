//
//  CUMeshNode.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a mesh node for mesh scene graph nodes.
//  This classes utilizes polygons to create meshes. Unlike a PolygonNode,
//  colors on meshes can be set direcly. In addition, the user has control
//  over the interior vertices.
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
//  Author: Allison Hsu
//  Version: 7/3/24 (CUGL 3.0 reorganization)
//
#ifndef __CU_MESH_NODE_H__
#define __CU_MESH_NODE_H__

#include <cugl/scene2/CUTexturedNode.h>
#include <cugl/core/math/CURect.h>
#include <cugl/graphics/CUTexture.h>


namespace cugl {

    /**
     * The classes to construct a 2-d scene graph.
     *
     * Even though this is an optional package, this is one of the core features
     * of CUGL. These classes provide basic UI support (including limited Figma)
     * support. Any 2-d game will make extensive use of these classes. And
     * even 3-d games may use these classes for the HUD overlay.
     */
    namespace scene2 {

#pragma mark -
#pragma mark MeshNode
/**
 * This is a scene graph node to support mesh manipulation
 *
 * The API for this class is very similar to {@link PolygonNode}, except that
 * the use specifies a mesh directly (instead of inferring it from the shape).
 * This allows the user direct control over the interior vertices, and the
 * individual vertex colors.
 *
 * Unlike polgon nodes, all mesh nodes use absolute positioning by default.
 */
class MeshNode : public TexturedNode {
protected:
    /** Used to keep track of the current flip state */
    int _flipFlags;
    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates an empty mesh with the degenerate texture.
     *
     * You must initialize this MeshNode before use.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    MeshNode() : TexturedNode(), _flipFlags(0) {
        _absolute = true;
        _classname = "MeshNode";
    }
    
    /**
     * Releases all resources allocated with this node.
     *
     * This will release, but not necessarily delete the associated texture.
     * However, the polygon and drawing commands will be deleted and no
     * longer safe to use.
     */
    ~MeshNode() { dispose(); }
    
    /**
     * Disposes all of the resources used by this node.
     *
     * A disposed Node can be safely reinitialized. Any children owned by this
     * node will be released.  They will be deleted if no other object owns them.
     *
     * It is unsafe to call this on a Node that is still currently inside of
     * a scene graph.
     */
    virtual void dispose() override;

    /**
     * Initializes a mesh node as a default equilateral triangle mesh.
     *
     * The mesh will use the texture {@link graphics::Texture#getBlank}, which 
     * is suitable for drawing solid shapes. The vertex colors will be blue,
     * red, and yellow.
     *
     * @return true if the mesh node is initialized properly, false otherwise.
     */
    virtual bool init() override;
    
    /**
     * Initializes a mesh node from the current mesh
     *
     * The mesh will use the texture {@link graphics::Texture#getBlank}, which
     * is suitable for drawing solid shapes.
     *
     * @param mesh  The mesh data
     *
     * @return true if the sprite is initialized properly, false otherwise.
     */
    bool initWithMesh(const graphics::Mesh<graphics::SpriteVertex>& mesh) {
        return initWithTextureMesh(nullptr, mesh);
    }

    /**
     * Initializes a mesh node using from a polygon.
     *
     * This method uses the polygon to construct a mesh for the mesh node.
     * The colors of all the vertices will be white.
     *
     * The mesh will use the texture {@link graphics::Texture#getBlank}, which
     * is suitable for drawing solid shapes.
     *
     * @param poly      The polygon to define the mesh
     *
     * @return true if the sprite is initialized properly, false otherwise.
     */
    bool initWithPoly(const Poly2& poly) {
        return initWithTexturePoly(nullptr, poly);
    }
    
    /**
     * Initializes a mesh node using from a polygon and set of colors.
     *
     * This method uses the polygon to construct a mesh for the mesh node.
     * The vertices are assigned the respective colors from the colors vector,
     * in the order that they are specified in the polygon.
     *
     * The mesh will use the texture {@link graphics::Texture#getBlank}, which
     * is suitable for drawing solid shapes.
     *
     * @param poly      The polygon to define the mesh
     * @param colors    The vertex colors
     *
     * @return true if the sprite is initialized properly, false otherwise.
     */
    bool initWithPoly(const Poly2& poly, const std::vector<Color4>& colors) {
        return initWithTexturePoly(nullptr, poly, colors);
    }
    
    /**
     * Initializes a mesh node from the image filename.
     *
     * After creation, the mesh will be a rectangle. The vertices of this
     * mesh will be the corners of the image. The rectangle will be
     * triangulated with the standard two triangles. The colors of all the
     * vertices will be white.
     *
     * @param filename  A path to image file, e.g., "scene1/earthtile.png"
     *
     * @return  true if the sprite is initialized properly, false otherwise.
     */
    virtual bool initWithFile(const std::string filename) override;

    /**
     * Initializes a mesh node from the image filename and mesh
     *
     * The texture coordinates in the mesh will determine how to interpret
     * the texture.
     *
     * @param filename  A path to image file, e.g., "scene1/earthtile.png"
     * @param mesh      The mesh data
     *
     * @return  true if the sprite is initialized properly, false otherwise.
     */
    bool initWithFileMesh(const std::string filename,
                          const graphics::Mesh<graphics::SpriteVertex>& mesh);

    /**
     * Initializes a mesh node from the image filename and the given polygon.
     *
     * This method uses the polygon to construct a mesh for the mesh node.
     * The vertices will all have color white
     *
     * @param filename  A path to image file, e.g., "scene1/earthtile.png"
     * @param poly      The polygon to define the mesh
     *
     * @return  true if the sprite is initialized properly, false otherwise.
     */
    bool initWithFilePoly(const std::string filename, const Poly2& poly);

    /**
     * Initializes a mesh node from the image filename and the given polygon.
     *
     * This method uses the polygon to construct a mesh for the mesh node.
     * The vertices are assigned the respective colors from the colors vector,
     * in the order that they are specified in the polygon.
     *
     * @param filename  A path to image file, e.g., "scene1/earthtile.png"
     * @param poly      The polygon to define the mesh
     * @param colors    The vertex colors
     *
     * @return  true if the sprite is initialized properly, false otherwise.
     */
    bool initWithFilePoly(const std::string filename, const Poly2& poly,
                          const std::vector<Color4>& colors);
    
    /**
     * Initializes a mesh node from a Texture object.
     *
     * After creation, the mesh will be a rectangle. The vertices of this
     * mesh will be the corners of the image. The rectangle will be
     * triangulated with the standard two triangles. The colors of all the
     * vertices will be white.
     *
     * @param texture   A shared pointer to a Texture object.
     *
     * @return  true if the sprite is initialized properly, false otherwise.
     */
    virtual bool initWithTexture(const std::shared_ptr<graphics::Texture>& texture) override;
    
    /**
     * Initializes a mesh node from the Texture object and mesh
     *
     * The texture coordinates in the mesh will determine how to interpret
     * the texture.
     *
     * @param texture   A shared pointer to a Texture object.
     * @param mesh      The mesh data
     *
     * @return  true if the sprite is initialized properly, false otherwise.
     */
    bool initWithTextureMesh(const std::shared_ptr<graphics::Texture>& texture,
                             const graphics::Mesh<graphics::SpriteVertex>& mesh);

    /**
     * Initializes a mesh node from the Texture object and the given polygon.
     *
     * This method uses the polygon to construct a mesh for the mesh node.
     * The vertices will all have color white
     *
     * @param texture   A shared pointer to a Texture object.
     * @param poly      The polygon to define the mesh
     *
     * @return  true if the sprite is initialized properly, false otherwise.
     */
    bool initWithTexturePoly(const std::shared_ptr<graphics::Texture>& texture,
                             const Poly2& poly);

    /**
     * Initializes a mesh node from the Texture object and the given polygon.
     *
     * This method uses the polygon to construct a mesh for the mesh node.
     * The vertices are assigned the respective colors from the colors vector,
     * in the order that they are specified in the polygon.
     *
     * @param texture   A shared pointer to a Texture object.
     * @param poly      The polygon to define the mesh
     * @param colors    The vertex colors
     *
     * @return  true if the sprite is initialized properly, false otherwise.
     */
    bool initWithTexturePoly(const std::shared_ptr<graphics::Texture>& texture,
                             const Poly2& poly,
                             const std::vector<Color4>& colors);

    /**
     * Initializes a mesh node with the given JSON specification.
     *
     * This initializer is designed to receive the "data" object from the
     * JSON passed to {@link Scene2Loader}. This JSON format supports all
     * of the attribute values of its parent class.  In addition, it supports
     * the following additional attributes:
     *
     *     "mesh":  A JSON object defining a mesh of SpriteVertex
     *
     * This JSON object for mesh is required. The mesh can be represented as
     * either an array or JSON object. If it is an array, it contains JSON
     * values compatible with the {@link graphics::SpriteVertex} constructor.
     * If it is a JSON object, then it supports the following attributes:
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
     * @param manager   The asset manager handling this asset
     * @param data      The JSON object specifying the node
     *
     * @return true if initialization was successful.
     */
    virtual bool initWithData(const AssetManager* manager,
                              const std::shared_ptr<JsonValue>& data) override;
    
    /**
     * Performs a shallow copy of this Node into dst.
     *
     * No children from this node are copied, and no children of dst are
     * modified. In addition, the parents of both Nodes are unchanged. However,
     * all other attributes of this node are copied.
     *
     * @param dst   The Node to copy into
     *
     * @return A reference to dst for chaining.
     */
    virtual std::shared_ptr<SceneNode> copy(const std::shared_ptr<SceneNode>& dst) const override;

    
#pragma mark -
#pragma mark Static Constructors
    /**
     * Returns a default mesh node.
     *
     * The mesh will use the texture {@link graphics::Texture#getBlank}, which
     * is suitable for drawing solid shapes. The vertex colors will be blue,
     * red, and yellow.
     *
     * @return a newly allocated default mesh node.
     */
    static std::shared_ptr<MeshNode> alloc() {
        std::shared_ptr<MeshNode> node = std::make_shared<MeshNode>();
        return (node->init() ? node : nullptr);
    }

    /**
     * Intializes a mesh node from the current mesh
     *
     * The mesh will use the texture {@link graphics::Texture#getBlank}, which
     * is suitable for drawing solid shapes.
     *
     * @param mesh  The mesh data
     *
     * @return true if the sprite is initialized properly, false otherwise.
     */
    static std::shared_ptr<SceneNode> allocWithMesh(const graphics::Mesh<graphics::SpriteVertex>& mesh) {
        std::shared_ptr<MeshNode> node = std::make_shared<MeshNode>();
        return (node->initWithMesh(mesh) ? node : nullptr);

    }

    /**
     * Returns a newly allocated mesh node using from a polygon and set of colors.
     *
     * This method uses the polygon to construct a mesh for the mesh node.
     * The vertices are assigned the respective colors from the colors vector,
     * in the order that they are specified in the polygon.
     *
     * The mesh will use the texture {@link graphics::Texture#getBlank}, which
     * is suitable for drawing solid shapes.
     *
     * @param poly      The polygon to define the mesh
     * @param colors    The vertex colors
     *
     * @return a newly allocated mesh node using from a polygon and set of colors.
     */
    static std::shared_ptr<SceneNode> allocWithPoly(const Poly2& poly,
                                                    const std::vector<Color4> colors) {
        std::shared_ptr<MeshNode> node = std::make_shared<MeshNode>();
        return (node->initWithPoly(poly, colors) ? node : nullptr);
    }
    
    /**
     * Returns a newly allocated mesh node from the image filename.
     *
     * After creation, the mesh will be a rectangle. The vertices of this
     * mesh will be the corners of the image. The rectangle will be
     * triangulated with the standard two triangles. The colors of all the
     * vertices will be white.
     *
     * @param filename  A path to image file, e.g., "scene1/earthtile.png"
     *
     * @return  true if the sprite is initialized properly, false otherwise.
     */
    static std::shared_ptr<SceneNode> allocWithFile(const std::string filename) {
        std::shared_ptr<MeshNode> node = std::make_shared<MeshNode>();
        return (node->initWithFile(filename) ? node : nullptr);
    }

    /**
     * Returns a newly allocated mesh node from the image filename and mesh
     *
     * The texture coordinates in the mesh will determine how to interpret
     * the texture.
     *
     * @param filename  A path to image file, e.g., "scene1/earthtile.png"
     * @param mesh      The mesh data
     *
     * @return  true if the sprite is initialized properly, false otherwise.
     */
    static std::shared_ptr<SceneNode> allocWithFileMesh(const std::string filename,
                                                        const graphics::Mesh<graphics::SpriteVertex>& mesh) {
        std::shared_ptr<MeshNode> node = std::make_shared<MeshNode>();
        return (node->initWithFileMesh(filename,mesh) ? node : nullptr);
    }

    /**
     * Returns a newly allocated mesh node from the image filename and the given polygon.
     *
     * This method uses the polygon to construct a mesh for the mesh node.
     * The vertices will all have color white
     *
     * @param filename  A path to image file, e.g., "scene1/earthtile.png"
     * @param poly      The polygon to define the mesh
     *
     * @return  true if the sprite is initialized properly, false otherwise.
     */
    static std::shared_ptr<SceneNode> allocWithFilePoly(const std::string filename,
                                                        const Poly2& poly) {
        std::shared_ptr<MeshNode> node = std::make_shared<MeshNode>();
        return (node->initWithFilePoly(filename,poly) ? node : nullptr);
    }


    /**
     * Returns a newly allocated mesh node from the image filename and the given polygon.
     *
     * This method uses the polygon to construct a mesh for the mesh node.
     * The vertices are assigned the respective colors from the colors vector,
     * in the order that they are specified in the polygon.
     *
     * @param filename  A path to image file, e.g., "scene1/earthtile.png"
     * @param poly      The polygon to define the mesh
     * @param colors    The vertex colors
     *
     * @return  true if the sprite is initialized properly, false otherwise.
     */
    static std::shared_ptr<SceneNode> allocWithFilePoly(const std::string filename,
                                                        const Poly2& poly,
                                                        std::vector<Color4> colors) {
        std::shared_ptr<MeshNode> node = std::make_shared<MeshNode>();
        return (node->initWithFilePoly(filename,poly,colors) ? node : nullptr);
    }
    
    /**
     * Returns a newly allocated mesh node from a Texture object.
     *
     * After creation, the mesh will be a rectangle. The vertices of this
     * mesh will be the corners of the image. The rectangle will be
     * triangulated with the standard two triangles. The colors of all the
     * vertices will be white.
     *
     * @param texture   A shared pointer to a Texture object.
     *
     * @return  true if the sprite is initialized properly, false otherwise.
     */
    static std::shared_ptr<SceneNode> allocWithTexture(const std::shared_ptr<graphics::Texture>& texture) {
        std::shared_ptr<MeshNode> node = std::make_shared<MeshNode>();
        return (node->initWithTexture(texture) ? node : nullptr);
    }
    
    /**
     * Returns a newly allocated mesh node from the Texture object and mesh
     *
     * The texture coordinates in the mesh will determine how to interpret
     * the texture.
     *
     * @param texture   A shared pointer to a Texture object.
     * @param mesh      The mesh data
     *
     * @return  true if the sprite is initialized properly, false otherwise.
     */
    static std::shared_ptr<SceneNode> allocWithTextureMesh(const std::shared_ptr<graphics::Texture>& texture,
                                                           const graphics::Mesh<graphics::SpriteVertex>& mesh) {
        std::shared_ptr<MeshNode> node = std::make_shared<MeshNode>();
        return (node->initWithTextureMesh(texture,mesh) ? node : nullptr);
    }

    /**
     * Returns a newly allocated mesh node from the Texture object and the given polygon.
     *
     * This method uses the polygon to construct a mesh for the mesh node.
     * The vertices will all have color white
     *
     * @param texture   A shared pointer to a Texture object.
     * @param poly      The polygon to define the mesh
     *
     * @return  true if the sprite is initialized properly, false otherwise.
     */
    static std::shared_ptr<SceneNode> allocWithTexturePoly(const std::shared_ptr<graphics::Texture>& texture,
                                                           const Poly2& poly) {
        std::shared_ptr<MeshNode> node = std::make_shared<MeshNode>();
        return (node->initWithTexturePoly(texture,poly) ? node : nullptr);
    }

    /**
     * Returns a newly allocated mesh node from the Texture object and the given polygon.
     *
     * This method uses the polygon to construct a mesh for the mesh node.
     * The vertices are assigned the respective colors from the colors vector,
     * in the order that they are specified in the polygon.
     *
     * @param texture   A shared pointer to a Texture object.
     * @param poly      The polygon to define the mesh
     * @param colors    The vertex colors
     *
     * @return  true if the sprite is initialized properly, false otherwise.
     */
    static std::shared_ptr<SceneNode> allocWithTexturePoly(const std::shared_ptr<graphics::Texture>& texture,
                                                           const Poly2& poly,
                                                           std::vector<Color4> colors) {
        std::shared_ptr<MeshNode> node = std::make_shared<MeshNode>();
        return (node->initWithTexturePoly(texture,poly,colors) ? node : nullptr);
    }
    
    /**
     * Returns a newly allocated mesh node with the given JSON specification.
     *
     * This initializer is designed to receive the "data" object from the
     * JSON passed to {@link Scene2Loader}. This JSON format supports all
     * of the attribute values of its parent class. In addition, it supports
     * the following additional attributes:
     *
     *     "mesh":  A JSON object defining a mesh of SpriteVertex
     *
     * This JSON object for mesh is required. The mesh can be represented as
     * either an array or JSON object. If it is an array, it contain JSON
     * values compatible with the {@link graphics::SpriteVertex} constructor.
     * If it is a JSON object, then it supports the following attributes:
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
     * @param manager   The asset manager handling this asset
     * @param data      The JSON object specifying the node
     *
     * @return a newly allocated mesh node with the given JSON specification.
     */
    static std::shared_ptr<SceneNode> allocWithData(const AssetManager* manager,
                                                    const std::shared_ptr<JsonValue>& data) {
        std::shared_ptr<MeshNode> result = std::make_shared<MeshNode>();
        if (!result->initWithData(manager,data)) { result = nullptr; }
        return std::dynamic_pointer_cast<SceneNode>(result);
    }
    
#pragma mark -
#pragma mark Mesh Attributes
    /**
     * Sets the mesh for the mesh node.
     *
     * @param mesh  The updated mesh.
     */
    void setMesh(const graphics::Mesh<graphics::SpriteVertex>& mesh);

    /**
     * Sets the mesh for the mesh node.
     *
     * @param mesh  The updated mesh.
     */
    void setMesh(graphics::Mesh<graphics::SpriteVertex>&& mesh);

    /**
     * Returns a reference to the underlying mesh.
     *
     * @return a reference to the underlying mesh.
     */
    const graphics::Mesh<graphics::SpriteVertex>& getMesh() const {
        return _mesh;
    }
    
    /**
     * Returns a reference to the underlying mesh.
     *
     * @return a reference to the underlying mesh.
     */
    graphics::Mesh<graphics::SpriteVertex>& getMesh() {
        return _mesh;
    }

    /**
     * Returns a pointer to the sprite vertex as the given index
     *
     * This sprite vertex can be updated to change the vertex position,
     * color, texture coordinates, or gradient coordinates. If there is
     * not vertex as that index, this method returns nullptr.
     *
     * @param index The sprite index
     *
     * @return a pointer to the sprite vertex as the given index
     */
    const graphics::SpriteVertex* getVertex(size_t index) const;
        
    /**
     * Returns a pointer to the sprite vertex as the given index
     *
     * This sprite vertex can be updated to change the vertex position,
     * color, texture coordinates, or gradient coordinates. If there is
     * not vertex as that index, this method returns nullptr.
     *
     * @param index The sprite index
     *
     * @return a pointer to the sprite vertex as the given index
     */
    graphics::SpriteVertex* getVertex(size_t index);
    
    /**
     * Sets the colors of the mesh vertices.
     *
     * The parameter vector should have a size equal to the number of 
     * vertices. If it is too long, extra colors are ignored. If it is too 
     * short, the the final color will be used for all remaining vertices.
     *
     * @param colors    The vertex colors
     */
    void setVertexColors(const std::vector<Color4>& colors);
    
    /**
     * Sets the texture coordinates of the mesh vertices.
     *
     * The parameter vector should have a size equal to the number of 
     * vertices. If it is too long, extra coordinates are ignored. If it is 
     * too short, the the final texture coordinate will be used for all 
     * remaining vertices.
     *
     * @param coords    The texture coordinates
     */
    void setVertexTexCoords(const std::vector<Vec2>& coords);
    
    /**
     * Sets the gradient coordinates of the mesh vertices.
     *
     * The parameter vector should have a size equal to the number of 
     * vertices. If it is too long, extra coordinates are ignored. If it is 
     * too short, the the final texture coordinate will be used for all 
     * remaining vertices.
     *
     * @param coords    The gradient coordinates
     */
    void setVertexGradCoords(const std::vector<Vec2>& coords);

    /**
     * Returns the rect of the mesh node in points
     *
     * The bounding rect is the smallest rectangle containing all
     * of the points in the mesh.
     *
     * This value also defines the content size of the node. The
     * mesh will be shifted so that its bounding rect is centered
     * at the node center.
     */
    const Rect getBoundingRect() const;
    
    /**
     * Sets the mesh to match the given polygon.
     *
     * The mesh textures and colors will be recomputed as if this were a
     * {@link PolygonNode}.
     *
     * @param poly  The new poly2 object.
     */
    void setPolygon(const Poly2& poly);

#pragma mark -
#pragma mark Rendering
    /**
     * Draws this mesh node via the given SpriteBatch.
     *
     * This method only worries about drawing the current node.  It does not
     * attempt to render the children.
     *
     * @param batch     The SpriteBatch to draw with.
     * @param transform The global transformation matrix.
     * @param tint      The tint to blend with the Node color.
     */
    virtual void draw(const std::shared_ptr<graphics::SpriteBatch>& batch,
                      const Affine2& transform, Color4 tint) override;

    
#pragma mark -
#pragma mark Internal Helpers
private:
    /**
     * Allocate the render data necessary to render this node.
     *
     * This method does nothing, as all render data is specified by the mesh.
     */
    virtual void generateRenderData() override {}

    /**
     * Updates the texture coordinates for this polygon
     *
     * This method inverts texture coordinates in response to a request to flip the
     * image.
     */
    virtual void updateTextureCoords() override;
    
    /**
     * Clears the render data.
     *
     * This method does nothing, as all render data is specified by the mesh.
     */
    virtual void clearRenderData() override {}

};
    }
}

#endif /* __CU_MESH_NODE_H__ */
