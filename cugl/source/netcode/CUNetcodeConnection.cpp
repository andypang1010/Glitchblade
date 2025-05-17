//
//  CUNetcodeConnection.cpp
//  Cornell University Game Library (CUGL)
//
//  This module is part of a Web RTC implementation of the classic CUGL
//  networking library. That library provided connected to a custom game server
//  for matchmaking, and used reliable UDP communication. This version replaces
//  the matchmaking server with a web socket, and uses web socket data channels
//  for communication.
//
//  This module specifically supports the top level connection classes. These
//  are the only part of the netcode communication classes that the user will
//  interface with directly.
//
//  This module is inspired by the original networking classes defined by
//  onewordstudios:
//
//  - Demi Chang
//  - Aashna Saxena
//  - Sam Sorenson
//  - Michael Xing
//  - Jeffrey Yao
//  - Wendy Zhang
//  https://onewordstudios.com/
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
//  Note, however, that it is never safe to put this particular object on the stack.
//  therefore, everything except for the static constructors are private.
//
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
//  Author: Walker White
//  Version: 7/3/24 (CUGL 3.0 reorganization)
//
#include <cugl/netcode/CUNetcodeChannel.h>
#include <cugl/netcode/CUNetcodeConnection.h>
#include <cugl/netcode/CUNetcodePeer.h>
#include <cugl/netcode/CUNetworkLayer.h>
#include <cugl/core/assets/CUJsonValue.h>
#include <cugl/core/util/CUDebug.h>
#include <cugl/core/util/CUHashtools.h>
#include <cugl/core/CUApplication.h>
#include <stdexcept>
#include <algorithm>
#include <random>
#include <cstring>
#include <SDL_app.h>

using namespace cugl::netcode;
using namespace std;
using namespace rtc;

/** The buffer size for message envelopes */
#define DEFAULT_BUFFER 64

/**
 * Copies information from a CUGL configuration to an RTC configuration
 *
 * The purpose of this function is to hide libdatachannel from the students.
 *
 * @param src   The CUGL configuration
 * @param dst 	The RTC configuration
 */
static void config2rtc(const NetcodeConfig& src, rtc::Configuration& dst) {
	for(auto it = src.iceServers.begin(); it != src.iceServers.end(); ++it) {
		dst.iceServers.emplace_back(it->toString());
	}
	dst.enableIceUdpMux = src.multiplex;
	dst.portRangeBegin = src.portRangeBegin;
	dst.portRangeEnd = src.portRangeEnd;
    if (src.maxMessage != 0) {
        dst.maxMessageSize = src.maxMessage;
    }
    if (src.mtu != 0) {
        dst.mtu = src.mtu;
    }
}

#pragma mark -
#pragma mark Constructors
/**
 * Creates a degenerate netcode connection.
 *
 * This object has not been initialized with a {@link NetcodeConfig} and
 * cannot be used.
 *
 * You should NEVER USE THIS CONSTRUCTOR. All connections should be created
 * by the static constructor {@link #alloc} instead.
 */
NetcodeConnection::NetcodeConnection() :
	_uuid(""),
	_host(""),
	_room(""),
	_socket(nullptr), 
	_ishost(false), 
	_initialPlayers(0),
	_migration(0),
	_buffsize(0),
	_buffhead(0),
	_bufftail(0),
	_debug(false),
	_open(false),
	_active(false),
	_state(State::INACTIVE),
	_previous(State::INACTIVE) {}

/**
 * Deletes this websocket connection, disposing all resources
 */
NetcodeConnection::~NetcodeConnection() {
	dispose();
}

/**
 * Disposes all of the resources used by this netcode connection.
 *
 * While we never expect to reinitialize a new netcode connection, this
 * method allows for a "soft" deallocation, where internal resources are
 * destroyed as soon as a connection is terminated. This simplifies the
 * cleanup process.
 */
void NetcodeConnection::dispose() {
	if (!_active) {
		return;
	}
	
	// ORDER MATTERS HERE (otherwise deadlock)

	// Critical section (clear peers first)
	std::unordered_map<std::string, std::shared_ptr<NetcodePeer>> peers;
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if (_active && !_peers.empty()) {
			// Clearing this would call dispose, which needs a lock
			// Copy it to invoke GC outside of lock
			peers = _peers;
			_peers.clear();
		}
	}
	peers.clear();

	// Critical section (shutdown socket)
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if (_active) {
			_active = false;	// Prevents cycles
			_open = false;

            _socket->close();
			_socket = nullptr;

			_host = "";
			_room = "";
			_ishost = false;
			
			_buffer.clear();
			_players.clear();
			_rtcconfig.iceServers.clear();
			
			// Leave other settings for debugging
		}
	}
}

/**
 * Initializes a new network connection as host.
 *
 * This method initializes this netcode connection with all of the correct
 * settings. However, it does **not** connect to the game lobby. You must
 * call the method {@link #open} to initiate connection. This design
 * decision is intended to give the user a chance to set the callback
 * functions before connection is established.
 *
 * This method will always return false if the {@link NetworkLayer} failed
 * to initialize.
 *
 * @param config    The connection configuration
 *
 * @return true if initialization was successful
 */
bool NetcodeConnection::init(const NetcodeConfig& config) {
	try {
		if (NetworkLayer::get() == nullptr) {
			CUAssertLog(false, "Network layer is not active");
			return false;
		}
		_debug  = NetworkLayer::get()->isDebug();
		
		_config = config;
		config2rtc(_config,_rtcconfig);
        _bufflimit = config.bufferSize == 0 ? DEFAULT_BUFFER : config.bufferSize;

        // Get the UUID
        if (_config.uuidSeed.empty()) {
            _uuid = hashtool::generate_uuid();
        } else {
            std::string device(APP_GetDeviceID());
            if (device.empty()) {
                _uuid = hashtool::generate_uuid();
            }
            _uuid = hashtool::create_uuid(_config.uuidSeed+device);
        }
		_ishost = true;
		_host = _uuid;
		
		return true;
	} catch (const std::exception &e) {
		CULogError("NETCODE ERROR: %s",e.what());
		_socket = nullptr;
		_active = false;
		return false;
	}	
}

