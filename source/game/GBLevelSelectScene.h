#pragma once

#include <cugl/cugl.h>
#include "ui/GBLevelSelectUI.h"
#include "GBSceneInterface.h"

class GBLevelSelectScene : public cugl::scene2::Scene2, public GBSceneInterface {
protected:
    std::shared_ptr<cugl::AssetManager> _assets;
    std::shared_ptr<GBLevelSelectUI> _levelSelectUI;

    std::shared_ptr<cugl::scene2::Button> _level3Button;

public:
    GBLevelSelectScene() {}
    ~GBLevelSelectScene() { dispose(); }

    virtual void dispose() override;

    bool init(const std::shared_ptr<cugl::AssetManager>& assets);

    static std::shared_ptr<GBLevelSelectScene> alloc(const std::shared_ptr<cugl::AssetManager>& assets) {
        std::shared_ptr<GBLevelSelectScene> result = std::make_shared<GBLevelSelectScene>();
        return (result->init(assets) ? result : nullptr);
    }

    const std::shared_ptr<GBLevelSelectUI>& getIngameUI() const { return _levelSelectUI; }

    void render();

    void fixedUpdate(float dt);

    void postUpdate(float dt);

    void preUpdate(float dt);
};
