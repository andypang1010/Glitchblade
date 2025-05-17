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
#include <cugl/scene2/CUMeshNode.h>
#include <cugl/core/math/polygon/CUPolyFactory.h>

using namespace cugl;
using namespace cugl::scene2;
using namespace cugl::graphics;

/** Flags for monitoring the flip state */
#define FLIPPED_HORZ 1
#define FLIPPED_VERT 2

/**
 * Extracts a list of vectors from the given JSON object, if possible.
 *
 * A vector list should be a list of even length (representing alternating x
 * and y). The extracted data will be stored in list. If such a list cannot be
 * extracted, this function returns false.
 *
 * @param data  The JSON object to extract from
 * @param list  The list to store the extracted data
 *
 * @return true if the list could be extracted
 */
static bool extractVec2List(const std::shared_ptr<JsonValue>& data,
                     std::vector<Vec2>& list) {
    if (data->size() % 2 != 0) {
        return false;
    }
    
    size_t size = list.size();
    for(int ii = 0; ii < data->size(); ii += 2) {
        Vec2 vector;
        vector.x = data->get(ii)->asFloat(0.0f);
        vector.y = data->get(ii+1)->asFloat(0.0f);
        list.push_back(vector);
    }
    return list.size() != size;
}

/**
 * Extracts a color from the given JSON object, if possible.
 *
 * A color is either represented as a string or as a four element number
 * array. The extracted data will be stored in color. If a valid color
 * cannot be extracted, this function returns false.
 *
 * @param data  The JSON object to extract from
 * @param list  The list to store the extracted data
 *
 * @return true if the list could be extracted
 */
static bool extractColor(const std::shared_ptr<JsonValue>& data, Color4& color) {
    if (data->isString()) {
        color.set(data->asString("#ffffff"));
        return true;
    } else {
        CUAssertLog(data->size() >= 4, "'color' must be a four element number array");
        color.r = std::max(std::min(data->get(0)->asInt(0),255),0);
        color.g = std::max(std::min(data->get(1)->asInt(0),255),0);
        color.b = std::max(std::min(data->get(2)->asInt(0),255),0);
        color.a = std::max(std::min(data->get(3)->asInt(0),255),0);
        return true;
    }

    return false;
}

/**
 * Extracts a list of colors from the given JSON object, if possible.
 *
 * A color list should be a list of color objects represented as either
 * strings or tuples. The extracted data will be stored in list. If such
 * a list cannot be extracted, this function returns false.
 *
 * @param data  The JSON object to extract from
 * @param list  The list to store the extracted data
 *
 * @return true if the list could be extracted
 */
static bool extractColorList(const std::shared_ptr<JsonValue>& data,
                      std::vector<Color4>& list) {
    size_t size = list.size();
    for(int ii = 0; ii < data->size(); ii++) {
        Color4 color;
        if (extractColor(data->get(ii),color)) {
            list.push_back(color);
        }
    }

    return list.size() != size;
}

#pragma mark -
#pragma mark Constructors
/**
 * Intializes a mesh node as a default equilateral triangle mesh.
 *
 * The mesh will use the texture {@link Texture#getBlank}, which is
 * suitable for drawing solid shapes. The vertex colors will be blue, red,
 * and yellow.
 *
 * @return true if the mesh node is initialized properly, false otherwise.
 */
