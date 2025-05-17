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
#include <ATK_file.h>
#include <ATK_error.h>
#include <kiss_fft.h>
#include "ATK_file_c.h"

#pragma mark Managed Files
/**
 * Loads an inactive ATK_RWops into memory
 *
 * This function assumes that there is capacity in the associated file pool to
 * load in the file. See {@link ATK_RWactivate} for a version that will make
 * room in the file pool if necessary
 *
 * This function does nothing if the ATK_RWops is already active.
 *
 * @pool context the ATK_RWops to load into memory
 *
 * @return 0 on success, -1 on failure
 */
static int ATK_RWLoad(ATK_RWops* context) {
    ATK_RWstate* state = (ATK_RWstate*)context->hidden.unknown.data1;
    if (state->active) {
        // Nothing to do
        return 0;
    }
    
    if (context->hidden.unknown.data2 != NULL) {
        ATK_SetError("Managed file was not paged out properly");
        return -1;
    }
    
    char mode[4];
    memset(mode, 0, 4);
    mode[0] = 'r';
    int next = 1;
    if (state->mode & ATK_RW_MODE_WRITE) {
        mode[next++] = '+';
    } else if (state->mode & ATK_RW_MODE_APPEND) {
        mode[0] = 'a';
        mode[1] = '+';
        next = 2;
    }
    if (state->mode & ATK_RW_MODE_BINARY) {
        mode[next] = 'b';
    }
    
    SDL_RWops* source = SDL_RWFromFile(state->name,mode);
    if (SDL_RWseek(source, state->pos, RW_SEEK_SET) < 0) {
        SDL_RWclose(context);
        return -1;
    }

    context->hidden.unknown.data2 = source;
    state->active = SDL_TRUE;
    state->touch  = SDL_TRUE;
    state->pool->active++;
    return 0;
}

/**
 * Deactivates a file in the file pool, making room for another
 *
 * This function uses a classic LRU clock algorithm to find the
 * next file to deactivate. It fails if there are no active files.
 *
 * @param pool  the file pool to evict from
 *
 * @return 0 on success, -1 on failure
 */
static int ATK_PageOutFile(ATK_FilePool* pool) {
    if (pool->head == NULL) {
        ATK_SetError("Attempt to page out an empty file pool");
        return -1;
    }
    if (pool->evict == NULL) {
        pool->evict = pool->head;
    }
    
    // Classic clock-style LRU algorithm
    ATK_RWstate* state = (ATK_RWstate*)pool->evict->file->hidden.unknown.data1;
    int inactives = 0;
    // Allow us to loop through twice to apply the clock
    while ((!state->active || state->touch) && inactives < 2*pool->total) {
        if (!state->active) {
            inactives++;
        }
        state->touch = SDL_FALSE;
        pool->evict = pool->evict->next;
        state = (ATK_RWstate*)pool->evict->file->hidden.unknown.data1;
    }
    ATK_RWops* curr = pool->evict->file;
    pool->evict = pool->evict->next;
    return state->active ? ATK_RWdeactivate(curr) : -1;
}

/**
 * Internal implementation of SDL_RWsize
 *
 * This function returns the total size of the file in bytes.
 *
 * @param context a pointer to an SDL_RWops structure
 *
 * @return the total size of the file in bytes or -1 on error
 */
static Sint64 ATK_RWsizeImp(SDL_RWops* context) {
    if (context == NULL) {
        ATK_SetError("Attempted to query a null context");
        return -1;
    } else if (context->type != SDL_RWOPS_UNKNOWN) {
        ATK_SetError("Attempted to query an unmanaged file");
        return -1;
    }
    
    Uint8* check = (Uint8*)context->hidden.unknown.data1;
    if (*check != ATK_FILE_CHECKSUM) {
        ATK_SetError("Attempted to query an unmanaged file");
        return -1;
    }
    
    ATK_RWactivate(context);
    SDL_RWops* source = (SDL_RWops*)context->hidden.unknown.data2;
    return source->size(source);
}

/**
 * Internal implementation of SDL_RWseek
 *
 * This function seeks within an SDL_RWops file.
 *
 * @param context   a pointer to an SDL_RWops structure
 * @param offset    an offset in bytes, relative to whence location; can be negative
 * @param whence    any of RW_SEEK_SET, RW_SEEK_CUR, RW_SEEK_END
 *
 * @return the final offset in the file after the seek or -1 on error
 */
