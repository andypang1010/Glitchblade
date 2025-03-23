//
//  GBIngameUI.h
//
//  This module handles the in-game UI, including pause button and HP bar.
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
#ifndef __GB_INGAME_UI_H__
#define __GB_INGAME_UI_H__
#include <cugl/cugl.h>

/**
 * A scene for demoing a simple button
 */
class GBIngameUI : public cugl::scene2::SceneNode {
protected:
    /** The asset manager for this game mode. */
    std::shared_ptr<cugl::AssetManager> _assets;

    /** The pause button in top-right corner. */
    std::shared_ptr<cugl::scene2::Button> _pauseButton;
    /** The HP bar in top-left corner. */
    std::shared_ptr<cugl::scene2::SceneNode> _hpbar;
    std::vector<std::shared_ptr<cugl::scene2::PolygonNode>> _hpSegments;
    
    int _maxHP = 5;
    int _currentHP = 5;

    bool _active;
    
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
    
    static std::shared_ptr<GBIngameUI> alloc(const std::shared_ptr<cugl::AssetManager>& assets) {
        std::shared_ptr<GBIngameUI> result = std::make_shared<GBIngameUI>();
        return (result->init(assets) ? result : nullptr);
    }
    
    /**
     * Sets whether the scene is currently active
     *
     * @param value whether the scene is currently active
     */
    void setActive(bool value);
    
    bool isActive() const { return _active;}

};

#endif /* __GB_INGAME_UI_H__ */
