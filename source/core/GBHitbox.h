#ifndef __GB_HITBOX_MODEL_H__
#define __GB_HITBOX_MODEL_H__
#include <cugl/cugl.h>
#include "../enemies/GBEnemyModel.h"

using namespace cugl;

#pragma mark -
#pragma mark Hitbox Model
class Hitbox : public physics2::BoxObstacle {
private:
    /** This macro disables the copy constructor (not allowed on physics objects) */
    CU_DISALLOW_COPY_AND_ASSIGN(Hitbox);

protected:
    /** The enemy that caused this hitbox */
    std::shared_ptr<EnemyModel> _enemy;
    /** The scene graph node for the Projectile. */
    std::shared_ptr<scene2::SceneNode> _node;
    /** The hitbox's relative position to the enemy */
    Vec2 _offset;
    /** The scale between the physics world and the screen (MUST BE UNIFORM) */
    float _drawScale;
    /** The remaining duration in frames of this hitbox */
    float _duration;
    /** The damage of this hitbox */
    int _damage;
	/** The knockback of this hitbox */
    float _knockback;
    /** Whether the hitbox attack can be parried to stun the enemy */
	bool _isParriable;

public:
#pragma mark Constructors
    /**
     * Creates a degenerate Projectile object.
     *
     * This constructor does not initialize any of the Projectile values beyond
     * the defaults.  To use a Projectile, you must call init().
     */
    Hitbox() : BoxObstacle() { }
    
    /**
     * Destroys this Projectile, releasing all resources.
     */
    virtual ~Hitbox(void) { dispose(); }
    
    /**
     * Disposes all resources and assets of this Projectile
     *
     * Any assets owned by this object will be immediately released.  Once
     * disposed, a Projectile may not be used until it is initialized again.
     */
    void dispose();

    virtual bool init(std::shared_ptr<EnemyModel> enemy, Vec2& pos, const Size& size, float scale, int damage, float knockback, float duration, bool parriable);
    
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
    static std::shared_ptr<Hitbox> alloc(std::shared_ptr<EnemyModel> enemy, Vec2& pos, const Size& size, float scale, int damage, float knockback, float duration, bool parriable) {
        std::shared_ptr<Hitbox> result = std::make_shared<Hitbox>();
        return (result->init(enemy, pos, size, scale, damage, knockback, duration, parriable) ? result : nullptr);
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
    const std::shared_ptr<scene2::SceneNode>& getSceneNode() const { return _node; }
    
    /**
     * Sets the scene graph node representing this Projectile.
     *
     * @param node  The scene graph node representing this Projectile, which has
     *              been added to the world node already.
     */
    void setSceneNode(const std::shared_ptr<scene2::SceneNode>& node) {
        _node = node;
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

#pragma mark -
#pragma mark Attribute Properties
    /**
     * Returns the enemy that created this hitbox.
     *
     * @return the hitbox's corresponding enemy.
     */
    std::shared_ptr<EnemyModel> getEnemy() const { return _enemy; }

    /**
     * Returns the position offset of this hitbox.
     *
     * @return the position relative to the enemy that caused this hitbox.
     */
    Vec2 getOffset() const { return _offset; }

    /**
     * Sets the relative position of this hitbox
     *
     * @param value new position
     */
    void setOffset(Vec2 value) {
        _offset = value;
    }

    /**
     * Returns the remaining time before this hitbox deactivates.
     *
     * @return the remaining duration(in frames) of this hitbox.
     */
    float getDuration() const { return _duration; }

    /**
     * Sets the duration of the hitbox
     *
     * @param value new duration
     */
    void setDuration(float value) {
        _duration = value;
    }

    /**
     * Returns the damage of this hitbox.
     *
     * @return the damage of this hitbox.
     */
    int getDamage() const { return _damage; }

    /**
     * Sets the damage of the hitbox
     *
     * @param value new damage
     */
    void setDamage(int value) {
        _damage = value;
    }

    /**
     * Returns the knockback magnitude of this hitbox.
     *
     * @return the knockback of this hitbox.
     */
	float getKnockback() const { return _knockback; }

    /**
     * Sets the knockback of the hitbox
     *
     * @param value new knockback
     */
	void setKnockback(float value) {
		_knockback = value;
	}

	/**
	 * Returns whether the hitbox can be parried
	 *
	 * @return true if the hitbox can be parried, false otherwise
	 */
	bool getIsParriable() const { return _isParriable; }

	/**
	 * Sets whether the hitbox can be parried
	 *
	 * @param value new parriable value
	 */
	void setIsParriable(bool value) {
		_isParriable = value;
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

#endif /* __GB_HITBOX_MODEL_H__ */
