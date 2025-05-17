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
 * @file ATK_dsp.h
 *
 * Header file for the digital signal processing component of SDL_atk
 *
 * This component provides support for IIR and convolution filters, which are
 * the building blocks for audio effects. The features in this component
 * greatly benefit from compiling with optimization on.
 */
#ifndef __ATK_DSP_H__
#define __ATK_DSP_H__
#include "SDL.h"
#include "SDL_version.h"
#include "begin_code.h"

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * An enumeration of the supported wave form types
 *
 * ATK supports both naive waveforms and bandwidth-limited forms. Bandwidth-limited
 * forms are design to reduce the aliasing that can occur at the discontinuites:
 *
 *     https://ccrma.stanford.edu/~stilti/papers/blit.pdf
 *
 * For simplicity, we do not use the BLIT integration techniques of Stilson
 * and Smith.  Those techniques are subject to error creep over time unless a
 * a backing table is used. Instead, we use the PolyBLEP technique:
 *
 *     https://ieeexplore.ieee.org/document/4117934
 *
 * This technique is not "music quality." It is known to have audible aliasing
 * near the Nyquist frequency and overly attenuate higher frequencies. However,
 * it is compact and ideal for real-time sound generation. It is also good
 * enough for procedural sound generation in most games.
 *
 * Waveforms can be normal or upper-half only. The meaning of upper half
 * (which generally implies only nonnegative samples) depends on the actual
 * shape listed in this enum.
 */
typedef enum {
    /**
     * A sine wave
     *
     * An upper-half sine wave is the absolute value (or the rectified
     * sine wave). The initial generated value is 0.
     */
    ATK_SHAPE_SINE          = 0x00000001,
    /**
     * A naive triangular wave
     *
     * The waveform will have first-order discontinuities at PI and 2PI.
     * This will create a more smoother sound than a square or sawtooth
     * wave of the same frequency.  The initial generated value is -1.
     *
     * An upper half triangle wave is a waveform of the same shape but
     * scaled and shifted to [0,1] instead of [-1,1].
     */
    ATK_SHAPE_NAIVE_TRIANG  = 0x00000002,
    /**
     * A naive square wave
     *
     * The waveform will have discontinuities at PI and 2PI. This will
     * create a harsh sound reminiscent of old-school games. The initial
     * generated value is 1.
     *
     * An upper half square wave is a waveform of the same shape but
     * scaled and shifted to [0,1] instead of [-1,1].
     */
    ATK_SHAPE_NAIVE_SQUARE  = 0x00000003,
    /**
     * A naive sawtooth wave
     *
     * The waveform will have a discontinuity at 2PI. This will create a
     * harsh sound reminiscent of old-school games.  The initial generated
     * value is -1.
     *
     * An upper half sawtooth wave is a waveform of the same shape but
     * scaled and shifted to [0,1] instead of [-1,1].
     */

    ATK_SHAPE_NAIVE_TOOTH   = 0x00000004,
    /**
     * An alternating sign impulse train
     *
     * The frequence of the waveform is the twice the period of the impulse.
     * The impulses occur at phases 0 and PI. In an upper half impulse train,
     * both values will be +1.  Otherwise, these values will be +1 and -1,
     * respectively. The initial generated value is 1.
     */
    ATK_SHAPE_NAIVE_TRAIN   = 0x00000005,
    /**
     * A bandlimited triangle wave
     *
     * When this type is selected, the algorithm uses a PolyBLEP curve to
     * create a bandlimited triangle wave, as reported in "Antialiasing
     * Oscillators in Subtractive Synthesis" by Valimaki and Huovilainen (2007).
     * This wave is then integrated to produce a triangle wave, using the
     * leaky integration in "Alias-Free Digital Synthesis of Classic Analog
     * Waveforms" by Stilson and Smith (1996). This particular version is
     * adapted from
     *
     *     http://www.martin-finke.de/blog/articles/audio-plugins-018-polyblep-oscillator/
     *
     * The initial generated value is 0.
     *
     * An upper half triangle wave is a waveform of the same shape but
     * scaled and shifted to [0,1] instead of [-1,1].
     */
    ATK_SHAPE_POLY_TRIANG   = 0x00000006,
    /**
     * A bandlimited square wave
     *
     * When this type is selected, the algorithm uses a PolyBLEP curve as
     * reported in "Antialiasing Oscillators in Subtractive Synthesis" by
     * Valimaki and Huovilainen (2007). This particular version is adapted
     * from
     *
     *     http://www.martin-finke.de/blog/articles/audio-plugins-018-polyblep-oscillator/
     *
     * The initial generated value is 0.
     *
     * An upper half square wave is a waveform of the same shape but
     * scaled and shifted to [0,1] instead of [-1,1].
     */
    ATK_SHAPE_POLY_SQUARE   = 0x00000007,
    /**
     * A bandlimited sawtooth wave with the given frequency.
     *
     * When this type is selected, the algorithm uses a PolyBLEP curve as
     * reported in "Antialiasing Oscillators in Subtractive Synthesis" by
     * Valimaki and Huovilainen (2007). This particular version is adapted
     * from
     *
     *     http://www.martin-finke.de/blog/articles/audio-plugins-018-polyblep-oscillator/
     *
     * The initial generated value is 0.
     *
     * An upper half sawtooth wave is a waveform of the same shape but
     * scaled and shifted to [0,1] instead of [-1,1].
     */

    ATK_SHAPE_POLY_TOOTH    = 0x00000008,
    /**
     * A bandlimited impulse train
     *
     * When this type is selected, the algorithm uses the closed-form algorithm
     * "Alias-Free Digital Synthesis of Classic Analog Waveforms" by Stilson
     * and Smith (1996). This implementation assumes the maximum number of
     * harmonics, and is based on code by Robin Davies, Gary Scavone, 2005-2006.
     *
     * Note that the classic algorithm is for creating an "upper half" impulse
     * train, with impulses at +1 at phases 0 and PI for period p. However,
     * that algorithm has oscillations around 0, and can still produce negative
     * values. For the regular (alternating) impulse train, we take two half
     * rate impulse trains (so one impulse per period) and subtract them with
     * the second phase shifted by PI. The result produces impulses +1 at
     * 0 and -1 at PI.
     */
    ATK_SHAPE_BLIT_TRAIN    = 0x00000009
} ATK_WaveformShape;

/**
 * A waveform generator
 *
 * This type is used to generate a wave of shape {@link ATK_WaveShape}.
 * Generators are stateful, in that they can be used to generate the waveform
 * in separate chunks at a time. This allows for efficient waveform creation
 * without significant memory overhead.
 *
 * Waveforms can be normal or upper-half only. The meaning of upper half
 * (which generally implies only nonnegative samples) depends on the actual
 * shape.  See {@link ATK_WaveformShape} for more information.
 */
typedef struct ATK_WaveformGen ATK_WaveformGen;

/**
 * Returns a newly allocated waveform generator for the given shape and frequency
 *
 * Frequencies are specified in "normalized" format. A normalized frequency
 * is frequency/sample rate. For example, a 7 kHz frequency with a 44100 Hz
 * sample rate has a normalized value 7000/44100 = 0.15873.
 *
 * While the output of a generator is in floats, our parameters are doubles in
 * order to preserve precision over time. When a generator is used, the first
 * sample depends upon the shape and the initial phase [0,2PI). The shapes in
 * {@link ATK_WaveformShape} are defined assuming an initial phase of 0.
 *
 * @param shape The waveform shape
 * @param freq  The normalized frequency (frequence / sample rate)
 * @param phase The initial phase [0,2PI)
 *
 * @return a newly allocated waveform generator for the given shape and frequency
 */
extern DECLSPEC ATK_WaveformGen* SDLCALL ATK_AllocWaveform(ATK_WaveformShape shape, double freq, double phase);

/**
 * Returns a newly allocated upper-half waveform generator
 *
 * The meaning of upper half generally means no negative values generated.
 * The exact meaning depends on the shaope. See {@link ATK_WaveformShape}
 * for more details.
 *
 * Frequencies are specified in "normalized" format. A normalized frequency
 * is frequency/sample rate. For example, a 7 kHz frequency with a 44100 Hz
 * sample rate has a normalized value 7000/44100 = 0.15873.
 *
 * While the output of a generator is in floats, our parameters are doubles in
 * order to preserve precision over time. When a generator is used, the first
 * sample depends upon the shape and the initial phase [0,2PI). The shapes in
 * {@link ATK_WaveformShape} are defined assuming an initial phase of 0.
 *
 * @param shape The waveform shape
 * @param freq  The normalized frequency (frequence / sample rate)
 * @param phase The initial phase [0,2PI)
 *
 * @return a newly allocated waveform generator for the given shape and frequency
 */
extern DECLSPEC ATK_WaveformGen* SDLCALL ATK_AllocUpperWaveform(ATK_WaveformShape shape, double freq, double phase);

