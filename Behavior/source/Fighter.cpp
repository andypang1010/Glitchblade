#include "Fighter.h"

using namespace cugl;
using namespace cugl::graphics;
using namespace cugl::scene2;

Node::State Fighter::block()
{
	if (_currentTexture != _blockTexture)
	{
		setTexture(_blockTexture);
	}
	CULog("%s blocks", this->name.c_str());
	this->_blocking = true;
	return Node::State::Success;
}

Node::State Fighter::idle()
{
	if (_currentTexture != _idleTexture)
	{
		setTexture(_idleTexture);
	}
	this->_blocking = false;
	CULog("%s idles", this->name.c_str());
	return Node::State::Success;
}

Node::State Fighter::jab()
{
	if (_currentTexture != _jabTexture)
	{
		setTexture(_jabTexture);
	}
	this->_blocking = false;
	CULog("%s jabs", this->name.c_str());

	if (!_target->isBlocking())
	{
		CULog("%s hits for %d damage!", this->name.c_str(), this->_jabDamage);
		_target->adjustHealth(-_jabDamage);
		_punchLanded = true;
		_drawPow = true;
		return Node::State::Success;
	}
	CULog("%s blocks the jab!", _target->name.c_str());
	return Node::State::Failure;
}

Node::State Fighter::punch()
{
	if (_currentTexture != _punchTexture)
	{
		setTexture(_punchTexture);
	}
	this->_blocking = false;
	CULog("%s punches", this->name.c_str());
	if (!_target->isBlocking())
	{
		CULog("%s hits for %d damage!", this->name.c_str(), this->_punchDamage);
		_target->adjustHealth(-_punchDamage);
		_punchLanded = true;
		_drawPow = true;
		return Node::State::Success;
	}
	CULog("%s blocks the punch!", _target->name.c_str());
	return Node::State::Failure;
}

void Fighter::setTarget(const std::shared_ptr<Fighter> &target)
{
	this->_target = target;
}

void Fighter::setTexture(const std::shared_ptr<Texture> &texture)
{
	if (_framecols > 0)
	{
		int rows = _framesize / _framecols;
		if (_framesize % _framecols != 0)
		{
			rows++;
		}
		_currentTexture = texture;
		_sprite = SpriteSheet::alloc(_currentTexture, rows, _framecols, _framesize);
		_sprite->setFrame(0);
		currFrame = 0;
		_sprite->setOrigin(_sprite->getFrameSize() / 2);
	}
}

void Fighter::drawPow(const std::shared_ptr<SpriteBatch> &batch, Size size)
{
	Affine2 trans;
	Vec2 powPos = this->_target->_pos;
	powPos.y += _currentTexture->getHeight() / 3;
	trans.translate(powPos);
	// delay pow by a frame
	if (_drawPow && powCounter > 1)
	{
		_powSprite->setFrame(currFrame);
		_powSprite->draw(batch, trans);
		if (powCounter >= _framesize + 1)
		{
			powCounter = 0;
			_drawPow = false;
		}
	}
}

void Fighter::draw(const std::shared_ptr<SpriteBatch> &batch, Size size)
{
	if (_sprite)
	{
		Affine2 trans;
		trans.scale(Vec2(1, 1));
		trans.translate(_pos);

		_sprite->setFrame(currFrame);
		_sprite->draw(batch, trans);
		delayCounter++;
		if (delayCounter > frameDelay)
		{
			currFrame++;
			if (_drawPow)
			{
				powCounter++;
			}
			if (currFrame >= _framesize)
			{
				currFrame = 0;
				if (_punchLanded && (_currentTexture == _jabTexture || _currentTexture == _punchTexture))
				{
					setTexture(_idleTexture);
					_punchLanded = false;
				}
			}
			delayCounter = 0;
		}
	}
}
