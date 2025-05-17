#pragma once

#include "../GBEnemyController.h"
#include "GBMinion2BModel.h"

class Minion2BController : public EnemyController {
public:
    Minion2BController();

    // Implement required virtuals from EnemyController
    void init(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& enemyJSON, std::vector<std::shared_ptr<ActionModel>> actions) override;
    void applyForce() override;
    void fixedUpdate(float timestep) override;
    void preUpdate(float dt) override;
    void postUpdate(float dt) override;
};
