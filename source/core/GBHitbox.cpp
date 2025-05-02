#include "../core/GBHitbox.h"
#include <cugl/scene2/CUPolygonNode.h>
#include <cugl/scene2/CUTexturedNode.h>
#include <cugl/core/assets/CUAssetManager.h>

#pragma mark -
#pragma mark Initializers

using namespace cugl;
using namespace cugl::graphics;
using namespace cugl::physics2;
using namespace cugl::scene2;

#define FPS 60

bool Hitbox::init(std::shared_ptr<EnemyModel> enemy, Vec2& pos, const Size& size, float scale, int damage, float duration, bool parriable) {
    _drawScale = scale;
    setDuration(duration);
    setDamage(damage);
	setIsParriable(parriable);
    if (!enemy->isFacingRight()) {
        pos.x = -pos.x;
    }
    if (BoxObstacle::init(enemy->getPosition() + pos, size)) {
        setFixedRotation(true); // OTHERWISE, HE IS A WEEBLE WOBBLE
        setSensor(true);
        setName("hitbox");
        setGravityScale(0);
        setDensity(0);
        _enemy = enemy;
        _offset = pos;
        _node = scene2::SceneNode::alloc();
        setSceneNode(_node);
        return true;
    }
    return false;

}

/**
 * Updates the object's physics state (NOT GAME LOGIC).
 *
 * We use this method to reset cooldowns.
 *
 * @param delta Number of seconds since last animation frame
 */
void Hitbox::update(float dt) {
	BoxObstacle::update(dt);
    setPosition(_enemy->getPosition() + _offset);

    _duration -= dt / (1.0 / FPS);
    if (_duration <= 0 || _enemy->isStunned()) {
        markRemoved(true);
    }

    if (_node != nullptr) {
        _node->setPosition(getPosition()*_drawScale);
        _node->setAngle(getAngle());
    }
}

/**
 * Disposes all resources and assets of this Projectile
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a Projectile may not be used until it is initialized again.
 */
void Hitbox::dispose() {
	_node = nullptr;
}
