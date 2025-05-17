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
#ifndef __CU_AUDIO_TYPES_H__
#define __CU_AUDIO_TYPES_H__
#include <string>

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
 * This enum represents the currently supported audio sources.
 *
 * Currenltly, we only support file types that are easy to stream into
 * a Linear PCM format. We recommend that you use OGG for music (which
 * is streamed) and WAV for sound effects (which is buffered).
 *
 * All audio sources in CUGL interleave the audio channels. MP3 and WAV
 * ADPCM only support mono or stereo. But all other formats can support
 * more channels. SDL supports up to 8 channels (7.1 stereo) in general.
 * Note that the channel layout for OGG data is nonstandard (e.g.
 * channels > 3 are not stereo compatible), so CUGL standardizes the
 * channel layout to agree with FLAC and other data encodings.
 */
enum class AudioType : int {
    /** An unknown audio file source */
    UNKNOWN   = -1,

    /**
     * A (Windows-style) WAV file
     *
     * CUGL supports PCM, IEEE Float, and ADPCM encoding (both MS and IMA).
     * However, it does not support MP3 data stored in a WAV file. It also
     * does not support A-law or mu-law.
     */
    WAV_FILE  = 0,

    /**
     * A simple MP3 file
     *
     * For licensing reasons, MP3 support is provided by minimp3. The does
     * provide support for VBR MP3 files, but the files must be mono or
     * stereo. CUGL does not support MP3 surround.
     */
    MP3_FILE  = 1,

    /**
     * An ogg vorbis file
     *
     * CUGL only supports Vorbis encodings. It does not support FLAC data
     * encoded in an ogg file container. It also does not support the newer
     * Opus codec.
     */
    OGG_FILE  = 2,

    /**
     * A FLAC file
     *
     * CUGL only supports native FLAC encodings. It does not support FLAC data
     * encoded in an ogg file container. In addition, the FLAC data must have a
     * complete stream info header containing the size and channel data.
     */
    FLAC_FILE = 3,

    /**
     * An in-memory sound source
     *
     * These sound sources are linear PCM signals that are generatee
     * programatically, and do not correspond to an audio file.
     */
    IN_MEMORY = 4
};

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
AudioType guessType(const std::string file);

/**
 * Returns a string description of the given type
 *
 * @param type  The audio source type
 *
 * @return a string description of the given type
 */
std::string typeName(AudioType type);

    }
}

#endif /* __CU_AUDIO_TYPES_H__ */
