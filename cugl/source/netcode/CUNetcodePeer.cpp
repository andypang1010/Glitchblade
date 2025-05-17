//
//  CUNetcodePeer.cpp
//  Cornell University Game Library (CUGL)
//
//  This module is part of a Web RTC implementation of the classic CUGL
//  networking library. That library provided connected to a custom game server
//  for matchmaking, and used reliable UDP communication. This version replaces
//  the matchmaking server with a web socket, and uses web socket data channels
//  for communication.
//
//  This module specifically supports the connections between the various devices
//  in the game. A peer is a device that that can send and receive messages from
//  this device, through one or more data channels.
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
//  This class has no public allocators. All allocation takes place in either
//  NetcodePeer or NetcodeConnection.
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
#include <cugl/core/assets/CUJsonValue.h>
#include <cugl/core/util/CUDebug.h>
#include <cstring>

using namespace cugl::netcode;
using namespace std;
using namespace rtc;

#pragma mark Constructors
/**
 * Creates a degenerate RTC peer connection.
 *
 * This object has not been initialized by a {@link NetcodeConnection} and cannot
 * be used.
 *
 * You should NEVER USE THIS CONSTRUCTOR.  Peer connections should be created by
 * a {@link NetcodeConnection} instead.
 */
NetcodePeer::NetcodePeer() : 
	_uuid(""), 
	_connection(nullptr), 
	_offered(false),
	_debug(false),
	_open(false),
	_active(false) {}

/**
 * Deletes this RTC peer connection, disposing all resources
 */
NetcodePeer::~NetcodePeer() {
	dispose();
}

/**
 * Disposes all of the resources used by this RTC peer connection.
 *
 *
 * While we never expect to reinitialize an RTC peer connection, this method
 * allows for a "soft" deallocation, where internal resources are destroyed
 * as soon as a connection is terminated. This simplifies the cleanup process.
 */
void NetcodePeer::dispose() {
	if (!_active) {
		return;
	}
	
	std::shared_ptr<NetcodeConnection> parent = nullptr;
	std::string uuid;

	// ORDER MATTERS HERE (otherwise deadlock)

	// Critical section (shutdown channels first)
	std::unordered_map<std::string, std::shared_ptr<NetcodeChannel>> channels;
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if (_active && !_channels.empty()) {
			// Clearing this would call dispose, which needs a lock
			// Copy it to invoke GC outside of lock
			channels = _channels;
			_channels.clear();
		}
	}
	channels.clear();
		
	// Critical section (now shutdown peer)
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if (_active) {
			_active = false;	// Prevents cycles
			_open   = false;
			_connection->close();
			_connection = nullptr;
			_offered = false;
			
			parent = _parent.lock();
			uuid = _uuid; 
		}
	}
	if (parent != nullptr) {
		parent->onPeerClosed(uuid);
	}
}

/**
 * Initializes a new RTC peer connection channel for the given id.
 *
 * The id should be the value {@link NetcodeConnection#getUUID} for the peer.
 *
 * Offered connections are responsible for creating all data channels. A
 * received connection does not create data channels.
 *
 * @param parent    The parent RTC websocket connection
 * @param id        The unique id for this peer
 * @param offered   Whether this is an offered (not received) connection
 *
 * @return true if initialization was successful
 */
bool NetcodePeer::init(const std::weak_ptr<NetcodeConnection>& parent,
                       const std::string id, bool offered) {
	auto p = parent.lock();
	if (p == nullptr) {
		return false;
	}

	_uuid = id;
	_parent = parent;
	_active = true;
	_offered = offered;
	
	// Atomics.  Safe to get without locks
	rtc::Configuration config = p->_rtcconfig;
	bool debug = p->_debug;
	_debug = debug;
	if (!p->_active) {
		return false;
	}
	
	try {
		_connection = std::make_shared<rtc::PeerConnection>(config);
		_connection->onStateChange([this](rtc::PeerConnection::State state) { 
			onStateChange(state); 
		});
		_connection->onGatheringStateChange([this](rtc::PeerConnection::GatheringState state) { 
			onGatheringStateChange(state); 
		});
		_connection->onLocalDescription([this](rtc::Description description) { 
			onLocalDescription(description); 
		});
		_connection->onLocalCandidate([this](rtc::Candidate candidate) { 
			onLocalCandidate(candidate); 
		});
		_connection->onDataChannel([this](shared_ptr<rtc::DataChannel> dc) { 
			onDataChannel(dc); 
		});
		if (_debug) {
			CULog("NETCODE: Allocated peer connection to %s",_uuid.c_str());
		}
		return true;
	} catch (const std::exception &e) {
		CULogError("NETCODE ERROR: %s",e.what());
		_active = false;
		return false;
	}	
}