static Sint64 ATK_RWseekImp(SDL_RWops* context, Sint64 offset, int whence) {
    if (context == NULL) {
        ATK_SetError("Attempted to seek a null context");
        return -1;
    } else if (context->type != SDL_RWOPS_UNKNOWN) {
        ATK_SetError("Attempted to seek an unmanaged file");
        return -1;
    }
    
    Uint8* check = (Uint8*)context->hidden.unknown.data1;
    if (*check != ATK_FILE_CHECKSUM) {
        ATK_SetError("Attempted to seek an unmanaged file");
        return -1;
    }
    
    ATK_RWactivate(context);
    SDL_RWops* source = (SDL_RWops*)context->hidden.unknown.data2;
    return source->seek(source,offset,whence);
}

/**
 * Internal implementation of SDL_RWread
 *
 * This function reads from an SDL_RWops file.
 *
 * @param context   a pointer to an SDL_RWops structure
 * @param ptr       a pointer to a buffer to read data into
 * @param size      the size of each object to read, in bytes
 * @param maxnum    the maximum number of objects to be read
 *
 * @return the number of objects read, or 0 at error or end of file
 */
static size_t ATK_RWreadImp(SDL_RWops *context, void *ptr, size_t size, size_t maxnum) {
    if (context == NULL) {
        ATK_SetError("Attempted to read a null context");
        return 0;
    } else if (context->type != SDL_RWOPS_UNKNOWN) {
        ATK_SetError("Attempted to read an unmanaged file");
        return 0;
    }
    
    Uint8* check = (Uint8*)context->hidden.unknown.data1;
    if (*check != ATK_FILE_CHECKSUM) {
        ATK_SetError("Attempted to read an unmanaged file");
        return 0;
    }
    
    ATK_RWactivate(context);
    SDL_RWops* source = (SDL_RWops*)context->hidden.unknown.data2;
    return source->read(source,ptr,size,maxnum);
}

/**
 * Internal implementation of SDL_RWwrite
 *
 * This function writes to an SDL_RWops file.
 *
 * @param context   a pointer to an SDL_RWops structure
 * @param ptr       a pointer to a buffer containing data to write
 * @param size      the size of each object to write, in bytes
 * @param num       the number of objects to write
 *
 * @return the number of objects written, which is less than num on error
 */
static size_t ATK_RWwriteImp(SDL_RWops *context, const void *ptr, size_t size, size_t num) {
    if (context == NULL) {
        ATK_SetError("Attempted to read a null context");
        return 0;
    } else if (context->type != SDL_RWOPS_UNKNOWN) {
        ATK_SetError("Attempted to read an unmanaged file");
        return 0;
    }
    
    Uint8* check = (Uint8*)context->hidden.unknown.data1;
    if (*check != ATK_FILE_CHECKSUM) {
        ATK_SetError("Attempted to read an unmanaged file");
        return 0;
    }
    
    ATK_RWactivate(context);
    SDL_RWops* source = (SDL_RWops*)context->hidden.unknown.data2;
    return source->write(source,ptr,size,num);
}

/**
 * Internal implementation of SDL_RWclose
 *
 * This function closes and frees an allocated SDL_RWops file.
 *
 * @param context   a pointer to an SDL_RWops structure to close
 *
 * @return 0 on success, negative error code on failure
 */
static int ATK_RWcloseImp(SDL_RWops *context) {
    if (context == NULL) {
        ATK_SetError("Attempted to close a null context");
        return -1;
    } else if (context->type != SDL_RWOPS_UNKNOWN) {
        ATK_SetError("Attempted to close an unmanaged file");
        return -1;
    }
    
    Uint8* check = (Uint8*)context->hidden.unknown.data1;
    if (*check != ATK_FILE_CHECKSUM) {
        ATK_SetError("Attempted to close an unmanaged file");
        return -1;
    }
    
    ATK_RWstate* state = (ATK_RWstate*)context->hidden.unknown.data1;
    ATK_FilePool* pool = state->pool;
    SDL_LockMutex(pool->mutex);
    if (state->active) {
        pool->total--;
        pool->active--;
    }
    
    SDL_RWops* source = (SDL_RWops*)context->hidden.unknown.data2;
    if (source != NULL) {
        int val = SDL_RWclose(source);
        if (val < 0) {
            SDL_UnlockMutex(pool->mutex);
            return val;
        }
    }
    context->hidden.unknown.data2 = NULL;
    
    ATK_FileNode* node = state->node;
    if (node->next != node) {
        node->prev->next = node->next;
        node->next->prev = node->prev;
        if (node == pool->head) {
            pool->head = node->next;
        }
        if (node == pool->tail) {
            pool->tail = node->prev;
        }
        if (node == pool->evict) {
            pool->evict = NULL;
        }
    } else {
        pool->head = NULL;
        pool->tail = NULL;
        pool->evict = NULL;
    }
    SDL_UnlockMutex(pool->mutex);

    // Time to delete the state
    node->prev = NULL;
    node->next = NULL;
    node->file = NULL;
    ATK_free(node);
    
    ATK_free(state->name);
    state->name = NULL;
    state->pool = NULL;
    state->node = NULL;
    ATK_free(state);

    context->hidden.unknown.data1 = NULL;
    ATK_free(context);
    return 0;
}

