//
//  GBPauseMenu.h
//
//  This module handles the pause menu UI, including menu background,
//  HP bar, player image, and four buttons: resume, restart, exit, and setting.
//  Based on:
//
//  UIButtonScene.h
//  UI demo
//
//  This module shows off a basic button.  The button is backed by a NinePatch
//  so it can resize dynamically.  It changes the text on a click.
//
//  Author: Walker White
//  Version: 1/20/18
//
#ifndef __GB_PAUSE_MENU_H__
#define __GB_PAUSE_MENU_H__
#include <cugl/cugl.h>

/**
 * A scene for the pause menu interface
 */
class GBPauseMenu : public cugl::scene2::SceneNode {
protected:
    /** The asset manager for this game mode. */
    std::shared_ptr<cugl::AssetManager> _assets;

    /** The pause menu buttons */
    std::shared_ptr<cugl::scene2::Button> _resumeButton;
    std::shared_ptr<cugl::scene2::Button> _restartButton;
    std::shared_ptr<cugl::scene2::Button> _exitButton;
    std::shared_ptr<cugl::scene2::Button> _settingButton;
    
    /** The HP bar container node. */
    std::shared_ptr<cugl::scene2::SceneNode> _hpbar;
    /** Full segment nodes */
    std::vector<std::shared_ptr<cugl::scene2::PolygonNode>> _hpSegments;
    /** Half segment nodes */
    std::vector<std::shared_ptr<cugl::scene2::PolygonNode>> _hpHalfSegments;

    bool _active;

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
    ~GBPauseMenu() { dispose(); }

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

    static std::shared_ptr<GBPauseMenu> alloc(const std::shared_ptr<cugl::AssetManager>& assets) {
        std::shared_ptr<GBPauseMenu> result = std::make_shared<GBPauseMenu>();
        return (result->init(assets) ? result : nullptr);
    }

    /**
     * Sets whether the scene is currently active
     *
     * @param value whether the scene is currently active
     */
    void setActive(bool value);
    
    void setHP(int hp);

    bool isActive() const { return _active; }
    
    std::shared_ptr<cugl::scene2::Button> getResumeButton() const {
        return _resumeButton;
    }

    std::shared_ptr<cugl::scene2::Button> getRestartButton() const {
        return _restartButton;
    }
};

#endif /* __GB_PAUSE_MENU_H__ */
