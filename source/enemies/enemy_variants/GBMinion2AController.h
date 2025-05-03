#pragma once

#include "../GBEnemyController.h"
#include "GBMinion2AModel.h"

class Minion2AController : public EnemyController {
public:
    Minion2AController();

    // Implement required virtuals from EnemyController
    void init(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& constantsRef, std::vector<std::shared_ptr<ActionModel>> actions) override;
    void applyForce() override;
    void fixedUpdate(float timestep) override;
    void preUpdate(float dt) override;
    void postUpdate(float dt) override;
};
