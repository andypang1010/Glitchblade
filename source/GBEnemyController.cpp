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

void EnemyController::start()
{
	CULog("Started enemycontroller");
}

void EnemyController::update(float timestep)
{
	// CULog("Updated enemycontroller");
}

void EnemyController::removeEnemy(const std::shared_ptr<EnemyModel>& enemy)
{
}
