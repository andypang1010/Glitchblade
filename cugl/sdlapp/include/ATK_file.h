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
 * @file ATK_file.h
 *
 * Header file for the file manager of SDL_atk
 *
 * This component provides support for a managed pool of files.  Managed files
 * support the SDL_RWops abstraction, but may or may not hold an active file
 * handle. If a managed file has been inactive long enough, it may be closed to
 * recover the file handle.  When the user access the SDL_RWops again, the file
 * will be reopened and returned to the position it was when closed. All of this
 * opening and closing of the file happens in the background and is hidden from
 * the user.
 *
 * It might seem weird that we need an alternate to the standard SDL_RWops options
 * just to support audio. But this library is a reaction to bugs discovered by my
 * students who aggressively worked with streaming (as opposed to buffered) audio.
 * Apple devices (and many POSIX systems) limit the number of open files to a
 * maximum of 20 per process. As the purpose of streaming audio is to read directly
 * from a file and not from memory, it is very easy to hit this limit when doing
 * vertical layering or sophisticated sequencing.
 */
#ifndef __ATK_FILE_H__
#define __ATK_FILE_H__
#include <SDL.h>
#include <SDL_version.h>
#include <begin_code.h>

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

#pragma mark -
#pragma mark RWops Functions
/**
 * An alias of SDL_RWops to give us an informal form of type safety
 *
 * As an instance of SDL_RWOPS_UNKNOWN, our SDL_RWops objects will make use of
 * the attributes data1 and data2 in the hidden attribute. The first will be
 * used to manage the file state. The second will be a reference to the file
 * SDL_RWops, if it is currently open.
 */
typedef struct SDL_RWops ATK_RWops;

/**
 * Close and free an allocated ATK_RWops structure.
 *
 * @param context   ATK_RWops structure to close
 *
 * @return 0 on success or a negative error code on failure
 */
#define ATK_RWclose     SDL_RWclose

/**
 * Read from a data source.
 *
 * @param context   a pointer to an ATK_RWops structure
 * @param ptr       a pointer to a buffer to read data into
 * @param size      the size of each object to read, in bytes
 * @param maxnum    the maximum number of objects to be read
 *
 * @return the number of objects read, or 0 at error or end of file
 */
#define ATK_RWread      SDL_RWread

/**
 * Seek within an ATK_RWops data stream.
 *
 * @param context   a pointer to an ATK_RWops structure
 * @param offset    an offset in bytes, relative to whence location; can be negative
 * @param whence    any of RW_SEEK_SET, RW_SEEK_CUR, RW_SEEK_END
 *
 * @return the final offset in the data stream after the seek or -1 on error.
 */
#define ATK_RWseek      SDL_RWseek

/**
 * Determine the current read/write offset in an ATK_RWops data stream.
 *
 * @param context   an ATK_RWops data stream object from which to get the current offset
 *
 * @return the current offset in the stream, or -1 if the information can not be determined.
 */
#define ATK_RWtell      SDL_RWtell

/**
 * Write to an ATK_RWops data stream.
 *
 * @param context   a pointer to an ATK_RWops structure
 * @param ptr       a pointer to a buffer containing data to write
 * @param size      the size of an object to write, in bytes
 * @param num       the number of objects to write
 *
  * @return the number of objects written, which will be less than num on error;
 */
#define ATK_RWwrite     SDL_RWwrite

#pragma mark -
#pragma mark Managed RWops
/**
 * The opaque type for a managed file pool
 *
 * Managed files are associated with a file pool. A file pool is a collection of
 * managed files, and which only allows a small number of files to be active
 * (i.e. in memory) at a time. If a file needs to be reactivated, and the number
 * of active files is at capacity, the file pool will first page out one of its
 * active members to make room.
 *
 * Deleting a file pool immediately disposes of all of its managed files.
 */
typedef struct ATK_FilePool ATK_FilePool;

/**
 * Initializes the managed file subsystem for ATK
 *
 * All of the ATK codec functions have the option to use a default managed
 * file subsystem, guaranteeing that we will never run out of file handles
 * as long as there is suitable memory. Calling this function will initialize
 * this subsystem.
 *
 * If this function is not called, all ATK codec functions will use the OS
 * for accessing files. This means that it is possible for a codec function
 * to fail if there are too many open files at once.
 *
 * @param capacity  The maximum number of active files in the pool
 *
 * @return 0 on success; -1 on failure
 */
extern DECLSPEC int SDLCALL ATK_init(Uint32 capacity);

/**
 * Shuts down the managed file subsystem for ATK
 *
 * Any files associated with the managed file subsystem will be immediately
 * closed and disposed. This function does nothing if the subsystem was not
 * initialized.
 */
extern DECLSPEC void SDLCALL ATK_quit();

/**
 * Returns the default managed file subsystem for ATK
 *
 * This is the managed file pool used by all ATK codec functions. If it is
 * null, those functions will all use the OS for accessing files instead.
 *
 * @return the default managed file subsystem for ATK
 */
extern DECLSPEC ATK_FilePool* SDLCALL ATK_DefaultFilePool();

/**
 * Returns a newly allocated file pool
 *
 * The file pool will only allow capacity many files to be active at once.
 * Note that this capacity is subject to the same file handle limits as
 * everything else. In particular, if the capacity exceeds the limit of
 * the number of simultaneously open files, it can still fail to open
 * files when there is too much demand. In addition, if there are multiple
 * file pools, their capacity should not sum more than the open file limit.
 *
 * @param capacity  The maximum number of active files in the pool
 *
 * @return a newly allocated file pool (or NULL on failure)
 */
extern DECLSPEC ATK_FilePool* SDLCALL ATK_AllocFilePool(Uint32 capacity);

/**
 * Frees a previously allocated file pool
 *
 * Any files associated with the managed file subsystem will be immediately
 * closed and disposed.
 *
 * @param pool  The file pool to dispose
 */
extern DECLSPEC void SDLCALL ATK_FreeFilePool(ATK_FilePool* pool);

/**
 * Returns a newly opened ATK_RWops from a named file
 *
 * This function is, for all intents and purposes, equivalent to SDL_RWFromFile.
 * It supports exactly the same file modes. The only difference is that the
 * file is associated with the given file pool.
 *
 * @param file  a UTF-8 string representing the filename to open
 * @param mode  an ASCII string representing the mode to be used for opening the file
 * @param pool  the associated file pool
 *
 * @return a newly opened ATK_RWops from a named file (or NULL on failure)
 */
extern DECLSPEC ATK_RWops* SDLCALL ATK_RWFromFilePool(const char *file, const char *mode, ATK_FilePool* pool);

/**
 * Returns SDL_TRUE if context is managed by pool; SDL_FALSE otherwise
 *
 * As both ATK_RWops and ATK_FilePool are somewhat opaque types, we provide this
 * function to check if a file is managed by a particular pool
 *
 * @param context   the ATK_RWops to check
 * @param pool      the file pool to query from
 *
 * @return SDL_TRUE if context is managed by pool; SDL_FALSE otherwise
 */
extern DECLSPEC SDL_bool SDLCALL ATK_RWInFilePool(ATK_RWops* context, ATK_FilePool* pool);


/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif /* __ATK_FILE_H__ */

