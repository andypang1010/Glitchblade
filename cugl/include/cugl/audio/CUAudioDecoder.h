//
//  CUAudioDecoder.h
//  Cornell University Game Library (CUGL)
//
//  An audio decoder converts a binary file into a pageable PCM data stream. It
//  is built on top of our extension to SDL2: SDL_Codec.  This class unifies the
//  API for all of the supported audio codes (WAV, MP3, OGG, FLAC)
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
#ifndef __CU_AUDIO_DECODER_H__
#define __CU_AUDIO_DECODER_H__
#include <SDL.h>
#include <string>
#include <memory>
#include <cugl/audio/CUAudioTypes.h>
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
 * This class represents an audio file decoder.
 *
 * An audio file decoder takes an audio file and converts into a linear-PCM
 * stream. This stream is used by the classes {@link AudioSample} (to read the
 * audio data into memory) and {@link audio::AudioPlayer} (to play an audio
 * stream directly from the file).
 *
 * This decoder supports all the file types in {@link AudioType}, with the
 * exception of {@link AudioType#IN_MEMORY}. The restrictions for the various
 * file types are described in the enumeration for that type.
 *
 * This class ensures that all memory pages are uniform in size. When the
 * page size is variable, this decoder tries to balance memory requirements
 * paging efficiency.
 *
 * The decoder always interleaves the audio channels. MP3 and WAV ADPCM only
 * support mono or stereo. But all other formats can support more channels.
 * SDL supports up to 8 channels (7.1 stereo) in general. Note that the channel
 * layout for OGG data is nonstandard (e.g. channels > 3 are not stereo compatible),
 * so this decoder standardizes the channel layout to agree with FLAC and other
 * data encodings.
 *
 * A decoder is NOT thread safe. If a decoder is used by an audio thread, then
 * it should not be accessed directly in the main thread, and vice versa.
 */
class AudioDecoder {
protected:
    /** The source for this decoder */
    std::string _file;
    
    /** The codec type for the audio file */
    AudioType _type;
    
    /** The number of channels in this sound source (max 32) */
    Uint8  _channels;
    
    /** The sampling rate (frequency) of this sound source */
    Uint32 _rate;
    
    /** The number of frames in this sounds source */
    Uint64 _frames;
    
    /** The size of a decoder chunk */
    Uint32 _pagesize;
    
    /** The current page in the stream */
    Uint32 _currpage;
    
    /** The final page in the stream */
    Uint32 _lastpage;
    
    /** The underlying decoder from SDL_codec */
    ATK_AudioSource* _source;
    
public:
    /**
     * Creates an initialized audio decoder
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an asset on
     * the heap, use one of the static constructors instead.
     */
    AudioDecoder();
    
    /**
     * Deletes this decoder, disposing of all resources.
     */
    ~AudioDecoder() { dispose(); }
    
    /**
     * Initializes a new decoder for the given file.
     *
     * The {@link AudioType} of the file will be inferred from the
     * file suffix. If this audio type is not correct, this initializer
     * will fail and return false.
     *
     * @param file  the source file for the decoder
     *
     * @return true if the decoder was initialized successfully
     */
    bool init(const std::string file);
    
    /**
     * Initializes a new decoder for the given file and type.
     *
     * If the audio type is not correct for this file, this initializer
     * will fail and return false.
     *
     * @param file  the source file for the decoder
     * @param type  the codec type for this file
     *
     * @return true if the decoder was initialized successfully
     */
    bool init(const std::string file, AudioType type);
    
    /**
     * Deletes the decoder resources and resets all attributes.
     *
     * This will close the associated file. You must reinitialize the decoder
     * to use it.
     */
    void dispose();
    
    
#pragma mark Static Constructors
    /**
     * Creates a newly allocated decoder for the given file.
     *
     * The {@link AudioType} of the file will be inferred from the
     * file suffix. If this audio type is not correct, this allocator
     * will fail and return nullptr.
     *
     * @param file  the source file for the decoder
     *
     * @return a newly allocated decoder for the given file.
     */
    static std::shared_ptr<AudioDecoder> alloc(const std::string file) {
        std::shared_ptr<AudioDecoder> result = std::make_shared<AudioDecoder>();
        return (result->init(file) ? result : nullptr);
    }
    
    /**
     * Creates a newly allocated decoder for the given file.
     *
     * If the audio type is not correct for this file, this allocator
     * will fail and return nullptr.
     *
     * @param file  the source file for the decoder
     * @param type  the codec type for this file
     *
     * @return a newly allocated decoder for the given file.
     */
    static std::shared_ptr<AudioDecoder> alloc(const std::string file, AudioType type) {
        std::shared_ptr<AudioDecoder> result = std::make_shared<AudioDecoder>();
        return (result->init(file,type) ? result : nullptr);
    }
    
    
#pragma mark Attributes
    /**
     * Returns the length of this sound source in seconds.
     *
     * The accuracy of this method depends on the specific implementation.
     *
     * @return the length of this sound source in seconds.
     */
    double getDuration() const { return (double)_frames/(double)_rate; }
    
    /**
     * Returns the sample rate of this sound source.
     *
     * @return the sample rate of this sound source.
     */
    Uint32 getSampleRate() const { return _rate; }
    
    /**
     * Returns the frame length of this sound source.
     *
     * The frame length is the duration times the sample rate.
     *
     * @return the frame length of this sound source.
     */
    Uint64 getLength() const { return _frames; }
    
    /**
     * Returns the number of channels used by this sound source
     *
     * A value of 1 means mono, while 2 means stereo. Depending on the file
     * format, other channels are possible. For example, 6 channels means
     * support for 5.1 surround sound.
     *
     * We support up to 32 possible channels.
     *
     * @return the number of channels used by this sound asset
     */
    Uint32 getChannels() const { return _channels; }
    
    /**
     * Returns the file for this audio source
     *
     * This value is the empty string if there was no source file.
     *
     * @return the file for this audio source
     */
    std::string getFile() const { return _file; }
    
    /**
     * Returns the number of frames in a single page of data
     *
     * When multiplied by the number of channels, this gives the number of
     * samples read per page
     *
     * @return the number of frames in a single page of data
     */
    Uint32 getPageSize() const { return _pagesize; }
    
    
#pragma mark Decoding
    /**
     * Returns true if there are still data to be read by the decoder
     *
     * This value will return false if the decoder is at the end of the file
     *
     * @return true if there are still data to be read by the decoder
     */
    bool ready() {
        return (_currpage < getPageCount());
    }
    
    /**
     * Reads a page of data into the provided buffer.
     *
     * The buffer should be able to hold channels * page size many elements.
     * The data is interpretted as floats and channels are all interleaved.
     * If a full page is read, this method should return the page size.  If
     * it reads less, it will return the number of frames read.  It will
     * return -1 on a processing error.
     *
     * @param buffer    The buffer to store the audio data
     *
     * @return the number of frames actually read (-1 on error).
     */
    Sint32 pagein(float* buffer);
    
    /**
     * Returns the current page of this decoder
     *
     * This value is the next page to be read in with the {@link pagein()} command.
     *
     * @return the current page of this decoder
     */
    Uint32 getPage() const { return _currpage; }
    
    /**
     * Sets the current page of this decoder
     *
     * This value is the next page to be read in with the {@link pagein()} command.
     * If the page is greater than the total number of pages, it will be set
     * just beyond the last page.
     *
     * @param page  The new page of this decoder
     */
    void setPage(Uint32 page);
    
    /**
     * Returns the total number of pages in this decoder
     *
     * This value is the maximum value for the {@link setPage} command.
     *
     * @return total number of pages in this decoder
     */
    Uint32 getPageCount() const {
        return _frames % _pagesize == 0 ? _lastpage : _lastpage+1;
    }
    
    /**
     * Rewinds this decoder back the beginning of the stream
     */
    void rewind() { setPage(0); }
    
    /**
     * Decodes the entire audio file, storing its value in buffer.
     *
     * The buffer should be able to hold channels * frames many elements.
     * The data is interpretted as floats and channels are all interleaved.
     * If the method returns -1, then an error occurred during reading.
     *
     * @param buffer    The buffer to store the audio data
     *
     * @return the number of frames actually read (-1 on error).
     */
    Sint64 decode(float* buffer);
    
};

    }
}


#endif /* __CU_AUDIO_DECODER_H__ */
