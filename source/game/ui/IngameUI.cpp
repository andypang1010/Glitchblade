////
////  GBIngameUI.cpp
////
////  This module handles all in-game UI, including HUD, pause menu, settings, and win/lose page.
////
////  Author: Vince Qian
////
////  Reference:
////  UIButtonScene.cpp
////  UI Demo by Walker White (Version: 1/20/18)
////
//#include "IngameUI.h"
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
//    auto layer = assets->get<scene2::SceneNode>("ingamescene");
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
//    _hpbar = layer->getChildByName("hpbar");
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
//    if (_hpbar) {
//        float segmentWidth = 35.0f;
//        float segmentHeight = 55.0f;
//        float halfSegmentHeight = 46.9f;
//        float segmentSpacing = 7.0f;
//        float startX = 15.0f;
//        float startY = 38.0f;
//
//        std::shared_ptr<cugl::graphics::Texture> texture = _assets->get<cugl::graphics::Texture>("hp_segment");
//        texture->setName("hp_segment");
//        std::shared_ptr<cugl::graphics::Texture> halfTexture = _assets->get<cugl::graphics::Texture>("half_hp_segment");
//        texture->setName("hp_segment");
//        for (int i = 0; i < 5; ++i) {
//            std::shared_ptr<scene2::PolygonNode> segment = scene2::PolygonNode::allocWithTexture(texture);
//            segment->setName("segment");
//            segment->setAnchor(Vec2::ANCHOR_TOP_LEFT);
//            segment->setPosition(startX + i * (segmentWidth + segmentSpacing), segmentHeight + startY);
//            segment->setContentSize(Size(segmentWidth, segmentHeight));
//            segment->setVisible(true);
//            _hpbar->addChild(segment);
//            _hpSegments.push_back(segment);
//            
//            std::shared_ptr<scene2::PolygonNode> half = scene2::PolygonNode::allocWithTexture(halfTexture);
//            half->setName("half_segment");
//            half->setAnchor(Vec2::ANCHOR_TOP_LEFT);
//            half->setPosition(startX + i * (segmentWidth + segmentSpacing), segmentHeight + 35.0f);
//            half->setContentSize(Size(segmentWidth, halfSegmentHeight));
//            half->setVisible(false);
//            _hpbar->addChild(half);
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
//    _hpbar = nullptr;
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
