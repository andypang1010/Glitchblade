// GBAudio.h

#ifndef __GB_Audio_H__
#define __GB_Audio_H__
#include <cugl/cugl.h>

using namespace cugl;
using namespace cugl::audio;

class AudioHelper {
public:
    static void init(std::shared_ptr<JsonValue> fxJSON, std::shared_ptr<AssetManager> assets);

    static void playSfx(const std::string& fx_name);

    
    static void dispose();

private:
    static std::shared_ptr<JsonValue> _fxJSON;
    static std::shared_ptr<AssetManager> _assets;
    AudioHelper() = delete;// Prevent creating instances
};

#endif
