/*
 * SDL_atk:  An audio toolkit library for use with SDL
 * Copyright (C) 2022-2023 Walker M. White
 *
 * This is a library to load different types of audio files as PCM data,
 * and process them with basic DSP tools. The goal of this library is to
 * create an audio equivalent of SDL_image, where we can load and process
 * audio files without having to initialize the audio subsystem. In addition,
 * giving us direct control of the audio allows us to add more custom
 * effects than is possible in SDL_mixer.
 *
 * This component provides support for IIR and convolution filters, which are the
 * building blocks for audio effects. In addition, this component provides support
 * for several popular effects. This features of the component are inspired by the
 * famous STK (Synthesis ToolKit):
 *
 * https://github.com/thestk/stk
 *
 * However, that toolkit uses aggressively inlining to compose filters together,
 * while our implementation is more focused on simplifying page-based stream
 * processing.
 *
 * SDL License:
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

/**
 * @file ATK_audio.h
 *
 * Header file for the audio effects supported by SDL_atk
 *
 * This component provides support for audio processing. This component differs
 * from the DSP component in that most of the functionality is applied to
 * multichannel audio rather than isolated signals.
 */
#ifndef __ATK_AUDIO_H__
#define __ATK_AUDIO_H__
#include "SDL.h"
#include "SDL_version.h"
#include "begin_code.h"

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

#pragma mark -
#pragma mark Latency Adapter
/**
 * Callback function for feeding audio data to a filter or processor.
 *
 * This callback is an alternative to the one used by SDL_AudioCVT. It allows
 * us to only fill a portion of a stream, rather than forcing us to pad with
 * silence.
 *
 * The value len represents the number of bytes requested. It is okay to return
 * less than the requested bytes so long as the data is aligned to the format
 * and the number of channels. In particular, returning an incomplete portion
 * of an audio frame can have undefined results.
 *
 * @param userdata  Generic ser data for the callback
 * @param stream    Buffer to store the audio data
 * @param len       The number bytes store in the buffer
 *
 * @return the number of bytes actually put in the buffer
 */
typedef size_t (*ATK_AudioCallback)(void* userdata, Uint8* stream, size_t len);

/**
 * An opaque type for a latency adapter.
 *
 * A latency adapter introduces asynchronous latency into a audio device to
 * increase the time budget for effects (e.g. filters or convolutions). It
 * does this by providing a backing buffer of a larger size that is filled
 * asynchronously to audio device requests.
 *
 * For example, if an audio device processes 48k audio with a buffer size of
 * 512 sample frames, that means that a signal processor has 9-10 ms to execute
 * any effects. While most effects do not take this long, convolutional reverb
 * can strain to hit this on modest hardware (particularly if the impulse is
 * multichannel over many seconds). Unlike video, exceeding this time budget
 * does not cause a slowdown; it causes silence. Increasing that buffer to 2048
 * will increase that time budget to ~40 ms, but with an associated increase in
 * latency.
 *
 * The adapter allows this latency to be introduced into part of the audio
 * subsystem without increasing the overall latency of the device. As an
 * example, atmospheric audio may not be as latency sensitive as real-time
 * sound effects. The atmospheric audio can be processed throught this adapter,
 * giving it time for more effects, while the sound effects are processed
 * directly.
 */
typedef struct ATK_LatencyAdapter ATK_LatencyAdapter;

/**
 * Returns a newly allocated latency adapter.
 *
 * A latency adapter assumes that input >= output. If this is not true, this
 * function will return NULL.
 *
 * The input and output sizes are specified in bytes, not sample frames. So
 * an AUDIO_F32 stereo buffer of 512 sample frames is 4096 bytes. The output
 * buffer should match the size used for {@link ATK_PollLatencyAdapter}. If
 * so, the callback will be executed with size output at a rate of output/input
 * the polling frequency. If {@link ATK_PollLatencyAdapter} is called with a
 * different size, the callback frequency is unspecified (though it will
 * be a function of the new output size).
 *
 * It is possible that callback is NULL. In that case, data should be pushed
 * to the latency adapter with {@link ATK_PushLatencyAdapter}. Data should be
 * pushed at a rate output/input the polling frequency. If the data cannot
 * match this frequency, {@link ATK_PollLatencyAdapter} may poll silence.
 *
 * A latency adapter always starts paused. You should unpause the adapter with
 * {@link ATK_PauseLatencyAdapter} when the callback function is ready to start
 * providing data.
 *
 * @param input     The desired buffer size for the audio device
 * @param output    The actual buffer size for the audio device
 * @param callback  An optional callback to gather input data
 *
 * @return a newly allocated latency adapter.
 */
