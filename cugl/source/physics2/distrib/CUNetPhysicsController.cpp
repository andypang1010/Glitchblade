//
//  CUNetPhysicsController.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a physics controller for the networked physics library.
//  It is responsible for all synchronization and object management across
//  shared physics worlds.
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
#include <cugl/physics2/distrib/CUNetWorld.h>
#include <cugl/physics2/distrib/CUNetPhysicsController.h>
#include <cugl/physics2/distrib/CULWSerializer.h>


using namespace cugl;
using namespace cugl::physics2;
using namespace cugl::physics2::distrib;


#pragma mark -
#pragma mark Constructors

/** Creates a parameter set with default values */
NetPhysicsController::TargetParams::TargetParams() {
    curStep = 0;
    numSteps = 0;
    targetAngle = 0;
    targetAngV = 0;
    numI = 0;
}

/**
 * Creates a degenerate physics controller with default values.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an asset on
 * the heap, use one of the static constructors instead.
 */
NetPhysicsController::NetPhysicsController() :
_itprMethod(0),
_itprDebug(false),
_itprCount(0),
_ovrdCount(0),
_stepSum(0),
_objRotation(0),
_isHost(false) {
}


/**
 * Initializes a new physics controller with the given values.
 *
 * If the function linkFunc is provided, the controller will automatically
 * link newly added obstacles to their corresponding scene nodes by calling
 * this function. The controller will also handle removal of scenes nodes
 * when removing obstacles.
 *
 * On the other hand, if linkFunc is nullptr, the user will be responsible
 * for linking obstacles to scene nodes. In that case, the user is
 * recommended to use custom NetEvent types to handle obstacle creation
 * without the use of the physics controller.
 *
 * @param world     The physics world instance to manage
 * @param shortUID   The shortUID assigned by NetEventController
 * @param linkFunc  Function for linking obstacle to scene node
 *
 * @return true if the controller was initialized successfully
 */
bool NetPhysicsController::init(std::shared_ptr<NetWorld>& world, Uint32 shortUID,
                                bool isHost, ObstacleLink linkFunc) {
    _world = world;
    // TODO: WHY IS THIS COMMENTED OUT
    //_world->setshortUID(shortUID);
    _linkSceneToObsFunc = linkFunc;
    _isHost = isHost;
    return true;
}

/**
 * Disposes the physics controller, releasing all resources.
 *
 * This controller can be safely reinitialized
 */
void NetPhysicsController::dispose() {
    reset();
    _obstacleFacts.clear();
    _sharedObsToNodeMap.clear();
    _world = nullptr;
    _isHost = false;
    _linkSceneToObsFunc = nullptr;
}

#pragma mark Object Management
/**
 * Adds a shared obstacle to the physics world.
 *
 * This method is used to add a shared obstacle across all clients. Users
 * can uses the returned references to manually link the obstacles to
 * scene graphs, or for custom obstacle setups.
 *
 * @param factoryID The id of the obstacle factory to use
 * @param bytes     The serialized parameters taken by the obstacle factory
 *
 * @return A pair of the added obstacle and its corresponding scene node
 */
ObstacleScene NetPhysicsController::addSharedObstacle(Uint32 factoryID,
                                                      std::shared_ptr<std::vector<std::byte>> bytes) {
    CUAssertLog(factoryID < _obstacleFacts.size(), "Unknown object Factory %u", factoryID);
    auto pair = _obstacleFacts[factoryID]->createObstacle(*bytes);
    pair.first->setShared(true);
    Uint64 objId = _world->placeObstacle(pair.first);
    if (_isHost){
        _world->getOwnedObstacles().insert(std::make_pair(pair.first,0));
    }
    if (_linkSceneToObsFunc) {
        _linkSceneToObsFunc(pair.first, pair.second);
    }
    _outEvents.push_back(PhysObstEvent::allocCreation(factoryID,objId,bytes));
    return pair;
}

