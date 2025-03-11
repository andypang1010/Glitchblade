#include "GBEnemyController.h"

EnemyController::EnemyController()
{
}

EnemyController::~EnemyController()
{
}

void EnemyController::init()
{
	CULog("Inited enemycontroller");
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

void EnemyController::removeEnemy(const std::shared_ptr<EnemyModel>& enemy)
{
}
