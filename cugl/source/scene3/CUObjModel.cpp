//
//  CUObjModel.cpp
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
#include <cugl/scene3/CUObjModel.h>
#include <cugl/scene3/CUMaterial.h>
#include <cugl/scene3/CUObjShader.h>

using namespace cugl;
using namespace cugl::graphics;
using namespace cugl::scene3;
using namespace std;

#pragma mark ModelInfo

/**
 * Returns a new GroupInfo allocated for this model.
 *
 * This method is used during parsing to add new render groups.
 *
 * @return a new GroupInfo allocated for this model.
 */
std::shared_ptr<GroupInfo> ModelInfo::acquireGroup() {
    auto result = std::make_shared<GroupInfo>();
    if (material != "") {
        result->material = material;
    }
    groups.push_back(result);
    return result;
}

#pragma mark -
#pragma mark OBJMesh
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
bool ObjMesh::initWithInfo(const std::shared_ptr<ModelInfo>& root,
                             const std::shared_ptr<GroupInfo>& info,
                             bool buffer) {
    if (root == nullptr || info == nullptr) {
        return false;
    }
    
    OBJVertex vert; // To define individual vertices

    for(auto it = info->vertices.begin(); it != info->vertices.end(); ++it) {
        VertexInfo ref = *it;
        vert.position = root->positions[ref.pindex];
        if (ref.tindex != -1) {
            vert.texcoord = root->texcoords[ref.tindex];
        }
        if (ref.nindex != -1) {
            vert.normal = root->normals[ref.nindex];
        }
        _mesh.vertices.push_back(vert);
    }
    
    _mesh.indices.insert(_mesh.indices.end(),
                         info->indices.begin(),
                         info->indices.end());
    _mesh.command = info->command;
    _tags.insert(info->tags.begin(),info->tags.end());
    _index  = info->index;
    _object = info->object;
    computeTangents();

    if (buffer && _mesh.vertices.size() > 0) {
        return createBuffer();
    }
    
    return true;
}

/**
 * Disposes all of the resources used by this mesh.
 *
 * A disposed mesh can be safely reinitialized. It is unsafe to call this
 * method on a mesh that is still inside of an active {@link ObjModel}.
 */
void ObjMesh::dispose() {
    _matname = "";
    _vertbuff = nullptr;
    _material = nullptr;
    _shader = nullptr;
    _tags.clear();
    _mesh.clear();
    _index = 0;
}

/**
 * Returns true if this mesh has the given tag.
 *
 * @param tag    The tag to query
 *
 * @return true if this mesh has the given tag.
 */
bool ObjMesh::hasTag(const std::string tag) {
    auto it = _tags.find(tag);
    return it != _tags.end();
}


/**
 * Computes the tangent vectors for this mesh.
 *
 * Only the positions, normals, and texture coordinates are specified in
 * the OBJ file. The tangents must be computed from these values.
 */
