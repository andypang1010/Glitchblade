//
//  CUPhysObstEvent.h
//  Cornell University Game Library (CUGL)
//
//  This module provides an event for an obstacle state change. This typically
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
#ifndef __CU_PHYS_OBJ_EVENT_H__
#define __CU_PHYS_OBJ_EVENT_H__

#include <cugl/physics2/distrib/CUNetEvent.h>
#include <cugl/physics2/distrib/CUObstacleFactory.h>
#include <cugl/physics2/CUObstacle.h>
#include <SDL_stdinc.h>

namespace cugl {

    /**
     * The classes to represent 2-d physics.
     *
     * For 2-d physics, CUGL uses the venerable box2d. For the most part, we
     * do not need anything more than that. However, box2d does involve a lot
     * of boilerplate code in setting up bodies and fixtures. Students have
     * found that they like the "training wheel" classes in this package.
     */
    namespace physics2 {

        /**
         * The classes to implement distributed box2d physics.
         *
         * This namespace represents an extension of our 2-d physics engine
         * to support networking. This package provides automatic synchronization
         * of physics objects across devices.
         */
        namespace distrib {


/**
 * This class represents an event for an obstacle state change.
 *
 * These events are created when the user sets the position or velocity manually,
 * outside of the simulation. It includes all changes to the object state.
 *
 * Instances of this class will be created automatically by the physics world
 * and gathered by the network controller.
 */
class PhysObstEvent : public NetEvent {
public:
    /** Enum for the type of the event. */
    enum class EventType : int  {
        /** An unknown event */
        UNKNOWN = 0,
        /** The creation of an obstacle */
        CREATION = 1,
        /** The delection of an obstacle */
        DELETION = 2,
        /** A change in body type */
        BODY_TYPE = 3,
        /** A change in position */
        POSITION = 4,
        /** A change in velocity */
        VELOCITY = 5,
        /** A change in angle */
        ANGLE = 6,
        /** A change in angular velocity */
        ANGULAR_VEL = 7,
        /** A change in (other) boolean constants */
        BOOL_CONSTS = 8,
        /** A change in (other) float constants */
        FLOAT_CONSTS = 9,
        /** A new owner acquiring this object */
        OWNER_ACQUIRE = 10,
        /** An owner releasing this object */
        OWNER_RELEASE = 11
    };
    
    /**
     * A class representing the additional boolean constants in an obstacle.
     *
     * The boolean attribute changes less frequently, so we pack their
     * changes into a single event.
     */
    class BoolConsts {
    public:
        /** Whether this obstacle is static */
        bool isStatic;
        /** Whether this obstacle is enabled */
        bool isEnabled;
        /** Whether this obstacle is awake */
        bool isAwake;
        /** Whether this obstacle is allowed to sleep */
        bool isSleepingAllowed;
        /** Whether this obstacle should be prevented from rotating */
        bool isFixedRotation;
        /** Whether this obstacle is a bullet */
        bool isBullet;
        /** Whether this obstacle is a sensor */
        bool isSensor;
        
        /** Creates a new constants group with default values */
        BoolConsts();
    };

    /**
     * A class representing the additional float constants in an obstacle.
     *
     * These particular attribute changes less frequently, so we pack their
     * changes into a single event.
     */
    class FloatConsts {
    public:
        /** The obstacle density */
        float density;
        /** The obstacle friction */
        float friction;
        /** The obstacle restitution */
        float restitution;
        /** The linear damping */
        float linearDamping;
        /** The angular damping */
        float angularDamping;
        /** The gravity scale */
        float gravityScale;
        /** The obstacle mass */
        float mass;
        /** The obstacle inertia */
        float inertia;
        /** The obstacle centroid*/
        Vec2 centroid;
        
        /** Creates a new constants group with default values */
        FloatConsts();
    };

    
protected:
    /** The type of the event. */
    EventType _type;
    /** The obstacle global id. */
    Uint64 _obstacleId;
    
    /**
     * The obstacle factory id.
     *
     * This is obtained by calling {@link NetPhysicsController#attachFactory}.
     */
    Uint32 _factoryId;
    
