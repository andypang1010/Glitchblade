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
 * @file ATK_error.h
 *
 * Header file for error management in SDL_atk
 *
 * For now, we just defer to SDL error handling, aliasing the functions as is
 * standard in the other SDL extension libraries. However, we keep this header
 * to allow us more error reporting in the future.
 */
#ifndef __ATK_ERROR_H__
#define __ATK_ERROR_H__
/**
 * Report SDL_codec errors
 *
 * @sa ATK_GetError
 */
#define ATK_SetError    SDL_SetError

/**
 * Get last SDL_codec error
 *
 * @sa ATK_SetError
 */
#define ATK_GetError    SDL_GetError

/**
 * Clear last SDL_mixer error
 *
 * @sa ATK_SetError
 */
#define ATK_ClearError  SDL_ClearError

/**
 * Set OutOfMemory error
 */
#define ATK_OutOfMemory SDL_OutOfMemory

#ifdef USE_SDL_MALLOC
/**
 * Allocate a block of memory
 */
#define ATK_malloc  SDL_malloc

/**
 * Reallocate a block of memory
 */
#define ATK_realloc  SDL_realloc

/**
 * Free a block of memory
 */
#define ATK_free    SDL_free

#else
#include <stdlib.h>
/**
 * Allocate a block of memory
 */
#define ATK_malloc  malloc

/**
 * Reallocate a block of memory
 */
#define ATK_realloc  realloc

/**
 * Free a block of memory
 */
#define ATK_free    free

#endif

#endif /* __ATK_ERROR_H__ */