extern DECLSPEC ATK_LatencyAdapter* SDLCALL ATK_AllocLatencyAdapter(size_t input, size_t output,
                                                                    ATK_AudioCallback callback,
                                                                    void* userdata);
/**
 * Frees a previously allocated latency adapter
 *
 * @param adapter   The latency adapter
 */
extern DECLSPEC void SDLCALL ATK_FreeLatencyAdapter(ATK_LatencyAdapter* adapter);

/**
 * Pulls delayed data from the latency buffer, storing it in output.
 *
 * This function pulls whatever data is currently available, up to size len.
 * If a callback exists, this function may instruct that callback to replenish
 * the buffer as needed. However, this function never blocks on this callback,
 * as it is executed asynchronously. If the buffer does not have enough data,
 * this function will return the number of bytes that could be read without
 * blocking (even while waiting for the callback to complete).
 *
 * @param adapter   The latency adapter
 * @param output    The output buffer
 * @param len       The number of bytes to poll
 *
 * @return the number of bytes read or -1 on error
 */
extern DECLSPEC Sint64 SDLCALL ATK_PollLatencyAdapter(ATK_LatencyAdapter* adapter, Uint8* output, int len);

/**
 * Pushes data to the latency adapter
 *
 * This is an optional way to repopulate the latency adapter, particularly if
 * no callback function was specified at the time it was allocated. With that
 * said, data can be pushed even if there is a callback function. Doing so will
 * simply reduce the demand for the callback.
 *
 * It is not possible to push more bytes that the (input) buffer size of the
 * latency adapter. This function will return the number of bytes that could
 * be pushed. For reasons of thread-safety, this function will not write any
 * bytes if the adapter has a callback function in flight.
 *
 * @param adapter   The latency adapter
 * @param input     The input buffer
 * @param len       The number of bytes to push
 *
 * @return the number of bytes pushed or -1 on error
 */
extern DECLSPEC Sint64 SDLCALL ATK_PushLatencyAdapter(ATK_LatencyAdapter* adapter, const Uint8* input, int len);

/**
 * Toggles the pause state for the latency adapter
 *
 * If pauseon is 1, this function pauses the asynchronous thread associated
 * with the adapter. If that thread is currently executing a read, this
 * function will block until the read is finished. If the value pauseon is 0,
 * this function will restart a previously paused thread.
 *
 * A latency adapter should be paused whenever the user needs to modify the
 * userdata associated with the adapter callback function. Modifying this data
 * while the thread is still active can result in data races.
 *
 * @param adapter   The latency adapter
 * @param pauseon   Nonzero to pause, zero to resume
 */
extern DECLSPEC void SDLCALL ATK_PauseLatencyAdapter(ATK_LatencyAdapter* adapter, int pauseon);

/**
 * Resets the latency adapter
 *
 * Reseting empties and zeroes all buffers. It also returns the latency adapter
 * to a paused state. The adapter will need to be unpaused with a call to
 * {@link ATK_PauseLatencyAdapter}
 *
 * @param adapter   The latency adapter
 */
extern DECLSPEC void SDLCALL ATK_ResetLatencyAdapter(ATK_LatencyAdapter* adapter);

/**
 * Blocks on the read thread for this latency adapter.
 *
 * This function blocks until the asynchronous read thread has populated the
 * backing buffer using the callback function. It does not block if the adapter
 * is paused or the backing buffer is full.
 *
 * @param adapter   The latency adapter
 *
 * @return 1 if this function blocked, 0 otherwise
 */
extern DECLSPEC int SDLCALL ATK_BlockLatencyAdapter(ATK_LatencyAdapter* adapter);

#pragma mark -
#pragma mark Conversion Filters

