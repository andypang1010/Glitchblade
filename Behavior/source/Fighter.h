#ifndef __DEMO_FIGHTER_H__
#define __DEMO_FIGHTER_H__

#include <cugl/cugl.h>
#include "CUNode.h"
using namespace cugl::btree;

class Fighter
{
private:
  cugl::Vec2 _pos;

  cugl::Vec2 _inPos;

  bool _blocking;

  bool _punchLanded;

  bool _drawPow;

  int _health;
  int _punchDamage;
  int _jabDamage;

  std::string name;

  std::shared_ptr<Fighter> _target;

  std::shared_ptr<cugl::graphics::SpriteSheet> _sprite;
  std::shared_ptr<cugl::graphics::SpriteSheet> _powSprite;

  int _framecols = 4;
  int _framesize = 4;
  int currFrame = 0;
  int frameDelay = 10;
  int delayCounter = 0;
  int powCounter = 0;

public:
  std::shared_ptr<cugl::graphics::Texture> _currentTexture;

  std::shared_ptr<cugl::graphics::Texture> _jabTexture;

  std::shared_ptr<cugl::graphics::Texture> _punchTexture;

  std::shared_ptr<cugl::graphics::Texture> _idleTexture;

  std::shared_ptr<cugl::graphics::Texture> _blockTexture;

  std::shared_ptr<cugl::graphics::Texture> _powTexture;
  Fighter()
  {
    _blocking = false;
    _punchLanded = false;
    _drawPow = false;
    _health = 100;
    _punchDamage = 12;
    _jabDamage = 8;
  };
  ~Fighter() {};

  Node::State jab();
  Node::State punch();
  Node::State idle();
  Node::State block();

  void setTarget(const std::shared_ptr<Fighter> &target);

  void setPosition(cugl::Vec2 pos)
  {
    this->_pos = pos;
  }

  void setName(std::string name)
  {
    this->name = name;
  }

  void init(const std::shared_ptr<cugl::AssetManager> &assets)
  {
    // set pow sprite
    _powTexture = assets->get<cugl::graphics::Texture>("pow");
    int rows = _framesize / _framecols;
    if (_framesize % _framecols != 0)
    {
      rows++;
    }
    _powSprite = cugl::graphics::SpriteSheet::alloc(_powTexture, rows, _framecols, _framesize);
    _powSprite->setFrame(0);
    _powSprite->setOrigin(_powSprite->getFrameSize() / 2);
  }

  std::string getName() { return name; }

  int getHealth() { return _health; }

  void adjustHealth(int delta)
  {
    _health += delta;
  }

  bool isBlocking() { return _blocking; }

  void setTexture(const std::shared_ptr<cugl::graphics::Texture> &texture);

  void draw(const std::shared_ptr<cugl::graphics::SpriteBatch> &batch,
            cugl::Size size);

  void drawPow(const std::shared_ptr<cugl::graphics::SpriteBatch> &batch,
               cugl::Size size);
};
#endif // __DEMO_FIGHTER_H__