#pragma mark -
#pragma mark Internal Callbacks
/**
 * Called when the peer state changes.
 *
 * @param state The new state
 */
void NetcodePeer::onStateChange(rtc::PeerConnection::State state) { 
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if (_debug) {
			CULog("NETCODE: Peer %s changed to state %d",_uuid.c_str(),static_cast<int>(state));
			switch (state) {
			case rtc::PeerConnection::State::Disconnected:
				CULog("NETCODE: Peer %s disconnected",_uuid.c_str());
				break;
			case rtc::PeerConnection::State::Failed:
				CULog("NETCODE: Peer %s failed",_uuid.c_str());
				break;
			case rtc::PeerConnection::State::Closed:
				CULog("NETCODE: Peer %s closed",_uuid.c_str());
				break;
			default:
				break;
			}
		}
	}
	
    // Clean-up shutdowns
	switch (state) {
	case rtc::PeerConnection::State::Disconnected:
	case rtc::PeerConnection::State::Failed:
	case rtc::PeerConnection::State::Closed:
		dispose();
		break;
	default:
		break;
	}
}

/**
 * Called when the peer gathering state changes.
 *
 * @param state The new gathering state
 */
void NetcodePeer::onGatheringStateChange(rtc::PeerConnection::GatheringState state) { 
	if (_debug) {
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		CULog("NETCODE: Peer %s changed to gathering state %d",_uuid.c_str(),static_cast<int>(state));
	}
}

/**
 * Called when the local description changes (usually via websocket)
 *
 * @param description   The new description
 */
void NetcodePeer::onLocalDescription(rtc::Description description) {
	if (_debug) {
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		CULog("NETCODE: Peer %s got a local description",_uuid.c_str());
	}
	
    // To prevent upwards locks
	std::shared_ptr<NetcodeConnection> parent = nullptr;
	auto json = cugl::JsonValue::allocObject();
	
	// Critical section
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if (_active) {
			json->appendValue("id",_uuid);
			json->appendValue("type",description.typeString());
			json->appendValue("description",std::string(description));
			parent = _parent.lock();
		}
	}

	// NEVER lock upwards
	if (parent != nullptr) {
		std::lock_guard<std::recursive_mutex> lock(parent->_mutex);
		parent->_socket->send(json->toString());
	}	
}

/**
 * Called when the local candidate changes (usually via websocket)
 *
 * @param candidate The new candidate
 */
void NetcodePeer::onLocalCandidate(rtc::Candidate candidate) {
	if (_debug) {
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		CULog("NETCODE: Peer %s got a local candidate",_uuid.c_str());
	}

    // To prevent upwards locks
	std::shared_ptr<NetcodeConnection> parent = nullptr;
	auto json = cugl::JsonValue::allocObject();
	
	// Critical section
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if (_active) {
			json->appendValue("id",_uuid);
			json->appendValue("type",std::string("candidate"));
			json->appendValue("candidate",std::string(candidate));
			json->appendValue("mid",candidate.mid());
			parent = _parent.lock();
		}
	}

    // NEVER lock upwards
	if (parent != nullptr) {
		std::lock_guard<std::recursive_mutex> lock(parent->_mutex);
		parent->_socket->send(json->toString());
	}	
}

/**
 * Called when a new data channel is created
 *
 * There is only one data channel of any given label between two peers. But
 * either of the peers could instantiate this channel. This callback is to
 * notify the other peer of its creation.
 *
 * @param dc The new channel
 */
void NetcodePeer::onDataChannel(const shared_ptr<rtc::DataChannel>& dc) {
	std::weak_ptr<NetcodePeer> wp = shared_from_this();

    // DO NOT HOLD LOCK HERE
	std::shared_ptr<NetcodeChannel> channel = NetcodeChannel::alloc(wp,dc);
	
	// Critical section
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if (_active) {
			if (_debug) {
				CULog("NETCODE: Data channel '%s' request received from %s.",dc->label().c_str(),_uuid.c_str());	
			}
			_channels.emplace(dc->label(), channel);
		}
	}
}

#pragma mark -
#pragma mark Netcode Coordination
/**
 * Called when a data channel is closed.
 *
 * This is used to notify the peer to stop tracking this data channel.
 *
 * @param label The data channel label
 */
