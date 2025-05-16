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
 * Initializes the in-game UI system with all necessary elements,
 * including HUD, pause menu, win/lose pages, and settings.
 */
bool GBIngameUI::init(const std::shared_ptr<AssetManager>& assets) {
    if (assets == nullptr) {
        return false;
    } else if (!SceneNode::init()) {
        return false;
    }

    _assets = assets;

    setContentSize(Size(1248, 576));

    // Load all UI layers from assets
    auto headsUpDisplay = assets->get<scene2::SceneNode>("hud");
    auto pauseMenu = assets->get<scene2::SceneNode>("pausemenu");
    auto losePage = assets->get<scene2::SceneNode>("lose");
    auto winPage = assets->get<scene2::SceneNode>("win");
    auto settingMenu = assets->get<scene2::SceneNode>("settingmenu");

    if (headsUpDisplay == nullptr || pauseMenu == nullptr|| losePage == nullptr || winPage == nullptr) {
        return false;
    }
    
    // Get references to specific UI elements (labels) for later update
    _hudTimeNum = std::dynamic_pointer_cast<scene2::Label>(headsUpDisplay->getChildByName("time_num"));
    _hudWaveNum = std::dynamic_pointer_cast<scene2::Label>(headsUpDisplay->getChildByName("wave_num"));
    _winTimeNum = std::dynamic_pointer_cast<scene2::Label>(winPage->getChildByName("time_num"));
    _winParryNum = std::dynamic_pointer_cast<scene2::Label>(winPage->getChildByName("parry_num"));
    _winHpNum = std::dynamic_pointer_cast<scene2::Label>(winPage->getChildByName("hp_num"));
    _loseEnemyNum = std::dynamic_pointer_cast<scene2::Label>(losePage->getChildByName("enemy_num"));

    // Add each UI layer to the main UI scene
    headsUpDisplay->setContentSize(Size(1248, 576));
    headsUpDisplay->doLayout();
    addChild(headsUpDisplay);

    pauseMenu->setContentSize(Size(1248, 576));
    pauseMenu->doLayout();
    pauseMenu->setVisible(false);
    addChild(pauseMenu);
    
    settingMenu->setContentSize(Size(1248, 576));
    settingMenu->doLayout();
    settingMenu->setVisible(false);
    addChild(settingMenu);
    
    losePage->setContentSize(Size(1248, 576));
    losePage->doLayout();
    losePage->setVisible(false);
    addChild(losePage);
    
    winPage->setContentSize(Size(1248, 576));
    winPage->doLayout();
    winPage->setVisible(false);
    addChild(winPage);

    setupHUD(headsUpDisplay);
    setupPause(pauseMenu);
    setupSetting(settingMenu);
    setupLose(losePage);
    setupWin(winPage);
    
    // Only show HUD by default
    showHeadsUpDisplay(true, true);
    showPauseMenu(false);
    showSettingMenu(false);
    showLosePage(false);
    showWinPage(false);
    
    // Set anchor point and position based on actual screen size for scaling
    setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    setPosition(Vec2::ZERO);

    float scaleX = Application::get()->getDisplaySize().width / 1248.0f;
    float scaleY = Application::get()->getDisplaySize().height / 576.0f;
    float scale = std::min(scaleX, scaleY);

    this->setScale(scale);
    float offsetX = (Application::get()->getDisplaySize().width - 1248 * scale) / 2.0f;
    float offsetY = (Application::get()->getDisplaySize().height - 576 * scale) / 2.0f;
    this->setPosition(Vec2(offsetX, offsetY));

    Application::get()->setClearColor(Color4f::BLACK);
    return true;
}

/**
 * Setup HUD elements: pause button, HP, combo, and progress bars
 */
void GBIngameUI::setupHUD(std::shared_ptr<cugl::scene2::SceneNode>& headsUpDisplay)
{
    _pauseButton = std::dynamic_pointer_cast<scene2::Button>(headsUpDisplay->getChildByName("pause"));

    // Add pause button listener to trigger pause callback
    if (_pauseButton) {
        _pauseButton->addListener([this](const std::string& name, bool down) {
            if (down && _pauseCallback) {
                CULog("Pause!");
                _pauseCallback();
            }
        });
    }

    // Collect all HP segment nodes for later display update
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
    _progressBar = headsUpDisplay->getChildByName<scene2::PolygonNode>("progress_ind");
    _comboBar = headsUpDisplay->getChildByName<scene2::PolygonNode>("combo_bar");
    
    // Store original width of bars for later resizing
    _comboBarOriginalWidth = _comboBar->getContentSize().width;
    _progressBarOriginalWidth = _progressBar->getContentSize().width;
}

