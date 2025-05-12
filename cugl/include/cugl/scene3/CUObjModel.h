//
//  CUObjModel.h
//  Cornell University Game Library (CUGL)
//
//  This module represents a WaveFront Object as defined by an OBJ file. It
//  includes classes both for object model and its meshes, as well as classes
//  for the AST built during parsing. Note that this module only represents
//  an OBJ as a static asset. OBJ models do not have any positional information.
//
//  We currently only support polygonal object meshes. We do not support any
//  freeform drawing commands. If you need more than this, you should use a
//  third party rendering library.
//
//  Because the AST classes are essentially structs with no strong invariants,
//  all attributes are public and we do not provide any initialization or
//  allocation methods for them. However, the object and its mesh has proper
//  encapsulation combined with our standard shared-pointer architecture.
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
//  Author: Walker White
//  Version: 7/3/24 (CUGL 3.0 reorganization)
//
#ifndef __CU_OBJ_MODEL_H__
#define __CU_OBJ_MODEL_H__
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cugl/core/util/CUHashtools.h>
#include <cugl/graphics/CUMesh.h>
#include <cugl/graphics/CUVertexBuffer.h>

namespace cugl {

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

// Forward references
class Material;
class MaterialInfo;
class MaterialLib;
class ObjShader;

#pragma mark AST Classes
/**
 * This class represents the indices of an OBJ vertex.
 *
 * All shapes in the OBJ file format are represented by three values: the
 * position, the texture coordinate, and the normal (though the last two are
 * optional). For compact representation, these values are stored individually
 * in {@link ModelInfo} and indexed by position. As GLSL shaders do not permit
 * these values to be streamed independently, this information must be
 * flattened before rendering.
 *
 * Indices must be a value >= 0. Setting an index to a negative value means
 * that it is undefined/unsupported. In practice, only the position index must
 * be defined.
 */
class VertexInfo {
public:
    /** The index of the vertex position in the model (-1 if undefined) */
    int pindex;
    /** The index of the texture coordinate in the model (-1 if undefined) */
    int tindex;
    /** The index of the vertex normal in the model (-1 if undefined) */
    int nindex;
    
    /**
     * Creates an uninitalized VertexInfo value.
     *
     * All indices start off as undefined.
     */
    VertexInfo() : pindex(-1), tindex(-1), nindex(-1) {}
    
    /**
     * Returns true if the VertexInfo rhs is equivalent to this one.
     *
     * All equivalence is determined by attribute comparison.
     *
     * @param rhs   The VertexInfo to compare
     *
     * @return true if the VertexInfo rhs is equivalent to this one.
     */
    bool operator==(const VertexInfo& rhs) const {
        return ((pindex == rhs.pindex) &&
                (tindex == rhs.tindex) &&
                (nindex == rhs.nindex));
    }
    
    /**
     * Returns true if the VertexInfo rhs is not equivalent to this one.
     *
     * All equivalence is determined attribute comparison
     *
     * @param rhs   The VertexInfo to compare
     *
     * @return true if the VertexInfo rhs is not equivalent to this one.
     */
    bool operator!=(const VertexInfo& rhs) const {
        return !operator==(rhs);
    }
};

/**
 * This struct is a hash function for VertexInfo.
 *
 * This function allows us to use VertexInfo as a key in STL containers like
 * unordered_set and unordered_map.
 */
struct VertexHasher {
    /**
     * Returns a hashs code for the given info
     *
     * @param info  The vertex info to hash
     *
     * @return a hashs code for the given info
     */
    std::size_t operator()(const VertexInfo& info) const {
        std::size_t result=0;
        cugl::hashtool::hash_combine(result,info.pindex,info.tindex,info.nindex);
        return result;
    }
};

/**
 * This class represents the attributes of an OBJ render group.
 *
 * OBJ models are broken up into multiple groups for rendering. A group
 * corresponds to a single draw call to the graphics pipeline. Therefore, we
 * need a new group whenever we have a new material. In addition, OBJ files
 * can explicitly create new groups with the "g" or "s" command. We also
 * create new groups with an "o" command, though that is not standard.
 */
class GroupInfo {
public:
    // TODO: Put this in parser.  It is a state value
    /** Whether this info has received any vertex information (for parsing) */
    bool touched;
    /** The smoothing index of for this render group (the OBJ s value) */
    unsigned index;
    /** The drawing command for this group (GL_FALSE for undefined) */
    GLenum command;
    /** The object group (the OBJ o value) */
    std::string object;
    /** The material name for this render group */
    std::string material;
    /** The tags for this render group (the OBJ g values) */
    std::unordered_set<std::string> tags;
    
