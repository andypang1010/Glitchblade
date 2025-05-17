//
//  CUAlgorthmicReverb.h
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
#ifndef __CU_ALGORITHMIC_REVERB_H__
#define __CU_ALGORITHMIC_REVERB_H__
#include <cugl/audio/graph/CUAudioNode.h>
#include <mutex>
#include <SDL_atk.h>

namespace cugl {

    /**
     * The classes supporting sound playback and recording.
     *
     * While sound is an important part of most games, it is less important
     * in early prototypes. Therefore, we have factored this subsystem out
     * to reduce the application footprint. Note that even without these
     * classes, it is still possible to play audio using the SDL API.
     */
    namespace audio {
    
/**
 * This class provides an algorithmic implementation of audio reverb.
 *
 * The implementation is modeled after the the open source Schroeder
 * reverberator, Freeverb. It is tunable with several attributes, including
 * wet/dry mix, damping, and room size. All of attributes except tail should
 * be between 0 and 1. More information about the algorithm can be found at:
 *
 *     https://ccrma.stanford.edu/~jos/pasp/Freeverb.html
 */
class AlgorithmicReverb : public AudioNode {
protected:
    /** The audio input node */
    std::shared_ptr<AudioNode> _input;

    /** This class needs a proper lock guard; too many race conditions */
    std::mutex _mutex;

    /** internal gain for producing wet mix */
    std::atomic<float> _ingain;

    /** Scales gain for the wet mix (stereo) */
    std::atomic<float> _wet;

    /** Scales gain for the dry mix */
    std::atomic<float> _dry;

    /** Sets the amount of feedback for the comb filters (wet tail length)*/
    std::atomic<float> _roomsize;

    /** Amount that the wet mix is damped */
    std::atomic<float> _damp;

    /** distance between left and right channels */
    std::atomic<float> _width;
    
    /** Whether the reverb settings have changed and need to be regenerated */
    std::atomic<bool> _dirty;

    /** The reverb filter from ATK */
    ATK_AlgoReverb* _reverb;

    /** The number of frames to fade-out; -1 if no active fade-out */
    Sint64 _outmark;
    /** The amount of fade-out remaining */
    Uint64 _fadeout;
    /** Whether we have completed this node due to a fadeout */
    bool   _outdone;
    
    /**
     * Initializes the reverb filter from the default settings.
     */
    void initFilter();

#pragma mark Constructors
public:
    /**
     * Creates a degenerate reverb node with no associated input.
     *
     * The node has no settings and so will not provide any reverb.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a graph node on
     * the heap, use one of the static constructors instead.
     */
    AlgorithmicReverb();

    /**
     * Deletes this node, disposing of all resources.
     */
    ~AlgorithmicReverb() { dispose(); }

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
    virtual bool init() override;

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
    virtual bool init(Uint8 channels, Uint32 rate) override;

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
    bool init(const std::shared_ptr<AudioNode>& input);

    /**
     * Disposes any resources allocated for this player
     *
     * The state of the node is reset to that of an uninitialized constructor.
     * Unlike the destructor, this method allows the node to be reinitialized.
     */
    virtual void dispose() override;

#pragma mark Static Constructors
    /**
     * Returns a newly allocated reverb with the default stereo settings
     *
     * The number of channels is two, for stereo output.  The sample rate is
     * the modern standard of 48000 HZ. Any input node must agree with these
     * settings.
     *
     * The reverb will be set with the default settings as defined by the
     * public domain FreeVerb algorithm.
     *
     * @return a newly allocated reverb node with the default stereo settings
     */
    static std::shared_ptr<AlgorithmicReverb> alloc() {
        std::shared_ptr<AlgorithmicReverb> result = std::make_shared<AlgorithmicReverb>();
        return (result->init() ? result : nullptr);
    }

