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
#include <ATK_error.h>
#include <ATK_dsp.h>
#include <math.h>

/**
 * @file ATK_DSPWaveform.c
 *
 * This component contains the functions for generating popular waveforms.
 * These can be used to synthesize new sounds or to test the DSP features.
 */

/** Constant representing our phase period */
#define TWO_PI  (2*M_PI)
/** The difference between 1 and its successor wrt doubles */
#define EPSILON_LIMIT 2e-52

#pragma mark -
#pragma mark Waveform Functions

/**
 * Returns the value of a PolyBLEP curve at time t.
 *
 * This code is adapted from "Antialiasing Oscillators in Subtractive Synthesis"
 * by Valimaki and Huovilainen (2007), more specifically the version at
 *
 *     http://www.kvraudio.com/forum/viewtopic.php?t=375517
 *
 * @param t     the time parameter
 * @param dt    the time resolution (frequency/rate)
 *
 * @return the value of a PolyBLEP curve at time t.
 */
static double poly_blep(double t, double dt) {
    t = SDL_fmod(t,1);

    if (t < dt) {               // 0 <= t < 1
        t /= dt;
        return t+t - t*t - 1.0;
    } else if (t > 1.0 - dt) {  // -1 < t < 0
        t = (t - 1.0) / dt;
        return t*t + t+t + 1.0;
    }

    // 0 otherwise
    return 0.0;
}

/**
 * Returns the next value for a sine wave.
 *
 * An upper-half sine wave is the absolute value (or the rectified
 * sine wave). The value at 0 is 0.
 *
 * @param index     The generator index
 * @param freq      The normalized frequency (frequency / sample rate)
 * @param offset    The phase offset
 * @param upper     Whether to use the upper-half of the signal
 *
 * @return the next value for a sine wave.
 */
inline static double tick_naive_sine(Uint64 index, double freq, double offset, SDL_bool upper) {
    double step = freq * TWO_PI;
    if (upper) {
        return SDL_fabs(sin(step*index+offset));
    }
    return sin(step*index+offset);
}

/**
 * Fills buffer with the next size values for a sine wave.
 *
 * An upper-half sine wave is the absolute value (or the rectified
 * sine wave). The value at 0 is 0.
 *
 * @param index     The generator index
 * @param freq      The normalized frequency (frequency / sample rate)
 * @param offset    The phase offset
 * @param upper     Whether to use the upper-half of the signal
 * @param buffer    The buffer to fill
 * @param stride    The bufffer stride
 * @param size      The number of elements to generate
 *
 * @return the last value generated
 */
static double fill_naive_sine(Uint64 index, double freq, double offset, SDL_bool upper,
                              float* buffer, size_t stride, size_t size) {
    double last;
    double step = freq * TWO_PI;
    float* output = buffer;
    if (upper) {
        if (stride) {
            while (size--) {
                last = SDL_fabs(sin(step*(index++)+offset));
                *output = (float)last;
                output += stride;
            }
        } else {
            while (size--) {
                last = SDL_fabs(sin(step*(index++)+offset));
                *output++ = (float)last;
            }
        }
    } else {
        if (stride) {
            while (size--) {
                last = sin(step*(index++)+offset);
                *output = (float)last;
                output += stride;
            }
        } else {
            while (size--) {
                last = sin(step*(index++)+offset);
                *output++ = (float)last;
            }
        }
    }
    return last;
}

/**
 * Returns the next value for a naive triangular wave.
 *
 * The waveform will have first-order discontinuities at PI and 2PI.
 * This will create a more smoother sound than a square or sawtooth
 * wave of the same frequency. The value at 0 is -1.
 *
 * An upper half triangle wave is a waveform of the same shape but
 * scaled and shifted to [0,1] instead of [-1,1].
 *
 * @param index     The generator index
 * @param freq      The normalized frequency (frequency / sample rate)
 * @param offset    The phase offset
 * @param upper     Whether to use the upper-half of the signal
 *
 * @return the next value for a naive triangular wave.
 */
inline static double tick_naive_triang(Uint64 index, double freq, double offset, SDL_bool upper) {
    double t = SDL_fmod(freq*index+offset/TWO_PI,1);
    double value = -1.0 + 2.0*t;
    return (upper ? SDL_fabs(value) : 2.0*SDL_fabs(value) - 1.0);
}

/**
 * Fills buffer with the next size values for a naive triangular wave.
 *
 * The waveform will have first-order discontinuities at PI and 2PI.
 * This will create a more smoother sound than a square or sawtooth
 * wave of the same frequency. The value at 0 is -1.
 *
 * An upper half triangle wave is a waveform of the same shape but
 * scaled and shifted to [0,1] instead of [-1,1].
 *
 * @param index     The generator index
 * @param freq      The normalized frequency (frequency / sample rate)
 * @param offset    The phase offset
 * @param upper     Whether to use the upper-half of the signal
 * @param buffer    The buffer to fill
 * @param stride    The bufffer stride
 * @param size      The number of elements to generate
 *
 * @return the last value generated
 */
