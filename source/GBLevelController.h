#ifndef LEVEL_CONTROLLER_H
#define LEVEL_CONTROLLER_H

#include <string>
#include <vector>
#include <cugl/cugl.h>
#include <iostream>

#include "GBActionModel.h"
#include "GBMeleeActionModel.h"
#include "GBRangedActionModel.h"
#include "GBMovementActionModel.h"

/**
 * A class that parses a JSON level file and generates corresponding action models.
 */
class LevelController {
public:
    /** Parses the JSON file and returns a vector of parsed actions. */
    static std::vector<std::shared_ptr<ActionModel>> parseActions(const std::shared_ptr<JsonValue>& json, const std::string enemyName);
};

#endif // LEVEL_CONTROLLER_H