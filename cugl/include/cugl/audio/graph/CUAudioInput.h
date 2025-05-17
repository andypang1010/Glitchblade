//
//  CUAudioInput.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides interface to an audio input device. This makes it
//  typically a leaf node in the audio stream DAG.  This can be used to
//  record audio, analyze the audio, or even play it back in real time.
//  However, all real-time processing should be cognizant of the (necessary)
//  latency in playing back the input.  Also, this node does not handle
//  audio feedback well at all.
//
//  The audio graph and its nodes will always be accessed by two threads: the
//  main thread and the audio thread.  The audio graph is designed to safely
//  coordinate between these two threads.  However, it is minimizes locking
//  and instead relies on a fail-fast model.  If part of the audio graph is
//  not in a state to be used by the audio thread, it will skip over that part
//  of the graph until the next render frame.  Hence some changes should only
//  be made if the graph is paused.  When there is some question about the
//  thread safety, the methods are clearly marked.
//
//  It is NEVER safe to access the audio graph outside of the main thread. The
//  coordination algorithms only assume coordination between two threads.
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
#ifndef __CU_AUDIO_INPUT_H__
#define __CU_AUDIO_INPUT_H__
#include <SDL.h>
#include <cugl/audio/graph/CUAudioNode.h>
#include <unordered_set>
#include <string>
#include <memory>
#include <vector>
#include <deque>
#include <mutex>

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
    
    /** Forward reference to the audio manager */
    class AudioDevices;
    /** Forward reference to an audio sample */
    class AudioSample;

/**
 * This class provides a graph node interface for an audio recording device.
 *
 * This audio node provides a modern, graph-based approach to sound design.
 * This input can be plugged into the audio graph and piped to the output
 * device with a small (but significant) amount of latency.  This node is
 * intended to be the root of an audio DAG, much like {@link AudioPlayer}.
 *
 * One of the important issues to understand about this class is that there
 * are actually two different latency values: the recording latency and the
 * playback latency.  The recording latency is time from when the data is
 * captured at the hardware device and is recorded at this node.  It is
 * entirely determined by the buffer size of this device, much like the
 * buffer determines the playback latency for {@link AudioOutput}.  However,
 * unlike AudioOutput, there is no guarantee that recording happens at uniform
 * time intervals. For example, on MacOS at 48000 Hz, a buffer size of 512 will
 * record twice (in immediate sucession) every 21 ms instead of once every
 * 11 ms (a buffer size of 1024 appers to record once every 21 ms as expected).
 *
 * The playback latency is the time from when data is passed to {@link record()}
 * and made available to {@link read()}; it does not include the latency of
 * any output device.  It is determined by the additional delay value, which is
 * the number of frames that must be recorded before any can be read.  So a
 * delay of 0 means that data is instantly available, while a delay of the
 * buffer size means that no data can be read until more than a single buffer
 * has been recorded (which means that at least two buffers worth of data must
 * have been recorded).  Because output and input devices run in different
 * threads and have no coordination at all, a delay of at least one buffer is
 * recommended for real-time playback.
 *
 * This method has no public initializers or non-degenerate constructors. That
 * is because all input nodes should be created by the factory methods in
 * {@link AudioDevices}.  That way, the AudioDevices can properly handle device
 * conflicts as they may arise.
 *
 * Audio devices in SDL are identified by name. If you have two devices with
 * the same name, SDL will add a distinguishing index to the name. You can
 * see the list of all available devices with the {@link AudioDevices#devices()}
 * method.  In addition, the empty string may be used to refer to the default
 * devices.  Instances of AudioOutput attached to a default device will roll
 * over (if possible) whenever the default device changes.
 *
 * The audio graph should only be accessed in the main thread. In addition,
 * no methods marked as AUDIO THREAD ONLY should ever be accessed by the
 * user.
 *
 * This class does not support any actions for the {@link AudioNode#setCallback}.
 */
class AudioInput : public AudioNode {
private:
    /** The device name for this output node.  Empty string for default */
    std::string _dvname;
    
    /** The audio device in use */
    SDL_AudioDeviceID _device;
    /** The audio specification */
    SDL_AudioSpec  _audiospec;
    /** The DESIRED audio specification */
    SDL_AudioSpec  _wantspec;

    /** Whether the device is actively recording */
    std::atomic<bool> _record;
    
    /** Whether or not the device is currently active */
    std::atomic<bool> _active;

