//
//  CUGameStateEvent.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides an event for a game state change. It is handled by
//  the NetEventController internally.
//
//  CUGL MIT License:
//      This software is provided 'as-is', without any express or implied
//      warranty.  In no event will the authors be held liable for any damages
//      arising from the use of this software.
//
//      Permission is granted to anyone to use this software for any purpose,
//      including commercial applications, and to alter it and redistribute it
//      freely, subject to the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not
//      be misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
//
//  Author: Barry Lyu
//  Version: 7/3/24 (CUGL 3.0 reorganization)
//
#include <cugl/physics2/distrib/CUGameStateEvent.h>

using namespace cugl;
using namespace cugl::physics2;
using namespace cugl::physics2::distrib;

/**
 * Returns a byte vector serializing this event
 *
 * @return a byte vector serializing this event
 */
std::vector<std::byte> GameStateEvent::serialize() {
    std::vector<std::byte> data;
    switch (_type) {
        case EventType::GAME_START:
            data.push_back(std::byte(EventType::GAME_START));
            break;
        case EventType::GAME_RESET:
            data.push_back(std::byte(EventType::GAME_RESET));
            break;
        case EventType::GAME_PAUSE:
            data.push_back(std::byte(EventType::GAME_PAUSE));
            break;
        case EventType::GAME_RESUME:
            data.push_back(std::byte(EventType::GAME_RESUME));
            break;
        case EventType::CLIENT_RDY:
            data.push_back(std::byte(EventType::CLIENT_RDY));
            break;
        case EventType::UID_ASSIGN:
            data.push_back(std::byte(EventType::UID_ASSIGN));
            data.push_back(std::byte(_shortUID));
            break;
        default:
            CUAssertLog(false, "Serializing invalid game state event type");
    }
    return data;
}

/**
 * Deserializes this event from a byte vector.
 *
 * This method will set the type of the event and all relevant fields.
 */
void GameStateEvent::deserialize(const std::vector<std::byte>& data) {
    EventType flag = (EventType)data[0];
    switch (flag) {
        case EventType::GAME_START:
            _type = EventType::GAME_START;
            break;
        case EventType::GAME_RESET:
            _type = EventType::GAME_RESET;
            break;
        case EventType::GAME_PAUSE:
            _type = EventType::GAME_PAUSE;
            break;
        case EventType::GAME_RESUME:
            _type = EventType::GAME_RESUME;
            break;
        case EventType::CLIENT_RDY:
            _type = EventType::CLIENT_RDY;
            break;
        case EventType::UID_ASSIGN:
            _type = EventType::UID_ASSIGN;
            _shortUID = (Uint8)data[1];
            break;
        default:
            CUAssertLog(false, "Deserializing game state event type");
    }
}
