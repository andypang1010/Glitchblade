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
    _spawnPosition = ENEMY_DEFAULT_INIT_POS; // default
    _enemy->resetAttributes();
    _enemy->setPosition(_spawnPosition);
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
void EnemyController::setSpawnPosition(Vec2 player_pos){
    if (inWorld){
        CULog("Cannot set spawn position for enemy that is already in the physics world");
        return;
    }
    CULog("setting enemy spawn, Player pos is (%f, %f)", player_pos.x, player_pos.y);
    Vec2 spawn_pos = player_pos;
    float distance = rand() % 2? SPAWN_RING.x : SPAWN_RING.y;//x is closer, y is farther
    // place enemy on the side of the player with mroe space
    // approximate middle of world with _enemy->worldRight / 2
    int direction = player_pos.x < _enemy->worldRight / 2? 1: -1;
    spawn_pos.x += direction * distance;
    spawn_pos.y = 5.0;
    if (spawn_pos.x > _enemy->worldLeft && spawn_pos.x < _enemy->worldRight){
        CULog("Setting enemy spawn to (%f, %f)", spawn_pos.x, spawn_pos.y);
        _enemy->setPosition(spawn_pos);
        }
    else {
        CULog("bad spawn position generated in EnemyController::setSpawnPosition, defaulting to safe position.");
        _enemy->setPosition(ENEMY_DEFAULT_INIT_POS);
    }
 
   
}
