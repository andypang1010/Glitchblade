#include "GBLevelController.h"

using namespace cugl::graphics;

/**
 * Parses the JSON file and returns a vector of parsed actions.
 */
std::vector<std::shared_ptr<ActionModel>> LevelController::parseActions(const std::shared_ptr<JsonValue>& json, const std::string enemyName) {
    std::vector<std::shared_ptr<ActionModel>> actions;

    if (!json || json->children().empty()) {
        CULogError("Invalid or empty JSON node!");
        return actions;
    }

    if (!json || !json->has("name") || !json->has("actions")) {
        CULogError("Invalid JSON format: Missing required fields!");
        return actions;
    }

    if (json->getString("name") != enemyName) {
        CULogError("Enemy not found!");
        return actions;
    }

    auto actionArray = json->get(enemyName + ".actions")->children();
    for (auto& action : actionArray) {
        std::string type = action->getString("type");
        std::string name = action->getString("name");


        std::string animationPath = action->getString("animation");
        std::shared_ptr<Texture> animationTexture = Texture::allocWithFile(animationPath);
        std::shared_ptr<SpriteSheet> animationSpritesheet;
        animationSpritesheet->init(animationTexture, action->getInt("animation_row"), action->getInt("animation_col"));

        if (type == "melee") {
            auto meleeAction = std::make_shared<MeleeActionModel>();
            meleeAction->setActionName(name);
            meleeAction->setActionAnimation(animationSpritesheet);

            cugl::Vec2 hitboxPos(action->get("hitboxPos")->asFloat(0), action->get("hitboxPos")->asFloat(1));
            cugl::Vec2 hitboxSize(action->get("hitboxSize")->asFloat(0), action->get("hitboxSize")->asFloat(1));

            meleeAction->setHitboxPos(hitboxPos);
            meleeAction->setHitboxSize(hitboxSize);
            meleeAction->setHitboxStartTime(action->getFloat("hitboxStartTime"));
            meleeAction->setHitboxEndTime(action->getFloat("hitboxEndTime"));
            meleeAction->setHitboxDamage(action->getFloat("hitboxDamage"));

            actions.push_back(meleeAction);
        }
        else if (type == "ranged") {
            auto rangedAction = std::make_shared<RangedActionModel>();
            rangedAction->setActionName(name);
            rangedAction->setActionAnimation(animationSpritesheet);

            std::shared_ptr<Texture> projectileTexture = Texture::allocWithFile(action->getString("projectileSprite"));
            rangedAction->setProjectileTexture(projectileTexture);
            rangedAction->setProjectileSpeed(action->getFloat("projectileSpeed"));

            cugl::Vec2 projectileDirection(action->get("projectileDirection")->asFloat(0), action->get("projectileDirection")->asFloat(1));
            rangedAction->setProjectileDirection(projectileDirection);
            rangedAction->setProjectileDamage(action->getFloat("projectileDamage"));

            actions.push_back(rangedAction);
        }
        else if (type == "movement") {
            auto movementAction = std::make_shared<MovementActionModel>();
            movementAction->setActionName(name);
            movementAction->setActionAnimation(animationSpritesheet);

            movementAction->setMoveToPlayer(action->getBool("moveToPlayer"));

            cugl::Vec2 moveDirection(action->get("moveDirection")->asFloat(0), action->get("moveDirection")->asFloat(1));
            movementAction->setMoveDirection(moveDirection);
            movementAction->setMoveDistance(action->getFloat("moveDistance"));

            actions.push_back(movementAction);
        }
        else {
            std::cerr << "Unknown action type: " << type << std::endl;
        }
    }

    return actions;
}
