//
//  CUAlgorthmicReverb.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides an audio node wrapper for the reverb support provided
//  by SDL_atk. That implementation is modeled after the the open source
//  Schroeder reverberator, Freeverb.
//
//  More about the program can be found at:
//  https://ccrma.stanford.edu/~jos/pasp/Freeverb.html
//
//  Open source code taken from:
//  https://github.com/tim-janik/beast/tree/master/plugins/freeverb
//
//  This class uses our standard shared-pointer architecture.
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
//
//     This software is provided 'as-is', without any express or implied
//     warranty.  In no event will the authors be held liable for any damages
//     arising from the use of this software.
//
//     Permission is granted to anyone to use this software for any purpose,
//     including commercial applications, and to alter it and redistribute it
//     freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you must not
//     claim that you wrote the original software. If you use this software
//     in a product, an acknowledgment in the product documentation would be
//     appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and must not
//     be misrepresented as being the original software.
//
//  3. This notice may not be removed or altered from any source distribution.
//
//  Author: Walker White
//  Version: 7/3/24 (CUGL 3.0 reorganization)
//
#include <cugl/audio/graph/CUAlgorithmicReverb.h>
#include <cugl/audio/CUAudioDevices.h>
#include <cugl/core/util/CUDebug.h>
#include <SDL_atk.h>
#include <algorithm>
#include <thread>

using namespace cugl::audio;

/**
 * Creates a degenerate audio player with no associated source.
 *
 * The player has no channels or source file, so read options will do nothing.
 * The player must be initialized to be used.
 */
AlgorithmicReverb::AlgorithmicReverb() : AudioNode(),
_reverb(NULL),
_outdone(false),
_outmark(-1),
_fadeout(0),
_dirty(false) {
    _classname = "AudioReverb";

    ATK_AlgoReverbDef settings;
    ATK_AlgoReverbDefaults(&settings);
    _wet = settings.wet;
    _dry = settings.dry;
    _damp = settings.damping;
    _width = settings.width;
    _ingain = settings.ingain;
    _roomsize = settings.roomsize;
}

/**
 * Initializes the reverb filter from the default settings.
 */
void AlgorithmicReverb::initFilter() {
    ATK_AlgoReverbDef settings;
    settings.dry = _dry;
    settings.wet = _wet;
    settings.ingain = _ingain;
    settings.width  = _width;
    settings.damping = _damp;
    settings.roomsize = _roomsize;
    _reverb = ATK_AllocAlgoReverb(&settings, _sampling, _channels, _readsize);
}

/**
 * Initializes the node with default stereo settings
 *
 * The number of channels is two, for stereo output. The sample rate is
 * the modern standard of 48000 HZ.
 *
 * These values determine the buffer the structure for all {@link read}
 * operations.  In addition, they also detemine whether this node can
 * serve as an input to other nodes in the audio graph.
 *
 * The reverb will be set with the default settings as defined by the
 * public domain FreeVerb algorithm.
 *
 * @return true if initialization was successful
 */
bool AlgorithmicReverb::init() {
    if (AudioNode::init()) {
        _input = nullptr;
        initFilter();
        return true;
    }
    return false;
}

/**
 * Initializes the node with the given number of channels and sample rate
 *
 * These values determine the buffer the structure for all {@link read}
 * operations.  In addition, they also detemine whether this node can
 * serve as an input to other nodes in the audio graph.
 *
 * The reverb will be set with the default settings as defined by the
 * public domain FreeVerb algorithm.
 *
 * @param channels  The number of audio channels
 * @param rate      The sample rate (frequency) in HZ
 *
 * @return true if initialization was successful
 */
bool AlgorithmicReverb::init(Uint8 channels, Uint32 rate) {
    if (AudioNode::init(channels, rate)) {
        _input = nullptr;
        initFilter();
        return true;
    }
    return false;

}

/**
 * Initializes reverb for the given input node.
 *
 * This node acquires the channels and sample rate of the input.  If
 * input is nullptr, this method will fail.
 *
 * The reverb will be set with the default settings as defined by the
 * public domain FreeVerb algorithm.
 *
 * @param input     The audio node to fade
 *
 * @return true if initialization was successful
 */
