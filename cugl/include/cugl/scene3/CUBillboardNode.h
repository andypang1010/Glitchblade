//
//  CUBillboardNode.h
//  Cornell University Game Library (CUGL)
//
//  This module an implementation of a billboard in 3d space. It is adapted
//  from the OpenGL tutorial series:
//
//     http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/billboards/
//
//  Some changes have been made from that tutorial. We do not fix the size of
//  the billboard, so it is possible to zoom in and out. In addition, the
//  billboard is always positioned at the origin, but it can be translated
//  (and rotated or scaled) by a model matrix.
//
//  Note that billboards have their own OpenGL buffer.  As there is some work
//  in allocating them, we have separated this portion out as a sprite mesh.
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
#ifndef __CU_BILLBOARD_NODE_H__
#define __CU_BILLBOARD_NODE_H__
#include <cugl/scene3/CUSceneNode3.h>

namespace cugl {

// Forward declarations
namespace graphics {
    class Texture;
    class Gradient;
    class SpriteMesh;
}

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

/**
 * This class represents an Billboard in a 3d scene graph.
 *
 * Billboards are designed to project 2d images in 3d space. The image is
 * always shown head on, no matter the angle or perspective. The only thing
 * that can vary is the origin of the billboard, which is a point in 3d space.
 * This origin can be transformed by {@link BillboardShader#setModelMatrix}.
 * So it is possible to move the image along all three axes in 3d space.
 *
 * Billboards support a single static {@link graphics::SpriteMesh}. Unlike
 * sprite batches billboards are not designed for compositing, as alpha blending
 * is completely incompatible 3d depth filtering. If you need to billboard a
 * composite image, you should first render it to a single texture with
 * {@link graphics::RenderTarget} before putting it in the billboard.
 *
 * The two features that do carry over from {@link graphics::SpriteBatch} are
 * textures and gradients. These are both optional. A mesh with neither texture
 * nor gradient is rendered as a solid color. Because billboards do not handle
 * alpha blending (and hence transparency) very well, it is best to make sure
 * that the mesh fits the texture as close as possible, rather than making the
 * mesh a default quad.
 *
 * Billboards can be animated in the same way that a {@link graphics::ParticleSystem}
 * can. A call to {@link BillboardShader#setTextureOffset} will offset all of
 * the texture coordinates in the mesh by the given amount. This node has an
 * interface similar to that of {@link scene2::SpriteNode} to aid in the
 * calculation of this offset. In particular, you can take a texture and break
 * it into a sprite sheet of equal-sized frames.
 */
class BillboardNode : public SceneNode {
private:
    /** The mesh associated with this node */
    std::shared_ptr<graphics::SpriteMesh> _mesh;
    /** The billboard texture (optional) */
    std::shared_ptr<graphics::Texture> _texture;
    /** The billboard gradient (optional) */
    std::shared_ptr<graphics::Gradient> _gradient;
    /** The size of a single animation frame */
    Rect _bounds;
    /** The current texture offset (for animation) */
    Vec2 _texoffset;
    /** The number of frames in the billboard sprite */
    int _size;
    /** The number of columns in the billboard sprite */
    int _rows;
    /** The number of columns in the billboard sprite */
    int _cols;
    /** The active animation frame */
    int _frame;

    /**
     * Computes the sprite bounds for the initial frame in the sprite sheet
     */
    void computeSpriteBounds();

public:
#pragma mark Constructors
    /**
     * Creates an uninitialized node.
     *
     * You must initialize this model before use.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a model on the
     * heap, use one of the static constructors instead.
     */
    BillboardNode();
    
    /**
     * Deletes this node, disposing all resources
     */
    ~BillboardNode() { dispose(); }
    
    /**
     * Disposes all of the resources used by this node.
     *
     * A disposed node can be safely reinitialized. Any children owned by this
     * node will be released. They will be deleted if no other object owns them.
     *
     * It is unsafe to call this on a node that is still currently inside of
     * a scene graph.
     */
    virtual void dispose() override;
    
