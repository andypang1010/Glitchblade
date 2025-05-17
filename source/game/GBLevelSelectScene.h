#pragma once

#include <cugl/cugl.h>
#include "../core/GBInput.h"
#include "ui/GBLevelSelectUI.h"
#include "ui/GBMenuUI.h"

using namespace cugl;

class LevelSelectScene : public scene2::Scene2 {
protected:
    /** The asset manager for this game mode. */
    std::shared_ptr<AssetManager> _assets;
    std::shared_ptr<JsonValue> _constantsJSON;

    // UI
    /** Ingame UI */
    std::shared_ptr<GBMenuUI> _ui;
    bool _isPaused = false;

    int _scene_to_load = 0;
    int _ui_switch;

public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new game world with the default values.
     *
     * This constructor does not allocate any objects or start the controller.
     * This allows us to use a controller without a heap pointer.
     */
    LevelSelectScene();

    /**
     * Disposes of all (non-static) resources allocated to this mode.
     *
     * This method is different from dispose() in that it ALSO shuts off any
     * static resources, like the input controller.
     */
    ~LevelSelectScene() { dispose(); }

    /**
     * Disposes of all (non-static) resources allocated to this mode.
     */
    void dispose();

    bool init(const std::shared_ptr<AssetManager>& assets, int highestPlayableLevel, int scene);
    void update(float dt);

#pragma mark -
#pragma mark State Info
    int sceneToLoad();
};