    /**
     * Returns a newly allocated reverb with the given number of channels and sample rate
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
     * @return a newly allocated reverb node with the given number of channels and sample rate
     */
    static std::shared_ptr<AlgorithmicReverb> alloc(Uint8 channels, Uint32 rate) {
        std::shared_ptr<AlgorithmicReverb> result = std::make_shared<AlgorithmicReverb>();
        return (result->init(channels, rate) ? result : nullptr);
    }

    /**
     * Returns a newly allocated reverb for the given input node.
     *
     * This node acquires the channels and sample rate of the input.  If
     * input is nullptr, this method will fail.
     *
     * The reverb will be set with the default settings as defined by the
     * public domain FreeVerb algorithm.
     *
     * @param input     The audio node to fade
     *
     * @return a newly allocated reverb node for the given input node.
     */
    static std::shared_ptr<AlgorithmicReverb> alloc(const std::shared_ptr<AudioNode>& input) {
        std::shared_ptr<AlgorithmicReverb> result = std::make_shared<AlgorithmicReverb>();
        return (result->init(input) ? result : nullptr);
    }

#pragma mark Audio Graph Methods
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
    bool attach(const std::shared_ptr<AudioNode>& node);

    /**
     * Detaches an audio node from this reverb node.
     *
     * If the method succeeds, it returns the audio node that was removed.
     *
     * @return  The audio node to detach (or null if failed)
     */
    std::shared_ptr<AudioNode> detach();

    /**
     * Returns the input node of this reverb node.
     *
     * @return the input node of this reverb node.
     */
    std::shared_ptr<AudioNode> getInput() { return _input; }

    /**
     * Clears all filters in the reverb subgraph.
     */
    void clear();

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
    virtual void setReadSize(Uint32 size) override;
    
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
    virtual Uint32 read(float* buffer, Uint32 frames) override;
        
#pragma mark Reverb Attributes
    /**
     * Sets the room size associated with each comb filter.
     *
     * This value should be between 0 and 1 for best effects.
     *
     * @param value  the room size as a float.
     */
    void setRoomSize(float value);

    /**
     * Returns the room size associated with each comb filter.
     *
     * This value should be between 0 and 1 for best effects.
     *
     * @return the room size as a float.
     */
    float getRoomSize();

    /**
     * Sets the damping associated with each comb filter.
     *
     * This value should be between 0 and 1 for best effects.
     *
     * @param value  the damping as a float.
     */
    void setDamp(float value);

    /**
     * Returns the damping associated with each comb filter.
     *
     * This value should be between 0 and 1 for best effects.
     *
     * @return the damping as a float.
     */
    float getDamp();

    /**
     * Sets the wetness scale for the reverb.
     *
     * This should be a value between 0 and 1. A value of 0 will mean that
     * no reverb is applied.
     *
     * @param value  the amount of wet to mix as a float.
     */
    void setWet(float value);

    /**
     * Returns the wetness scale for the reverb.
     *
     * This should be a value between 0 and 1. A value of 0 will mean that
     * no reverb is applied.
     *
     * @return  the wetness scale for the reverb.
     */
    float getWet();

    /**
     * Sets the dryness scale for the reverb.
     *
     * This should be a value between 0 and 1. A value of 0 will mean that
     * only the wet mix (reverb) is played.
     *
     * @param value  the amount of dry to mix as a float.
     */
    void setDry(float value);

    /**
     * Returns the dryness scale for the reverb.
     *
     * This should be a value between 0 and 1. A value of 0 will mean that
     * only the wet mix (reverb) is played.
     *
     * @return the dryness scale for the reverb.
     */
    float getDry();

    /**
     * Sets the width between the stereo channels
     *
     * @param value  the distance between channels
     */
    void setWidth(float value);

    /**
     * Returns the width between the stereo channels
     *
     * @return the width between the stereo channels
     */
    float getWidth();
    
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
    void setTail(double duration);

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
    double getTail();

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
    virtual bool completed() override;
    