/**
 * Frees a previously allocated waveform generator
 *
 * @param gen   The waveform generator
 */
extern DECLSPEC void SDLCALL ATK_FreeWaveform(ATK_WaveformGen* gen);

/**
 * Resets the waveform generator to its initial state
 *
 * The generator will be in the state it was when first created.  When a generator
 * is used, the first sample is guaranteed to be 0.
 *
 * @param gen   The waveform generator
 */
extern DECLSPEC void SDLCALL ATK_ResetWaveform(ATK_WaveformGen* gen);

/**
 * Returns the next sample created by this waveform generator
 *
 * Waveform generators are stateful, in that a loop over this function is
 * identical to {@link ATK_WaveformFill}. However, this also means that the
 * same generator should not be used on multiple channels in multichannel
 * audio.
 *
 * @param gen       The waveform generator
 *
 * @return the next sample created by this waveform generator
 */
extern DECLSPEC float SDLCALL ATK_StepWaveform(ATK_WaveformGen* gen);

/**
 * Fills the buffer using data from the waveform generator
 *
 * Waveform generators are stateful, in that a single call to this function is
 * indentical to two calls to the function on the two halves of the buffer.
 *
 * @param gen       The waveform generator
 * @param buffer    The buffer to fill
 * @param size      The size of the buffer
 *
 * @return 0 if data was successfully generated; -1 if an error.
 */
extern DECLSPEC Sint32 SDLCALL ATK_FillWaveform(ATK_WaveformGen* gen, float* buffer, size_t size);

/**
 * Fills the stride-aware buffer using data from the waveform generator
 *
 * The buffer will only be filled at every stride entries. This is useful for
 * embedding a waveform into a channel for multichannel audio.
 *
 * Waveform generators are stateful, in that a single call to this function is
 * indentical to two calls to the function on the two halves of the buffer.
 * However, this also means that the same generator should not be used on
 * multiple channels in multichannel audio.
 *
 * @param gen       The waveform generator
 * @param buffer    The buffer to fill
 * @param stride    The buffer stride
 * @param size      The size of the buffer
 *
 * @return 0 if data was successfully generated; -1 if an error.
 */
extern DECLSPEC Sint32 SDLCALL ATK_FillWaveform_stride(ATK_WaveformGen* gen, float* buffer,
                                                       size_t stride, size_t size);

#pragma mark -
#pragma mark FFT Support
/**
 * The internal state for a real-valued FFT
 *
 * This algorithm is 45% faster than a complex-valued FFT on real-valued
 * signals.
 *
 * A real-valued FFT can either be a normal FFT or an inverse. Inverse
 * real-value FFTs may only be used with {@link ATK_ApplyRealInvFFT}
 * and {@link ATK_ApplyRealInvFFT_stride}.
 */
typedef struct ATK_RealFFT ATK_RealFFT;

/**
 * Returns the best real-valued FFT size for the given window length
 *
 * The result will be a value >= size.
 *
 * @param size  The desired window length
 *
 * @return the  best real-valued FFT size for the given window length
 */
extern DECLSPEC size_t SDLCALL ATK_GetRealFFTBestSize(size_t size);

/**
 * Returns a newly allocated real-valued FFT
 *
 * The window length is suggestion. The actual length will be computed from
 * {@link ATK_GetRealFFTBestSize}. Use {@link ATK_GetRealFFTSize} to query the
 * actual size. The size of a real-valued FFT must be even.
 *
 * The resulting FFT can either be F or F^-1 (the inverse transform) as
 * specified. Like the scipy implementation, the inverse FFT is not just a
 * phase shift. It also normalizes the results, guaranteeing that it is a
 * true inverse on the input buffer. Note that inverse FFTs may only be
 * used by the functions {@link ATK_ApplyRealInvFFT} and
 * {@link ATK_ApplyRealInvFFT_stride}.
 *
 * @param size      The window length (suggested)
 * @param inverse   Whether to create the inverse transform
 *
 * @return a newly allocated real-valued FFT
 */
extern DECLSPEC ATK_RealFFT* SDLCALL ATK_AllocRealFFT(size_t size, SDL_bool inverse);

/**
 * Frees a previously allocated real-valued FFT
 *
 * @param fft   The FFT state
 */
extern DECLSPEC void SDLCALL ATK_FreeRealFFT(ATK_RealFFT* fft);

/**
 * Returns the window length of the real-valued FFT
 *
 * This value is the actual window length, and not (necessarily) the one
 * suggested at the time of creation
 *
 * @param fft   The FFT state
 *
 * @return the window length of the real-valued FFT
 */
extern DECLSPEC size_t SDLCALL ATK_GetRealFFTSize(ATK_RealFFT* fft);

/**
 * Applies the FFT to the real input signal, storing the result in output
 *
 * The input must be an array of floats of size N where N is the FFT size
 * given by {@link ATK_GetRealFFTSize}. The output will be an array of complex
 * numbers of size N/2+1 (so the array itself must be size N+2). The even
 * elements of this array are the real components while the odd values are
 * the imaginary components.
 *
 * The output arrary is shorter than N because the output of a real-valued
 * signal is symmetric. In this case the output is the first half of the
 * output values. This function will fail (returning -1 and setting an error)
 * if the fft is an inverse real-valued fft.
 *
 * @param fft       The FFT state
 * @param input     The input signal
 * @param output    The FFT output
 *
 * @return 0 if the FFT is successful, -1 otherwise
 */
extern DECLSPEC int SDLCALL ATK_ApplyRealFFT(ATK_RealFFT* fft, const float* input, float* output);

/**
 * Applies the FFT to the real input signal, storing the result in output
 *
 * The input must be an array of floats of size N where N is the FFT size
 * given by {@link ATK_GetRealFFTSize}. The output will be an array of complex
 * numbers of size N/2+1 (so the array itself must be size N+2). The stride
 * for output applies to the complex numbers, not the components. So if the
 * output buffer has stride 3, all positions at multiples of 6 are real,
 * followed by an imaginary at the next position.
 *
 * The output arrary is shorter than N because the output of a real-valued
 * signal is symmetric. In this case the output is the first half of the
 * output values. This function will fail (returning -1 and setting an error)
 * if the fft is an inverse real-valued fft.
 *
 * @param fft       The FFT state
 * @param input     The input signal
 * @param istride   The input stride
 * @param output    The FFT output
 * @param ostride   The output stride
 *
 * @return 0 if the FFT is successful, -1 otherwise
 */
extern DECLSPEC int SDLCALL ATK_ApplyRealFFT_stride(ATK_RealFFT* fft,
                                                    const float* input, size_t istride,
                                                    float* output, size_t ostride);


/**
 * Applies the inverse FFT to the real input signal, storing the result in output
 *
 * The output will be an array of floats of size N where N is the FFT size
 * given by {@link ATK_GetRealFFTSize}. The input must be an array of complex
 * numbers of size N/2+1 (so the array itself must be size N+2). The even
 * elements of this array are the real components while the odd values are
 * the imaginary components.
 *
 * The input array is shorter than N because the output of a real-valued
 * signal is symmetric. In this case the input is the first half of the
 * FFT values. This function will fail (returning -1 and setting an error)
 * if the fft is not an inverse real-valued fft.
 *
 * @param fft       The FFT state
 * @param input     The input signal
 * @param output    The FFT output
 *
 * @return 0 if the FFT is successful, -1 otherwise
 */
extern DECLSPEC int SDLCALL ATK_ApplyRealInvFFT(ATK_RealFFT* fft, const float* input, float* output);

/**
 * Applies the inverse FFT to the real input signal, storing the result in output
 *
 * The output will be an array of floats of size N where N is the FFT size
 * given by {@link ATK_GetRealFFTSize}. The input must be an array of complex
 * numbers of size N/2+1 (so the array itself must be size N+2). The stride
 * for input applies to the complex numbers, not the components. So if the
 * input buffer has stride 3, all positions at multiples of 6 are real,
 * followed by an imaginary at the next position.
 *
 * The input array is shorter than N because the output of a real-valued
 * signal is symmetric. In this case the input is the first half of the
 * FFT values. This function will fail (returning -1 and setting an error)
 * if the fft is not an inverse real-valued fft.
 *
 * @param fft       The FFT state
 * @param input     The input signal
 * @param istride   The input stride
 * @param output    The FFT output
 * @param ostride   The output stride
 *
 * @return 0 if the FFT is successful, -1 otherwise
 */
extern DECLSPEC int SDLCALL ATK_ApplyRealInvFFT_stride(ATK_RealFFT* fft,
                                                       const float* input, size_t istride,
                                                       float* output, size_t ostride);

