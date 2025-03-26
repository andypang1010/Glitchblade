#pragma once

#include "GBEnemyController.h"

class Boss1Controller : public EnemyController {
public:
    Boss1Controller();

    // Implement required virtuals from EnemyController
    void applyForce() override;
    void fixedUpdate(float timestep) override;
    void preUpdate(float dt) override;
    void postUpdate(float dt) override;
};
