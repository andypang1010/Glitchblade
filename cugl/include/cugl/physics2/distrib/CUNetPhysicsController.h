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
#ifndef __CU_NET_PHYSICS_CONTROLLER_H__
#define __CU_NET_PHYSICS_CONTROLLER_H__

// TODO: Some of these can be removed with forward declarations
#include <cugl/physics2/distrib/CUNetEvent.h>
#include <cugl/physics2/distrib/CUPhysObstEvent.h>
#include <cugl/physics2/distrib/CUPhysSyncEvent.h>
#include <cugl/physics2/distrib/CUGameStateEvent.h>
#include <cugl/physics2/distrib/CUObstacleFactory.h>
#include <queue>

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
 * Function for linking an obstacle to a specific scene node.
 *
 * This function should be provided by the user to support animations in
 * reaction to changes in the physics simulation.
 *
 * @param obs   The obstacle in the simulation
 * @param node  The scene graph node representing the obstacle
 */
typedef std::function<void(const std::shared_ptr<physics2::Obstacle>& obs,
                           const std::shared_ptr<scene2::SceneNode>& node)> ObstacleLink;
        
/**
 * This class is the physics controller for the networked physics library.
 * 
 * This class holds a reference to a {@link NetWorld} instance. It is built
 * on top of that class, and is responsible for all networked physics
 * synchronization and object management.
 */
class NetPhysicsController {
public:
    /**
     * The target target parameters for interpolation.
     *
     * These are used to smooth errors in the simulation.
     */
    class TargetParams {
    public:
        /** The current step of interpolation */
        int curStep;
        /** The total steps designated for interpolation */
        int numSteps;
        /** The source position */
        Vec2 P0;
        /** The first control point (for spline interpolation) */
        Vec2 P1;
        /** The second control point (for spline interpolation) */
        Vec2 P2;
        /** The target position */
        Vec2 P3;
        /** The target velocity */
        Vec2 targetVel;
        /** The target angle */
        float targetAngle;
        /** The target angular velocity */
        float targetAngV;
        /** The integral term sum For PID interpolation*/
        Vec2 I;
        /** The number of integral terms summed For PID interpolation*/
        Uint64 numI;
        
        /** Creates a parameter set with default values */
        TargetParams();
    };
    
    
    /**
     * The event types for physics synchronization.
     */
    enum class SyncType : int {
        /**
         * Synchronize all objects (shared or unshared) in the world/
         *
         * Objects that other clients do not recognize will be ignored.
         */
        OVERRIDE_FULL_SYNC,
        /** Synchronize all shared objects in the world */
        FULL_SYNC,
        /** Prioritize syncing volatile objects */
        PRIO_SYNC
    };
    
    
#pragma mark PhysicsController Stats
protected:
    /** The current interpolation method */
    Uint32 _itprMethod;
    /** Whether to display debug information for the interpolation */
    bool _itprDebug;
    /** Total number of interpolations done */
    long _itprCount;
    /** Total number of overriden interpolations */
    long _ovrdCount;
    /** Total number of steps interpolated */
    long _stepSum;
    /** Whether this instance acts as host. */
    bool _isHost;
    
    /** The next available obstacle ID */
    Uint64 _objRotation;
    /** The physics world instance */
    std::shared_ptr<NetWorld> _world;
    /** Cache of all on-ogoing interpolations */
    std::unordered_map<std::shared_ptr<physics2::Obstacle>,std::shared_ptr<TargetParams>> _cache;
    /** Temporary cache for removal after traversal */
    std::vector<std::shared_ptr<physics2::Obstacle>> _deleteCache;
    
    /** Vector of attached obstacle factories for obstacle creation */
    std::vector<std::shared_ptr<ObstacleFactory>> _obstacleFacts;
    /** Function for linking newly added obstacle to a scene node */
    ObstacleLink _linkSceneToObsFunc;
    /** Local map from added obstacles to scene nodes */
    std::unordered_map<std::shared_ptr<physics2::Obstacle>,
    std::shared_ptr<scene2::SceneNode>> _sharedObsToNodeMap;
    
    /** Vector of generated events to be sent */
    std::vector<std::shared_ptr<NetEvent>> _outEvents;
    
    /**
     * Returns the result of linear object interpolation.
     *
     * Formula: (target-source)/stepsLeft + source
     *
     * @return the result of linear object interpolation.
     */
    float interpolate(int stepsLeft, float target, float source);
    
    
#pragma mark Constructors
public:
    /**
     * Creates a degenerate physics controller with default values.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an asset on
     * the heap, use one of the static constructors instead.
     */
    NetPhysicsController();
    
