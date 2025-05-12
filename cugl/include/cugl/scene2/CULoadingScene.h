//
//  CULoadingScene.h
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
#ifndef __CU_LOADING_SCENE_H__
#define __CU_LOADING_SCENE_H__
#include <cugl/core/assets/CUAssetManager.h>
#include <cugl/scene2/CUScene2.h>
#include <cugl/scene2/CUProgressBar.h>
#include <cugl/scene2/CUButton.h>

namespace cugl {

    /**
     * The classes to construct a 2-d scene graph.
     *
     * Even though this is an optional package, this is one of the core features
     * of CUGL. These classes provide basic UI support (including limited Figma)
     * support. Any 2-d game will make extensive use of these classes.  And
     * even 3-d games may use these classes for the HUD overlay.
     */
    namespace scene2 {
    
/**
 * This class is a simple loading screen for asychronous asset loading.
 *
 * This class will either create its own {@link AssetManager}, or it can be
 * assigned on. In the latter case, it needs a json file defining the assets
 * for the initial loading screen. In the former, the asset manager should
 * come preloaded with these assets. To properly display to the screen,
 * these assets must include a {@link scene2::SceneNode} named "load". This
 * node must have at least four children:
 *
 *     - "load.before": The scene to display while loading is in progress
 *     - "load.after": The scene to display when the loading is complete
 *     - "load.bar": A {@link ProgressBar} for showing the loading progress
 *     - "load.play" A play {@link Button} for the user to start the game
 *
 * When the play button is pressed, this scene is deactivated, indicating to
 * the application that it is time to switch scenes.
 *
 * In addition to these assets, the loading scene will take the take of an
 * asset directory. This is a JSON file defining the assets that should be
 * loaded asynchronously by this loading scene. Accessing the asset manager
 * with {@link #getAssetManager} gives access to these assets.
 */
class LoadingScene : public Scene2 {
protected:
    /** The asset manager for loading. */
    std::shared_ptr<cugl::AssetManager> _assets;
    /** The asset directory reference */
    std::string _directory;
    
    /** The scene during loading */
    std::shared_ptr<cugl::scene2::SceneNode>  _before;
    /** The scene during when complete */
    std::shared_ptr<cugl::scene2::SceneNode>  _after;
    /** The "play" button */
    std::shared_ptr<cugl::scene2::Button>     _button;
    /** The animated progress bar */
    std::shared_ptr<cugl::scene2::ProgressBar>  _bar;
    
    /** The progress displayed on the screen */
    float _progress;
    /** Whether or not the player has pressed play to continue */
    bool  _completed;
    /** Whether or not the asset loader has started loading */
    bool  _started;

public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new loading scene with the default values.
     *
     * This constructor does not allocate any objects or start the scene.
     * This allows us to use the object without a heap pointer.
     */
    LoadingScene() : Scene2(), _progress(0.0f), _completed(false), _started(false) {}
    
    /**
     * Deletes this scene, disposing all resources.
     */
    ~LoadingScene() { dispose(); }
    
    /**
     * Disposes all of the resources used by this sceene.
     *
     * A disposed node can be safely reinitialized. The scene graph owned by
     * this scene will be released, as well as the asset manager. They will be
     * deleted if no other object owns them.
     */
    void dispose();
    
    /**
     * Initializes a loading scene with the given scene and directory.
     *
     * This class will create its own {@link AssetManager}, which can be
     * accessed via {@link #getAssetManager}. This asset manager will only
     * attach loaders for {@link graphics::Font}, {@link graphics::Texture},
     * {@link scene2::SceneNode} and {@link WidgetValue}.
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
     * by this scene. The progress on this directory can be monitored via
     * {@link #getProgress}.
     *
     * @param scene     A JSON file with the scene graph for this scene
     * @param directory The asset directory to load asynchronously
     *
     * @return true if the scene is initialized properly, false otherwise.
     */
    bool init(const std::string scene, const std::string directory);
    
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
     * @param manager   A previously initialized asset manager
     * @param directory The asset directory to load asynchronously
     *
     * @return true if the scene is initialized properly, false otherwise.
     */
    bool init(const std::shared_ptr<cugl::AssetManager>& manager,
              const std::string directory);
    
    /**
     * Returns a newly allocated loading scene with the given scene and directory.
     *
     * This class will create its own {@link AssetManager}, which can be
     * accessed via {@link #getAssetManager}. This asset manager will only
     * attach loaders for {@link graphics::Font}, {@link graphics::Texture},
     * {@link scene2::SceneNode} and {@link WidgetValue}.
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
     * by this scene. The progress on this directory can be monitored via
     * {@link #getProgress}.
     *
     * @param scene     A JSON file with the scene graph for this scene
     * @param directory The asset directory to load asynchronously
     *
     * @return a newly allocated loading scene with the given scene and directory.
     */
    static std::shared_ptr<LoadingScene> alloc(const std::string scene,
                                               const std::string directory) {
        std::shared_ptr<LoadingScene> result = std::make_shared<LoadingScene>();
        return (result->init(scene,directory) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated loading scene with the given asset manager and directory.
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
     * @param manager   A previously initialized asset manager
     * @param directory The asset directory to load asynchronously
     *
     * @return a newly allocated loading scene with the given asset manager and directory.
     */
    static std::shared_ptr<LoadingScene> alloc(const std::shared_ptr<cugl::AssetManager>& manager,
                                               const std::string directory) {
        std::shared_ptr<LoadingScene> result = std::make_shared<LoadingScene>();
        return (result->init(manager,directory) ? result : nullptr);
    }
    
    
#pragma mark -
#pragma mark Progress Monitoring
    /**
     * Returns the asset manager for this loading scene
     *
     * @returns the asset manager for this loading scene
     */
    std::shared_ptr<cugl::AssetManager> getAssetManager() const { return _assets; }
    
    /**
     * Starts the loading progress for this scene
     *
     * This method has no affect if loading is already in progress.
     */
    void start();
    
    /**
     * Updates the loading scene progress.
     *
     * This method queries the asset manager to update the progress bar amount.
     *
     * @param timestep  The amount of time (in seconds) since the last frame
     */
    void update(float timestep);

    /**
     * Returns the current progress of this this loading scene.
     *
     * The value is in the range [0,1] where 0 means no progress and 1 means
     * that loading has completed.
     *
     * @return the current progress of this this loading scene.
     */
    float getProgress( ) const { return _progress; }

    /**
     * Returns true if loading is complete, but the player has not pressed play
     *
     * @return true if loading is complete, but the player has not pressed play
     */
    bool isPending( ) const { return _button != nullptr && _button->isVisible(); }
    
    /**
     * Returns true if loading is complete, and the player has pressed play
     *
     * @return true if loading is complete, and the player has pressed play
     */
    bool isComplete( ) const { return _completed; }
};
    }

}

#endif /* __CU_LOADING_SCENE_H__ */
