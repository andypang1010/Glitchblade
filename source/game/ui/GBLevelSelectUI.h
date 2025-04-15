#pragma once

#include <cugl/cugl.h>

class GBLevelSelectUI : public cugl::scene2::SceneNode {
protected:
    /** The asset manager for this game mode. */
    std::shared_ptr<cugl::AssetManager> _assets;

    std::shared_ptr<cugl::scene2::Button> _level1Button;
    std::shared_ptr<cugl::scene2::Button> _level2Button;
    std::shared_ptr<cugl::scene2::Button> _level3Button;

    std::string _loadSceneKey = "";

    bool _active;

public:
#pragma mark -
#pragma mark Constructors

    /**
     * Disposes of all (non-static) resources allocated to this mode.
     *
     * This method is different from dispose() in that it ALSO shuts off any
     * static resources, like the input controller.
     */
    ~GBLevelSelectUI() { dispose(); }

    /**
     * Disposes of all (non-static) resources allocated to this mode.
     */
    virtual void dispose() override;

    /**
     * Initializes the controller contents, and starts the game
     *
     * The constructor does not allocate any objects or memory.  This allows
     * us to have a non-pointer reference to this controller, reducing our
     * memory allocation.  Instead, allocation happens in this method.
     *
     * @param assets    The (loaded) assets for this game mode
     *
     * @return true if the controller is initialized properly, false otherwise.
     */
    bool init(const std::shared_ptr<cugl::AssetManager>& assets);

    static std::shared_ptr<GBLevelSelectUI> alloc(const std::shared_ptr<cugl::AssetManager>& assets) {
        std::shared_ptr<GBLevelSelectUI> result = std::make_shared<GBLevelSelectUI>();
        return (result->init(assets) ? result : nullptr);
    }

    /**
     * Sets whether the scene is currently active
     *
     * @param value whether the scene is currently active
     */
    void setActive(bool value);

    std::string getLoadSceneKey();

    void setLoadSceneKey(std::string key);

    bool isActive() const { return _active; }
};
