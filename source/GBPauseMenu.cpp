//
//  GBPauseMenu.cpp
//
//  This module handles the pause menu UI, including menu background,
//  HP bar, player image, and four buttons: resume, restart, exit, and setting.
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
#include "GBPauseMenu.h"

using namespace cugl;

/** This is adjusted by screen aspect ratio to get the height */
#define SCENE_WIDTH 1024

/**
 * Initializes the controller contents, and starts the game
 *
 * @param assets    The (loaded) assets for this game mode
 *
 * @return true if the controller is initialized properly, false otherwise.
 */
bool GBPauseMenu::init(const std::shared_ptr<AssetManager>& assets) {
    if (assets == nullptr) {
        return false;
    } else if (!SceneNode::init()) {
        return false;
    }

    _assets = assets;

    Size screenSize = Application::get()->getDisplaySize();
    setContentSize(screenSize);

    auto layer = assets->get<scene2::SceneNode>("pausemenu");

    if (layer == nullptr) {
        return false;
    }

    layer->setContentSize(screenSize);
    layer->doLayout();
    addChild(layer);
    setActive(true);

    _resumeButton = std::dynamic_pointer_cast<scene2::Button>(layer->getChildByName("play"));
    _restartButton = std::dynamic_pointer_cast<scene2::Button>(layer->getChildByName("restart"));
    _exitButton = std::dynamic_pointer_cast<scene2::Button>(layer->getChildByName("exit"));
    _settingButton = std::dynamic_pointer_cast<scene2::Button>(layer->getChildByName("setting"));
    _hpbar = layer->getChildByName("hpbar");

    if (_resumeButton) {
        _resumeButton->addListener([](const std::string& name, bool down) {
            if (down) CULog("Resume pressed");
        });
        _resumeButton->activate();
    }

    if (_restartButton) {
        _restartButton->addListener([](const std::string& name, bool down) {
            if (down) CULog("Restart pressed");
        });
        _restartButton->activate();
    }

    if (_exitButton) {
        _exitButton->addListener([](const std::string& name, bool down) {
            if (down) CULog("Exit pressed");
        });
        _exitButton->activate();
    }

    if (_settingButton) {
        _settingButton->addListener([](const std::string& name, bool down) {
            if (down) CULog("Settings pressed");
        });
        _settingButton->activate();
    }

    if (_hpbar) {
        float segmentWidth = 42.0f;
        float segmentHeight = 61.8f;
        float halfSegmentHeight = 52.7f;
        float segmentSpacing = 8.4f;
        float startX = 15.0f;
        float startY = 34.0f;

        std::shared_ptr<cugl::graphics::Texture> texture = _assets->get<cugl::graphics::Texture>("hp_segment");
        std::shared_ptr<cugl::graphics::Texture> halfTexture = _assets->get<cugl::graphics::Texture>("half_hp_segment");
        for (int i = 0; i < 5; ++i) {
            std::shared_ptr<scene2::PolygonNode> segment = scene2::PolygonNode::allocWithTexture(texture);
            segment->setAnchor(Vec2::ANCHOR_TOP_LEFT);
            segment->setPosition(startX + i * (segmentWidth + segmentSpacing), segmentHeight + startY);
            segment->setContentSize(Size(segmentWidth, segmentHeight));
            segment->setVisible(true);
            _hpbar->addChild(segment);
            _hpSegments.push_back(segment);
            
            std::shared_ptr<scene2::PolygonNode> half = scene2::PolygonNode::allocWithTexture(halfTexture);
            half->setAnchor(Vec2::ANCHOR_TOP_LEFT);
            half->setPosition(startX + i * (segmentWidth + segmentSpacing), segmentHeight + 31.5f);
            half->setContentSize(Size(segmentWidth, halfSegmentHeight));
            half->setVisible(false);
            _hpbar->addChild(half);
            _hpHalfSegments.push_back(half);
        }
    }

    Application::get()->setClearColor(Color4f::CORNFLOWER);
    return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GBPauseMenu::dispose() {
    _resumeButton = nullptr;
    _restartButton = nullptr;
    _exitButton = nullptr;
    _settingButton = nullptr;
    _assets = nullptr;

    removeAllChildren();
}

/**
 * Sets whether the scene is currently active
 *
 * @param value whether the scene is currently active
 */
void GBPauseMenu::setActive(bool value) {
    _active = value;

    if (_resumeButton) {
        value ? _resumeButton->activate() : _resumeButton->deactivate();
    }
    if (_restartButton) {
        value ? _restartButton->activate() : _restartButton->deactivate();
    }
    if (_exitButton) {
        value ? _exitButton->activate() : _exitButton->deactivate();
    }
    if (_settingButton) {
        value ? _settingButton->activate() : _settingButton->deactivate();
    }

    setVisible(value);
}

void GBPauseMenu::setHP(int hp) {
    hp = std::max(0, std::min(hp, 100));

    int filled = hp / 20;
    bool showHalf = (hp % 20) == 10;

    for (int i = 0; i < 5; ++i) {
        _hpSegments[i]->setVisible(i < filled);
        _hpHalfSegments[i]->setVisible(i == filled && showHalf);
    }
    
}