void ObjMesh::computeTangents() {
    int start = 0;
    int step = 0;
    if (_mesh.command == GL_TRIANGLES) {
        start = 0;
        step = 3;
    } else if (_mesh.command == GL_TRIANGLE_STRIP || 
               _mesh.command == GL_TRIANGLE_FAN) {
        start = 2;
        step = 1;
    } else {
        // Tangents not supported
        return;
    }
    
    // Loop over the triangles
    for(int ii = start; ii < _mesh.indices.size(); ii += step) {
        OBJVertex *v0, *v1, *v2;
        if (_mesh.command == GL_TRIANGLES) {
            v0 = &_mesh.vertices[_mesh.indices[ii  ]];
            v1 = &_mesh.vertices[_mesh.indices[ii+1]];
            v2 = &_mesh.vertices[_mesh.indices[ii+2]];
        } else if (_mesh.command == GL_TRIANGLE_STRIP) {
            v0 = &_mesh.vertices[_mesh.indices[ii-2]];
            v1 = &_mesh.vertices[_mesh.indices[ii-1]];
            v2 = &_mesh.vertices[_mesh.indices[ii  ]];
        } else {
            // Triangle fan
            v0 = &_mesh.vertices[_mesh.indices[0]];
            v1 = &_mesh.vertices[_mesh.indices[ii-1]];
            v2 = &_mesh.vertices[_mesh.indices[ii]];
        }
        
        float dt1, dt2;
        Vec3 dv1, dv2;

        /* Compute the tangent vector for this polygon. */
        dv1 = v1->position-v0->position;
        dv2 = v2->position-v0->position;

        dt1 = v1->texcoord.y - v0->texcoord.y;
        dt2 = v2->texcoord.y - v0->texcoord.y;

        Vec3 u = dt2*dv1 - dt1*dv2;
        u.normalize();

        /* Accumulate the tangent vectors for this polygon's vertices. */
        v0->tangent += u;
        v1->tangent += u;
        v2->tangent += u;
    }
    
    /* Orthonormalize each tangent basis. */
    for(auto it = _mesh.vertices.begin(); it != _mesh.vertices.end(); ++it) {
        Vec3 v;
        Vec3::cross(it->normal, it->tangent, &v);
        Vec3::cross(v, it->normal, &(it->tangent));
        it->tangent.normalize();
    }    
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
bool ObjMesh::createBuffer() {
    if (_vertbuff != nullptr) {
        return true;
    }
    
    // Allocate the vertex buffer (this binds as well)
    _vertbuff = VertexBuffer::alloc((GLsizei)_mesh.indices.size(),sizeof(OBJVertex));
    _vertbuff->setupAttribute("aPosition", 3, GL_FLOAT, GL_FALSE,
                              offsetof(OBJVertex,position));
    _vertbuff->setupAttribute("aTexCoord", 2, GL_FLOAT, GL_FALSE,
                              offsetof(OBJVertex,texcoord));
    _vertbuff->setupAttribute("aNormal", 3, GL_FLOAT, GL_FALSE,
                              offsetof(OBJVertex,normal));
    _vertbuff->setupAttribute("aTangent", 3, GL_FLOAT, GL_FALSE,
                              offsetof(OBJVertex,tangent));

    _vertbuff->bind();
    _vertbuff->loadVertexData(_mesh.vertices.data(), (int)_mesh.vertices.size(),GL_STREAM_DRAW);
    _vertbuff->loadIndexData(_mesh.indices.data(), (int)_mesh.indices.size(),GL_STREAM_DRAW);
    _vertbuff->unbind();

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        CULogError("VertexBuffer: %s", gl_error_name(error).c_str());
        _vertbuff = nullptr;
        return false;
    }
    
    // Verify the sizes
    
    
    return true;
}
    
/**
 * Sets the material associated with this mesh.
 *
 * If the mesh has no material, it will drawn using a default white color.
 *
 * @param material  The material associated with this mesh.
 */
void ObjMesh::setMaterial(const std::shared_ptr<Material>& material) {
    _material = material;
    if (material != nullptr) {
        _matname = material->getName();
    }
}


/**
 * Draws this mesh with the provided shader
 *
 * @param shader    The shader program to use
 */
void ObjMesh::draw(const std::shared_ptr<ObjShader>& shader) {
    if (_vertbuff == nullptr) {
        return;
    }

    if (shader != _shader) {
        _shader = shader;
        _vertbuff->attach(shader);
    }

    if (_material) {
        _material->bind(shader);
    }
    
    _vertbuff->bind();

    _vertbuff->draw(_mesh.command, (int)_mesh.indices.size(), 0);

    _vertbuff->unbind();

    if (_material) {
        _material->unbind();
    }

}

/**
 * Draws this mesh with the provided shader and material.
 *
 * The material will only be used on meshes that do not already have a
 * material assigned (e.g. it works as a default material).
 *
 * @param shader    The shader program to use
 * @param material  The material to use as a default
 */