/**
 * Applies the FFT to the real input signal, storing the magnitudes in output
 *
 * The input must be an array of floats of size N where N is the FFT size
 * given by {@link ATK_GetRealFFTSize}. The output will be an array of size
 * N/2+1, and will store the magnitudes of the FFT result.
 *
 * The output arrary is shorter than N because the output of a real-valued
 * signal is symmetric. In this case the output is the first half of the
 * output values. This function will fail (returning -1 and setting an error)
 * if the fft is an inverse real-valued fft.
 *
 * @param fft       The FFT state
 * @param input     The input signal
 * @param output    The FFT output
 *
 * @return 0 if the FFT is successful, -1 otherwise
 */
extern DECLSPEC int SDLCALL ATK_ApplyRealFFTMag(ATK_RealFFT* fft, const float* input, float* output);

/**
 * Applies the FFT to the real input signal, storing the magnitudes in output
 *
 * The input must be an array of floats of size N where N is the FFT size
 * given by {@link ATK_GetRealFFTSize}. The output will be an array of size
 * N/2+1, and will store the magnitudes of the FFT result.
 *
 * The output arrary is shorter than N because the output of a real-valued
 * signal is symmetric. In this case the output is the first half of the
 * output values. This function will fail (returning -1 and setting an error)
 * if the fft is an inverse real-valued fft.
 *
 * @param fft       The FFT state
 * @param input     The input signal
 * @param istride   The input stride
 * @param output    The FFT output
 * @param ostride   The output stride
 *
 * @return 0 if the FFT is successful, -1 otherwise
 */
extern DECLSPEC int SDLCALL ATK_ApplyRealFFTMag_stride(ATK_RealFFT* fft,
                                                       const float* input, size_t istride,
                                                       float* output, size_t ostride);

/**
 * The internal state for a complex-valued FFT
 *
 * This algorithm is slower than a real-valued FFT on real-valued signals.
 * It should only be used for properly complex input.
 */
typedef struct ATK_ComplexFFT ATK_ComplexFFT;

/**
 * Returns the best complex-valued FFT size for the given window length
 *
 * The result will be a value >= size.
 *
 * @param size  The desired window length
 *
 * @return the  best complex-valued FFT size for the given window length
 */
extern DECLSPEC size_t SDLCALL ATK_GetComplexFFTBestSize(size_t size);

/**
 * Returns a newly allocated complex-valued FFT
 *
 * The window length is suggestion. The actual length will be computed from
 * {@link ATK_GetComplexFFTBestSize}. Use {@link ATK_GetComplexFFTSize} to
 * query the actual size.
 *
 * The resulting FFT can either be F or F^-1 (the inverse transform) as
 * specified. Like the scipy implementation, the inverse FFT is not just a
 * phase shift. It also normalizes the results, guaranteeing that it is a
 * true inverse on the input buffer.
 *
 * @param size      The window length (suggested)
 * @param inverse   Whether to create the inverse transform
 *
 * @return a newly allocated complex-valued FFT
 */
extern DECLSPEC ATK_ComplexFFT* SDLCALL ATK_AllocComplexFFT(size_t size, SDL_bool inverse);

/**
 * Frees a previously allocated complex-valued FFT
 *
 * @param fft   The FFT state
 */
extern DECLSPEC void SDLCALL ATK_FreeComplexFFT(ATK_ComplexFFT* fft);

/**
 * Returns the window length of the complex-valued FFT
 *
 * This value is the actual window length, and not (necessarily) the one
 * suggested at the time of creation
 *
 * @param fft   The FFT state
 *
 * @return the window length of the complex-valued FFT
 */
extern DECLSPEC size_t SDLCALL ATK_GetComplexFFTSize(ATK_ComplexFFT* fft);

/**
 * Applies the FFT to the input signal, storing the result in output
 *
 * Both input and output should be size 2*{@link ATK_GetComplexFFTSize}.
 * The should consist of interleaved real and imaginary values where the
 * even positions are the real components and the odd positions are the
 * imaginary components.
 *
 * @param fft       The FFT state
 * @param input     The input signal
 * @param output    The FFT output
 */
extern DECLSPEC void SDLCALL ATK_ApplyComplexFFT(ATK_ComplexFFT* fft, const float* input, float* output);

/**
 * Applies the FFT to the complex input signal, storing the result in output
 *
 * Both input and output should be size 2*{@link ATK_GetComplexFFTSize}.
 * The should consist of interleaved real and imaginary values where the
 * even positions are the real components and the odd positions are the
 * imaginary components.
 *
 * The stride is applied to the complex numbers, not the components. So if
 * a buffer has stride 3, all positions at multiples of 6 are real, followed
 * by an imaginary at the next position.
 *
 * @param fft       The FFT state
 * @param input     The input signal
 * @param istride   The input stride
 * @param output    The FFT output
 * @param ostride   The output stride
 */
extern DECLSPEC void SDLCALL ATK_ApplyComplexFFT_stride(ATK_ComplexFFT* fft, const float* input, size_t istride,
                                                        float* output, size_t ostride);

/**
 * Applies the FFT to the input separated into real and complex components
 *
 * Both the input (realin, imagin) and output (realout, imagout) should be
 * arrays of size {@link ATK_GetComplexFFTSize}. They consiste of the real
 * and imaginary components as separate arrays.
 *
 * @param fft       The FFT state
 * @param realin    The real component of the input signal
 * @param imagin    The imaginary component of the input signal
 * @param realout   The real component of the FFT output
 * @param imagout   The imaginary component of the FFT output
 */
extern DECLSPEC void SDLCALL ATK_ApplySplitComplexFFT(ATK_ComplexFFT* fft, const float* realin, const float* imagin,
                                                      float* realout, float* imagout);

/**
 * Applies the FFT to the input separated into real and complex components
 *
 * Both the input (realin, imagin) and output (realout, imagout) should be
 * arrays of size {@link ATK_GetComplexFFTSize}. They consiste of the real
 * and imaginary components as separate arrays.
 *
 * @param fft       The FFT state
 * @param realin    The real component of the input signal
 * @param ristride  The stride of the real input component
 * @param imagin    The imaginary component of the input signal
 * @param iistride  The stride of the imaginary input component
 * @param realout   The real component of the FFT output
 * @param rostride  The stride of the real output component
 * @param imagout   The imaginary component of the FFT output
 * @param iostride  The stride of the imaginary output component
 */
extern DECLSPEC void SDLCALL ATK_ApplySplitComplexFFT_stride(ATK_ComplexFFT* fft,
                                                             const float* realin, size_t ristride,
                                                             const float* imagin, size_t iistride,
                                                             float* realout, size_t rostride,
                                                             float* imagout, size_t iostride);

#pragma mark -
#pragma mark Filters
/**
 * Internal state of an IIR filter
 *
 * This type is used for FIR filters as well. The code will optimize for
 * the filter type. Filters are stateful and should be reset whenever they
 * are applied to a new audio signal.
 */
typedef struct ATK_IIRFilter  ATK_IIRFilter;

/**
 * Types for common first-order IIR filters
 *
 * These types are used with {@link ATK_AllocFOFilter} to create an optimized
 * filter. The meaning of the parameter in {@link ATK_AllocFOFilter}  is type
 * specific.
 */
typedef enum {
    /**
     * A first-order low-pass filter
     *
     * When used with {@link ATK_AllocFOFilter}, the parameter is the
     * normalized frequency (frequency / sample rate) of the cutoff.
     */
    ATK_FO_LOWPASS   = 0x00000001,
    /**
     * A first-order high-pass filter
     *
     * When used with {@link ATK_AllocFOFilter}, the parameter is the
     * normalized frequency (frequency / sample rate) of the cutoff.
     */
    ATK_FO_HIGHPASS  = 0x00000002,
    /**
     * A first-order all-pass filter
     *
     * When used with {@link ATK_AllocFOFilter}, the parameter is the allpass
     * coefficient.  The allpass filter has unity gain at all frequencies. The
     * parameter magnitude must be less than one to maintain filter stability.
     */
    ATK_FO_ALLPASS   = 0x00000003,
    /**
     * A first-order DC blocking filter
     *
     * When used with {@link ATK_AllocFOFilter}, the parameter is the pole for
     * the filter. The parameter magnitude should be close to (but less than)
     * one to minimize low-frequency attenuation.
     */
    ATK_DC_BLOCKING  = 0x00000004,
} ATK_FOFilter;

/** The default Q factor */
#define ATK_Q_VALUE (1.0/sqrt(2.0))

/**
 * Types for common second-order IIR filters
 *
 * These types are used with {@link ATK_AllocSOFilter} to create an optimized
 * filter. All filters are implemented as a biquad filter. The parameter qfactor
 * in {@link ATK_AllocSOFilter} is the Q factor of this filter, as described
 *
 *    https://www.motioncontroltips.com/what-are-biquad-and-other-filter-types-for-servo-tuning
 *
 * This quality factor represents the ratio of energy stored to energy dissipated
 * at the resonance frequency. While this has a specific meaning for a few of
 * the filters below (particularly the filters {@link ATK_SO_BANDPASS} and
 * {@link ATK_SO_RESONANCE}), for the many filters it is safe to set this value
 * to 1/sqrt(2), or ATK_Q_VALUE.
 *
 * The implementations for these types is taken from
 *
 *    http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
 */
