// Just a try, not correct.
//
//#ifndef __GB_UI_CONTROLLER_H__
//#define __GB_UI_CONTROLLER_H__
//#include <cugl/cugl.h>
//#include "GBIngameUI.h"
//
//class GBUIController {
//protected:
//    /** The asset manager for UI elements */
//    std::shared_ptr<cugl::AssetManager> _assets;
//
//    /** In-game UI (pause button, hp bar, etc.) */
//    std::shared_ptr<GBIngameUI> _ingameUI;
//
//public:
//    /** Default constructor */
//    GBUIController() {}
//
//    /** Destructor */
//    ~GBUIController() { dispose(); }
//
//    /**
//     * Initialize all UI scenes
//     *
//     * @param assets The loaded asset manager
//     * @return true if initialized successfully
//     */
//    bool init(const std::shared_ptr<cugl::AssetManager>& assets);
//
//    /**
//     * Disposes all UI components
//     */
//    void dispose();
//
//    /**
//     * Update all active UI
//     *
//     * @param timestep The time in seconds since last update
//     */
//    void update(float timestep);
//
//    bool isActive() const;
//    
//    void render(const std::shared_ptr<cugl::graphics::SpriteBatch>& batch);
//    
//    /** Activates or deactivates the UI scene */
//    void setActive(bool value);
//    
//    /**
//     * Returns the in-game UI scene
//     */
//    const std::shared_ptr<GBIngameUI>& getIngameUI() const { return _ingameUI; }
//    
//    /**
//     * Updates the visible HP bar via the Ingame UI.
//     *
//     * @param hp  The current HP value (0–100)
//     */
//    void setHP(int hp);
//    
//};
//
//#endif /* __GB_UI_CONTROLLER_H__ */

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