static double fill_naive_triang(Uint64 index, double freq, double offset, SDL_bool upper,
                                float* buffer, size_t stride, size_t size) {
    double value, t;
    double last;
    float* output = buffer;
    double off = offset/TWO_PI;
    if (upper) {
        if (stride) {
            while (size--) {
                t = SDL_fmod(freq*(index++)+off,1);
                value = -1.0 + 2.0*t;
                last = SDL_fabs(value);
                *output = (float)last;
                output += stride;
            }
        } else {
            while (size--) {
                t = SDL_fmod(freq*(index++)+off,1);
                value = -1.0 + 2.0*t;
                last = SDL_fabs(value);
                *output++ = (float)last;
            }
        }
    } else {
        if (stride) {
            while (size--) {
                t = SDL_fmod(freq*(index++)+off,1);
                value = -1.0 + 2.0*t;
                last =  2.0*SDL_fabs(value) - 1.0;
                *output = (float)last;
                output += stride;
            }
        } else {
            while (size--) {
                t = SDL_fmod(freq*(index++)+off,1);
                value = -1.0 + 2.0*t;
                last =  2.0*SDL_fabs(value) - 1.0;
                *output++ = (float)last;
            }
        }
    }
    return last;
}

/**
 * Returns the next value for a naive square wave.
 *
 * The waveform will have discontinuities at PI and 2PI. This will
 * create a harsh sound reminiscent of old-school games. The value
 * at 0 is 1.
 *
 * An upper half square wave is a waveform of the same shape but
 * scaled and shifted to [0,1] instead of [-1,1].
 *
 * @param index     The generator index
 * @param freq      The normalized frequency (frequency / sample rate)
 * @param offset    The phase offset
 * @param upper     Whether to use the upper-half of the signal
 *
 * @return the next value for a naive square wave.
 */
inline static double tick_naive_square(Uint64 index, double freq, double offset, SDL_bool upper) {
    double t = SDL_fmod(freq*index+offset/TWO_PI,1);
    return t <= 0.5 ? 1.0f : (upper ? 0.0f : -1.0f);
}

/**
 * Fills buffer with the next size values for a naive square wave.
 *
 * The waveform will have discontinuities at PI and 2PI. This will
 * create a harsh sound reminiscent of old-school games. The value
 * at 0 is 1.
 *
 * An upper half square wave is a waveform of the same shape but
 * scaled and shifted to [0,1] instead of [-1,1].
 *
 * @param index     The generator index
 * @param freq      The normalized frequency (frequency / sample rate)
 * @param offset    The phase offset
 * @param upper     Whether to use the upper-half of the signal
 * @param buffer    The buffer to fill
 * @param stride    The bufffer stride
 * @param size      The number of elements to generate
 *
 * @return the last value generated
 */
static double fill_naive_square(Uint64 index, double freq, double offset, SDL_bool upper,
                                float* buffer, size_t stride, size_t size) {
    double t;
    double last;
    float* output = buffer;
    double off = offset/TWO_PI;
    if (upper) {
        if (stride) {
            while (size--) {
                t = SDL_fmod(freq*(index++)+off,1);
                last = t <= 0.5 ? 1.0f : 0.0f;
                *output = (float)last;
                output += stride;
            }
        } else {
            while (size--) {
                t = SDL_fmod(freq*(index++)+off,1);
                last = t <= 0.5 ? 1.0f : 0.0f;
                *output++ = (float)last;
            }
        }
    } else {
        if (stride) {
            while (size--) {
                t = SDL_fmod(freq*(index++)+off,1);
                last = t <= 0.5 ? 1.0f : -1.0f;
                *output = (float)last;
                output += stride;
            }
        } else {
            while (size--) {
                t = SDL_fmod(freq*(index++)+off,1);
                last = t <= 0.5 ? 1.0f : -1.0f;
                *output++ = (float)last;
            }
        }
    }
    return last;
}

/**
 * Returns the next value for a naive sawtooth wave.
 *
 * The waveform will have a discontinuity at 2PI. This will create a
 * harsh sound reminiscent of old-school games. The value at 0 is -1.
 *
 * An upper half sawtooth wave is a waveform of the same shape but
 * scaled and shifted to [0,1] instead of [-1,1].
 *
 * @param index     The generator index
 * @param freq      The normalized frequency (frequency / sample rate)
 * @param offset    The phase offset
 * @param upper     Whether to use the upper-half of the signal
 *
 * @return the next value for a naive sawtooth wave.
 */
inline static double tick_naive_tooth(Uint64 index, double freq, double offset, SDL_bool upper) {
    double value = SDL_fmod(freq*index+offset/TWO_PI,1);
    return upper ? value : 2.0 * value - 1.0;
}