bool AlgorithmicReverb::init(const std::shared_ptr<AudioNode>& input) {
    if (input && AudioNode::init(input->getChannels(), input->getRate())) {
        attach(input);
        initFilter();
        return true;
    }
    return false;

}

/**
 * Disposes any resources allocated for this player
 *
 * The state of the node is reset to that of an uninitialized constructor.
 * Unlike the destructor, this method allows the node to be reinitialized.
 */
void AlgorithmicReverb::dispose() {
    if (_booted) {
        AudioNode::dispose();
        _wet = 0;
        _dry = 0;
        _damp = 0;
        _width = 0;
        _ingain = 0;
        _roomsize = 0;
        if (_reverb != NULL) {
            ATK_FreeAlgoReverb(_reverb);
            _reverb = NULL;
        }
    }
}

#pragma mark -
#pragma mark Fade In/Out Support
/**
 * Attaches an audio node to this reverb node.
 *
 * This method will fail if the channels of the audio node do not agree
 * with this node.
 *
 * @param node  The audio node to add reverb to
 *
 * @return true if the attachment was successful
 */
bool AlgorithmicReverb::attach(const std::shared_ptr<AudioNode>& node) {
    if (!_booted) {
        CUAssertLog(_booted, "Cannot attach to an uninitialized audio node");
        return false;
    } else if (node == nullptr) {
        detach();
        return true;
    } else if (node->getChannels() != _channels) {
        CUAssertLog(false,"AudioNode has wrong number of channels: %d vs %d",
                    node->getChannels(),_channels);
        return false;
    } else if (node->getRate() != _sampling) {
        CUAssertLog(false, "Input node has wrong sample rate: %d", node->getRate());
        return false;
    }

    std::atomic_exchange_explicit(&_input, node, std::memory_order_relaxed);
    return true;
}

/**
 * Detaches an audio node from this reverb node.
 *
 * If the method succeeds, it returns the audio node that was removed.
 *
 * @return  The audio node to detach (or null if failed)
 */
std::shared_ptr<AudioNode> AlgorithmicReverb::detach() {
    if (!_booted) {
        CUAssertLog(_booted, "Cannot detach from an uninitialized audio node");
        return nullptr;
    }

    std::shared_ptr<AudioNode> result = std::atomic_exchange_explicit(&_input, {}, std::memory_order_relaxed);
    ATK_ResetAlgoReverb(_reverb);
    return result;
}


/**
 * Clears all filters in the reverb subgraph.
 */
void AlgorithmicReverb::clear() {
    ATK_ResetAlgoReverb(_reverb);
}

/**
 * Reads up to the specified number of frames into the given buffer
 *
 * AUDIO THREAD ONLY: Users should never access this method directly.
 * The only exception is when the user needs to create a custom subclass
 * of this AudioNode.
 *
 * The buffer should have enough room to store frames * channels elements.
 * The channels are interleaved into the output buffer.
 *
 * This method will always forward the read position after reading. Reading
 * again may return different data.
 *
 * @param buffer    The read buffer to store the results
 * @param frames    The maximum number of frames to read
 *
 * @return the actual number of frames read
 */
Uint32 AlgorithmicReverb::read(float* buffer, Uint32 frames) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_dirty.load(std::memory_order_relaxed)) {
        updateReverb();
    }
    
    std::shared_ptr<AudioNode> input = _input;
    Uint32 actual = 0;
    if (input == nullptr || _paused.load(std::memory_order_relaxed)) {
        std::memset(buffer, 0, frames*_channels * sizeof(float));
        actual = frames;
    } else if (_fadeout > 0) {
        actual = std::min(frames,(Uint32)_fadeout);
        std::memset(buffer, 0, actual*_channels * sizeof(float));
        float start = (float)_fadeout/(float)_outmark;
        float ends  = (float)(_fadeout-actual)/(float)_outmark;
        _fadeout -= actual;
        _outdone = (_fadeout == 0);

        ATK_ApplyAlgoReverb(_reverb, buffer, buffer, actual);
        ATK_VecSlide(buffer,start,ends,buffer,actual*_channels);
        if (_ndgain != 1) {
            ATK_VecScale(buffer, _ndgain, buffer, _channels*actual);
        }
    } else if (!_outdone) {
        actual = input->read(buffer, frames);
        Uint32 fadeidx = actual;
        if ((actual < frames || input->completed()) && _outmark > 0) {
            Uint32 remain = frames - actual;
            remain = (Uint32)(remain < _outmark ? remain : _outmark);
            std::memset(buffer+actual*_channels, 0, remain*sizeof(float)*_channels);
            actual += remain;
            _fadeout = _outmark-remain;
            _outdone = _fadeout == 0;
        }

        ATK_ApplyAlgoReverb(_reverb, buffer, buffer, actual);
        if (fadeidx < actual) {
            Uint32 left = std::min(actual-fadeidx,(Uint32)_fadeout);
            float start = (float)_fadeout/(float)_outmark;
            float ends  = (float)(_fadeout-left)/(float)_outmark;
            ATK_VecSlide(buffer+fadeidx*_channels,start,ends,
                         buffer+fadeidx*_channels,left*_channels);
        }
        if (_ndgain != 1) {
            ATK_VecScale(buffer, _ndgain.load(std::memory_order_relaxed), buffer, _channels*actual);
        }
    }
    return actual;
}

