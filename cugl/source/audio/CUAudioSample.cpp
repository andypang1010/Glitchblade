//
//  CUAudioSample.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides the base class for representing an audio sample, or
//  a file with prerecorded audio. An audio sample is not a node in the audio
//  graph.  Instead, a sample is provided to an AudioPlayer for playback.
//  Multiple AudioPlayers can share the same sample, allowing copies of the
//  sound to be played simultaneously
//
//  This module provides support for both in-memory audio samples and streaming
//  audio. The former is ideal for sound effects, but not long-playing music.
//  The latter introduces some latency and is only ideal for long-playing music.
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
#include <cugl/audio/CUAudioSample.h>
#include <cugl/audio/graph/CUAudioPlayer.h>
#include <cugl/core/util/CUDebug.h>
#include <cugl/core/util/CUFiletools.h>

using namespace cugl;
using namespace cugl::audio;

#pragma mark Constructors

/**
 * Creates a degenerate audio sample with no buffer.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an asset on
 * the heap, use one of the static constructors instead.
 */
AudioSample::AudioSample() : Sound(),
_frames(0),
_stream(false),
_buffer(nullptr) {
    _type = AudioType::UNKNOWN;
}

/**
 * Initializes a new audio sample for the given file.
 *
 * The choice of buffered or streaming is independent of the file type.
 * If the file is streamed, it will not be loaded into memory.  Otherwise,
 * this initializer will allocate memory to read the asset into memory.
 *
 * @param file      The source file for the audio sample
 * @param stream    Wether to stream the audio from the file.
 *
 * @return true if the sound source was initialized successfully
 */
bool AudioSample::init(const std::string file, bool stream) {
    std::string path = filetool::normalize_path(file);
    if (!filetool::file_exists(path)) {
        CULogError("Cannot find file %s",path.c_str());
        return false;
    }
    
    _file = file;
    _type = audio::guessType(file);
    _stream = stream;
    std::shared_ptr<AudioDecoder> decoder = getDecoder();
    if (decoder == nullptr) {
        CULogError("Could not open '%s': %s\n", path.c_str(), SDL_GetError());
        return false;
    }
    
    _channels = decoder->getChannels();
    _frames = decoder->getLength();
    _rate   = decoder->getSampleRate();
    
    if (!_stream) {
        _buffer = (float*)SDL_malloc((size_t)(_frames*_channels*sizeof(float)));
        Sint64 size = decoder->decode(_buffer);
        return size >= 0;
    }
    return true;
}

/**
 * Initializes an empty audio sample of the given size.
 *
 * The audio sample will be in-memory (not streamed).  The contents of the
 * buffer will all be 0s.  Use the {@link getBuffer()} method to write data
 * to this buffer.
 *
 * @param channels  the number of audio channels
 * @param rate      the sampling rate of this source
 * @param frames    the number of frames in this source
 *
 * @return true if the audio sample was initialized successfully
 */
bool AudioSample::init(Uint8 channels, Uint32 rate, Uint32 frames) {
    _channels = channels;
    _frames = frames;
    _rate = rate;
    _buffer = (float*)SDL_malloc((size_t)(_channels*_frames*sizeof(float)));
    _stream = false;
    _type  = AudioType::IN_MEMORY;
    return true;
}

/**
 * Initializes an audio sample with the given JSON specificaton.
 *
 * This initializer is designed to receive the "data" object from the
 * JSON passed to {@link SceneLoader}.  This JSON format supports the
 * following attribute values:
 *
 *      "file":     The path to the source, relative to the asset directory
 *      "stream":   A boolean, indicating whether to stream the sample
 *      "volume":   A float, representing the volume
 *
 * All attributes are optional.  There are no required attributes. By default,
 * audio samples are not streamed, meaning they are fully loaded into memory.
 * This is recommended for sound effects, but not for music.
 *
 * @param data      The JSON object specifying the audio sample
 *
 * @return true if the audio sample was initialized successfully
 */
bool AudioSample::initWithData(const std::shared_ptr<JsonValue>& data) {
    std::string source = data->has("file") ? filetool::normalize_path(data->getString("file","")) : "";
    bool stream = data->getBool("stream",false);
    if (init(source,stream)) {
        _volume = data->getFloat("volume",1.0f);
        return true;
    }
    return false;
}

/**
 * Deletes the sample resources and resets all attributes.
 *
 * This will delete the file reference and any allocated buffers. You must
 * reinitialize the sound data to use the object.
 */
void AudioSample::dispose() {
    _rate = 0;
    _frames = 0;
    _channels = 0;
    _stream = false;
    if (_buffer != nullptr) {
        SDL_free(_buffer);
        _buffer = nullptr;
    }
    _type = AudioType::UNKNOWN;
}

#pragma mark -
#pragma mark Decoder Supports
/**
 * Returns a new decoder for this audio sample
 *
 * A decoder is used to extract the sound data into a PCM buffer.  It should
 * not be accessed directly. Instead it is used by the audio graph to acquire
 * playback data.
 *
 * @return a new decoder for this audio sample
 */
std::shared_ptr<AudioDecoder> AudioSample::getDecoder() {
    return AudioDecoder::alloc(_file,_type);
}

/**
 * Returns a playble audio node for this asset.
 *
 * This audio node may be attached to an {@link AudioOutput} for immediate
 * playback.  Nodes are distinct.  Each call to this method allocates
 * a new audio node.
 *
 * @return a playble audio node for this asset.
 */
std::shared_ptr<audio::AudioNode> AudioSample::createNode() {
    std::shared_ptr<Sound> source = shared_from_this();
    std::shared_ptr<audio::AudioPlayer> player = audio::AudioPlayer::alloc(std::dynamic_pointer_cast<AudioSample>(source));
    player->setGain(_volume);
    return std::dynamic_pointer_cast<audio::AudioNode>(player);
}
