#ifndef __GB_UI_CONTROLLER_H__
#define __GB_UI_CONTROLLER_H__
#include <cugl/cugl.h>
#include "GBIngameUI.h"

class GBUIController : public cugl::scene2::Scene2 {
protected:
    std::shared_ptr<cugl::AssetManager> _assets;
    std::shared_ptr<GBIngameUI> _ingameUI;

public:
    GBUIController() {}
    ~GBUIController() { dispose(); }

    virtual void dispose() override;

    bool init(const std::shared_ptr<cugl::AssetManager>& assets);

    static std::shared_ptr<GBUIController> alloc(const std::shared_ptr<cugl::AssetManager>& assets) {
        std::shared_ptr<GBUIController> result = std::make_shared<GBUIController>();
        return (result->init(assets) ? result : nullptr);
    }

    const std::shared_ptr<GBIngameUI>& getIngameUI() const { return _ingameUI; }
    
    void render(const std::shared_ptr<cugl::graphics::SpriteBatch>& batch);
    
    void update(float dt);

    void setIngameUIActive(bool value);

    void setHP(int hp);
};

#endif /* __GB_UI_CONTROLLER_H__ */