/**
 * Sets the typical read size of this node.
 *
 * Some audio nodes need an internal buffer for operations like mixing or
 * resampling. In that case, it helps to know the requested {@link read}
 * size ahead of time. The capacity is the minimal required read amount
 * of the {@link AudioEngine} and corresponds to {@link AudioEngine#getReadSize}.
 *
 * It is not actually necessary to set this size. However for nodes with
 * internal buffer, setting this value can optimize performance.
 *
 * This method is not synchronized because it is assumed that this value
 * will **never** change while the audio engine in running. The average
 * user should never call this method explicitly. You should always call
 * {@link AudioEngine#setReadSize} instead.
 *
 * @param size  The typical read size of this node.
 */
void AlgorithmicReverb::setReadSize(Uint32 size) {
    if (_readsize != size) {
        _readsize = size;
        std::shared_ptr<AudioNode> temp = _input;
        if (temp != nullptr) {
            temp->setReadSize(_readsize);
        }
    }
}

/**
 * Recalculate internal values after parameter change
 */
void AlgorithmicReverb::updateReverb() {
    
    ATK_AlgoReverbDef settings;
    settings.wet = _wet.load(std::memory_order_relaxed);
    settings.dry = _dry.load(std::memory_order_relaxed);
    settings.width = _width.load(std::memory_order_relaxed);
    settings.damping = _damp.load(std::memory_order_relaxed);
    settings.ingain  = _ingain.load(std::memory_order_relaxed);
    settings.roomsize = _roomsize.load(std::memory_order_relaxed);

    ATK_UpdateAlgoReverb(_reverb, &settings);
}


/**
 * Sets the room size associated this reverb filter.
 *
 * Filters cannot have unique room sizes.
 *
 * @param value  the room size as a float.
 */
void AlgorithmicReverb::setRoomSize(float value) {
    _roomsize = value;
    _dirty = true;
}

/**
 * Returns the room size associated this reverb filter.
 *
 * Filters cannot have unique room sizes.
 *
 * @return the room size associated this reverb filter.
 */
float AlgorithmicReverb::getRoomSize() {
    return _roomsize.load(std::memory_order_relaxed);
}

/**
 * Sets the damping associated with each comb filter.
 *
 * Filters cannot have unique damping.
 *
 * @param value  the damping as a float.
 */
void AlgorithmicReverb::setDamp(float value) {
    _damp = value;
    _dirty = true;
}

/**
 * Returns the damping associated with each comb filter.
 *
 * Filters cannot have unique room sizes.
 *
 * @return the damping as a float.
 */
float AlgorithmicReverb::getDamp() {
    return _damp.load(std::memory_order_relaxed);

}

/**
 * Sets the wetness scale for the reverb. Should be a value
 * between 0 and 1. A value of 0 will leave the input unchanged.
 *
 * @param value  the amount of wet to mix as a float.
 */
void AlgorithmicReverb::setWet(float value) {
    _wet = value;
    _dirty = true;
}

/**
 * Returns the wetness scale for the reverb.
 *
 * @return the amount of wet to mix as a float between 0 and 1
 */
float AlgorithmicReverb::getWet() {
    return _wet.load(std::memory_order_relaxed);
}