/**
 * Initializes a new network connection as a client.
 *
 * This method initializes this websocket connection with all of the correct
 * settings. However, it does **not** connect to the game lobby. You must
 * call the method {@link #open} to initiate connection. This design decision
 * is intended to give the user a chance to set the callback functions before
 * connection is established.
 *
 * The room should match one specified by the host. If you are using the
 * traditional CUGL lobby server, this will be a hexadecimal string.
 *
 * This method will always return false if the {@link NetworkLayer} failed
 * to initialize.
 *
 * @param config    The connection configuration
 * @param room      The host's assigned room id
 *
 * @return true if initialization was successful
 */
bool NetcodeConnection::init(const NetcodeConfig& config, const std::string room) {
	try {
		if (NetworkLayer::get() == nullptr) {
			CUAssertLog(false, "Network layer is not active");
			return false;
		}
		_debug  = NetworkLayer::get()->isDebug();
		
		_config = config;
		config2rtc(_config,_rtcconfig);
        _bufflimit = config.bufferSize == 0 ? DEFAULT_BUFFER : config.bufferSize;

		// Get the UUID 
        if (_config.uuidSeed.empty()) {
            _uuid = hashtool::generate_uuid();
        } else {
            std::string device(APP_GetDeviceID());
            if (device.empty()) {
                _uuid = hashtool::generate_uuid();
            }
            _uuid = hashtool::create_uuid(_config.uuidSeed+device);
        }
        
		_ishost = false;
		_room = room;
		
		return true;
	} catch (const std::exception &e) {
		CULogError("NETCODE ERROR: %s",e.what());
		_socket = nullptr;
		_active = false;
		return false;
	}
}

#pragma mark -
#pragma mark Internal Callbacks
/**
 * Called when the web socket first opens
 */
void NetcodeConnection::onOpen() {
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if (_active) {
			if (_debug) {
				CULog("NETCODE: WebSocket %s connected, negotiating role",_uuid.c_str());
			}			
			_state = State::NEGOTIATING;
			_open = true;
		}
	}
}

/**
 * Called when the websocket experiences an error
 *
 * @param s The error message
 */
void NetcodeConnection::onError(std::string s) {
	if (_debug) {
		std::lock_guard<std::recursive_mutex> lock(_mutex);
        CULogError("NETCODE: WebSocket error '%s' on %s",s.c_str(),_uuid.c_str());
	}
}

/**
 * Called when the web socket closes
 */
void NetcodeConnection::onClosed() { 
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if (_active) {
			if (_debug) {
				CULog("NETCODE: WebSocket %s closed, ",_uuid.c_str());
			}			
			_state = State::DISCONNECTED;
			_open = false;
		}
	}
	dispose();
}

/**
 * Called when this websocket (and not a peer channel) receives a message
 *
 * @param data  The message received
 */
void NetcodeConnection::onMessage(message_variant data) {
	// data holds either std::string or rtc::binary
	if (!std::holds_alternative<std::string>(data)) {
		return;
	}
	
 	std::function<bool()> callback;
	std::string value = std::get<std::string>(data);
	auto json = cugl::JsonValue::allocWithJson(value);
	if (_debug) {
		CULog("NETCODE: Received '%s'",value.c_str());
	}

	if (json != NULL) {
		std::string type = json->getString("type");
		if (type == "lobby") {
			std::string category = json->getString("category");
			if (category == "room-assign") {
				handleNegotiation(json);
			} else if (category == "player" || category == "session") {
				handleSession(json);
			} else if (category == "migration" || category == "promotion") {
				handleMigration(json);				
            } else if (category == "failed") {
                close();
            } else {
				std::lock_guard<std::recursive_mutex> lock(_mutex);
				if (_active) {
					CUAssertLog(false,"NETCODE: WebSocket %s got unrecognized category '%s'",_uuid.c_str(),category.c_str());
					_previous = _state;
					_state = State::FAILED;
					if (_onStateChange) {
						callback = [=,this]() {
							_onStateChange(_state);
							return false;
						};
					}
				}
			}
			
			if (callback) {
                Application::get()->schedule(callback);
			}
		} else {
			handleSignal(json);
		}
	} else if (_debug) {
		CULog("NETCODE: Invalid message '%s'",value.c_str());
	}
}

/** 
 * Called when a peer has established BOTH data channels
 *
 * @param uuid  The UUID of the peer connection
 */
void NetcodeConnection::onPeerEstablished(const std::string uuid) {
 	std::function<bool()> callback;
    // Critical section
    {
		std::lock_guard<std::recursive_mutex> lock(_mutex);
    	if (_state != State::MIGRATING) {
			if (uuid == _host)	{
				_previous = _state;
				_state = State::CONNECTED;
				if (_onStateChange) {
					callback = [=,this]() {
						_onStateChange(_state);
						return false;
					};
				}
			} else {
                // Incoming sibling player
                _players.emplace(uuid);
                if (_onConnect) {
                    callback = [=,this]() {
                        _onConnect(uuid);
                        return false;
                    };
                }
			}
		} else if (_migration == 1) {
			// This is the result of host migration
			auto response = cugl::JsonValue::allocObject();
			response->appendValue("id",_uuid);
			response->appendValue("type",std::string("lobby"));
			response->appendValue("category",std::string("promotion"));
			response->appendValue("status",std::string("complete"));
			_socket->send(response->toString());
			_migration = 0;
		} else if (_migration > 1) {
			_migration--;
		}
	}

	if (callback) {
        Application::get()->schedule(callback);
	}
}