/**
 * Fills buffer with the next size values for a naive sawtooth wave.
 *
 * The waveform will have a discontinuity at 2PI. This will create a
 * harsh sound reminiscent of old-school games. The value at 0 is -1.
 *
 * An upper half sawtooth wave is a waveform of the same shape but
 * scaled and shifted to [0,1] instead of [-1,1].
 *
 * @param index     The generator index
 * @param freq      The normalized frequency (frequency / sample rate)
 * @param offset    The phase offset
 * @param upper     Whether to use the upper-half of the signal
 * @param buffer    The buffer to fill
 * @param stride    The bufffer stride
 * @param size      The number of elements to generate
 *
 * @return the last value generated
 */
static double fill_naive_tooth(Uint64 index, double freq, double offset, SDL_bool upper,
                               float* buffer, size_t stride, size_t size) {
    double value;
    double last;
    float* output = buffer;
    double off = offset/TWO_PI;
    if (upper) {
        if (stride) {
            while (size--) {
                value = SDL_fmod(freq*(index++)+off,1);
                last = value;
                *output = (float)last;
                output += stride;
            }
        } else {
            while (size--) {
                value = SDL_fmod(freq*(index++)+off,1);
                last = value;
                *output++ = (float)last;
            }
        }
    } else {
        if (stride) {
            while (size--) {
                value = SDL_fmod(freq*(index++)+off,1);
                last = 2.0 * value - 1.0;
                *output = (float)last;
                output += stride;
            }
        } else {
            while (size--) {
                value = SDL_fmod(freq*(index++)+off,1);
                last = 2.0 * value - 1.0;
                *output++ = (float)last;
            }
        }
    }
    return last;
}

/**
 * Returns the next value for a naive impulse train.
 *
 * The frequence of the waveform is the twice the period of the impulse.
 * The impulses occur at phase 0 and PI. In an upper half impulse train,
 * both values will be +1. Otherwise these values will be +1 and -1,
 * respectively.
 *
 * @param index     The generator index
 * @param freq      The normalized frequency (frequency / sample rate)
 * @param offset    The phase offset
 * @param upper     Whether to use the upper-half of the signal
 *
 * @return the next value for a naive impulse train.
 */
inline static double tick_naive_train(Uint64 index, double freq, double offset, SDL_bool upper) {
    double t = SDL_fmod(freq*index+offset/TWO_PI,1);
    if (upper && (t <= freq/2 || SDL_fabs(t-0.5) <= freq/2)) {
        return 1.0;
    } else {
        // Peaks at 1/4 and 3/4 of the period
        if (t <= freq/2) {
            return 1.0;
        } else if (SDL_fabs(t-0.5) <= freq/2) {
            return -1.0;
        }
    }
    return 0;
}

/**
 * Fills buffer with the next size values for a naive impulse train.
 *
 * The frequence of the waveform is the twice the period of the impulse.
 * The impulses occur at phase 0 and PI. In an upper half impulse train,
 * both values will be +1. Otherwise these values will be +1 and -1,
 * respectively.
 *
 * @param index     The generator index
 * @param freq      The normalized frequency (frequency / sample rate)
 * @param offset    The phase offset
 * @param upper     Whether to use the upper-half of the signal
 * @param buffer    The buffer to fill
 * @param stride    The bufffer stride
 * @param size      The number of elements to generate
 *
 * @return the last value generated
 */
static double fill_naive_train(Uint64 index, double freq, double offset, SDL_bool upper,
                               float* buffer, size_t stride, size_t size) {
    double t;
    double last;
    float* output = buffer;
    double off = offset/TWO_PI;
    if (upper) {
        if (stride) {
            while (size--) {
                t = SDL_fmod(freq*(index++)+off,1);
                last = (t <= freq/2 || SDL_fabs(t-0.5) <= freq/2) ? 1.0 : 0.0;
                *output = (float)last;
                output += stride;
            }
        } else {
            while (size--) {
                t = SDL_fmod(freq*(index++)+off,1);
                last = (t <= freq/2 || SDL_fabs(t-0.5) <= freq/2) ? 1.0 : 0.0;
                *output++ = (float)last;
            }
        }
    } else {
        if (stride) {
            while (size--) {
                t = SDL_fmod(freq*(index++)+off,1);
                last = (t <= freq/2) ? 1.0 : ((SDL_fabs(t-0.5) <= freq/2) ? -1.0 : 0.0);
                *output = (float)last;
                output += stride;
            }
        } else {
            while (size--) {
                double p = freq*offset;
                t = SDL_fmod(freq*(index++)+off,1);
                last = (t <= freq/2) ? 1.0 : ((SDL_fabs(t-0.5) <= freq/2) ? -1.0 : 0.0);
                *output++ = (float)last;
            }
        }
    }
    return last;
}

/**
 * Returns the next value for a bandlimited triangle wave.
 *
 * This function uses a PolyBLEP curve to create a bandwidth-limited
 * square wave, as reported in "Antialiasing Oscillators in Subtractive
 * Synthesis" by Valimaki and Huovilainen (2007). This wave is then
 * integrated to produce a triangle wave, using the leaky integration in
 * "Alias-Free Digital Synthesis of Classic Analog Waveforms" by Stilson
 * and Smith (1996). This particular version is adapted from
 *
 *     http://www.martin-finke.de/blog/articles/audio-plugins-018-polyblep-oscillator/
 *
 * The value at 0 is 0.
 *
 * An upper half triangle wave is a waveform of the same shape but
 * scaled and shifted to [0,1] instead of [-1,1].
 *
 * @param index     The generator index
 * @param freq      The normalized frequency (frequency / sample rate)
 * @param offset    The phase offset
 * @param prev      The previous value (for integration)
 * @param upper     Whether to use the upper-half of the signal
 *
 * @return the next value for a bandlimited triangle wave.
 */