/**
 * Sets the dryness scale for the reverb. Should be a value
 * between 0 and 1. A value of 0 will output only the wet mix.
 *
 * @param value  the amount of dry to mix as a float.
 */
void AlgorithmicReverb::setDry(float value) {
    _dry = value;
    _dirty = true;
}

/**
 * Returns the wetness scale for the reverb.
 *
 * @return the amount of dry to mix as a float between 0 and 1
 */
float AlgorithmicReverb::getDry() {
    return _dry.load(std::memory_order_relaxed);
}

/**
     * Sets the width between stereo channels
     *
     * @param value  the distance between channels
     */
void AlgorithmicReverb::setWidth(float value) {
    _width = value;
    _dirty = true;
}

/**
 * Returns the width between stereo channels
 *
 * @return the distance between channels
 */
float AlgorithmicReverb::getWidth() {
    return _width.load(std::memory_order_relaxed);
}

/**
 * Sets the fade-out tail for this reverb node
 *
 * A reverb node is technically complete when its input node is
 * complete. But for long enough echoes, this can cause the echo
 * to be cut off. Therefore, it makes sense to add a tail where
 * the echo is allowed to persist a little bit longer. This echo
 * will linearly fade to 0 over the tail duration.
 *
 * If this value is 0 or less, there will be no tail. You should
 * not add a tail if you want this sound to be looped with the echo.
 *
 * @param duration  The fade-out tail in seconds
 */
void AlgorithmicReverb::setTail(double duration) {
    std::lock_guard<std::mutex> lock(_mutex);
    _outmark = (Sint64)(duration*_sampling);
    _fadeout = 0;
    _outdone = false;
}

/**
 * Returns the fade-out tail for this reverb node
 *
 * A reverb node is technically complete when its input node is
 * complete. But for long enough echoes, this can cause the echo
 * to be cut off. Therefore, it makes sense to add a tail where
 * the echo is allowed to persist a little bit longer. This echo
 * will linearly fade to 0 over the tail duration.
 *
 * If this value is 0 or less, there will be no tail. You should
 * not add a tail if you want this sound to be looped with the echo.
 *
 * @return the fade-out tail for this reverb node
 */
double AlgorithmicReverb::getTail() {
    std::lock_guard<std::mutex> lock(_mutex);
    return ((double)_outmark)/_sampling;
}

#pragma mark -
#pragma mark Optional Methods
/**
 * Returns true if this audio node has no more data.
 *
 * An audio node is typically completed if it return 0 (no frames read) on
 * subsequent calls to {@link read()}.  However, for infinite-running
 * audio threads, it is possible for this method to return true even when
 * data can still be read; in that case the node is notifying that it
 * should be shut down.
 *
 * @return true if this audio node has no more data.
 */
bool AlgorithmicReverb::completed() {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->completed() && _outdone;
    }
    return true;
}

/**
 * Marks the current read position in the audio steam.
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns false if there is no input node, indicating it is unsupported.
 *
 * This method is typically used by {@link reset()} to determine where to
 * restore the read position. For some nodes (like {@link AudioInput}),
 * this method may start recording data to a buffer, which will continue
 * until {@link clear()} is called.
 *
 * It is possible for {@link reset()} to be supported even if this method
 * is not.
 *
 * @return true if the read position was marked.
 */
bool AlgorithmicReverb::mark() {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->mark();
    }
    return false;
}

/**
 * Clears the current marked position.
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns false if there is no input node, indicating it is unsupported.
 *
 * If the method {@link mark()} started recording to a buffer (such as
 * with {@link AudioInput}), this method will stop recording and release
 * the buffer.  When the mark is cleared, {@link reset()} may or may not
 * work depending upon the specific node.
 *
 * @return true if the read position was marked.
 */
bool AlgorithmicReverb::unmark() {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->unmark();
    }
    return false;
}

/**
 * Resets the read position to the marked position of the audio stream.
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns false if there is no input node, indicating it is unsupported.
 *
 * When no {@link mark()} is set, the result of this method is node
 * dependent.  Some nodes (such as {@link AudioPlayer}) will reset to the
 * beginning of the stream, while others (like {@link AudioInput}) only
 * support a rest when a mark is set. Pay attention to the return value of
 * this method to see if the call is successful.
 *
 * @return true if the read position was moved.
 */
