//
//  CUAudioPanner.cpp
//  Cornell University Game Library (CUGL)
//
//  This module is a general purpose audio panner.  It can convert an audio node
//  with any given number of channels to one with a different number of channels
//  (but the same sampling rate).  It does this via a panning matrix.  This
//  matrix specifies the contribution (in a range of 0 to 1) of each input
//  channel to each output channel.
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
#include <cugl/audio/graph/CUAudioPanner.h>
#include <cugl/audio/CUAudioDevices.h>
#include <cugl/core/util/CUDebug.h>
#include <cmath>

using namespace cugl::audio;

/**
 * Creates a degenerate audio panner
 *
 * The node has no channels, so read options will do nothing. The node must
 * be initialized to be used.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a graph node on
 * the heap, use one of the static constructors instead.
 */
AudioPanner::AudioPanner() : AudioNode(),
_field(0),
_identity(true),
_mapper(nullptr) {
    _input = nullptr;
    _classname = "AudioPanner";
}

/**
 * Initializes the node with default stereo settings
 *
 * The number of input channels (the field) and the number of output
 * channels is two, for stereo output.  The sample rate is the modern standard
 * of 48000 HZ.
 *
 * This initializer will create a default stereo panner.  The initial
 * panning matrix will map left to left and right to right.
 *
 * @return true if initialization was successful
 */
bool AudioPanner::init() {
    return init(DEFAULT_CHANNELS,DEFAULT_CHANNELS,DEFAULT_SAMPLING);
}

/**
 * Initializes the node with the given number of channels and sample rate
 *
 * The number of input channels (the field) and the number of output
 * channels will be the same.  The initial panning matrix will map each
 * channel to itself.  This is a generalization of a default stereo panner.
 *
 * @param channels  The number of audio channels
 * @param rate      The sample rate (frequency) in HZ
 *
 * @return true if initialization was successful
 */
bool AudioPanner::init(Uint8 channels, Uint32 rate) {
    return init(channels,channels,rate);
}

/**
 * Initializes the node with the given number of input/output channels.
 *
 * The number of input channels is given by `field`, while `channels` is
 * the number of output channels. The initial panning matrix will map each
 * channel to itself, and drop those input channels that do not have a
 * corresponding output channel.
 *
 * @param channels  The number of output channels
 * @param field     The number of input channels
 * @param rate      The sample rate (frequency) in HZ
 *
 * @return true if initialization was successful
 */
bool AudioPanner::init(Uint8 channels, Uint8 field, Uint32 rate) {
    if (AudioNode::init(channels,rate)) {
        setField(field);
        _buffer = (float*)malloc(_readsize*_field*sizeof(float));
        std::memset(_buffer,0,_readsize*_field*sizeof(float));
        return true;
    }
    return false;
}

/**
 * Initializes a panner for the given input node.
 *
 * This node acquires the channels and sample rate of the input.  If
 * input is nullptr, this method will fail.
 *
 * @param channels  The number of output channels
 * @param input     The audio node to pan
 *
 * @return true if initialization was successful
 */
bool AudioPanner::init(Uint8 channels, const std::shared_ptr<AudioNode>& input) {
    if (input && AudioNode::init(channels,input->getRate())) {
        setField(input->getChannels());
        _buffer = (float*)malloc(_readsize*_field*sizeof(float));
        std::memset(_buffer,0,_readsize*_field*sizeof(float));
        attach(input);
        return true;
    }
    return false;
}

/**
 * Disposes any resources allocated for this panner
 *
 * The state of the node is reset to that of an uninitialized constructor.
 * Unlike the destructor, this method allows the node to be reinitialized.
 */
void AudioPanner::dispose() {
    if (_booted) {
        AudioNode::dispose();
        if (_mapper != nullptr) {
            delete[] _mapper;
            _mapper = nullptr;
        }
        if (_buffer != nullptr) {
            free(_buffer);
            _buffer = nullptr;
        }
        _input = nullptr;
        _field = 0;
    }
}

#pragma mark -
#pragma mark Audio Graph
/**
 * Attaches an audio node to this panner.
 *
 * This method will fail if the channels of the audio node do not agree
 * with the field size of this panner
 *
 * @param node  The audio node to pan
 *
 * @return true if the attachment was successful
 */
bool AudioPanner::attach(const std::shared_ptr<AudioNode>& node) {
    if (!_booted) {
        CUAssertLog(_booted, "Cannot attach to an uninitialized audio node");
        return false;
    } else if (node == nullptr) {
        detach();
        return true;
    } else if (node->getChannels() != _field) {
        CUAssertLog(false,"Input node has wrong number of channels: %d", node->getChannels());
        return false;
    } else if (node->getRate() != _sampling) {
        CUAssertLog(false,"Input node has wrong sample rate: %d", node->getRate());
        return false;
    }
    
    // Reset the read size if necessary
    if (node->getReadSize() != _readsize) {
        node->setReadSize(_readsize);
    }
    
    std::atomic_exchange_explicit(&_input,node,std::memory_order_relaxed);
    return true;
}

/**
 * Detaches an audio graph from this output node.
 *
 * This method will fail if the output node is playing.  You must stop the
 * output node to reconfigure the audio graph.  If the method succeeds, it
 * returns the terminal node of the audio graph.
 *
 * @return  the terminal node of the audio graph (or null if failed)
 */
