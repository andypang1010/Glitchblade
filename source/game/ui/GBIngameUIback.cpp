////
////  GBIngameUI.cpp
////
////  This module handles the in-game UI, including pause button and HP bar.
////  Based on:
////
////  UIButtonScene.cpp
////  UI Demo
////
////  This module shows off a basic button.  The button is backed by a NinePatch
////  so it can resize dynamically.  It changes the text on a click.
////
////  Author: Walker White
////  Version: 1/20/18
////
//#include "GBIngameUI.h"
//
//using namespace cugl;
//
///** This is adjusted by screen aspect ratio to get the height */
//#define SCENE_WIDTH 1248
//
///**
// * Initializes the controller contents, and starts the game
// *
// * The constructor does not allocate any objects or memory.  This allows
// * us to have a non-pointer reference to this controller, reducing our
// * memory allocation.  Instead, allocation happens in this method.
// *
// * @param assets    The (loaded) assets for this game mode
// *
// * @return true if the controller is initialized properly, false otherwise.
// */
//bool GBIngameUI::init(const std::shared_ptr<AssetManager>& assets) {
//    if (assets == nullptr) {
//        return false;
//    } else if (!SceneNode::init()) {
//        return false;
//    }
//    
//    _assets = assets;
//    
//    setContentSize(Size(1248, 576));
//    
//    auto layer = assets->get<scene2::SceneNode>("hud");
//    
//    if (layer == nullptr) {
//        return false;
//    }
//    
//    layer->setContentSize(Size(1248, 576));
//    layer->doLayout(); // This rearranges the children to fit the screen
//    addChild(layer);
//    setActive(true);
//    _pauseButton = std::dynamic_pointer_cast<scene2::Button>(layer->getChildByName("pause"));
//    
//    if (_pauseButton) {
//        _pauseButton->addListener([this](const std::string& name, bool pressed) {
//            if (pressed) {
//                CULog("Pause!");
//            }
//        });
//
//        if (_active) {
//            _pauseButton->activate();
//        }
//    }
//    
//    for (int i = 1; i <= 5; ++i) {
//        std::string fullName = "hp_full_segment_" + std::to_string(i);
//        std::string halfName = "hp_half_segment_" + std::to_string(i);
//
//        auto full = layer->getChildByName<scene2::PolygonNode>(fullName);
//        auto half = layer->getChildByName<scene2::PolygonNode>(halfName);
//
//        if (full != nullptr) {
//            _hpSegments.push_back(full);
//        }
//        if (half != nullptr) {
//            _hpHalfSegments.push_back(half);
//        }
//    }
//
//    _screenOffset = getPosition();
//    
//    // XNA nostalgia
//    Application::get()->setClearColor(Color4f::CORNFLOWER);
//    return true;
//}
//
///**
// * Disposes of all (non-static) resources allocated to this mode.
// */
//void GBIngameUI::dispose() {
//    _pauseButton = nullptr;
//    _assets = nullptr;
//
//    removeAllChildren();
//}
//
///**
// * Sets whether the scene is currently active
// *
// * @param value whether the scene is currently active
// */
//void GBIngameUI::setActive(bool value) {
//    _active = value;
//    if (_pauseButton) {
//        if (value && !_pauseButton->isActive()) {
//            _pauseButton->activate();
//        } else if (!value && _pauseButton->isActive()) {
//            _pauseButton->deactivate();
//        }
//    }
//    
//    setVisible(value);
//}
//
//void GBIngameUI::setHP(int hp) {
//    hp = std::max(0, std::min(hp, 100));
//
//    int filled = hp / 20;
//    bool showHalf = (hp % 20) == 10;
//
//    for (int i = 0; i < 5; ++i) {
//        _hpSegments[i]->setVisible(i < filled);
//        _hpHalfSegments[i]->setVisible(i == filled && showHalf);
//    }
//    
//}
//
