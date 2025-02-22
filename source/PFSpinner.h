//
//  GBSpinner.h
//  PlatformerDemo
//
//  This class provides a spinning rectangle on a fixed pin, as ComplexObstacles
//  always make joint management easier.
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
#ifndef __GB_SPINNER_H__
#define __GB_SPINNER_H__
#include <cugl/cugl.h>

using namespace cugl;

#pragma mark -
#pragma mark Spinner Constants
/** The key for the spinner texture */
#define SPINNER_TEXTURE "spinner"
/** The debug name for the entire obstacle */
#define SPINNER_NAME    "spinner"
/** The debug name for the spinning barrier */
#define BARRIER_NAME    "barrier"
/** The debug name for the central pin */
#define SPIN_PIN_NAME   "pin"


#pragma mark -
#pragma mark Spinner Obstacle
/**
* Spinning platform for the plaform game.
*
* We did not really need a separate class for this, as it has no update.  However, 
* ComplexObstacles always make joint management easier.
*/
class Spinner {
private:
    /** The object name */
    std::string _name;
    /** The collection of obstacles in this model */
    std::vector<std::shared_ptr<physics2::Obstacle>> _obstacles;
    /** The collection of joints in this model */
    std::vector<std::shared_ptr<physics2::Joint>> _joints;
	/** The scene graph node for the Spinner. */
	std::shared_ptr<scene2::SceneNode> _node;
	/** The scale between the physics world and the screen (MUST BE UNIFORM) */
	float _drawScale;
    /** The image used to draw the Spinner */
	std::shared_ptr<graphics::Texture> _image;

public:
#pragma mark Constructors
    /**
     * Creates a degenerate spinner object.
     *
     * This constructor does not initialize any of the spinner values beyond
     * the defaults.  To use a Spinner, you must call init().
     */
    Spinner() : _drawScale(0) { }
    
    /**
     * Destroys this Spinner, releasing all resources.
     */
    virtual ~Spinner(void) { dispose(); }
    
    /**
     * Disposes all resources and assets of this Spinner
     *
     * Any assets owned by this object will be immediately released.  Once
     * disposed, a Spinner may not be used until it is initialized again.
     */
    void dispose();
    
    /**
     * Initializes a new spinner at the origin.
     *
     * The spinner is a unit square, scaled so that 1 pixel = 1 Box2d unit.
     * The spinner is centered on its position.
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    bool init() {
        return init(Vec2::ZERO, Size(1,1), 1.0f);
    }
    
    /**
     * Initializes a new spinner at the given position.
     *
     * The spinner is a unit square, scaled so that 1 pixel = 1 Box2d unit.
     * The spinner is centered on its position.
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param pos   Initial position in world coordinates
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    bool init(const Vec2 pos) {
        return init(pos, Size(1,1), 1.0f);
    }
    
    /**
     * Initializes a new spinner at the given position.
     *
     * The spinner has the given size, scaled so that 1 pixel = 1 Box2d unit.
     * The spinner is centered on its position.
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param pos   Initial position in world coordinates
     * @param size  The spinner size in world units
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    bool init(const Vec2 pos, const Vec2 size) {
        return init(pos, size, 1.0f);
    }
    
    /**
     * Initializes a new spinner at the given position.
     *
     * The spinner is sized according to the given drawing scale. The spinner 
     * is centered on its position.
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param pos   Initial position in world coordinates
     * @param size  The spinner size in world units
     * @param scale The drawing scale (world to screen)
     *
     * @return  true if the obstacle is initialized properly, false otherwise.
     */
    bool init(const Vec2 pos, const Vec2 size, float scale);

    
#pragma mark -
#pragma mark Static Constructors
	/**
	 * Creates a new spinner at the origin.
	 *
     * The spinner is a unit square, scaled so that 1 pixel = 1 Box2d unit.
     * The spinner is centered on its position.
	 *
	 * The scene graph is completely decoupled from the physics system.
	 * The node does not have to be the same size as the physics body. We
	 * only guarantee that the scene graph node is positioned correctly
	 * according to the drawing scale.
	 *
	 * @return  A newly allocated Spinner at the origin
	 */
	static std::shared_ptr<Spinner> alloc() {
		std::shared_ptr<Spinner> result = std::make_shared<Spinner>();
		return (result->init() ? result : nullptr);
	}

    /**
     * Creates a new spinner at the given position.
     *
     * The spinner is a unit square, scaled so that 1 pixel = 1 Box2d unit.
     * The spinner is centered on its position.
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param pos   Initial position in world coordinates
     *
     * @return  A newly allocated spinner at the given position
     */
    static std::shared_ptr<Spinner> alloc(const Vec2& pos) {
        std::shared_ptr<Spinner> result = std::make_shared<Spinner>();
        return (result->init(pos) ? result : nullptr);
    }
    
    /**
     * Creates a new spinner at the given position.
     *
     * The spinner has the given size, scaled so that 1 pixel = 1 Box2d unit.
     * The spinner is centered on its position.
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param pos   Initial position in world coordinates
     * @param size  The spinner size in world units
     *
     * @return  A newly allocated spinner at the given position, with the given size.
     */
    static std::shared_ptr<Spinner> alloc(const Vec2& pos,
                                          const Size& size) {
        std::shared_ptr<Spinner> result = std::make_shared<Spinner>();
        return (result->init(pos, size) ? result : nullptr);
    }
    
    /**
     * Creates a new spinner at the given position.
     *
     * The spinner is sized according to the given drawing scale. The spinner
     * is centered on its position.
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param pos   Initial position in world coordinates
     * @param size  The spinner size in world units
     * @param scale The drawing scale (world to screen)
     *
     * @return  A newly allocated spinner at the given position, with the given drawing scale.
     */
    static std::shared_ptr<Spinner> alloc(const Vec2& pos,
                                          const Size& size, float scale) {
        std::shared_ptr<Spinner> result = std::make_shared<Spinner>();
        return (result->init(pos, size, scale) ? result : nullptr);
    }


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
	 * Returns the scene graph node representing this Spinner.
	 *
	 * By storing a reference to the scene graph node, the model can update
	 * the node to be in sync with the physics info. It does this via the
	 * {@link Obstacle#update(float)} method.
	 *
	 * @return the scene graph node representing this Spinner.
	 */
	const std::shared_ptr<scene2::SceneNode>& getSceneNode() const { return _node; }

    /**
     * Sets the scene graph node representing this Spinner.
     *
     * Note that this method also handles creating the nodes for the body parts
     * of this Spinner. Since the obstacles are decoupled from the scene graph,
     * initialization (which creates the obstacles) occurs prior to the call to
     * this method. Therefore, to be drawn to the screen, the nodes of the attached
     * bodies must be added here.
     *
     * By storing a reference to the scene graph node, the model can update
     * the node to be in sync with the physics info. It does this via the
     * {@link Obstacle#update(float)} method.
     *
     * @param node  The scene graph node representing this Spinner, which has 
     *              been added to the world node already.
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

#endif /* __GB_SPINNER_H__ */
