#include "GBLevelSelectScene.h"

LevelSelectScene::LevelSelectScene() : _ui(nullptr), _scene_to_load(0), _ui_switch("") {}

void LevelSelectScene::dispose()
{
	_ui = nullptr;
}

bool LevelSelectScene::init(const std::shared_ptr<AssetManager>& assets, int highestPlayableLevel, bool firsttime)
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
    if (!Scene2::init()) {
        return false;
    }

    _ui = GBMenuUI::alloc(_assets, firsttime);
    if (_ui != nullptr) {
        _ui->setLevel1Callback([this]() { _scene_to_load = 1; });
        _ui->setLevel2Callback([this]() { _scene_to_load = 2; });
        _ui->setLevel3Callback([this]() { _scene_to_load = 3; });
        _ui->setLevel4Callback([this]() { _scene_to_load = 3; });
        _ui->setLevel5Callback([this]() { _scene_to_load = 3; });

        _ui->setStartCallback([this]() { _ui_switch = "levelselect"; CULog("Start pressed");});
        _ui->setInfoCallback([this]() { _ui_switch = "info"; CULog("Info pressed");});
        _ui->setHomeSettingCallback([this]() { _ui_switch = "setting"; CULog("Home Setting pressed");});
        _ui->setHomeCallback([this]() { _ui_switch = "home"; CULog("Home pressed");});
        addChild(_ui);
        _ui->setHighestPlayable(highestPlayableLevel);
    }
    return true;
}

void LevelSelectScene::update(float dt) {
    if (_ui_switch == "levelselect") {
        _ui->showHome(false);
//        _ui->showLevelSelectionHead(true);
        _ui->showLevelSelection1(true);
    }
    else if (_ui_switch == "info") {
        _ui->showHome(false);
//        _ui->showInfo(true);
    }
    else if (_ui_switch == "setting") {
        _ui->showHome(false);
//        _ui->showHomeSetting(true);
    }
    else if (_ui_switch == "home") {
//        _ui->showLevelSelectionHead(false);
        _ui->showLevelSelection1(false);
        _ui->showLevelSelection2(false);
        _ui->showLevelSelection3(false);
        _ui->showLevelSelection4(false);
//        _ui->showInfo(false);
//        _ui->showHomeSetting(false);
        _ui->showHome(true);
    }

    // Reset label after acting
    _ui_switch = "";
}

int LevelSelectScene::sceneToLoad()
{
    return _scene_to_load;
}
