//
//  PFRopeBridge.h
//  PlatformerDemo
//
//  This module provides the connected ropebridge from 3152. This time it is fully
//  assembled for you.  Note that this module handles its own scene graph management.
//  As a ComplexObstacle owns all of its child obstacles, it is natural for it to
//  own the corresponding scene graph.
//
//  WARNING: There are a lot of shortcuts in this design that will do not adapt
//  well to data driven design. This demo has a lot of simplifications to make
//  it a bit easier to see how everything fits together. However, the model
//  classes and how they are initialized will need to be changed if you add
//  dynamic level loading.
//
//  This file is based on the CS 3152 PhysicsDemo Lab by Don Holden (2007).
//  This file has been refactored to support the physics changes in CUGL 2.5.
//
//  Author: Walker White and Anthony Perello
//  Version:  2/9/24
//
#ifndef __PF_ROPE_BRIDGE_H__
#define __PF_ROPE_BRIDGE_H__
#include <cugl/cugl.h>

using namespace cugl;

#pragma mark -
#pragma mark Bridge Constants
/** The key for the texture for the bridge planks */
#define BRIDGE_TEXTURE "bridge"
/** The debug name for the entire obstacle */
#define BRIDGE_NAME     "bridge"
/** The debug name for each plank */
#define PLANK_NAME      "barrier"
/** The debug name for each anchor pin */
#define BRIDGE_PIN_NAME "pin"


#pragma mark -
#pragma mark Rope Bridge
/**
* A bridge with planks connected by revolute joints.
*
* Note that this module handles its own scene graph management.  As a ComplexObstacle
* owns all of its child obstacles, it is natural for it to own the corresponding
* scene graph.
*/
class RopeBridge {
protected:
    /** The object name */
    std::string _name;
    /** The collection of obstacles in this model */
    std::vector<std::shared_ptr<physics2::Obstacle>> _obstacles;
    /** The collection of joints in this model */
    std::vector<std::shared_ptr<physics2::Joint>> _joints;
	/** The width of a single plank in the bridge */
	float _linksize;
	/** A parent scene graph node for the bridge. */
	std::shared_ptr<scene2::SceneNode> _node;
	/** The scale between the physics world and the screen (MUST BE UNIFORM) */
	float _drawScale;
    /** The image that is used for drawing links in the bridge */
	std::shared_ptr<graphics::Texture> _image;

    
public:
#pragma mark Constructors
    /**
     * Creates a degenerate rope bridge.
     *
     * This constructor does not initialize any of the RopeBridge values beyond
     * the defaults.  To use a rope bridge, you must call init().
     */
    RopeBridge() : _linksize(0), _drawScale(0) { }
    
    /**
     * Destroys this RopeBridge, releasing all resources.
     */
    virtual ~RopeBridge(void) { dispose(); }
    
    /**
     * Disposes all resources and assets of this RopeBridge
     *
     * Any assets owned by this object will be immediately released.  Once
     * disposed, a RopeBridge may not be used until it is initialized again.
     */
    void dispose();
    