    /** The vertices associated with this group */
    std::vector<VertexInfo> vertices;
    /** The render group shape represented as indexed vertices */
    std::vector<GLuint> indices;
    /** A vertex cache to eliminate redundancy */
    std::unordered_map<VertexInfo,GLuint,VertexHasher> vertCache;
    
    /**
     * Creates an unintialized GroupInfo with default values
     */
    GroupInfo() : touched(false), index(0), command(GL_FALSE) {
        object = "";
        material = "";
    }
    
};

/**
 * This class represents the data in an OBJ file.
 *
 * While a single OBJ file can potentially represeting multiple models, we
 * consider them all to be a single model and only separate up files into
 * rendering groups (which is all the specification really allows)
 *
 * All data is stored in the same order that it is presented in the file.
 * That makes it possible for a {@link VertexInfo} to refer to data by its
 * index.
 */
class ModelInfo {
public:
    /** The name of the object model (e.g. the reference key) */
    std::string name;
    /** The path to the OBJ file */
    std::string path;
    // TODO: Put this in the parser
    /** The name for the current active material */
    std::string material;
    /** The vertex positions in this file */
    std::vector<cugl::Vec3> positions;
    /** The texture coordinates in this file */
    std::vector<cugl::Vec2> texcoords;
    /** The vertex normals in this file */
    std::vector<cugl::Vec3> normals;
    /** The render groups in this file */
    std::vector<std::shared_ptr<GroupInfo>> groups;
    /** The imported libraries */
    std::unordered_map<std::string,std::shared_ptr<MaterialLib>> libraries;
    
    /**
     * Creates an unintialized ModelInfo with default values
     */
    ModelInfo() {
        name = "";
        path = "";
        material = "";
    }
    
    /**
     * Returns a new GroupInfo allocated for this model.
     *
     * This method is used during parsing to add new render groups.
     *
     * @return a new GroupInfo allocated for this model.
     */
    std::shared_ptr<GroupInfo> acquireGroup();
    
    /**
     * Returns the currently active GroupInfo object.
     *
     * This method is used during parsing to update the current render group.
     *
     * @return the currently active GroupInfo object.
     */
    std::shared_ptr<GroupInfo> currentGroup() const {
        return groups.empty() ? nullptr : groups.back();
    }
    
};

#pragma mark -
#pragma mark OBJVertex
/**
 * This class is a vertex in an OBJ mesh.
 *
 * Each vertex must have a position. All other values are optional, and are
 * zero by default. Tangents and normals provide support for optional bump
 * mapping.
 */
class OBJVertex {
public:
    /** The vertex position */
    cugl::Vec3    position;
    /** The vertex normal */
    cugl::Vec3    normal;
    /** The vertex tangent */
    cugl::Vec3    tangent;
    /** The vertex texture coordinate */
    cugl::Vec2    texcoord;
    
    /** The memory offset of the vertex position */
    static const GLvoid* positionOffset()   {
        return (GLvoid*)offsetof(OBJVertex, position);
    }
    /** The memory offset of the vertex normal */
    static const GLvoid* normalOffset()      {
        return (GLvoid*)offsetof(OBJVertex, normal);
    }
    /** The memory offset of the vertex tangent */
    static const GLvoid* tangentOffset()      {
        return (GLvoid*)offsetof(OBJVertex, tangent);
    }
    /** The memory offset of the vertex texture coordinate */
    static const GLvoid* texcoordOffset()   {
        return (GLvoid*)offsetof(OBJVertex, texcoord);
    }
};

#pragma mark -
#pragma mark OBJMesh
/**
 * This class represents a single mesh or surface in an OBJ file.
 *
 * A OBJ surface is a single unit of rendering. A new mesh is created any time
 * an OBJ file issues face commands after declaring a new object, group, or
 * material. Meshes doe not have their own ModelView matrix. That is stored in
 * the {@link ObjModel} that groups them together.
 *
 * We currently only support fully specified polygonal meshes. We do not support
 * any freeform drawing commands, even though those are included in the OBJ
 * specification.
 */
class ObjMesh {
private:
    /** The smoothing index for this mesh (default is 0) */
    GLuint _index;
    /** The object this mesh is associated with (default is "") */
    std::string _object;
    /** The group tags for this mesh */
    std::unordered_set<std::string> _tags;
    
