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

#include "Dog.h"

using namespace cugl;
using namespace cugl::graphics;
using namespace cugl::scene2;

/**
 * Executes the bark action.
 *
 * @return Success
 */
Node::State Dog::bark()
{
	CULog("barking");
	if (_currentTexture != _barkTexture)
	{
		setTexture(_barkTexture);
	}
	return Node::State::Success;
}

/**
 * Executes the play action.
 *
 * @return Success
 */
Node::State Dog::play()
{
	CULog("playing");
	return Node::State::Success;
}

/**
 * Executes the sleep action.
 *
 * @return Running
 */
Node::State Dog::sleep()
{
	CULog("sleeping");
	if (_currentTexture != _sleepTexture)
	{
		setTexture(_sleepTexture);
	}
	return Node::State::Running;
}

/**
 * Executes the mark action.
 *
 * @return Success
 */
Node::State Dog::mark()
{
	CULog("marking");
	if (_currentTexture != _markTexture)
	{
		setTexture(_markTexture);
	}
	return Node::State::Success;
}

/**
 * Executes the run action. The dog moves towards its target tree.
 *
 * @return Success if the dog is at its target, otherwise running.
 */
Node::State Dog::run()
{
	CULog("running");
	if (_currentTexture != _idleTexture) {
		setTexture(_idleTexture);
	}
	if (hasTarget == false)
	{
		hasTarget = true;
		chooseTarget();
	}

	Vec2 dir = (_target - _pos).getNormalization();
	facingLeft = dir.x < 0;
	_pos += speed * dir;

	float distToTarget = (_target - _pos).length();
	CULog("dist %f", distToTarget);
	if (distToTarget < 15.01f)
	{
		CULog("run succeeds");
		hasTarget = false;
		return Node::State::Success;
	}
	return Node::State::Running;
}

/**
 * Selects a random target tree to run to.
 */
void Dog::chooseTarget()
{
	std::random_device rd;
	unsigned int seed;
	if (this->seed != 0)
	{
		seed = this->seed;
	}
	else
	{
		seed = rd();
	}
	std::mt19937 g(seed);
	std::uniform_int_distribution<int> dis(0, (int)treePositions.size() - 1);
	_target = treePositions[dis(g)];
	unsigned int newSeed = g();
	this->seed = newSeed;
}

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
void Dog::setTexture(const std::shared_ptr<Texture> &texture)
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
		_sprite->setOrigin(_sprite->getFrameSize() / 2);
	}
}

/**
 * Draws this dog to the sprite batch within the given bounds.
 *
 * @param batch     The sprite batch to draw to
 * @param size      The size of the window (for wrap around)
 */
void Dog::draw(const std::shared_ptr<SpriteBatch> &batch, Size size)
{
	if (_sprite)
	{
		Affine2 dogTrans;
		dogTrans.scale(Vec2(1, 1));
		dogTrans.translate(_pos);

		_sprite->setFrame(currFrame);
		_sprite->draw(batch, dogTrans);
		delayCounter++;
		if (delayCounter > frameDelay)
		{
			currFrame++;
			if (currFrame >= _framesize)
			{
				currFrame = 0;
			}
			delayCounter = 0;
		}
	}
}
