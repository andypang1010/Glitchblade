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
#include "GBTypes.h"
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
 * Add a new projectile to the world and send it in the right direction.
 */
ObstacleNodePair Projectile::createProjectile(const std::shared_ptr<AssetManager>& assetRef,const std::shared_ptr<JsonValue>& constantsRef, Vec2 pos, Vec2 direction, bool isPlayerFired, bool face_right) {
    float offset = constantsRef->get("projectile")->getFloat("offset");
    float scale = constantsRef->get("scene")->getFloat("scale");
    if (isPlayerFired) {
        pos.x += (face_right ? offset : -offset);
        pos.y += 0.5f;
    }
    
    std::shared_ptr<JsonValue> projJ = isPlayerFired? constantsRef->get("player_projectile"): constantsRef->get("projectile");
    std::shared_ptr<JsonValue> physicsJ = constantsRef->get("physics");
    std::shared_ptr<JsonValue> fxJ = constantsRef->get("audio")->get("effects");
    
    std::shared_ptr<Texture> image = assetRef->get<Texture>(projJ->getString("texture"));
    float radius = 0.5f * image->getSize().width / scale;

    // Change last parameter to test player-fired or regular projectile
    std::shared_ptr<Projectile> projectile = alloc(pos, radius, isPlayerFired);
    projectile->setName(projJ->getString("name"));
    projectile->setDensity(physicsJ->getFloat("heavy_density"));
    projectile->setBullet(true);
    projectile->setGravityScale(0);
    projectile->setDebugColor(physicsJ->get("debug")->getString("color"));
    projectile->setDrawScale(scale);
    projectile->setSensor(true);
    projectile->setIsPlayerFired(isPlayerFired);

    std::shared_ptr<scene2::PolygonNode> sprite = scene2::PolygonNode::allocWithTexture(image);
    projectile->setSceneNode(sprite);

    sprite->flipHorizontal(direction.x < 0);
    
    float proj_speed = projJ->getFloat("speed");
    // Compute position and velocity
    Vec2 speed = isPlayerFired ? direction.getNormalization()* proj_speed : direction.getNormalization() * proj_speed / 2;
    projectile->setLinearVelocity(speed);
    ObstacleNodePair proj_pair = std::make_pair(projectile, sprite);
    // audio 
    std::shared_ptr<audio::Sound> source = assetRef->get<audio::Sound>(fxJ->getString("pew"));
    audio::AudioEngine::get()->play(fxJ->getString("pew"), source, false, fxJ->getFloat("volume"), true);
    
    return proj_pair;
}

/**
 * Disposes all resources and assets of this Projectile
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a Projectile may not be used until it is initialized again.
 */
void Projectile::dispose() {
	_node = nullptr;
    _debug = nullptr;
}
