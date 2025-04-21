//
//  GBProjectile.h
//
//  This class is a simple extension of WheelObstacle in order to simplify the process of adding and removing Projectiles
//  from the game world, as well as for drawing the sprite of the projectile.
//
//  Author: Walker White and Anthony Perello
//  Version:  2/9/17
//
#ifndef __GB_PROJECTILE_MODEL_H__
#define __GB_PROJECTILE_MODEL_H__
#include <cugl/cugl.h>
#include "../../core/GBTypes.h"

using namespace cugl;

#pragma mark -
#pragma mark Projectile Model
class Projectile : public physics2::BoxObstacle {
private:
    /** This macro disables the copy constructor (not allowed on physics objects) */
    CU_DISALLOW_COPY_AND_ASSIGN(Projectile);
    
protected:
    /** The scale between the physics world and the screen (MUST BE UNIFORM) */
    float _drawScale;

    /** The scene graph node for the Projectile. */
    std::shared_ptr<scene2::SpriteNode> _spriteNode;
    Vec2 _spawnOffset;
    Vec2 _animOffset;
    Size _size;
    float _damage;
    bool _isPlayerFired;
    int frameCounter = 0;

public:
#pragma mark Constructors
    /**
     * Creates a degenerate Projectile object.
     *
     * This constructor does not initialize any of the Projectile values beyond
     * the defaults.  To use a Projectile, you must call init().
     */
    Projectile() : BoxObstacle() { }
    
    /**
     * Destroys this Projectile, releasing all resources.
     */
    virtual ~Projectile(void) { dispose(); }
    
    /**
     * Disposes all resources and assets of this Projectile
     *
     * Any assets owned by this object will be immediately released.  Once
     * disposed, a Projectile may not be used until it is initialized again.
     */
    void dispose();
    
#pragma mark -
#pragma mark Static Constructors
    /**
     * Creates a new Projectile at the given position.
     *
     * The projectile is scaled so that 1 pixel = 1 Box2d unit
     *
     * The scene graph is completely decoupled from the physics system.
     * The node does not have to be the same size as the physics body. We
     * only guarantee that the scene graph node is positioned correctly
     * according to the drawing scale.
     *
     * @param  pos      Initial position in world coordinates
     * @param  radius   The radius of the Projectile obstacle.
     *
     * @return  A newly allocated Projectile at the given position, with the given radius
     */
    static std::shared_ptr<Projectile> alloc(const Vec2& pos, Size size, bool isPlayerFired) {
        std::shared_ptr<Projectile> result = std::make_shared<Projectile>();
        result->setIsPlayerFired(isPlayerFired);
        return (result->init(pos, size) ? result : nullptr);
    }

    static std::shared_ptr<Projectile> allocWithProjectile(const Vec2& pos, const std::shared_ptr<Projectile> projectile) {
		std::shared_ptr<Projectile> result = std::make_shared<Projectile>();
		result->setIsPlayerFired(projectile->getIsPlayerFired());
		return (result->init(pos, projectile->getSize()) ? result : nullptr);
    }

#pragma mark -
#pragma mark Animation
    /**
     * Returns the scene graph node representing this Projectile.
     *
     * By storing a reference to the scene graph node, the model can update
     * the node to be in sync with the physics info. It does this via the
     * {@link Obstacle#update(float)} method.
     *
     * @return the scene graph node representing this Projectile.
     */
    const std::shared_ptr<scene2::SpriteNode>& getSceneNode() const { return _spriteNode; }
    
    /**
     * Sets the scene graph node representing this Projectile.
     *
     * @param node  The scene graph node representing this Projectile, which has
     *              been added to the world node already.
     */
    void setSceneNode(const std::shared_ptr<scene2::SpriteNode>& node) {
        _spriteNode = node;
    }
    
    /**
     * Sets the ratio of the Projectile sprite to the physics body
     *
     * The Projectile needs this value to convert correctly between the physics
     * coordinates and the drawing screen coordinates.  Otherwise it will
     * interpret one Box2D unit as one pixel.
     *
     * All physics scaling must be uniform.  Rotation does weird things when
     * attempting to scale physics by a non-uniform factor.
     *
     * @param scale The ratio of the Projectile sprite to the physics body
     */
    void setDrawScale(float scale) {
        _drawScale = scale;
    }
    float getDamage() { return _damage;}
    void setDamage(float dmg) {_damage = dmg;}
    
#pragma mark -
#pragma mark Attribute Properties
    /**
    * Returns the spawn offset of the projectile.
    *
    * @return The spawn offset as a Vec2.
    */
    Vec2 getSpawnOffset() const {
        return _spawnOffset;
    }

    /**
    * Sets the spawn offset of the projectile.
    *
    * @param offset The new spawn offset as a Vec2.
    */
    void setSpawnOffset(const Vec2& offset) {
        _spawnOffset = offset;
    }
    /**
* Returns the spawn offset of the projectile.
*
* @return The spawn offset as a Vec2.
*/
    Vec2 getAnimOffset() const {
        return _animOffset;
    }

    /**
    * Sets the spawn offset of the projectile.
    *
    * @param offset The new spawn offset as a Vec2.
    */
    void setAnimOffset(const Vec2& offset) {
        _animOffset = offset;
    }

    /**
    * Returns the size of the projectile.
    *
    * @return The size as a Size object.
    */
    Size getSize() const {
        return _size;
    }

    /**
    * Sets the size of the projectile.
    *
    * @param size The new size as a Size object.
    */
    void setSize(const Size& size) {
        _size = size;
    }
    /**
     * Returns whether the projectile is fired by player or not.
     *
     * @return True iff player fired this projectile.
     */
    bool getIsPlayerFired() const { return _isPlayerFired; }

    /**
     * Sets the shooter of the projectile
     *
     * @param isPlayerFired
     */
    void setIsPlayerFired(float isPlayerFired) {
        _isPlayerFired = isPlayerFired;
    }
#pragma mark -
#pragma mark Creating, Destroying Projectiles

    static ObstacleNodePair createProjectileNodePair(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& constantsRef, Vec2 spawnPos, std::shared_ptr<Projectile> projectile, bool isFacingRight);

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

#endif /* __GB_PROJECTILE_MODEL_H__ */
