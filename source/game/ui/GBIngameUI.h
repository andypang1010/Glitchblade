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

    /** The pause button in top-right corner. */
    std::shared_ptr<cugl::scene2::Button> _pauseButton;
    
    /** The pause menu buttons */
    std::shared_ptr<cugl::scene2::Button> _resumeButton;
    std::shared_ptr<cugl::scene2::Button> _retryButton;
    std::shared_ptr<cugl::scene2::Button> _quitButton;
    std::shared_ptr<cugl::scene2::Button> _settingButton;
    
    /** The lose page buttons */
    std::shared_ptr<cugl::scene2::Button> _loseRetryButton;
    std::shared_ptr<cugl::scene2::Button> _loseQuitButton;
    
    /** The win page 1 buttons */
    std::shared_ptr<cugl::scene2::Button> _continueButton;
    
    /** The win page 2 buttons */
    std::shared_ptr<cugl::scene2::Button> _winContinueButton;
    std::shared_ptr<cugl::scene2::Button> _winRetryButton;
    
    std::function<void()> _pauseCallback;
    std::function<void()> _resumeCallback;
    std::function<void()> _retryCallback;
    std::function<void()> _continueCallback;

    /** HP segments */
    std::vector<std::shared_ptr<cugl::scene2::PolygonNode>> _hpSegments;
    std::vector<std::shared_ptr<cugl::scene2::PolygonNode>> _hpHalfSegments;
    
    std::shared_ptr<cugl::scene2::Label> _timeNum;
    std::shared_ptr<cugl::scene2::Label> _parryNum;
    std::shared_ptr<cugl::scene2::Label> _hpNum;

    int _maxHP = 100;
    int _currentHP = 100;

    bool _active;
    
    void setButtonsActive(std::shared_ptr<cugl::scene2::SceneNode> layer, bool active);
    
public:
    cugl::Vec2 _screenOffset;

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
    bool init(const std::shared_ptr<cugl::AssetManager>& assets);

    void setupLose(std::shared_ptr<cugl::scene2::SceneNode>& losePage);

    void setupPause(std::shared_ptr<cugl::scene2::SceneNode>& pauseMenu);

    void setupHUD(std::shared_ptr<cugl::scene2::SceneNode>& headsUpDisplay);
    
    void setupWin1(std::shared_ptr<cugl::scene2::SceneNode>& winPage1);
    
    void setupWin2(std::shared_ptr<cugl::scene2::SceneNode>& winPage2);
    
    static std::shared_ptr<GBIngameUI> alloc(const std::shared_ptr<cugl::AssetManager>& assets) {
        std::shared_ptr<GBIngameUI> result = std::make_shared<GBIngameUI>();
        return (result->init(assets) ? result : nullptr);
    }
    
    /**
     * Updates the visible HP bar segments to match current HP.
     *
     * @param hp  The current HP value (must be <= _maxHP)
     */
    void setHP(int hp);
    
    void resetUI();

    bool isActive() const { return _active;}
    
    void setPauseCallback(const std::function<void()>& callback) { _pauseCallback = callback; }
    void setResumeCallback(const std::function<void()>& callback) { _resumeCallback = callback; }
    void setRetryCallback(const std::function<void()>& callback) { _retryCallback = callback; }
    void setContinueCallback(const std::function<void()>& callback) { _continueCallback = callback; }

    void showHeadsUpDisplay(bool visible);
    void showPauseMenu(bool visible);
//    void showSettingMenu(bool visible);
    void showWinPage1(bool visible);
    void showWinPage2(bool visible);
    void showLosePage(bool visible);
    
    void updateStats(float timeSpent, int parryCount);
    
    // Accessors
    std::shared_ptr<cugl::scene2::Button> getPauseButton() const {
        return _pauseButton;
    }

    std::shared_ptr<cugl::scene2::Button> getResumeButton() const {
        return _resumeButton;
    }

    std::shared_ptr<cugl::scene2::Button> getRetryButton() const {
        return _retryButton;
    }

    std::shared_ptr<cugl::scene2::Button> getQuitButton() const {
        return _quitButton;
    }

    std::shared_ptr<cugl::scene2::Button> getSettingButton() const {
        return _settingButton;
    }
    
};

#endif /* __GB_INGAME_UI_H__ */