    /**
     * Initializes a new RopeBridge at the origin.
     *
     * The RopeBridge is scaled so that 1 pixel = 1 Box2d unit. The rope bridge
     * has a single plank and is one unit long.
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    bool init() {
        return init(Vec2::ZERO, Vec2::ONE);
    }
    
    /**
     * Initializes a new RopeBridge with the given start and end positions
     *
     * The RopeBridge is scaled so that 1 pixel = 1 Box2d unit.  The rope bridge
     * has a single plank.
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param start The starting position of the bridge, in world coordinates
     * @param end   The ending position of the bridge, in world coordinates
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    bool init(const Vec2& start, const Vec2& end) {
        return init(start, end, end-start, 1.0f);
    }
    
    /**
     * Initializes a new RopeBridge with the given start and end positions
     *
     * The RopeBridge is scaled so that 1 pixel = 1 Box2d unit.  The number of
     * planks in the bridge is determined by psize.
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param start The starting position of the bridge, in world coordinates
     * @param end   The ending position of the bridge, in world coordinates
     * @param psize The size of a single plank, in world units
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    bool init(const Vec2& start, const Vec2& end,
              const Size& psize) {
        return init(start, end, psize, 1.0f);
    }
    
    /**
     * Initializes a new RopeBridge with the given start and end positions, and drawing scale.
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param start The starting position of the bridge, in world coordinates
     * @param end   The ending position of the bridge, in world coordinates
     * @param psize The size of a single plank, in world units
     * @param scale The drawing scale (world to screen)
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    bool init(const Vec2& start, const Vec2& end,
              const Size& psize, float scale);

    
#pragma mark -
#pragma mark Static Constructors
	/**
     * Creates a new RopeBridge at the origin.
     *
     * The RopeBridge is scaled so that 1 pixel = 1 Box2d unit. The rope bridge
     * has a single plank and is one unit long.
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @return  A newly allocated RopeBridge at the origin
     */
	static std::shared_ptr<RopeBridge> alloc() {
		std::shared_ptr<RopeBridge> result = std::make_shared<RopeBridge>();
		return (result->init() ? result : nullptr);
	}

	/**
     * Creates a new RopeBridge with the given start and end positions
     *
     * The RopeBridge is scaled so that 1 pixel = 1 Box2d unit. The rope bridge
     * has a single plank.
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param  start   The starting position of the bridge, in world coordinates
     * @param  end     The ending position of the bridge, in world coordinates
     *
     * @return  A newly allocated RopeBridge with the given start and end positions
     */
	static std::shared_ptr<RopeBridge> alloc(const Vec2& start, 
                                             const Vec2& end) {
		std::shared_ptr<RopeBridge> result = std::make_shared<RopeBridge>();
		return (result->init(start, end) ? result : nullptr);
	}

	/**
     * Creates a new RopeBridge with the given start and end positions
     *
     * The RopeBridge is scaled so that 1 pixel = 1 Box2d unit.  The number of
     * planks in the bridge is determined by psize.
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param start The starting position of the bridge, in world coordinates
     * @param end   The ending position of the bridge, in world coordinates
     * @param psize The size of a single plank, in world units
     *
     * @return  A newly allocated RopeBridge with the given start and end positions and drawing scale
     */
	static std::shared_ptr<RopeBridge> alloc(const Vec2& start,
                                             const Vec2& end,
                                             const Size& psize) {
		std::shared_ptr<RopeBridge> result = std::make_shared<RopeBridge>();
		return (result->init(start, end, psize) ? result : nullptr);
	}

    /**
     * Creates a new RopeBridge with the given start and end positions, and drawing scale.
     *
     * The RopeBridge is sized according to the given drawing scale.
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param start The starting position of the bridge, in world coordinates
     * @param end   The ending position of the bridge, in world coordinates
     * @param psize The size of a single plank, in world units
     * @param scale The drawing scale (world to screen)
     *
     * @return  A newly allocated RopeBridge with the given start and end positions and drawing scale
     */
    static std::shared_ptr<RopeBridge> alloc(const Vec2& start,
                                             const Vec2& end,
                                             const Size& psize, float scale) {
		std::shared_ptr<RopeBridge> result = std::make_shared<RopeBridge>();
		return (result->init(start, end, psize, scale) ? result : nullptr);
	}
    
    
#pragma mark -
#pragma mark Physics Methods
    /**
     * Activates all of the obstacles in this model.
     *
     * This method invokes {@link Obstacle#activatePhysics} for the each
     * of the obstacles in this model.
     *
     * @param world    The associated obstacle world
     */
    void activate(const std::shared_ptr<physics2::ObstacleWorld>& world);
    