typedef enum {
    /**
     * A second-order lowpass filter
     *
     * When used with {@link ATK_AllocSOFilter}, it is generally best to use
     * ATK_Q_VALUE for the Q factor.
     */
    ATK_SO_LOWPASS   = 0x00000001,
    /**
     * A second-order highpass filter
     *
     * When used with {@link ATK_AllocSOFilter}, it is generally best to use
     * ATK_Q_VALUE for the Q factor.
     */
    ATK_SO_HIGHPASS  = 0x00000002,
    /**
     * A second-order allpass filter
     *
     * When used with {@link ATK_AllocSOFilter}, it is generally best to use
     * ATK_Q_VALUE for the Q factor.
     */
    ATK_SO_ALLPASS   = 0x00000003,
    /**
     * A second-order bandpass filter
     *
     * When used with {@link ATK_AllocSOFilter}, it is generally best to use
     * the result of {@link ATK_GetBandwidthQ} for the Q factor.
     */
    ATK_SO_BANDPASS  = 0x00000004,
    /**
     * A second-order notch (bandstop) filter
     *
     * When used with {@link ATK_AllocSOFilter}, it is generally best to use
     * the result of {@link ATK_BandwidthQ} for the Q factor.
     */
    ATK_SO_NOTCH     = 0x00000005,
    /**
     * A second-order filter for a parametric equalizer
     *
     * When used with {@link ATK_AllocSOFilter}, it is generally best to use
     * the result of {@link ATK_BandwidthQ} for the Q factor.
     */
    ATK_SO_PEAK      = 0x00000006,
    /**
     * A second-order lowshelf filter
     *
     * When used with {@link ATK_AllocSOFilter}, it is generally best to use
     * the result of {@link ATK_ShelfSlopeQ} for the Q factor.
     */
    ATK_SO_LOWSHELF  = 0x00000007,
    /**
     * A second-order highshelf filter
     *
     * When used with {@link ATK_AllocSOFilter}, it is generally best to use
     * the result of {@link ATK_ShelfSlopeQ} for the Q factor.
     */
    ATK_SO_HIGHSHELF = 0x00000008,
    /**
     * A second-order resonance filter
     *
     * The frequency response has a resonance at the given frequency. The
     * Q factor defines the radius of this resonance.
     */
    ATK_SO_RESONANCE = 0x00000009,
} ATK_SOFilter;

/**
 * Returns the Q factor for the given bandwidth
 *
 * This value is used by the second-order filters {@link ATK_SO_BANDPASS},
 * {@link ATK_SO_NOTCH}, and {@link ATK_SO_RESONANCE}.
 *
 * @param bandwidth The filter bandwidth
 *
 * @return the Q factor for the given bandwidth
 */
extern DECLSPEC float SDLCALL ATK_BandwidthQ(float bandwidth);

/**
 * Returns the Q factor for the given slope
 *
 * This value is used by the second-order shelf filters.
 *
 * @param slope The shelf filter slope
 *
 * @return the Q factor for the given slope
 */
extern DECLSPEC float SDLCALL ATK_ShelfSlopeQ(float slope);

/**
 * Returns a newly allocated IIR (infinite impulse response) filter
 *
 * The resulting filter implements the standard difference equation:
 *
 *   a[0]*y[n] = b[0]*x[n]+...+b[nb]*x[n-nb]-a[1]*y[n-1]-...-a[na]*y[n-na]
 *
 * If a[0] is not equal to 1, the filter coeffcients are normalized by a[0].
 *
 * The values asize and bsize can be 0. If asize is 0, the filter is a FIR
 * filter. If asize and bsize are both 2, the result is a classic biquad filter.
 * Both first-order and second-order filters are optimized for better performance.
 *
 * The filter does NOT acquire ownership of the coefficient arrays a and b.
 * Disposing of the filter will leave these arrays unaffected. A newly allocated
 * filter will zero-pad its inputs for calculation.
 *
 * @param a     The a (feedback) coefficients
 * @param asize The number of a coefficients
 * @param b     The b (feedforward) coefficients
 * @param bsize The number of a coefficients
 *
 * @return a newly allocated IIR (infinite impulse response) filter
 */
extern DECLSPEC ATK_IIRFilter* SDLCALL ATK_AllocIIRFilter(float* a, size_t asize,
                                                          float* b, size_t bsize);

/**
 * Returns a newly allocated first-order filter.
 *
 * First order filters have at most 1 feedback and feedforward coefficient.
 * They typically have a semantic meaning, as defined by {@link ATK_FOFilter}.
 * The parameter value is filter specific.
 *
 * @param type  The filter type
 * @param param The type-specific parameter
 */
extern DECLSPEC ATK_IIRFilter* SDLCALL ATK_AllocFOFilter(ATK_FOFilter type, float param);

/**
 * Returns a newly allocated second-order filter.
 *
 * Second order filters have at most 2 feedback and feedforward coefficients
 * each. They are typically represented as biquad filter, where qfactor is
 * the classic biquad quality factor:
 *
 *    https://www.motioncontroltips.com/what-are-biquad-and-other-filter-types-for-servo-tuning
 *
 * For many applications, a Q factor of 1/sqrt(2) (or ATK_Q_VALUE) is sufficient.
 * Specialized filters should compute the Q factor from either {@link ATK_BandwidthQ}
 * or {@link ATK_ShelfSlopeQ}.
 *
 *
 * @param type      The filter type
 * @param freq      The normalized frequency (frequence / sample rate)
 * @param gain      The filter input gain
 * @param qfactor   The biquad quality factor
 *
 * @return a newly allocated second-order filter.
 */
extern DECLSPEC ATK_IIRFilter* SDLCALL ATK_AllocSOFilter(ATK_SOFilter type, float frequency,
                                                         float gain, float qfactor);

/**
 * Frees a previously allocated IIR (infinite impulse response) filter
 *
 * @param filter    The IIR filter
 */
extern DECLSPEC void SDLCALL ATK_FreeIIRFilter(ATK_IIRFilter* filter);

/**
 * Resets the state of a IIR (infinite impulse response) filter
 *
 * IIR filters have to keep state of the inputs they have received so far. This makes
 * it not safe to use a filter on multiple streams simultaneously. Reseting a filter
 * zeroes the state so that it is the same as if the filter were just allocated.
 *
 * @param filter    The IIR filter
 */
extern DECLSPEC void SDLCALL ATK_ResetIIRFilter(ATK_IIRFilter* filter);

/**
 * Returns the next value of the IIR (infinite impulse response) filter
 *
 * IIR filters have to keep state of the inputs they have received so far. This makes
 * it not safe to use a filter on multiple streams simultaneously.
 *
 * @param filter    The IIR filter
 * @param value     The input value
 *
 * @return the next value of this IIR (infinite impulse response) filter
 */
extern DECLSPEC float SDLCALL ATK_StepIIRFilter(ATK_IIRFilter* filter, float value);

/**
 * Applies the IIR filter to an input buffer, storing the result in output
 *
 * Both input and output should have size len. It is safe for these buffers to be the
 * same. IIR filters have to keep state of the inputs they have received so far. This
 * makes it not safe to use a filter on multiple streams simultaneously.
 *
 * @param filter    The IIR filter
 * @param input     The input buffer
 * @param output    The output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_ApplyIIRFilter(ATK_IIRFilter* filter, const float* input,
                                                float* output, size_t len);
/**
 * Applies the IIR filter to an input buffer, storing the result in output
 *
 * Both input and output should have size len. It is safe for these buffers to be the
 * same assuming that the strides match. IIR filters have to keep state of the inputs
 * they have received so far. This makes it not safe to use a filter on multiple streams
 * simultaneously.
 *
 * @param filter    The IIR filter
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_ApplyIIRFilter_stride(ATK_IIRFilter* filter,
                                                       const float* input,  size_t istride,
                                                       float* output, size_t ostride, size_t len);

/**
 * A long-running integral delay filter
 *
 * This filter requires a buffer the size of the delay. The value represents
 * the maximum delay. However, it is possible to use this filter to apply
 * any delay up to its maximum value.
 */
typedef struct ATK_DelayFilter ATK_DelayFilter;

/**
 * Returns a newly allocated delay filter.
 *
 * The filter starts off zero padded, so that all results of the filter are
 * zero until the delay is reached.
 *
 * The delay specified is the maxium delay length. It is possible to have smaller
 * delays using {@link ATK_TapOutDelayFilter} or {@link ATK_TapApplyDelayFilter}.
 *
 * @param delay     The maximum delay length
 *
 * @return a newly allocated delay filter.
 */
extern DECLSPEC ATK_DelayFilter* SDLCALL ATK_AllocDelayFilter(size_t delay);

/**
 * Frees a previously allocated delay filter.
 *
 * @param filter    The delay filter
 */