/**
 * Called when a peer connection closes
 *
 * @param id  The UUID of the peer connection
 */
void NetcodeConnection::onPeerClosed(std::string id) {
    // Critical section
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        if (_active) {
            if (_debug) {
                CULog("NETCODE: WebSocket %s cleaned-up peer connection %s",_uuid.c_str(),id.c_str());
            }
            _peers.erase(id);
        }
    }
}

#pragma mark -
#pragma mark Internal Communication
/**
 * Offers a peer connection to the host with the given UUID
 *
 * Only clients offer peer connections. The host only receives peer 
 * connections.
 *
 * @param uuid  The UUID for the peer connection to create
 *
 * @return true if the peer connection was successfully created
 */
bool NetcodeConnection::offerPeer(const std::string uuid) {
	try {
		std::shared_ptr<NetcodePeer> peer = NetcodePeer::alloc(shared_from_this(),uuid,true);

		// Critical section
		{
			std::lock_guard<std::recursive_mutex> lock(_mutex);
			if (_active) {
				_peers.emplace(uuid,peer);
			} else {
				return false;
			}
		}

        // We are the offerer, so create a data channel to initiate the process
        peer->createChannel("public");
        return true;
	} catch (const std::exception &e) {
		CULogError("NETCODE ERROR: %s", e.what());
		dispose();
		return false;
	}
}

/**
 * Processes a JSON message that is part of the initial room negotiation
 *
 * All websocket messages are JSON objects. Once we parse the bytes into
 * JSON, we can determine the type of message. These messages are custom
 * to our game lobby server, and not part of a typical signaling server.
 *
 * @param json  The message to handle
 */
void NetcodeConnection::handleNegotiation(const std::shared_ptr<JsonValue>&  json) {
	std::shared_ptr<cugl::JsonValue> response = nullptr;
 	
 	// Schedulable tasks for main thread
    std::vector<std::string> outgoing;
 	std::function<bool()> callback;
 	bool statech = false;
 	bool connect = false;
 	std::string host = "";
	
 	// Critical section
 	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if (!_active) {
			return;
		}
		
		// Negotiation messages all have category "room-assign".
		// They only differ in status
		std::string status = json->getString("status");
		
		if (status == "handshake") {
			// Declare our roles	
			response = cugl::JsonValue::allocObject();
			response->appendValue("id",_uuid);
			response->appendValue("type",std::string("lobby"));
			response->appendValue("category",std::string("room-assign"));
			response->appendValue("status",std::string("request"));
			if (_ishost) {
				response->appendValue("host",true);
				response->appendValue("maxPlayers",(long)_config.maxPlayers);
				response->appendValue("apiVersion",(long)_config.apiVersion);
			} else {
				response->appendValue("host",false);
				response->appendValue("room",_room);
				response->appendValue("apiVersion",(long)_config.apiVersion);
			}
		} else if (status == "success") {
			// We have a room. Time to start signaling
			// Just in case things change about host
			_room = json->getString("room");
			_host = json->getString("host");
			auto child = json->get("players");
			for(int ii = 0; ii < child->size(); ii++) {
                std::string value = child->get(ii)->asString();
				_players.emplace(value);
                if (value != _uuid) {
                    outgoing.push_back(value);
                }
			}
			
			connect = !_ishost;
			host = _host;
			if (_ishost) {
				// Only the host gets connected immediately
				statech = true;
				_previous = _state;
				_state = State::CONNECTED;
				if (_debug) {
					CULog("NETCODE: Assigned room %s",_room.c_str());
				}
			}
        } else if (status == "invalid") {
            // The room did not exist
            statech = true;
            _previous = _state;
            _state = State::INVALID;
            _socket->close();
		} else if (status == "denial") {
			// We are not allowed access to that room
			statech = true;
			_previous = _state;
			_state = State::DENIED;
			_socket->close();
		} else if (status == "mismatch") {
			// We do not have the correct API version
			statech = true;
			_previous = _state;
			_state = State::MISMATCHED;
			_socket->close();
		}
		
		// Finish up
		if (response != nullptr) {
			_socket->send(response->toString());		
		}
		if (statech && _onStateChange) {
			callback = [=,this]() {
				_onStateChange(_state);
				return false;
			};
		}
	}

    if (connect) {
        for(auto it = outgoing.begin(); it != outgoing.end(); ++it) {
            offerPeer(*it);
        }
    }

	if (callback) {
        Application::get()->schedule(callback);
	}
}

/**
 * Processes a JSON message that is part of an ongoing game session
 *
 * All websocket messages are JSON objects. Once we parse the bytes into
 * JSON, we can determine the type of message. These messages are custom
 * to our game lobby server, and not part of a typical signaling server.
 *
 * @param json  The message to handle
 */
void NetcodeConnection::handleSession(const std::shared_ptr<JsonValue>&  json) {
 	// Schedulable tasks for main thread
 	std::function<bool()> callback;
 	bool statech = false;
 	
	// Critical section
 	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if (!_active) {
			return;
		}
		
		// Session messages can have category "player" or "session".
		// We use status for further differentiation
		std::string category = json->getString("category");
		std::string status = json->getString("status");
		
		if (category == "player") {
			if (status == "connect") {
				// A player was added to our room
				std::string player = json->getString("player");
			} else if (status == "disconnect") {
				// A (non-host) player was removed from our room
				std::string player = json->getString("player");
				_players.erase(player);
                _peers.erase(player);
				if (_onDisconnect) {
					callback = [=,this]() {
						_onDisconnect(player);
						return false;
					};
				}
			}
		} else if (category == "session") {
			if (status == "start") {
				// The game session has started
				_players.clear();
				auto child = json->get("players");
				for(int ii = 0; ii < child->size(); ii++) {
                    std::string value = child->get(ii)->asString();
                    auto find = _peers.find(value);
                    if (value == _uuid || find != _peers.end()) {
                        _players.emplace(value);
                    }
				}
				_initialPlayers = _players.size();
				statech = true;
				_previous = _state;
				_state = State::INSESSION;
			} else if (status == "shutdown") {
				// The game session has ended (forced shutdown)
				statech = true;
				_previous = _state;
				_state = State::DISCONNECTED;
				_socket->close();
			}
		}

		// Finish up
		if (statech && _onStateChange) {
			callback = [=,this]() {
				_onStateChange(_state);
				return false;
			};
		}
	}

	if (callback) {
        Application::get()->schedule(callback);
	}
}