    /** The mesh for storing the drawing data */
    graphics::Mesh<OBJVertex> _mesh;
    /** A vertex buffer to receive our triangle */
    std::shared_ptr<graphics::VertexBuffer> _vertbuff;
    /** The material for this shape */
    std::shared_ptr<Material> _material;
    /** The name of the material (for delayed instantiation) */
    std::string _matname;
    
    /** The (cached) shader to associate with this mesh */
    std::shared_ptr<graphics::Shader> _shader;
    
    /**
     * Computes the tangent vectors for this mesh.
     *
     * Only the positions, normals, and texture coordinates are specified in
     * the OBJ file. The tangents must be computed from these values.
     */
    void computeTangents();
    
public:
#pragma mark Constructors
    /**
     * Creates an uninitialized mesh.
     *
     * You must initialize this mesh before use.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a mesh on the
     * heap, use one of the static constructors instead.
     */
    ObjMesh() : _index(0) {}
    
    /**
     * Deletes this mesh, disposing all resources
     */
    ~ObjMesh() { dispose(); }
    
    /**
     * Disposes all of the resources used by this mesh.
     *
     * A disposed mesh can be safely reinitialized. It is unsafe to call this
     * method on a mesh that is still inside of an active {@link ObjModel}.
     */
    void dispose();
    
    /**
     * Initializes this mesh with the given AST.
     *
     * This method will build the vertices and mesh information for the mesh.
     * However, it will only create the OpenGL buffer if the parameter buffer
     * is true. This allows us to create meshes off of the main thread (and
     * allocate the buffer once back on the main thread).
     *
     * @param root      The AST root
     * @param info      The AST for this particular mesh
     * @param buffer    Whether to construct the OpenGL buffer
     *
     * @return true if initialization was successful.
     */
    bool initWithInfo(const std::shared_ptr<ModelInfo>& root,
                      const std::shared_ptr<GroupInfo>& info,
                      bool buffer = true);
    
    /**
     * Returns a newly allocated mesh with the given AST.
     *
     * This method will build the vertices and mesh information for the mesh.
     * However, it will only create the OpenGL buffer if the parameter buffer
     * is true. This allows us to create meshes off of the main thread (and
     * allocate the buffer once back on the main thread).
     *
     * @param root      The AST root
     * @param info      The AST for this particular mesh
     * @param buffer    Whether to construct the OpenGL buffer
     *
     * @return a newly allocated mesh with the given AST.
     */
    static std::shared_ptr<ObjMesh> allocWithInfo(const std::shared_ptr<ModelInfo>& root,
                                                  const std::shared_ptr<GroupInfo>& info,
                                                  bool buffer = true) {
        std::shared_ptr<ObjMesh> result = std::make_shared<ObjMesh>();
        return (result->initWithInfo(root,info,buffer) ? result : nullptr);
    }
    
    /**
     * Returns true if an OpenGL buffer was successfully built for this mesh.
     *
     * If the mesh is already built this method will return true. Otherwise,
     * it will attempt to build the mesh and return true on success. This
     * method should only be called on the main thread. It exists to support
     * asset loading off the main thread.
     *
     * @return true if an OpenGL buffer was successfully built for this mesh.
     */
    bool createBuffer();
    
#pragma mark Attributes
    /**
     * Returns the smoothing index for this mesh.
     *
     * As per the OBJ specification, this value is 0 by default.
     *
     * @return the smoothing index for this mesh.
     */
    GLuint getIndex() const { return _index; }
    
    /**
     * Sets the smoothing index for this mesh.
     *
     * As per the OBJ specification, this value is 0 by default.
     *
     * @param value The smoothing index for this mesh.
     */
    void setIndex(GLuint value) { _index = value; }
    
    /**
     * Returns the object that this mesh is associated with.
     *
     * Objects are defined with the o command in OBJ files. This value is the
     * empty string by default.
     *
     * @return the object that this mesh is associated with.
     */
    std::string getObject() const { return _object; }
    