inline static double tick_poly_triang(Uint64 index, double freq, double offset,
                                      double prev, SDL_bool upper) {
    const double STEP = freq*TWO_PI;
    double t = SDL_fmod(freq*index+offset/TWO_PI,1);
    double value = t <= 0.5 ? 1.0 : -1.0;
    value += poly_blep(t,freq);
    value -= poly_blep(SDL_fmod(t + 0.5, 1.0),freq);
    if (upper) {
        value = 0.5 * (STEP * value + (1 - STEP) * (2.0*prev-1.0) + 1.0);
    } else {
        value = STEP * value + (1 - STEP) * prev;
    }
    return value;
}

/**
 * Fills buffer with the next size values for a bandlimited triangle wave.
 *
 * This function uses a PolyBLEP curve to create a bandwidth-limited
 * square wave, as reported in "Antialiasing Oscillators in Subtractive
 * Synthesis" by Valimaki and Huovilainen (2007). This wave is then
 * integrated to produce a triangle wave, using the leaky integration in
 * "Alias-Free Digital Synthesis of Classic Analog Waveforms" by Stilson
 * and Smith (1996). This particular version is adapted from
 *
 *     http://www.martin-finke.de/blog/articles/audio-plugins-018-polyblep-oscillator/
 *
 * The value at 0 is 0.
 *
 * An upper half triangle wave is a waveform of the same shape but
 * scaled and shifted to [0,1] instead of [-1,1].
 *
 * @param index     The generator index
 * @param freq      The normalized frequency (frequency / sample rate)
 * @param offset    The phase offset
 * @param prev      The previous value (for integration)
 * @param upper     Whether to use the upper-half of the signal
 * @param buffer    The buffer to fill
 * @param stride    The bufffer stride
 * @param size      The number of elements to generate
 *
 * @return the last value generated
 */
static double fill_poly_triang(Uint64 index, double freq, double offset,
                               double prev, SDL_bool upper,
                               float* buffer, size_t stride, size_t size) {
    const double STEP = freq*TWO_PI;

    double value, t;
    double last;
    float* output = buffer;
    double off = offset/TWO_PI;
    if (upper) {
        if (stride) {
            while (size--) {
                t = SDL_fmod(freq*(index++)+off,1);
                value = t <= 0.5 ? 1.0 : -1.0;
                value += poly_blep(t,freq);
                value -= poly_blep(SDL_fmod(t + 0.5, 1.0),freq);
                last = 0.5 * (STEP * value + (1 - STEP) * (2.0*prev-1.0) + 1.0);
                prev = last;
                *output = (float)last;
                output += stride;
            }
        } else {
            while (size--) {
                t = SDL_fmod(freq*(index++)+off,1);
                value = t <= 0.5 ? 1.0 : -1.0;
                value += poly_blep(t,freq);
                value -= poly_blep(SDL_fmod(t + 0.5, 1.0),freq);
                last = 0.5 * (STEP * value + (1 - STEP) * (2.0*prev-1.0) + 1.0);
                prev = last;
                *output++ = (float)last;
            }
        }
    } else {
        if (stride) {
            while (size--) {
                t = SDL_fmod(freq*(index++)+off,1);
                value = t <= 0.5 ? 1.0 : -1.0;
                value += poly_blep(t,freq);
                value -= poly_blep(SDL_fmod(t + 0.5, 1.0),freq);
                last = STEP * value + (1 - STEP) * prev;
                prev = last;
                *output = (float)last;
                output += stride;
            }
        } else {
            while (size--) {
                t = SDL_fmod(freq*(index++)+off,1);
                value = t <= 0.5 ? 1.0 : -1.0;
                value += poly_blep(t,freq);
                value -= poly_blep(SDL_fmod(t + 0.5, 1.0),freq);
                last = STEP * value + (1 - STEP) * prev;
                prev = last;
                *output++ = (float)last;
            }
        }
    }
    return last;
}

/**
 * Returns the next value for a bandlimited square wave.
 *
 * This function uses a PolyBLEP curve as reported in "Antialiasing
 * Oscillators in Subtractive Synthesis" by Valimaki and Huovilainen
 * (2007). This particular version is adapted from
 *
 *     http://www.martin-finke.de/blog/articles/audio-plugins-018-polyblep-oscillator/
 *
 * The value at 0 is 0.
 *
 * An upper half square wave is a waveform of the same shape but
 * scaled and shifted to [0,1] instead of [-1,1].
 *
 * @param index     The generator index
 * @param freq      The normalized frequency (frequency / sample rate)
 * @param offset    The phase offset
 * @param upper     Whether to use the upper-half of the signal
 *
 * @return the next value for a bandlimited square wave.
 */
