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
    auto winPage = assets->get<scene2::SceneNode>("win");

    if (headsUpDisplay == nullptr || pauseMenu == nullptr|| losePage == nullptr || winPage1 == nullptr || winPage == nullptr) {
        return false;
    }

    _hudTimeNum = std::dynamic_pointer_cast<scene2::Label>(headsUpDisplay->getChildByName("time_num"));
    _timeNum = std::dynamic_pointer_cast<scene2::Label>(winPage->getChildByName("time_num"));
    _parryNum = std::dynamic_pointer_cast<scene2::Label>(winPage->getChildByName("parry_num"));
    _hpNum = std::dynamic_pointer_cast<scene2::Label>(winPage->getChildByName("hp_num"));

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
    
    winPage->setContentSize(Size(1248, 576));
    winPage->doLayout();
    winPage->setVisible(false);
    addChild(winPage);

    setupHUD(headsUpDisplay);
    setupPause(pauseMenu);
    setupLose(losePage);
    setupWin1(winPage1);
    setupWin(winPage);
    
    showHeadsUpDisplay(true, true);
    showPauseMenu(false);
    showLosePage(false);
    showWinPage1(false);
    showWinPage(false);

    _screenOffset = getPosition();

    Application::get()->setClearColor(Color4f::BLACK);
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
        _quitButton->addListener([this](const std::string& name, bool down) {
            if (down && _quitCallback) {
                CULog("Quit pressed");
                _quitCallback();
            }
        });
    }

    if (_settingButton) {
        _settingButton->addListener([this](const std::string& name, bool down) {
            if (down && _settingsCallback) {
                CULog("Setting pressed");
                _settingsCallback();
            }
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
        _loseQuitButton->addListener([this](const std::string& name, bool down) {
            if (down && _loseQuitCallback) {
                CULog("Lose Quit pressed");
                _loseQuitCallback();
            }
        });
    }
}

void GBIngameUI::setupWin1(std::shared_ptr<cugl::scene2::SceneNode>& winPage1)
{
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
}

void GBIngameUI::setupWin(std::shared_ptr<cugl::scene2::SceneNode>& winPage)
{
    _winContinueButton = std::dynamic_pointer_cast<scene2::Button>(winPage->getChildByName("win_continue"));
    _winRetryButton = std::dynamic_pointer_cast<scene2::Button>(winPage->getChildByName("win_retry"));
    if (_winContinueButton) {
        _winContinueButton->addListener([this](const std::string& name, bool down) {
            if (down) {
                CULog("Win Continue pressed");
                _winContinueCallback();
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
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GBIngameUI::dispose() {
    _pauseButton->clearListeners();
    _resumeButton->clearListeners();
    _retryButton->clearListeners();
    _quitButton->clearListeners();
    _settingButton->clearListeners();
    _loseRetryButton->clearListeners();
    _loseQuitButton->clearListeners();
    _continueButton->clearListeners();
    _winContinueButton->clearListeners();
    _winRetryButton->clearListeners();

    _pauseButton = nullptr;
    _resumeButton = nullptr;
    _retryButton = nullptr;
    _quitButton = nullptr;
    _settingButton = nullptr;
    _loseRetryButton = nullptr;
    _loseQuitButton = nullptr;
    _continueButton = nullptr;
    _winContinueButton = nullptr;
    _winRetryButton = nullptr;

    _assets = nullptr;

    removeAllChildren();
}

void GBIngameUI::setHP(int hp) {
    hp = std::max(0, std::min(hp, 100));

    int fullSegments = hp / 20;
    bool hasHalfSegment = (hp % 20) >= 10;

    for (int i = 0; i < 5; ++i) {
        _hpSegments[i]->setVisible(i < fullSegments);
        _hpHalfSegments[i]->setVisible(i == fullSegments && hasHalfSegment);
    }

    _currentHP = hp;
}

void GBIngameUI::setTime(float timeSpent) {
    int minutes = static_cast<int>(timeSpent) / 60;
    int seconds = static_cast<int>(timeSpent) % 60;
    char timeStr[16];
    snprintf(timeStr, sizeof(timeStr), "%d:%02d", minutes, seconds);
    if (_hudTimeNum) {
        _hudTimeNum->setText(timeStr);
    }
}

void GBIngameUI::resetUI() {
    _currentHP = _maxHP;
    setHP(_currentHP);
    showHeadsUpDisplay(true, true);
    showPauseMenu(false);
    showLosePage(false);
    showWinPage1(false);
    showWinPage(false);
}


/**
 * Manage activation and visibility of pages
 */
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

void GBIngameUI::showHeadsUpDisplay(bool visible, bool active) {
    auto layer = getChildByName("hud");
    if (layer) {
        layer->setVisible(visible);
        layer->getChildByName("pause")->setVisible(active);
        setButtonsActive(layer, active);
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

void GBIngameUI::showWinPage(bool visible) {
    auto layer = getChildByName("win");
    if (layer) {
        layer->setVisible(visible);
        setButtonsActive(layer, visible);
    }
}

void GBIngameUI::updateStats(float timeSpent, int parryCount) {
    if (_timeNum) {
        int minutes = static_cast<int>(timeSpent) / 60;
        int seconds = static_cast<int>(timeSpent) % 60;
        char timeStr[16];
        snprintf(timeStr, sizeof(timeStr), "%d:%02d", minutes, seconds);
        _timeNum->setText(timeStr);
    }

    if (_parryNum) {
        _parryNum->setText(std::to_string(parryCount));
    }
    
    if (_hpNum) {
        _hpNum->setText(std::to_string(_currentHP));
    }
}