    /** To allow manual stopping of this node */
    std::atomic<Sint64> _timeout;

    /** Mutex to protect more sophisticated synchronization */
    mutable std::mutex _buffmtex;

    // We were forced to use mutexes for this class, so there are no atomics
    /** The recording buffer */
    float* _buffer;
    /** The total size of the recording buffer */
    Uint32 _capacity;
    /** The write size for this device */
    Uint32 _writesize;
    /** The read delay for this device */
    Uint32 _readdelay;
    /** The number of elements currently in the buffer */
    Uint32 _buffsize;
    /** The head of the circular buffer queue */
    Uint32 _buffhead;
    /** The tail of the circular buffer queue */
    Uint32 _bufftail;
    
    /** The location of a data mark in playback */
    Sint64 _playmark;
    /** The current read head position of playback */
    Sint64 _playpost;
    /** The data buffer for recording when mark is set */
    std::deque<float> _playback;
    
    /** Whether this device is write locked */
    bool _locked;

#pragma mark -
#pragma mark AudioDevices Methods
    /**
     * Initializes the default input device with 2 channels at 48000 Hz.
     *
     * This node will have a read size of {@link AudioDevices#getReadSize()}
     * frames, and will use {@link AudioDevices#getWriteSize()} for the size
     * of the recording buffer. The delay will be equal to the read size,
     * which means that that many frames must be recorded before anything
     * can be read.
     *
     * By default, this value of each of theses is 512 frames. This means that,
     * at stereo 48000 Hz, the recording delay is 21ms and the playback delay
     * is an (additional) 21 ms. So 42 ms passes between data is captured at
     * the hardware device and when it can be processed by the audio graph.
     * While this may seem like a lot of overhead, our experience (particularly
     * on macOS, iOS) has shown this is necessary for smooth real-time processing.
     *
     * An input device is initialized with both active as false and record as
     * true. That means it will start recording as soon as {@link AudioDevices}
     * sets this device to active. In addition, it is also unpaused, meaning
     * that playback will start as soon as it is attached to an audio graph.
     *
     * This node is always logically attached to the default input device.
     * That means it will switch devices whenever the default input changes.
     * This method may fail if the default device is in use.
     *
     * @return true if initialization was successful
     */
    virtual bool init() override;
    
    /**
     * Initializes the default input device with the given channels and sample rate.
     *
     * This node will have a read size of {@link AudioDevices#getReadSize()}
     * frames, and will use {@link AudioDevices#getWriteSize()} for the size
     * of the recording buffer. The delay will be equal to the read size,
     * which means that that many frames must be recorded before anything
     * can be read.
     *
     * By default, this value of each of theses is 512 frames. This means that,
     * at stereo 48000 Hz, the recording delay is 21ms and the playback delay
     * is an (additional) 21 ms. So 42 ms passes between data is captured at
     * the hardware device and when it can be processed by the audio graph.
     * While this may seem like a lot of overhead, our experience (particularly
     * on macOS, iOS) has shown this is necessary for smooth real-time processing.
     *
     * An input device is initialized with both active as false and record as
     * true. That means it will start recording as soon as {@link AudioDevices}
     * sets this device to active. In addition, it is also unpaused, meaning
     * that playback will start as soon as it is attached to an audio graph.
     *
     * This node is always logically attached to the default input device.
     * That means it will switch devices whenever the default input changes.
     * This method may fail if the default input device is in use.
     *
     * @param channels  The number of audio channels
     * @param rate      The sample rate (frequency) in Hz
     *
     * @return true if initialization was successful
     */
    virtual bool init(Uint8 channels, Uint32 rate) override;
    
