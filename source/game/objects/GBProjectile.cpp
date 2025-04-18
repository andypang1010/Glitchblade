//
//  GBProjectile.cpp
//
//  This class is a simple extension of WheelObstacle in order to simplify the
//  process of adding and removing Projectiles from the game world, as well as for
//  drawing the sprite of the projectile.
//
//  Author:  Walker White and Anthony Perello
//  Version: 2/9/21
//
#include "GBProjectile.h"
#include <cugl/scene2/CUPolygonNode.h>
#include <cugl/scene2/CUTexturedNode.h>
#include <cugl/core/assets/CUAssetManager.h>
#include "../../core/GBTypes.h"
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
	BoxObstacle::update(dt);
    if (_spriteNode != nullptr) {
        _spriteNode->setPosition(getPosition()*_drawScale);
        _spriteNode->setAngle(getAngle());
    
        frameCounter++;
		if (frameCounter % 4 == 0) {
            _spriteNode->setFrame((_spriteNode->getFrame() + 1) % _spriteNode->getCount());
            frameCounter = 0;
		}
    }
}


ObstacleNodePair Projectile::createProjectileNodePair(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& constantsRef, Vec2 spawnPos, std::shared_ptr<Projectile> projectile, bool isFacingRight)
{
    float scale = constantsRef->get("scene")->getFloat("scale");
    std::shared_ptr<JsonValue> projJ = projectile->getIsPlayerFired() ? constantsRef->get("player_projectile") : constantsRef->get("projectile");
    std::shared_ptr<JsonValue> physicsJ = constantsRef->get("physics");
    std::shared_ptr<JsonValue> fxJ = constantsRef->get("audio")->get("effects");

    std::shared_ptr<Projectile> newProjectile = Projectile::allocWithProjectile(spawnPos + (projectile->getSpawnOffset() * Vec2(isFacingRight ? 1 : -1, 1)), projectile);
    newProjectile->setName(projJ->getString("name"));
    newProjectile->setDensity(physicsJ->getFloat("heavy_density"));
    newProjectile->setBullet(true);
    newProjectile->setGravityScale(0);
    newProjectile->setDebugColor(physicsJ->get("debug")->getString("color"));
    newProjectile->setDrawScale(scale);
    newProjectile->setSensor(true);
    newProjectile->setLinearVelocity(projectile->getLinearVelocity() * Vec2(isFacingRight ? 1 : -1, 1));

    std::shared_ptr<scene2::SpriteNode> newSprite = scene2::SpriteNode::allocWithSprite(projectile->getSceneNode());
    newSprite->flipHorizontal(newProjectile->getLinearVelocity().x < 0);
    newSprite->setPosition(projectile->getAnimOffset());
//    CULog(projectile->getAnimOffset().toString().c_str());
	newSprite->setFrame(0);

	newProjectile->setSceneNode(newSprite);

    ObstacleNodePair proj_pair = std::make_pair(newProjectile, newSprite);
    return proj_pair;
}

/**
 * Disposes all resources and assets of this Projectile
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a Projectile may not be used until it is initialized again.
 */
void Projectile::dispose() {
	_spriteNode = nullptr;
    _debug = nullptr;
}