/**
 * A structure to resample audio to a different rate.
 *
 * This structure supports resampling via bandlimited interpolation, as described
 * here:
 *
 *     https://ccrma.stanford.edu/~jos/resample/Implementation.html
 *
 * Techinically, this process is supported by SDL_AudioCVT in SDL. However, we
 * have had problems with that resampler in the past. As of SDL 2.0.14, there
 * was a bug that could cause the resampler to be caught zero-padding in an
 * infinite loop, resulting in the audio cutting out. This was a major problem
 * on iPhones as the device can switch between 44.1k and 48k, depending on
 * whether you are using headphones or speakers.
 *
 * More importantly, in the public API of SDL (as opposed to the undocumented
 * stream API), resampling can only happen just before audio is sent to the
 * device. In an audio engine, you often want to resample much earlier in the
 * DSP graph (e.g. reading in an audio file compressed with a much lower sample
 * rate). That is why we have separated out this feature.
 */
typedef struct ATK_Resampler ATK_Resampler;

/**
 * The default resampler stopband.
 *
 * The stopband is used to generate the resampler sinc filter as described here:
 *
 *     https://tomroelandts.com/articles/how-to-create-a-configurable-filter-using-a-kaiser-window
 */
#define ATK_RESAMPLE_STOPBAND    80
/**
 * The default resampler zero crossings
 *
 * The zero-crossings of a sinc filter are relevant because the determine the
 * number of coefficients in a single filter convolution. For X zero-crossings,
 * a single output sample requires 2*(X-1) input computations. Increasing this
 * value can give some increased value in filter. However, the droppoff for sinc
 * filters is large enough that eventually that large enough values will have
 * no discernable effect.
 *
 * The default number of zero crossing is 5, meaning that this filter roughly
 * causes an 8x-10x decrease in performance when processing audio (when taking
 * all the relevant overhead into account). This value is that one recommended
 * by this tutorial website:
 *
 *     https://www.dsprelated.com/freebooks/pasp/Windowed_Sinc_Interpolation.html
 */
#define ATK_RESAMPLE_ZEROCROSS    5
/**
 * The default resampler bit depth.
 *
 * The bitdepth is the precision of the audio processed by the resampler. Even
 * though our audio streams are all floats, most audio files (e.g. WAV files)
 * have 16 bit precision. A 16 bit filter uses a very reasonable 512 entries
 * per zero crossing. On the other hand, a 32 bit filter would require 131072
 * entries per zero crossing. Given the limitations of real-time resampling, it
 * typically does not make much sense to assume more than 16 bits.
 */
#define ATK_RESAMPLE_BITDEPTH    16

/**
 * A structure storing the resampler settings.
 *
 * The resampler is an opaque structure that cannot be changed once it is
 * allocated. However, there are several settings that can be customized before
 * the resampler is created.
 */
typedef struct ATK_ResamplerDef {
    /**
     * The number of channels in the input (and output) streams.
     *
     * Resamplers assume that audio is interleaved according to the number of
     * channels. Unlike more primitive SDL_atk functions, we do not have
     * stride-aware resamplers. If you need such a feature, you should use
     * deinterleaving functions or stride-aware copying to extract the channel
     * you wish to resample.
     */
    Uint8 channels;
    /**
     * The sample rate of the input stream.
     *
     * This value is fixed and cannot be changed without reallocating the
     * resampler.
     */
    Uint32 inrate;
    /**
     * The sample rate of the output stream.
     *
     * This value is fixed and cannot be changed without reallocating the
     * resampler.
     */
    Uint32 outrate;
    /**
     * The stopband of the resampler sinc filter.
     *
     * The stopband is used to generate the resampler sinc filter as described
     * here:
     *
     *     https://tomroelandts.com/articles/how-to-create-a-configurable-filter-using-a-kaiser-window
     *
     * It is generally safe to use {@link ATK_RESAMPLER_STOPBAND}.
     */
    float stopband;
    /**
     * The number of zero crossings of the sinc filter.
     *
     * The zero-crossings of a sinc filter are relevant because the determine
     * the number of coefficients in a single filter convolution. For X
     * zero-crossings, a single output sample requires 2*(X-1) input computations.
     * Increasing this value can give some increased value in filter. However,
     * the droppoff for sinc filters is large enough that eventually that large
     * enough values will have no discernable effect.
     *
     * The default number of zero crossing is 5, meaning that this filter
     * roughly causes an 8x-10x decrease in performance when processing audio
     * (when taking all the relevant overhead into account). This value is that
     * one recommended by this tutorial website:
     *
     *     https://www.dsprelated.com/freebooks/pasp/Windowed_Sinc_Interpolation.html
     */
    Uint32 zerocross;
    /**
     * The resampler bit depth.
     *
     * The bitdepth is the precision of the audio processed by the resampler.
     * Even though our audio streams are all floats, most audio files (e.g. WAV
     * files) have 16 bit precision. A 16 bit filter uses a very reasonable 512
     * entries per zero crossing. On the other hand, a 32 bit filter would
     * require 131072 entries per zero crossing. Given the limitations of
     * real-time resampling, it typically does not make much sense to assume
     * more than 16 bits.
     */
    Uint32 bitdepth;
    /**
     * The resampler buffer size.
     *
     * This value is used to compute the amount of memory needed for resampling.
     * It specifies the expected size of the output at each call to the resampler
     * (e.g. it should match the device buffer size). It is possible to call
     * the resampler to convert more (or less) samples, but the call will be
     * less efficient.
     */
    Uint32 buffsize;
    /**
     * An optional callback function to fill the buffer.
     *
     * The resampler can only process data in its buffer. If this value is not
     * NULL, it will use this callback function to populate the buffer on demand.
     * If it is NULL, then the user must populate the buffer manually with
     * {@link ATK_PushResampler}.
     */
    ATK_AudioCallback callback;
    /**
     * The user data for the callback function.
     */
    void* userdata;
} ATK_ResamplerDef;