    /** The packed parameter for obstacle creation. */
    std::shared_ptr<std::vector<std::byte>> _packedParam;

    /** The field for EventType::POSITION */
    Vec2 _pos;
    /** The field for EventType::VELOCITY */
    Vec2 _vel;
    /** The field for EventType::ANGLE */
    float _angle;
    /** The field for EventType::ANGULAR_VEL */
    float _angularVel;
    
    // Fields for EventType::BOOL_CONSTS
    /** Whether the event represents a static obstacle */
    bool _isStatic;
    /** Whether the event represents an enabled obstacle */
    bool _isEnabled;
    /** Whether the event represents an awake obstacle */
    bool _isAwake;
    /** Whether the event represents a sleepable obstacle */
    bool _isSleepingAllowed;
    /** Whether the event represents an obstacle with fixed rotation */
    bool _isFixedRotation;
    /** Whether the event represents a bullet */
    bool _isBullet;
    /** Whether the event represents a sensor */
    bool _isSensor;
    
    // Fields for EventType::FLOAT_CONSTS
    /** The density of the obstacle in this event */
    float _density;
    /** The friction of the obstacle in this event */
    float _friction;
    /** The restitution of the obstacle in this event */
    float _restitution;
    /** The linear damping of the obstacle in this event */
    float _linearDamping;
    /** The angular damping of the obstacle in this event */
    float _angularDamping;
    /** The gravity scale of the obstacle in this event */
    float _gravityScale;
    /** The mass of the obstacle in this event */
    float _mass;
    /** The inertia of the obstacle in this event */
    float _inertia;
    /** The centroid of the obstacle in this event */
    Vec2 _centroid;
    
    /** The field for OBJ_BODY_TYPE */
    b2BodyType _bodyType;
    
    /** The field for OBJ_OWNER_ACQUIRE */
    Uint64 _duration;
    
    /** A serializer for packing data */
    LWSerializer _serializer;
    /** A deserializer for unpacking data */
    LWDeserializer _deserializer;
    
#pragma mark Attributes
public:
    /**
     * Returns the type of this event.
     *
     * @return the type of this event.
     */
    EventType getType() const { return _type; }
    
    /**
     * Returns the obstacle global id of this event.
     *
     * @return the obstacle global id of this event.
     */
    Uint64 getObstacleId() const { return _obstacleId; }
    
    /**
     * Returns the obstacle factory id of this event.
     *
     * This only valid for {@link EventType#CREATION} events.
     *
     * @return the obstacle factory id of this event.
     */
    Uint32 getFactoryId() const { return _factoryId; }
    
    /**
     * Returns the packed parameters for creating the obstacle.
     *
     * This only valid for {@link EventType#CREATION} events.
     *
     * @return the packed parameters for creating the obstacle.
     */
    const std::shared_ptr<std::vector<std::byte>> getPackedParam() const {
        return _packedParam;
    }
    
#pragma mark Event Creation
    /**
     * Initializes an empty event as {@link EventType#CREATION}.
     *
     * This event symbolizes the creation of an obstacle.
     *
     * @param factoryId     The obstacle factory id
     * @param obsId         The obstacle global id
     * @param packedParam   The packed parameters for the obstacle
     */
    void initCreation(Uint32 factoryId, Uint64 obsId,
                      std::shared_ptr<std::vector<std::byte>> packedParam) {
        _type = EventType::CREATION;
        _factoryId  = factoryId;
        _obstacleId = obsId;
        _packedParam = packedParam;
    }
    
    /**
     * Initializes an empty event to {@link EventType#DELETION}.
     *
     * This event symbolizes the deletion of an obstacle.
     *
     * @param obsId The obstacle global id
     */
    void initDeletion(Uint64 obsId) {
        _type = EventType::DELETION;
        _obstacleId = obsId;
    }
    