void NetcodePeer::onChannelClosed(const std::string label) {
	// Critical section
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if (_active) {
			if (_debug) {
				CULog("NETCODE: Peer connection %s cleaned-up data channel '%s'",_uuid.c_str(),label.c_str());
			}
			_channels.erase(label);
		}
	}
}

/**
 * Called when a data channel is opened.
 *
 * In our experiments, it is only safe to open one data channel at a time.
 * This callback informs this peer when it is safe to make a new channel.
 *
 * @param label The data channel label
 */
void NetcodePeer::onChannelOpened(const std::string label) {
	bool offered = false;
	std::string uuid;
	std::shared_ptr<NetcodeConnection> parent;
	// Critical section
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if (_active) {
			if (_debug) {
				CULog("NETCODE: Peer connection %s opened data channel '%s'",_uuid.c_str(),label.c_str());
			}
			offered  = _offered;
			parent = _parent.lock();
			uuid = _uuid;
		}
	}
	if (label == "public") {
		parent->onPeerEstablished(uuid);
	}
}

/**
 * Creates a data channel with the given label
 *
 * There can only be one data channel of any label.
 *
 * @return true if creation was successful.
 */
bool NetcodePeer::createChannel(const std::string label) {
	std::weak_ptr<NetcodePeer> wp = shared_from_this();
    
    // DO NOT HOLD LOCK HERE
    std::shared_ptr<NetcodeChannel> channel = NetcodeChannel::alloc(wp,label);
	
	// Critical section
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if (_active) {
			if (_debug) {
				CULog("NETCODE: Peer connection %s created data channel '%s'",_uuid.c_str(),label.c_str());	
			}
			_channels.emplace(label, channel);
			return true;
		}
	}
	return false;
}

#pragma mark Communication
/**
 * Returns the data channel with the associated label.
 *
 * If there is no such channel, it returns nullptr. Most users should never 
 * need to access this method. It is provided for debugging purposes only.
 *
 * This method is not const because it requires a lock.
 *
 * @return the data channel with the associated label.
 */
const std::shared_ptr<NetcodeChannel> NetcodePeer::getChannel(const std::string channel) {
	std::shared_ptr<NetcodeChannel> stream = nullptr;
	// Critical section
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if (_active) {
			auto it = _channels.find(channel);
			if (it == _channels.end()) {
				return nullptr;
			}
			stream = it->second;	
		}
	}
	return stream;
}

/**
 * Returns the parent {@link NetcodeConnection} of this data channel
 *
 * Most users should never need to access this method. It is provided for 
 * debugging purposes only.
 *
 * This method is not const because it requires a lock.
 *
 * @return the parent {@link NetcodeConnection} of this data channel
 */
const std::shared_ptr<NetcodeConnection> NetcodePeer::getConnection() {
    std::lock_guard<std::recursive_mutex> lock(_mutex);
    return _parent.lock();
}

/**
 * Closes this peer connection.
 *
 * All associated data channels will be destroyed. In addition, this
 * connection will be removed from this parent.
 *
 * @return true if the channel was successfully closed.
 */
bool NetcodePeer::close() {
	// Critical section
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if (_active) {
			_open = false;
			_connection->close();
			return true;
		}
	}
	return false;
}

/**
 * Sends data along the data channel of the given name
 *
 * Most users should never need to access this method. All communication
 * should take place using the associated {@link NetcodeConnection}. It is
 * provided for debugging purposes only.
 *
 * @param channel   The data channel label
 * @param data      The data to send (a string or byte vector)
 *
 * @return true if transmission was (apparently) successful
 */
bool NetcodePeer::send(const std::string channel, const std::vector<std::byte>& data) {
	std::shared_ptr<NetcodeChannel> stream = nullptr;
	std::string uuid  = "";
	
	// Critical section
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if (_active) {
			auto it = _channels.find(channel);
			if (it == _channels.end()) {
				return false;
			}
			uuid = _uuid;
			stream = it->second;	
		}
	}
	
	// Hold no more than one lock at a time
	if (stream != nullptr) {
		if (_debug) {
			CULog("NETCODE: Peer connection %s sending %zu bytes data channel '%s'",uuid.c_str(),data.size(),channel.c_str());
		}
		stream->send(data);
		return true;	
	}
	return false;
}

/**
 * Toggles the debugging status of this peer.
 *
 * If debugging is active, connections will be quite verbose
 *
 * @param flag  Whether to activate debugging
 */
void NetcodePeer::setDebug(bool flag)  { 
	std::lock_guard<std::recursive_mutex> lock(_mutex);
	_debug = flag; 
	for (auto it = _channels.begin(); it != _channels.end(); ++it) {
		it->second->setDebug(flag);
	}
}

