//
//  GBMenuUI.cpp
//
//  This module handles all non-gameplay scenes, including homepage, level select, settings, and info page.
//
//  Author: Vince Qian
//
//  Reference:
//  UIButtonScene.cpp
//  UI Demo by Walker White (Version: 1/20/18)
//
#include "GBMenuUI.h"

using namespace cugl;

#pragma mark - Initialization
/**
 * Initializes the non-gameplay UI system with all necessary elements,
 * including homepage, level select, settings, and info page.
 */
bool GBMenuUI::init(const std::shared_ptr<AssetManager>& assets, int scene) {
    if (!SceneNode::init() || assets == nullptr) return false;
    _assets = assets;

    setContentSize(Size(1248, 576));
    setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    setPosition(Vec2::ZERO);

    // Load all menu pages
    _home = assets->get<scene2::SceneNode>("home");
    _levelSelectionHead = assets->get<scene2::SceneNode>("levelselectionhead");
    _levelSelection1 = assets->get<scene2::SceneNode>("levelselection1");
    _levelSelection2 = assets->get<scene2::SceneNode>("levelselection2");
    _levelSelection3 = assets->get<scene2::SceneNode>("levelselection3");
    _levelSelection4 = assets->get<scene2::SceneNode>("levelselection4");
    _homeSetting = assets->get<scene2::SceneNode>("homesetting");
    _info = assets->get<scene2::SceneNode>("info");

    if (!_home || !_homeSetting || !_info || !_levelSelectionHead || !_levelSelection1 || !_levelSelection2 || !_levelSelection3 || !_levelSelection4)
        return false;

    // Add and layout all pages
    for (auto& page : {_home,  _info, _levelSelectionHead, _levelSelection1, _levelSelection2, _levelSelection3, _levelSelection4, _homeSetting}) {
        page->setContentSize(getContentSize());
        page->doLayout();
        page->setVisible(false);
        addChild(page);
    }

    // Setup buttons for each page
    setupHome(_home);
    setupLevelSelectionHead(_levelSelectionHead);
    setupLevelSelection1(_levelSelection1);
    setupLevelSelection2(_levelSelection2);
    setupLevelSelection3(_levelSelection3);
    setupLevelSelection4(_levelSelection4);
    setupHomeSetting(_homeSetting);
    setupInfo(_info);

    // Start with homepage visible
    if (scene == 0) {
        showHome(true);
    } else if (scene == 1) {
        showLevelSelectionHead(true, false, true);
        showLevelSelection1(true);
    } else if (scene == 2) {
        showLevelSelectionHead(true);
        showLevelSelection2(true);
    } else if (scene == 3) {
        showLevelSelectionHead(true);
        showLevelSelection3(true);
    } else if (scene == 4) {
        showLevelSelectionHead(true, true, false);
        showLevelSelection4(true);
    }

    // Handle screen scaling
    float scaleX = Application::get()->getDisplaySize().width / 1248.0f;
    float scaleY = Application::get()->getDisplaySize().height / 576.0f;
    float scale = std::min(scaleX, scaleY);
    this->setScale(scale);
    this->setPosition(Vec2((Application::get()->getDisplaySize().width - 1248 * scale) / 2.0f,
                           (Application::get()->getDisplaySize().height - 576 * scale) / 2.0f));
    return true;
}

void GBMenuUI::dispose() {
    _assets = nullptr;
    
    // Level Selection Head buttons
    _homeButton->clearListeners();
    _levelSettingButton->clearListeners();
    _previousSceneButton->clearListeners();
    _nextSceneButton->clearListeners();
    
    // Level buttons
    _level1Button->clearListeners();
    _level2Button->clearListeners();
    _level3Button->clearListeners();
    _level4Button->clearListeners();
    _level5Button->clearListeners();

    // Home buttons
    _startButton->clearListeners();
    _infoButton->clearListeners();
    _homeSettingButton->clearListeners();
    
    // Setting buttons
    _musicButton->clearListeners();
    _soundButton->clearListeners();
    _backButton->clearListeners();

    // Level Selection Head buttons
    _homeButton = nullptr;
    _levelSettingButton = nullptr;
    _previousSceneButton = nullptr;
    _nextSceneButton = nullptr;
    
    // Level buttons
    _level1Button = nullptr;
    _level2Button = nullptr;
    _level3Button = nullptr;
    _level4Button = nullptr;
    _level5Button = nullptr;

    // Home buttons
    _startButton = nullptr;
    _infoButton = nullptr;
    _homeSettingButton = nullptr;
    
    // Setting buttons
    _musicButton = nullptr;
    _soundButton = nullptr;
    _backButton = nullptr;

    removeAllChildren();
}