/**
 * Returns a newly allocated structure to resample audio
 *
 * Audio resampling is performed using bandlimited interpolation, as described
 * here:
 *
 *     https://ccrma.stanford.edu/~jos/resample/Implementation.html
 *
 * It is not possible to change any of the resampler settings after it is
 * allocated, as the filter is specifically tailored to these values. If you
 * need to change the settings, you should allocate a new resampler. It is the
 * responsiblity of the caller to use {@link ATK_FreeResampler} to deallocate
 * the structure when done.
 *
 * @param def   The resampler settings
 */
extern DECLSPEC ATK_Resampler* SDLCALL ATK_AllocResampler(const ATK_ResamplerDef* def);

/**
 * Frees a previously allocated resampler
 *
 * @param resampler The resampler state
 */
extern DECLSPEC void SDLCALL ATK_FreeResampler(ATK_Resampler* resampler);

/**
 * Resets a resampler back to its initial (zero-padded) state.
 *
 * Resamplers have to keep state of the conversion performed so far. This makes
 * it not safe to use a resampler on multiple streams simultaneously. Resetting
 * a resampler zeroes the state so that it is the same as if the filter were
 * just allocated.
 *
 * @param resampler The resampler state
 */
extern DECLSPEC void SDLCALL ATK_ResetResampler(ATK_Resampler* resampler);

/**
 * Pulls converted data from the resampler, populating it in output
 *
 * This function will convert up to len audio frames, storing the result in
 * output. An audio frame is a collection of samples for all of the available
 * channels, so output must be able to support len*channels many elements.
 *
 * It is possible for this function to convert less than len audio frames,
 * particularly if the buffer empties and there is no callback function to
 * repopulate it. In that case, the return value is the number of audio frames
 * read. The output will always consist of complete audio frames. It will never
 * convert some channels for an audio frame while not converting others.
 *
 * @param resampler The resampler state
 * @param output    The output buffer
 * @param frames    The number of audio frames to convert
 *
 * @return the number of audio frames read or -1 on error
 */
extern DECLSPEC Sint64 SDLCALL ATK_PollResampler(ATK_Resampler* resampler, float* output, size_t frames);

/**
 * Pushes data to the resampler buffer
 *
 * This is an optional way to repopulate the resampler buffer, particularly if
 * no callback function was specified at the time it was allocated. Data is
 * pushed as complete audio frames. An audio frame is a collection of samples
 * for all of the available channels, so input must hold len*channels many
 * elements. It is not possible to push an incomplete audio frame that stores
 * data for some channels, but not others.
 *
 * The limits on the buffer capacity may mean that not all data can be pushed
 * (particularly if this function is competing with a callback function). The
 * value returned is the number of audio frames successfully stored in the buffer.
 *
 * @param resampler The resampler state
 * @param input     The input buffer
 * @param frames    The number of audio frames to push
 *
 * @return the number of audio frames pushed or -1 on error
 */
extern DECLSPEC Sint64 SDLCALL ATK_PushResampler(ATK_Resampler* resampler, const float* input, size_t frames);


