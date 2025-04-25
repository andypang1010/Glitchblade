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
    _level1Button = std::dynamic_pointer_cast<cugl::scene2::Button>(menu->getChildByName("level1"));
    _level2Button = std::dynamic_pointer_cast<cugl::scene2::Button>(menu->getChildByName("level2"));
    _level3Button = std::dynamic_pointer_cast<cugl::scene2::Button>(menu->getChildByName("level3"));

    // Add listeners TODO: probably make this a loop in the future

    if (_level1Button) {
        _level1Button->addListener([this](const std::string& name, bool down) {
            if (down) {
                CULog("Pressed level 1 button");
                _level1Callback();
            }
        });
    }

    if (_level2Button) {
        _level2Button->addListener([this](const std::string& name, bool down) {
            if (down) {
                CULog("Pressed level 2 button");
                _level2Callback();
            }
        });
    }

    if (_level3Button) {
        _level3Button->addListener([this](const std::string& name, bool down) {
            if (down) {
                CULog("Pressed level 3 button");
                _level3Callback();
            }
        });
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