/**
 * Processes a JSON message that is part of host migration
 *
 * All websocket messages are JSON objects. Once we parse the bytes into
 * JSON, we can determine the type of message. These messages are custom
 * to our game lobby server, and not part of a typical signaling server.
 *
 * @param json  The message to handle
 */
void NetcodeConnection::handleMigration(const std::shared_ptr<JsonValue>&  json) {
    std::shared_ptr<cugl::JsonValue> response = nullptr;

    // Schedulable tasks for main thread
    std::function<bool()> callback;
    bool statech = false;
    bool migrate = false;
    std::string host = "";
    
    // For migration
    std::vector<std::string> to_open;
    std::vector<std::shared_ptr<NetcodePeer>> to_close;
    
    // Critical section
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        if (!_active) {
            return;
        }
       
        // Migration messages can have category "migration" or "promotion".
        // We use status for further differentiation
        std::string category = json->getString("category");
        std::string status = json->getString("status");
        
        if (category == "migration") {
            if (status == "start") {
                // We are starting host migration
                statech = true;
                _previous = _state;
                _state = State::MIGRATING;
            } else if (status == "attempt") {
                _ishost = false;
                _host = json->getString("host");
                _players.clear();
                auto child = json->get("players");
                for(int ii = 0; ii < child->size(); ii++) {
                    _players.emplace(child->get(ii)->asString());
                }
                migrate = true;
                host = _host;
                
                // Determine if any reconfiguration is necessary
                for(auto it = _players.begin(); it != _players.end(); ++it) {
                    std::string uuid = *it;
                    if (uuid != _uuid) {
                        auto jt = _peers.find(uuid);
                        if (jt == _peers.end()) {
                            to_open.push_back(uuid);
                        }
                    }
                }
                
                for(auto it = _peers.begin(); it != _peers.end(); ++it) {
                    auto jt = _players.find(it->first);
                    if (jt == _players.end()) {
                        to_close.push_back(it->second);
                    }
                }
                
            } else if (status == "complete") {
                CULog("NETCODE: Migration complete");
                // Host migration is resolved
                statech = true;
                _state = _previous;
            }
        } else if (category == "promotion") {
            if (status == "query") {
                // We are being asked to promote to the host
                if (_onPromotion) {
                    std::weak_ptr<NetcodeConnection> wp = shared_from_this();
                    callback = [=,this]() {
                        bool result = _onPromotion(false);
                        auto wwp = wp.lock();
                        if (wwp) {
                            std::lock_guard<std::recursive_mutex> lock(wwp->_mutex);
                            auto response = cugl::JsonValue::allocObject();
                            response->appendValue("id",_uuid);
                            response->appendValue("type",std::string("lobby"));
                            response->appendValue("category",std::string("promotion"));
                            response->appendValue("status",std::string("response"));
                            response->appendValue("response",result);
                            wwp->_socket->send(response->toString());
                        }
                        return false;
                    };
                } else {
                    // Automatic reject
                    response = cugl::JsonValue::allocObject();
                    response->appendValue("id",_uuid);
                    response->appendValue("type",std::string("lobby"));
                    response->appendValue("category",std::string("promotion"));
                    response->appendValue("status",std::string("response"));
                    response->appendValue("response",false);
                }
            } else if (status == "confirmed") {
                _ishost = true;
                _host = _uuid;
                _players.clear();
                auto child = json->get("players");
                for(int ii = 0; ii < child->size(); ii++) {
                    _players.emplace(child->get(ii)->asString());
                }
                
                // Determine if any reconfiguration is necessary
                _migration = 0;
                migrate = true;
                for(auto it = _players.begin(); it != _players.end(); ++it) {
                    std::string uuid = *it;
                    if (uuid != _uuid) {
                        auto jt = _peers.find(uuid);
                        if (jt == _peers.end()) {
                            _migration++;
                        }
                    }
                }
                
                for(auto it = _peers.begin(); it != _peers.end(); ++it) {
                    auto jt = _players.find(it->first);
                    if (jt == _players.end()) {
                        to_close.push_back(it->second);
                    }
                }
               
                if (_onPromotion) {
                    std::weak_ptr<NetcodeConnection> wp = shared_from_this();
                    callback = [=,this]() {
                        bool result = _onPromotion(true);
                        if (!result) {
                            auto wwp = wp.lock();
                            if (wwp) {
                                std::lock_guard<std::recursive_mutex> lock(wwp->_mutex);
                                auto response = cugl::JsonValue::allocObject();
                                response->appendValue("id",_uuid);
                                response->appendValue("type",std::string("lobby"));
                                response->appendValue("category",std::string("session"));
                                response->appendValue("status",std::string("shutdown"));
                                wwp->_socket->send(response->toString());
                            }
                        }
                        return false;
                    };
                }
                if (_migration == 0) {
                    response = cugl::JsonValue::allocObject();
                    response->appendValue("id",_uuid);
                    response->appendValue("type",std::string("lobby"));
                    response->appendValue("category",std::string("promotion"));
                    response->appendValue("status",std::string("complete"));
                }
            }
        }
        
        // Finish up
        if (response != nullptr) {
            _socket->send(response->toString());
        }
        if (statech && _onStateChange) {
            callback = [=,this]() {
                _onStateChange(_state);
                return false;
            };
        }
    }
    
    // Reconfigure outside of locks
    if (migrate) {
        for(auto it = to_close.begin(); it != to_close.end(); ++it) {
            (*it)->close();
        }
        for(auto it = to_open.begin(); it != to_open.end(); ++it) {
            offerPeer(*it);
        }
    }
    
    if (callback) {
        Application::get()->schedule(callback);
    }
}