void GBMenuUI::setHighestPlayable(int highestPlayableLevel) {
    std::vector<std::shared_ptr< cugl::scene2::Button>> levelButtons = { _level1Button, _level2Button, _level3Button, _level4Button, _level5Button};
    std::vector<std::function<void()>> levelCallbacks = { _level1Callback, _level2Callback, _level3Callback, _level4Callback, _level5Callback };
    
    for (int i = 0; i < highestPlayableLevel && i < levelButtons.size(); i++) {
        if (levelButtons[i]) {
            levelButtons[i]->setColor(Color4(255, 255, 255, 255));
            levelButtons[i]->addListener([this, levelCallbacks, i](const std::string& name, bool down) {
                if (!down && levelCallbacks[i]) {
                    CULog("Pressed level %d button", i);
                    levelCallbacks[i]();
                }
            });
        }
    }

//    for (int i = highestPlayableLevel; i < levelButtons.size(); i++) {
//        levelButtons[i]->setColor(Color4(80, 80, 80, 255));
//        levelButtons[i]->deactivate();
//    }
}

void GBMenuUI::setButtonsActive(std::shared_ptr<scene2::SceneNode> layer, bool active) {
    if (!layer) return;
    for (auto& child : layer->getChildren()) {
        auto button = std::dynamic_pointer_cast<scene2::Button>(child);
        if (button) {
            active ? button->activate() : button->deactivate();
        }
    }
}

void GBMenuUI::showHome(bool visible) {
    if (_home) {
        _home->setVisible(visible);
        setButtonsActive(_home, visible);
    }
}

void GBMenuUI::showLevelSelectionHead(bool visible, std::optional<bool> showPrev, std::optional<bool> showNext) {
    if (_levelSelectionHead) {
        _levelSelectionHead->setVisible(visible);
        setButtonsActive(_levelSelectionHead, visible);

        bool prevVisible = showPrev.value_or(visible);
        bool nextVisible = showNext.value_or(visible);
        
        auto prevScene = std::dynamic_pointer_cast<scene2::Button>(_levelSelectionHead->getChildByName("previousscene"));
        auto nextScene = std::dynamic_pointer_cast<scene2::Button>(_levelSelectionHead->getChildByName("nextscene"));

        if (prevScene) prevScene->setVisible(prevVisible);
        prevVisible ? prevScene->activate() : prevScene->deactivate();
        if (nextScene) nextScene->setVisible(nextVisible);
        nextVisible ? nextScene->activate() : nextScene->deactivate();
    }
}

void GBMenuUI::showLevelSelection1(bool visible) {
    if (_levelSelection1) {
        _levelSelection1->setVisible(visible);
        setButtonsActive(_levelSelection1, visible);
    }
}

void GBMenuUI::showLevelSelection2(bool visible) {
    if (_levelSelection2) {
        _levelSelection2->setVisible(visible);
        setButtonsActive(_levelSelection2, visible);
    }
}

void GBMenuUI::showLevelSelection3(bool visible) {
    if (_levelSelection3) {
        _levelSelection3->setVisible(visible);
        setButtonsActive(_levelSelection3, visible);
    }
}

void GBMenuUI::showLevelSelection4(bool visible) {
    if (_levelSelection4) {
        _levelSelection4->setVisible(visible);
        setButtonsActive(_levelSelection4, visible);
    }
}

void GBMenuUI::showHomeSetting(bool visible) {
    if (_homeSetting) {
        _homeSetting->setVisible(visible);
        setButtonsActive(_homeSetting, visible);
    }
}

void GBMenuUI::showInfo(bool visible) {
    if (_info) {
        _info->setVisible(visible);
        setButtonsActive(_info, visible);
    }
}