/**
 * A structure to redistribute audio channels.
 *
 * Channel redistribution works by using a matrix to redistribute the input
 * channels, in much the same way that a matrix decoder works. However, unlike
 * a matrix decoder, it is possible to use a redistributor to reduce the number
 * of channels (with a matrix whose rows are less that is columns). Furthermore,
 * a redistributor does not support phase shifting.
 */
typedef struct ATK_Redistributor ATK_Redistributor;

/**
 * Returns a newly allocated channel redistributor
 *
 * Redistribution works by using a matrix to redistribute the input channels,
 * in much the same way that a matrix decoder works. The value matrix should
 * be a MxN matrix in row major order, where N is the number of input channels
 * and M is the number of output channels.
 *
 * The matrix will be copied. The redistributor will not claim ownership of the
 * existing matrix. It is possible for matrix to be NULL. In that case, the
 * redistributor will use the default redistribution matrix.
 *
 * @param inchan    The number of channels of the input stream
 * @param outchan   The number of channels of the output stream
 * @param matrix    The redistribution matrix (can be NULL)
 */
extern DECLSPEC ATK_Redistributor* SDLCALL ATK_AllocRedistributor(Uint32 inchan, Uint32 outchan,
                                                                  float* matrix);

/**
 * Frees a previously allocated channel redistributor
 *
 * @param distrib   The redistributor state
 */
extern DECLSPEC void SDLCALL ATK_FreeRedistributor(ATK_Redistributor* distrib);

/**
 * Applies channel redistribution to input, storing the result in output.
 *
 * Frames is the number of audio frames, which is a collection of simultaneous
 * samples for each channel. Thus input should hold frames*inchan samples, while
 * output should be able to store frames*outchan samples (inchan and outchan
 * were specified when the redistributor was allocated).
 *
 * Redistributors are not stateful, and can freely be applied to multiple
 * streams.
 *
 * @param distrib   The redistributor state
 * @param input     The input stream
 * @param output    The output stream
 * @param frames    The number of audio frames to process
 *
 * @return the number of frames processed, or -1 on error
 */
extern DECLSPEC Sint64 SDLCALL ATK_ApplyRedistributor(ATK_Redistributor* distrib,
                                                      const float* input, float* output, size_t frames);

/**
 * Converts the audio data in input to the format required by output
 *
 * It is safe for input and output to be the same buffer.
 *
 * @param input     The input data
 * @param informat  The input format
 * @param output    The output buffer
 * @param outformat The output format
 * @param len       The number of bytes (in input) to convert
 *
 * @return 0 if conversion is successful, -1 otherwise
 */
extern DECLSPEC int SDLCALL ATK_ConvertAudioFormat(const Uint8* input, SDL_AudioFormat informat,
                                                   Uint8* output, SDL_AudioFormat outformat, size_t len);
/**
 * A structure to convert audio data between different formats.
 *
 * This structure is an alternative to SDL_AudioCVT. We have had problems with
 * that structure in the past. While it is fine for format and channel conversion,
 * we have found it to be quite unreliable for rate conversion. As of SDL 2.0.14,
 * there was a bug in the resampler that could cause the converter to be caught
 * in an infinite zero-padding loop, resulting in the audio cutting out. While
 * this may have been fixed in more recent versions of SDL, we prefer this
 * version which gives us a little more control over the conversion process. In
 * particular, it is possible to convert audio before it is sent to the device.
 */
typedef struct ATK_AudioCVT ATK_AudioCVT;

/**
 * Returns a newly allocated ATK_AudioCVT to convert between audio specs
 *
 * The conversion program will use the samples attribute of output to determine
 * the size of the intermediate buffer, but it will use the callback function
 * for input to fill this buffer (in increments of the input sample size). If
 * no callback function is specified, the user must fill the buffer explicitly
 * using {@link ATK_PushAudioCVT}.
 *
 * @param input     The input audio specification
 * @param output    The output audio specification
 */
extern DECLSPEC ATK_AudioCVT* SDLCALL ATK_AllocAudioCVT(const SDL_AudioSpec* input,
                                                        const SDL_AudioSpec* output,
                                                        ATK_AudioCallback callback);

/**
 * Frees a previously allocated audio CVT
 *
 * @param cvt   The audio CVT
 */
extern DECLSPEC void SDLCALL ATK_FreeAudioCVT(ATK_AudioCVT* cvt);

