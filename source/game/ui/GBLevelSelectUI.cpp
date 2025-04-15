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

    Size screen = Application::get()->getDisplaySize();
    setContentSize(screen);

    auto layer = assets->get<scene2::SceneNode>("levelselectscene");
    if (layer == nullptr) {
        CULog("LevelSelect scene not found in assets!");
        return false;
    }

    layer->setContentSize(screen);
    layer->doLayout();

    float buttonWidth = screen.width / 3.0f;
    float buttonHeight = screen.height;

    auto level1 = layer->getChildByName("level1");
    if (level1) {
        level1->setContentSize(Size(buttonWidth, buttonHeight));
        level1->setPosition(Vec2(buttonWidth / 2.0f, screen.height / 2.0f));
        level1->doLayout();
    }

    auto level2 = layer->getChildByName("level2");
    if (level2) {
        level2->setContentSize(Size(buttonWidth, buttonHeight));
        level2->setPosition(Vec2(screen.width / 2.0f, screen.height / 2.0f));
        level2->doLayout();
    }

    auto level3 = layer->getChildByName("level3");
    if (level3) {
        level3->setContentSize(Size(buttonWidth, buttonHeight));
        level3->setPosition(Vec2(screen.width - buttonWidth / 2.0f, screen.height / 2.0f));
        level3->doLayout();
    }

    addChild(layer);
    setActive(true);

    // Cache the level3 (pause) button for later use.
    _level3Button = std::dynamic_pointer_cast<scene2::Button>(level3);
    if (_level3Button) {
        _level3Button->activate();
        _level3Button->addListener([](const std::string& name, bool down) {
            if (!down) {
                CULog("Button level3 was clicked");
            }
        });
    }
    else {
        CULog("Level3 button not found directly in levelselect scene.");
    }

    _level2Button = std::dynamic_pointer_cast<scene2::Button>(level2);
    if (_level2Button) {
        _level2Button->activate();
        _level2Button->addListener([](const std::string& name, bool down) {
            if (!down) {
                CULog("Button level2 was clicked");
            }
        });
    }
    else {
        CULog("Level2 button not found directly in levelselect scene.");
    }

    _level1Button = std::dynamic_pointer_cast<scene2::Button>(level1);
    if (_level1Button) {
        _level1Button->activate();
        _level1Button->addListener([](const std::string& name, bool down) {
            if (!down) {
                CULog("Button level1 was clicked");
            }
        });
    }
    else {
        CULog("Level1 button not found directly in levelselect scene.");
    }

    // Set the background clear color.
    Application::get()->setClearColor(Color4f::BLACK);
    return true;
}

void GBLevelSelectUI::dispose() {
    _level3Button = nullptr;
    _level2Button = nullptr;
    _level1Button = nullptr;
    _assets = nullptr;
    removeAllChildren();
}

void doButtonSetActive(bool value, const std::shared_ptr<cugl::scene2::Button>& button) {
    if (button) {
        if (value && !button->isActive()) {
            button->activate();
        }
        else if (!value && button->isActive()) {
            button->deactivate();
        }
    }
}

void GBLevelSelectUI::setActive(bool value) {
    _active = value;
    doButtonSetActive(value, _level1Button);
    doButtonSetActive(value, _level2Button);
    doButtonSetActive(value, _level3Button);
    setVisible(value);
}