    /**
     * Marks the current read position in the audio steam.
     *
     * DELEGATED METHOD: This method delegates its call to the input node.  It
     * returns false if there is no input node or if this method is unsupported
     * in that node
     *
     * This method is typically used by {@link reset()} to determine where to
     * restore the read position. For some nodes (like {@link AudioInput}),
     * this method may start recording data to a buffer, which will continue
     * until {@link reset()} is called.
     *
     * It is possible for {@link reset()} to be supported even if this method
     * is not.
     *
     * @return true if the read position was marked.
     */
    virtual bool mark() override;
    
    /**
     * Clears the current marked position.
     *
     * DELEGATED METHOD: This method delegates its call to the input node.  It
     * returns false if there is no input node or if this method is unsupported
     * in that node
     *
     * If the method {@link mark()} started recording to a buffer (such as
     * with {@link AudioInput}), this method will stop recording and release
     * the buffer.  When the mark is cleared, {@link reset()} may or may not
     * work depending upon the specific node.
     *
     * @return true if the read position was marked.
     */
    virtual bool unmark() override;
    
    /**
     * Resets the read position to the marked position of the audio stream.
     *
     * DELEGATED METHOD: This method delegates its call to the input node.  It
     * returns false if there is no input node or if this method is unsupported
     * in that node
     *
     * When no {@link mark()} is set, the result of this method is node
     * dependent.  Some nodes (such as {@link AudioPlayer}) will reset to the
     * beginning of the stream, while others (like {@link AudioInput}) only
     * support a rest when a mark is set. Pay attention to the return value of
     * this method to see if the call is successful.
     *
     * @return true if the read position was moved.
     */
    virtual bool reset() override;
    
    /**
     * Advances the stream by the given number of frames.
     *
     * DELEGATED METHOD: This method delegates its call to the input node.  It
     * returns -1 if there is no input node or if this method is unsupported
     * in that node
     *
     * This method only advances the read position, it does not actually
     * read data into a buffer. This method is generally not supported
     * for nodes with real-time input like {@link AudioInput}.
     *
     * @param frames    The number of frames to advace
     *
     * @return the actual number of frames advanced; -1 if not supported
     */
    virtual Sint64 advance(Uint32 frames) override;
    
    /**
     * Returns the current frame position of this audio node
     *
     * DELEGATED METHOD: This method delegates its call to the input node.  It
     * returns -1 if there is no input node or if this method is unsupported
     * in that node
     *
     * In some nodes like {@link AudioInput}, this method is only supported
     * if {@link mark()} is set.  In that case, the position will be the
     * number of frames since the mark. Other nodes like {@link AudioPlayer}
     * measure from the start of the stream.
     *
     * @return the current frame position of this audio node.
     */
    virtual Sint64 getPosition() const override;
    
    /**
     * Sets the current frame position of this audio node.
     *
     * DELEGATED METHOD: This method delegates its call to the input node.  It
     * returns -1 if there is no input node or if this method is unsupported
     * in that node
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
    virtual Sint64 setPosition(Uint32 position) override;
    
    /**
     * Returns the elapsed time in seconds.
     *
     * DELEGATED METHOD: This method delegates its call to the input node.  It
     * returns -1 if there is no input node or if this method is unsupported
     * in that node
     *
     * In some nodes like {@link AudioInput}, this method is only supported
     * if {@link mark()} is set.  In that case, the times will be the
     * number of seconds since the mark. Other nodes like {@link AudioPlayer}
     * measure from the start of the stream.
     *
     * @return the elapsed time in seconds.
     */
    virtual double getElapsed() const override;
    
    /**
     * Sets the read position to the elapsed time in seconds.
     *
     * DELEGATED METHOD: This method delegates its call to the input node.  It
     * returns -1 if there is no input node or if this method is unsupported
     * in that node
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
    virtual double setElapsed(double time) override;
    
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
    virtual double getRemaining() const override;
    
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
    virtual double setRemaining(double time) override;
    
#pragma mark Reverb Support
private:
    /**
     * Recalculates the reverb filter after a setting change.
     */
    void updateReverb();

};
    }
}
#endif /* __CU_ALGORITHMIC_REVERB_H__ */