/**
 * Resets an audio CVT back to its initial (zero-padded) state.
 *
 * Specification converters have to keep state of the conversion performed so
 * far. This makes it not safe to use an audio CVT on multiple streams
 * simultaneously. Resetting an audio CVT zeroes the state so that it is the
 * same as if the converter were just allocated.
 *
 * @param cvt   The audio CVT
 */
extern DECLSPEC void SDLCALL ATK_ResetAudioCVT(ATK_AudioCVT* cvt);

/**
 * Pulls converted data from the input buffer, populating it in output
 *
 * This function will convert up to len bytes, storing the result in output.
 * In line with SDL, we do not require that len represent a full audio frame,
 * or even a complete aligned sample.
 *
 * It is possible for this function to convert less than len bytes, particularly
 * if the buffer empties and there is no callback function to repopulate it. In
 * that case, the return value is the number of bytes read.
 *
 * @param cvt       The audio CVT
 * @param output    The output buffer
 * @param len       The number of bytes to convert
 *
 * @return the number of bytes read or -1 on error
 */
extern DECLSPEC Sint64 SDLCALL ATK_PollAudioCVT(ATK_AudioCVT* cvt, Uint8* output, int len);

/**
 * Pushes data to the audio CVT buffer
 *
 * This is an optional way to repopulate the audio CVT buffer, particularly if
 * no callback function was specified at the time it was allocated. Data is does
 * not have to be pushed as complete audio frames, or even aligned samples.
 *
 * The limits on the buffer capacity may mean that not all data can be pushed
 * (particularly if this function is competing with a callback function). The
 * value returned is the number of bytes successfully stored in the buffer.
 *
 * @param cvt       The audio CVT
 * @param input     The input buffer
 * @param len       The number of bytes to push
 *
 * @return the number of bytes pushed or -1 on error
 */
extern DECLSPEC Sint64 SDLCALL ATK_PushAudioCVT(ATK_AudioCVT* cvt, const Uint8* input, int len);


#pragma mark -
#pragma mark Reverb Filters
// Algorithmic
/**
 * An algorithmic reverb filter.
 *
 * This is an opaque type defined via {@link ATK_AlgoReverbDef}. It is stateful,
 * in that there is always an unprocessed reverb tail (accessible via the
 * function {@link ATK_DrainAlgoReverb}). Therefore, you should only apply
 * this filter to one audio signal at a time.
 *
 * It is possible to update the reverb settings at any time with a call to
 * {@link ATK_UpdateAlgoReverb}. However, there is no way to extract the
 * current settings for a reverb filter. That is up to the programmer to
 * remember those values.
 */
typedef struct ATK_AlgoReverb ATK_AlgoReverb;

/**
 * The settings for algorithmic reverb.
 *
 * The reverb algorithm is modeled after the open source Schroeder reverberator,
 * Freeverb. The attributes below are used to tune the algorithm. You can
 * initialize the values using the function {@link ATK_AlgoReverbDefaults} or
 * set them explicitly. All of values should be between 0 and 1. More information
 * about the algorithm can be found at:
 *
 *     https://ccrma.stanford.edu/~jos/pasp/Freeverb.html
 */
typedef struct {
    /** The gain (0-1) to apply to the input signal */
    float ingain;
    /** The gain (0-1) to apply to the reverb component of the output */
    float wet;
    /** The gain (0-1) to apply to the original component of the output */
    float dry;
    /** The speaker distance, normalized to be a value 0 to 1. */
    float width;
    /** The damping factor. Typically much less than 1. */
    float damping;
    /** The room size, normalized to be a value 0 to 1. */
    float roomsize;
} ATK_AlgoReverbDef;

/**
 * Initializes the algorithmic reverb settings to their defaults.
 *
 * These defaults are the ones chosen by Jezar at Dreampoint, the original
 * FreeVerb author.
 *
 * @param def   The algorithmic reverb settings to initialize
 */
extern DECLSPEC void SDLCALL ATK_AlgoReverbDefaults(ATK_AlgoReverbDef* def);