    /**
     * Initializes an empty event to {@link EventType#POSITION}.
     *
     * This event symbolizes a change in the position of an obstacle.
     *
     * @param obsId The obstacle global id
     * @param pos   The obstacle position
     */
    void initPos(Uint64 obsId, Vec2 pos) {
        _type = EventType::POSITION;
        _obstacleId = obsId;
        _pos = pos;
    }
    
    /**
     * Initializes an empty event to {@link EventType#VELOCITY}.
     *
     * This event symbolizes a change in the velocity of an obstacle.
     *
     * @param obsId The obstacle global id
     * @param vel   The obstacle velocity
     */
    void initVel(Uint64 obsId, Vec2 vel) {
        _type = EventType::VELOCITY;
        _obstacleId = obsId;
        _vel = vel;
    }
    
    /**
     * Initializes an empty event to {@link EventType#ANGLE}.
     *
     * This event symbolizes a change in the angle of an obstacle.
     *
     * @param obsId The obstacle global id
     * @param angle The obstacle angle
     */
    void initAngle(Uint64 obsId, float angle) {
        _type = EventType::ANGLE;
        _obstacleId = obsId;
        _angle = angle;
    }
    
    /**
     * Initializes an empty event to {@link EventType#ANGULAR_VEL}.
     *
     * This event symbolizes a change in the anglular velocity of an obstacle.
     *
     * @param obsId         The obstacle global id
     * @param angularVel    The angular velocity
     */
    void initAngularVel(Uint64 obsId, float angularVel) {
        _type = EventType::ANGULAR_VEL;
        _obstacleId = obsId;
        _angularVel = angularVel;
    }
    
    /**
     * Initializes an empty event to {@link EventType#BODY_TYPE}.
     *
     * This event symbolizes a change in the body type of an obstacle.
     *
     * @param obsId     The obstacle global id
     * @param bodyType  The body type
     */
    void initBodyType(Uint64 obsId, b2BodyType bodyType) {
        _type = EventType::BODY_TYPE;
        _obstacleId = obsId;
        _bodyType = bodyType;
    }
    
    /**
     * Initializes an empty event to {@link EventType#BOOL_CONSTS}.
     *
     * This event symbolizes a change in the boolean constants of an obstacle.
     * Due to the relatively rarer use of these constants, they are packed
     * into a single event.
     *
     * @param obsId     The obstacle global id
     * @param values    The boolean constants
     */
    void initBoolConsts(Uint64 obsId, const BoolConsts& values) {
        _type = EventType::BOOL_CONSTS;
        _obstacleId = obsId;
        _isEnabled = values.isEnabled;
        _isAwake = values.isAwake;
        _isSleepingAllowed = values.isSleepingAllowed;
        _isFixedRotation = values.isFixedRotation;
        _isBullet = values.isBullet;
        _isSensor = values.isSensor;
    }
    
    /**
     * Initializes an empty event to {@link EventType::FLOAT_CONSTS}.
     *
     * This event symbolizes a change in the additional float constants of an
     * obstacle. Due to the relatively rarer use of these constants, they are
     * packed into a single event.
     *
     * @param obsId     The obstacle global id
     * @param values    The float constants
     */
    void initFloatConsts(Uint64 obsId, const FloatConsts& values) {
        _type = EventType::FLOAT_CONSTS;
        _obstacleId = obsId;
        _density = values.density;
        _friction = values.friction;
        _restitution = values.restitution;
        _linearDamping = values.linearDamping;
        _angularDamping = values.angularDamping;
        _gravityScale = values.gravityScale;
        _mass = values.mass;
        _inertia = values.inertia;
        _centroid = values.centroid;
    }
    
    /**
     * Initializes an empty event to {@link EventType::OWNER_ACQUIRE}.
     *
     * This event symbolizes a change in obstacle ownership. Setting duration
     * to 0 will acquire ownership permanently.
     *
     * @param obsId     The obstacle global id
     * @param duration  The length of time to acquire the obstacle.
     */
    void initOwnerAcquire(Uint64 obsId, Uint64 duration){
        _type = EventType::OWNER_ACQUIRE;
        _obstacleId = obsId;
        _duration = duration;
    }
    
