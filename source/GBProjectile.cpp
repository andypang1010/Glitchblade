//
<<<<<<< HEAD:source/GBProjectile.cpp
//  GBProjectile.cpp
=======
//  GBBullet.cpp
//  PlatformDemo
>>>>>>> fd88916c00735a7574c04a0efd54f6274e134bf4:source/GBBullet.cpp
//
//  This class is a simple extension of WheelObstacle in order to simplify the
//  process of adding and removing Projectiles from the game world, as well as for
//  drawing the sprite of the projectile.
//
//  Author:  Walker White and Anthony Perello
//  Version: 2/9/21
//
<<<<<<< HEAD:source/GBProjectile.cpp
#include "GBProjectile.h"
=======
#include "GBBullet.h"
>>>>>>> fd88916c00735a7574c04a0efd54f6274e134bf4:source/GBBullet.cpp
#include <cugl/scene2/CUPolygonNode.h>
#include <cugl/scene2/CUTexturedNode.h>
#include <cugl/core/assets/CUAssetManager.h>

#pragma mark -
#pragma mark Initializers

using namespace cugl;
using namespace cugl::graphics;
using namespace cugl::physics2;
using namespace cugl::scene2;

/**
 * Updates the object's physics state (NOT GAME LOGIC).
 *
 * We use this method to reset cooldowns.
 *
 * @param delta Number of seconds since last animation frame
 */
void Projectile::update(float dt) {
    WheelObstacle::update(dt);

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
void Projectile::dispose() {
    _node = nullptr;
}




