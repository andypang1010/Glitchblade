#pragma once

#include "GBEnemyModel.h"
#include "cugl/cugl.h"

using namespace cugl;
using namespace cugl::graphics;

/**
* Abstract class that controls the logic of an enemy type.
*
* Subclasses like Boss1Controller should implement the enemy-specific behavior.
*/
class EnemyController {
protected:
    std::shared_ptr<EnemyModel> _enemy;
    std::shared_ptr<JsonValue> _enemyJSON;
    std::shared_ptr<scene2::Label> _hpNode;
    std::shared_ptr<scene2::Label> _stunNode;
    const float ENEMY_INIT_POS[2] = { 36.0f, 5.0f };

public:
    EnemyController() {}
    virtual ~EnemyController() { dispose(); }

    virtual void init(const std::shared_ptr<AssetManager>& assetRef,
        const std::shared_ptr<JsonValue>& constantsRef,
        std::vector<std::shared_ptr<ActionModel>> actions);

    virtual void reset();
    virtual void dispose();

    virtual void applyForce() = 0;
    virtual void fixedUpdate(float timestep) = 0;
    virtual void preUpdate(float dt) = 0;
    virtual void postUpdate(float dt) = 0;

    std::shared_ptr<EnemyModel> getEnemy() const { return _enemy; }
};