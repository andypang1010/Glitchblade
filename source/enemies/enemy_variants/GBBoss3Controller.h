#pragma once

#include "../GBEnemyController.h"
#include "GBBoss3Model.h"

class Boss3Controller : public EnemyController {
public:
    Boss3Controller();

    // Implement required virtuals from EnemyController
    void init(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& enemiesJSON, std::vector<std::shared_ptr<ActionModel>> actions) override;
    void applyForce() override;
    void fixedUpdate(float timestep) override;
    void preUpdate(float dt) override;
    void postUpdate(float dt) override;
};