    /**
     * Destroys the obstacles in this model, if appropriate,
     * removing them from the world.
     *
     * @param world    The associated obstacle world
     */
    void deactivate(const std::shared_ptr<physics2::ObstacleWorld>& world);

    /**
     * Creates the joints for this object.
     *
     * This method defines the joints but does not activate them. That is the
     * purpose of {@link #activate}.
     *
     * @return true if object allocation succeeded
     */
    bool createJoints();
    
#pragma mark -
#pragma mark Animation
    /**
     * Returns the plank texture
     * 
     * This texture is used to construct the individual scene graph nodes
     * for each plank
     *
     * @return the plank texture
     */
    const std::shared_ptr<graphics::Texture>& getTexture() const { return _image; }
    
    /**
     * Sets the plank texture
     *
     * This texture is used to construct the individual scene graph nodes
     * for each plank
     *
     * @param texture   The plank texture
     */
    void setTexture(const std::shared_ptr<graphics::Texture>& texture);
    
	/**
     * Returns the scene graph node representing this RopeBridge.
     *
     * By storing a reference to the scene graph node, the model can update
     * the node to be in sync with the physics info. It does this via the
     * {@link Obstacle#update(float)} method.
     *
     * @return the scene graph node representing this RopeBridge.
     */
	const std::shared_ptr<scene2::SceneNode>& getNode() const { return _node; }

	/**
     * Sets the scene graph node representing this RopeBridge.
     *
     * Note that this method also handles creating the nodes for the body parts
     * of this RopeBridge. Since the obstacles are decoupled from the scene graph,
     * initialization (which creates the obstacles) occurs prior to the call to
     * this method. Therefore, to be drawn to the screen, the nodes of the attached
     * bodies must be added here.
     *
     * By storing a reference to the scene graph node, the model can update
     * the node to be in sync with the physics info. It does this via the
     * {@link Obstacle#update(float)} method.
     *
     * @param node  The scene graph node representing this RopeBridge, which has been added to the world node already.
     */
	void setSceneNode(const std::shared_ptr<scene2::SceneNode>& node);

    /**
     * Sets the parent scene graph node for the debug wireframe
     *
     * The given node is the parent coordinate space for drawing physics.
     * All debug nodes for physics objects are drawn within this coordinate
     * space.  Setting the visibility of this node to false will disable
     * any debugging.  Similarly, setting this value to nullptr will
     * disable any debugging.
     *
     * This scene graph node is intended for debugging purposes only.  If
     * you want a physics body to update a proper texture image, you should
     * either use the method {@link update(float)} for subclasses or
     * {@link setListener} for decoupled classes.
     *
     * @param node  the parent scene graph node for the debug wireframe
     */
    void setDebugScene(const std::shared_ptr<scene2::SceneNode>& node);
    
    /**
     * Sets the color of the debug wireframe.
     *
     * The default color is white, which means that the objects will be shown
     * with a white wireframe.
     *
     * @param color the color of the debug wireframe.
     */
    void setDebugColor(Color4 color);
    
    /**
     * Sets the ratio of the Ragdoll sprite to the physics body
     *
     * The Ragdoll needs this value to convert correctly between the physics
     * coordinates and the drawing screen coordinates.  Otherwise it will
     * interpret one Box2D unit as one pixel.
     *
     * All physics scaling must be uniform.  Rotation does weird things when
     * attempting to scale physics by a non-uniform factor.
     *
     * @param scale The ratio of the Ragdoll sprite to the physics body
     */
    void setDrawScale(float scale);
    
    
#pragma mark -
#pragma mark Interpolation
    /**
     * Updates the scene graph to interpolate the visuals.
     *
     * This method is called AFTER all physics is complete. The value delta
     * is the time remaining that could not be simulated. The objects in the
     * scene graph node are interpolated to match that time.
     *
     * @param delta The time since the last physics simulation call
     */
	void update(float delta);

};

#endif /* __PF_ROPE_BRIDGE_H__ */