    /**
     * Initializes an empty event to {@link EventType::OWNER_RELEASE}.
     *
     * This event symbolizes a change in obstacle ownership.
     *
     * @param obsId     The obstacle global id
     */
    void initOwnerRelease(Uint64 obsId){
        _type = EventType::OWNER_RELEASE;
        _obstacleId = obsId;
    }
    
    
#pragma Event Allocators
    /**
     * Returns a newly allocated event of this type.
     *
     * This method is used by the NetEventController to create a new event with
     * this type as a reference.
     
     * Note that this method is not static, unlike the alloc method present
     * in most of CUGL. That is because we need this factory method to be
     * polymorphic. All custom subclasses must implement this method.
     *
     * @return a newly allocated event of this type.
     */
    std::shared_ptr<NetEvent> newEvent() override {
        return std::make_shared<PhysObstEvent>();
    }
    
    /**
     * Returns a newly created {@link EventType::CREATION} event.
     *
     * This method is a shortcut for creating a shared object on
     * {@link #initCreation}.
     *
     * @param factoryId     The obstacle factory id
     * @param obsId         The obstacle global id
     * @param packedParam   The packed parameters for the obstacle
     *
     * @return a newly created {@link EventType::CREATION} event.
     */
    static std::shared_ptr<PhysObstEvent> allocCreation(Uint32 factoryId, Uint64 obsId,
                                                       std::shared_ptr<std::vector<std::byte>> packedParam) {
        auto e = std::make_shared<PhysObstEvent>();
        e->initCreation(factoryId, obsId, packedParam);
        return e;
    }
    
    /**
     * Returns a newly created {@link EventType::DELETION} event.
     *
     * This method is a shortcut for creating a shared object on
     * {@link #initDeletion}.
     *
     * @param obsId The obstacle global id
     *
     * @return a newly created {@link EventType::DELETION} event.
     */
    static std::shared_ptr<PhysObstEvent> allocDeletion(Uint64 obsId) {
        auto e = std::make_shared<PhysObstEvent>();
        e->initDeletion(obsId);
        return e;
    }
    
    /**
     * Returns a newly created {@link EventType::POSITION} event.
     *
     * This method is a shortcut for creating a shared object on {@link #initPos}.
     *
     * @param obsId The obstacle global id
     * @param pos   The obstacle position
     *
     * @return a newly created {@link EventType::POSITION} event.
     */
    static std::shared_ptr<PhysObstEvent> allocPos(Uint64 obsId, Vec2 pos) {
        auto e = std::make_shared<PhysObstEvent>();
        e->initPos(obsId, pos);
        return e;
    }
    
    /**
     * Returns a newly created {@link EventType::VELOCITY} event.
     *
     * This method is a shortcut for creating a shared object on {@link #initVel}.
     *
     * @param obsId The obstacle global id
     * @param vel   The obstacle velocity
     *
     * @return a newly created {@link EventType::VELOCITY} event.
     */
    static std::shared_ptr<PhysObstEvent> allocVel(Uint64 obsId, Vec2 vel) {
        auto e = std::make_shared<PhysObstEvent>();
        e->initVel(obsId, vel);
        return e;
    }
    
    /**
     * Returns a newly created {@link EventType::ANGLE} event.
     *
     * This method is a shortcut for creating a shared object on
     * {@link #initAngle}.
     *
     * @param obsId The obstacle global id
     * @param angle The obstacle angle
     *
     * @return a newly created {@link EventType::ANGLE} event.
     */
    static std::shared_ptr<PhysObstEvent> allocAngle(Uint64 obsId, float angle) {
        auto e = std::make_shared<PhysObstEvent>();
        e->initAngle(obsId, angle);
        return e;
    }
    
    /**
     * Returns a newly created {@link EventType::ANGULAR_VEL} event.
     *
     * This method is a shortcut for creating a shared object on
     * {@link #initAngularVel}.
     *
     * @param obsId         The obstacle global id
     * @param angularVel    The anglular velocity
     *
     * @return a newly created {@link EventType::ANGULAR_VEL} event.
     */
    static std::shared_ptr<PhysObstEvent> allocAngularVel(Uint64 obsId, float angularVel) {
        auto e = std::make_shared<PhysObstEvent>();
        e->initAngularVel(obsId, angularVel);
        return e;
    }

