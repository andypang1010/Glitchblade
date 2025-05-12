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
#include <cugl/physics2/distrib/CUNetEventController.h>
#include <cugl/physics2/distrib/CULWSerializer.h>
#include <cugl/netcode/CUNetworkLayer.h>

/** The minimum message length */
#define MIN_MSG_LENGTH sizeof(std::byte)+sizeof(Uint64)

using namespace cugl;
using namespace cugl::physics2;
using namespace cugl::physics2::distrib;

#pragma mark -
#pragma mark Constructors
/**
 * Creates a degenerate network controller
 *
 * This object will have only default values and has not yet been
 * initialized.
 *
 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
 * the heap, use one of the static constructors instead.
 */
NetEventController::NetEventController(void):
_isHost(false),
_shortUID(0),
_numReady(0),
_roomid(""),
_physEnabled(false),
_status(Status::IDLE),
_startGameTimeStamp(0) {
}

/**
 * Disposes the network controller, releasing all resources.
 *
 * This controller can be safely reinitialized
 */
void NetEventController::dispose() {
    disconnect();
}

/**
 * Initializes the controller for the given asset manager.
 *
 * This method requires the asset manager to have a JSON value with key
 * "server". The JSON value should match the structure required by
 * {@link NetworkConfig}.
 *
 * @param assets    The asset manager
 *
 * @return true if the network controller was successfully initialized
 */
bool NetEventController::init(const std::shared_ptr<cugl::AssetManager>& assets) {
    // Attach the primitive event types for deserialization
    attachEventType<GameStateEvent>();

    // Configure the NetcodeConnection
    auto json = assets->get<JsonValue>("server");
    _config.set(json);
    _status = Status::IDLE;
    return true;
}

#pragma mark Connection Management
/**
 * Returns the number of players in the lobby.
 *
 * This value is only valid after a connection. If there is no connection,
 * it returns 1 (for this player).
 *
 * @return the number of players in the lobby.
 */
int NetEventController::getNumPlayers() const {
    if (_network) {
        return (int)(_network->getNumPlayers());
    }
    return 1;
}

/**
 * Connects to a new lobby as host.
 *
 * If successful, the controller status changes to {@link Status#CONNECTED},
 * and {@link #getRoomID} is set to the lobby id.
 */
bool NetEventController::connectAsHost() {
    if (_status == Status::NETERROR) {
        disconnect();
    }

    _isHost = true;
    if (_status == Status::IDLE) {
        _status = Status::CONNECTING;
        _network = cugl::netcode::NetcodeConnection::alloc(_config);
        _network->open();
    }
    return checkConnection();
}

/**
 * Connects to an existing lobby as client.
 *
 * If successful, the controller status changes to {@link Status#CONNECTED}.
 *
 * @param roomID    The room to connect to
 */
bool NetEventController::connectAsClient(std::string roomID) {
    if (_status == Status::NETERROR) {
        disconnect();
    }

    _isHost = false;
    if (_status == Status::IDLE) {
        _status = Status::CONNECTING;
        _network = cugl::netcode::NetcodeConnection::alloc(_config, roomID);
        _network->open();
    }
    _roomid = roomID;
    return checkConnection();

}

/**
 * Disconnects from the current lobby.
 */
void NetEventController::disconnect() {
    if(_network && _network->isOpen()) {
        _network->close();
    }
    _network = nullptr;
    _physController = nullptr;
    _shortUID = 0;
    _status = Status::IDLE;
    _physEnabled = false;
    _isHost = false;
    _startGameTimeStamp = 0;
    _numReady = 0;
    _outEventQueue.clear();
    
    while (!_inEventQueue.empty()) {
        _inEventQueue.pop();
    }
}

/**
 * Starts the handshake process for starting a game.
 *
 * Once the handshake is finished, the controller changes status to
 * {@link Status#INGAME}. It also starts sending synchronization events
 * if physics is enabled.
 */
void NetEventController::startGame() {
    CUAssertLog(_isHost, "Only host should call startGame()");
    if (_status == Status::CONNECTED) {
        _network->startSession();
    }
}

/**
 * Marks the client as ready for game start.
 *
 * This method is only valid after receiving a shortUID from the host.
 *
 * @return true if the communiction was successful.
 */
bool NetEventController::markReady() {
    if (_status == Status::HANDSHAKE && _shortUID) {
        _status = Status::READY;
        pushOutEvent(GameStateEvent::allocReady());
        return true;
    }
    return false;
}

#pragma mark Physics Synchronization
/**
 * Returns the discrete timestamp since the game started.
 *
 * Peers should have similar timestamps regardless of when their app was
 * launched, although peer gameticks might fluctuate due to network
 * latency.
 *
 * @return the discrete timestamp since the game started.
 */
