#pragma once

#include <cugl/cugl.h>

/**
 * A unified UI scene for in-game HUD, pause menu, and related UI elements.
 */
class LevelSelectUI : public cugl::scene2::SceneNode {
protected:
    /** The asset manager for this game mode. */
    std::shared_ptr<cugl::AssetManager> _assets;

    std::shared_ptr<cugl::scene2::Button> _level1Button;
    std::shared_ptr<cugl::scene2::Button> _level2Button;
    std::shared_ptr<cugl::scene2::Button> _level3Button;

    std::function<void()> _level1Callback;
    std::function<void()> _level2Callback;
    std::function<void()> _level3Callback;

    bool _active;

    void setButtonsActive(std::shared_ptr<cugl::scene2::SceneNode> layer, bool active);

public:
    cugl::Vec2 _screenOffset;

#pragma mark -
#pragma mark Constructors

    ~LevelSelectUI() { dispose(); }

    virtual void dispose() override;

    bool init(const std::shared_ptr<cugl::AssetManager>& assets);

    void setupMenu(std::shared_ptr<cugl::scene2::SceneNode>& menu);

    static std::shared_ptr<LevelSelectUI> alloc(const std::shared_ptr<cugl::AssetManager>& assets) {
        std::shared_ptr<LevelSelectUI> result = std::make_shared<LevelSelectUI>();
        return (result->init(assets) ? result : nullptr);
    }

    bool isActive() const { return _active; }

    void setLevel1Callback(const std::function<void()>& callback) { _level1Callback = callback; }
    void setLevel2Callback(const std::function<void()>& callback) { _level2Callback = callback; }
    void setLevel3Callback(const std::function<void()>& callback) { _level3Callback = callback; }

    void showMenu(bool visible);
};