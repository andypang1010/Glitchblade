#include "GBPlayerController.h"
#include "GBInput.h"
#include "GBPlayerModel.h"
using namespace cugl;
using namespace cugl::graphics;

#define DEBUG_COLOR     Color4::YELLOW

// CONSTANTS:
float DUDE_POS[] = { 2.5f, 5.0f };

PlayerController::PlayerController()
{
}

PlayerController::~PlayerController()
{
}

void PlayerController::init(cugl::Rect bounds, const std::shared_ptr<AssetManager>& assetRef, float scale)
{
    std::shared_ptr<Texture> image;
    std::shared_ptr<scene2::PolygonNode> sprite;

    _input.init(bounds);
	Vec2 dudePos = DUDE_POS;
    image = assetRef->get<Texture>(DUDE_TEXTURE);
    _player = PlayerModel::alloc(dudePos, image->getSize() / scale, scale);
    sprite = scene2::PolygonNode::allocWithTexture(image);
    _player->setSceneNode(sprite);
    _player->setDebugColor(DEBUG_COLOR);

	CULog("Inited playercontoller");
}

void PlayerController::dispose() {
    _input.dispose();
    _playerHPNode = nullptr;
}

/**
 * Resets the status of the game so that we can play again.
 *
 * This method disposes of the world and creates a new one.
 */
void PlayerController::reset() {
    _player = nullptr;
}

void PlayerController::fixedUpdate(float timestep)
{
	// CULog("updated playercontroller");
}

void PlayerController::preUpdate(float dt)
{
}

void PlayerController::postUpdate(float dt)
{
}

void PlayerController::activateShield()
{
}

void PlayerController::deactivateShield()
{
}

void PlayerController::fireProjectile()
{
}

void PlayerController::deflectProjectile()
{
}