inline static double tick_poly_square(Uint64 index, double freq, double offset, SDL_bool upper) {
    double t = SDL_fmod(freq*index+offset/TWO_PI,1);
    double value = t <= 0.5 ? 1.0 : -1.0;
    value += poly_blep(t,freq);
    value -= poly_blep(SDL_fmod(t + 0.5, 1.0),freq);
    return upper ? 0.5*(value+1) : value;
}

/**
 * Fills buffer with the next size values for a bandlimited square wave.
 *
 * This function uses a PolyBLEP curve as reported in "Antialiasing
 * Oscillators in Subtractive Synthesis" by Valimaki and Huovilainen
 * (2007). This particular version is adapted from
 *
 *     http://www.martin-finke.de/blog/articles/audio-plugins-018-polyblep-oscillator/
 *
 * An upper half square wave is a waveform of the same shape but
 * scaled and shifted to [0,1] instead of [-1,1].
 *
 * @param index     The generator index
 * @param freq      The normalized frequency (frequency / sample rate)
 * @param offset    The phase offset
 * @param upper     Whether to use the upper-half of the signal
 * @param buffer    The buffer to fill
 * @param stride    The bufffer stride
 * @param size      The number of elements to generate
 *
 * @return the last value generated
 */
static double fill_poly_square(Uint64 index, double freq, double offset, SDL_bool upper,
                               float* buffer, size_t stride, size_t size) {
    double value, t;
    double last;
    float* output = buffer;
    double off = offset/TWO_PI;
    if (upper) {
        if (stride) {
            while (size--) {
                t = SDL_fmod(freq*(index++)+off,1);
                value = t <= 0.5 ? 1.0 : -1.0;
                value += poly_blep(t,freq);
                value -= poly_blep(SDL_fmod(t + 0.5, 1.0),freq);
                last = 0.5*(value+1);
                *output = (float)last;
                output += stride;
            }
        } else {
            while (size--) {
                t = SDL_fmod(freq*(index++)+off,1);
                value = t <= 0.5 ? 1.0 : -1.0;
                value += poly_blep(t,freq);
                value -= poly_blep(SDL_fmod(t + 0.5, 1.0),freq);
                last = 0.5*(value+1);
                *output++ = (float)last;
            }
        }
    } else {
        if (stride) {
            while (size--) {
                t = SDL_fmod(freq*(index++)+off,1);
                value = t <= 0.5 ? 1.0 : -1.0;
                value += poly_blep(t,freq);
                last = value-poly_blep(SDL_fmod(t + 0.5, 1.0),freq);
                *output = (float)last;
                output += stride;
            }
        } else {
            while (size--) {
                t = SDL_fmod(freq*(index++)+off,1);
                value = t <= 0.5 ? 1.0 : -1.0;
                value += poly_blep(t,freq);
                last = value-poly_blep(SDL_fmod(t + 0.5, 1.0),freq);
                *output++ = (float)last;
            }
        }
    }
    return last;
}

/**
 * Returns the next value for a bandlimited sawtooth wave.
 *
 * This function uses a PolyBLEP curve as reported in "Antialiasing
 * Oscillators in Subtractive Synthesis" by Valimaki and Huovilainen
 * (2007). This particular version is adapted from
 *
 *     http://www.martin-finke.de/blog/articles/audio-plugins-018-polyblep-oscillator/
 *
 * An upper half sawtooth wave is a waveform of the same shape but
 * scaled and shifted to [0,1] instead of [-1,1].
 *
 * @param index     The generator index
 * @param freq      The normalized frequency (frequency / sample rate)
 * @param offset    The phase offset
 * @param upper     Whether to use the upper-half of the signal
 *
 * @return the next value for a bandlimited sawtooth wave.
 */
inline static double tick_poly_tooth(Uint64 index, double freq, double offset, SDL_bool upper) {
    double t = SDL_fmod(freq*index+offset/TWO_PI,1);
    double value = 2.0 * t - 1.0;
    value -= poly_blep(t,freq);
    return (upper ? 0.5*(value+1) : value);
}

/**
 * Fills buffer with the next size values for a bandlimited sawtooth wave.
 *
 * This function uses a PolyBLEP curve as reported in "Antialiasing
 * Oscillators in Subtractive Synthesis" by Valimaki and Huovilainen
 * (2007). This particular version is adapted from
 *
 *     http://www.martin-finke.de/blog/articles/audio-plugins-018-polyblep-oscillator/
 *
 * An upper half sawtooth wave is a waveform of the same shape but
 * scaled and shifted to [0,1] instead of [-1,1].
 *
 * @param index     The generator index
 * @param freq      The normalized frequency (frequency / sample rate)
 * @param offset    The phase offset
 * @param upper     Whether to use the upper-half of the signal
 * @param buffer    The buffer to fill
 * @param stride    The bufffer stride
 * @param size      The number of elements to generate
 *
 * @return the last value generated
 */
