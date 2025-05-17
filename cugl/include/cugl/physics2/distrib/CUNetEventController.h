//
//  CUNetEventController.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a network controller for multiplayer physics based
//  game.  It is an extension of the networking tools in cugl::net. It is
//  built around an event-based system that fully encapsulates the network
//  connection. Events across the network are automatically serialized and
//  deserialized.
//
//  The class for a general network controller for multiplayer physics based game.
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
//     pointer
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
#ifndef __CU_NET_EVENT_CONTROLLER_H__
#define __CU_NET_EVENT_CONTROLLER_H__

// TODO: Some of these can be removed with forward declarations
#include <cugl/physics2/distrib/CUNetWorld.h>
#include <cugl/physics2/distrib/CUNetEvent.h>
#include <cugl/physics2/distrib/CUNetPhysicsController.h>
#include <cugl/core/assets/CUAssetManager.h>
#include <cugl/physics2/CUObstacle.h>
#include <cugl/physics2/CUObstacleWorld.h>
#include <cugl/netcode/CUNetcodeConfig.h>
#include <cugl/netcode/CUNetcodeConnection.h>
#include <cugl/core/CUApplication.h>
#include <unordered_map>
#include <typeindex>
#include <vector>
#include <queue>
#include <memory>

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
 * This class is a network controller for multiplayer physics based game.
 *
 * This class holds a {@link cugl::netcode::NetcodeConnection} and is an extension
 * of the original network controller. It is built around an event-based system
 * that fully encapsulates the network connection. Events across the network
 * are automatically serialized and deserialized.
 *
 * Connection to to the lobby is provided by the methods {@link #connectAsHost}
 * and {@link #connectAsClient}. When starting a game, the host locks the lobby
 * and calls {@link startGame()} to initiate a handshake. The host then
 * distributes a shortUID to all players (including the host), and players
 * respond by calling {@link #markReady} after they receive the shortUID and
 * finish their initialization. When host receives responses from all players,
 * the game will officially start and {@link #getStatus} will return INGAME.
 *
 * Physics synchronization is an optional feature, and is enabled by calling
 * {@link #enablePhysics}. Upon enabling physics, a dedicated controller is
 * created to handle physics synchronization. For fine-tuning and more info,
 * see {@link NetPhysicsController}.
 *
 * There are three built-in event types: {@link GameStateEvent},
 * {@link PhysSyncEvent}, and {@link PhysObstEvent}. See the {@link NetEvent}
 * class and {@link #attachEventType} for how to add and setup custom events.
 */
class NetEventController {
public:
    /** This enum represents the current session status */
    enum class Status : int {
        /** No connection requested */
        IDLE = 0,
        /** Connecting to lobby (negotating) */
        CONNECTING = 1,
        /** Connected to lobby */
        CONNECTED = 2,
        /** Handshaking for game start */
        HANDSHAKE = 3,
        /** Ready for game start */
        READY = 4,
        /** Game is in progress */
        INGAME = 5,
        /** Error in connection */
        NETERROR = 6
    };
    
protected:
    /** The App fixed-time stamp when the game starts */
    Uint64 _startGameTimeStamp;
    
    /** The network configuration */
    cugl::netcode::NetcodeConfig _config;
    /** The network connection */
    std::shared_ptr<cugl::netcode::NetcodeConnection> _network;
    
    /** The network controller status */
    Status _status;
    /** The room id of the connected lobby. */
    std::string _roomid;
    /** Whether this device is host */
    bool _isHost;
    /** The number of ready players during game start handshake (HOST ONLY). */
    Uint8 _numReady;
    
    /** Map from attached NetEvent types to uniform event type id */
    std::unordered_map<std::type_index, Uint8> _eventTypeMap;
    /** Vector of NetEvents instances for constructing new events */
    std::vector<std::shared_ptr<NetEvent>> _newEventVector;
    
    /** Queue for all received custom events. Preserved across updates.*/
    std::queue<std::shared_ptr<NetEvent>> _inEventQueue;
    /** Queue reserved for built-in events */
    std::queue<std::shared_ptr<NetEvent>> _reservedInEventQueue;
    /** Queue for all outbound events. Cleared every update */
    std::vector<std::shared_ptr<NetEvent>> _outEventQueue;
    
    /** Short user id assigned by the host during session */
    Uint32 _shortUID;
    /** Whether physics is enabled. */
    bool _physEnabled;
    /** The physics synchronization controller */
    std::shared_ptr<NetPhysicsController> _physController;
    
    /*
     * =================== Note for clarification ===================
     * Outbound events are generated locally and sent to peers.
     * Inbound events are received from peers and processed locally.
     */
    
#pragma mark Networking Internals
    /**
     * Unwraps the a byte vector data into a NetEvent.
     *
     * The controller automatically detects the type of event, spawns a new
     * empty instance of that event, and calls the event's
     * {@link NetEvent#deserialize} method. This method is only called on
     * outbound events.
     *
     * @param data      The message received
     * @param source    The UUID of the sender
     */
    std::shared_ptr<NetEvent> unwrap(const std::vector<std::byte>& data,std::string source);
    
    /**
     * Wraps a NetEvent into a byte vector.
     *
     * The controller calls the event's {@link NetEvent#serialize()} method
     * and packs the event into byte data. This method is only on inbound
     * events.
     *
     * @param e The event to wrap
     */
    const std::vector<std::byte> wrap(const std::shared_ptr<NetEvent>& e);
    
    /**
     * Processes all received packets received during the last update.
     *
     * This method unwraps byte vectors into NetEvents and calls
     * {@link processReceivedEvent()}.
     */
    void processReceivedData();
    
    /**
     * Processes all events received during the last update.
     *
     * This method either processes events internally if it is a built-in event
     * and adds them to the inbound event queue otherwise.
     *
     * @param e The received event
     */
    void processReceivedEvent(const std::shared_ptr<NetEvent>& e);
    
    /**
     * Processes a GameStateEvent.
     *
     * This method updates the controller status based on the event received.
     *
     * @param e The received event
     */
    void processGameStateEvent(const std::shared_ptr<GameStateEvent>& e);
    
    /**
     * Returns true if the connection is still active after a status check
     *
     * This method updates the controller status according to the protocol.
     *
     * @return true if the connection is still active after a status check
     */
    bool checkConnection();
    
    /**
     * Broadcasts all queued outbound events.
     */
    void sendQueuedOutData();
    
    /**
     * Returns the type id of a NetEvent.
     *
     * @return the type id of a NetEvent.
     */
    Uint8 getType(const NetEvent& e) {
        return _eventTypeMap.at(std::type_index(typeid(e)));
    }
    
#pragma mark Constructors
public:
    /**
     * Creates a degenerate network controller
     *
     * This object will have only default values and has not yet been
     * initialized.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    NetEventController(void);
    
    /**
     * Deletes this network controller and all of its resources.
     */
    ~NetEventController() { dispose(); }
    
    /**
     * Disposes the network controller, releasing all resources.
     *
     * This controller can be safely reinitialized
     */
    void dispose();
    
    /**
     * Initializes the controller for the given asset manager.
     *
     * This method requires the asset manager to have a JSON value with key
     * "server". The JSON value should match the structure required by
     * {@link cugl::netcode::NetcodeConfig}.
     *
     * @param assets    The asset manager
     *
     * @return true if the network controller was successfully initialized
     */
    bool init(const std::shared_ptr<AssetManager>& assets);
    
    /**
     * Returns a newly allocated controller for the given asset manager.
     *
     * This method requires the asset manager to have a JSON value with key
     * "server". The JSON value should match the structure required by
     * {@link cugl::netcode::NetcodeConfig}.
     *
     * @param assets    The asset manager
     *
     * @return a newly allocated controller for the given asset manager.
     */
    static std::shared_ptr<NetEventController> alloc(const std::shared_ptr<AssetManager>& assets) {
        std::shared_ptr<NetEventController> result = std::make_shared<NetEventController>();
        return (result->init(assets) ? result : nullptr);
    }
    
#pragma mark Controller Attributes
    /**
     * Returns whether this device is host.
     *
     * This value is only valid after a connection. It will always return false
     * if there is no connection.
     *
     * @return whether this device is host.
     */
    bool isHost() const {
        return _isHost;
    }
    
    /**
     * Returns the room ID currently assigned to this controller.
     *
     * This value is only valid after a connection. It will always return the
     * empty string if there is no connection.
     *
     * @return the room ID currently assigned to this controller.
     */
    std::string getRoomID() const { return _roomid; }
    
    /**
     * Returns the shortUID assigned by the host.
     *
     * This value is only valid after connection. If the shortUID is 0, the
     * controller did not receive a ID from the host yet. An assigned shortUID
     * is required for physics synchronization, and is always non-zero.
     *
     * @return the shortUID assigned by the host.
     */
    Uint32 getShortUID() const { return _shortUID; }
    
    /**
     * Returns the number of players in the lobby.
     *
     * This value is only valid after a connection. If there is no connection,
     * it returns 1 (for this player).
     *
     * @return the number of players in the lobby.
     */
    int getNumPlayers() const;
    
    /**
     * Returns the current status of the controller.
     *
     * @return the current status of the controller.
     */
    Status getStatus() const { return _status; }
            
#pragma mark Connection Management
    /**
     * Connects to a new lobby as host.
     *
     * If successful, the controller status changes to {@link Status#CONNECTED},
     * and {@link #getRoomID} is set to the lobby id.
     */
    bool connectAsHost();
    
    /**
     * Connects to an existing lobby as client.
     *
     * If successful, the controller status changes to {@link Status#CONNECTED}.
     *
     * @param roomID    The room to connect to
     */
    bool connectAsClient(std::string roomID);
    
    /**
     * Disconnects from the current lobby.
     */
    void disconnect();
    
    /**
     * Starts the handshake process for starting a game.
     *
     * Once the handshake is finished, the controller changes status to
     * {@link Status#INGAME}. It also starts sending synchronization events
     * if physics is enabled.
     */
    void startGame();
    
    /**
     * Marks the client as ready for game start.
     *
     * This method is only valid after receiving a shortUID from the host.
     *
     * @return true if the communiction was successful.
     */
    bool markReady();
    
#pragma mark Physics Synchronization
    /**
     * Returns the physics synchronization controller.
     *
     * If physics has not been enabled, this method returns nullptr.
     *
     * @return the physics synchronization controller.
     */
    std::shared_ptr<NetPhysicsController> getPhysController() { return _physController; }
    
    /**
     * Returns the discrete timestamp since the game started.
     *
     * Peers should have similar timestamps regardless of when their app was
     * launched, although peer gameticks might fluctuate due to network
     * latency.
     *
     * @return the discrete timestamp since the game started.
     */
    Uint64 getGameTick() const;
    /**
     * Enables physics synchronization.
     *
     * This method requires the shortUID to be assigned to this controller.
     * This version of the method does not link the physics world to a
     * secent graph and requires the user to handle view changes (due to
     * obstacle creation and deletion) manually.
     *
     * @param world The physics world to be synchronized.
     */
    void enablePhysics(std::shared_ptr<NetWorld>& world) {
        enablePhysics(world,nullptr);
    }
    
    /**
     * Enables physics synchronization.
     *
     * This method requires the shortUID to be assigned to this controller.
     * The linkFunc should be a function that links a scene node to an obstacle
     * with a listener, and then adds that scene node to a scene graph. See
     * {@link NetPhysicsController} for more information.
     *
     * @param world The physics world to be synchronized.
     * @param linkFunc Function that links a scene node to an obstacle.
     */
    void enablePhysics(std::shared_ptr<NetWorld>& world, ObstacleLink linkFunc);
    
    /**
     * Disables physics synchronization.
     */
    void disablePhysics();
    
#pragma mark Event Management
    /**
     * Attaches a new NetEvent type to the controller.
     *
     * This method allows the controller the receive and send custom NetEvent
     * classes. The template type T must be a subclass of NetEvent.
     *
     * @tparam T The event type to be attached
     */
    template <typename T>
    void attachEventType() {
        if (!_eventTypeMap.count(std::type_index(typeid(T)))) {
            _eventTypeMap.insert(std::make_pair(std::type_index(typeid(T)), _newEventVector.size()));
            _newEventVector.push_back(std::make_shared<T>());
        }
    }
    
    /**
     * Returns true if there are remaining custom inbound events.
     *
     * Thhe events in this queue is to be polled and processed by outside
     * classes. Inbound events are preserved acrossupdates, and only cleared
     * by {@link #popInEvent}.
     *
     * @return true if there are remaining custom inbound events.
     */
    bool isInAvailable();
    
    /**
     * Returns the next custom inbound event and removes it from the queue.
     *
     * This method requires there to be remaining inbound events. If there are
     * none, it returns nullptr.
     *
     * @return the next custom inbound event
     */
    std::shared_ptr<NetEvent> popInEvent();
    
    /**
     * Queues an outbound event to be sent to peers.
     *
     * Queued events are sent when {@link #updateNet} is called. and cleared
     * after sending.
     *
     * @param e The event to send
     */
    void pushOutEvent(const std::shared_ptr<NetEvent>& e);
    
    /**
     * Updates the network controller.
     *
     * This method pushes out all queued events and processes all incoming
     * events.
     */
    void updateNet();
    
};

        }
    }
}

#endif /* __CU_NET_EVENT_CONTROLLER_H__ */