extern DECLSPEC void SDLCALL ATK_FreeDelayFilter(ATK_DelayFilter* filter);

/**
 * Resets a delay filter to its initial state.
 *
 * The filter buffer will be zeroed, so that no data is stored in the filter.
 *
 * @param filter    The delay filter
 */
extern DECLSPEC void SDLCALL ATK_ResetDelayFilter(ATK_DelayFilter* filter);

/**
 * Returns the maximum delay supported by this filter.
 *
 * @param filter    The delay filter
 *
 * @return the maximum delay supported by this filter.
 */
extern DECLSPEC size_t SDLCALL ATK_GetDelayFilterMaximum(ATK_DelayFilter* filter);

/**
 * Returns the next value of the delay filter.
 *
 * The value returned will have maximum delay. Delay filters have to keep state of the
 * inputs they have received so far, so this function moves the filter forward. This makes
 * it not safe to use a filter on multiple streams simultaneously.
 *
 * @param filter    The delay filter
 * @param value     The input value
 *
 * @return the next value of the delay filter.
 */
extern DECLSPEC float SDLCALL ATK_StepDelayFilter(ATK_DelayFilter* filter, float value);

/**
 * Returns the value in this filter with the given tap position.
 *
 * The value tap should be less than the maximum delay. This function does not
 * modify the filter or move it forward (e.g. the state is unchanged).
 *
 * @param filter    The delay filter
 * @param tap       The tap position
 *
 * @return the value in this filter with the given tap position.
 */
extern DECLSPEC float SDLCALL ATK_TapOutDelayFilter(ATK_DelayFilter* filter, size_t tap);

/**
 * Sets the filter tap position to have the given value.
 *
 * The value tap should be less than the maximum delay. This function does modify
 * the filter at the given position, but does not move it forward (so calls to
 * {@link ATK_StepDelayFilter} are unaffected if this tap is not at the end).
 *
 * @param filter    The delay filter
 * @param tap       The tap position
 * @param value     The value for the tap position
 *
 * @return the value in this filter with the given tap position.
 */
extern DECLSPEC void SDLCALL ATK_TapInDelayFilter(ATK_DelayFilter* filter, size_t tap, float value);

/**
 * Applies the delay to an input buffer, storing the result in output
 *
 * The values stored in output will have maximum delay. Both input and output
 * should have size len. It is safe for these two buffers to be the same.
 *
 * Delay filters have to keep state of the inputs they have received so far, so
 * this function moves the filter forward by the given length. This makes it not
 * safe to use a filter on multiple streams simultaneously.
 *
 * @param filter    The delay filter
 * @param input     The input buffer
 * @param output    The output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_ApplyDelayFilter(ATK_DelayFilter* filter, const float* input,
                                                  float* output, size_t len);
/**
 * Applies the delay to an input buffer, storing the result in output
 *
 * The values stored in output will have maximum delay. Both input and output
 * should have size len. It is safe for these two buffers to be the same provided
 * that the strides match.
 *
 * Delay filters have to keep state of the inputs they have received so far, so
 * this function moves the filter forward by the given length. This makes it not
 * safe to use a filter on multiple streams simultaneously.
 *
 * @param filter    The delay filter
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_ApplyDelayFilter_stride(ATK_DelayFilter* filter,
                                                         const float* input,  size_t istride,
                                                         float* output, size_t ostride, size_t len);

/**
 * Applies a tapped delay to an input buffer, storing the result in output
 *
 * The values stored in output will be delayed by the given tap. Both input and
 * output should have size len. It is safe for these two buffers to be the same.
 *
 * Delay filters have to keep state of the inputs they have received so far, so
 * this function moves the filter forward by the given length. This means that
 * the last len delayed values will be lost if tap is less than the maximum delay.
 * If you want to have a delay less than the maximum delay without losing state,
 * you should use {@link ATK_TapOutDelayFilter}.
 *
 * @param filter    The delay filter
 * @param input     The input buffer
 * @param output    The output buffer
 * @param tap       The delay tap position
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_TapApplyDelayFilter(ATK_DelayFilter* filter, const float* input,
                                                     float* output, size_t tap, size_t len);
/**
 * Applies a tapped delay to an input buffer, storing the result in output
 *
 * The values stored in output will have maximum delay. Both input and output
 * should have size len. It is safe for these two buffers to be the same provided
 * that the strides match.
 *
 * Delay filters have to keep state of the inputs they have received so far, so
 * this function moves the filter forward by the given length. This means that
 * the last len delayed values will be lost if tap is less than the maximum delay.
 * If you want to have a delay less than the maximum delay without losing state,
 * you should use {@link ATK_TapOutDelayFilter}.
 *
 * @param filter    The delay filter
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param tap       The delay tap position
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_TapApplyDelayFilter_stride(ATK_DelayFilter* filter,
                                                            const float* input,  size_t istride,
                                                            float* output, size_t ostride,
                                                            size_t tap, size_t len);
/**
 * A fractional delay filter.
 *
 * Fractional delay filters can be computed using either linear or allpass
 * interpolation. Linear interpolation is efficient but it does introduce
 * high-frequency signal attenuation. Allpass interpolation has unity magnitude
 * gain but variable phase delay properties, making it useful in achieving
 * fractional delays without affecting a signal's frequency magnitude response.
 * The algorithm must be set at the time of filter creation. Both are taken
 * from STK by Perry R. Cook and Gary P. Scavone, 1995--2021.
 *
 *     https://github.com/thestk/stk
 *
 * As with a normal {@link ATK_DelayFilter} the filter delay represents the
 * maximum delay. It is possible to use this filter to apply any delay up to
 * its maximum value. However, due to state limitations, any tap uses linear
 * interpolation, regardless of the filter type.
 */
typedef struct ATK_FractionalFilter ATK_FractionalFilter;

/**
 * Returns a newly allocated fractional delay filter.
 *
 * The filter starts off zero padded, so that all results of the filter are
 * zero until the delay is reached. The delay cannot be less than 0.5.
 *
 * If allpass is non-zero, this filter will use allpass interpolation. Allpass
 * interpolation has unity magnitude gain but variable phase delay properties,
 * making it useful in achieving fractional delays without affecting a signal's
 * frequency magnitude response. Otherwise, it will use linear interpolation,
 * which is efficient but does introduce high-frequency signal attenuation.
 *
 * The delay specified is the maxium delay length. It is possible to have
 * smaller fractional delays using {@link ATK_TapOutFractionalFilter} or
 * {@link ATK_TapApplyFractionalFilter}. Not that all taps must use linear
 * interpolation, regardless of the filter type.
 *
 * @param delay     The maximum delay length
 * @param allpass   Whether to use allpass interpolation
 *
 * @return a newly allocated fractional delay filter.
 */
extern DECLSPEC ATK_FractionalFilter* SDLCALL ATK_AllocFractionalFilter(float delay, int allpass);

/**
 * Frees a previously allocated fractional delay filter.
 *
 * @param filter    The fractional delay filter
 */
extern DECLSPEC void SDLCALL ATK_FreeFractionalFilter(ATK_FractionalFilter* filter);

/**
 * Resets a fractional delay filter to its initial state.
 *
 * The filter buffer will be zeroed, so that no data is stored in the filter.
 *
 * @param filter    The fractional delay filter
 */
extern DECLSPEC void SDLCALL ATK_ResetFractionalFilter(ATK_FractionalFilter* filter);

/**
 * Returns the maximum delay supported by this filter.
 *
 * @param filter    The fractional delay filter
 *
 * @return the maximum delay supported by this filter.
 */
extern DECLSPEC float SDLCALL ATK_GetFractionalFilterDelay(ATK_FractionalFilter* filter);

/**
 * Returns the next value of the delay filter.
 *
 * The value returned will have maximum delay. Fractional delay filters have
 * to keep state of the inputs they have received so far, so this function
 * moves the filter forward. This makes it not safe to use a filter on multiple
 * streams simultaneously.
 *
 * This function will use allpass interpolation if the filter was allocated with
 * that option.
 *
 * @param filter    The fractional delay filter
 * @param value     The input value
 *
 * @return the next value of the delay filter.
 */
extern DECLSPEC float SDLCALL ATK_StepFractionalFilter(ATK_FractionalFilter* filter, float value);

/**
 * Returns the value in this filter with the given tap position.
 *
 * The value tap should be less than the maximum delay. This function does
 * not modify the filter or move it forward (e.g. the state is unchanged). Note
 * that all tapped outputs must use linear interpolation.
 *
 * @param filter    The fractional delay filter
 * @param tap       The tap position
 *
 * @return the value in this filter with the given tap position.
 */
extern DECLSPEC float SDLCALL ATK_TapOutFractionalFilter(ATK_FractionalFilter* filter, float tap);

