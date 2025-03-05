//
// Dog.h
// Dog Demo
//
// This class tracks all of the state of a single dog and implements methods
// that will be used as leaf node execute functions to control this dog using
// a behavior tree.
//
// Authors: David Kim and James Liu
// Version 12/19/24

#ifndef __DEMO_DOG_H__
#define __DEMO_DOG_H__

#include <cugl/cugl.h>
#include <random>
#include "CUNode.h"
using namespace cugl::btree;

class Dog
{
private:
  /** Reference to the dog's sprite sheet */
  std::shared_ptr<cugl::graphics::SpriteSheet> _sprite;
  /** Position of the dog */
  cugl::Vec2 _pos;
  /** The target postion for this dog to run to */
  cugl::Vec2 _target;
  /** If this dog is facing left, used to flip animations */
  bool facingLeft = true;
  /** The speed at which this dog moves */
  float speed = 30;
  /** The number of columns in the sprite sheet */
  int _framecols = 4;
  /** The number of frams in the sprite sheet */
  int _framesize = 4;
  /** The current frame in the animation */
  int currFrame = 0;
  /** The frame delay between changing animation frames */
  int frameDelay = 10;
  /** The counter for tracking frame delay */
  int delayCounter = 0;
  /** If this dog has a valid target */
  bool hasTarget = false;

public:
  unsigned int seed = 0;
  std::vector<cugl::Vec2> treePositions;

  std::shared_ptr<cugl::graphics::Texture> _currentTexture;

  std::shared_ptr<cugl::graphics::Texture> _sleepTexture;

  std::shared_ptr<cugl::graphics::Texture> _barkTexture;

  std::shared_ptr<cugl::graphics::Texture> _idleTexture;

  std::shared_ptr<cugl::graphics::Texture> _runTexture;

  std::shared_ptr<cugl::graphics::Texture> _markTexture;

  Dog() {};

  ~Dog() {};

  std::string name;

  /**
   * Executes the bark action.
   *
   * @return Success
   */
  Node::State bark();

  /**
   * Executes the sleep action.
   *
   * @return Running
   */
  Node::State sleep();

  /**
   * Executes the play action.
   *
   * @return Success
   */
  Node::State play();

  /**
   * Executes the mark action.
   *
   * @return Success
   */
  Node::State mark();

  /**
   * Executes the run action. The dog moves towards its target tree.
   *
   * @return Success if the dog is at its target, otherwise running.
   */
  Node::State run();

  /**
   * Selects a random target tree to run to.
   */
  void chooseTarget();

  /**
   * Sets the position of this dog.
   *
   * @param value the position of this dog
   */
  void setPosition(cugl::Vec2 value) { _pos = value; }

  /**
   * Sets the texture for this dog.
   *
   * The texture should be formated as a sprite sheet, and the size and
   * layout of the sprite sheet should already be specified in the
   * initializing JSON. If so, this method will construct a sprite sheet
   * from this texture. Otherwise, the texture will be ignored.
   *
   * @param texture   The texture for the sprite sheet
   */
  void setTexture(const std::shared_ptr<cugl::graphics::Texture> &texture);

  /**
   * Draws this dog to the sprite batch within the given bounds.
   *
   * @param batch     The sprite batch to draw to
   * @param size      The size of the window (for wrap around)
   */
  void draw(const std::shared_ptr<cugl::graphics::SpriteBatch> &batch, cugl::Size size);
};

#endif // __DEMO_DOG_H__
