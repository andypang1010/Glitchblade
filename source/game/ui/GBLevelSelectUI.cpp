#include "GBLevelSelectUI.h"

using namespace cugl;

void LevelSelectUI::setButtonsActive(std::shared_ptr<cugl::scene2::SceneNode> layer, bool active)
{
    if (!layer) return;

    for (auto& child : layer->getChildren()) {
        auto button = std::dynamic_pointer_cast<cugl::scene2::Button>(child);
        if (button) {
            active ? button->activate() : button->deactivate();
        }
    }
}

void LevelSelectUI::dispose() {
    _assets = nullptr;

    _level1Button->clearListeners();
    _level2Button->clearListeners();
    _level3Button->clearListeners();

    _level1Button = nullptr;
    _level2Button = nullptr;
    _level3Button = nullptr;

    removeAllChildren();
}

bool LevelSelectUI::init(const std::shared_ptr<cugl::AssetManager>& assets)
{
    if (assets == nullptr) {
        return false;
    }
    else if (!SceneNode::init()) {
        return false;
    }

    _assets = assets;

    setContentSize(Size(1248, 576));
    auto menu = assets->get<scene2::SceneNode>("levelselectmenu");

    if (menu == nullptr) {
        return false;
    }

    menu->setContentSize(Size(1248, 576));
    menu->doLayout();
    menu->setVisible(false);
    addChild(menu);

    setupMenu(menu);
    showMenu(true);
    _screenOffset = getPosition();

    Application::get()->setClearColor(Color4f::BLACK);
    return true;
}

void LevelSelectUI::setupMenu(std::shared_ptr<cugl::scene2::SceneNode>& menu)
{
    _level1Button = std::dynamic_pointer_cast<cugl::scene2::Button>(menu->getChildByName("selector1_1_1"));
    _level2Button = std::dynamic_pointer_cast<cugl::scene2::Button>(menu->getChildByName("selector1_2_1"));
    _level3Button = std::dynamic_pointer_cast<cugl::scene2::Button>(menu->getChildByName("selector1_3_1"));

    // Old way of setting up buttons
    //if (_level1Button) {
    //    _level1Button->addListener([this](const std::string& name, bool down) {
    //        if (down && _level1Callback) {
    //            CULog("Pressed level 1 button");
    //            _level1Callback();
    //        }
    //    });
    //}

    //if (_level2Button) {
    //    _level2Button->addListener([this](const std::string& name, bool down) {
    //        if (down && _level2Callback) {
    //            CULog("Pressed level 2 button");
    //            _level2Callback();
    //        }
    //    });
    //}

    //if (_level3Button) {
    //    _level3Button->addListener([this](const std::string& name, bool down) {
    //        if (down && _level3Callback) {
    //            CULog("Pressed level 3 button");
    //            _level3Callback();
    //        }
    //    });
    //}
}

void LevelSelectUI::setHighestPlayable(int highestPlayableLevel) {
    std::vector<std::shared_ptr< cugl::scene2::Button>> levelButtons = { _level1Button, _level2Button, _level3Button };
    std::vector<std::function<void()>> levelCallbacks = { _level1Callback, _level2Callback, _level3Callback };
    
    for (int i = 0; i < highestPlayableLevel && i < levelButtons.size(); i++) {
        if (levelButtons[i]) {
            levelButtons[i]->setColor(Color4(255, 255, 255, 255));
            levelButtons[i]->addListener([this, levelCallbacks, i](const std::string& name, bool down) {
                if (down && levelCallbacks[i]) {
                    CULog("Pressed level %d button", i);
                    levelCallbacks[i]();
                }
            });
        }
    }

    for (int i = highestPlayableLevel; i < levelButtons.size(); i++) {
        levelButtons[i]->setColor(Color4(80, 80, 80, 255));
        levelButtons[i]->deactivate();
    }
}

void LevelSelectUI::showMenu(bool visible)
{
    auto layer = getChildByName("levelselectmenu");
    if (layer) {
        layer->setVisible(visible);
        setButtonsActive(layer, visible);
    }
}