/**
 * Sets the filter tap position to have the given value.
 *
 * The value tap should be less than the maximum delay. This function does
 * modify the filter at the given position, but does not move it forward
 * (so calls to {@link ATK_StepFractionalFilter} are unaffected if this tap
 * is not at the end). Note that even though the delay is fractional, input
 * taps must be integral.
 *
 * @param filter    The fractional delay filter
 * @param tap       The tap position
 * @param value     The value for the tap position
 *
 * @return the value in this filter with the given tap position.
 */
extern DECLSPEC void SDLCALL ATK_TapInFractionalFilter(ATK_FractionalFilter* filter, size_t tap, float value);

/**
 * Applies the delay to an input buffer, storing the result in output
 *
 * The values stored in output will have maximum delay. Both input and output
 * should have size len. It is safe for these two buffers to be the same.
 *
 * Fractional delay filters have to keep state of the inputs they have
 * received so far, so this function moves the filter forward by the given
 * length. This makes it not safe to use a filter on multiple streams
 * simultaneously.
 *
 * This function will use allpass interpolation if the filter was allocated with
 * that option.
 *
 * @param filter    The fractional delay filter
 * @param input     The input buffer
 * @param output    The output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_ApplyFractionalFilter(ATK_FractionalFilter* filter, const float* input,
                                                       float* output, size_t len);
/**
 * Applies the delay to an input buffer, storing the result in output
 *
 * The values stored in output will have maximum delay. Both input and output
 * should have size len. It is safe for these two buffers to be the same
 * provided that the strides match.
 *
 * Fractional delay filters have to keep state of the inputs they have
 * received so far, so this function moves the filter forward by the given
 * length. This makes it not safe to use a filter on multiple streams
 * simultaneously.
 *
 * This function will use allpass interpolation if the filter was allocated with
 * that option.
 *
 * @param filter    The fractional delay filter
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_ApplyFractionalFilter_stride(ATK_FractionalFilter* filter,
                                                              const float* input,  size_t istride,
                                                              float* output, size_t ostride, size_t len);

/**
 * Applies a tapped delay to an input buffer, storing the result in output
 *
 * The values stored in output will be delayed by the given tap. Both input
 * and output should have size len. It is safe for these two buffers to be
 * the same.
 *
 * Fractional delay filters have to keep state of the inputs they have received
 * so far, so this function moves the filter forward by the given length. This
 * means that the last len delayed values will be lost if tap is less than the
 * maximum delay. If you want to have a delay less than the maximum delay without
 * losing state, you should use {@link ATK_TapOutDelayFilter}. Also note that
 * taped outputs always use linear interpolation.
 *
 * @param filter    The delay filter
 * @param input     The input buffer
 * @param output    The output buffer
 * @param tap       The delay tap position
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_TapApplyFractionalFilter(ATK_FractionalFilter* filter, const float* input,
                                                          float* output, float tap, size_t len);
/**
 * Applies a tapped delay to an input buffer, storing the result in output
 *
 * The values stored in output will be delayed by the given tap. Both input
 * and output should have size len. It is safe for these two buffers to be
 * the same, provided that the strides match.
 *
 * Fractional delay filters have to keep state of the inputs they have received
 * so far, so this function moves the filter forward by the given length. This
 * means that the last len delayed values will be lost if tap is less than the
 * maximum delay. If you want to have a delay less than the maximum delay without
 * losing state, you should use {@link ATK_TapOutDelayFilter}. Also note that
 * taped outputs always use linear interpolation.
 *
 * @param filter    The delay filter
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param tap       The delay tap position
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_TapApplyFractionalFilter_stride(ATK_FractionalFilter* filter,
                                                                 const float* input,  size_t istride,
                                                                 float* output, size_t ostride,
                                                                 float tap, size_t len);

/**
 * An allpass delay filter, such as the one used by FreeVerb.
 *
 * This filter has an integral delay, like {@link ATK_DelayFilter}. However,
 * it has additional feedback coefficients to introduce interferance in the
 * signal. Because of this interferance, we do not allow allpass filters to
 * be tapped in or out like a normal delay filter.
 */
typedef struct ATK_AllpassFilter ATK_AllpassFilter;

/**
 * Returns a newly allocated allpass filter.
 *
 * The filter starts off zero padded, so that all results of the filter are
 * zero until the delay is reached.
 *
 * The delay of this filter can never be resized. However, the coefficients can
 * be updated at any time with {@link ATK_UpdateAllpassFilter}.
 *
 * @param delay     The delay length
 * @param feedback  The feedback coefficient
 *
 * @return a newly allocated allpass filter.
 */
extern DECLSPEC ATK_AllpassFilter* SDLCALL ATK_AllocAllpassFilter(size_t delay, float feedback);

/**
 * Frees a previously allocated allpass filter.
 *
 * @param filter    The allpass filter
 */
extern DECLSPEC void SDLCALL ATK_FreeAllpassFilter(ATK_AllpassFilter* filter);

/**
 * Resets a allpass filter to its initial state.
 *
 * The filter buffer will be zeroed, so that no data is stored in the filter.
 *
 * @param filter    The allpass filter
 */
extern DECLSPEC void SDLCALL ATK_ResetAllpassFilter(ATK_AllpassFilter* filter);

/**
 * Updates the allpass filter feedback.
 *
 * The filter buffer is unaffected by this function. Note that the delay cannot
 * be altered.
 *
 * @param filter    The allpass filter
 * @param feedback  The feedback coefficient
 */
extern DECLSPEC void SDLCALL ATK_UpdateAllpassFilter(ATK_AllpassFilter* filter, float feedback);

/**
 * Returns the delay supported by this allpass filter.
 *
 * @param filter    The comb filter
 *
 * @return the delay supported by this allpass filter.
 */
extern DECLSPEC size_t SDLCALL ATK_GetAllpassFilterDelay(ATK_AllpassFilter* filter);

/**
 * Returns the next value of the allpass filter.
 *
 * Allpass filters have to keep state of the inputs they have received so far,
 * so this function moves the filter forward. This makes it not safe to use a
 * filter on multiple streams simultaneously.
 *
 * @param filter    The comb filter
 * @param value     The input value
 *
 * @return the next value of the comb filter.
 */
extern DECLSPEC float SDLCALL ATK_StepAllpassFilter(ATK_AllpassFilter* filter, float value);

/**
 * Applies the filter to an input buffer, storing the result in output
 *
 * Allpass filters have to keep state of the inputs they have received so far,
 * so this function moves the filter forward by the given length. This makes it
 * not safe to use a filter on multiple streams simultaneously.
 *
 * @param filter    The comb filter
 * @param input     The input buffer
 * @param output    The output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_ApplyAllpassFilter(ATK_AllpassFilter* filter, const float* input,
                                                    float* output, size_t len);
/**
 * Applies the filter to an input buffer, storing the result in output
 *
 * Allpass filters have to keep state of the inputs they have received so far,
 * so this function moves the filter forward by the given length. This makes it
 * not safe to use a filter on multiple streams simultaneously.

 *
 * @param filter    The comb filter
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_ApplyAllpassFilter_stride(ATK_AllpassFilter* filter,
                                                           const float* input,  size_t istride,
                                                           float* output, size_t ostride, size_t len);

/**
 * A comb delay filter, such as the one used by FreeVerb.
 *
 * This filter has an integral delay, like {@link ATK_DelayFilter}. However, it has
 * additional feedforward and feedback coefficients to introduce interferance in the
 * signal. Because of this interferance, we do not allow comb filters to be tapped
 * in or out like a normal delay filter.
 */
typedef struct ATK_CombFilter ATK_CombFilter;

/**
 * Returns a newly allocated comb filter.
 *
 * The filter starts off zero padded, so that all results of the filter are
 * zero until the delay is reached.
 *
 * The delay of this filter can never be resized. However, the coefficients can be
 * updated at any time with {@link ATK_UpdateCombFilter}.
 *
 * @param delay     The delay length
 * @param feedback  The feedback coefficient
 * @param damping   The filter damping factor
 *
 * @return a newly allocated comb filter.
 */
extern DECLSPEC ATK_CombFilter* SDLCALL ATK_AllocCombFilter(size_t delay, float feedback, float damping);

/**
 * Frees a previously allocated comb filter.
 *
 * @param filter    The comb filter
 */
extern DECLSPEC void SDLCALL ATK_FreeCombFilter(ATK_CombFilter* filter);

/**
 * Resets a comb filter to its initial state.
 *
 * The filter buffer will be zeroed, so that no data is stored in the filter.
 *
 * @param filter    The comb filter
 */
extern DECLSPEC void SDLCALL ATK_ResetCombFilter(ATK_CombFilter* filter);

/**
 * Updates the comb filter coefficients.
 *
 * The filter buffer is unaffected by this function. Note that the delay cannot
 * be altered.
 *
 * @param filter    The comb filter
 * @param feedback  The feedback coefficient
 * @param damping   The filter damping factor
 */
extern DECLSPEC void SDLCALL ATK_UpdateCombFilter(ATK_CombFilter* filter, float feedback, float damping);