/**
 * Processes a JSON message that comes from a peer connection
 *
 * All websocket messages are JSON objects. Once we parse the bytes into
 * JSON, we can determine the type of message. These messages are standard
 * signaling operations as part of the RTC specification.
 *
 * @param json  The message to handle
 */
void NetcodeConnection::handleSignal(const std::shared_ptr<JsonValue>&  json) {
	std::string id = json->getString("id");
	std::string type = json->getString("type");
	shared_ptr<NetcodePeer> peer;
	
	// Critical Section
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if (auto jt = _peers.find(id); jt != _peers.end()) {
			peer = jt->second;
		}
	}
	
	if (peer == nullptr && type == "offer") {
		// DO NOT HOLD LOCK HERE
		peer = NetcodePeer::alloc(shared_from_this(),id);
		{
			std::lock_guard<std::recursive_mutex> lock(_mutex);
			if (_debug) {
				CULog("NETCODE: Answering offer from %s",id.c_str());
			}
			_peers.emplace(id,peer);		
		}
	}
	
	if (peer == nullptr) {
		return;
	}

	if (type == "offer" || type == "answer") {
		std::string sdp = json->getString("description");
		peer->_connection->setRemoteDescription(rtc::Description(sdp, type));
	} else if (type == "candidate") {
		std::string sdp = json->getString("candidate");
		std::string mid = json->getString("mid");
		peer->_connection->addRemoteCandidate(rtc::Candidate(sdp, mid));
	} 
}

/** 
 * Appends the given data to the ring buffer.
 *
 * This method is used to store an incoming message for later consumption.
 *
 * @param source    The message source
 * @param data      The message data
 *
 * @return if the message was successfully added to the buffer.
 */
bool NetcodeConnection::append(const std::string source, const std::vector<std::byte>& data) {
 	std::function<bool()> callback;
	bool success = false;
	
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if (_active) {
			if (_onReceipt) {
				callback = [=,this]() {
					_onReceipt(source,data);
					return false;
				};
			} else {
				// Buffer it
				if (_buffsize == _buffer.size()) {
					// Drops the oldest message
					_buffhead = ((_buffhead + 1) % _buffer.size());
					_buffsize--;
				}
		
				Envelope* env = &(_buffer[_bufftail]);
				env->source  = source;
				env->message = data;

				_bufftail = ((_bufftail + 1) % _buffer.size());
				_buffsize++;
			}		
			success = true;
		}
	}
	
	if (callback) {
        Application::get()->schedule(callback);
	}
	
	return success;
}

#pragma mark -
#pragma mark Accessors
/**
 * Returns a globally unique UUID representing this connection.
 *
 * While room IDs are assigned by the lobby server, connections must assign
 * their own IDs. The only way to guarantee that this IDs are unique is to
 * use Universally Unique Identifiers (UUID) as defined here:
 *
 *     https://en.wikipedia.org/wiki/Universally_unique_identifier
 *
 * This number is assigned on an open allocation of this connection.
 * Different connections, even on the same device, typically have different
 * UUIDs.
 *
 * This UUID is typically randomly generated (e.g. UUID Version 4). However,
 * it is possible to get a Version 5 UUID by specifying a seed in the
 * {@link NetcodeConfig}. This seed will guarantee that the netcode
 * connection will have the same UUID every time that it attempts to
 * connect.
 *
 * This method is not const because it requires a lock.
 *
 * @return a globally unique UUID representing this connection.
 */
std::string NetcodeConnection::getUUID() {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	return _uuid; 
}

/**
 * Returns the UUID for the (current) game host
 *
 * This method is not const because it requires a lock.
 *
 * @return the UUID for the (current) game host
 */
const std::string NetcodeConnection::getHost() {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	return _host; 
}
    
/**
 * Returns the message buffer capacity.
 *
 * It is possible for this connection to receive several messages over the
 * network before it has a chance to all {@link #receive}. This buffer
 * stores those messages to be read later. The capacity indicates the
 * number of messages that can be stored.
 *
 * Note that this is NOT the same as the capacity of a single message. That
 * value was set as part of the initial {@link NetcodeConfig}.
 *
 * This method is not const because it requires a lock.
 *
 * @return the message buffer capacity.
 */
size_t NetcodeConnection::getCapacity() {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	return _buffer.size();
}

/**
 * Sets the message buffer capacity.
 *
 * It is possible for this connection to receive several messages over the
 * network before it has a chance to all {@link #receive}. This buffer
 * stores those messages to be read later. The capacity indicates the
 * number of messages that can be stored.
 *
 * Note that this is NOT the same as the capacity of a single message. That
 * value was set as part of the initial {@link NetcodeConfig}.
 *
 * @param capacity  The new message buffer capacity.
 */
void NetcodeConnection::setCapacity(size_t capacity) {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	
	// Rotate to correct position
	size_t pos = _buffhead;
	if (capacity < _buffsize) {
		pos = ((_buffhead + (_buffsize-capacity)) % _buffer.size());
		_buffsize = capacity;
	}
	if (pos != 0) {
		std::rotate(_buffer.begin(), _buffer.begin() + pos, _buffer.end());
	}
	_buffhead = 0;
	_bufftail = ((_bufftail + (_buffer.size()-pos)) % _buffer.size());
	_buffer.resize(capacity);
    _bufflimit = capacity;
}
    
