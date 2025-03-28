#include "GBEnemyController.h"

using namespace cugl;

void EnemyController::init(const std::shared_ptr<AssetManager>& assetRef,
    const std::shared_ptr<JsonValue>& constantsRef,
    std::vector<std::shared_ptr<ActionModel>> actions) {
    std::string enemy_debug_font = _enemyJSON->get("debug")->getString("font");

    _hpNode = scene2::Label::allocWithText("100", assetRef->get<Font>(enemy_debug_font));
    _hpNode->setAnchor(Vec2::ANCHOR_CENTER);
    _hpNode->setForeground(Color4::RED);
    _hpNode->setPosition(0, 115);

    _stunNode = scene2::Label::allocWithText("STUN", assetRef->get<Font>(enemy_debug_font));
    _stunNode->setAnchor(Vec2::ANCHOR_CENTER);
    _stunNode->setForeground(Color4::RED);
    _stunNode->setPosition(0, 135);

    _enemy->getSceneNode()->addChild(_hpNode);
    _enemy->getSceneNode()->addChild(_stunNode);
}

void EnemyController::reset() {
    _enemy->resetAttributes();
    _enemy->setPosition(ENEMY_INIT_POS);
}

void EnemyController::dispose() {
    if (_enemy) {
        _enemy->dispose();
        _enemy = nullptr;
    }
    _enemyJSON = nullptr;
    _hpNode = nullptr;
    _stunNode = nullptr;
}
