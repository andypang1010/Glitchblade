//
//  CUAudioTypes.h
//  Cornell University Game Library (CUGL)
//
//  This header provides the enumeration that specifies the various audio types.
//  These types are determined by the current version of SDL_Codec.
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
#include <cugl/audio/CUAudioTypes.h>
#include <cugl/core/CUBase.h>

using namespace cugl;
using namespace cugl::audio;

/**
 * Returns the type suggested by the given file name
 *
 * The type will be determined from the file extension (e.g. .wav, .mp3,
 * .ogg, etc).
 *
 * @param file  The file name
 *
 * @return the type suggested by the given file name
 */
AudioType cugl::audio::guessType(const std::string file) {
    AudioType type = AudioType::UNKNOWN;
    const char *ret = std::strrchr(file.c_str(), '.');
    if (ret == nullptr) {
        return AudioType::UNKNOWN;
    }
    
    ret++;
    switch(*ret) {
        case 'w':
        case 'W':
            if ((strcasecmp(ret, "wav") == 0 || strcasecmp(ret, "wave") == 0)) {
                type = AudioType::WAV_FILE;
            }
            break;
        case 'm':
        case 'M':
            if ((strcasecmp(ret, "mp3") == 0 || strcasecmp(ret, "mpg") == 0)) {
                type = AudioType::MP3_FILE;
            }
            break;
        case 'o':
        case 'O':
            if ((strcasecmp(ret, "ogg") == 0 || strcasecmp(ret, "oga") == 0)) {
                type = AudioType::OGG_FILE;
            }
            break;
        case 'f':
        case 'F':
            if ((strcasecmp(ret, "flac") == 0 || strcasecmp(ret, "flc") == 0)) {
                type = AudioType::FLAC_FILE;
            }
            break;
        default:
            type = AudioType::UNKNOWN;
    }
    
    return type;
}

/**
 * Returns a string description of the given type
 *
 * @param type  The audio source type
 *
 * @return a string description of the given type
 */
std::string cugl::audio::typeName(AudioType type) {
    switch(type) {
    case AudioType::WAV_FILE:
        return "WAV file";
    case AudioType::MP3_FILE:
        return "MP3 file";
    case AudioType::OGG_FILE:
        return "OGG Vorbis file";
    case AudioType::FLAC_FILE:
        return "OGG Vorbis file";
    case AudioType::IN_MEMORY:
        return "In-memory audio source";
    default:
        return "Unknown file source";
    }
}
