//
//  CUAnimateSprite.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides support for spritesheet animation in a 2d scene graph.
//  The animation is is represented as a sequence of frames. There is no
//  tweening support between animation frames.
//
//  The class in this module is actually a factory for creating actions from
//  scene graph nodes. This class uses our standard shared-pointer architecture.
//
//  1. The constructor does not perform any initialization; it just sets all
//     attributes to their defaults.
//
//  2. All initialization takes place via init methods, which can fail if an
//     object is initialized more than once.
//
//  3. All allocation takes place via static constructors which return a shared
//     pointer.
//
//  CUGL MIT License:
//      This software is provided 'as-is', without any express or implied
//      warranty.  In no event will the authors be held liable for any damages
//      arising from the use of this software.
//
//      Permission is granted to anyone to use this software for any purpose,
//      including commercial applications, and to alter it and redistribute it
//      freely, subject to the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not
//      be misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
//
//  Author: Walker White (based on work by Sophie Huang)
//  Version: 7/3/24 (CUGL 3.0 reorganization)
//
#include <cugl/core/actions/CUAnimateSprite.h>
#include <cugl/core/util/CUDebug.h>

using namespace cugl;

/**
 * Initializes an animation sequence of frames start to end (inclusive).
 *
 * The animation sequence has start as its first frame and end as its last.
 * Animation will be in frame order, with an equal amount of time spent
 * on each frame. The value start must be less than (or equal to) end, as
 * this action does not know the spritesheet size.
 *
 * The repeat argument is optional. It specifies the number of times to
 * repeat the animation sequence.
 *
 * @param start     The initial frame to animate
 * @param end       The final frame to animate
 * @param repeat    The number of times to repeat the sequence
 *
 * @return true if initialization was successful.
 */
bool AnimateSprite::init(int start, int end, int repeat) {
    _frameset = std::vector<int>();
    _weights = std::vector<float>();
    for(int jj = 0; jj < repeat; jj++) {
        for(int ii = start; ii <= end; ii++) {
            _frameset.push_back(ii);
        }
    }
    _uniform = true;
    return true;
    
}

/**
 * Initializes an animation sequence of uniform speed
 *
 * The animation sequence is given by the specified vector. The animation
 * will spend an equal amount of time on each frame.
 *
 * @param frames    The animation sequence
 *
 * @return true if initialization was successful.
 */
bool AnimateSprite::init(const std::vector<int>& frames) {
    _uniform = true;
    _frameset = frames;
    _weights = std::vector<float>();
    return true;
}

/**
 * Initializes an animation sequence of variable speed
 *
 * The animation sequence is given by the first specified vector. The
 * second vector specifies the weight of each frame. The weights specify
 * the percentage of time spent on each frame. These weights must sum to
 * 1. If they do not, then only the initial segment of weights suming to
 * 1 will be used; the other weights will be zeroed.
 *
 * Both vectors must be the same length. They can be empty.
 *
 * @param frames    The animation sequence
 * @param weights   The percentage of time to spend on each frame
 *
 * @return true if initialization was successful.
 */
bool AnimateSprite::init(const std::vector<int>& frames, const std::vector<float>& weights) {
    CUAssertLog(_frameset.size() == _weights.size(),
                "The weights do not agree with the frame sequence");
    _frameset = frames;
    _weights = weights;
    float total = 0;
    for(int ii = 0; ii < _weights.size(); ii++) {
        if (total >= 1) {
            _weights[ii] = 0;
        }
        total += _weights[ii];
        if (total >= 1) {
            _weights[ii] -= (total-1);
        }
    }
    _uniform = false;
    return true;
}

/**
 * Disposes all of the resources used by this animation.
 *
 * A disposed action can be safely reinitialized.
 */
void AnimateSprite::dispose() {
    _frameset.clear();
    _weights.clear();
    _uniform = true;
}


#pragma mark -
#pragma mark Atributes
/**
 * Returns the frame in the filmstrip to be animated at time index t in [0,1]
 *
 * @return the frame in the filmstrip to be animated at time index t in [0,1]
 */
int AnimateSprite::getFrame(float t) const {
    int pos = 0;
    if (_uniform) {
        pos = (int)((int)_frameset.size()*t);
    } else if (!_weights.empty()) {
        float total = 0;
        for(int ii = 0; total < t && ii < _frameset.size(); ii++) {
            pos = ii;
            total += _weights[ii];
        }
    }
    if (pos >= _frameset.size()) {
        pos = (int)_frameset.size()-1;
    } else if (pos <= 0) {
        pos = 0;
    }
    
    return _frameset[pos];
}

/**
 * Sets the sequence of frames used in this animation
 *
 * If this set has a different size than the one initial set, this setter
 * will keep the overall animation duration, but will revert to a uniform
 * time step.
 *
 * Changing this value for an actively animating action can have undefined
 * side effects.
 *
 * @param frames    the sequence of frames used in this animation
 */
void AnimateSprite::setSequence(const std::vector<int>& frames) {
    _frameset = frames;
    if (!_uniform && _frameset.size() != _weights.size()) {
        _uniform = true;
        _weights.clear();
    }
}

/**
 * Sets the sequence of frames used in this animation
 *
 * The animation sequence is given by the first specified vector. The
 * second vector specifies the weight of each frame. The weights specify
 * the percentage of time spent on each frame. These weights must sum to
 * 1. If they do not, then only the initial segment of weights suming to
 * 1 will be used; the other weights will be zeroed.
 *
 * Both vectors must be the same length. They can be empty.
 *
 * Changing this value for an actively animating action can have undefined
 * side effects.
 *
 * @param frames    The animation sequence
 * @param weights   The percentage of time to spend on each frame
 */
void AnimateSprite::setSequence(const std::vector<int>& frames,
                                const std::vector<float>& weights) {
    CUAssertLog(_frameset.size() == _weights.size(),
                "The weights do not agree with the frame sequence");
    _frameset = frames;
    _weights = weights;
    float total = 0;
    for(int ii = 0; ii < _weights.size(); ii++) {
        if (total >= 1) {
            _weights[ii] = 0;
        }
        total += _weights[ii];
        if (total >= 1) {
            _weights[ii] -= (total-1);
        }
    }
    _uniform = false;
}

/**
 * Forces this animation to use a uniform time step for all frames
 *
 * Changing this value for an actively animating action can have undefined
 * side effects.
 */
void AnimateSprite::setUniform() {
    _uniform = true;
    _weights.clear();
}