/**
 * Returns the list of active players
 *
 * This vector stores the UUIDs of all the players who are currently playing
 * the game. This list will continually update as players join and leave
 * the game.
 *
 * This method is not const because it requires a lock.
 *
 * @return the list of active players
 */
const std::unordered_set<std::string> NetcodeConnection::getPlayers() {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	return _players;
}
    
/**
 * Returns the list of peer connections for this websocket connection
 * 
 * If the connection is a client, there will only be one peer (the host).
 * Otherwise, this list contains all the peer connections to the clients.
 * Most users should never need this method, as all communication should
 * be initiated through the websocket. It is provided for debugging purposes
 * only.
 *
 * This method is not const because it requires a lock.
 *
 * @return the list of peer connections for this websocket connection
 */
const std::unordered_map<std::string, std::shared_ptr<NetcodePeer>> NetcodeConnection::getPeers() {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	return _peers;
}

/**
 * Returns true if the given player UUID is currently connected to the game.
 *
 * This method is not const because it requires a lock.
 *
 * @param player    The player to test for connection
 *
 * @return true if the given player UUID is currently connected to the game.
 */
bool NetcodeConnection::isPlayerActive(const std::string player) {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	auto it = _players.find(player);
	return it != _players.end();
}

/**
 * Returns the number of players currently connected to this game 
 *
 * This does not include any players that have been disconnected.
 *
 * This method is not const because it requires a lock.
 *
 * @return the number of players currently connected to this game 
 */
size_t NetcodeConnection::getNumPlayers() {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	return _players.size();
}

/**
 * Returns the number of players present when the game was started
 *
 * This includes any players that may have disconnected. It returns 0 if
 * the game has not yet started.
 *
 * This method is not const because it requires a lock.
 *
 * @return the number of players present when the game was started
 */
size_t NetcodeConnection::getTotalPlayers() {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	return _initialPlayers;
}

/**
 * Toggles the debugging status of this connection.
 *
 * If debugging is active, connections will be quite verbose
 *
 * @param flag  Whether to activate debugging
 */
void NetcodeConnection::setDebug(bool flag) {
    std::lock_guard<std::recursive_mutex> lock(_mutex);
    _debug = flag;
    for (auto it = _peers.begin(); it != _peers.end(); ++it) {
        it->second->setDebug(flag);
    }
}

#pragma mark -
#pragma mark Communication
/**
 * Opens the connection to the game lobby sever
 *
 * This process is **not** instantaneous. Upon calling this method, you
 * should wait for {@link #getState} or the callback {@link #onStateChange}
 * to return {@link State#CONNECTED}. Once it does, {@link #getRoom} will
 * be your assigned room ID.
 *
 * This method can only be called once. Future calls to this method are
 * ignored. If you need to reopen a closed or failed connection, should
 * should make a new {@link NetcodeConnection} object. This method was only
 * separated from the static allocator so that the user could have the
 * opportunity to register callback functions.
 */
void NetcodeConnection::open() {
	if (_debug) {
		CULog("NETCODE: Socket connection %s allocated",_uuid.c_str());
	}

	_socket = std::make_shared<rtc::WebSocket>();
	_socket->onOpen([this]() { onOpen(); });
	_socket->onError([this](std::string s) { onError(s); });
	_socket->onClosed([this]() { onClosed(); });
	_socket->onMessage([this](auto data) { onMessage(data); });
	
	_buffer.resize(_bufflimit);
	
	// Start the connection
	_active = true;
	_state = State::CONNECTING;

    const std::string prefix = _config.secure ? "wss://" : "ws://";
	const std::string url = prefix + _config.lobby.toString() + "/" + _uuid;
	if (_debug) {
		CULog("NETCODE: Connecting to websocket %s",url.c_str());
	}
	
	_players.emplace(_uuid);
	_socket->open(url);
	if (_debug) {
		CULog("NETCODE: Waiting for lobby '%s' to connect",url.c_str());
	}
}

/**
 * Closes this connection normally.
 *
 * If this method is called on a client, it simply leaves the game; the game
 * can continue without this. If the method is called on the host, shutdown
 * commands are issued to all of the clients. Host migration will never take
 * place when this method is called. Migration requires that the host
 * disconnect without first closing.
 *
 * Because this requires coordination with this connection, this method
 * does not close the connection immediately. Verify that the state is
 * {@link State#DISCONNECTED} before destroying this object.
 */
void NetcodeConnection::close() {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	if (_active) {
		_socket->close();
		_open = false;
	}	
}

/**
 * Sends a byte array to the specified connection.
 *
 * As the underlying connection of this netcode is peer-to-peer, this method
 * can be used to send a communication to any other player in the game,
 * regardless of host status (e.g. two non-host players can communicate
 * this way). If the destination is this connection, the message will be
 * immediately appended to the receipt buffer.
 *
 * Communication from a source is guaranteed to be ordered. So if connection
 * A sends two messages to connection B, connection B will receive those
 * messages in the same order. However, there is no relationship between
 * the messages coming from different sources.
 *
 * You may choose to either send a byte array directly, or you can use the
 * {@link NetcodeSerializer} and {@link NetcodeDeserializer} classes to
 * encode more complex data.
 *
 * This requires a connection be established. Otherwise it will return
 * false. It will also return false if the host is currently migrating.
 *
 * @param dst   The UUID of the peer to receive the message
 * @param data  The byte array to send.
 *
 * @return true if the message was (apparently) sent
 */
