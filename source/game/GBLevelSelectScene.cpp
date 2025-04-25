#include "GBLevelSelectScene.h"

LevelSelectScene::LevelSelectScene() : _ui(nullptr)
{
}

void LevelSelectScene::dispose()
{
	_ui = nullptr;
}

bool LevelSelectScene::init(const std::shared_ptr<AssetManager>& assets)
{
    if (assets == nullptr) {
        return false;
    }
    _assets = assets;

    // prepare constants
    std::shared_ptr<JsonReader> constants_reader = JsonReader::allocWithAsset("json/constants.json");
    _constantsJSON = constants_reader->readJson();
    if (_constantsJSON == nullptr) {
        CULog("Failed to load constants.json");
        return false;
    }

    std::shared_ptr<JsonValue> sceneJ = _constantsJSON->get("scene");
    if (!Scene2::initWithHint(Size(sceneJ->getInt("width"), sceneJ->getInt("height")))) {
        return false;
    }

    _ui = LevelSelectUI::alloc(_assets);
    if (_ui != nullptr) {
        CULog("YEAH SETTING CALLBACKS");
        _ui->setLevel1Callback([this]() {
            _scene_to_load = "Level 1";
            });
        _ui->setLevel2Callback([this]() {
            _scene_to_load = "Level 2";
            });
        _ui->setLevel3Callback([this]() {
            _scene_to_load = "Level 3";
            });
        addChild(_ui);
        _ui->setVisible(true);
        _ui->showMenu(true);
    }

    return true;
}

std::string LevelSelectScene::sceneToLoad()
{
    return _scene_to_load;
}