std::shared_ptr<AudioNode> AudioPanner::detach() {
    if (!_booted) {
        CUAssertLog(_booted, "Cannot detach from an uninitialized audio node");
        return nullptr;
    }
    
    std::shared_ptr<AudioNode> result = std::atomic_exchange_explicit(&_input,{},std::memory_order_relaxed);
    return result;
}

/**
 * Sets the input field size of this panner.
 *
 * The field can only be reset if there is no attached node.  Otherwise
 * this method will fail
 *
 * @param field     The number of input channels
 *
 * @return true if the field was successfully reset
 */
bool AudioPanner::setField(Uint8 field) {
    if (_input != nullptr) {
        CUAssertLog(false, "Cannot set the field on an active panner");
        return false;
    }
    
    _field  = field;
    _mapper = new std::atomic<float>[field*_channels];
    for(int ii = 0; ii < field; ii++) {
        for(int jj = 0; jj < _channels; jj++) {
            if (ii == jj) {
                _mapper[ii*_channels+jj] = 1;
            } else {
                _mapper[ii*_channels+jj] = 0;
            }
        }
    }
    _identity = (field == _channels);
    return true;
}

/**
 * Returns the matrix pan value for input field and output channel.
 *
 * The pan value is the percentage (gain) of the input channel (field)
 * that is sent to the given output channel.  Technically, this value
 * can be more than 1, but it cannot be negative.
 *
 * @return the matrix pan value for input field and output channel.
 */
float AudioPanner::getPan(Uint32 field, Uint32 channel) const {
    return _mapper[field*_channels+channel].load(std::memory_order_relaxed);
}


/**
 * Awra the matrix pan value for input field and output channel.
 *
 * The pan value is the percentage (gain) of the input channel (field)
 * that is sent to the given output channel.  Technically, this value
 * can be more than 1, but it cannot be negative.
 *
 * @param field     The input channel
 * @param channel   The output channel
 * @param value     The percentage gain
 */
void AudioPanner::setPan(Uint32 field, Uint32 channel, float value) {
    CUAssertLog(field < _field, "Field %d is out of range",field);
    CUAssertLog(channel < _channels, "Channel %d is out of range",channel);
    _identity.store(false,std::memory_order_relaxed);
    _mapper[field*_channels+channel].store(value,std::memory_order_relaxed);
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
void AudioPanner::setReadSize(Uint32 size) {
    _readsize = size;
    if (_buffer != nullptr) {
        free(_buffer);
        _buffer = nullptr;
    }
    _buffer = (float*)malloc(_readsize*_field*sizeof(float));
    std::memset(_buffer,0,_readsize*_field*sizeof(float));

    std::shared_ptr<AudioNode> node = _input;
    if (node != nullptr) {
        node->setReadSize(size);
    }
}


#pragma mark -
#pragma mark Playback Control
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
bool AudioPanner::completed() {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    return (input == nullptr || input->completed());
}

/**
 * Reads up to the specified number of frames into the given buffer
 *
 * AUDIO THREAD ONLY: Users should never access this method directly.
 * The only exception is when the user needs to create a custom subclass
 * of this AudioOutput.
 *
 * The buffer should have enough room to store frames * channels elements.
 * The channels are interleaved into the output buffer.
 *
 * This method will always forward the read position.
 *
 * @param buffer    The read buffer to store the results
 * @param frames    The maximum number of frames to read
 *
 * @return the actual number of frames read
 */
Uint32 AudioPanner::read(float* buffer, Uint32 frames) {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    bool identity = _identity.load(std::memory_order_relaxed);
    if (input == nullptr || _paused.load(std::memory_order_relaxed)) {
        std::memset(buffer,0,frames*_channels*sizeof(float));
        return frames;
    } else if (identity) {
        return input->read(buffer, frames);
    } else {
        Uint32 actual = 0;
        Uint32 remain = frames;
        float* current = buffer;
        while (remain > 0) {
            Uint32 chunk = std::min(remain,_readsize);
            std::memset(current,0,chunk*_channels*sizeof(float));
            Uint32 taken = input->read(_buffer, chunk);
            for(int ii = 0; ii < _field; ii++) {
                for(int jj = 0; jj < _channels; jj++) {
                    float percent =  _mapper[ii*_channels+jj].load(std::memory_order_relaxed);
                    if (percent > 0) {
                        float* output = current+jj;
                        float* input  = _buffer+ii;
                        Uint32 amt = taken;
                        while (amt--) {
                            *output += *input*percent;
                            output += _channels;
                            input  += _field;
                        }
                    }
                }
            }
            if (taken == 0) {
                remain = 0;
            } else {
                remain -= taken;
            }
            actual += taken;
            current += taken*_channels;
        }
        return actual;
    }
}

#pragma mark -
#pragma mark Optional Methods
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
bool AudioPanner::mark() {
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
bool AudioPanner::unmark() {
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
bool AudioPanner::reset() {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->reset();
    }
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
Sint64 AudioPanner::advance(Uint32 frames) {
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
Sint64 AudioPanner::getPosition() const {
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
Sint64 AudioPanner::setPosition(Uint32 position) {
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
double AudioPanner::getElapsed() const {
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
double AudioPanner::setElapsed(double time) {
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
double AudioPanner::getRemaining() const {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->getRemaining();
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
double AudioPanner::setRemaining(double time) {
    std::shared_ptr<AudioNode> input = std::atomic_load_explicit(&_input,std::memory_order_relaxed);
    if (input) {
        return input->setRemaining(time);
    }
    return -1;
}
