//  GBAudio.h

#include "../core/GBAudio.h"

using namespace cugl;
using namespace cugl::audio;

std::shared_ptr<cugl::JsonValue> AudioHelper::_fxJSON = nullptr;
std::shared_ptr<cugl::AssetManager> AudioHelper::_assets = nullptr;

void AudioHelper::init(std::shared_ptr<JsonValue> fxJSON, std::shared_ptr<AssetManager> assets) {
    CULog("initing audiohelper");
    _fxJSON = fxJSON;
    _assets = assets;
}

void AudioHelper::play_sfx(const std::string& fx_name) {
    if (!_fxJSON || !_assets) {
        CULog("audio helper not inited properly");
        return;
    }
    float fx_volume = _fxJSON->getFloat("volume");
    
    int version = rand() % _fxJSON->getInt(fx_name) + 1 ;
    std::string mapped_name = fx_name + ("_" + std::to_string(version));
    
    std::shared_ptr<Sound> source = _assets->get<Sound>(mapped_name);

    if (source) {
        AudioEngine::get()->play(mapped_name, source, false, fx_volume);
    }
    else{
        CULog("source is null in audiohelper");
    }
}

void AudioHelper::dispose() {
    // release shared ptr refs
    _fxJSON.reset();
    _assets.reset();
}