    /**
     * Sets the object that this mesh is associated with.
     *
     * Objects are defined with the o command in OBJ files. This value is the
     * empty string by default.
     *
     * @param value The object that this mesh is associated with.
     */
    void getObject(const std::string value) { _object = value; }
    
    /**
     * Returns the group tags associated with this mesh.
     *
     * This tag set is returned by reference. That means modifying this set
     * will modify the associated tags.
     *
     * @return the group tags associated with this mesh.
     */
    std::unordered_set<std::string>& getTags() { return _tags; }
    
    /**
     * Returns the group tags associated with this mesh.
     *
     * This tag set is returned by reference. This version of the method
     * prevents the tag set from being modified, so that it can safely be
     * used in read-only settings.
     *
     * @return the group tags associated with this mesh.
     */
    const std::unordered_set<std::string>& getTags() const { return _tags; }
    
    /**
     * Returns true if this mesh has the given tag.
     *
     * @param tag	The tag to query
     *
     * @return true if this mesh has the given tag.
     */
    bool hasTag(const std::string tag);
    
    /**
     * Returns the CUGL mesh for storing the drawing data
     *
     * CUGL meshes store the geometry but are independent of any OpenGL buffer.
     * This method is read-only, as it is unsafe to modify the mesh without
     * updating the associated OpenGL buffer.
     *
     * @return the CUGL mesh for storing the drawing data
     */
    const graphics::Mesh<OBJVertex>& getMesh() const { return _mesh; }
    
    /**
     * Returns the name of the material associated with this mesh.
     *
     * If the mesh has no material, this will return the empty string.
     *
     * @return the name of the material associated with this mesh.
     */
    std::string getMaterialName() const { return _matname; }
    
    /**
     * Sets the name of the material associated with this mesh.
     *
     * If the mesh has no material, this will return the empty string.
     *
     * @param name  The name of the material associated with this mesh.
     */
    void setMaterialName(const std::string name) { _matname = name; }
    
    /**
     * Returns the material associated with this mesh.
     *
     * If the mesh has no material, it will drawn using a default white color.
     *
     * @return the material associated with this mesh.
     */
    std::shared_ptr<Material> getMaterial() const { return _material; }
    
    /**
     * Sets the material associated with this mesh.
     *
     * If the mesh has no material, it will drawn using a default white color.
     *
     * @param material  The material associated with this mesh.
     */
    void setMaterial(const std::shared_ptr<Material>& material);
    
    /**
     * Draws this mesh with the provided shader
     *
     * @param shader    The shader program to use
     */
    void draw(const std::shared_ptr<ObjShader>& shader);
    
    /**
     * Draws this mesh with the provided shader and material.
     *
     * The material will only be used on meshes that do not already have a
     * material assigned (e.g. it works as a default material).
     *
     * @param shader    The shader program to use
     * @param material  The material to use as a default
     */
    void draw(const std::shared_ptr<ObjShader>& shader, const std::shared_ptr<Material>& material);
};

#pragma mark -
#pragma mark ObjModel
/**
 * This class represents an OBJ model.
 *
 * An OBJ model is one or more meshes (surfaces) combined with a ModelView
 * matrix specifying its coordinate system. It is possible to have multiple
 * models that share the same meshes but with different ModelView matrices.
 * Such models are clones of each other drawn in different orientations.
 * Indeed, the ModelView matrix is what distinguishes an OBJ instance from
 * an OBJ asset.
 *
 * OBJ models typically correspond to a single OBJ file. However, it is
 * possible to use groups and object names to extract a portion of an OBJ
 * file as its own object. This is similar to the concept of texture atlases
 * but for 3d models. In particular, this can be used to break up a model
 * into articulated joints for animation. With that said, CUGL does not
 * currently support any sort of bone animation.
 *
 * All OBJ models are nodes in a 3d scene graph. As such, they can have
 * children. However, there is no parent-child structure in an OBJ file.
 * These relationships must be built manually.
 */
class ObjModel {
private:
    /** The model name (file, object, or group name) */
    std::string _name;
    /** The meshes associated with this object */
    std::vector<std::shared_ptr<ObjMesh>> _meshes;
    
public:
#pragma mark Constructors
    /**
     * Creates an uninitialized model.
     *
     * You must initialize this model before use.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a model on the
     * heap, use one of the static constructors instead.
     */
    ObjModel();
    
