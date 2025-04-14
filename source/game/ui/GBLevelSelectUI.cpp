#include "GBLevelSelectUI.h"
using namespace cugl;

bool GBLevelSelectUI::init(const std::shared_ptr<AssetManager>& assets) {
    if (assets == nullptr) {
        return false;
    }
    if (!SceneNode::init()) {
        return false;
    }

    _assets = assets;

    // Use the full display size.
    Size screen = Application::get()->getDisplaySize();
    setContentSize(screen);

    // Retrieve your level select scene node.
    auto layer = assets->get<scene2::SceneNode>("levelselectscene");
    if (layer == nullptr) {
        CULog("LevelSelect scene not found in assets!");
        return false;
    }
    layer->setContentSize(screen);
    layer->doLayout();
    addChild(layer);

    // Mark the UI as active.
    setActive(true);

    // Attempt to get the pause button directly.
    _pauseButton = std::dynamic_pointer_cast<scene2::Button>(layer->getChildByName("pause"));
    if (_pauseButton) {
        // Attach a listener on the pause button that toggles the pause menu.
        if (_active) {
            _pauseButton->activate();
        }
    }
    else {
        CULog("Pause button not found directly in levelselect scene.");
    }

    // Set a background clear color.
    Application::get()->setClearColor(Color4f::CORNFLOWER);
    return true;
}

void GBLevelSelectUI::dispose() {
    _pauseButton = nullptr;
    _assets = nullptr;
    removeAllChildren();
}

void GBLevelSelectUI::setActive(bool value) {
    _active = value;
    if (_pauseButton) {
        if (value && !_pauseButton->isActive()) {
            _pauseButton->activate();
        }
        else if (!value && _pauseButton->isActive()) {
            _pauseButton->deactivate();
        }
    }
    setVisible(value);
}
