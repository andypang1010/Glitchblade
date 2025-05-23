#include "GBLevelSelectScene.h"

using namespace cugl::audio;

LevelSelectScene::LevelSelectScene() : _ui(nullptr), _scene_to_load(0) {}

void LevelSelectScene::dispose()
{
	_ui = nullptr;
}

bool LevelSelectScene::init(const std::shared_ptr<AssetManager>& assets, int highestPlayableLevel, int scene)
{
    if (assets == nullptr) {
        return false;
    }
    _assets = assets;
    _ui_switch = scene;
    _setting = false;

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

    _ui = GBMenuUI::alloc(_assets, scene);
    if (_ui != nullptr) {
        _ui->setLevel1Callback([this]() { _scene_to_load = 1; });
        _ui->setLevel2Callback([this]() { _scene_to_load = 2; });
        _ui->setLevel3Callback([this]() { _scene_to_load = 3; });
        _ui->setLevel4Callback([this]() { _scene_to_load = 4; });
        _ui->setLevel5Callback([this]() { _scene_to_load = 5; });

        _ui->setStartCallback([this, highestPlayableLevel]() { _ui_switch = highestPlayableLevel; CULog("Start pressed");});
        _ui->setHomeCallback([this]() { _ui_switch = 0; CULog("Home pressed");});
        _ui->setInfoCallback([this]() { _ui_switch = -1; CULog("Info pressed");});
        _ui->setHomeSettingCallback([this]() { _setting = true;; CULog("Home Setting pressed");});
        _ui->setBackCallback([this]() { _setting = false; CULog("Setting back pressed");});
        _ui->setPreviousSceneCallback([this]() { _ui_switch -= 1; CULog("Previous Scene pressed %d", _ui_switch);});
        _ui->setNextSceneCallback([this]() { _ui_switch += 1; CULog("Next Scene pressed %d", _ui_switch);});
        addChild(_ui);
        _ui->setHighestPlayable(highestPlayableLevel);
    }
    
    //play menu music
    std::shared_ptr<JsonValue> musicJ = _constantsJSON->get("audio")->get("music");
    std::shared_ptr<Sound> source = _assets->get<Sound>("menu");
    AudioEngine::get()->getMusicQueue()->play(source, true, 1.0);
    return true;
}

void LevelSelectScene::update(float dt) {
    if (_setting) {
        _ui->showHomeSetting(true);
        _ui->showLevelSelectionHead(false);
        _ui->showLevelSelection1(false);
        _ui->showLevelSelection2(false);
        _ui->showLevelSelection3(false);
        _ui->showLevelSelection4(false);
        _ui->showLevelSelection5(false);
        _ui->showInfo(false);
        _ui->showHome(false);
    } else {
        _ui->showHomeSetting(false);
        if (_ui_switch == 1) {
            _ui->showHome(false);
            _ui->showLevelSelectionHead(true, false, true);
            _ui->showLevelSelection1(true);
            _ui->showLevelSelection2(false);
        }
        else if (_ui_switch == 2) {
            _ui->showHome(false);
            _ui->showLevelSelectionHead(true);
            _ui->showLevelSelection1(false);
            _ui->showLevelSelection2(true);
            _ui->showLevelSelection3(false);
        }
        else if (_ui_switch == 3) {
            _ui->showHome(false);
            _ui->showLevelSelectionHead(true);
            _ui->showLevelSelection2(false);
            _ui->showLevelSelection3(true);
            _ui->showLevelSelection4(false);
        }
        else if (_ui_switch == 4) {
            _ui->showHome(false);
            _ui->showLevelSelectionHead(true);
            _ui->showLevelSelection3(false);
            _ui->showLevelSelection4(true);
            _ui->showLevelSelection5(false);
        }
        else if (_ui_switch == 5) {
            _ui->showHome(false);
            _ui->showLevelSelectionHead(true, true, false);
            _ui->showLevelSelection4(false);
            _ui->showLevelSelection5(true);
        }
        else if (_ui_switch == -1) {
            _ui->showHome(false);
            _ui->showInfo(true);
        }
        else if (_ui_switch == 0) {
            _ui->showLevelSelectionHead(false);
            _ui->showLevelSelection1(false);
            _ui->showLevelSelection2(false);
            _ui->showLevelSelection3(false);
            _ui->showLevelSelection4(false);
            _ui->showLevelSelection5(false);
            _ui->showInfo(false);
            _ui->showHomeSetting(false);
            _ui->showHome(true);
        }
    }
}

int LevelSelectScene::sceneToLoad()
{
    return _scene_to_load;
}
