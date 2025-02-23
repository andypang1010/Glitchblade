//
//  GBBullet.h
//  PlatformDemo
//
//  This class is a simple extension of WheelObstacle in order to simplify the process of adding and removing Bullets
//  from the game world, as well as for drawing the sprite of the bullet.
//
//  Author: Walker White and Anthony Perello
//  Version:  2/9/17
//
#ifndef __GB_BULLET_MODEL_H__
#define __GB_BULLET_MODEL_H__
#include <cugl/cugl.h>

using namespace cugl;

#pragma mark -
#pragma mark Bullet Model
class Bullet : public physics2::WheelObstacle {
private:
	/** This macro disables the copy constructor (not allowed on physics objects) */
	CU_DISALLOW_COPY_AND_ASSIGN(Bullet);

protected:
	/** The scene graph node for the Bullet. */
	std::shared_ptr<scene2::SceneNode> _node;
	/** The scale between the physics world and the screen (MUST BE UNIFORM) */
	float _drawScale;

public:
#pragma mark Constructors
    /**
     * Creates a degenerate Bullet object.
     *
     * This constructor does not initialize any of the Bullet values beyond
     * the defaults.  To use a Bullet, you must call init().
     */
    Bullet() : WheelObstacle() { }
    
    /**
     * Destroys this Bullet, releasing all resources.
     */
    virtual ~Bullet(void) { dispose(); }
    
    /**
     * Disposes all resources and assets of this Bullet
     *
     * Any assets owned by this object will be immediately released.  Once
     * disposed, a Bullet may not be used until it is initialized again.
     */
    void dispose();
    
#pragma mark -
#pragma mark Static Constructors
    /**
     * Creates a new Bullet at the given position.
     *
     * The bullet is scaled so that 1 pixel = 1 Box2d unit
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param  pos      Initial position in world coordinates
     * @param  radius   The radius of the Bullet obstacle.
     *
     * @return  A newly allocated Bullet at the given position, with the given radius
     */
    static std::shared_ptr<Bullet> alloc(const Vec2& pos, float radius) {
        std::shared_ptr<Bullet> result = std::make_shared<Bullet>();
        return (result->init(pos, radius) ? result : nullptr);
    }

#pragma mark -
#pragma mark Animation
    /**
     * Returns the scene graph node representing this Bullet.
     *
     * By storing a reference to the scene graph node, the model can update
     * the node to be in sync with the physics info. It does this via the
     * {@link Obstacle#update(float)} method.
     *
     * @return the scene graph node representing this Bullet.
     */
    const std::shared_ptr<scene2::SceneNode>& getSceneNode() const { return _node; }
    
    /**
     * Sets the scene graph node representing this Bullet.
     *
     * @param node  The scene graph node representing this Bullet, which has 
     *              been added to the world node already.
     */
    void setSceneNode(const std::shared_ptr<scene2::SceneNode>& node) {
        _node = node;
    }
    
    /**
     * Sets the ratio of the Bullet sprite to the physics body
     *
     * The Bullet needs this value to convert correctly between the physics
     * coordinates and the drawing screen coordinates.  Otherwise it will
     * interpret one Box2D unit as one pixel.
     *
     * All physics scaling must be uniform.  Rotation does weird things when
     * attempting to scale physics by a non-uniform factor.
     *
     * @param scale The ratio of the Bullet sprite to the physics body
     */
    void setDrawScale(float scale) {
        _drawScale = scale;
    }

#pragma mark -
#pragma mark Physics Methods
	/**
	* Updates the object's physics state (NOT GAME LOGIC).
	*
	* We use this method to reset cooldowns.
	*
	* @param delta Number of seconds since last animation frame
	*/
	void update(float dt) override;



};

#endif /* __GB_BULLET_MODEL_H__ */