    /**
     * Initializes this node with the given sprite mesh
     *
     * This node will keep a reference to sprite mesh, but it will have its own
     * position, orientation, texture, and gradient. Therefore it is possible
     * for multiple nodes to share the same sprite mesh.
     *
     * The billboard will assume that the sprite mesh is to represent a static
     * image. Therefore, there is only one animation frame in the texture
     * (e.g. rows and cols are 1).
     *
     * @param mesh  The sprite mesh
     *
     * @return true if initialization was successful.
     */
    bool initWithSpriteMesh(const std::shared_ptr<graphics::SpriteMesh>& mesh);

    /**
     * Initializes a node with the given JSON specification.
     *
     * This initializer is designed to receive the "data" object from the
     * JSON passed to {@link Scene3Loader}. This JSON format supports all
     * of the attribute values of its parent class. In addition, it supports
     * the following additional attribute:
     *
     *      "mesh":     A string with the name of a previously loaded sprite mesh asset
     *      "texture":  A string with the name of a previously loaded texture asset
     *      "gradient": A JSON object defining a gradient. See {@link Gradient}.
     *      "span":     The number of frames in the texture sprite sheet
     *      "rows":     An int specifying the number of rows in the sprite sheet
     *      "cols":     An int specifying the number of columns in the sprite sheet
     *      "frame":    The initial starting frame.
     *
     * All attributes are optional. However, not specifying the mesh means that
     * nothing is drawn.
     *
     * @param manager   The asset manager handling this asset
     * @param json      The JSON object specifying the node
     *
     * @return true if initialization was successful.
     */
    bool initWithData(const AssetManager* manager,
                      const std::shared_ptr<JsonValue>& json) override;

    /**
     * Performs a shallow copy of this node into dst.
     *
     * The nodes will initially share the same sprite mesh, texture, and
     * gradient. However, the position and orientation are copied by value.
     *
     * No children from this model are copied, and no children of dst are
     * modified. In addition, the parents of both node are unchanged. However,
     * all other attributes of this node are copied.
     *
     * @param dst   The node to copy into
     *
     * @return a reference to dst for chaining.
     */
    virtual std::shared_ptr<SceneNode> copy(const std::shared_ptr<SceneNode>& dst) const override;

    /**
     * Returns a shallow copy of this node.
     *
     * The nodes will initially share the same sprite mesh, texture, and
     * gradient. However, the position and orientation are copied by value.
     *
     * No children from this model are copied, and no children of dst are
     * modified. In addition, the parents of both node are unchanged. However,
     * all other attributes of this node are copied.
     *
     * @return a shallow copy of this node.
     */
    std::shared_ptr<SceneNode> clone() const;
    