Uint64 NetEventController::getGameTick() const {
    Uint64 time = Application::get()->getFixedCount();
    return time - _startGameTimeStamp;
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
void NetEventController::enablePhysics(std::shared_ptr<NetWorld>& world, ObstacleLink linkFunc) {
    CUAssertLog(_shortUID, "You must receive a UID assigned from host before enabling physics.");
    _physEnabled = true;
    _physController = NetPhysicsController::alloc(world,_shortUID,_isHost,linkFunc);
    //CULog("ENABLED PHYSICS");
    attachEventType<PhysSyncEvent>();
    attachEventType<PhysObstEvent>();
    if(_isHost) {
        _physController->ownAll();
    }
}

/**
 * Disables physics synchronization.
 */
void NetEventController::disablePhysics() {
    _physEnabled = false;
    _physController = nullptr;
}


#pragma mark Event Management
/**
 * Returns true if there are remaining custom inbound events.
 *
 * Thhe events in this queue is to be polled and processed by outside
 * classes. Inbound events are preserved acrossupdates, and only cleared
 * by {@link #popInEvent}.
 *
 * @return true if there are remaining custom inbound events.
 */
bool NetEventController::isInAvailable() {
    if ( _inEventQueue.empty() ) {
        return false;
    }
    
    Uint64 time = Application::get()->getFixedCount();
    std::shared_ptr<NetEvent> top = _inEventQueue.front();
    return top->_eventTimeStamp <= time-_startGameTimeStamp;
}

/**
 * Returns the next custom inbound event and removes it from the queue.
 *
 * This method requires there to be remaining inbound events. If there are
 * none, it returns nullptr.
 *
 * @return the next custom inbound event
 */
std::shared_ptr<NetEvent> NetEventController::popInEvent() {
	auto e = _inEventQueue.front();
	_inEventQueue.pop();
	return e;
}

/**
 * Queues an outbound event to be sent to peers.
 *
 * Queued events are sent when {@link #updateNet} is called. and cleared
 * after sending.
 *
 * @param e The event to send
 */
void NetEventController::pushOutEvent(const std::shared_ptr<NetEvent>& e) {
	_outEventQueue.push_back(e);
}

/**
 * Updates the network controller.
 *
 * This method pushes out all queued events and processes all incoming
 * events.
 */
void NetEventController::updateNet() {
    if(_network){
        checkConnection();

        if (_status == Status::INGAME && _physEnabled) {
            _physController->packPhysSync(NetPhysicsController::SyncType::FULL_SYNC);
            _physController->packPhysObj();
            _physController->updateSimulation();
            for (auto it = _physController->getOutEvents().begin(); it != _physController->getOutEvents().end(); it++) {
                pushOutEvent(*it);
            }
            _physController->getOutEvents().clear();
                
        }
        
        processReceivedData();
        sendQueuedOutData();
    }
}


#pragma mark Networking Internals
/**
 * Unwraps the a byte vector data into a NetEvent.
 *
 * The controller automatically detects the type of event, spawns a new
 * empty instance of that event, and calls the event's
 * {@link NetEvent#deserialize} method. This method is only called on
 * outbound events.
 *
 * @param byte      The message received
 * @param source    The UUID of the sender
 */
std::shared_ptr<NetEvent> NetEventController::unwrap(const std::vector<std::byte>& data, std::string source) {
    CUAssertLog(data.size() >= MIN_MSG_LENGTH && (Uint8)data[0] < _newEventVector.size(),
                "Unwrapping invalid event");
    LWDeserializer deserializer;
    deserializer.receive(data);
    Uint8 eventType = (Uint8)deserializer.readByte();
    std::shared_ptr<NetEvent> e = _newEventVector[eventType]->newEvent();
    Uint64 eventTimeStamp = deserializer.readUint64();

    Uint64 time = Application::get()->getFixedCount();
    Uint64 receiveTimeStamp = time-_startGameTimeStamp;
    e->setMetaData(eventTimeStamp, receiveTimeStamp, source);
    e->deserialize(std::vector(data.begin()+MIN_MSG_LENGTH,data.end()));
    return e;
}

/**
 * Wraps a NetEvent into a byte vector.
 *
 * The controller calls the event's {@link NetEvent#serialize()} method
 * and packs the event into byte data. This method is only on inbound
 * events.
 *
 * @param e The event to wrap
 */
const std::vector<std::byte> NetEventController::wrap(const std::shared_ptr<NetEvent>& e) {
    LWSerializer serializer;
    serializer.writeByte((std::byte)getType(*e));
    //CULog("flag: %x",(std::byte)getType(*e));

    Uint64 time = Application::get()->getFixedCount();
    serializer.writeUint64(time-_startGameTimeStamp);
    serializer.writeByteVector(e->serialize());
    return serializer.serialize();
}

/**
 * Processes all received packets received during the last update.
 *
 * This method unwraps byte vectors into NetEvents and calls
 * {@link processReceivedEvent()}.
 */
void NetEventController::processReceivedData(){
    _network->receive([this](const std::string source,
        const std::vector<std::byte>& data) {
        //if (cugl::net::NetworkLayer::get()->isDebug()) {
        //    CULog("DATA %d, CUR STATE %d, SOURCE %s", data[0], _status, source.c_str());
        //}
        processReceivedEvent(unwrap(data, source));
    });
}

/**
 * Processes all events received during the last update.
 *
 * This method either processes events internally if it is a built-in event
 * and adds them to the inbound event queue otherwise.
 *
 * @param e The received event
 */
void NetEventController::processReceivedEvent(const std::shared_ptr<NetEvent>& e) {
    if (auto game = std::dynamic_pointer_cast<GameStateEvent>(e)) {
        processGameStateEvent(game);
    } else if (_status == Status::INGAME){
        if (auto phys = std::dynamic_pointer_cast<PhysSyncEvent>(e)) {
            if(_physEnabled)
                _physController->processPhysSyncEvent(phys);
        }
        else if (auto phys = std::dynamic_pointer_cast<PhysObstEvent>(e)) {
            if (_physEnabled) {
                _physController->processPhysObstEvent(phys);
            }
        }
        else {
            _inEventQueue.push(e);
        }
    }
}

/**
 * Processes a GameStateEvent.
 *
 * This method updates the controller status based on the event received.
 *
 * @param e The received event
 */
void NetEventController::processGameStateEvent(const std::shared_ptr<GameStateEvent>& e) {
    bool debug = cugl::netcode::NetworkLayer::get()->isDebug();
    
    if (debug) {
        CULog("NET PHYSICS: Game State %d, Status %d", e->getType(), _status);
    }
    if (_status == Status::HANDSHAKE && e->getType() == GameStateEvent::EventType::UID_ASSIGN) {
        _shortUID = e->getShortUID();
        if (debug) {
            CULog("Net PHYSICS: Assigned short UID %x", _shortUID);
        }
    }
    if (_status == Status::READY && e->getType() == GameStateEvent::EventType::GAME_START) {
        _status = Status::INGAME;
        _startGameTimeStamp = Application::get()->getFixedCount();
    }
    if (_isHost) {
        if (e->getType() == GameStateEvent::EventType::CLIENT_RDY) {
            _numReady++;
            if (debug) {
                CULog("NET PHYSICS: Received ready from %s", e->getSourceId().c_str());
            }
        }
    }
    if (debug) {
        CULog("NET PHYSICS: Processed status %d", _status);
    }
}

/**
 * Returns true if the connection is still active after a status check
 *
 * This method updates the controller status according to the protocol.
 *
 * @return true if the connection is still active after a status check
 */
bool NetEventController::checkConnection() {
    auto state = _network->getState();
    bool debug = cugl::netcode::NetworkLayer::get()->isDebug();

    if (state == cugl::netcode::NetcodeConnection::State::CONNECTED) {
        if(_status == Status::CONNECTING || _status == Status::IDLE)
            _status = Status::CONNECTED;
        if (_isHost) {
            _roomid = _network->getRoom();
        }
        return true;
    } else if (_status == Status::CONNECTED &&
             state == cugl::netcode::NetcodeConnection::State::INSESSION) {
        _status = Status::HANDSHAKE;
        if (_isHost) {
            auto players = _network->getPlayers();
            Uint32 shortUID = 1;
            if (debug) {
                CULog("NET PHYSICS: %zu players found", players.size());
            }
            for (auto it = players.begin(); it != players.end(); it++) {
                if (debug) {
                    CULog("NET PHYSICS: Player '%s'", (*it).c_str());
                }
                _network->sendTo((*it), wrap(GameStateEvent::allocUIDAssign(shortUID++)));
            }
        }
        return true;
    } else if (_status == Status::READY && _numReady == _network->getNumPlayers() && _isHost) {
        if (debug) {
            CULog("NET PHYSICS: Start message sent");
        }
        pushOutEvent(GameStateEvent::allocGameStart());
    } else if (state == cugl::netcode::NetcodeConnection::State::NEGOTIATING) {
        _status = Status::CONNECTING;
        return true;
    } else if (state == cugl::netcode::NetcodeConnection::State::DENIED ||
               state == cugl::netcode::NetcodeConnection::State::DISCONNECTED ||
               state == cugl::netcode::NetcodeConnection::State::FAILED ||
               state == cugl::netcode::NetcodeConnection::State::INVALID ||
               state == cugl::netcode::NetcodeConnection::State::MISMATCHED) {
        _status = Status::NETERROR;
        return false;
    }
    return true;
}

/**
 * Broadcasts all queued outbound events.
 */
void NetEventController::sendQueuedOutData(){
    int msgCount = 0;
    int byteCount = 0;
    for(auto it = _outEventQueue.begin(); it != _outEventQueue.end(); it++){
        auto e = *(it);
        auto wrapped = wrap(e);
        msgCount++;
        byteCount += wrapped.size();
        //CULog("flag: %x", (std::byte)getType(*e))
        _network->broadcast(wrap(e));
    }
    _outEventQueue.clear();
}