static double fill_poly_tooth(Uint64 index, double freq, double offset, SDL_bool upper,
                              float* buffer, size_t stride, size_t size) {
    double value, t;
    double last;
    float* output = buffer;
    double off = offset/TWO_PI;
    if (upper) {
        if (stride) {
            while (size--) {
                t = SDL_fmod(freq*(index++)+off,1);
                value = 2.0 * t - 1.0;
                value -= poly_blep(t,freq);
                last = 0.5*(value+1);
                *output = (float)last;
                output += stride;
            }
        } else {
            while (size--) {
                t = SDL_fmod(freq*(index++)+off,1);
                value = 2.0 * t - 1.0;
                value -= poly_blep(t,freq);
                last = 0.5*value+0.5;
                *output++ = (float)last;
            }
        }
    } else {
        if (stride) {
            while (size--) {
                t = SDL_fmod(freq*(index++)+off,1);
                value = 2.0 * t - 1.0;
                last = value-poly_blep(t,freq);
                *output = (float)last;
                output += stride;
            }
        } else {
            while (size--) {
                t = SDL_fmod(freq*(index++)+off,1);
                value = 2.0 * t - 1.0;
                last = value-poly_blep(t,freq);
                *output++ = (float)last;
            }
        }
    }
    return last;
}

/**
 * Returns the next value for a bandlimited impulse train.
 *
 * This function uses the closed-form algorithm "Alias-Free Digital
 * Synthesis of Classic Analog Waveforms" by Stilson and Smith (1996).
 * This implementation assumes the maximum number of harmonics, and is
 * based on code by Robin Davies, Gary Scavone, 2005-2006.
 *
 * Note that the classic algorithm is for creating an "upper half" impulse
 * train, with impulses at +1 at phases 0 and PI. However, that algorithm
 * has oscillations around 0, and can still produce negative values. For
 * the regular (alternating) impulse train, we take two half rate impulse
 * trains (so one impulse per period) and subtract them with the second
 * phase shifted by PI. The result produces impulses +1 at phase 0 and -1
 * at phase PI.
 *
 * @param index     The generator index
 * @param freq      The normalized frequency (frequency / sample rate)
 * @param offset    The phase offset
 * @param upper     Whether to use the upper-half of the signal
 *
 * @return the next value for a bandlimited impulse train.
 */
inline static double tick_blit_train(Uint64 index, double freq, double offset, SDL_bool upper) {
    Uint64 mval = 2*(Uint64)SDL_floor(0.5/freq)+1;
    double step = freq*TWO_PI;
    if (upper) {
        double phase = SDL_fmod(step*index+offset,TWO_PI);
        double denom = sin(phase);
        double result = (SDL_fabs(denom) <= EPSILON_LIMIT) ? 1.0 : sin(mval*phase)/(mval*denom);
        if (result > 1) { result = 1.0f; }
        else if (result < -1.0f) { result = -1.0f; }
        return result;
    } else {
        double phase, denom;
        double value1, value2, result;
        phase = SDL_fmod((step*index+offset)/2,TWO_PI);
        denom = sin(phase);
        value1 = (SDL_fabs(denom) <= EPSILON_LIMIT) ? 1.0 : sin(mval*phase)/(mval*denom);

        phase = SDL_fmod((step*index+offset+M_PI)/2,TWO_PI);
        denom = sin(phase);
        value2 = (SDL_fabs(denom) <= EPSILON_LIMIT) ? 1.0 : sin(mval*phase)/(mval*denom);

        result = value1-value2;
        if (result > 1) { result = 1.0f; }
        else if (result < -1.0f) { result = -1.0f; }
        return result;
    }
}

/**
 * Fills buffer with the next size values for a bandlimited impulse train.
 *
 * This function uses the closed-form algorithm "Alias-Free Digital
 * Synthesis of Classic Analog Waveforms" by Stilson and Smith (1996).
 * This implementation assumes the maximum number of harmonics, and is
 * based on code by Robin Davies, Gary Scavone, 2005-2006.
 *
 * Note that the classic algorithm is for creating an "upper half" impulse
 * train, with impulses at +1 at phases 0 and PI. However, that algorithm
 * has oscillations around 0, and can still produce negative values. For
 * the regular (alternating) impulse train, we take two half rate impulse
 * trains (so one impulse per period) and subtract them with the second
 * phase shifted by PI. The result produces impulses +1 at phase 0 and -1
 * at phase PI.
 *
 * @param index     The generator index
 * @param freq      The normalized frequency (frequency / sample rate)
 * @param offset    The phase offset
 * @param upper     Whether to use the upper-half of the signal
 * @param buffer    The buffer to fill
 * @param stride    The bufffer stride
 * @param size      The number of elements to generate
 *
 * @return the last value generated
 */