bool NetcodeConnection::sendTo(const std::string dst, const std::vector<std::byte>& data) {
	std::shared_ptr<NetcodeChannel> channel;
    bool self = false;
	
	// Critical section
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
        if (_active && _state != State::MIGRATING) {
            self = dst == _uuid;
            if (!self) {
                auto find = _peers.find(dst);
                if (!_active || find == _peers.end()) {
                    CUAssertLog(false,"No direct route to '%s'",dst.c_str());
                    return false;
                } else if (_state == State::MIGRATING) {
                    return false;
                }
                
                // Locking downwards is allowed
                auto peer = find->second;
                std::lock_guard<std::recursive_mutex> sublock(peer->_mutex);
                for(auto jt = peer->_channels.begin();
                    channel == nullptr && jt != peer->_channels.end(); ++jt) {
                    if (jt->first == "public") {
                        channel = jt->second;
                    }
                }
            }
        }
	}
	
    // Do not hold locks on send
    if (self) {
        append(dst,data);
    } else if (channel != nullptr) {
        channel->send(data);
    } else {
        return false;
    }
    return true;
}

/**
 * Sends a byte array to the host player.
 *
 * This method is similar to {@link #sendTo}, except that it always sends to
 * the host player. If this connection is the host, the message will be
 * immediately appended to the receipt buffer.
 *
 * Communication from a source is guaranteed to be ordered. So if connection
 * A sends two messages to connection B, connection B will receive those
 * messages in the same order. However, there is no relationship between
 * the messages coming from different sources.
 *
 * You may choose to either send a byte array directly, or you can use the
 * {@link NetcodeSerializer} and {@link NetcodeDeserializer} classes to
 * encode more complex data.
 *
 * This requires a connection be established. Otherwise it will return
 * false. It will also return false if the host is currently migrating.
 *
 * @param data  The byte array to send.
 *
 * @return true if the message was (apparently) sent
 */
bool NetcodeConnection::sendToHost(const std::vector<std::byte>& data) {
    std::shared_ptr<NetcodeChannel> channel;
    bool self = false;
    std::string uuid;
    
    // Critical section
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        if (_active && _state != State::MIGRATING) {
            self = _host == _uuid;
            uuid = _host;
            if (!self) {
                auto find = _peers.find(_host);
                if (!_active || find == _peers.end()) {
                    CUAssertLog(false,"No direct route to '%s'",_host.c_str());
                    return false;
                } else if (_state == State::MIGRATING) {
                    return false;
                }
                
                // Locking downwards is allowed
                auto peer = find->second;
                std::lock_guard<std::recursive_mutex> sublock(peer->_mutex);
                for(auto jt = peer->_channels.begin();
                    channel == nullptr && jt != peer->_channels.end(); ++jt) {
                    if (jt->first == "public") {
                        channel = jt->second;
                    }
                }
            }
        }
    }
    
    // Do not hold locks on send
    if (self) {
        append(uuid,data);
    } else if (channel != nullptr) {
        channel->send(data);
    } else {
        return false;
    }
    return true;
}

/**
 * Sends a byte array to all other players.
 *
 * Within a few frames, other players should receive this via a call to
 * {@link #receive} or the callback function {@link #onReceipt}. As this
 * is a broadcast message, this player will receive it as well (with the
 * indication of this connection as the sender).
 *
 * As with {@link #sendTo}, communication from a particular source is
 * guaranteed to be ordered. So if connection A broadcasts two messages,
 * all other connections will receive those messages in the same order.
 * However, there is no relationship between the messages coming from
 * different sources.
 *
 * You may choose to either send a byte array directly, or you can use the
 * {@link NetcodeSerializer} and {@link NetcodeDeserializer} classes to
 * encode more complex data.
 *
 * This requires a connection be established. Otherwise it will return false.
 * It will also return false if the host is currently migrating. In fact,
 * to return true, it must succeed on sending to all of the connections.
 *
 * @param data The byte array to send.
 *
 * @return true if the message was (apparently) sent
 */
bool NetcodeConnection::broadcast(const std::vector<std::byte>& data) {
    std::vector<std::shared_ptr<NetcodeChannel>> channels;
    bool success = true;
    std::string uuid;
    {
        // Critical section
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        if (_active && _state != State::MIGRATING) {
            for(auto it = _peers.begin(); it != _peers.end(); ++it) {
                // Locking downwards is allowed
                auto peer = it->second;
                std::lock_guard<std::recursive_mutex> sublock(peer->_mutex);
                for(auto jt = peer->_channels.begin(); jt != peer->_channels.end(); ++jt) {
                    if (jt->first == "public") {
                        channels.push_back(jt->second);
                    }
                }
            }
        } else {
            success = false;
        }
    }
        
    // Do not hold locks on send
    for(auto it = channels.begin(); it != channels.end(); ++it) {
        success = (*it)->send(data) && success;
    }
        
    append(uuid,data);
    return success;
}

/**
 * Receives incoming network messages.
 *
 * When executed, the function `dispatch` willl be called on every received
 * byte array since the last call to {@link #receive}. It is up to you to
 * interpret this data on your own or with {@link NetcodeDeserializer}
 *
 * A network frame can, but need not be, the same as a render frame. Your
 * dispatch function should be prepared to be called multiple times a
 * render frame, or even not at all.
 *
 * If a dispatcher callback has been registered with {@link #onReceipt},
 * this method will never do anything. In that case, messages are not
 * buffered and are processed as soon as they are received. However, this
 * method has the advantage that it can be read on a separate thread.
 *
 * @param dispatcher    The function to process received data
 */
