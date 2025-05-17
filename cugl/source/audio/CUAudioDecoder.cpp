//
//  CUAudioDecoder.cpp
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
#include <cugl/audio/CUAudioDecoder.h>
#include <cugl/core/util/CUDebug.h>

using namespace cugl;
using namespace cugl::audio;

/**
 * Creates an initialized audio decoder
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an asset on
 * the heap, use one of the static constructors instead.
 */
AudioDecoder::AudioDecoder() :
_rate(0),
_file(""),
_frames(0),
_channels(0),
_pagesize(0),
_lastpage(0),
_currpage(0)
{
    _type = AudioType::UNKNOWN;
}

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
bool AudioDecoder::init(const std::string file) {
    AudioType type = audio::guessType(file);
    if (type == AudioType::UNKNOWN) {
        CULogError("File %s does not match any supported audio type.", file.c_str());
        return false;
    }
    return init(file,type);
}

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
bool AudioDecoder::init(const std::string file, AudioType type) {
    switch(type) {
    case AudioType::WAV_FILE:
        _source = ATK_LoadWAV(file.c_str());
        break;
    case AudioType::MP3_FILE:
        _source = ATK_LoadMP3(file.c_str());
        break;
    case AudioType::OGG_FILE:
        _source = ATK_LoadVorbis(file.c_str());
        break;
    case AudioType::FLAC_FILE:
        _source = ATK_LoadFLAC(file.c_str());
        break;
    default:
        CULogError("No decoder support for type %s", audio::typeName(type).c_str());
        return false;
    }
    if (_source == NULL) {
        CULogError("File %s is not a valid %s.",file.c_str(),audio::typeName(type).c_str());
        return false;
    }
    _file = file;
    _type = type;
 
    _channels = _source->metadata.channels;
    _rate   = _source->metadata.rate;
    _frames = _source->metadata.frames;

    _pagesize = ATK_GetSourcePageSize(_source);
    _lastpage = ATK_GetSourceLastPage(_source);
    _currpage = 0;
    return true;
}

/**
 * Deletes the decoder resources and resets all attributes.
 *
 * This will close the associated file. You must reinitialize the decoder
 * to use it.
 */
void AudioDecoder::dispose() {
    if (_source != NULL) {
        ATK_UnloadSource(_source);
        _source = NULL;
    }
    _rate = 0;
    _file = "";
    _frames = 0;
    _channels = 0;
    _pagesize = 0;
    _lastpage = 0;
    _currpage = 0;
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
Sint32 AudioDecoder::pagein(float* buffer) {
    return ATK_ReadSourcePage(_source,buffer);
}

/**
 * Sets the current page of this decoder
 *
 * This value is the next page to be read in with the {@link pagein()} command.
 * If the page is greater than the total number of pages, it will be set
 * just beyond the last page.
 *
 * @param page  The new page of this decoder
 */
void AudioDecoder::setPage(Uint32 page) {
    _currpage = ATK_SeekSourcePage(_source, page);
}

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
Sint64 AudioDecoder::decode(float* buffer) {
    return ATK_ReadSource(_source, buffer);
}