static double fill_blit_train(Uint64 index, double freq, double offset, SDL_bool upper,
                              float* buffer, size_t stride, size_t size) {
    Uint64 mval = 2*(Uint64)SDL_floor(0.5/freq)+1;
    double step = freq*TWO_PI;
    double phase, denom;
    double value1, value2;
    double last;
    float* output = buffer;
    if (upper) {
        if (stride) {
            while (size--) {
                phase = SDL_fmod(step*(index++)+offset,TWO_PI);
                denom = sin(phase);
                last = (SDL_fabs(denom) <= EPSILON_LIMIT) ? 1.0 : sin(mval*phase)/(mval*denom);
                if (last > 1) { last = 1.0f; }
                else if (last < -1.0f) { last = -1.0f; }
                *output = (float)last;
                output += stride;
            }
        } else {
            while (size--) {
                phase = SDL_fmod(step*(index++)+offset,TWO_PI);
                denom = sin(phase);
                last = (SDL_fabs(denom) <= EPSILON_LIMIT) ? 1.0 : sin(mval*phase)/(mval*denom);
                if (last > 1) { last = 1.0f; }
                else if (last < -1.0f) { last = -1.0f; }
                *output++ = (float)last;
            }
        }
    } else {
        if (stride) {
            while (size--) {
                phase = SDL_fmod((step*index+offset)/2,TWO_PI);
                denom = sin(phase);
                value1 = (SDL_fabs(denom) <= EPSILON_LIMIT) ? 1.0 : sin(mval*phase)/(mval*denom);

                phase = SDL_fmod((step*(index++)+offset+M_PI)/2,TWO_PI);
                denom = sin(phase);
                value2 = (SDL_fabs(denom) <= EPSILON_LIMIT) ? 1.0 : sin(mval*phase)/(mval*denom);

                last = value1-value2;
                if (last > 1) { last = 1.0f; }
                else if (last < -1.0f) { last = -1.0f; }
                *output = (float)last;
                output += stride;
            }
        } else {
            while (size--) {
                phase = SDL_fmod((step*index+offset)/2,TWO_PI);
                denom = sin(phase);
                value1 = (SDL_fabs(denom) <= EPSILON_LIMIT) ? 1.0 : sin(mval*phase)/(mval*denom);

                phase = SDL_fmod((step*(index++)+offset+M_PI)/2,TWO_PI);
                denom = sin(phase);
                value2 = (SDL_fabs(denom) <= EPSILON_LIMIT) ? 1.0 : sin(mval*phase)/(mval*denom);

                last = value1-value2;
                if (last > 1) { last = 1.0f; }
                else if (last < -1.0f) { last = -1.0f; }
                *output++ = (float)last;
            }
        }
    }
    return last;
}

#pragma mark -
#pragma mark WaveForm Generators
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
typedef struct ATK_WaveformGen {
    /** The waveform shape */
    ATK_WaveformShape shape;
    /** The current position to generate (overflow should be non-issue) */
    Uint64 sample;
    /** The normalized frequency */
    double freq;
    /** The initial phase */
    double phase;
    /** The last sample created (for integration purposes). */
    double last;
    /** Whether to limit the waveform to the positive y-axis. */
    SDL_bool upper;
} ATK_WaveformGen;

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
 * @param freq  The normalized frequency
 * @param phase The initial phase [0,2PI)
 *
 * @return a newly allocated waveform generator for the given shape and frequency
 */
ATK_WaveformGen* ATK_AllocWaveform(ATK_WaveformShape shape, double freq, double phase) {
    ATK_WaveformGen* result = (ATK_WaveformGen*)ATK_malloc(sizeof(ATK_WaveformGen));
    result->shape = shape;
    result->freq  = freq;
    result->phase = phase;

    result->sample = 0;
    result->last   = 0;
    result->upper  = SDL_FALSE;
    return result;
}

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
 * @param freq  The normalized frequency.
 * @param phase The initial phase [0,2PI)
 *
 * @return a newly allocated waveform generator for the given shape and frequency
 */
ATK_WaveformGen* ATK_AllocUpperWaveform(ATK_WaveformShape shape, double freq, double phase) {
    ATK_WaveformGen* result = (ATK_WaveformGen*)ATK_malloc(sizeof(ATK_WaveformGen));
    result->shape = shape;
    result->freq  = freq;
    if (phase > TWO_PI) {
        result->phase = SDL_fmod(phase,TWO_PI);
    } else if (phase < 0) {
        result->phase = TWO_PI+SDL_fmod(phase,TWO_PI);
    } else {
        result->phase = phase;
    }

    result->sample = 0;
    result->last   = 0.5;
    result->upper  = SDL_TRUE;
    return result;
}

/**
 * Frees a previously allocated waveform generator
 *
 * @param gen   The waveform generator
 */
void ATK_FreeWaveform(ATK_WaveformGen* gen) {
    ATK_free(gen);
}

/**
 * Resets the waveform generator to its initial state
 *
 * The generator will be in the state it was when first created.  When a generator
 * is used, the first sample is guaranteed to be 0.
 *
 * @param gen   The waveform generator
 */
void ATK_ResetWaveform(ATK_WaveformGen* gen) {
    gen->sample = 0;
    gen->last = gen->upper ? 0.5 : 0.0;
}