/**
 * Removes a shared obstacle from the physics world.
 *
 * If a linking function was provided, the scene node will also be removed.
 *
 * @param obs   the obstacle to remove
 */
void NetPhysicsController::removeSharedObstacle(std::shared_ptr<physics2::Obstacle> obj) {
    auto map = _world->getObstacleIds();
    if (map.count(obj)) {
        Uint64 objId = map.at(obj);
        _outEvents.push_back(PhysObstEvent::allocDeletion(objId));
        _world->removeObstacle(obj);
        if (_sharedObsToNodeMap.count(obj)) {
            _sharedObsToNodeMap.at(obj)->removeFromParent();
            _sharedObsToNodeMap.erase(obj);
        }
    }
}

/**
 * Acquires the ownership of the object for an amount of time
 *
 * This method is used for one client to obtain ownership of the obstacle.
 * This ownership can be temporary, measured in terms of physics steps.
 * If the duration is 0, ownership will last until it is released. When
 * this method is called by the host, the duration is permenant.
 *
 * Normally, the host would own all objects upon their creation. This
 * method allows any client to be the owner of an obstacle, therefore
 * potentially reducing response time for client controlled objects.
 *
 * WARNING: only one client should call this method on an object within
 * a period of time to avoid race conditions.
 *
 * @param obs       the obstacle to acquire
 * @param duration  number of physics steps to hold ownership
 */
void NetPhysicsController::acquireObs(std::shared_ptr<physics2::Obstacle> obs, Uint64 duration){
    auto owned = _world->getOwnedObstacles();
    if(_isHost){
        owned.insert(std::make_pair(obs, 0));
    } else {
        owned.insert(std::make_pair(obs,duration));
    }
    Uint64 id = _world->getObstacleIds().at(obs);
    auto event = PhysObstEvent::allocOwnerAcquire(id, duration);
    _outEvents.push_back(event);
}

/**
 * Releases the ownership of the object.
 *
 * This method is the opposite of {@link #acquireObs}. When called by a
 * client, it will return ownership to the host. This method has no effect
 * if the client does not have ownership of that obstacle or if it is
 * called by the host.
 *
 * @param obs       the obstacle to release
 */
void NetPhysicsController::releaseObs(std::shared_ptr<physics2::Obstacle> obs) {
    if (!_isHost) {
        _world->getOwnedObstacles().erase(obs);
        Uint64 id = _world->getObstacleIds().at(obs);
        auto event = PhysObstEvent::allocOwnerRelease(id);
        _outEvents.push_back(event);
    }
}

/**
 * Makes this client the owner of all objects in the simulation.
 *
 * This method does not actually send any information to the other
 * clients on the network.  It should be used for initial objects only.
 */
void NetPhysicsController::ownAll() {
    auto obstacles = _world->getObstacles();
    auto ownership = _world->getOwnedObstacles();
    // TODO: This really needs a better method
    for(auto it = obstacles.begin(); it != obstacles.end(); ++it){
        ownership.insert(std::make_pair((*it), 0));
    }
}

/**
 * Adds an object to interpolate with the given target parameters.
 *
 * This method is used for error smoothing.
 *
 * @param obj   The obstacle to interpolate
 * @param param The target parameters for interpolation
 */
void NetPhysicsController::addSyncObject(std::shared_ptr<physics2::Obstacle> obj,
                                         const std::shared_ptr<TargetParams>& param) {
    if (_itprMethod == 1) {
        return;
    } else if (_cache.count(obj)) {
        auto oldParam = _cache.at(obj);
        obj->setShared(false);
        // ===== BEGIN NON-SHARED BLOCK =====
        obj->setLinearVelocity(oldParam->targetVel);
        obj->setAngularVelocity(oldParam->targetAngV);
        // ====== END NON-SHARED BLOCK ======
        obj->setShared(true);
        param->I = oldParam->I;
        param->numI = oldParam->numI;
    }
    _cache.erase(obj);
    _cache.insert(std::make_pair(obj,param));
    _stepSum += param->numSteps;
    _itprCount++;
}