bool AlgorithmicReverb::reset() {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->reset();
    }
    _outdone = false;
    _fadeout = 0;
    return false;
}

/**
 * Advances the stream by the given number of frames.
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns -1 if there is no input node, indicating it is unsupported.
 *
 * This method only advances the read position, it does not actually
 * read data into a buffer. This method is generally not supported
 * for nodes with real-time input like {@link AudioInput}.
 *
 * @param frames    The number of frames to advace
 *
 * @return the actual number of frames advanced; -1 if not supported
 */
Sint64 AlgorithmicReverb::advance(Uint32 frames) {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->advance(frames);
    }
    return -1;
}

/**
 * Returns the current frame position of this audio node
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns -1 if there is no input node, indicating it is unsupported.
 *
 * In some nodes like {@link AudioInput}, this method is only supported
 * if {@link mark()} is set.  In that case, the position will be the
 * number of frames since the mark. Other nodes like {@link AudioPlayer}
 * measure from the start of the stream.
 *
 * @return the current frame position of this audio node.
 */
Sint64 AlgorithmicReverb::getPosition() const {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->getPosition();
    }
    return -1;
}

/**
 * Sets the current frame position of this audio node.
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns -1 if there is no input node, indicating it is unsupported.
 *
 * In some nodes like {@link AudioInput}, this method is only supported
 * if {@link mark()} is set.  In that case, the position will be the
 * number of frames since the mark. Other nodes like {@link AudioPlayer}
 * measure from the start of the stream.
 *
 * @param position  the current frame position of this audio node.
 *
 * @return the new frame position of this audio node.
 */
Sint64 AlgorithmicReverb::setPosition(Uint32 position) {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->setPosition(position);
    }
    return -1;
}

/**
 * Returns the elapsed time in seconds.
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns -1 if there is no input node, indicating it is unsupported.
 *
 * In some nodes like {@link AudioInput}, this method is only supported
 * if {@link mark()} is set.  In that case, the times will be the
 * number of seconds since the mark. Other nodes like {@link AudioPlayer}
 * measure from the start of the stream.
 *
 * @return the elapsed time in seconds.
 */
double AlgorithmicReverb::getElapsed() const {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->getElapsed();
    }
    return -1;
}

/**
 * Sets the read position to the elapsed time in seconds.
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns -1 if there is no input node, indicating it is unsupported.
 *
 * In some nodes like {@link AudioInput}, this method is only supported
 * if {@link mark()} is set.  In that case, the new time will be meaured
 * from the mark. Other nodes like {@link AudioPlayer} measure from the
 * start of the stream.
 *
 * @param time  The elapsed time in seconds.
 *
 * @return the new elapsed time in seconds.
 */
double AlgorithmicReverb::setElapsed(double time) {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->setElapsed(time);
    }
    return -1;
}

/**
 * Returns the remaining time in seconds.
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns -1 if there is no input node or if this method is unsupported
 * in that node
 *
 * In some nodes like {@link AudioInput}, this method is only supported
 * if {@link setRemaining()} has been called.  In that case, the node will
 * be marked as completed after the given number of seconds.  This may or may
 * not actually move the read head.  For example, in {@link AudioPlayer} it
 * will skip to the end of the sample.  However, in {@link AudioInput} it
 * will simply time out after the given time.
 *
 * @return the remaining time in seconds.
 */
double AlgorithmicReverb::getRemaining() const {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->getRemaining()+_outmark/(float)_sampling;
    }
    return -1;
}

/**
 * Sets the remaining time in seconds.
 *
 * DELEGATED METHOD: This method delegates its call to the input node.  It
 * returns -1 if there is no input node or if this method is unsupported
 * in that node
 *
 * If this method is supported, then the node will be marked as completed
 * after the given number of seconds.  This may or may not actually move
 * the read head.  For example, in {@link AudioPlayer} it will skip to the
 * end of the sample.  However, in {@link AudioInput} it will simply time
 * out after the given time.
 *
 * @param time  The remaining time in seconds.
 *
 * @return the new remaining time in seconds.
 */
double AlgorithmicReverb::setRemaining(double time) {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->setRemaining(time-_outmark/(float)_sampling);
    }
    return -1;
}


