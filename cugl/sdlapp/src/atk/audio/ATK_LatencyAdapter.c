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
#include <ATK_audio.h>
#include <ATK_math.h>
#include <ATK_error.h>

/**
 * @file ATK_LatencyAdapter
 *
 * This component provides support for a latency adapter.  A latency adapter
 * is a "last ditch save" on devices with weaker hardware.  It allows the
 * programmer to increase the latency (and the time budget) of a portion of
 * the audio subsystem, without increasing the overall latency of the device.
 */

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
typedef struct ATK_LatencyAdapter {
    /** The read buffer size (should be > output size) */
    size_t insize;
    /** The write buffer size (should be < input size) */
    size_t outsize;
    /** A callback function for populating the read buffer asynchronously */
    ATK_AudioCallback callback;
    /** User data for the callback function */
    void* userdata;
    
    /** The thread to process asynchronous reads */
    SDL_Thread* thread;
    /** A mutex to protect critical sections */
    SDL_mutex* mutex;
    /** A semaphore signaling data should be pushed to the read buffer */
    SDL_sem* spush;
    /** A semaphore signaling data can be polled to the write buffer */
    SDL_sem* spoll;
    
    /** The front part of the double buffer */
    Uint8* front;
    /** The back part of the double buffer */
    Uint8* back;
    
    /** The number of available bytes in the front buffer */
    size_t frontavail;
    /** The last read byte in the front buffer */
    size_t frontoffst;
    /** The number of available bytes in the back buffer */
    size_t backavail;
    
    /** Whether the latency adapter is paused */
    int paused;
    /** Whether the latency adapter thread is still active */
    SDL_atomic_t active;
} ATK_LatencyAdapter;

/**
 * Swaps the back buffer with the front buffer.
 *
 * If the front buffer still has unread data, this data is shifted to the left
 * and the backing buffer is read afterwards.  Any data left over in the backing
 * buffer is shifted left.  For performance reasons, it is best to minimize this
 * shifting.  This can be done by ensure the input buffer size is a multiple of
 * the output buffer size.
 *
 * @param adapter   The latency adapter
 *
 * @return 0 if swap was successful, otherwise return an error code
 */
static int swap_buffers(ATK_LatencyAdapter* adapter) {
    int result = SDL_SemTryWait(adapter->spoll);
    if (result) {
        return result;
    }
        
    SDL_LockMutex(adapter->mutex);
    if (adapter->frontoffst != adapter->frontavail) {
        memmove(adapter->front,adapter->front+adapter->frontoffst,adapter->frontavail-adapter->frontoffst);
        adapter->frontavail -= adapter->frontoffst;
        adapter->frontoffst = 0;
        size_t rem = adapter->insize-adapter->frontavail;
        if (adapter->backavail >= rem) {
            memcpy(adapter->front+adapter->frontavail, adapter->back, rem);
            memmove(adapter->back,adapter->back+rem,adapter->insize-rem);
            adapter->frontavail += rem;
            adapter->backavail -= rem;
            memset(adapter->back+adapter->backavail,0,adapter->insize-adapter->backavail);
        } else {
            rem -= adapter->backavail;
            memcpy(adapter->front+adapter->frontavail, adapter->back, adapter->backavail);
            memset(adapter->front+adapter->frontavail+adapter->backavail, 0, rem);
            adapter->frontavail += adapter->backavail;
            adapter->backavail = 0;
            memset(adapter->back,0,adapter->insize);
        }
    } else {
        Uint8* temp = adapter->front;
        adapter->front = adapter->back;
        adapter->back = temp;
        adapter->frontoffst = 0;
        adapter->frontavail = adapter->backavail;
        adapter->backavail  = 0;
        memset(adapter->back,0,adapter->insize);
    }
    SDL_UnlockMutex(adapter->mutex);
    
    if (!adapter->paused && !SDL_SemValue(adapter->spush)) {
        SDL_SemPost(adapter->spush);
    }
    return 0;
}