/**
 * Returns the delay supported by this comb filter.
 *
 * @param filter    The comb filter
 *
 * @return the delay supported by this comb filter.
 */
extern DECLSPEC size_t SDLCALL ATK_GetCombFilterDelay(ATK_CombFilter* filter);

/**
 * Returns the next value of the comb filter.
 *
 * Comb filters have to keep state of the inputs they have received so far, so
 * this function moves the filter forward. This makes it not safe to use a filter
 * on multiple streams simultaneously.
 *
 * @param filter    The comb filter
 * @param value     The input value
 *
 * @return the next value of the comb filter.
 */
extern DECLSPEC float SDLCALL ATK_StepCombFilter(ATK_CombFilter* filter, float value);

/**
 * Applies the filter to an input buffer, storing the result in output
 *
 * Comb filters have to keep state of the inputs they have received so far, so
 * this function moves the filter forward by the given length. This makes it not
 * safe to use a filter on multiple streams simultaneously.
 *
 * @param filter    The comb filter
 * @param input     The input buffer
 * @param output    The output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_ApplyCombFilter(ATK_CombFilter* filter, const float* input,
                                                 float* output, size_t len);
/**
 * Applies the filter to an input buffer, storing the result in output
 *
 * Comb filters have to keep state of the inputs they have received so far, so
 * this function moves the filter forward by the given length. This makes it not
 * safe to use a filter on multiple streams simultaneously.
 *
 * @param filter    The comb filter
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_ApplyCombFilter_stride(ATK_CombFilter* filter,
                                                        const float* input,  size_t istride,
                                                        float* output, size_t ostride, size_t len);

/**
 * Applies the filter to an input buffer, adding the result to output
 *
 * Comb filters have to keep state of the inputs they have received so far, so
 * this function moves the filter forward by the given length. This makes it not
 * safe to use a filter on multiple streams simultaneously.
 *
 * @param filter    The comb filter
 * @param input     The input buffer
 * @param output    The output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_AddCombFilter(ATK_CombFilter* filter, const float* input,
                                               float* output, size_t len);
/**
 * Applies the filter to an input buffer, adding the result to output
 *
 * Comb filters have to keep state of the inputs they have received so far, so
 * this function moves the filter forward by the given length. This makes it not
 * safe to use a filter on multiple streams simultaneously.
 *
 * @param filter    The comb filter
 * @param input     The input buffer
 * @param istride   The data stride of the input buffer
 * @param output    The output buffer
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements in the buffers
 */
extern DECLSPEC void SDLCALL ATK_AddCombFilter_stride(ATK_CombFilter* filter,
                                                      const float* input,  size_t istride,
                                                      float* output, size_t ostride, size_t len);

#pragma mark -
#pragma mark Convolutions

// Window generation
/**
 * Returns a newly allocated Blackman window.
 *
 * Blackman windows are implemented as defined here
 *
 *    https://en.wikipedia.org/wiki/Window_function
 *
 * This function allocates an array that must be freed with ATK_free when it
 * is no longer needed.
 *
 * If half is non-zero, this function allocates a window only (size+1)/2 points.
 * These points are the first half of the window if half is negative, and the
 * second half of the window if half is positive. Either half will contain the
 * center point.
 *
 * @param size  The window size
 * @param half  Whether to restrict to a half window
 *
 * @returns a newly allocated Blackman window.
 */
extern DECLSPEC float* SDLCALL ATK_AllocBlackmanWindow(size_t size, int half);

/**
 * Fills the buffer with a Blackman window.
 *
 * Blackman windows are implemented as defined here
 *
 *    https://en.wikipedia.org/wiki/Window_function
 *
 * The buffer should be able to store size values if it is a full window. If
 * half is non-zero, this function will only copy (size+1)/2 points into the
 * buffer. These points are the first half of the window if half is negative,
 * and the second half of the window if half is positive. Either half will
 * contain the center point.
 *
 * @param buffer    The buffer to store the window
 * @param size      The window size
 * @param half      Whether to restrict to a half window
 */
extern DECLSPEC void SDLCALL ATK_FillBlackmanWindow(float* buffer, size_t size, int half);

/**
 * Returns a newly allocated Hamming window.
 *
 * Hamming windows are implemented as defined here
 *
 *    https://en.wikipedia.org/wiki/Window_function
 *
 * This function allocates an array that must be freed with ATK_free when it
 * is no longer needed.
 *
 * If half is non-zero, this function allocates a window only (size+1)/2 points.
 * These points are the first half of the window if half is negative, and the
 * second half of the window if half is positive. Either half will contain the
 * center point.
 *
 * @param size  The window size
 * @param half  Whether to restrict to a half window
 *
 * @returns a newly allocated Hamming window.
 */
extern DECLSPEC float* SDLCALL ATK_AllocHammingWindow(size_t size, SDL_bool half);

/**
 * Fills the buffer with a Hamming window.
 *
 * Hamming windows are implemented as defined here
 *
 *    https://en.wikipedia.org/wiki/Window_function
 *
 * The buffer should be able to store size values if it is a full window. If
 * half is non-zero, this function will only copy (size+1)/2 points into the
 * buffer. These points are the first half of the window if half is negative,
 * and the second half of the window if half is positive. Either half will
 * contain the center point.
 *
 * @param buffer    The buffer to store the window
 * @param size      The window size
 * @param half      Whether to restrict to a half window
 */
extern DECLSPEC void SDLCALL ATK_FillHammingWindow(float* buffer, size_t size, int half);

/**
 * Returns a newly allocated Hann window.
 *
 * Hann windows are implemented as defined here
 *
 *    https://en.wikipedia.org/wiki/Window_function
 *
 * This function allocates an array that must be freed with ATK_free when it
 * is no longer needed.
 *
 * If half is non-zero, this function allocates a window only (size+1)/2 points.
 * These points are the first half of the window if half is negative, and the
 * second half of the window if half is positive. Either half will contain the
 * center point.
 *
 * @param size  The window size
 * @param half  Whether to restrict to a half window
 *
 * @returns a newly allocated Hann window.
 */
extern DECLSPEC float* SDLCALL ATK_AllocHannWindow(size_t size, SDL_bool half);

/**
 * Fills the buffer with a Hann window.
 *
 * Hann windows are implemented as defined here
 *
 *    https://en.wikipedia.org/wiki/Window_function
 *
 * The buffer should be able to store size values if it is a full window. If
 * half is non-zero, this function will only copy (size+1)/2 points into the
 * buffer. These points are the first half of the window if half is negative,
 * and the second half of the window if half is positive. Either half will
 * contain the center point.
 *
 * @param buffer    The buffer to store the window
 * @param size      The window size
 * @param half      Whether to restrict to a half window
 */
extern DECLSPEC void SDLCALL ATK_FillHannWindow(float* buffer, size_t size, int half);

/**
 * Returns a newly allocated Kaiser window.
 *
 * Kaiser windows are implemented as defined here
 *
 *    https://ccrma.stanford.edu/~jos/sasp/Kaiser_Window.html
 *
 * This function allocates an array that must be freed with ATK_free when it
 * is no longer needed.
 *
 * If half is non-zero, this function allocates a window only (size+1)/2 points.
 * These points are the first half of the window if half is negative, and the
 * second half of the window if half is positive. Either half will contain the
 * center point.
 *
 * @param size  The window size
 * @param beta  The window beta parameter
 * @param half  Whether to restrict to a half window
 *
 * @returns a newly allocated Kaiser window.
 */
extern DECLSPEC float* SDLCALL ATK_AllocKaiserWindow(size_t size, float beta, int half);

/**
 * Fills the buffer with a Kaiser window.
 *
 * Kaiser windows are implemented as defined here
 *
 *    https://ccrma.stanford.edu/~jos/sasp/Kaiser_Window.html
 *
 * The buffer should be able to store size values if it is a full window. If
 * half is non-zero, this function will only copy (size+1)/2 points into the
 * buffer. These points are the first half of the window if half is negative,
 * and the second half of the window if half is positive. Either half will
 * contain the center point.
 *
 * @param buffer    The buffer to store the window
 * @param size      The window size
 * @param beta      The window beta parameter
 * @param half      Whether to restrict to a half window
 */
extern DECLSPEC void SDLCALL ATK_FillKaiserWindow(float* buffer, size_t size, float beta, int half);

/**
 * A one-dimensional (linear) convolution filter.
 *
 * A convolution filter has state. The state consists of the tail of the
 * previously executed convolution. This tail is preserved so that it can be
 * used in the overlap-add portion of the next convolution. This means that
 * it is not safe to use a convolution filter on multiple streams without
 * first reseting it.
 *
 * A convolution filter may be used for either naive or FFT convolutions.
 * Furthermore, it is possible to mix-and-match these algorithms as the tail
 * will be the same in each case. The choice of filter depends on the size
 * of the kernel and/or signal. For optimized code, the break-over point
 * for these buffers can be as high as 512 samples, depending on hardware.
 */
