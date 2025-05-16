//
//  GBIngameUI.h
//
//  This module handles all in-game UI, including HUD, pause menu, settings, and win/lose page.
//
//  Author: Vince Qian
//
//  Reference:
//  UIButtonScene.h
//  UI Demo by Walker White (Version: 1/20/18)
//
#ifndef __GB_INGAME_UI_H__
#define __GB_INGAME_UI_H__
#include <cugl/cugl.h>

/**
 * A unified UI scene for in-game HUD, pause menu, and related UI elements.
 */
class GBIngameUI : public cugl::scene2::SceneNode {
protected:
    /** The asset manager for this game mode. */
    std::shared_ptr<cugl::AssetManager> _assets;

    /** The HUD button */
    std::shared_ptr<cugl::scene2::Button> _pauseButton;
    
    /** The pause menu buttons */
    std::shared_ptr<cugl::scene2::Button> _resumeButton;
    std::shared_ptr<cugl::scene2::Button> _retryButton;
    std::shared_ptr<cugl::scene2::Button> _quitButton;
    std::shared_ptr<cugl::scene2::Button> _settingButton;
    
    /** The setting menu buttons */
    std::shared_ptr<cugl::scene2::Button> _settingResumeButton;
    std::shared_ptr<cugl::scene2::Button> _musicButton;
    std::shared_ptr<cugl::scene2::Button> _soundButton;
    std::shared_ptr<cugl::scene2::Button> _backButton;
    
    /** The lose page buttons */
    std::shared_ptr<cugl::scene2::Button> _loseRetryButton;
    std::shared_ptr<cugl::scene2::Button> _loseQuitButton;
    
    /** The win page buttons */
    std::shared_ptr<cugl::scene2::Button> _winContinueButton;
    std::shared_ptr<cugl::scene2::Button> _winQuitButton;
    
    /** Button callbacks for interactivity */
    std::function<void()> _pauseCallback;
    std::function<void()> _resumeCallback;
    std::function<void()> _retryCallback;
    std::function<void()> _quitCallback;
    std::function<void()> _settingCallback;
    std::function<void()> _musicCallback;
    std::function<void()> _soundCallback;
    std::function<void()> _winContinueCallback;

    /** HP Bar segments */
    std::vector<std::shared_ptr<cugl::scene2::PolygonNode>> _hpSegments;
    std::vector<std::shared_ptr<cugl::scene2::PolygonNode>> _hpHalfSegments;
    
    /** Combo Bar segments */
    std::shared_ptr<cugl::scene2::PolygonNode> _comboBar;
    float _comboValue = 0.0f;
    float _comboMax = 100.0f;
    float _comboBarOriginalWidth = 0;
   
    /** Progress Bar segments */
    std::shared_ptr<cugl::scene2::PolygonNode> _progressBar;
    float _progressBarOriginalWidth = 0;
    
    /** HUD info labels (time & wave number) */
    std::shared_ptr<cugl::scene2::Label> _hudTimeNum;
    std::shared_ptr<cugl::scene2::Label> _hudWaveNum;

    /** Win screen labels */
    std::shared_ptr<cugl::scene2::Label> _winTimeNum;
    std::shared_ptr<cugl::scene2::Label> _winParryNum;
    std::shared_ptr<cugl::scene2::Label> _winHpNum;

    /** Lose screen label */
    std::shared_ptr<cugl::scene2::Label> _loseEnemyNum;

    int _maxHP = 100;
    int _currentHP = 100;
    
    /**
     * Enables or disables all buttons within a UI layer.
     *
     * Used when showing/hiding menus.
     */
    void setButtonsActive(std::shared_ptr<cugl::scene2::SceneNode> layer, bool active);
    
public:

#pragma mark -
#pragma mark Constructors
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     *
     * This method is different from dispose() in that it ALSO shuts off any
     * static resources, like the input controller.
     */
    ~GBIngameUI() { dispose(); }
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     */
    virtual void dispose() override;
    
    /**
     * Initializes the in-game UI system with all necessary elements,
     * including HUD, pause menu, win/lose pages, and settings.
     */
    bool init(const std::shared_ptr<cugl::AssetManager>& assets);

    /**
     * Initializes and lays out each page.
     */
    void setupLose(std::shared_ptr<cugl::scene2::SceneNode>& losePage);
    void setupPause(std::shared_ptr<cugl::scene2::SceneNode>& pauseMenu);
    void setupSetting(std::shared_ptr<cugl::scene2::SceneNode>& settingMenu);
    void setupHUD(std::shared_ptr<cugl::scene2::SceneNode>& headsUpDisplay);
    void setupWin(std::shared_ptr<cugl::scene2::SceneNode>& winPage);
    
    static std::shared_ptr<GBIngameUI> alloc(const std::shared_ptr<cugl::AssetManager>& assets) {
        std::shared_ptr<GBIngameUI> result = std::make_shared<GBIngameUI>();
        return (result->init(assets) ? result : nullptr);
    }
    
    /**
     * Updates each UI segments.
     */
    void updateHP(int hp);
    void updateTime(float timeSpent);
    void updateProgression(int spawnedCount, int totalCount, int waveIndex);
    void updateStats(float timeSpent, int parryCount, int spawnedCount, int totalCount);
    void updateComboBar(float value);
    
    /**
     * Resets UI state for a new game session.
     */
    void resetUI();
    
    /**
     * Set button callbacks.
     */
    void setPauseCallback(const std::function<void()>& callback) { _pauseCallback = callback; }
    void setResumeCallback(const std::function<void()>& callback) { _resumeCallback = callback; }
    void setRetryCallback(const std::function<void()>& callback) { _retryCallback = callback; }
    void setQuitCallback(const std::function<void()>& callback) { _quitCallback = callback; }
    void setSettingCallback(const std::function<void()>& callback) { _settingCallback = callback; }
    void setMusicCallback(const std::function<void()>& callback) { _musicCallback = callback; }
    void setSoundCallback(const std::function<void()>& callback) { _soundCallback = callback; }
    void setWinContinueCallback(const std::function<void()>& callback) { _winContinueCallback = callback; }

    /**
     * Shows or hides each page.
     */
    void showHeadsUpDisplay(bool visible, bool active);
    void showPauseMenu(bool visible);
    void showSettingMenu(bool visible);
    void showWinPage(bool visible);
    void showLosePage(bool visible);
};

#endif /* __GB_INGAME_UI_H__ */