/**
 * Fills the back buffer with data.
 *
 * This function is called by the asychronous thread to gather data.  If the
 * latency adapter does not have a callback function, then this function does
 * nothing.
 *
 * @param adapter   The latency adapter
 */
static void fill_adapter(ATK_LatencyAdapter* adapter) {
    if (adapter->callback == NULL) {
        if (!SDL_SemValue(adapter->spoll)) {
            SDL_SemPost(adapter->spoll);
        }
        return;
    }
    
    SDL_LockMutex(adapter->mutex);
    
    size_t amt = adapter->insize-adapter->backavail;
    if (amt) {
        amt = adapter->callback(adapter->userdata,adapter->back+adapter->backavail,amt);
        adapter->backavail += amt;
    }
    
    SDL_UnlockMutex(adapter->mutex);
    
    if (!SDL_SemValue(adapter->spoll)) {
        SDL_SemPost(adapter->spoll);
    }
}

/**
 * The SDL thread for the latency adapter
 *
 * @param data  The thread user data (likely NULL)
 *
 * @return 0 on successful completion, otherwise -1
 */
static int thread_func(void* data) {
    if (data == NULL) {
        return -1;
    }
    
    ATK_LatencyAdapter* adapter = (ATK_LatencyAdapter*)data;
    while (SDL_AtomicGet(&(adapter->active))) {
        SDL_SemWait(adapter->spush);
        fill_adapter(adapter);
    }
    return 0;
}

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
ATK_LatencyAdapter* ATK_AllocLatencyAdapter(size_t input, size_t output,
                                            ATK_AudioCallback callback,
                                            void* userdata) {
    size_t maxsize = input < output ? output : input;
    if (maxsize > input) {
    	ATK_SetError("Latency adapters require input (%zu) >= output (%zu)",input,output);
    	return NULL;
    }
    
    Uint8* front = NULL;
    Uint8* back  = NULL;
    SDL_sem* spush = NULL;
    SDL_sem* spoll = NULL;
    SDL_mutex* mutex = NULL;
    SDL_Thread* thread = NULL;
    ATK_LatencyAdapter* result = NULL;

    front = (Uint8*)ATK_malloc(maxsize*sizeof(Uint8));
    back  = (Uint8*)ATK_malloc(maxsize*sizeof(Uint8));
    if (front == NULL || back == NULL) {
        ATK_OutOfMemory();
        goto out;
    }
    memset(front,0,maxsize*sizeof(Uint8));
    memset(back,0,maxsize*sizeof(Uint8));

    spush = SDL_CreateSemaphore(1);
    spoll = SDL_CreateSemaphore(0);
    if (spush == NULL || spoll == NULL) {
        goto out;
    }

    mutex = SDL_CreateMutex();
    if (mutex == NULL) {
        goto out;
    }
    
    result = (ATK_LatencyAdapter*)ATK_malloc(sizeof(ATK_LatencyAdapter));
    if (result == NULL) {
        ATK_OutOfMemory();
        goto out;
    }
    
    result->front = front;
    result->back  = back;
    result->insize  = input;
    result->outsize = output;
    result->backavail  = 0;
    result->frontavail = 0;
    result->frontoffst = 0;

    result->paused = 1;
    result->spush = spush;
    result->spoll = spoll;
    result->mutex = mutex;
    result->thread = NULL;
    SDL_AtomicSet(&(result->active), 1);

    result->callback = callback;
    result->userdata = userdata;
    
    thread = SDL_CreateThread(thread_func, "Latency Adapter", result);
    if (thread == NULL) {
        goto out;
    }
    
    result->thread = thread;
    return result;
out:
    if (front != NULL) {
        ATK_free(front);
    }
    if (back != NULL) {
        ATK_free(back);
    }
    if (spush != NULL) {
        SDL_DestroySemaphore(spush);
    }
    if (spoll != NULL) {
        SDL_DestroySemaphore(spoll);
    }
    if (mutex != NULL) {
        SDL_DestroyMutex(mutex);
    }
    if (result != NULL) {
        ATK_free(result);
    }
    // Thread cannot be null
    return NULL;
}