    /**
     * Initializes the default input device with the given channels and sample rate.
     *
     * The write size is the number of samples recorded at each poll, while
     * the read size is the number of frames that must be recorded before a
     * single frame can be read.  These determine the recording latency and
     * playback latency, respectively.
     *
     * It is not necessary for the write size of an input device match the
     * read size of an output device.  Indeed, some many systems, an input
     * buffer size of less than 1024 stereo frames is not supported, while
     * output devices can process much faster than that. What is important
     * is ensuring enough delay so that the audio graph does not outrun the
     * input device. Therefore, a delay of less than the read size is not
     * recommended for real-time audio processing.
     *
     * We have found that minimum buffer size of 1024 frames and an equal delay
     * of 1024 is the minimum value for most systems. That is because there is
     * no thread coordination at all between the {@link record()} method
     * (called by the input device) and the {@link read()} method (called by
     * the audio graph). If you find that you need more delay, use the method
     * {@link readDelay}.

     *
     * An input device is initialized with both active as false and record as
     * true. That means it will start recording as soon as {@link AudioDevices}
     * sets this device to active. In addition, it is also unpaused, meaning
     * that playback will start as soon as it is attached to an audio graph.
     *
     * This node is always logically attached to the default input device.
     * That means it will switch devices whenever the default input changes.
     * This method may fail if the default input device is in use.
     *
     * @param channels  The number of audio channels
     * @param rate      The sample rate (frequency) in Hz
     * @param readsize  The size of the buffer to output audio
     * @param writesize The size of the buffer to record audio
     *
     * @return true if initialization was successful
     */
    bool init(Uint8 channels, Uint32 rate, Uint32 readsize, Uint32 writesize);
    
    /**
     * Initializes the given input device with 2 channels at 48000 Hz.
     *
     * This node will have a read size of {@link AudioDevices#getReadSize()}
     * frames, and will use {@link AudioDevices#getWriteSize()} for the size
     * of the recording buffer. The delay will be equal to the read size,
     * which means that that many frames must be recorded before anything
     * can be read.
     *
     * By default, this value of each of theses is 512 frames. This means that,
     * at stereo 48000 Hz, the recording delay is 21ms and the playback delay
     * is an (additional) 21 ms. So 42 ms passes between data is captured at
     * the hardware device and when it can be processed by the audio graph.
     * While this may seem like a lot of overhead, our experience (particularly
     * on macOS, iOS) has shown this is necessary for smooth real-time processing.
     *
     * An input device is initialized with both active as false and record as
     * true. That means it will start recording as soon as {@link AudioDevices}
     * sets this device to active. In addition, it is also unpaused, meaning
     * that playback will start as soon as it is attached to an audio graph.
     *
     * This method may fail if the given device is in use.
     *
     * @param device    The name of the input device
     *
     * @return true if initialization was successful
     */
    bool init(const std::string device);
    
    /**
     * Initializes the input device with the given channels and sample rate.
     *
     * The write size is the number of samples recorded at each poll, while
     * the read size is the number of frames that must be recorded before a
     * single frame can be read.  These determine the recording latency and
     * playback latency, respectively.
     *
     * It is not necessary for the write size of an input device match the
     * read size of an output device.  Indeed, some many systems, an input
     * buffer size of less than 1024 stereo frames is not supported, while
     * output devices can process much faster than that. What is important
     * is ensuring enough delay so that the audio graph does not outrun the
     * input device. Therefore, a delay of less than the read size is not
     * recommended for real-time audio processing.
     *
     * We have found that minimum buffer size of 1024 frames and an equal delay
     * of 1024 is the minimum value for most systems. That is because there is
     * no thread coordination at all between the {@link record()} method
     * (called by the input device) and the {@link read()} method (called by
     * the audio graph). If you find that you need more delay, use the method
     * {@link readDelay}.
     *
     * An input device is initialized with both active as false and record as
     * true. That means it will start recording as soon as {@link AudioDevices}
     * sets this device to active. In addition, it is also unpaused, meaning
     * that playback will start as soon as it is attached to an audio graph.
     *
     * This method may fail if the given device is in use.
     *
     * @param device    The name of the input device
     * @param channels  The number of audio channels
     * @param rate      The sample rate (frequency) in Hz
     * @param readsize  The size of the buffer to output audio
     * @param writesize The size of the buffer to record audio
     *
     * @return true if initialization was successful
     */
    bool init(const std::string device, Uint8 channels, Uint32 rate,
              Uint32 readsize, Uint32 writesize);
    
    /**
     * Disposes any resources allocated for this input device node.
     *
     * The state of the node is reset to that of an uninitialized constructor.
     * Unlike the destructor, this method allows the node to be reinitialized.
     */
    virtual void dispose() override;
    
    /**
     * Sets the active status of this node.
     *
     * An active device will have its {@link record()} method called at regular
     * intervals.  This setting is to allow {@link AudioDevices} to release and
     * acquire an input device without overriding the user settings.
     *
     * @param active    Whether to set this node to active
     */
    void setActive(bool active);
    
