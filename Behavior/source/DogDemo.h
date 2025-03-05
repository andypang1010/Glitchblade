#ifndef __DEMO_DOG_DEMO_H__
#define __DEMO_DOG_DEMO_H__

#include <cugl/cugl.h>
#include "CUTreeManager.h"
#include "PineController.h"
#include "Dog.h"

class DogDemo : public cugl::scene2::Scene2 {
    
  std::shared_ptr<cugl::AssetManager> _assets;

  std::shared_ptr<TreeManager> _treeManager;

  PineController _pineController;

  std::shared_ptr<cugl::graphics::Texture> _background;

  float delay;

  float delayCounter;

  int step;

  std::shared_ptr<Dog> _dog;
  std::shared_ptr<Dog> _dog2;

public:
  /**
   * Creates a new game mode with the default values.
   *
   * This constructor does not allocate any objects or start the game.
   * This allows us to use the object without a heap pointer.
   */
  DogDemo() : cugl::scene2::Scene2() {}

  /**
   * Disposes of all (non-static) resources allocated to this mode.
   *
   * This method is different from dispose() in that it ALSO shuts off any
   * static resources, like the input controller.
   */
  ~DogDemo() { dispose(); }

  /**
   * Disposes of all (non-static) resources allocated to this mode.
   */
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
   *
   * @param batch     The SpriteBatch to draw with.
   */
  void render() override;

  /**
   * Resets the status of the game so that we can play again.
   */
  void reset() override;
};

#endif // __DEMO_DOG_DEMO_H__