bool MeshNode::init() {
    PolyFactory factory;
    auto poly = factory.makeTriangle(0,0,100,0,50,80);
    std::vector<Color4> colors;
    colors.push_back(Color4(42,101,180));
    colors.push_back(Color4(204,10,48));
    colors.push_back(Color4(246,210,101));
    return initWithPoly(poly,colors);
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
bool MeshNode::initWithFile(const std::string filename) {
    if (TexturedNode::initWithFile(filename)) {
        Size size = _texture == nullptr ? Size::ZERO : _texture->getSize();
        setPolygon(Rect(Vec2::ZERO,size));
        return true;
    }
    return false;
}

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
bool MeshNode::initWithFileMesh(const std::string filename, const Mesh<SpriteVertex>& mesh) {
    if (TexturedNode::initWithFile(filename)) {
        _mesh = mesh;
        _rendered = true;
        return true;
    }
    return false;
}

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
bool MeshNode::initWithFilePoly(const std::string filename, const Poly2& poly) {
    if (TexturedNode::initWithFile(filename)) {
        setPolygon(poly);
        _rendered = true;
        return true;
    }
    return false;
}


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
bool MeshNode::initWithFilePoly(const std::string filename, const Poly2& poly,
                                const std::vector<Color4>& colors) {
    if (TexturedNode::initWithFile(filename)) {
        setPolygon(poly);
        setVertexColors(colors);
        _rendered = true;
        return true;
    }
    return false;
}

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
bool MeshNode::initWithTextureMesh(const std::shared_ptr<Texture>& texture,
                                   const Mesh<SpriteVertex>& mesh) {
    if (TexturedNode::initWithTexture(texture)) {
        _mesh = mesh;
        _rendered = true;
        return true;
    }
    return false;
}


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
bool MeshNode::initWithTexturePoly(const std::shared_ptr<Texture>& texture, const Poly2& poly) {
    if (TexturedNode::initWithTexture(texture)) {
        setPolygon(poly);
        _rendered = true;
        return true;
    }
    return false;
}

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
bool MeshNode::initWithTexturePoly(const std::shared_ptr<Texture>& texture, const Poly2& poly,
                                   const std::vector<Color4>& colors) {
     if (TexturedNode::initWithTexture(texture)) {
         setPolygon(poly);
         setVertexColors(colors);
         _rendered = true;
         return true;
     }
     return false;
}

/**
 * Returns a newly allocated mesh node with the given JSON specification.
 *
 * This initializer is designed to receive the "data" object from the
 * JSON passed to {@link Scene2Loader}. This JSON format supports all
 * of the attribute values of its parent class.  In addition, it supports
 * the following additional attributes:
 *
 *     "mesh":  A JSON object defining a mesh of SpriteVertex
 *
 * This JSON object for mesh is required. The mesh can be represented as
 * either an array or JSON object. If it is an array, it contain JSON
 * values compatible with the {@link SpriteVertex} constructor. If it is
 * a JSON object, then it supports the following attributes:
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
bool MeshNode::initWithData(const AssetManager* manager,
                            const std::shared_ptr<JsonValue>& data) {
    if (!data) {
        return init();
    } else if (!TexturedNode::initWithData(manager, data)) {
        return false;
    }
    
    if (!data->has("mesh")) {
        parse_mesh<SpriteVertex>(_mesh,data->get("mesh"));
    }

    return true;
}

/**
 * Disposes all of the resources used by this node.
 *
 * A disposed Node can be safely reinitialized. Any children owned by this
 * node will be released.  They will be deleted if no other object owns them.
 *
 * It is unsafe to call this on a Node that is still currently inside of
 * a scene graph.
 */
void MeshNode::dispose() {
    TexturedNode::dispose();
    _classname = "TexturedNode";
}

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
std::shared_ptr<SceneNode> MeshNode::copy(const std::shared_ptr<SceneNode>& dst) const {
    SceneNode::copy(dst);
    std::shared_ptr<MeshNode> node = std::dynamic_pointer_cast<MeshNode>(dst);
    if (node) {
        node->_flipFlags = _flipFlags;
    }
    return dst;
}

#pragma mark -
#pragma mark Mesh Attributes
/**
 * Sets the mesh for the mesh node.
 *
 * @param mesh  The updated mesh.
 */
void MeshNode::setMesh(const Mesh<SpriteVertex>& mesh) {
    _mesh = mesh;
    _flipFlags = 0;
}

/**
 * Sets the mesh for the mesh node.
 *
 * @param mesh  The updated mesh.
 */
void MeshNode::setMesh(graphics::Mesh<graphics::SpriteVertex>&& mesh) {
    _mesh = std::move(mesh);
    _flipFlags = 0;
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
const SpriteVertex* MeshNode::getVertex(size_t index) const {
    if (index < _mesh.vertices.size()) {
        return &(_mesh.vertices.at(index));
    }
    return nullptr;
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
SpriteVertex* MeshNode::getVertex(size_t index) {
    if (index < _mesh.vertices.size()) {
        return &(_mesh.vertices.at(index));
    }
    return nullptr;
}

/**
 * Sets the colors of the mesh vertices.
 *
 * The parameter vector should have a size equal to the number of
 * vertices. If it is too long, extra colors are ignored. If it is too
 * short, the the final color will be used for all remaining vertices.
 *
 * @param colors    The vertex colors
 */
void MeshNode::setVertexColors(const std::vector<Color4>& colors) {
    int curr = Color4::WHITE.getPacked();
    int pos = 0;
    for(auto it = _mesh.vertices.begin(); it != _mesh.vertices.end(); ++it) {
        if (pos < colors.size()) {
            curr = colors[pos].getPacked();
        }
        it->color = curr;
        pos++;
    }
}

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
void MeshNode::setVertexTexCoords(const std::vector<Vec2>& coords) {
    Vec2 curr;
    int pos = 0;
    for(auto it = _mesh.vertices.begin(); it != _mesh.vertices.end(); ++it) {
        if (pos < coords.size()) {
            curr = coords[pos];
        }
        it->texcoord = curr;
        pos++;
    }
}

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
void MeshNode::setVertexGradCoords(const std::vector<Vec2>& coords) {
    Vec2 curr;
    int pos = 0;
    for(auto it = _mesh.vertices.begin(); it != _mesh.vertices.end(); ++it) {
        if (pos < coords.size()) {
            curr = coords[pos];
        }
        it->gradcoord = curr;
        pos++;
    }
}

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
const Rect MeshNode::getBoundingRect() const {
    if (_mesh.vertices.empty()) {
        return Rect::ZERO;
    }
        
    float minx, maxx;
    float miny, maxy;
        
    minx = _mesh.vertices[0].position.x;
    maxx = _mesh.vertices[0].position.x;
    miny = _mesh.vertices[0].position.y;
    maxy = _mesh.vertices[0].position.y;
    for(auto it = _mesh.vertices.begin()+1; it != _mesh.vertices.end(); ++it) {
        if (it->position.x < minx) {
            minx = it->position.x;
        } else if (it->position.x > maxx) {
            maxx = it->position.x;
        }
        if (it->position.y < miny) {
            miny = it->position.y;
        } else if (it->position.y > maxy) {
            maxy = it->position.y;
        }
    }
        
    return Rect(minx,miny,maxx-minx,maxy-miny);
}

/**
 * Sets the mesh to match the given polygon.
 *
 * The mesh textures and colors will be recomputed as if this were a
 * {@link PolygonNode}.
 *
 * @param poly  The new poly2 object.
 */
void MeshNode::setPolygon(const Poly2& poly) {
    
    _mesh.set(poly);
    _mesh.command = GL_TRIANGLES;
    
    // Adjust the mesh as necesary
    Size nsize = getContentSize();
    Size bsize = poly.getBounds().size;

    Mat4 shift;
    bool adjust = false;
    if (nsize != bsize) {
        adjust = true;
        shift.scale((bsize.width > 0  ? nsize.width/bsize.width : 0),
                    (bsize.height > 0 ? nsize.height/bsize.height : 0), 1);
    }

    if (adjust) {
        _mesh *= shift;
    }

    if (_texture != nullptr) {
        Size tsize = _texture->getSize();
        Vec2 off = _offset+poly.getBounds().origin;
        for(auto it = _mesh.vertices.begin(); it != _mesh.vertices.end(); ++it) {
            float s = (it->position.x+off.x)/tsize.width;
            float t = (it->position.y+off.y)/tsize.height;

            it->texcoord.x = s*_texture->getMaxS()+(1-s)*_texture->getMinS();
            it->texcoord.y = t*_texture->getMaxT()+(1-t)*_texture->getMinT();
        
            if (_gradient) {
                s = (it->position.x+off.x)/poly.getBounds().size.width;
                t = (it->position.x+off.x)/poly.getBounds().size.height;
                it->gradcoord.x = s;
                it->gradcoord.y = t;
            }
        }
    } else if (_gradient != nullptr) {
        Vec2 off = _offset+poly.getBounds().origin;
        for(auto it = _mesh.vertices.begin(); it != _mesh.vertices.end(); ++it) {
            float s = (it->position.x+off.x)/poly.getBounds().size.width;
            float t = (it->position.x+off.x)/poly.getBounds().size.height;
            it->gradcoord.x = s;
            it->gradcoord.y = t;
        }
    }
}

#pragma mark -
#pragma mark Rendering
/**
 * Draws this polygon node via the given SpriteBatch.
 *
 * This method only worries about drawing the current node.  It does not
 * attempt to render the children.
 *
 * @param batch     The SpriteBatch to draw with.
 * @param transform The global transformation matrix.
 * @param tint      The tint to blend with the Node color.
 */
void MeshNode::draw(const std::shared_ptr<SpriteBatch>& batch, const Affine2& transform,
                    Color4 tint) {
    batch->setColor(tint);
    batch->drawMesh(_mesh, transform);
}

/**
 * Updates the texture coordinates for this polygon
 *
 * This method inverts texture coordinates in response to a request to flip the
 * image.
 */
void MeshNode::updateTextureCoords() {
    bool flipHorz = false;
    if (_flipHorizontal && !(_flipFlags & FLIPPED_HORZ)) {
        flipHorz = true;
        _flipFlags = _flipFlags ^ FLIPPED_HORZ;
    } else if (!_flipHorizontal && (_flipFlags & FLIPPED_HORZ)) {
        flipHorz = true;
        _flipFlags = _flipFlags ^ FLIPPED_HORZ;
    }

    bool flipVert = false;
    if (_flipVertical && !(_flipFlags & FLIPPED_VERT)) {
        flipVert = true;
        _flipFlags = _flipFlags ^ FLIPPED_VERT;
    } else if (!_flipVertical && (_flipFlags & FLIPPED_VERT)) {
        flipVert = true;
        _flipFlags = _flipFlags ^ FLIPPED_VERT;
    }
    
    if (!flipHorz && !flipVert) {
        return;
    }
    
    float maxs = 1.0;
    float mins = 0.0;
    float maxt = 1.0;
    float mint = 0.0;
    if (_texture) {
        maxs = _texture->getMaxS();
        mins = _texture->getMinS();
        maxt = _texture->getMaxT();
        mint = _texture->getMinT();
    }

    for(auto it = _mesh.vertices.begin(); it != _mesh.vertices.end(); ++it) {
        if (flipHorz) {
            float s = (it->texcoord.x-mins)/(maxs-mins);
            it->texcoord.x = (1-s)*maxs+s*mins;
            it->gradcoord.x = 1-it->gradcoord.x;
        }
        if (flipVert) {
            float t = (it->texcoord.y-mint)/(maxt-mint);
            it->texcoord.y = (1-t)*maxt+t*mint;
            it->gradcoord.y = 1-it->gradcoord.y;
        }
    }
}