void NetcodeConnection::receive(const Dispatcher& dispatcher) {
    if (dispatcher == nullptr || !_open || _buffsize == 0) {
        return;
    }
    
    // Sigh. Dispatch is also a callback. Cannot hold a lock when we use.
    std::vector<Envelope> messages;
    messages.reserve(_buffsize);
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        size_t limit = _buffsize;
        if (_buffer.empty()) {
            return;
        }
        
        size_t off = 0;
        for (auto it = _buffer.begin()+_buffhead; it != _buffer.end() && off < limit; off++, ++it) {
            messages.emplace_back(std::move(*it));
        }
        // Wrap around ring buffer
        for (auto it = _buffer.begin(); it != _buffer.end() && off < limit; off++, ++it) {
            messages.emplace_back(std::move(*it));
        }
        
        _buffhead = ((_buffhead + off) % _buffer.size());
        _buffsize -= off;
    }
    
    // Now with lock released we can consume messages
    for(auto it = messages.begin(); it != messages.end(); ++it) {
        dispatcher(it->source,it->message);
    }
}

/**
 * Marks the game as started and bans incoming connections.
 *
 * Note: This can only be called by the host. This method is ignored for 
 * clients.
 */
void NetcodeConnection::startSession() {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	if (_active) {
		CUAssertLog(_ishost,"Only a host should execute this method");
		if (_ishost) {
			auto response = cugl::JsonValue::allocObject();
			response->appendValue("id",_uuid);
			response->appendValue("type",std::string("lobby"));
			response->appendValue("category",std::string("session"));
			response->appendValue("status",std::string("request"));
			_socket->send(response->toString());
		}
	}
}

/**
 * Marks the game as completed.
 *
 * This will issue shutdown commands to call clients, disconnecting them
 * from the game.
 *
 * Note: This can only be called by the host. This method is ignored for
 * clients.
 */
void NetcodeConnection::endSession() {
    std::lock_guard<std::recursive_mutex> lock(_mutex);
    if (_active) {
        CUAssertLog(_ishost,"Only a host should execute this method");
        if (_ishost) {
            auto response = cugl::JsonValue::allocObject();
            response->appendValue("id",_uuid);
            response->appendValue("type",std::string("lobby"));
            response->appendValue("category",std::string("session"));
            response->appendValue("status",std::string("shutdown"));
            _socket->send(response->toString());
        }
    }
}
    
#pragma mark -
#pragma mark Callbacks
/**
 * Sets a callback function to invoke on message receipt
 *
 * This callback is alternative to the method {@link #receive}. Instead of
 * buffering messages and calling that method each frame, this callback
 * function will be invoked as soon as the message is received.
 *
 * All callback functions are guaranteed to be called on the main thread.
 * They are called at the start of an animation frame, before the method
 * {@link Application#update(float) }.
 *
 * @param callback  The dispatcher callback
 */
void NetcodeConnection::onReceipt(Dispatcher callback) {
    std::lock_guard<std::recursive_mutex> lock(_mutex);
    _onReceipt = callback;
}

/**
 * Sets a callback function to invoke on player connections
 *
 * The websocket will keep a player aware of any connections that may
 * happen. This callback will update {@link #getPlayers} after any such
 * connection. Hence connections can be detected through polling or this
 * callback interface. If this information is important to you, the
 * callback interface is preferred.
 *
 * All callback functions are guaranteed to be called on the main thread.
 * They are called at the start of an animation frame, before the method
 * {@link Application#update(float) }.
 *
 * @param callback  The connection callback
 */
void NetcodeConnection::onConnect(ConnectionCallback callback) {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	_onConnect = callback;
}

/**
 * Sets a callback function to invoke on player disconnections
 *
 * The websocket will keep a player aware of any disconnections that may
 * happen. This callback will update {@link #getPlayers} after any such
 * disconnection. Hence disconnections can be detected through polling or
 * this callback interface. If this information is important to you, the
 * callback interface is preferred.
 *
 * All callback functions are guaranteed to be called on the main thread.
 * They are called at the start of an animation frame, before the method
 * {@link Application#update(float) }.
 *
 * @param callback  The disconnection callback
 */
void NetcodeConnection::onDisconnect(ConnectionCallback callback) {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	_onDisconnect = callback;
}

/**
 * Sets a callback function to invoke on state changes
 *
 * Monitoring state is one of the most important components of working
 * with a {@link NetcodeConnection}. Many state changes (moving from
 * {@link State#CONNECTED} to {@link State#INSESSION}, or {@link State#INSESSION}
 * to {@link State#MIGRATING}) can happen due to circumstances beyond the
 * control of this connection. It is particularly important to pay
 * attention to the {@link State#MIGRATING} state, as no messages can be
 * sent during that time.
 *
 * State can either be monitored via a callback with this method, or with
 * polling the method {@link #getState}.
 *
 * @param callback  The state change callback
 */
void NetcodeConnection::onStateChange(StateCallback callback) {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	_onStateChange = callback;
}

/**
 * Sets a callback function to invoke on host migration.
 *
 * Host migration occurs if the host quits (even before the play session
 * starts) without first calling {@link #close}. During migration, the game
 * lobby will attempt to chose a host. If present, this callback will be
 * invoked (with the argument set to false) for all surviving clients. If
 * no callback function is registered, it is assumed this connection
 * declines to become host. Otherwise, if the callback returns true, this
 * client will be considered as a candidate for the new host.
 *
 * The game lobby will select a new host if at least one client returns
 * true to the promotion request (otherwise migration fails and all clients
 * are disconnected). If this connection is selected as the new host, the
 * callback will be invoked a second time with the argument set to true. If
 * the callback returns false on that second invocation, it is assumed that
 * the migration failed and all clients are disconnected.
 *
 * Host migration is only designed for isolated disconnects. If any client
 * disconnects during the host migration process, the migration will fail
 * and all clients will be disconnected.
 *
 * All callback functions are guaranteed to be called on the main thread.
 * They are called at the start of an animation frame, before the method
 * {@link Application#update(float) }.
 *
 * @param callback  The promotion callback
 */
void NetcodeConnection::onPromotion(PromotionCallback callback) {
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	_onPromotion = callback;
}