/**
 * Returns the next sample created by this waveform generator
 *
 * Waveform generators are stateful, in that a loop over this function is
 * identical to {@link ATK_WaveformFill}. However, this also means that
 * the same generator should not be used on multiple channels in
 * multichannel audio.
 *
 * @param gen       The waveform generator
 *
 * @return the next sample created by this waveform generator
 */
float ATK_StepWaveform(ATK_WaveformGen* gen) {
    if (gen == NULL) {
        return -1;
    }

    // Number of harmonics for BLIT impulse
    Uint64 pos  = gen->sample++;
    double last = gen->last;

    switch (gen->shape) {
        case ATK_SHAPE_SINE:
            last = tick_naive_sine(pos, gen->freq, gen->phase, gen->upper);
            break;
        case ATK_SHAPE_NAIVE_TRIANG:
            last = tick_naive_triang(pos, gen->freq, gen->phase, gen->upper);
            break;
        case ATK_SHAPE_NAIVE_SQUARE:
            last = tick_naive_square(pos, gen->freq, gen->phase, gen->upper);
            break;
        case ATK_SHAPE_NAIVE_TOOTH:
            last = tick_naive_tooth(pos, gen->freq, gen->phase, gen->upper);
            break;
        case ATK_SHAPE_NAIVE_TRAIN:
            last = tick_naive_train(pos, gen->freq, gen->phase, gen->upper);
            break;
        case ATK_SHAPE_POLY_TRIANG:
            last = tick_poly_triang(pos, gen->freq, gen->phase, last, gen->upper);
            break;
        case ATK_SHAPE_POLY_SQUARE:
            last = tick_poly_square(pos, gen->freq, gen->phase, gen->upper);
            break;
        case ATK_SHAPE_POLY_TOOTH:
            last = tick_poly_tooth(pos, gen->freq, gen->phase, gen->upper);
            break;
        case ATK_SHAPE_BLIT_TRAIN:
            last = tick_blit_train(pos, gen->freq, gen->phase, gen->upper);
            break;
        default:
            return 0;
    }
    gen->last = last;
    return (float)last;
}

/**
 * Fills the buffer using data from the waveform generator
 *
 * Waveform generators are stateful, in that a single call to this function is
 * identical to two calls to the function on the two halves of the buffer.
 * However, this also means that the same generator should not be used on
 * multiple channels in multichannel audio.
 *
 * @param gen       The waveform generator
 * @param buffer    The buffer to fill
 * @param size      The size of the buffer
 *
 * @return 0 if data was successfully generated; -1 if an error.
 */
Sint32 ATK_FillWaveform(ATK_WaveformGen* gen, float* buffer, size_t size) {
    return ATK_FillWaveform_stride(gen,buffer,0,size);
}

/**
 * Fills the stride-aware buffer using data from the waveform generator
 *
 * The buffer will only be filled at every stride entries. This is useful for
 * embedding a waveform into a channel for multichannel audio.
 *
 * Waveform generators are stateful, in that a single call to this function is
 * identical to two calls to the function on the two halves of the buffer.
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
Sint32 ATK_FillWaveform_stride(ATK_WaveformGen* gen, float* buffer, size_t stride, size_t size) {
    if (gen == NULL) {
        return -1;
    }

    // Number of harmonics for BLIT impulse
    Uint64 pos  = gen->sample;
    double last = gen->last;
    float* output = buffer;

    switch (gen->shape) {
        case ATK_SHAPE_SINE:
            last = fill_naive_sine(pos, gen->freq, gen->phase, gen->upper, buffer, stride, size);
            break;
        case ATK_SHAPE_NAIVE_TRIANG:
            last = fill_naive_triang(pos, gen->freq, gen->phase, gen->upper, buffer, stride, size);
            break;
        case ATK_SHAPE_NAIVE_SQUARE:
            last = fill_naive_square(pos, gen->freq, gen->phase, gen->upper, buffer, stride, size);
            break;
        case ATK_SHAPE_NAIVE_TOOTH:
            last = fill_naive_tooth(pos, gen->freq, gen->phase, gen->upper, buffer, stride, size);
            break;
        case ATK_SHAPE_NAIVE_TRAIN:
            last = fill_naive_train(pos, gen->freq, gen->phase, gen->upper, buffer, stride, size);
            break;
        case ATK_SHAPE_POLY_TRIANG:
            last = fill_poly_triang(pos, gen->freq, gen->phase, last, gen->upper, buffer, stride, size);
            break;
        case ATK_SHAPE_POLY_SQUARE:
            last = fill_poly_square(pos, gen->freq, gen->phase, gen->upper, buffer, stride, size);
            break;
        case ATK_SHAPE_POLY_TOOTH:
            last = fill_poly_tooth(pos, gen->freq, gen->phase, gen->upper, buffer, stride, size);
            break;
        case ATK_SHAPE_BLIT_TRAIN:
            last = fill_blit_train(pos, gen->freq, gen->phase, gen->upper, buffer, stride, size);
            break;
        default:
            return -1;
    }
    gen->last = last;
    gen->sample += size;
    return 0;
}