void ObjMesh::draw(const std::shared_ptr<ObjShader>& shader,
                   const std::shared_ptr<Material>& material) {
    if (_vertbuff == nullptr) {
        return;
    }

    if (shader != _shader) {
        _shader = shader;
        _vertbuff->attach(shader);
    }

    // Check if we need to use the default
    auto mat = _material;
    if (!_material) {
        mat = material;
    }
    
    if (mat) {
        mat->bind(shader);
    }
    
    _vertbuff->bind();

    _vertbuff->draw(_mesh.command, (int)_mesh.indices.size(), 0);

    _vertbuff->unbind();

    if (mat) {
        mat->unbind();
    }
    
}

#pragma mark -
#pragma mark ObjModel
/**
 * Creates an uninitialized model.
 *
 * You must initialize this model before use.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a model on the
 * heap, use one of the static constructors instead.
 */
ObjModel::ObjModel() {
    _name = "";
}

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
bool ObjModel::initWithInfo(const std::shared_ptr<ModelInfo>& info, Uint32 depth) {
    if (info == nullptr) {
        return false;
    }
    
    _name = info->name;
    std::unordered_map<std::string, std::shared_ptr<Material>> materials;
    if (depth > 2) {
        for(auto it = info->libraries.begin(); it != info->libraries.end(); ++it) {
            for(auto jt = it->second->matinfos.begin(); jt != it->second->matinfos.end(); ++jt) {
                materials[jt->first] = Material::allocWithInfo(jt->second,depth>=3);
            }
        }
    }
    
    for(auto it = info->groups.begin(); it != info->groups.end(); ++it) {
        auto mesh = ObjMesh::allocWithInfo(info, *it, depth > 0);
        if (mesh == nullptr) {
            return false;
        }
        auto jt = materials.find((*it)->material);
        if (jt != materials.end()) {
            mesh->setMaterial(jt->second);
        }
        mesh->setMaterialName((*it)->material);
        _meshes.push_back(mesh);
    }
    return true;
}

/**
 * Disposes all of the resources used by this model.
 *
 * A disposed model can be safely reinitialized. Any children owned by this
 * node will be released. They will be deleted if no other object owns them.
 *
 * It is unsafe to call this on a Node that is still currently inside of
 * a scene graph.
 */
void ObjModel::dispose() {
    _name = "";
    _meshes.clear();
}

/**
 * Returns a submodel consisting of meshes that match the given tag.
 *
 * The tag can either be an object name or an group tag. Any mesh that
 * matches either will be added to the new model. The meshes will be
 * added by reference (not copied).
 *
 * @param tag   The tag identifying the submodel
 *
 * @return a submodel consisting of meshes that match the given tag.
 */
std::shared_ptr<ObjModel> ObjModel::getSubModel(const std::string tag) const {
    std::shared_ptr<ObjModel> result = std::make_shared<ObjModel>();
    
    result->_name = tag;
    for(auto it = _meshes.begin(); it != _meshes.end(); ++it) {
        auto mesh = *it;
        if (mesh->hasTag(tag)) {
            result->_meshes.push_back(mesh);
        }
    }
    return result;
}

/**
 * Draws this model with the provided shader
 *
 * @param shader    The shader program to use
 */
void ObjModel::draw(const std::shared_ptr<ObjShader>& shader) {
    for(auto it = _meshes.begin(); it != _meshes.end(); ++it) {
        (*it)->draw(shader);
    }
}


/**
 * Draws this model with the provided shader and material.
 *
 * The material will only be used on meshes that do not already have a
 * material assigned (e.g. it works as a default material).
 *
 * @param shader    The shader program to use
 * @param material  The material to use as a default
 */
void ObjModel::draw(const std::shared_ptr<ObjShader>& shader,
                    const std::shared_ptr<Material>& material) {
    for(auto it = _meshes.begin(); it != _meshes.end(); ++it) {
        (*it)->draw(shader, material);
    }
}