    /**
     * Returns a newly created {@link EventType::BODY_TYPE} event.
     *
     * This method is a shortcut for creating a shared object on
     * {@link #initBodyType}.
     *
     * @param obsId     The obstacle global id
     * @param bodyType  The obstacle body type
     *
     * @return a newly created {@link EventType::BODY_TYPE} event.
     */
    static std::shared_ptr<PhysObstEvent> allocBodyType(Uint64 obsId, b2BodyType bodyType) {
        auto e = std::make_shared<PhysObstEvent>();
        e->initBodyType(obsId, bodyType);
        return e;
    }

    /**
     * Returns a newly created {@link EventType::BOOL_CONSTS} event.
     *
     * This method is a shortcut for creating a shared object on
     * {@link #initBoolConsts}.
     *
     * @param obsId     The obstacle global id
     * @param values    The boolean constants
     *
     * @return a newly created {@link EventType::BOOL_CONSTS} event.
     */
    static std::shared_ptr<PhysObstEvent> allocBoolConsts(Uint64 obsId, const BoolConsts& values) {
        auto e = std::make_shared<PhysObstEvent>();
        e->initBoolConsts(obsId, values);
        return e;
    }
    
    /**
     * Returns a newly created {@link EventType::FLOAT_CONSTS} event.
     *
     * This method is a shortcut for creating a shared object on
     * {@link #initFloatConsts}.
     *
     * @param obsId     The obstacle global id
     * @param values    The boolean constants
     *
     * @return a newly created {@link EventType::FLOAT_CONSTS} event.
     */
    static std::shared_ptr<PhysObstEvent> allocFloatConsts(Uint64 obsId, const FloatConsts& values) {
        auto e = std::make_shared<PhysObstEvent>();
        e->initFloatConsts(obsId, values);
        return e;
    }
    
    /**
     * Returns a newly created {@link EventType::OWNER_ACQUIRE} event.
     *
     * This method is a shortcut for creating a shared object on
     * {@link #initOwnerAcquire}.
     *
     * @param obsId     The obstacle global id
     * @param duration  The length of time to acquire the obstacle.
     *
     * @return a newly created {@link EventType::OWNER_ACQUIRE} event.
     */
    static std::shared_ptr<PhysObstEvent> allocOwnerAcquire(Uint64 obsId, Uint64 duration){
        auto e = std::make_shared<PhysObstEvent>();
        e->initOwnerAcquire(obsId,duration);
        return e;
    }

    /**
     * Returns a newly created {@link EventType::OWNER_RELEASE} event.
     *
     * This method is a shortcut for creating a shared object on
     * {@link #initOwnerRelease}.
     *
     * @param obsId     The obstacle global id
     *
     * @return a newly created {@link EventType::OWNER_RELEASE} event.
     */
    static std::shared_ptr<PhysObstEvent> allocOwnerRelease(Uint64 obsId){
        auto e = std::make_shared<PhysObstEvent>();
        e->initOwnerRelease(obsId);
        return e;
    }

#pragma mark Attributes
    /**
     * Returns the body type for this physics event
     *
     * @return the body type for this physics event
     */
    const b2BodyType getBodyType() const { return _bodyType; }

    /**
     * Returns the position for this physics event
     *
     * @return the position for this physics event
     */
    const Vec2 getPosition() const { return _pos; }
        
    /**
     * Returns the linear velocity for this physics event
     *
     * @return the linear velocity for this physics event
     */
    const Vec2 getLinearVelocity() const { return _vel; }
    
    /**
     * Returns the angle for this physics event
     *
     * The value returned is in radians
     *
     * @return the angle for this physics event
     */
    float getAngle() const { return _angle; }
    
    /**
     * Returns the angular velocity for this physics event
     *
     * @return the angular velocity for this physics event
     */
    const float getAngularVelocity() const { return _angularVel; }
    
