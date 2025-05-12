/*
 * SDL_atk:  An audio toolkit library for use with SDL
 * Copyright (C) 2022-2023 Walker M. White
 *
 * This is a library to load different types of audio files as PCM data,
 * and process them with basic DSP tools. The goal of this library is to
 * provide an alternative to SDL_sound that supports efficient streaming
 * and file output. In addition, it provides a minimal math library akin
 * to (and inspired by) numpy for audio processing. This enables the
 * developer to add custom audio effects that are not possible in SDL_mixer.
 *
 * Normally, SDL extensions (SDL_image, SDL_ttf, SDL_mixer) have one
 * self-contained header file. However, there are easily identiable
 * components to this library and it is a lot easier to add features if
 * we keep them separate. So each component has its own header, and this
 * header simply includes them all.
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
 * @file SDL_atk.h
 *
 * Header file for SDL_atk library
 *
 * This is library to load different types of audio files as PCM data, and
 * process them with basic DSP tools. The goal of this library is to
 * provide an alternative to SDL_sound that supports efficient streaming
 * and file output. In addition, it provides a minimal math library akin
 * to (and inspired by) numpy for audio processing. This enables the
 * developer to add custom audio effects that are not possible in SDL_mixer.
 */
#ifndef __SDL_ATK_H__
#define __SDL_ATK_H__
#include <SDL.h>
#include <SDL_version.h>

#include "ATK_error.h"
#include "ATK_file.h"
#include "ATK_rand.h"
#include "ATK_math.h"
#include "ATK_codec.h"
#include "ATK_dsp.h"
#include "ATK_audio.h"

#endif /* __SDL_ATK_H__ */