/**
 * Deactivates an active ATK_RWops
 *
 * This function saves the current state of the ATK_RWops and pages it
 * out, making room for more active files. This function does nothing
 * if the file is not active.
 *
 * @param context   the ATK_RWops to deactivate
 *
 * @return 0 on success, -1 on failure
 */
int ATK_RWdeactivate(ATK_RWops* context) {
    if (context == NULL) {
        ATK_SetError("Attempted to page out a null context");
        return -1;
    } else if (context->type != SDL_RWOPS_UNKNOWN) {
        ATK_SetError("Attempted to page out an unmanaged file");
        return -1;
    }
    
    Uint8* check = (Uint8*)context->hidden.unknown.data1;
    if (*check != ATK_FILE_CHECKSUM) {
        ATK_SetError("Attempted to page out an unmanaged file");
        return -1;
    }
    
    ATK_RWstate* state = (ATK_RWstate*)context->hidden.unknown.data1;
    if (!state->active) {
        // Nothing to do
        return 0;
    }
    
    if (context->hidden.unknown.data2 == NULL) {
        ATK_SetError("Managed file was not paged in properly");
        return -1;
    }
    
    SDL_RWops* source = (SDL_RWops*)context->hidden.unknown.data2;
    Sint64 pos = SDL_RWtell(source);
    if (pos < 0 || SDL_RWclose(source) < 0) {
        return -1;
    }
    
    context->hidden.unknown.data2 = NULL;
    state->active = SDL_FALSE;
    state->pos = (size_t)pos;
    state->pool->active--;
    return 0;
}

/**
 * Activates a file in the file pool, loading it into memory
 *
 * If the file pool is already at capacity, this function will instruct
 * the file pool to deactivate another file, to make room. If no file
 * can be deactivated, this function will fail.
 *
 * @param context   the ATK_RWops to activate
 *
 * @return 0 on success, -1 on failure
 */
int ATK_RWactivate(ATK_RWops* context) {
    ATK_RWstate* state = (ATK_RWstate*)context->hidden.unknown.data1;
    if (state->active) {
        state->touch = SDL_TRUE;
        return 0;
    }

    ATK_FilePool* pool = state->pool;
    SDL_LockMutex(pool->mutex);
    int result = 0;
    if (pool->active >= pool->capacity && ATK_PageOutFile(pool) < 0) {
        result = -1;
    } else {
        result = ATK_RWLoad(context);
    }
    SDL_UnlockMutex(pool->mutex);
    return result;
}

#pragma mark -
#pragma mark File Pools
/** The "hidden" default file pool. Use ATK_DefaultFilePool to access. */
ATK_FilePool* _atk_default_pool = NULL;

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
int ATK_init(Uint32 capacity) {
    if (_atk_default_pool != NULL) {
        ATK_SetError("ATK subsystem already initialized");
        return -1;
    }
    
    _atk_default_pool = ATK_AllocFilePool(capacity);
    return _atk_default_pool == NULL ? -1 : 0;
}

/**
 * Shuts down the managed file subsystem for ATK
 *
 * Any files associated with the managed file subsystem will be immediately
 * closed and disposed. This function does nothing if the subsystem was not
 * initialized.
 */
void ATK_quit() {
    if (_atk_default_pool != NULL) {
        ATK_FreeFilePool(_atk_default_pool);
        _atk_default_pool = NULL;
    }
    kiss_fft_cleanup(); // Just in case
}

/**
 * Returns the default managed file subsystem for ATK
 *
 * This is the managed file pool used by all ATK codec functions. If it is
 * null, those functions will all use the OS for accessing files instead.
 *
 * @return the default managed file subsystem for ATK
 */
ATK_FilePool* ATK_DefaultFilePool() {
    return _atk_default_pool;
}

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
ATK_FilePool* ATK_AllocFilePool(Uint32 capacity) {
    if (capacity == 0) {
        ATK_SetError("ATK capacity must be non-zero");
        return NULL;
    }
    
    ATK_FilePool* pool = ATK_malloc(sizeof(ATK_FilePool));
    if (pool) {
        memset(pool, 0, sizeof(ATK_FilePool));
        pool->capacity = capacity;
        pool->mutex = SDL_CreateMutex();
    }
    return pool;
}

/**
 * Frees a previously allocated file pool
 *
 * Any files associated with the managed file subsystem will be immediately
 * closed and disposed.
 *
 * @param pool  The file pool to dispose
 */
