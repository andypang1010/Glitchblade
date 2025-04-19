//
//  GBIngameUI.cpp
//
//  This module handles all in-game UI, including HUD, pause menu, settings, and win/lose page.
//
//  Author: Vince Qian
//
//  Reference:
//  UIButtonScene.cpp
//  UI Demo by Walker White (Version: 1/20/18)
//
#include "GBIngameUI.h"

using namespace cugl;

/**
 * Initializes the controller contents, and starts the game
 *
 * The constructor does not allocate any objects or memory.  This allows
 * us to have a non-pointer reference to this controller, reducing our
 * memory allocation.  Instead, allocation happens in this method.
 *
 * @param assets    The (loaded) assets for this game mode
 *
 * @return true if the controller is initialized properly, false otherwise.
 */
bool GBIngameUI::init(const std::shared_ptr<AssetManager>& assets) {
    if (assets == nullptr) {
        return false;
    } else if (!SceneNode::init()) {
        return false;
    }

    _assets = assets;

    setContentSize(Size(1248, 576));

    auto headsUpDisplay = assets->get<scene2::SceneNode>("hud");
    auto pauseMenu = assets->get<scene2::SceneNode>("pausemenu");
    auto losePage = assets->get<scene2::SceneNode>("lose");
    auto winPage1 = assets->get<scene2::SceneNode>("win1");
    auto winPage2 = assets->get<scene2::SceneNode>("win2");

    if (headsUpDisplay == nullptr || pauseMenu == nullptr|| losePage == nullptr || winPage1 == nullptr || winPage2 == nullptr) {
        return false;
    }

    headsUpDisplay->setContentSize(Size(1248, 576));
    headsUpDisplay->doLayout();
    addChild(headsUpDisplay);

    pauseMenu->setContentSize(Size(1248, 576));
    pauseMenu->doLayout();
    pauseMenu->setVisible(false);
    addChild(pauseMenu);
    
    losePage->setContentSize(Size(1248, 576));
    losePage->doLayout();
    losePage->setVisible(false);
    addChild(losePage);
    
    winPage1->setContentSize(Size(1248, 576));
    winPage1->doLayout();
    winPage1->setVisible(false);
    addChild(winPage1);
    
    winPage2->setContentSize(Size(1248, 576));
    winPage2->doLayout();
    winPage2->setVisible(false);
    addChild(winPage2);

//    setActive(true);

    setupHUD(headsUpDisplay);
    setupPause(pauseMenu);
    setupLose(losePage);
    
    _screenOffset = getPosition();

    Application::get()->setClearColor(Color4f::CORNFLOWER);
    return true;
}

void GBIngameUI::setupHUD(std::shared_ptr<cugl::scene2::SceneNode>& headsUpDisplay)
{
    _pauseButton = std::dynamic_pointer_cast<scene2::Button>(headsUpDisplay->getChildByName("pause"));

    if (_pauseButton) {
        _pauseButton->addListener([this](const std::string& name, bool down) {
            if (down && _pauseCallback) {
                CULog("Pause!");
                _pauseCallback();
            }
        });
    }

    for (int i = 1; i <= 5; ++i) {
        std::string fullName = "hp_full_segment_" + std::to_string(i);
        std::string halfName = "hp_half_segment_" + std::to_string(i);

        auto full = headsUpDisplay->getChildByName<scene2::PolygonNode>(fullName);
        auto half = headsUpDisplay->getChildByName<scene2::PolygonNode>(halfName);

        if (full != nullptr) {
            _hpSegments.push_back(full);
        }
        if (half != nullptr) {
            _hpHalfSegments.push_back(half);
        }
    }
}

void GBIngameUI::setupPause(std::shared_ptr<cugl::scene2::SceneNode>& pauseMenu)
{
    _resumeButton = std::dynamic_pointer_cast<scene2::Button>(pauseMenu->getChildByName("resume"));
    _retryButton = std::dynamic_pointer_cast<scene2::Button>(pauseMenu->getChildByName("retry"));
    _quitButton = std::dynamic_pointer_cast<scene2::Button>(pauseMenu->getChildByName("quit"));
    _settingButton = std::dynamic_pointer_cast<scene2::Button>(pauseMenu->getChildByName("setting"));

    if (_resumeButton) {
        _resumeButton->addListener([this](const std::string& name, bool down) {
            if (down && _resumeCallback) {
                CULog("Resume pressed");
                _resumeCallback();
            }
        });
    }

    if (_retryButton) {
        _retryButton->addListener([this](const std::string& name, bool down) {
            if (down && _retryCallback) {
                CULog("Retry pressed");
                _retryCallback();
            }
        });
    }

    if (_quitButton) {
        _quitButton->addListener([](const std::string& name, bool down) {
            if (down) CULog("Quit pressed");
        });
    }

    if (_settingButton) {
        _settingButton->addListener([](const std::string& name, bool down) {
            if (down) CULog("Setting pressed");
        });
    }
}