/**
 * Setup pause menu buttons and their respective callbacks
 */
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
            if (down && _settingCallback) {
                CULog("Setting pressed");
                _settingCallback();
            }
        });
    }
}

/**
 * Setup settings menu buttons and callbacks
 */
void GBIngameUI::setupSetting(std::shared_ptr<cugl::scene2::SceneNode>& settingMenu)
{
    _settingResumeButton = std::dynamic_pointer_cast<scene2::Button>(settingMenu->getChildByName("resume"));
    _musicButton = std::dynamic_pointer_cast<scene2::Button>(settingMenu->getChildByName("music"));
    _soundButton = std::dynamic_pointer_cast<scene2::Button>(settingMenu->getChildByName("sound"));
    _backButton = std::dynamic_pointer_cast<scene2::Button>(settingMenu->getChildByName("back"));

    if (_settingResumeButton) {
        _settingResumeButton->addListener([this](const std::string& name, bool down) {
            if (down && _resumeCallback) {
                CULog("Setting Resume pressed");
                _resumeCallback();
            }
        });
    }

    if (_musicButton) {
        _musicButton->addListener([this](const std::string& name, bool down) {
            if (down && _musicCallback) {
                CULog("Music pressed");
                _musicCallback();
            }
        });
    }

    if (_soundButton) {
        _soundButton->addListener([this](const std::string& name, bool down) {
            if (down && _soundCallback) {
                CULog("Sound pressed");
                _soundCallback();
            }
        });
    }

    if (_backButton) {
        _backButton->addListener([this](const std::string& name, bool down) {
            if (down && _pauseCallback) {
                CULog("Back pressed");
                _pauseCallback();
            }
        });
    }
}

/**
 * Setup lose screen buttons
 */
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
            if (down && _quitCallback) {
                CULog("Lose Quit pressed");
                _quitCallback();
            }
        });
    }
}

/**
 * Setup win screen buttons
 */
void GBIngameUI::setupWin(std::shared_ptr<cugl::scene2::SceneNode>& winPage)
{
    _winContinueButton = std::dynamic_pointer_cast<scene2::Button>(winPage->getChildByName("win_continue"));
    _winQuitButton = std::dynamic_pointer_cast<scene2::Button>(winPage->getChildByName("win_quit"));
    if (_winContinueButton) {
        _winContinueButton->addListener([this](const std::string& name, bool down) {
            if (down) {
                CULog("Win Continue pressed");
                _winContinueCallback();
            }
        });
    }
    
    if (_winQuitButton) {
        _winQuitButton->addListener([this](const std::string& name, bool down) {
            if (down && _quitCallback) {
                CULog("Win Quit pressed");
                _quitCallback();
            }
        });
    }
}

/**
 * Clean up all button listeners and clear references to UI elements
 */
void GBIngameUI::dispose() {
    _pauseButton->clearListeners();
    _resumeButton->clearListeners();
    _retryButton->clearListeners();
    _quitButton->clearListeners();
    _settingButton->clearListeners();
    _settingResumeButton->clearListeners();
    _musicButton->clearListeners();
    _soundButton->clearListeners();
    _backButton->clearListeners();
    _loseRetryButton->clearListeners();
    _loseQuitButton->clearListeners();
    _winContinueButton->clearListeners();
    _winQuitButton->clearListeners();

    _pauseButton = nullptr;
    _resumeButton = nullptr;
    _retryButton = nullptr;
    _quitButton = nullptr;
    _settingButton = nullptr;
    _settingResumeButton = nullptr;
    _musicButton = nullptr;
    _soundButton = nullptr;
    _backButton = nullptr;
    _loseRetryButton = nullptr;
    _loseQuitButton = nullptr;
    _winContinueButton = nullptr;
    _winQuitButton = nullptr;

    _assets = nullptr;

    removeAllChildren();
}