    /** Allow AudioDevices to access intializers */
    friend class AudioDevices;

#pragma mark -
#pragma mark (Re)initialization Methods
    /**
     * Opens a device according to the wanted specification
     *
     * This method is necessary because sometimes we need to close and reopen
     * a device, particularly on a format change.
     *
     * @return true if the device was successfully reinitialized
     */
    bool reopenDevice();
    
    /**
     * Allocates the recording buffer
     */
    void allocateBuffer();

public:
    /**
     * Temporarily locks this input device
     *
     * A locked input device cannot record. However, it will still respond
     * to read events if it is in an audio graph. This method will have no
     * effect if the device is already locked.
     *
     * **IMPORTANT**: You must call unlock on this device to use it again.
     * The device will only unlock itself on destruction.
     */
    void lock();

    /**
     * Unlocks this input device
     *
     * A locked input device cannot record. However, it will still respond
     * to read events if it is in an audio graph. This method will have no
     * effect if the device is not locked.
     */
    void unlock();
    
    /**
     * Returns true if the input device is currently locked.
     *
     * A locked input device cannot record. However, it will still respond
     * to read events if it is in an audio graph. This method will have no
     * effect if the device is not locked.
     *
     * @return true if the input device is currently locked.
     */
    bool isLocked() const { return _locked; }
    
    /**
     * Creates a degenerate audio input node.
     *
     * The node has not been initialized, so it is not active.  The node
     * must be initialized to be used.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a node on
     * the heap, use the factory in {@link AudioDevices}.
     */
    AudioInput();
    
    /**
     * Deletes the audio input node, disposing of all resources
     */
    ~AudioInput();


#pragma mark -
#pragma mark Data Access
    /**
     * Returns the audio device identifier associated with this audio input.
     *
     * @return the audio device identifier associated with this audio input.
     */
    const SDL_AudioDeviceID getAUID() const  { return _device; }
    
    /**
     * Returns the device associated with this input node.
     *
     * @return the device associated with this input node.
     */
    const std::string getDevice() const;
    
    /**
     * Returns true if this output node is associated with the default device
     *
     * A graph on the default device will switch devices whenever the default
     * device changes.
     *
     * @return true if this audio graph is associated with the default device
     */
    bool isDefault() const { return _dvname == ""; }
    
    /**
     * Returns the write size of this input node.
     *
     * The buffer value is the number of samples recorded at each poll. Smaller
     * buffers clearly tax the CPU, as the device is collecting data at a higher
     * rate. Furthermore, if the value is too small, the recording device may
     * not be able to keep up with the output device.  For example, we find
     * on MacOS that the input device at a sample rate of 48000 Hz can only
     * record stereo data every 21 milliseconds. Hence so it can only (at best)
     * keep up with a 512 frame playback buffer.
     *
     * This method is not synchronized as it can only be changed by locking the
     * input device.
     *
     * @return the write size of this input node.
     */
    Uint32 getWriteSize() const { return _writesize; }
    
    /**
     * Sets the write size of this input node.
     *
     * The buffer value is the number of samples recorded at each poll. Smaller
     * buffers clearly tax the CPU, as the device is collecting data at a higher
     * rate. Furthermore, if the value is too small, the recording device may
     * not be able to keep up with the output device.  For example, we find
     * on MacOS that the input device at a sample rate of 48000 Hz can only
     * record stereo data every 21 milliseconds. Hence so it can only (at best)
     * keep up with a 512 frame playback buffer.
     *
     * Changing this value will temporarily lock this input device.
     *
     * @param size  The write size of this input node.
     */
    void setWriteSize(Uint32 size);
    
    /**
     * Returns the additional read delay of this input node.
     *
     * Most of the time, a delay of the read size is enough. However, sometimes
     * an additional delay is required for performance reasons. This playback
     * delay is added on top of the normal read size delay. By default, this
     * value is 0.
     *
     * Changing this value will temporarily lock this input device.
     *
     * @return the additional read delay of this input node.
     */
    Uint32 getReadDelay() const { return _readdelay; }
    
