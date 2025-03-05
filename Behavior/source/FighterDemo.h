#ifndef __DEMO_FIGHTER_DEMO_H__
#define __DEMO_FIGHTER_DEMO_H__

#include <cugl/cugl.h>
#include "CUTreeManager.h"
#include "Fighter.h"

class FighterDemo : public cugl::scene2::Scene2 {
protected:
  std::shared_ptr<cugl::AssetManager> _assets;

  std::shared_ptr<TreeManager> _treeManager;

  std::shared_ptr<cugl::graphics::Texture> _background;
  std::shared_ptr<cugl::graphics::Texture> _foreground;
  std::shared_ptr<cugl::graphics::Texture> _blueWins;
  std::shared_ptr<cugl::graphics::Texture> _redWins;

  float delay;
  float delayCounter;
  int step;

  bool knockout;

  std::shared_ptr<Fighter> fighter1;
  std::shared_ptr<Fighter> fighter2;

public:
  FighterDemo() : cugl::scene2::Scene2() {}
  ~FighterDemo() {};

  void onWin();

  void dispose() override;

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
  bool init(const std::shared_ptr<cugl::AssetManager> &assets);

  /**
   * The method called to update the game mode.
   *
   * This method contains any gameplay code that is not an OpenGL call.
   *
   * @param timestep  The amount of time (in seconds) since the last frame
   */
  void update(float timestep) override;

  /**
   * Draws all this scene to the given SpriteBatch.
   *
   * The default implementation of this method simply draws the scene graph
   * to the sprite batch.  By overriding it, you can do custom drawing
   * in its place.
   */
  void render() override;

  /**
   * Resets the status of the game so that we can play again.
   */
  void reset() override;
};

#endif /* __DEMO_FIGHTER_DEMO_H__*/
