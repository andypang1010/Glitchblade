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
	CULog("LevelController::update");
}

void LevelController::start()
{
	CULog("LevelController::start");
}