/**
 * Returns the result of linear object interpolation.
 *
 * Formula: (target-source)/stepsLeft + source
 *
 * @return the result of linear object interpolation.
 */
float NetPhysicsController::interpolate(int stepsLeft, float target, float source) {
    return (target-source)/stepsLeft+source;
}

#pragma mark Synchronization
/**
 * Updates the physics controller.
 */
void NetPhysicsController::updateSimulation() {
    packPhysObj();
    
    // Ownership transfer
    std::vector<std::shared_ptr<cugl::physics2::Obstacle>> deleteCache;
    auto ownership = _world->getOwnedObstacles();
    for(auto it = _world->getObstacles().begin() ; it != _world->getObstacles().end(); ++it) {
        if (ownership.count(*it)){
            Uint64 left = ownership.at(*it);
            if (left==1){
                releaseObs(*it);
            } else if (left>1) {
                ownership[*it]=left-1;
            }
        }
    }
    for(auto it = deleteCache.begin(); it != deleteCache.end(); ++it){
        ownership.erase((*it));
    }

    for(auto it = _cache.begin(); it != _cache.end(); it++){
        auto obj = it->first;
        if(!obj->isShared()){
            _deleteCache.push_back(it->first);
            continue;
        }
        obj->setShared(false);
        // ===== BEGIN NON-SHARED BLOCK =====
        std::shared_ptr<TargetParams> param = it->second;
        int stepsLeft = param->numSteps-param->curStep;
        
        if (stepsLeft <= 1){
            obj->setPosition(param->P3);
            obj->setLinearVelocity(param->targetVel);
            obj->setAngle(param->targetAngle);
            obj->setAngularVelocity(param->targetAngV);
            _deleteCache.push_back(it->first);
            _ovrdCount++;
        } else{
            float t = ((float)param->curStep)/param->numSteps;
            CUAssert(t<=1.f && t>=0.f);
            
            switch (_itprMethod) {
                case 1:
                {
                    Vec2 P1 = obj->getPosition()+obj->getLinearVelocity()/10.f;
                    Vec2 pos = (1-t)*(1-t)*(1-t)*obj->getPosition() + 3*(1-t)*(1-t)*t*P1 + 3*(1-t)*t*t*param->P2 + t*t*t*param->P3;
                    obj->setPosition(pos);
                }
                    break;
                case 2:
                {
                    Vec2 pos = (2*t*t*t-3*t*t+1)*obj->getPosition() +
                    (t*t*t-2*t*t+t)*obj->getLinearVelocity() +
                    (-2*t*t*t+3*t*t)*param->P3 +
                    (t*t*t-t*t)*param->targetVel;
                    obj->setPosition(pos);
                }
                    break;
                case 3:
                {
                    Vec2 E = param->P3-obj->getPosition();
                    param->numI++;
                    param->I = param->I + E;
                    
                    Vec2 P = E*10.f;
                    Vec2 I = param->I*0.01f;
                    Vec2 D = obj->getLinearVelocity()*0.5f;
                    obj->setLinearVelocity(obj->getLinearVelocity()+P-D+I);
                }
                    break;
                default:
                    obj->setX(interpolate(stepsLeft,param->P3.x,obj->getX()));
                    obj->setY(interpolate(stepsLeft,param->P3.y,obj->getY()));
                    obj->setVX(interpolate(stepsLeft, param->targetVel.x, obj->getVX()));
                    obj->setVY(interpolate(stepsLeft, param->targetVel.y, obj->getVY()));
                    break;
            }
            
            obj->setAngle(interpolate(stepsLeft, param->targetAngle, obj->getAngle()));
            obj->setAngularVelocity(interpolate(stepsLeft, param->targetAngV, obj->getAngularVelocity()));
        }
        param->curStep++;
        // ====== END NON-SHARED BLOCK ======
        obj->setShared(true);
    }

    for(auto it = _deleteCache.begin(); it != _deleteCache.end(); it++){
        _cache.erase(*it);
    }
    _deleteCache.clear();

    if (_itprDebug) {
        CULog("%ld/%ld overriden", _itprCount-_ovrdCount,_itprCount);
        CULog("Average step: %f", ((float)_stepSum)/_itprCount);
    }
}

