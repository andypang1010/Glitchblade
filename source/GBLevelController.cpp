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

	CULog("LevelController::init");
}

void LevelController::update(float timestep)
{
	_enemyController->update(timestep);
	_playerController->update(timestep);

	CULog("LevelController::update");
}

void LevelController::start()
{
	_enemyController->start();
	_playerController->start();

	CULog("LevelController::start");
}
