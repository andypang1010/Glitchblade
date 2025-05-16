#include "GBEnemyController.h"

using namespace cugl;

void EnemyController::init(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& enemyJSON,
    std::vector<std::shared_ptr<ActionModel>> actions) {
    std::string enemy_debug_font = "debug";

    _hpNode = scene2::Label::allocWithText("1000", assetRef->get<Font>(enemy_debug_font));
    _hpNode->setAnchor(Vec2::ANCHOR_CENTER);
    _hpNode->setForeground(Color4::RED);
    _hpNode->setPosition(0, 115);

    _stunNode = scene2::Label::allocWithText("STUN", assetRef->get<Font>(enemy_debug_font));
    _stunNode->setAnchor(Vec2::ANCHOR_CENTER);
    _stunNode->setForeground(Color4::RED);
    _stunNode->setPosition(0, 135);

    _enemy->getSceneNode()->addChild(_hpNode);
    _enemy->getSceneNode()->addChild(_stunNode);

	_hpNode->setVisible(false);
    _stunNode->setVisible(false);
}

void EnemyController::reset() {
    _enemy->resetAttributes();
	_enemy->setPosition(ENEMY_DEFAULT_INIT_POS);
}

void EnemyController::dispose() {
    if (_enemy) {
        _enemy->dispose();
        inWorld = false;
        _enemy = nullptr;
    }
    _enemyJSON = nullptr;
    _hpNode = nullptr;
    _stunNode = nullptr;
}

/**Based on the @param player's position, use the enemy's spawn ring to set the enemy spawn position**/
void EnemyController::spawnAt(Vec2 spawnPosition) {
    if (inWorld){
        CULog("Cannot set spawn position for enemy that is already in the physics world");
        return;
    }

	spawnPosition /= 32;

	if (spawnPosition.x > _enemy->worldLeft && spawnPosition.x < _enemy->worldRight) {
        _enemy->setPosition(spawnPosition);
    }
    else {
        CULog("bad spawn position generated in EnemyController::setSpawnPosition, defaulting to safe position.");
        _enemy->setPosition(ENEMY_DEFAULT_INIT_POS);
    }
}