    /**
     * Returns true if the obstacle in this event is enabled
     *
     * @return true if the obstacle in this event is enabled
     */
    bool isEnabled() const { return _isEnabled; }
    
    /**
     * Returns true if the obstacle in this event is awake
     *
     * @return true if the obstacle in this event is awake
     */
    bool isAwake() const { return _isAwake; }
    
    /**
     * Returns false if the obstacle in this event should never fall asleep
     *
     * @return false if the obstacle in this event should never fall asleep
     */
    bool isSleepingAllowed() const { return _isSleepingAllowed; }
    
    /**
     * Returns true if the obstacle in this event is a bullet
     *
     * @return true if the obstacle in this event is a bullet
     */
    bool isBullet() const { return _isBullet; }
    
    /**
     * Returns true if the obstacle in this event is prevented from rotating
     *
     * @return true if the obstacle in this event is prevented from rotating
     */
    bool isFixedRotation() const { return _isFixedRotation; }
        
    /**
     * Returns the gravity scale to apply to this physics event
     *
     * @return the gravity scale to apply to this physics event
     */
    float getGravityScale() const { return _gravityScale; }
    
    /**
     * Returns the linear damping for this physics event.
     *
     * @return the linear damping for this physics event.
     */
    float getLinearDamping() const { return _linearDamping; }
    
    /**
     * Returns the angular damping for this physics event.
     *
     * @return the angular damping for this physics event.
     */
    float getAngularDamping() const { return _angularDamping; }
    
    /**
     * Returns the density of this physics event.
     *
     * The density is typically measured in usually in kg/m^2. The density can
     * be zero or positive. You should generally use similar densities for all
     * your fixtures. This will improve stacking stability.
     *
     * @return the density of this physics event.
     */
    float getDensity() const { return _density; }
    
    /**
     * Returns the friction coefficient of this physics event.
     *
     * The friction parameter is usually set between 0 and 1, but can be any
     * non-negative value. A friction value of 0 turns off friction and a value
     * of 1 makes the friction strong. When the friction force is computed
     * between two shapes, Box2D must combine the friction parameters of the
     * two parent fixtures. This is done with the geometric mean.
     *
     * @return the friction coefficient of this physics event.
     */
    float getFriction() const { return _friction; }
    
    /**
     * Returns the restitution of this physics event.
     *
     * Restitution is used to make objects bounce. The restitution value is
     * usually set to be between 0 and 1. Consider dropping a ball on a table.
     * A value of zero means the ball won't bounce. This is called an inelastic
     * collision. A value of one means the ball's velocity will be exactly
     * reflected. This is called a perfectly elastic collision.
     *
     * @return the restitution of this physics event.
     */
    float getRestitution() const { return _restitution; }
    
    /**
     * Returns true if the obstacle in this event is a sensor.
     *
     * Sometimes game logic needs to know when two entities overlap yet there
     * should be no collision response. This is done by using sensors. A sensor
     * is an entity that detects collision but does not produce a response.
     *
     * @return true if the obstacle in this event is a sensor.
     */
    bool isSensor() const { return _isSensor; }
    
    /**
     * Returns the center of mass of this physics event.
     *
     * @return the center of mass of this physics event.
     */
    const Vec2 getCentroid() const { return _centroid; }
    
    /**
     * Returns the rotational inertia of this physics event.
     *
     * @return the rotational inertia of this physics event.
     */
    float getInertia() const { return _inertia; }
    
    /**
     * Returns the mass of this physics event.
     *
     * @return the mass of this physics event.
     */
    float getMass() const { return _mass; }

#pragma mark Serialization/Deserialization
    /**
     * Returns a byte vector serializing this event
     *
     * @return a byte vector serializing this event
     */
    std::vector<std::byte> serialize() override;
    
    /**
     * Deserializes this event from a byte vector.
     *
     * This method will set the type of the event and all relevant fields.
     */
    void deserialize(const std::vector<std::byte>& data) override;
    
};

        }
    }
}

#endif /* __CU_PHYS_OBJ_EVENT_H__ */