    /**
     * Deletes this physics controller, disposing of all resources.
     */
    ~NetPhysicsController() {
        dispose();
    }
    
    /**
     * Disposes the physics controller, releasing all resources.
     *
     * This controller can be safely reinitialized
     */
    void dispose();
    
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
     * @param shortUID  The shortUID assigned by NetEventController
     * @param isHost    Whether this connection is the game host
     * @param linkFunc  Function for linking obstacle to scene node
     *
     * @return true if the controller was initialized successfully
     */
    bool init(std::shared_ptr<NetWorld>& world, Uint32 shortUID,
              bool isHost, ObstacleLink linkFunc=nullptr);
    
    /**
     * Returns a newly allocated physics controller with the given values.
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
     * @param shortUID  The shortUID assigned by NetEventController
     * @param isHost    Whether this connection is the game host
     * @param linkFunc  Function for linking obstacle to scene node
     *
     * @return a newly allocated physics controller with the given values.
     */
    static std::shared_ptr<NetPhysicsController> alloc(std::shared_ptr<NetWorld>& world, Uint32 shortUID,
                                                       bool isHost, ObstacleLink linkFunc=nullptr) {
        std::shared_ptr<NetPhysicsController> result = std::make_shared<NetPhysicsController>();
        return (result->init(world,shortUID,isHost,linkFunc) ? result : nullptr);
    };

#pragma mark Object Management
    /**
     * Add a custom obstacle factory to the controller.
     *
     * This method allows users to leverage automatic object synchronization
     * to add obstacles to the physics world. See {@link ObstacleFactory} for
     * how to implement a custom obstacle factory.
     *
     * @return The id of the added obstacle factory
     */
    Uint32 attachFactory(std::shared_ptr<ObstacleFactory> fact) {
        _obstacleFacts.push_back(fact);
        return (uint32)_obstacleFacts.size() - 1;
    }
    
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
    ObstacleScene addSharedObstacle(Uint32 factoryID, std::shared_ptr<std::vector<std::byte>> bytes);
    
    /**
     * Removes a shared obstacle from the physics world.
     *
     * If a linking function was provided, the scene node will also be removed.
     *
     * @param obs   the obstacle to remove
     */
    void removeSharedObstacle(std::shared_ptr<physics2::Obstacle> obs);

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
    void acquireObs(std::shared_ptr<physics2::Obstacle> obs, Uint64 duration);
    
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
    void releaseObs(std::shared_ptr<physics2::Obstacle> obs);
    
    /**
     * Makes this client the owner of all objects in the simulation.
     *
     * This method does not actually send any information to the other
     * clients on the network.  It should be used for initial objects only.
     */
    void ownAll();
    
    /**
     * Returns true if the given obstacle is being interpolated.
     *
     * @param obs   the obstacle to query
     *
     * @return true if the given obstacle is being interpolated.
     */
    bool isInSync(std::shared_ptr<physics2::Obstacle> obs) {
        return _cache.count(obs) > 0;
    }
    
    /**
     * Adds an object to interpolate with the given target parameters.
     *
     * This method is used for error smoothing.
     *
     * @param obj   The obstacle to interpolate
     * @param param The target parameters for interpolation
     */
    void addSyncObject(std::shared_ptr<physics2::Obstacle> obj,
                       const std::shared_ptr<TargetParams>& param);
    
#pragma mark World Synchronization
    /**
     * Returns the vector of generated events to be sent.
     *
     * @return the vector of generated events to be sent.
     */
    std::vector<std::shared_ptr<NetEvent>>& getOutEvents() {
        return _outEvents;
    }
    
    /**
     * Updates the physics controller.
     */
    void updateSimulation();
    
    /**
     * Processes a physics object synchronization event.
     *
     * This method is called automatically by the NetEventController.
     *
     * @param event The event to be processed
     */
    void processPhysObstEvent(const std::shared_ptr<PhysObstEvent>& event);
    
    /**
     * Processes a physics synchronization event.
     */
    void processPhysSyncEvent(const std::shared_ptr<PhysSyncEvent>& event);

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
    void packPhysSync(SyncType type);
    
    /**
     * Packs any changed object information
     *
     * This method checks the world for any dirty objects (e.g. objects that
     * have changed state outside of the simulation). If so, it packages
     * that information as an event to send out to other machines on the
     * network.
     */
    void packPhysObj();
    
    /**
     * Resets the physics controller.
     */
    void reset();
    
};

        }
    }
}
#endif /* __CU_NET_PHYSICS_CONTROLLER_H__ */
