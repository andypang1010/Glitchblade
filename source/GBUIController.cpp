// Just a try, not correct.
//
//#include "GBUIController.h"
//#include "GBIngameUI.h"
//
//using namespace cugl;
//
//bool GBUIController::init(const std::shared_ptr<AssetManager>& assets) {
//    _assets = assets;
//
//    _ingameUI = GBIngameUI::alloc(assets);
//    if (_ingameUI == nullptr) {
//        CULog("Failed to initialize InGame UI");
//        return false;
//    }
//    _ingameUI->setActive(true);
//
//    return true;
//}
//
//void GBUIController::dispose() {
//    if (_ingameUI) {
//        _ingameUI->dispose();
//        _ingameUI = nullptr;
//    }
//    _assets = nullptr;
//}
//
//void GBUIController::update(float dt) {
//    if (_ingameUI && _ingameUI->isActive()) {
//        _ingameUI->update(dt);
//    }
//}
//
//void GBUIController::setActive(bool value) {
//    if (_ingameUI) {
//        _ingameUI->setActive(value);
//    }
//}
//
//void GBUIController::setHP(int hp) {
//    if (_ingameUI && _ingameUI->isActive()) {
//        _ingameUI->setHP(hp);
//    }
//}
//
//bool GBUIController::isActive() const {
//    return _ingameUI && _ingameUI->isActive();
//}
//
//void GBUIController::render(const std::shared_ptr<cugl::graphics::SpriteBatch>& batch) {
//    if (_ingameUI && _ingameUI->isActive()) {
//        _ingameUI->render();
//    }
//}

#include "GBUIController.h"

using namespace cugl;

bool GBUIController::init(const std::shared_ptr<AssetManager>& assets) {
    if (!Scene2::init()) return false;

    _assets = assets;

    _ingameUI = GBIngameUI::alloc(assets);
        if (_ingameUI != nullptr) {
            // 这才是可以 setContentSize 的对象
            Size screen = Application::get()->getDisplaySize();
            _ingameUI->setContentSize(screen);
            _ingameUI->doLayout();

            addChild(_ingameUI);
            _ingameUI->setActive(true);
        }


    return true;
}

void GBUIController::render(const std::shared_ptr<cugl::graphics::SpriteBatch>& batch) {
    if (_ingameUI && _ingameUI->isActive()) {
        _ingameUI->render(batch);
    }
}


void GBUIController::dispose() {
    if (_ingameUI) {
        _ingameUI->removeFromParent();
        _ingameUI = nullptr;
    }
    _assets = nullptr;
    Scene2::dispose();
}

void GBUIController::update(float dt) {
    if (_ingameUI && _ingameUI->isActive()) {
        // 暂时 IngameUI 不需要每帧更新，你可以加你自己的逻辑
    }
}

void GBUIController::setIngameUIActive(bool value) {
    if (_ingameUI) {
        _ingameUI->setActive(value);
    }
}

void GBUIController::setHP(int hp) {
    if (_ingameUI) {
        _ingameUI->setHP(hp);
    }
}
