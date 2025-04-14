#pragma once

#include <cugl/cugl.h>
#include "GBSceneInterface.h"

using namespace cugl;

class LevelSelectScene : public scene2::Scene2, public GBSceneInterface {
protected:
    /** The asset manager for loading scene assets */
    std::shared_ptr<cugl::AssetManager> _assets;
    std::shared_ptr<JsonValue> _constantsJSON;

    // Root node
    std::shared_ptr<cugl::scene2::SceneNode> _root;

    // PolygonNodes for background sections
    std::shared_ptr<cugl::scene2::PolygonNode> _bgLeft;
    std::shared_ptr<cugl::scene2::PolygonNode> _bgCenter;
    std::shared_ptr<cugl::scene2::PolygonNode> _bgRight;

    // Buttons for for each selectable level
    std::shared_ptr<cugl::scene2::Button> _btnLeft;
    std::shared_ptr<cugl::scene2::Button> _btnCenter;
    std::shared_ptr<cugl::scene2::Button> _btnRight;

    std::shared_ptr<scene2::SceneNode> _rootNode;

    // 0 = do not swap; 1 = swap to level select; 2 = swap to game scene
    int _swapSceneSignal = 0;

    float _scale;
    Vec2 _offset;

    /**
     * Stores which level the user has selected.
     * If 0, no level has been chosen yet.
     */
    int  _selectedLevel;

public:
#pragma mark -
#pragma mark Constructors
    LevelSelectScene() : Scene2(), _selectedLevel(0) {}

    ~LevelSelectScene() { dispose(); }

    /**
     * Initializes this scene from the provided asset manager.
     *
     * @param assets The (already loaded) assets for this scene
     * @return true if scene initialization was successful
     */
    bool init(const std::shared_ptr<cugl::AssetManager>& assets);

    bool init(const std::shared_ptr<AssetManager>& assets, const Rect& rect);

    /**
     * Disposes of all (non-static) scene resources.
     */
    void dispose();

    int getSwapSignal();

    void setSwapSignal(int signal);

#pragma mark -
#pragma mark Scene State

    /**
     * Returns the chosen level number: 1, 2, or 3 (0 if none yet).
     */
    int getSelectedLevel() const { return _selectedLevel; }

#pragma mark -
#pragma mark Update
    /**
     * The method called to indicate the start of a deterministic loop.
     * 
     * @param dt    The amount of time (in seconds) since the last frame
    */
    void preUpdate(float dt);

    /**
     * The method called to indicate the end of a deterministic loop.
     *
     * @param remain    The amount of time (in seconds) last fixedUpdate
    */
    void postUpdate(float remain);

    /**
     * The method called to update the scene every animation frame.
     *
     * @param step  The number of fixed seconds for this step
     */
    void fixedUpdate(float step);

    // Make compiler stop whining
    void render(void) {
        scene2::Scene2::render();
    }
};