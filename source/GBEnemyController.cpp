#include "GBEnemyController.h"
#include "GBEnemyModel.h"
using namespace cugl;
using namespace cugl::graphics;

#define SIGNUM(x)  ((x > 0) - (x < 0))

//CONSTANTS (INITIAL POSITIONS SHOULD BE LATER DETERMINED BY LEVEL MODEL/CONTROLLER
float ENEMY_POS[] = { 12.5f, 5.0f };
void EnemyController::init(const std::shared_ptr<AssetManager>& assetRef, const cugl::Rect bounds,  float scale, std::vector<std::shared_ptr<ActionModel>> actions)
{
	_enemy  = EnemyModel::alloc(assetRef, ENEMY_POS, scale, actions);
    
    #pragma mark hp node
    _hpNode = scene2::Label::allocWithText("100", assetRef->get<Font>(DEBUG_FONT));
    _hpNode->setAnchor(Vec2::ANCHOR_CENTER);
    _hpNode->setForeground(Color4::CYAN);
    _hpNode->setPosition(0, 55);
    _enemy->getSceneNode()->addChild(_hpNode);
}

void EnemyController::reset()
{
}

void EnemyController::fixedUpdate(float timestep)
{
	// CULog("Updated enemycontroller");
}

void EnemyController::preUpdate(float dt)
{
}

void EnemyController::postUpdate(float dt)
{
}

