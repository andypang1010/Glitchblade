//
//  CUPhysObstEvent.cpp
//  Cornell University Game Library (CUGL)
//
//  This module represents an event for an obstacle state change. This typically
//  occurs when the user sets the position or velocity (or any other changes
//  to the obstacle state) manually, outside of the simulation.
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
#include <cugl/physics2/distrib/CUPhysObstEvent.h>

using namespace cugl;
using namespace cugl::physics2;
using namespace cugl::physics2::distrib;

/** Creates a new constants group with default values */
PhysObstEvent::BoolConsts::BoolConsts() {
    isStatic = false;
    isEnabled = false;
    isAwake = false;
    isSleepingAllowed = false;
    isFixedRotation = false;
    isBullet = false;
    isSensor = false;
}

/** Creates a new constants group with default values */
PhysObstEvent::FloatConsts::FloatConsts() {
    density = 0;
    friction = 0;
    restitution = 0;
    linearDamping = 0;
    angularDamping = 0;
    gravityScale = 0;
    mass = 0;
    inertia = 0;
}

/**
 * Returns a byte vector serializing this event
 *
 * @return a byte vector serializing this event
 */
std::vector<std::byte> PhysObstEvent::serialize() {
    _serializer.reset();
    _serializer.writeUint32((uint32)_type);
    _serializer.writeUint64(_obstacleId);
    switch (_type) {
        case PhysObstEvent::EventType::CREATION:
            _serializer.writeUint32(_factoryId);
            _serializer.writeByteVector(*_packedParam);
            break;
        case PhysObstEvent::EventType::DELETION:
            break;
        case PhysObstEvent::EventType::BODY_TYPE:
            _serializer.writeUint32(_bodyType);
            break;
        case PhysObstEvent::EventType::POSITION:
            _serializer.writeFloat(_pos.x);
            _serializer.writeFloat(_pos.y);
            break;
        case PhysObstEvent::EventType::VELOCITY:
            _serializer.writeFloat(_vel.x);
            _serializer.writeFloat(_vel.y);
            break;
        case PhysObstEvent::EventType::ANGLE:
            _serializer.writeFloat(_angle);
            break;
        case PhysObstEvent::EventType::ANGULAR_VEL:
            _serializer.writeFloat(_angularVel);
            break;
        case PhysObstEvent::EventType::BOOL_CONSTS:
            _serializer.writeBool(_isEnabled);
            _serializer.writeBool(_isAwake);
            _serializer.writeBool(_isSleepingAllowed);
            _serializer.writeBool(_isFixedRotation);
            _serializer.writeBool(_isBullet);
            _serializer.writeBool(_isSensor);
            break;
        case PhysObstEvent::EventType::FLOAT_CONSTS:
            _serializer.writeFloat(_density);
            _serializer.writeFloat(_friction);
            _serializer.writeFloat(_restitution);
            _serializer.writeFloat(_linearDamping);
            _serializer.writeFloat(_angularDamping);
            _serializer.writeFloat(_gravityScale);
            _serializer.writeFloat(_mass);
            _serializer.writeFloat(_inertia);
            _serializer.writeFloat(_centroid.x);
            _serializer.writeFloat(_centroid.y);
            break;
        case PhysObstEvent::EventType::OWNER_ACQUIRE:
            _serializer.writeUint64(_duration);
            break;
        case PhysObstEvent::EventType::OWNER_RELEASE:
            break;
        default:
            CUAssertLog(false, "Serializing invalid obstacle event type");
    }
    return _serializer.serialize();
}

/**
 * Deserializes this event from a byte vector.
 *
 * This method will set the type of the event and all relevant fields.
 */
void PhysObstEvent::deserialize(const std::vector<std::byte>& data) {
    if (data.size() < sizeof(Uint32) + sizeof(Uint64))
        return;
    _deserializer.reset();
    _deserializer.receive(data);
    _type = (EventType)_deserializer.readUint32();
    _obstacleId = _deserializer.readUint64();
    switch (_type) {
        case PhysObstEvent::EventType::CREATION:
            _factoryId = _deserializer.readUint32();
            _packedParam = std::make_shared<std::vector<std::byte>>(data.begin() + 2 * sizeof(Uint32) + sizeof(Uint64), data.end());
            break;
        case PhysObstEvent::EventType::DELETION:
            break;
        case PhysObstEvent::EventType::BODY_TYPE:
            _bodyType = (b2BodyType)_deserializer.readUint32();
            break;
        case PhysObstEvent::EventType::POSITION:
            _pos.x = _deserializer.readFloat();
            _pos.y = _deserializer.readFloat();
            break;
        case PhysObstEvent::EventType::VELOCITY:
            _vel.x = _deserializer.readFloat();
            _vel.y = _deserializer.readFloat();
            break;
        case PhysObstEvent::EventType::ANGLE:
            _angle = _deserializer.readFloat();
            break;
        case PhysObstEvent::EventType::ANGULAR_VEL:
            _angularVel = _deserializer.readFloat();
            break;
        case PhysObstEvent::EventType::BOOL_CONSTS:
            _isEnabled = _deserializer.readBool();
            _isAwake = _deserializer.readBool();
            _isSleepingAllowed = _deserializer.readBool();
            _isFixedRotation = _deserializer.readBool();
            _isBullet = _deserializer.readBool();
            _isSensor = _deserializer.readBool();
            break;
        case PhysObstEvent::EventType::FLOAT_CONSTS:
            _density = _deserializer.readFloat();
            _friction = _deserializer.readFloat();
            _restitution = _deserializer.readFloat();
            _linearDamping = _deserializer.readFloat();
            _angularDamping = _deserializer.readFloat();
            _gravityScale = _deserializer.readFloat();
            _mass = _deserializer.readFloat();
            _inertia = _deserializer.readFloat();
            _centroid.x = _deserializer.readFloat();
            _centroid.y = _deserializer.readFloat();
            break;
        case PhysObstEvent::EventType::OWNER_ACQUIRE:
            _duration = _deserializer.readUint64();
            break;
        case PhysObstEvent::EventType::OWNER_RELEASE:
            break;
        default:
            CUAssertLog(false, "Deserializing invalid obstacle event type");
    }
}