/**
 * Processes a physics object synchronization event.
 *
 * This method is called automatically by the NetEventController.
 *
 * @param event The event to be processed
 */
void NetPhysicsController::processPhysObstEvent(const std::shared_ptr<PhysObstEvent>&event) {
    if (event->getSourceId() == "")
        return; // Ignore physic syncs from self.

    if (event->getType() == PhysObstEvent::EventType::CREATION) {
        CUAssertLog(event->getFactoryId() < _obstacleFacts.size(), "Unknown object Factory %u",
                    event->getFactoryId());
        auto pair = _obstacleFacts[event->getFactoryId()]->createObstacle(*event->getPackedParam());
        _world->activateObstacle(event->getObstacleId(),pair.first);
        if (_linkSceneToObsFunc) {
            _linkSceneToObsFunc(pair.first, pair.second);
            _sharedObsToNodeMap.insert(std::make_pair(pair.first, pair.second));
        }
        if(_isHost){
            _world->getOwnedObstacles().insert({pair.first,0});
        }
        return;
    }

    // Ignore event if object is not found.
    // TODO: Send request to object owner to sync object.
    auto obj = _world->getObstacle(event->getObstacleId());
    if (obj == nullptr) {
        return;
    }

    if (event->getType() == PhysObstEvent::EventType::DELETION) {
        _cache.erase(obj);
        _world->removeObstacle(obj);
        if (_sharedObsToNodeMap.count(obj)) {
            _sharedObsToNodeMap.at(obj)->removeFromParent();
            _sharedObsToNodeMap.erase(obj);
        }
        return;
    }

    obj->setShared(false);
    // ===== BEGIN NON-SHARED BLOCK =====
    switch (event->getType()) {
        case PhysObstEvent::EventType::BODY_TYPE:
			obj->setBodyType(event->getBodyType());
			break;
        case PhysObstEvent::EventType::POSITION:
            obj->setPosition(event->getPosition());
            break;
        case PhysObstEvent::EventType::VELOCITY:
            obj->setLinearVelocity(event->getLinearVelocity());
            break;
        case PhysObstEvent::EventType::ANGLE:
			obj->setAngle(event->getAngle());
			break;
        case PhysObstEvent::EventType::ANGULAR_VEL:
            obj->setAngularVelocity(event->getAngularVelocity());
            break;
        case PhysObstEvent::EventType::BOOL_CONSTS:
            if (event->isEnabled() != obj->isEnabled()) {
                obj->setEnabled(event->isEnabled());
            }
            if (event->isAwake() != obj->isAwake()) {
                obj->setAwake(event->isAwake());
            }
            if (event->isSleepingAllowed() != obj->isSleepingAllowed()) { obj->setSleepingAllowed(event->isSleepingAllowed());
            }
            if (event->isFixedRotation() != obj->isFixedRotation()) { obj->setFixedRotation(event->isFixedRotation());
            }
            if (event->isBullet() != obj->isBullet()) {
                obj->setBullet(event->isBullet());
            }
            if (event->isSensor() != obj->isSensor()) {
                obj->setSensor(event->isSensor());
            }
			break;
        case PhysObstEvent::EventType::FLOAT_CONSTS:
            if (event->getDensity() != obj->getDensity()) {
                obj->setDensity(event->getDensity());
            }
            if (event->getFriction() != obj->getFriction()) {
                obj->setFriction(event->getFriction());
            }
            if (event->getRestitution() != obj->getRestitution()) {
                obj->setRestitution(event->getRestitution());
            }
            if (event->getLinearDamping() != obj->getLinearDamping()) {
                obj->setLinearDamping(event->getLinearDamping());
            }
            if (event->getGravityScale() != obj->getGravityScale()) {
                obj->setGravityScale(event->getGravityScale());
            }
            if (event->getMass() != obj->getMass()) {
                obj->setMass(event->getMass());
            }
            if (event->getInertia() != obj->getInertia()) {
                obj->setInertia(event->getInertia());
            }
            if (event->getCentroid() != obj->getCentroid()) {
                obj->setCentroid(event->getCentroid());
            }
            break;
        case PhysObstEvent::EventType::OWNER_ACQUIRE:
            _world->getOwnedObstacles().erase(obj);
            //CULog("Erased ownership for %llu",event->getObjId());
            break;
        case PhysObstEvent::EventType::OWNER_RELEASE:
            if (_isHost) {
                _world->getOwnedObstacles().insert(std::make_pair(obj,0));
                //CULog("Regained ownership for %llu",event->getObjId());
            }
        default:
            break;
	}
    // ====== END NON-SHARED BLOCK ======
    obj->setShared(true);
}