/**
 * Frees a previously allocated latency adapter
 *
 * @param adapter   The latency adapter
 */
void ATK_FreeLatencyAdapter(ATK_LatencyAdapter* adapter) {
    if (adapter == NULL) {
        return;
    }
    
    // Force the thread to end
    SDL_AtomicSet(&(adapter->active), 0);
    SDL_SemPost(adapter->spush);
    SDL_WaitThread(adapter->thread,NULL);
    
    adapter->thread = NULL;
    SDL_DestroySemaphore(adapter->spush);
    adapter->spush = NULL;
    SDL_DestroySemaphore(adapter->spoll);
    adapter->spoll = NULL;
    SDL_DestroyMutex(adapter->mutex);
    adapter->mutex = NULL;
    ATK_free(adapter->front);
    adapter->front = NULL;
    ATK_free(adapter->back);
    adapter->back = NULL;

    ATK_free(adapter);
}

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
Sint64 ATK_PollLatencyAdapter(ATK_LatencyAdapter* adapter, Uint8* output, int len) {
    if (len > adapter->frontavail-adapter->frontoffst) {
        if (swap_buffers(adapter) < 0) {
            return -1;
        }
    }
    
    Sint64 amt = len < adapter->frontavail-adapter->frontoffst ? len : adapter->frontavail-adapter->frontoffst;
    memcpy(output,adapter->front+adapter->frontoffst,amt);
    adapter->frontoffst += amt;
    
    return amt;
}

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
Sint64 ATK_PushLatencyAdapter(ATK_LatencyAdapter* adapter, const Uint8* input, int len) {
    
    int result = SDL_TryLockMutex(adapter->mutex);
    if (result < 0) {
        return -1;
    } else if (result == SDL_MUTEX_TIMEDOUT) {
        return 0;
    }
    
    Sint64 amt = adapter->insize-adapter->backavail;
    amt = amt < len ? amt : len;
    memcpy(adapter->back+adapter->backavail,input,amt);
    adapter->backavail += amt;
    
    SDL_UnlockMutex(adapter->mutex);
    return amt;
}

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
void ATK_PauseLatencyAdapter(ATK_LatencyAdapter* adapter, int pauseon) {
    if (adapter->paused == pauseon) {
        return;
    }
    
    adapter->paused = pauseon;
    if (pauseon) {
        SDL_SemWait(adapter->spoll);
    } else if (!SDL_SemValue(adapter->spush)) {
        SDL_SemPost(adapter->spush);
    }
}

/**
 * Resets the latency adapter
 *
 * Reseting empties and zeroes all buffers. It also returns the latency adapter
 * to a paused state. The adapter will need to be unpaused with a call to
 * {@link ATK_PauseLatencyAdapter}
 *
 * @param adapter   The latency adapter
 */
void ATK_ResetLatencyAdapter(ATK_LatencyAdapter* adapter) {
    ATK_PauseLatencyAdapter(adapter,1);
    SDL_LockMutex(adapter->mutex);
    size_t maxsize = adapter->insize < adapter->outsize ? adapter->outsize : adapter->insize;
    memset(adapter->front,0,maxsize*sizeof(Uint8));
    memset(adapter->back,0,maxsize*sizeof(Uint8));
    adapter->frontavail = 0;
    adapter->frontoffst = 0;
    adapter->backavail = 0;
    SDL_UnlockMutex(adapter->mutex);
}

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
int ATK_BlockLatencyAdapter(ATK_LatencyAdapter* adapter) {
    if (!adapter->paused && !SDL_SemValue(adapter->spoll)) {
        SDL_SemWait(adapter->spoll);
        SDL_SemPost(adapter->spoll);
        return 1;
    }
    return 0;
}
