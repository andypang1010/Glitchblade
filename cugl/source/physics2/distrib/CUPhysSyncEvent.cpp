//
//  CUPhysSyncEvent.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides events for physics synchronization, which are handled
//  by the NetEventController internally.
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
#include <cugl/physics2/distrib/CUPhysSyncEvent.h>

using namespace cugl;
using namespace cugl::physics2;
using namespace cugl::physics2::distrib;

/** Creates a new parameter set with default values */
PhysSyncEvent::Parameters::Parameters() {
    obsId = 0;
    x = 0;
    y = 0;
    vx = 0;
    vy = 0;
    angle = 0;
    vAngular = 0;
}

/**
 * Snapshots an obstacle's current position and velocity.
 *
 * This snapshot is then added to the list for serialization.
 *
 * @param obs   the obstacle reference to add
 * @param id    the global id of the obstacle
 */
void PhysSyncEvent::addObstacle(Uint64 id, const std::shared_ptr<physics2::Obstacle>& obs) {
    if (_obsSet.count(id))
        return;
    
    _obsSet.insert(id);
    Parameters param;
    param.obsId = id;
    param.x = obs->getX();
    param.y = obs->getY();
    param.vx = obs->getVX();
    param.vy = obs->getVY();
    param.angle = obs->getAngle();
    param.vAngular = obs->getAngularVelocity();
    _syncList.push_back(param);
}

/**
 * Returns a byte vector serializing the current list of snapshots.
 *
 * @return a byte vector serializing the current list of snapshots.
 */
std::vector<std::byte> PhysSyncEvent::serialize() {
    _serializer.reset();
    _serializer.writeUint64((Uint64)_syncList.size());
    for (auto it = _syncList.begin(); it != _syncList.end(); it++) {
        Parameters& obj = (*it);
        _serializer.writeUint64(obj.obsId);
        _serializer.writeFloat(obj.x);
        _serializer.writeFloat(obj.y);
        _serializer.writeFloat(obj.vx);
        _serializer.writeFloat(obj.vy);
        _serializer.writeFloat(obj.angle);
        _serializer.writeFloat(obj.vAngular);
    }
    return _serializer.serialize();
}

/**
 * Unpacks a byte vector into a list of snapshots.
 *
 * These snapshots can then be used in physics synchronizations.
 *
 * @param data the byte vector to deserialize
 */
void PhysSyncEvent::deserialize(const std::vector<std::byte>& data) {
    if (data.size() < 4)
        return;
    
    _deserializer.reset();
    _deserializer.receive(data);
    Uint64 numObjs = _deserializer.readUint64();
    for (size_t i = 0; i < numObjs; i++) {
        Parameters param;
        param.obsId = _deserializer.readUint64();
        param.x = _deserializer.readFloat();
        param.y = _deserializer.readFloat();
        param.vx = _deserializer.readFloat();
        param.vy = _deserializer.readFloat();
        param.angle = _deserializer.readFloat();
        param.vAngular = _deserializer.readFloat();
        _syncList.push_back(param);
    }
}