/**
 * Processes a physics synchronization event.
 */
void NetPhysicsController::processPhysSyncEvent(const std::shared_ptr<PhysSyncEvent>& event) {
    if (event->getSourceId() == "") {
        return; // Ignore physic syncs from self.
    }
    const std::vector<PhysSyncEvent::Parameters>& params = event->getSyncList();
    for (auto it = params.begin(); it != params.end(); it++) {
        PhysSyncEvent::Parameters param = (*it);
        
        auto obj = _world->getObstacle(param.obsId);
        if (obj == nullptr) {
            //CUAssertLog(obs, "Invalid PhysSyncEvent, obj %llu not found.",param.obsId);
            // Ugh
            continue;
        }
            
        float x = param.x;
        float y = param.y;
        float angle = param.angle;
        float vAngular = param.vAngular;
        float vx = param.vx;
        float vy = param.vy;
        float diff = (obj->getPosition() - Vec2(x, y)).length();
        float angDiff = 10 * abs(obj->getAngle() - angle);
            
        int steps = SDL_max(1, SDL_min(30, SDL_max((int)(diff * 30), (int)angDiff)));

        std::shared_ptr<TargetParams> target = std::make_shared<TargetParams>();
        target->targetVel = Vec2(vx, vy);
        target->targetAngle = angle;
        target->targetAngV = vAngular;
        target->curStep = 0;
        target->numSteps = steps;
        target->P0 = obj->getPosition();
        target->P1 = obj->getPosition() + obj->getLinearVelocity() / 10.f;
        target->P3 = Vec2(x, y);
        target->P2 = target->P3 - target->targetVel / 10.f;

        addSyncObject(obj, target);
    }
}

/**
 * Packs object data for synchronization.
 *
 * This data will be added to {@link #getOutEvents}, which is the queue
 * of information to be sent over the network.
 *
 * This method can be used to prompt the physics controller to synchronize
 * objects. It is called automatically by {@link NetEventController}, but
 * additional calls to it can help fix potential desyncing.
 *
 * @param type  the type of synchronization
 */
