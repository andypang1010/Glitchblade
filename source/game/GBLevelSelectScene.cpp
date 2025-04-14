#include "GBLevelSelectScene.h"

using namespace cugl;
using namespace cugl::graphics;

bool LevelSelectScene::init(const std::shared_ptr<cugl::AssetManager>& assets)
{
    return false;
}

void LevelSelectScene::dispose()
{
    if (_active) {
        Scene2::dispose();
    }
}

void LevelSelectScene::preUpdate(float dt)
{
}

void LevelSelectScene::postUpdate(float remain)
{
}

void LevelSelectScene::fixedUpdate(float step)
{
}