    /**
     * Returns a newly allocated node with the given sprite mesh
     *
     * This node will keep a reference to sprite mesh, but it will have its own
     * position, orientation, texture, and gradient. Therefore it is possible
     * for multiple nodes to share the same sprite mesh.
     *
     * The billboard will assume that the sprite mesh is to represent a static
     * image. Therefore, there is only one animation frame in the texture
     * (e.g. rows and cols are 1).
     *
     * @param mesh  The sprite mesh
     *
     * @return a newly allocated node with the given sprite mesh
     */
    static std::shared_ptr<BillboardNode> allocWithSpriteMesh(const std::shared_ptr<graphics::SpriteMesh>& mesh) {
        std::shared_ptr<BillboardNode> result = std::make_shared<BillboardNode>();
        return (result->initWithSpriteMesh(mesh) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated node with the given JSON specification.
     *
     * This initializer is designed to receive the "data" object from the
     * JSON passed to {@link Scene3Loader}. This JSON format supports all
     * of the attribute values of its parent class. In addition, it supports
     * the following additional attribute:
     *
     *      "mesh":     A string with the name of a previously loaded sprite mesh asset
     *      "texture":  A string with the name of a previously loaded texture asset
     *      "gradient": A JSON object defining a gradient. See {@link Gradient}.
     *      "span":     The number of frames in the texture sprite sheet
     *      "rows":     An int specifying the number of rows in the sprite sheet
     *      "cols":     An int specifying the number of columns in the sprite sheet
     *      "frame":    The initial starting frame.
     *
     * All attributes are optional. However, not specifying the mesh means that
     * nothing is drawn.
     *
     * @param manager   The asset manager handling this asset
     * @param json      The JSON object specifying the node
     *
     * @return a newly allocated node with the given JSON specification.
     */
    static std::shared_ptr<BillboardNode> allocWithData(const AssetManager* manager,
                                                        const std::shared_ptr<JsonValue>& json) {
        std::shared_ptr<BillboardNode> result = std::make_shared<BillboardNode>();
        return (result->initWithData(manager,json) ? result : nullptr);
    }
    
#pragma mark Attributes
    /**
     * Returns the sprite mesh associated with this node.
     *
     * If this value is nullptr, nothing will be drawn for this node.
     *
     * @return the sprite mesh associated with this node.
     */
    std::shared_ptr<graphics::SpriteMesh> getSpriteMesh() const {
        return _mesh;
    }
    
    /**
     * Sets the sprite mesh associated with this node.
     *
     * If this value is nullptr, nothing will be drawn for this node.
     *
     * @param mesh  The sprite mesh associated with this node.
     */
    void setSpriteMesh(const std::shared_ptr<graphics::SpriteMesh>& mesh) {
        _mesh = mesh;
    }

    /**
     * Returns the texture associated with this billboard.
     *
     * If this value is nullptr, the mesh will be rendered as a single color.
     *
     * @return the texture associated with this billboard.
     */
    std::shared_ptr<graphics::Texture> getTexture() const { return _texture; }

    /**
     * Sets the texture associated with this billboard.
     *
     * If this value is nullptr, the mesh will be rendered as a single color.
     *
     * @param value The texture associated with this billboard.
     */
    void setTexture(const std::shared_ptr<graphics::Texture>& value) {
        _texture = value;
    }
    
    /**
     * Returns the gradient associated with this billboard.
     *
     * If this value is nullptr, the no gradient will be applied.
     *
     * @return the gradient associated with this billboard.
     */
    std::shared_ptr<graphics::Gradient> getGradient() const { return _gradient; }

    /**
     * Sets the gradient associated with this billboard.
     *
     * If this value is nullptr, the no gradient will be applied.
     *
     * @param value The gradient associated with this billboard.
     */
    void setGradient(const std::shared_ptr<graphics::Gradient>& value) {
        _gradient = value;
    }

#pragma mark Animation
    /**
     * Sets the sprite dimensions of the associated texture
     *
     * This method is used to divide the texture up into a sprite sheet for
     * animation. If it size is set to a value > 1, then calls to
     * {@link #setFrame} will adjust the current animation frame. This has
     * no effect if the billboard has no texture.
     *
     * Note that the sprite mesh already has its texture coordinates assigned.
     * The only way to animate the sprite is by applying an offset to the
     * texture coordinates. Therefore, for animation to work correctly, the
     * texture coordinates of the sprite mesh must all fit in a single frame
     * (the initial one). If these texture coordinate fit in a frame other than
     * frame 0, that should be specified.
     *
     * @param rows  The number of rows in the sprite sheet
     * @param cols  The number of columns in the sprite sheet
     * @param size  The number of frames in the sprite sheet
     * @param frame The initial frame represented by the sprite sheet
     */
    void setSpriteSheet(int rows, int cols, int size, int frame=0);

    /**
     * Removes all animation information, setting the billboard to a static image
     */
    void clearSpriteSheet();
    
    /**
     * Returns the number of frames in the texture sprite sheet
     *
     * @return the number of frames in the texture sprite sheet
     */
    int getSpan() const { return _size; }
    
    /**
     * Returns the current active frame.
     *
     * @return the current active frame.
     */
    unsigned int getFrame() const { return _frame; }
    
    /**
     * Sets the active frame as the given index.
     *
     * If the frame index is invalid, an error is raised.
     *
     * @param frame the index to make the active frame
     */
    void setFrame(int frame);
    
    /**
     * Returns the texture offset for the given frame.
     *
     * @return the texture offset for the given frame.
     */
    Vec2 getTextureOffset() const { return _texoffset; }
    
};
    }
}


#endif /* __CU_BILLBOARD_NODE_H__ */