/**
 * Update HP segments on the HUD based on player's current HP
 */
void GBIngameUI::updateHP(int hp) {
    hp = std::max(0, std::min(hp, 100));

    int fullSegments = hp / 20;
    bool hasHalfSegment = (hp % 20) >= 10;

    for (int i = 0; i < 5; ++i) {
        _hpSegments[i]->setVisible(i < fullSegments);
        _hpHalfSegments[i]->setVisible(i == fullSegments && hasHalfSegment);
    }

    _currentHP = hp;
}

/**
 * Format and display time on the HUD
 */
void GBIngameUI::updateTime(float timeSpent) {
    int minutes = static_cast<int>(timeSpent) / 60;
    int seconds = static_cast<int>(timeSpent) % 60;
    char timeStr[16];
    snprintf(timeStr, sizeof(timeStr), "%d:%02d", minutes, seconds);
    if (_hudTimeNum) {
        _hudTimeNum->setText(timeStr);
    }
}

/**
 * Update progression bar and wave label on the HUD
 */
void GBIngameUI::updateProgression(int spawnedCount, int totalCount, int waveIndex) {
    if (!_progressBar) return;
    
    // Resize progress bar proportionally to spawned enemy ratio
    float ratio = (totalCount > 0.0f) ? ((float)spawnedCount) / totalCount : 0.0f;
    float barWidth = _progressBarOriginalWidth * ratio;
    float barHeight = _progressBar->getContentSize().height;

    _progressBar->setContentSize(Size(barWidth, barHeight));
    _progressBar->setPolygon(Rect(0, 0, barWidth, barHeight));
    
    // Display wave number or "Final Wave" on HUD
    if (_hudWaveNum) {
        if (waveIndex == -1) {
            _hudWaveNum->setText("Final Wave");
        } else {
            _hudWaveNum->setText("Wave " + std::to_string(waveIndex + 1));
        }
    }
}

/**
 * Update win/lose screen stats: time, parries, HP, enemy kill percentage
 */
void GBIngameUI::updateStats(float timeSpent, int parryCount, int spawnedCount, int totalCount) {
    int minutes = static_cast<int>(timeSpent) / 60;
    int seconds = static_cast<int>(timeSpent) % 60;
    char timeStr[16];
    snprintf(timeStr, sizeof(timeStr), "%d:%02d", minutes, seconds);
    
    if (_winTimeNum) {
        _winTimeNum->setText(timeStr);
    }

    if (_winParryNum) {
        _winParryNum->setText(std::to_string(parryCount));
    }
    
    if (_winHpNum) {
        _winHpNum->setText(std::to_string(_currentHP));
    }
    
    // Show % of enemies defeated on lose screen
    if (_loseEnemyNum) {
        if (spawnedCount > 0) {
            spawnedCount--;
        }
        float percentage = totalCount > 0 ? (float)spawnedCount / totalCount * 100.0f : 0.0f;
        std::string text = std::to_string((int)percentage) + "%";
        _loseEnemyNum->setText(text);
    }
}

/**
 * Update combo bar width based on current combo value
 */
void GBIngameUI::updateComboBar(float value) {
    _comboValue = std::max(0.0f, std::min(value, _comboMax));

    float ratio = _comboValue / _comboMax;
    float barWidth = _comboBarOriginalWidth * ratio;
    float barHeight = _comboBar->getContentSize().height;

    _comboBar->setContentSize(Size(barWidth, barHeight));
    _comboBar->setPolygon(Rect(0, 0, barWidth, barHeight));
}

/**
 * Reset UI to default state, used when restarting the game
 */
void GBIngameUI::resetUI() {
    _currentHP = _maxHP;
    updateHP(_currentHP);
    showHeadsUpDisplay(true, true);
    showPauseMenu(false);
    showSettingMenu(false);
    showLosePage(false);
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

void GBIngameUI::showSettingMenu(bool visible) {
    auto layer = getChildByName("settingmenu");
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

void GBIngameUI::showWinPage(bool visible) {
    auto layer = getChildByName("win");
    if (layer) {
        layer->setVisible(visible);
        setButtonsActive(layer, visible);
    }
}