void GBMenuUI::setupHome(std::shared_ptr<scene2::SceneNode>& home) {
    _startButton = std::dynamic_pointer_cast<scene2::Button>(home->getChildByName("start"));
    _infoButton = std::dynamic_pointer_cast<scene2::Button>(home->getChildByName("info"));
    _homeSettingButton = std::dynamic_pointer_cast<scene2::Button>(home->getChildByName("setting"));

    if (_startButton) {
        _startButton->addListener([this](const std::string& name, bool down) {
            if (!down && _startCallback) _startCallback();
        });
    }

    if (_infoButton) {
        _infoButton->addListener([this](const std::string& name, bool down) {
            if (!down && _infoCallback) _infoCallback();
        });
    }

    if (_homeSettingButton) {
        _homeSettingButton->addListener([this](const std::string& name, bool down) {
            if (!down && _homeSettingCallback) _homeSettingCallback();
        });
    }
}

void GBMenuUI::setupLevelSelectionHead(std::shared_ptr<scene2::SceneNode>& head) {
    _homeButton = std::dynamic_pointer_cast<scene2::Button>(head->getChildByName("lshome"));
    _levelSettingButton = std::dynamic_pointer_cast<scene2::Button>(head->getChildByName("lssetting"));
    _previousSceneButton = std::dynamic_pointer_cast<scene2::Button>(head->getChildByName("previousscene"));
    _nextSceneButton = std::dynamic_pointer_cast<scene2::Button>(head->getChildByName("nextscene"));

    if (_homeButton) {
        _homeButton->addListener([this](const std::string& name, bool down) {
            if (!down && _homeCallback) _homeCallback();
        });
    }

    if (_levelSettingButton) {
        _levelSettingButton->addListener([this](const std::string& name, bool down) {
            if (!down && _homeSettingCallback) _homeSettingCallback();
        });
    }
    
    if (_previousSceneButton) {
        _previousSceneButton->addListener([this](const std::string& name, bool down) {
            if (!down && _previousSceneCallback) _previousSceneCallback();
        });
    }
    
    if (_nextSceneButton) {
        _nextSceneButton->addListener([this](const std::string& name, bool down) {
            if (!down && _nextSceneCallback) _nextSceneCallback();
        });
    }
}

void GBMenuUI::setupLevelSelection1(std::shared_ptr<scene2::SceneNode>& page) {
    _level1Button = std::dynamic_pointer_cast<scene2::Button>(page->getChildByName("selector1_1_1"));
    _level2Button = std::dynamic_pointer_cast<scene2::Button>(page->getChildByName("selector1_2_1"));
}

void GBMenuUI::setupLevelSelection2(std::shared_ptr<scene2::SceneNode>& page) {
    _level3Button = std::dynamic_pointer_cast<scene2::Button>(page->getChildByName("selector2_1_1"));
}

void GBMenuUI::setupLevelSelection3(std::shared_ptr<scene2::SceneNode>& page) {
    _level4Button = std::dynamic_pointer_cast<scene2::Button>(page->getChildByName("selector3_1_1"));
}

void GBMenuUI::setupLevelSelection4(std::shared_ptr<scene2::SceneNode>& page) {
    _level5Button = std::dynamic_pointer_cast<scene2::Button>(page->getChildByName("selector4_1_1"));
}

void GBMenuUI::setupHomeSetting(std::shared_ptr<cugl::scene2::SceneNode>& homeSetting)
{
    _musicButton = std::dynamic_pointer_cast<scene2::Button>(homeSetting->getChildByName("music"));
    _soundButton = std::dynamic_pointer_cast<scene2::Button>(homeSetting->getChildByName("sound"));
    _backButton = std::dynamic_pointer_cast<scene2::Button>(homeSetting->getChildByName("back"));

    if (_musicButton) {
        _musicButton->addListener([this](const std::string& name, bool down) {
            if (!down && _musicCallback) {
                CULog("Music pressed");
                _musicCallback();
            }
        });
    }

    if (_soundButton) {
        _soundButton->addListener([this](const std::string& name, bool down) {
            if (!down && _soundCallback) {
                CULog("Sound pressed");
                _soundCallback();
            }
        });
    }

    if (_backButton) {
        _backButton->addListener([this](const std::string& name, bool down) {
            if (!down && _backCallback) {
                CULog("Back pressed");
                _backCallback();
            }
        });
    }
}

void GBMenuUI::setupInfo(std::shared_ptr<scene2::SceneNode>& info) {
    _infoHomeButton = std::dynamic_pointer_cast<scene2::Button>(info->getChildByName("lshome"));
    if (_infoHomeButton) {
        _infoHomeButton->addListener([this](const std::string& name, bool down) {
            if (!down && _homeCallback) _homeCallback();
        });
    }
}
