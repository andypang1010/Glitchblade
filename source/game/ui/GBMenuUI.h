//
//  GBMenuUI.h
//
//  This module handles all non-gameplay scenes, including homepage, level select, settings, and info page.
//
//  Author: Vince Qian
//
//  Reference:
//  UIButtonScene.h
//  UI Demo by Walker White (Version: 1/20/18)
//
#ifndef __GB_MENU_UI_H__
#define __GB_MENU_UI_H__

#include <cugl/cugl.h>

/**
 * A unified UI controller for all non-gameplay scenes:
 * homepage, level select, settings, and info.
 */
class GBMenuUI : public cugl::scene2::SceneNode {
protected:
    std::shared_ptr<cugl::AssetManager> _assets;

    // UI Layers
    std::shared_ptr<cugl::scene2::SceneNode> _home;
    std::shared_ptr<cugl::scene2::SceneNode> _levelSelectionHead;
    std::shared_ptr<cugl::scene2::SceneNode> _levelSelection1;
    std::shared_ptr<cugl::scene2::SceneNode> _levelSelection2;
    std::shared_ptr<cugl::scene2::SceneNode> _levelSelection3;
    std::shared_ptr<cugl::scene2::SceneNode> _levelSelection4;
    std::shared_ptr<cugl::scene2::SceneNode> _homeSetting;
    std::shared_ptr<cugl::scene2::SceneNode> _info;

    // Level Selection Head buttons
    std::shared_ptr<cugl::scene2::Button> _homeButton;
    std::shared_ptr<cugl::scene2::Button> _levelSettingButton;
    std::shared_ptr<cugl::scene2::Button> _previousSceneButton;
    std::shared_ptr<cugl::scene2::Button> _nextSceneButton;
    
    // Level buttons
    std::shared_ptr<cugl::scene2::Button> _level1Button;
    std::shared_ptr<cugl::scene2::Button> _level2Button;
    std::shared_ptr<cugl::scene2::Button> _level3Button;
    std::shared_ptr<cugl::scene2::Button> _level4Button;
    std::shared_ptr<cugl::scene2::Button> _level5Button;

    // Home buttons
    std::shared_ptr<cugl::scene2::Button> _startButton;
    std::shared_ptr<cugl::scene2::Button> _infoButton;
    std::shared_ptr<cugl::scene2::Button> _homeSettingButton;
    
    // Home Setting buttons
    std::shared_ptr<cugl::scene2::Button> _musicButton;
    std::shared_ptr<cugl::scene2::Button> _soundButton;
    std::shared_ptr<cugl::scene2::Button> _backButton;
    
    // Info button
    std::shared_ptr<cugl::scene2::Button> _infoHomeButton;

    // Callback handlers
    std::function<void()> _startCallback;
    std::function<void()> _infoCallback;
    std::function<void()> _homeSettingCallback;
    std::function<void()> _homeCallback;
    std::function<void()> _previousSceneCallback;
    std::function<void()> _nextSceneCallback;
    std::function<void()> _musicCallback;
    std::function<void()> _soundCallback;
    std::function<void()> _backCallback;
    std::function<void()> _level1Callback;
    std::function<void()> _level2Callback;
    std::function<void()> _level3Callback;
    std::function<void()> _level4Callback;
    std::function<void()> _level5Callback;

    void setButtonsActive(std::shared_ptr<cugl::scene2::SceneNode> layer, bool active);

public:
    ~GBMenuUI() { dispose(); }

    virtual void dispose() override;
    bool init(const std::shared_ptr<cugl::AssetManager>& assets, int scene);

    static std::shared_ptr<GBMenuUI> alloc(const std::shared_ptr<cugl::AssetManager>& assets, int scene) {
        std::shared_ptr<GBMenuUI> result = std::make_shared<GBMenuUI>();
        return (result->init(assets, scene) ? result : nullptr);
    }

    // Setup methods for each UI section
    void setupHome(std::shared_ptr<cugl::scene2::SceneNode>& home);
    void setupLevelSelectionHead(std::shared_ptr<cugl::scene2::SceneNode>& levelSelectionHead);
    void setupLevelSelection1(std::shared_ptr<cugl::scene2::SceneNode>& levelSelection1);
    void setupLevelSelection2(std::shared_ptr<cugl::scene2::SceneNode>& levelSelection2);
    void setupLevelSelection3(std::shared_ptr<cugl::scene2::SceneNode>& levelSelection3);
    void setupLevelSelection4(std::shared_ptr<cugl::scene2::SceneNode>& levelSelection4);
    void setupHomeSetting(std::shared_ptr<cugl::scene2::SceneNode>& homeSetting);
    void setupInfo(std::shared_ptr<cugl::scene2::SceneNode>& info);

    // UI control
    void showHome(bool visible);
    void showLevelSelectionHead(bool visible, std::optional<bool> showPrev = std::nullopt, std::optional<bool> showNext = std::nullopt);
    void showLevelSelection1(bool visible);
    void showLevelSelection2(bool visible);
    void showLevelSelection3(bool visible);
    void showLevelSelection4(bool visible);
    void showHomeSetting(bool visible);
    void showInfo(bool visible);

    // Unlockable level logic
    void setHighestPlayable(int highestPlayableLevel);

    // Callback setters
    void setStartCallback(const std::function<void()>& callback) { _startCallback = callback; }
    void setInfoCallback(const std::function<void()>& callback) { _infoCallback = callback; }
    void setHomeSettingCallback(const std::function<void()>& callback) { _homeSettingCallback = callback; }
    void setHomeCallback(const std::function<void()>& callback) { _homeCallback = callback; }
    void setPreviousSceneCallback(const std::function<void()>& callback) { _previousSceneCallback = callback; }
    void setNextSceneCallback(const std::function<void()>& callback) { _nextSceneCallback = callback; }
    void setMusicCallback(const std::function<void()>& callback) { _musicCallback = callback; }
    void setSoundCallback(const std::function<void()>& callback) { _soundCallback = callback; }
    void setBackCallback(const std::function<void()>& callback) { _backCallback = callback; }
    void setLevel1Callback(const std::function<void()>& callback) { _level1Callback = callback; }
    void setLevel2Callback(const std::function<void()>& callback) { _level2Callback = callback; }
    void setLevel3Callback(const std::function<void()>& callback) { _level3Callback = callback; }
    void setLevel4Callback(const std::function<void()>& callback) { _level4Callback = callback; }
    void setLevel5Callback(const std::function<void()>& callback) { _level5Callback = callback; }
};

#endif /* __GB_MENU_UI_H__ */