/**
 * Returns a newly allocated algorithmic reverb filter with the given settings
 *
 * The initialized filter will be padded with zeros, so that the tail is all
 * silence. The settings can be updated at any time with a call to the function
 * {@link ATK_UpdateAlgoReverb}.
 *
 * The value block is used to allocate the size of the internal buffers (i.e.
 * the block size). The best performance is achieved when this matches the
 * value provided to {@link ATK_ApplyAlgoReverb}. The number of channels
 * supported by the filter, as well as the sample rate, is fixed at the time
 * of creation.
 *
 * @param def       The algorithmic reverb settings
 * @param rate      The sample rate for the reverb
 * @param channels  The number of channels in an audio frame
 * @param block     The expected number of frames to process at each step
 *
 * @return a newly allocated algorithmic reverb filter with the given settings
 */
extern DECLSPEC ATK_AlgoReverb* SDLCALL ATK_AllocAlgoReverb(ATK_AlgoReverbDef* def, Uint32 rate,
                                                            Uint32 channels, size_t block);

/**
 * Updates the settings of the given algorithmic reverb
 *
 * These settings can be updated at any time.
 *
 * @param filter    The algorithmic reverb filter
 * @param def       The algorithmic reverb settings
 */
extern DECLSPEC void SDLCALL ATK_UpdateAlgoReverb(ATK_AlgoReverb* filter, ATK_AlgoReverbDef* def);

/**
 * Frees a previously allocated algorithmic reverb filter
 *
 * @param filter    The algorithmic reverb filter
 */
extern DECLSPEC void SDLCALL ATK_FreeAlgoReverb(ATK_AlgoReverb* filter);

/**
 * Resets an allocated algorithmic reverb filter to its intial state
 *
 * The reverb tail will be zero padded so that it is all silence.
 *
 * @param filter    The algorithmic reverb filter
 */
extern DECLSPEC void SDLCALL ATK_ResetAlgoReverb(ATK_AlgoReverb* filter);

/**
 * Applies the algorithmic reverb filter to single audio frame.
 *
 * The buffers input and output should store a single audio frame, and hence
 * they should be the same size as the number of channels supported by this
 * filter. It is safe for input and output to be the same buffer (provided
 * the channels agree).
 *
 * @param filter    The algorithmic reverb filter
 * @param input     The buffer for the input audio frame
 * @param output    The buffer for the output audio frame
 */
extern DECLSPEC void SDLCALL ATK_StepAlgoReverb(ATK_AlgoReverb* filter,
                                                const float* input, float* output);

/**
 * Applies the algorithmic reverb filter to the given input signal.
 *
 * The input (and output) buffer should have size frames*channels, where
 * channels is the number of channels supported by this filter. The samples
 * for each channel should be interleaved. It is safe for input and output
 * to be the same buffer (provided the channels agree).
 *
 * @param filter    The algorithmic reverb filter
 * @param input     The buffer for the input
 * @param output    The buffer for the output
 * @param frames    The number of audio frames to process
 */
extern DECLSPEC void SDLCALL ATK_ApplyAlgoReverb(ATK_AlgoReverb* filter,
                                                 const float* input, float* output,
                                                 size_t frames);

/**
 * Drains the contents of the algorithmic reverb filter into the buffer.
 *
 * Even when the input has stopped, there is still some reverb left to process.
 * In the case of algorithmic reverb (as opposed to convolutional reverb), this
 * tail can be infinite, especially if the damping is inadequate.  In actual
 * audio system, this tail would be set to automatically fade out over time.
 * However, we separate that from the reverb algorithm, meaning that this
 * function is the same as passing an input of all 0s.
 *
 * The buffer should have size frames*channels, where channels is the number of
 * channels supported by this filter.
 *
 * @param filter    The algorithmic reverb filter
 * @param buffer    The buffer to store the output
 * @param frames    The maximum number of frames to drain
 *
 * @return the number of audio frames stored in buffer
 */
extern DECLSPEC size_t SDLCALL ATK_DrainAlgoReverb(ATK_AlgoReverb* filter, float* buffer, size_t frames);

// TODO: Convolutional reverb

#pragma mark -
#pragma mark Time/Pitch Shift Filters
// TODO: Pitch Shifting
// https://github.com/sannawag/TD-PSOLA
// TODO: Time Shifting
// https://www.surina.net/article/time-and-pitch-scaling.html (beware that sample code is GPL)

#pragma mark -
#pragma mark Beat Detection
// TODO: Beat Detection
// https://mziccard.me/2015/05/28/beats-detection-algorithms-1
// https://github.com/mziccard/scala-audio-file

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif /* __ATK_AUDIO_H__ */
