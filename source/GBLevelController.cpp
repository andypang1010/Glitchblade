#include "GBLevelController.h"

void LevelController::checkWinCondition()
{
}

void LevelController::checkLoseCondition()
{
}

LevelController::LevelController()
{
}

LevelController::~LevelController()
{
}

void LevelController::init()
{
	// Setup enemy controller
	_enemyController = std::make_shared<EnemyController>();

	// Setup player controller
	_playerController = std::make_shared<PlayerController>();

	_enemyController->init();
	_playerController->init();

	CULog("LevelController::init");
}

void LevelController::preUpdate(float dt)
{
	_enemyController->preUpdate(dt);
	_playerController->preUpdate(dt);
}

void LevelController::postUpdate(float dt)
{
	_enemyController->postUpdate(dt);
	_playerController->postUpdate(dt);
}

void LevelController::fixedUpdate(float timestep)
{
	_enemyController->fixedUpdate(timestep);
	_playerController->fixedUpdate(timestep);

	CULog("LevelController::update");
}