typedef struct ATK_Convolution ATK_Convolution;

/**
 * Returns a newly allocated convolution filter for the given kernel.
 *
 * The convolution block size is used to partition the convolution into blocks.
 * If it is zero, the block size will be the same size as the kernel, which
 * means that signal blocks will be padded to match. For best performance in
 * real-time playback, the blocksize should be the same size as the expected
 * signal, which is typically the buffer size of the output device. This can
 * make a significant performance difference on large kernels, such as those
 * used in convolutional reverb.
 *
 * This function will copy the kernel, and not try to acquire ownership of it.
 * Future changes to the kernel will leave this filter unaffected.
 *
 *
 * @param kernel    The convolution kernel
 * @param size      The kernel size
 * @param block     The convolution block size
 *
 * @return a newly allocated convolution filter for the given kernel.
 */
extern DECLSPEC ATK_Convolution* SDLCALL ATK_AllocConvolution(float* kernel, size_t size, size_t block);

/**
 * Frees a previously allocated convolution filter.
 *
 * @param filter    The convolution filter
 */
extern DECLSPEC void SDLCALL ATK_FreeConvolution(ATK_Convolution* filter);

/**
 * Resets a convolution filter.
 *
 * The internal buffer will be zeroed, reseting the convolution back
 * the beginning.
 *
 * @param filter    The convolution filter
 */
extern DECLSPEC void SDLCALL ATK_ResetConvolution(ATK_Convolution* filter);

/**
 * Scales a convolution by the given amount.
 *
 * This scaling factor is applied to the kernel, allowing for normalization
 * before a convolution is applied.
 *
 * @param filter    The convolution filter
 * @param scalar    The amount to scale the convolution
 */
extern DECLSPEC void SDLCALL ATK_ScaleConvolution(ATK_Convolution* filter, float scalar);

/**
 * Returns the size of the convolution kernel.
 *
 * @param filter    The convolution filter
 *
 * @return the size of the convolution kernel.
 */
extern DECLSPEC size_t SDLCALL ATK_GetConvolutionSize(ATK_Convolution* filter);

/**
 * Returns the next value in this convolution.
 *
 * This steps the convolution ahead by one value. This is a particularly
 * inefficient way to apply a convolution and should be avoided unless
 * completely necessary. Note that this function updates the convolution
 * state, where the state is tail of convolution for use in overlap-add.
 * This means that it is not safe to use a convolution filter on multiple
 * streams without first reseting it.
 *
 * @param filter    The convolution filter
 * @param value     The input value
 *
 * @return the next value in this convolution.
 */
extern DECLSPEC float SDLCALL ATK_StepConvolution(ATK_Convolution* filter, float value);

/**
 * Applies a naive convolution on the given input, storing it in output
 *
 * A naive convolution uses an O(nm) nested loop where n is the size of the
 * buffer and m is the size of the convolution. While generally slower, this
 * can be faster than an FFT convolution if either n or m are small.
 *
 * The input and output should both have size len. It is safe for these to be
 * the same buffer.
 *
 * Note that the restriction on size means this function does not place the
 * tail (e.g. the last {@link ATK_GetConvolutionSize}-1 elemetns) of the
 * convolution in output. Instead, it keeps it internally for later use in
 * overlap-add. That way, calling this function twice on two-halves of an
 * array is the same as calling it once on the entire array. This allows us
 * to apply convolutions to streaming data. To access the final tail of the
 * convolution, call {@link ATK_FinishConvolution}.
 *
 * @param filter    The convolution filter
 * @param input     The input values
 * @param output    The buffer to store len output values
 * @param len       The number of elements to process
 */
extern DECLSPEC void SDLCALL ATK_ApplyNaiveConvolution(ATK_Convolution* filter, const float* input,
                                                       float* output, size_t len);

/**
 * Applies a naive convolution on the given input, storing it in output
 *
 * A naive convolution uses an O(nm) nested loop where n is the size of the
 * buffer and m is the size of the convolution. While generally slower, this
 * can be faster than an FFT convolution if either n or m are small.
 *
 * The input and output should both have size len. It is safe for these to be
 * the same buffer, provided that the strides align.
 *
 * Note that the restriction on size means this function does not place the
 * tail (e.g. the last {@link ATK_GetConvolutionSize}-1 elemetns) of the
 * convolution in output. Instead, it keeps it internally for later use in
 * overlap-add. That way, calling this function twice on two-halves of an
 * array is the same as calling it once on the entire array. This allows us
 * to apply convolutions to streaming data. To access the final tail of the
 * convolution, call {@link ATK_FinishConvolution}.
 *
 * @param filter    The convolution filter
 * @param input     The input values
 * @param istride   The data stride of the input buffer
 * @param output    The buffer to store len output values
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to process
 */
extern DECLSPEC void SDLCALL ATK_ApplyNaiveConvolution_stride(ATK_Convolution* filter,
                                                              const float* input, size_t istride,
                                                              float* output, size_t ostride, size_t len);

/**
 * Applies an FFT convolution on the given input, storing it in output
 *
 * An FFT convolution breaks the convolution down into several O(n log n) size
 * convolutions where n is the minimum of len and the convolution kernel. This
 * is significantly faster on larger convolutions, those it can be worse than
 * a naive convolution if either len or the kernel are small.
 *
 * The input and output should both have size len. It is safe for these
 * to be the same buffer.
 *
 * Note that the restriction on size means this function does not place the
 * tail (e.g. the last {@link ATK_GetConvolutionSize}-1 elemetns) of the
 * convolution in output. Instead, it keeps it internally for later use in
 * overlap-add. That way, calling this function twice on two-halves of an
 * array is the same as calling it once on the entire array. This allows us
 * to apply convolutions to streaming data. To access the final tail of the
 * convolution, call {@link ATK_FinishConvolution}.
 *
 * @param filter    The convolution filter
 * @param input     The input values
 * @param output    The buffer to store len output values
 * @param len       The number of elements to process
 */
extern DECLSPEC void SDLCALL ATK_ApplyFFTConvolution(ATK_Convolution* filter, const float* input,
                                                     float* output, size_t len);


/**
 * Applies an FFT convolution on the given input, storing it in output
 *
 * An FFT convolution breaks the convolution down into several O(n log n) size
 * convolutions where n is the minimum of len and the convolution kernel. This
 * is significantly faster on larger convolutions, those it can be worse than
 * a naive convolution if either len or the kernel are small.
 *
 * The input and output should both have size len. It is safe for these
 * to be the same buffer, provided that the strides align.
 *
 * Note that the restriction on size means this function does not place the
 * tail (e.g. the last {@link ATK_GetConvolutionSize}-1 elemetns) of the
 * convolution in output. Instead, it keeps it internally for later use in
 * overlap-add. That way, calling this function twice on two-halves of an
 * array is the same as calling it once on the entire array. This allows us
 * to apply convolutions to streaming data. To access the final tail of the
 * convolution, call {@link ATK_FinishConvolution}.
 *
 * @param filter    The convolution filter
 * @param input     The input values
 * @param istride   The data stride of the input buffer
 * @param output    The buffer to store len output values
 * @param ostride   The data stride of the output buffer
 * @param len       The number of elements to process
 */
extern DECLSPEC void SDLCALL ATK_ApplyFFTConvolution_stride(ATK_Convolution* filter,
                                                            const float* input, size_t istride,
                                                            float* output, size_t ostride, size_t len);

/**
 * Completes the convolution, storing the final elements in buffer.
 *
 * At each step, the convolution keeps the tail in its internal state for
 * use in overlap-add. The function finishes the convolution, storing
 * this tail in the provided buffer. The buffer should be able to hold
 * {@link ATK_GetConvolutionSize}-1 elements, which is the size of this
 * tail.
 *
 * Once finished, the convolution filter will be reset and can be safely
 * reused. This function does not deallocate the filter.
 *
 * @param filter    The convolution filter
 * @param buffer    The buffer to store the convolution tail
 *
 * @return the number of elements stored in the buffer.
 */
extern DECLSPEC size_t SDLCALL ATK_FinishConvolution(ATK_Convolution* filter, float* buffer);

/**
 * Completes the convolution, storing the final elements in buffer.
 *
 * At each step, the convolution keeps the tail in its internal state for
 * use in overlap-add. The function finishes the convolution, storing
 * this tail in the provided buffer. The buffer should be able to hold
 * {@link ATK_GetConvolutionSize}-1 elements, which is the size of this
 * tail.
 *
 * Once finished, the convolution filter will be reset and can be safely
 * reused. This function does not deallocate the filter.
 *
 * @param filter    The convolution filter
 * @param buffer    The buffer to store the convolution tail
 * @param stride    The buffer stride
 *
 * @return the number of elements stored in the buffer.
 */
extern DECLSPEC size_t SDLCALL ATK_FinishConvolution_stride(ATK_Convolution* filter,
                                                            float* buffer, size_t stride);



/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif /* __ATK_DSP_H__ */
