#include "GBLevelController.h"

using namespace cugl::graphics;

/**
 * Parses the JSON file and returns a vector of parsed actions.
 */
std::vector<std::shared_ptr<ActionModel>> LevelController::parseActions(const std::shared_ptr<JsonValue>& json, const std::string enemyName) {
    std::vector<std::shared_ptr<ActionModel>> actions;

    //CULog((json->toString()).c_str());

    if (!json || json->children().empty()) {
        CULogError("Invalid or empty JSON node!");
        return actions;
}

    if (json->get(enemyName) == nullptr) {
        CULogError("Enemy not found!");
        return actions;
}

    if (!json->get(enemyName)->has("actions")) {
        CULogError("Invalid JSON format: Missing actions!");
        return actions;
}

    std::vector<std::shared_ptr<JsonValue>> actionArray = json->get(enemyName)->get("actions")->children();
    for (std::shared_ptr<JsonValue> action : actionArray) {

        CULog(action->toString().c_str());

        std::string type = action->getString("type");
        std::string name = action->getString("name");

        std::string animationPath = action->getString("animation");
        std::shared_ptr<Texture> animationTexture = Texture::allocWithFile(animationPath);
        std::shared_ptr<SpriteSheet> animationSprite = SpriteSheet::alloc(animationTexture, action->getInt("animation_row"), action->getInt("animation_col"), 40);
        animationSprite->setFrame(0);

        if (type == "melee") {
            auto meleeAction = std::make_shared<MeleeActionModel>();
            meleeAction->setActionName(name);
            meleeAction->setActionAnimation(animationSprite);

            cugl::Vec2 hitboxPos(action->get("hitboxPos")->asFloatArray().front(), action->get("hitboxPos")->asFloatArray().back());
            cugl::Vec2 hitboxSize(action->get("hitboxSize")->asFloatArray().front(), action->get("hitboxSize")->asFloatArray().back());

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
            rangedAction->setActionAnimation(animationSprite);

            std::shared_ptr<Texture> projectileTexture = Texture::allocWithFile(action->getString("projectileSprite"));
            rangedAction->setProjectileTexture(projectileTexture);
            rangedAction->setProjectileSpeed(action->getFloat("projectileSpeed"));

            cugl::Vec2 projectileDirection(action->get("projectileDirection")->asFloatArray().front(), action->get("projectileDirection")->asFloatArray().back());
            rangedAction->setProjectileDirection(projectileDirection);
            rangedAction->setProjectileDamage(action->getFloat("projectileDamage"));

            actions.push_back(rangedAction);
}
        else if (type == "movement") {
            auto movementAction = std::make_shared<MovementActionModel>();
            movementAction->setActionName(name);
            movementAction->setActionAnimation(animationSprite);

            movementAction->setMoveToPlayer(action->getBool("moveToPlayer"));

            cugl::Vec2 moveDirection(action->get("moveDirection")->asFloatArray().front(), action->get("moveDirection")->asFloatArray().back());
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
