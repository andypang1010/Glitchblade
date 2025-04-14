#include "GBLevelSelectScene.h"

using namespace cugl;
using namespace cugl::graphics;
using namespace cugl::physics2;

bool GBLevelSelectScene::init(const std::shared_ptr<AssetManager>& assets) {
    if (!Scene2::init()) return false;

    _assets = assets;

    _levelSelectUI = GBLevelSelectUI::alloc(assets);
    if (_levelSelectUI != nullptr) {
        // Set the level select UI size to the display size.
        Size screen = Application::get()->getDisplaySize();
        _levelSelectUI->setContentSize(screen);
        _levelSelectUI->doLayout();
        addChild(_levelSelectUI);
        _levelSelectUI->setActive(true);
    }

    return true;
}

void GBLevelSelectScene::dispose() {
    if (_active) {
        Scene2::dispose();
    }
}

void GBLevelSelectScene::preUpdate(float dt) {}
void GBLevelSelectScene::postUpdate(float remain) {}
void GBLevelSelectScene::render() {
    Scene2::render();
}
void GBLevelSelectScene::fixedUpdate(float step) {}
