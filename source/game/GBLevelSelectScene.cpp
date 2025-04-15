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
        _levelSelected = "";
        _levelSelectUI->dispose();
        Scene2::dispose();
    }
}

void GBLevelSelectScene::setActive(bool active) {
    _active = active;
    if(_levelSelectUI != nullptr)
        _levelSelectUI->setActive(active);
}

void GBLevelSelectScene::preUpdate(float dt) {
    std::string keySelected = _levelSelectUI->getLoadSceneKey();
    if (keySelected != "") {
        _levelSelected = keySelected;
        setSwapSignal(2);
    }
}

std::string GBLevelSelectScene::getLevelSelected() {
    return _levelSelected;
}

void GBLevelSelectScene::setLevelSelected(std::string lvlString) {
    if (_levelSelectUI != nullptr)
        _levelSelectUI->setLoadSceneKey(lvlString);
    _levelSelected = lvlString;
}

void GBLevelSelectScene::postUpdate(float remain) {}
void GBLevelSelectScene::render() {
    Scene2::render();
}
void GBLevelSelectScene::fixedUpdate(float step) {}