void GBIngameUI::setupLose(std::shared_ptr<cugl::scene2::SceneNode>& losePage)
{
    _loseRetryButton = std::dynamic_pointer_cast<scene2::Button>(losePage->getChildByName("lose_retry"));
    _loseQuitButton = std::dynamic_pointer_cast<scene2::Button>(losePage->getChildByName("lose_quit"));

    if (_loseRetryButton) {
        _loseRetryButton->addListener([this](const std::string& name, bool down) {
            if (down && _retryCallback) {
                CULog("Lose Retry pressed");
                _retryCallback();
            }
        });
    }

    if (_loseQuitButton) {
        _loseQuitButton->addListener([](const std::string& name, bool down) {
            if (down) CULog("Lose Quit pressed");
        });
    }
    
    // Win Page 1 UI
    _continueButton = std::dynamic_pointer_cast<scene2::Button>(winPage1->getChildByName("continue"));
    if (_continueButton) {
        _continueButton->addListener([this](const std::string& name, bool down) {
            if (down && _continueCallback) {
                CULog("Continue pressed");
                _continueCallback();
            }
        });
    }
    
    // Win Page 2 UI
    _winContinueButton = std::dynamic_pointer_cast<scene2::Button>(winPage2->getChildByName("win_continue"));
    _winRetryButton = std::dynamic_pointer_cast<scene2::Button>(winPage2->getChildByName("win_retry"));
    if (_winContinueButton) {
        _winContinueButton->addListener([this](const std::string& name, bool down) {
            if (down) {
                CULog("Win Continue pressed");
            }
        });
    }
    
    if (_winRetryButton) {
        _winRetryButton->addListener([this](const std::string& name, bool down) {
            if (down && _retryCallback) {
                CULog("Win Retry pressed");
                _retryCallback();
            }
        });
    }
    
    showHeadsUpDisplay(true);
    showPauseMenu(false);
    showLosePage(false);
    showWinPage1(false);
    showWinPage2(false);

    
    _screenOffset = getPosition();

    Application::get()->setClearColor(Color4f::CORNFLOWER);
    return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GBIngameUI::dispose() {
    _pauseButton = nullptr;
    _resumeButton = nullptr;
    _retryButton = nullptr;
    _quitButton = nullptr;
    _settingButton = nullptr;
    _assets = nullptr;

    removeAllChildren();
}

/**
 * Sets whether the scene is currently active
 *
 * @param value whether the scene is currently active
 */
void GBIngameUI::setActive(bool value) {
//    _active = value;
//
//    if (_pauseButton) {
//        value ? _pauseButton->activate() : _pauseButton->deactivate();
//    }
//
//    if (_resumeButton) {
//        value ? _resumeButton->activate() : _resumeButton->deactivate();
//    }
//    if (_retryButton) {
//        value ? _retryButton->activate() : _retryButton->deactivate();
//    }
//    if (_quitButton) {
//        value ? _quitButton->activate() : _quitButton->deactivate();
//    }
//    if (_settingButton) {
//        value ? _settingButton->activate() : _settingButton->deactivate();
//    }
//
//    setVisible(value);
}

void GBIngameUI::setHP(int hp) {
    hp = std::max(0, std::min(hp, 100));

    int filled = hp / 20;
    bool showHalf = (hp % 20) == 10;

    for (int i = 0; i < 5; ++i) {
        _hpSegments[i]->setVisible(i < filled);
        _hpHalfSegments[i]->setVisible(i == filled && showHalf);
    }
}

void GBIngameUI::resetUI() {
    _currentHP = _maxHP;
    setHP(_currentHP);

//    _active = true;
    
    showHeadsUpDisplay(true);
    showPauseMenu(false);
    showLosePage(false);
    showWinPage1(false);
    showWinPage2(false);
    
//    auto hud = getChildByName("hud");
//        if (hud) {
//            hud->setVisible(true);
//        }
//
//    auto pausemenu = getChildByName("pausemenu");
//    if (pausemenu) {
//        pausemenu->setVisible(false);
//    }
//
//    auto settingMenu = getChildByName("settingmenu");
//    if (settingMenu) {
//        settingMenu->setVisible(false);
//    }
//
//    auto win1 = getChildByName("win1");
//    if (win1) {
//        win1->setVisible(false);
//    }
//    
//    auto win2 = getChildByName("win2");
//    if (win2) {
//        win2->setVisible(false);
//    }
//
//    auto lose = getChildByName("lose");
//    if (lose) {
//        lose->setVisible(false);
//    }
//
//    if (_pauseButton && !_pauseButton->isActive()) _pauseButton->activate();
//    if (_resumeButton && !_resumeButton->isActive()) _resumeButton->activate();
//    if (_retryButton && !_retryButton->isActive()) _retryButton->activate();
}

void GBIngameUI::setButtonsActive(std::shared_ptr<scene2::SceneNode> layer, bool active) {
    if (!layer) return;

    for (auto& child : layer->getChildren()) {
        auto button = std::dynamic_pointer_cast<scene2::Button>(child);
        if (button) {
            active ? button->activate() : button->deactivate();
        }
    }
}

void GBIngameUI::showPauseMenu(bool visible) {
    auto layer = getChildByName("pausemenu");
    if (layer) {
        layer->setVisible(visible);
        setButtonsActive(layer, visible);
    }
}

void GBIngameUI::showHeadsUpDisplay(bool visible) {
    auto layer = getChildByName("hud");
    if (layer) {
        layer->setVisible(visible);
        setButtonsActive(layer, visible);
    }
}

void GBIngameUI::showLosePage(bool visible) {
    auto layer = getChildByName("lose");
    if (layer) {
        layer->setVisible(visible);
        setButtonsActive(layer, visible);
    }
}

void GBIngameUI::showWinPage1(bool visible) {
    auto layer = getChildByName("win1");
    if (layer) {
        layer->setVisible(visible);
        setButtonsActive(layer, visible);
    }
}

void GBIngameUI::showWinPage2(bool visible) {
    auto layer = getChildByName("win2");
    if (layer) {
        layer->setVisible(visible);
        setButtonsActive(layer, visible);
    }
}