void ATK_FreeFilePool(ATK_FilePool* pool) {
    if (pool == NULL) {
        return;
    }
    
    while (pool->tail) {
        ATK_RWcloseImp(pool->tail->file);
    }
    
    SDL_DestroyMutex(pool->mutex);
    pool->mutex = NULL;
    ATK_free(pool);
}

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
ATK_RWops* ATK_RWFromFilePool(const char *file, const char *mode, ATK_FilePool* pool) {
    // Need to initially open the file
    if (pool == NULL) {
        ATK_SetError("File pool is NULL");
        return NULL;
    } else if (pool->active >= pool->capacity && ATK_PageOutFile(pool) < 0) {
        return NULL;
    }
    
    // Make sure we can load the file normally.
    SDL_RWops* source = SDL_RWFromFile(file, mode);
    if (source == NULL) {
        return NULL;
    }
    
    ATK_RWops* result  = NULL;
    ATK_RWstate* state = NULL;
    ATK_FileNode* node = NULL;
    char* namecopy =  NULL;
    result = ATK_malloc(sizeof(ATK_RWops));
    if (result == NULL) {
        ATK_OutOfMemory();
        goto fail;
    }
    memset(result, 0, sizeof(ATK_RWops));

    state = ATK_malloc(sizeof(ATK_RWstate));
    if (state == NULL) {
        ATK_OutOfMemory();
        goto fail;
    }
    memset(state, 0, sizeof(ATK_RWstate));

    node = ATK_malloc(sizeof(ATK_FileNode));
    if (node == NULL) {
        ATK_OutOfMemory();
        goto fail;
    }
    memset(node, 0, sizeof(ATK_FileNode));

    namecopy = ATK_malloc(strlen(file)+1);
    if (namecopy == NULL) {
        ATK_OutOfMemory();
        goto fail;
    }
    memset(namecopy, 0, strlen(file)+1);
    strcpy(namecopy,file);
    
    Uint32 bitset = 0;
    for(const char* c = mode; *c != '\0'; c++) {
        switch(*c) {
            case 'r':
                bitset |= ATK_RW_MODE_READ;
                break;
            case 'w':
                bitset |= ATK_RW_MODE_WRITE;
                break;
            case 'a':
                bitset |= ATK_RW_MODE_APPEND;
                break;
            case 'b':
                bitset |= ATK_RW_MODE_BINARY;
                break;
            case '+':
                if (bitset & ATK_RW_MODE_READ) {
                    bitset |= ATK_RW_MODE_WRITE;
                } else {
                    bitset |= ATK_RW_MODE_READ;
                }
                break;
        }
    }
    
    SDL_LockMutex(pool->mutex);
    
    state->name   = namecopy;
    state->node   = node;
    state->mode   = bitset;
    state->active = SDL_TRUE;
    state->touch  = SDL_TRUE;
    state->pool   = pool;
    state->checksum = ATK_FILE_CHECKSUM;
    
    result->size = ATK_RWsizeImp;
    result->seek = ATK_RWseekImp;
    result->read = ATK_RWreadImp;
    result->write = ATK_RWwriteImp;
    result->close = ATK_RWcloseImp;
    result->type = SDL_RWOPS_UNKNOWN;
    result->hidden.unknown.data1 = state;
    result->hidden.unknown.data2 = source;

    node->file  = result;
    if (pool->tail != NULL) {
        node->prev = pool->tail;
        node->next = pool->head;
        pool->tail->next = node;
        pool->head->prev = node;
        if (pool->head->next == pool->head) {
            pool->head->next = node;
        }
    } else {
        node->next = node;
        node->prev = node;
        pool->head = node;
    }
    pool->tail = node;
    pool->active++;
    pool->total++;

    SDL_UnlockMutex(pool->mutex);
    return result;
    
fail:
    if (result != NULL) {
        ATK_free(result);
    }
    if (state != NULL) {
        ATK_free(state);
    }
    if (node != NULL) {
        ATK_free(node);
    }
    if (namecopy != NULL) {
        ATK_free(namecopy);
    }
    return NULL;
}

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
SDL_bool ATK_RWInFilePool(ATK_RWops* context, ATK_FilePool* pool) {
    if (context == NULL || context->type != SDL_RWOPS_UNKNOWN) {
        return SDL_FALSE;
    }
    
    Uint8* check = (Uint8*)context->hidden.unknown.data1;
    if (*check != ATK_FILE_CHECKSUM) {
        return SDL_FALSE;
    }
    
    ATK_RWstate* state = (ATK_RWstate*)context->hidden.unknown.data1;
    return state->pool == pool;
}