    /**
     * Returns the write size of this input node.
     *
     * Most of the time, a delay of the read size is enough. However, sometimes
     * an additional delay is required for performance reasons. This playback
     * delay is added on top of the normal read size delay. By default, this
     * value is 0.
     *
     * Changing this value will temporarily lock this input device.
     *
     * @param size  The additional read delay of this input node.
     */
    void setReadDelay(Uint32 size);
    
    
    /**
     * Sets the read size of this input node.
     *
     * The read size determines the minimal delay of this input node. All
     * audio is guaranteed to be delay by one read size cycle.
     *
     * Changing this value will temporarily lock this input device.
     *
     * @param size  The read size of this input node.
     */
    virtual void setReadSize(Uint32 size) override;
    
#pragma mark -
#pragma mark Playback Control
    /**
     * Pauses this node, preventing any data from being played back.
     *
     * As with all other audio nodes, pausing effects the playback.  However, it
     * does not affect recording.  Recording will still happen in the background
     * and may be recovered if {@link mark()} is set. To stop recording (but
     * not playback) call {@link release()} instead.
     *
     * If the node is already paused, this method has no effect. Pausing will
     * not go into effect until the next render call in the audio thread.
     *
     * @return true if the node was successfully paused
     */
    virtual bool pause() override;
    
    /**
     * Resumes this previously paused node, allowing data to be played back.
     *
     * As with all other audio nodes, pausing effects the playback.  However,
     * does not affect recording.  When play is resumed, the playback will
     * either return with immediate playback or the recording buffer,
     * depending on whether {@link mark()} is set.
     *
     * If the node is not paused, this method has no effect.
     *
     * @return true if the node was successfully resumed
     */
    virtual bool resume() override;
    
    /**
     * Returns true if this node is currently recording audio.
     *
     * Recording is completely independent of playback.  An input node can
     * be recording, but have its played paused, and vice versa.
     *
     * @return true if this node is currently recording audio.
     */
    bool isRecording() const;
    
    /**
     * Stops this input node from recording.
     *
     * This method does not effect playback. Unpaused playback will continue
     * until the write delay has caught up.  After that point, it will only
     * play silence.
     *
     * If the node is not recording, this method has no effect.
     *
     * @return true if the node was successfully released
     */
    bool release();

    /**
     * Resumes recording for a previously released node.
     *
     * This method does not effect playback.  If playback is paused, then
     * recording will be buffered if {@link mark()} is set, or else it will
     * overwrite itself in the circular buffer.
     *
     * If the node is already recording, this method has no effect.
     *
     * @return true if the node was successfully acquired
     */
    bool acquire();
    
    /**
     * Instantly stops this node from both recording and playback.
     *
     * This method is the same as calling both the methods {@link pause()} and
     * {@link release()}.  In addition, the input node will be marked as
     * {@link completed()} for the purpose of the audio graph.
     */
    void stop();

    /**
     * Returns any cached data as an in-memory audio sample.
     *
     * This method is potentially expensive and should only be called when
     * the audio node has stopped recording (via the {@link release()} method,
     * and when the node is not part of an audio graph giving real-time
     * playback.
     *
     * If {@link mark()} is not set, this will return null rather than return
     * an empty audio sample.
     *
     * @return any cached data as an in-memory audio sample.
     */
    std::shared_ptr<AudioSample> save();

#pragma mark -
#pragma mark Audio Graph
    /**
     * Returns true if this audio node has no more data.
     *
     * An audio node is typically completed if it return 0 (no frames read) on
     * subsequent calls to {@link read()}. However, input nodes may run
     * infinitely.  Therefore this method only returns true when either of the
     * methods {@link stop()} or {@link setRemaining()} are called.
     *
     * @return true if this audio node has no more data.
     */
    virtual bool completed() override;
    
    /**
     * Reads up to the specified number of frames into the given buffer
     *
     * AUDIO THREAD ONLY: Users should never access this method directly.
     * The only exception is when the user needs to create a custom subclass
     * of this AudioOutput.
     *
     * The buffer should have enough room to store frames * channels elements.
     * The channels are interleaved into the output buffer. The number of
     * frames read is determined by the audio graph, not the buffer of this
     * device.
     *
     * This method will always forward the read position.
     *
     * @param buffer    The read buffer to store the results
     * @param frames    The maximum number of frames to read
     *
     * @return the actual number of frames read
     */
    virtual Uint32 read(float* buffer, Uint32 frames) override;
    
    /**
     * Records the specified number of frames to this audio node
     *
     * AUDIO THREAD ONLY: Users should never access this method directly.
     * The only exception is when the user needs to create a custom subclass
     * of this AudioOutput.
     *
     * If {@link mark()} is not set, this method records to a circular buffer
     * that has the given {@link getWriteSize()}. Data that is not read in a
     * timely manner is lost from the buffer.
     *
     * However, if mark is set, then this method writes to an ever-increasing
     * queue.  This queue can be accessed at any time with {@link reset()}
     * or {@link setPosition()}.  This can potentially take a lot of memory
     * and so it should be used carefully.  Use {@link release()} to stop
     * recording to the buffer while still having access to it.
     */
    Uint32 record(float* buffer, Uint32 frames);
    
