#ifndef __DEMO_PINE_CONTROLLER_H__
#define __DEMO_PINE_CONTROLLER_H__

#include <cugl/cugl.h>
#include <random>

class PineController
{
public:
  class Pine
  {
  public:
    cugl::Vec2 _pos;

    Pine(cugl::Vec2 pos)
    {
      _pos = pos;
    };
    ~Pine() {};
    void setPosition(cugl::Vec2 value) { _pos = value; }
  };

protected:
  std::unordered_set<std::shared_ptr<Pine>> _pines;
  std::shared_ptr<cugl::graphics::SpriteSheet> _sprite;

public:
  std::shared_ptr<cugl::graphics::Texture> _texture;

  PineController() {};
  ~PineController() {};

  bool init(int n, cugl::Size size) {
    _sprite = cugl::graphics::SpriteSheet::alloc(_texture, 1, 1, 1);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, 100);

    for (int i = 0; i < n; i++)
    {
      int x = dis(gen);
      int y = dis(gen);

      std::shared_ptr pine = std::make_shared<Pine>(cugl::Vec2(x * size.width / 110, y * size.height / 110));
      _pines.insert(pine);
    }
    return true;
  }
  bool init(int n, cugl::Size size, unsigned int seed)
  {
      _sprite = cugl::graphics::SpriteSheet::alloc(_texture, 1, 1, 1);
      std::mt19937 gen(seed);
      std::uniform_int_distribution<int> dis(0, 100);

      for (int i = 0; i < n; i++)
      {
          int x = dis(gen);
          int y = dis(gen);

          std::shared_ptr pine = std::make_shared<Pine>(cugl::Vec2(x * size.width / 110, y * size.height / 110));
          _pines.insert(pine);
      }
      return true;
  }

  void setTexture()
  {
  }

  std::vector<cugl::Vec2> getPositions()
  {
    std::vector<cugl::Vec2> positions;
    for (auto it = _pines.begin(); it != _pines.end(); ++it)
    {
      cugl::Vec2 pos = (*it)->_pos;
      positions.push_back(pos);
    }
    return positions;
  }

  void draw(const std::shared_ptr<cugl::graphics::SpriteBatch> &batch)
  {
    if (_texture)
    {
      for (auto it = _pines.begin(); it != _pines.end(); ++it)
      {
        cugl::Vec2 pos = (*it)->_pos;
        cugl::Affine2 trans;
        trans.scale(cugl::Vec2(1, 1));
        trans.translate(pos);
        _sprite->draw(batch, trans);
      }
    }
  }
};

#endif //__DEMO_PINE_CONTROLLER_H__