    /**
     * Deletes this mesh, disposing all resources
     */
    ~ObjModel() { dispose(); }
    
    /**
     * Disposes all of the resources used by this model.
     *
     * A disposed model can be safely reinitialized. Any children owned by this
     * node will be released. They will be deleted if no other object owns them.
     *
     * It is unsafe to call this on a Node that is still currently inside of
     * a scene graph.
     */
    void dispose();
    
    // TODO: More Initializers?
    /**
     * Initializes this model with the given AST.
     *
     * This method will build the model recursively. As the AST holds both OBJ
     * and MTL information, the number and type of objects created depends upon
     * the depth parameter
     *
     *      Depth 0: The meshes are created without OpenGL buffers or materials
     *      Depth 1: The meshes are created with OpenGL buffers but no materials
     *      Depth 2: Materials are created for the meshes, but without textures
     *      Depth 3: Textures are created for the materials as necessary
     *
     * Note that only depth 0 is safe for off the main thread. Therefore, depth
     * 0 is typically used by asset loaders while the latter depths are
     * convenience methods meant to cut down on the number of steps to assemble
     * the model.
     *
     * @param info  The AST for this model
     * @param depth The detail depth of the construction process
     *
     * @return true if initialization was successful.
     */
    bool initWithInfo(const std::shared_ptr<ModelInfo>& info, Uint32 depth=3);
    
    /**
     * Returns a newly allocated model with the given AST.
     *
     * This method will build the model recursively. As the AST holds both OBJ
     * and MTL information, the number and type of objects created depends upon
     * the depth parameter
     *
     *      Depth 0: The meshes are created without OpenGL buffers or materials
     *      Depth 1: The meshes are created with OpenGL buffers but no materials
     *      Depth 2: Materials are created for the meshes, but without textures
     *      Depth 3: Textures are created for the materials as necessary
     *
     * Note that only depth 0 is safe for off the main thread. Therefore, depth
     * 0 is typically used by asset loaders while the latter depths are
     * convenience methods meant to cut down on the number of steps to assemble
     * the model.
     *
     * @param info  The AST for this model
     * @param depth The detail depth of the construction process
     *
     * @return a newly allocated model with the given AST.
     */
    static std::shared_ptr<ObjModel> allocWithInfo(const std::shared_ptr<ModelInfo>& info, Uint32 depth=3) {
        std::shared_ptr<ObjModel> result = std::make_shared<ObjModel>();
        return (result->initWithInfo(info,depth) ? result : nullptr);
    }
    
#pragma mark Attributes
    /**
     * Returns the name of this model
     *
     * @return the name of this model
     */
    const std::string getName() const { return _name; }
    
    /**
     * Returns the meshes that make up this object.
     *
     * @return the meshes that make up this object.
     */
    const std::vector<std::shared_ptr<ObjMesh>>& getMeshes() const {
        return _meshes;
    }
    
    /**
     * Returns a submodel consisting of meshes that match the given tag.
     *
     * The tag can either be an object name or an group tag. Any mesh that
     * matches either will be added to the new model. The meshes will be
     * added by reference (not copied).
     *
     * This method works very similar to a {@link graphics::Texture} atlas.
     * It allows you to break up a single OBJ model into several components.
     * All components will share references to the same meshes.
     *
     * @param tag   The tag identifying the submodel
     *
     * @return a submodel consisting of meshes that match the given tag.
     */
    std::shared_ptr<ObjModel> getSubModel(const std::string tag) const;
    
    /**
     * Draws this model with the provided shader
     *
     * @param shader    The shader program to use
     */
    void draw(const std::shared_ptr<ObjShader>& shader);
    
    /**
     * Draws this model with the provided shader and material.
     *
     * The material will only be used on meshes that do not already have a
     * material assigned (e.g. it works as a default material).
     *
     * @param shader    The shader program to use
     * @param material  The material to use as a default
     */
    void draw(const std::shared_ptr<ObjShader>& shader, const std::shared_ptr<Material>& material);
    
};

    }
}

#endif /* __CU_OBJ_MODEL_H__ */