void NetPhysicsController::packPhysSync(SyncType type) {
    auto event = PhysSyncEvent::alloc();
    
    switch (type) {
        case SyncType::OVERRIDE_FULL_SYNC:
            for (auto it = _world->getObstacleMap().begin(); it != _world->getObstacleMap().end(); it++) {
                Uint64 id = (*it).first;
                auto obj = (*it).second;
                if (obj->isShared()) {
                    event->addObstacle(id,obj);
                }
            }
            break;
        case SyncType::FULL_SYNC:
        {
            auto ownership = _world->getOwnedObstacles();
            for (auto it = _world->getObstacleMap().begin(); it != _world->getObstacleMap().end(); it++) {
                Uint64 id = (*it).first;
                auto obj = (*it).second;
                if (obj->isShared() && ownership.count(obj))
                    event->addObstacle(id,obj);
            }
        }
            break;
        case SyncType::PRIO_SYNC:
        {
            std::vector<Uint64> velQueue;
            auto objmap = _world->getObstacleMap();
            for (auto it = objmap.begin(); it != objmap.end(); it++) {
                Uint64 id = (*it).first;
                auto obj = (*it).second;
                if(obj->isShared()) {
                    velQueue.push_back(id);
                }
            }
            
            std::sort(velQueue.begin(), velQueue.end(), [this](Uint64 const& l, Uint64 const& r) {
                auto omap = _world->getObstacleMap();
                return omap.at(l)->getLinearVelocity().length() > omap.at(r)->getLinearVelocity().length();
            });
            
            size_t defaultPri = 60;
            size_t numPrioObj = std::min(defaultPri,velQueue.size());
            
            
            for (size_t ii = 0; ii < numPrioObj; ii++) {
                auto obj = objmap.at(velQueue[ii]);
                if(obj->isShared()) {
                    event->addObstacle(velQueue[ii],obj);
                }
            }
            
            defaultPri = 20;
            for (size_t ii = 0; ii < std::min(defaultPri,velQueue.size()); ii++) {
                // Refactored as obstacles are no longer a vector
                auto obj = _world->getNextObstacle();
                event->addObstacle(_world->getObstacleId(obj),obj);
            }
        }
            break;
    }
    
    _outEvents.push_back(event);
}

/**
 * Packs any changed object information
 *
 * This method checks the world for any dirty objects (e.g. objects that
 * have changed state outside of the simulation). If so, it packages
 * that information as an event to send out to other machines on the
 * network.
 */

void NetPhysicsController::packPhysObj() {
    auto objs = _world->getObstacles();
    for (auto it = objs.begin(); it != objs.end(); it++) {
        auto obj = (*it);
        Uint64 id = _world->getObstacleIds().at(obj);
        if (obj->isShared()) {
            if (obj->hasDirtyPosition()) {
                _outEvents.push_back(PhysObstEvent::allocPos(id,obj->getPosition()));
            }
            if (obj->hasDirtyAngle()) {
				_outEvents.push_back(PhysObstEvent::allocAngle(id,obj->getAngle()));
			}
            if (obj->hasDirtyVelocity()) {
				_outEvents.push_back(PhysObstEvent::allocVel(id,obj->getLinearVelocity()));
			}
            if (obj->hasDirtyAngularVelocity()) {
				_outEvents.push_back(PhysObstEvent::allocAngularVel(id,obj->getAngularVelocity()));
			}
            if (obj->hasDirtyType()) {
                _outEvents.push_back(PhysObstEvent::allocBodyType(id,obj->getBodyType()));
            }
            if (obj->hasDirtyBool()) {
                PhysObstEvent::BoolConsts values;
                values.isEnabled = obj->isEnabled();
                values.isAwake = obj->isAwake();
                values.isSleepingAllowed = obj->isSleepingAllowed();
                values.isFixedRotation = obj->isFixedRotation();
                values.isBullet = obj->isBullet();
                values.isSensor = obj->isSensor();
				_outEvents.push_back(PhysObstEvent::allocBoolConsts(id,values));
			}
            if (obj->hasDirtyFloat()) {
                PhysObstEvent::FloatConsts values;
                values.density = obj->getDensity();
                values.friction = obj->getFriction();
                values.restitution = obj->getRestitution();
                values.linearDamping = obj->getLinearDamping();
                values.angularDamping = obj->getAngularDamping();
                values.gravityScale = obj->getGravityScale();
                values.mass = obj->getMass();
                values.inertia = obj->getInertia();
                values.centroid = obj->getCentroid();
                _outEvents.push_back(PhysObstEvent::allocFloatConsts(id,values));
            }
            obj->clearSharingDirtyBits();
        }
	}
}

/**
 * Resets the physics controller.
 */
void NetPhysicsController::reset() {
    _itprCount = 0;
    _ovrdCount = 0;
    _stepSum = 0;
    _cache.clear();
    _objRotation = 0;
    _deleteCache.clear();
    _outEvents.clear();
    _sharedObsToNodeMap.clear();
}