    /**
     * Reboots the audio input node without interrupting any active polling.
     *
     * AUDIO THREAD ONLY: Users should never access this method directly.
     * The only exception is when the user needs to create a custom subclass
     * of this AudioNode.
     *
     * This method will close and reopen the associated audio device.  It
     * is primarily used when an node on the default device needs to migrate
     * between devices.
     */
    void reboot();

#pragma mark -
#pragma mark Optional Methods
    /**
     * Marks the current read position in the audio steam.
     *
     * This methods creates an internal buffer for recording audio data.
     * This buffer may be returned to at any time with {@link reset()} command.
     * Doing so introduces an inherent delay going forward, as the playback
     * comes from the recorded buffer.
     *
     * @return true if the read position was marked.
     */
    virtual bool mark() override;
    
    /**
     * Clears the current marked position.
     *
     * If the method {@link mark()} has started recording to a buffer, then
     * this method will stop recording and release the buffer.  When the mark
     * is cleared the method {@link reset()} will no longer work.
     *
     * @return true if the read position was marked.
     */
    virtual bool unmark() override;
    
    /**
     * Resets the read position to the marked position of the audio stream.
     *
     * This method does nothing (and returns false) if no {@link mark()} is set.
     * Otherwise, it resets to the start of the buffer created by the call to
     * mark. This introduces an inherent delay going forward, as the playback
     * comes from the recorded buffer.
     *
     * @return true if the read position was moved.
     */
    virtual bool reset() override;
    
    /**
     * Returns the current frame position of this audio node
     *
     * This method returns -1 (indicating it is not supported) if {@link mark()}
     * is not set.  Otherwise, the position will be the number of frames since
     * the mark.
     *
     * @return the current frame position of this audio node.
     */
    virtual Sint64 getPosition() const override;
    
    /**
     * Sets the current frame position of this audio node.
     *
     * This method returns -1 (indicating it is not supported) if {@link mark()}
     * is not set.  Otherwise, it will set the position to the number of frames
     * since the mark.  If the position is in the future (a frame not already
     * buffered) then this method will fail and return -1.
     *
     * @param position  the current frame position of this audio node.
     *
     * @return the new frame position of this audio node.
     */
    virtual Sint64 setPosition(Uint32 position) override;
    
    /**
     * Returns the elapsed time in seconds.
     *
     * This method returns -1 (indicating it is not supported) if {@link mark()}
     * is not set.  Otherwise, the position will be the number of seconds since
     * the mark.
     *
     * @return the elapsed time in seconds.
     */
    virtual double getElapsed() const override;
    
    /**
     * Sets the read position to the elapsed time in seconds.
     *
     * This method returns -1 (indicating it is not supported) if {@link mark()}
     * is not set.  Otherwise, it will set the position to the number of seconds
     * since the mark.  If the position is in the future (a time not already
     * buffered) then this method will fail and return -1.
     *
     * @param time  The elapsed time in seconds.
     *
     * @return the new elapsed time in seconds.
     */
    virtual double setElapsed(double time) override;
    
    /**
     * Returns the remaining time in seconds.
     *
     * This method returns -1 (indicating it is not supported) if the method
     * {@link setRemaining()} has not been called or has been interrupted.
     * Otherwise, it returns the amount of time left in the countdown timer
     * until this node completes.
     *
     * @return the remaining time in seconds.
     */
    virtual double getRemaining() const override;
    
    /**
     * Sets the remaining time in seconds.
     *
     * This method sets a countdown timer on the input node, forcing it to
     * complete in the given number of seconds.  If the audio has been reading
     * from the buffer (because of a call to {@link setPosition()}, this method
     * immediately skips ahead to real-time recording.  Any call to
     * {@link setPosition()} or {@link setElapsed()} before this time is up
     * will cancel the countdown.
     *
     * @param time  The remaining time in seconds.
     *
     * @return the new remaining time in seconds.
     */
    virtual double setRemaining(double time) override;
    
};
    }
}
#endif /* __CU_AUDIO_INPUT_H__ */
