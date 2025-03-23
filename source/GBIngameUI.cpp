//
//  GBIngameUI.cpp
//
//  This module handles the in-game UI, including pause button and HP bar.
//  Based on:
//
//  UIButtonScene.cpp
//  UI Demo
//
//  This module shows off a basic button.  The button is backed by a NinePatch
//  so it can resize dynamically.  It changes the text on a click.
//
//  Author: Walker White
//  Version: 1/20/18
//
#include "GBIngameUI.h"

using namespace cugl;

/** This is adjusted by screen aspect ratio to get the height */
#define SCENE_WIDTH 1024

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
        CULog("❌ false1");
        return false;
    } else if (!SceneNode::init()) {
        CULog("❌ false2");
        return false;
    }
    
    _assets = assets;
    
    Size screenSize = Application::get()->getDisplaySize();
    setContentSize(screenSize);
    
    auto layer = assets->get<scene2::SceneNode>("ingamescene");
    
    if (layer == nullptr) {
        CULog("❌ ingamescene not loaded");
        return false;
    }
    
    layer->setContentSize(screenSize);
    layer->doLayout(); // This rearranges the children to fit the screen
    addChild(layer);
    
    if (layer == nullptr) {
        CULog("❌ ingamescene not loaded");
    } else {
        CULog("✅ ingamescene loaded, children = %d", layer->getChildren().size());
    }
    
    _pauseButton = std::dynamic_pointer_cast<scene2::Button>(layer->getChildByName("pause"));
    _hpbar = layer->getChildByName("hpbar");
    
    if (_pauseButton) {
        _pauseButton->addListener([this](const std::string& name, bool pressed) {
            if (pressed) {
                CULog("Pause!");
            }
        });

        if (_active) {
            _pauseButton->activate();
        }
    }
    
    // XNA nostalgia
    Application::get()->setClearColor(Color4f::CORNFLOWER);
    return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GBIngameUI::dispose() {
    _pauseButton = nullptr;
    _hpbar = nullptr;
    _assets = nullptr;
    
    removeAllChildren();
}

/**
 * Sets whether the scene is currently active
 *
 * @param value whether the scene is currently active
 */
void GBIngameUI::setActive(bool value) {
    _active = value;
    if (_pauseButton) {
        if (value && !_pauseButton->isActive()) {
            _pauseButton->activate();
        } else if (!value && _pauseButton->isActive()) {
            _pauseButton->deactivate();
        }
    }
    
    setVisible(value);
}

