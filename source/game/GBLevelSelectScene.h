#pragma once

#include <cugl/cugl.h>
#include "ui/GBLevelSelectUI.h"
#include "GBSceneInterface.h"

class GBLevelSelectScene : public cugl::scene2::Scene2, public GBSceneInterface {
protected:
    std::shared_ptr<cugl::AssetManager> _assets;
    std::shared_ptr<GBLevelSelectUI> _levelSelectUI;
    std::shared_ptr<cugl::scene2::Button> _level3Button;
    std::string _levelSelected = "";

public:
    GBLevelSelectScene() {}
    ~GBLevelSelectScene() { dispose(); }

    virtual void dispose() override;

    void setActive(bool active) override;

    bool init(const std::shared_ptr<cugl::AssetManager>& assets);

    static std::shared_ptr<GBLevelSelectScene> alloc(const std::shared_ptr<cugl::AssetManager>& assets) {
        std::shared_ptr<GBLevelSelectScene> result = std::make_shared<GBLevelSelectScene>();
        return (result->init(assets) ? result : nullptr);
    }

    const std::shared_ptr<GBLevelSelectUI>& getIngameUI() const { return _levelSelectUI; }

    void render() override;

    void fixedUpdate(float dt) override;

    void postUpdate(float dt) override;

    void preUpdate(float dt) override;
    std::string getLevelSelected();
    void setLevelSelected(std::string lvlString);
};
