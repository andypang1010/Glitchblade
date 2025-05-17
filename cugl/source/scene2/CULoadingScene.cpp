//
//  CULoadingScene.cpp
//  Cornell University Game Library (CUGL)
//
//  This module creates a generic loading scene for games. For years we copied
//  this class from demo to demo without ever making it part of the engine.
//  At first it was because it was because the loading scene depends on specific
//  assets, and we do not want to include hard assets like images and fonts in
//  the engine itself. But with the addition of JSON scene loading and Figma
//  support, this made less sense.
//
//  With that said, this loading scene is still very simple. It has no animation
//  beyond a simple progress bar. Students often replace this class when making
//  their own games.
//
//  This class uses our standard shared-pointer architecture.
//
//  1. The constructor does not perform any initialization; it just sets all
//     attributes to their defaults.
//
//  2. All initialization takes place via init methods, which can fail if an
//     object is initialized more than once.
//
//  3. All allocation takes place via static constructors which return a shared
//     pointer.
//
//  CUGL MIT License:
//      This software is provided 'as-is', without any express or implied
//      warranty.  In no event will the authors be held liable for any damages
//      arising from the use of this software.
//
//      Permission is granted to anyone to use this software for any purpose,
//      including commercial applications, and to alter it and redistribute it
//      freely, subject to the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not
//      be misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
//
//  Author: Walker White
//  Version: 7/3/24 (CUGL 3.0 reorganization)
//
#include <cugl/scene2/CULoadingScene.h>
#include <cugl/graphics/loaders/CUTextureLoader.h>
#include <cugl/graphics/loaders/CUFontLoader.h>
#include <cugl/scene2/CUScene2Loader.h>
#include <cugl/core/assets/CUWidgetLoader.h>

using namespace cugl;
using namespace cugl::scene2;
using namespace cugl::graphics;

/**
 * Initializes a loading scene with the given scene and directory.
 *
 * This class will create its own {@link AssetManager}, which can be
 * accessed via {@link #getAssetManager}. This asset manager will only
 * attach loaders for {@link Font}, {@link Texture}, {@link scene2::SceneNode}
 * and {@link WidgetValue}.
 *
 * The string scene should be a path to a JSON file that defines the scene
 * graph for this loading scene. This file will be loaded synchronously, so
 * it should be lightweight. The scene must include a {@link scene2::SceneNode}
 * named "load". This node must have at least four children:
 *
 *     - "load.before": The scene to display while loading is in progress
 *     - "load.after": The scene to display when the loading is complete
 *     - "load.bar": A {@link ProgressBar} for showing the loading progress
 *     - "load.play" A play {@link Button} for the user to start the game
 *
 * The string directory is the asset directory to be loaded asynchronously
 * by this scene. Loading will commence after a call to {@link #start}. The
 * progress on this directory can be monitored via {@link #getProgress}.
 *
 * @param scene     A JSON file with the scene graph for this scene
 * @param directory The asset directory to load asynchronously
 *
 * @return true if the scene is initialized properly, false otherwise.
 */
bool LoadingScene::init(const std::string scene, const std::string directory) {
    _assets = AssetManager::alloc();
    if (_assets == nullptr || !_assets->loadDirectory(scene) ) {
        return false;
    }
    
    _assets->attach<Font>(FontLoader::alloc()->getHook());
    _assets->attach<Texture>(TextureLoader::alloc()->getHook());
    _assets->attach<WidgetValue>(WidgetLoader::alloc()->getHook());
    _assets->attach<scene2::SceneNode>(Scene2Loader::alloc()->getHook());
    
    return init(_assets,directory);
}

/**
 * Initializes a loading scene with the given asset manager and directory.
 *
 * The asset manager must already contain the scene graph used by this
 * scene. The scene must include a {@link scene2::SceneNode} named
 * "load". This node must have at least four children:
 *
 *     - "load.before": The scene to display while loading is in progress
 *     - "load.after": The scene to display when the loading is complete
 *     - "load.bar": A {@link ProgressBar} for showing the loading progress
 *     - "load.play" A play {@link Button} for the user to start the game
 *
 * The string directory is the asset directory to be loaded asynchronously
 * by this scene. The progress on this directory can be monitored via
 * {@link #getProgress}.
 *
 * @param asset     A previously initialized asset manager
 * @param directory The asset directory to load asynchronously
 *
 * @return true if the scene is initialized properly, false otherwise.
 */
bool LoadingScene::init(const std::shared_ptr<cugl::AssetManager>& assets,
                        const std::string directory) {
    // First make sure we have the scene
    auto layer = assets->get<scene2::SceneNode>("load");
    if (layer == nullptr) {
        CUAssertLog(false,"Missing \"load\" in scene specification");
        return false;
    }
    _before  = assets->get<scene2::SceneNode>("load.before");
    _after = assets->get<scene2::SceneNode>("load.after");
    _bar = std::dynamic_pointer_cast<scene2::ProgressBar>(assets->get<scene2::SceneNode>("load.bar"));
    _button = std::dynamic_pointer_cast<scene2::Button>(assets->get<scene2::SceneNode>("load.play"));
    if (_bar == nullptr) {
        CUAssertLog(false,"Missing progress bar in scene specification");
        return false;
    }
    if (_button == nullptr) {
        CUAssertLog(false,"Missing play button in scene specification");
        return false;
    }
    
    // Check to see if we have a size hint
    if (layer->getJSON()->has("size")) {
        if (!Scene2::initWithHint(layer->getContentSize())) {
            return false;
        }
    } else if (!Scene2::init()) {
        return false;
    }

    // Rearrange the children to fit the screen
    layer->setContentSize(_size);
    layer->doLayout();
    
    // Add a listener for the button
    _button->addListener([=,this](const std::string& name, bool down) {
        this->_active = down;
    });
    
    // Ensure the correct visibility
    if (_before) {
        _before->setVisible(true);
    }
    if (_after) {
        _after->setVisible(false);
    }
    _bar->setVisible(true);
    _button->setVisible(false);

    //Application::get()->setClearColor(Color4(192,192,192,255));
    addChild(layer);
    
    _assets = assets;
    _directory = directory;
    return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void LoadingScene::dispose() {
    // Deactivate the button (platform dependent)
    if (isPending()) {
        _button->deactivate();
    }
    _assets = nullptr;
    _before = nullptr;
    _after  = nullptr;
    _button = nullptr;
    _bar = nullptr;
    _progress = 0.0f;
    _completed = false;
}

/**
 * Starts the loading progress for this scene
 *
 * This method has no affect if loading is already in progress.
 */
void LoadingScene::start() {
    if (_started) {
        return;
    }
    _started = true;
    //_assets->loadDirectoryAsync(_directory, nullptr);
    _assets->loadDirectoryAsync(_directory, [this](const std::string& key, bool success){
        if (success) {
            auto test = _assets->progress();
        }
    });
}

/**
 * Updates the loading scene progress.
 *
 * This method queries the asset manager to update the progress bar amount.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void LoadingScene::update(float progress) {
    if (_progress < 1) {
        _progress = _assets->progress();
        if (_progress >= 1) {
            _progress = 1.0f;
            if (_before) {
                _before->setVisible(false);
            }
            if (_after) {
                _after->setVisible(true);
            }
            _bar->setVisible(false);
            _button->setVisible(true);
            _button->activate();
        }
        _bar->setProgress(_progress);
    }
}
