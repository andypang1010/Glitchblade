//
//  CUNetcodeChannel.cpp
//  Cornell University Game Library (CUGL)
//
//  This module is part of a Web RTC implementation of the classic CUGL
//  networking library. That library provided connected to a custom game server
//  for matchmaking, and used reliable UDP communication. This version replaces
//  the matchmaking server with a web socket, and uses web socket data channels
//  for communication.
//
//  This module specifically supports the data channels between the various
//  devices in the game. It is possible for a device to support multiple
//  communication channels, even with respect to just one other device on the
//  network.
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
#include <vector>

using namespace cugl::netcode;
using namespace std;
using namespace rtc;

#pragma mark Constructors
/**
 * Creates a degenerate RTC data channel.
 *
 * This object has not been initialized by a {@link NetcodePeer} and cannot be used.
 *
 * You should NEVER USE THIS CONSTRUCTOR. All data channels should be created by
 * a {@link NetcodeConnection} instead.
 */
NetcodeChannel::NetcodeChannel() : 
	_label(""), 
	_channel(nullptr), 
	_debug(false),
	_active(false),
	_open(false) {
}

/**
 * Deletes this RTC data channel, disposing all resources
 */
NetcodeChannel::~NetcodeChannel() { dispose(); }

/**
 * Disposes all of the resources used by this RTC data channel.
 *
 * While we never expect to reinitialize an RTC data channel, this method allows for
 * a "soft" deallocation, where internal resources are destroyed as soon as a connection
 * is terminated. This simplifies the cleanup process.
 */
void NetcodeChannel::dispose() {
	// Paranoid about reentrant locks
	std::shared_ptr<NetcodePeer> peer = nullptr;
	std::string label = "";
	
	// Critical section
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if (_active) {
			_active = false;	// Prevents cycles
			_channel->close();
			_channel = nullptr;

			peer  = _parent.lock();
			label = _label;
			_open = false;
		}
	}
	if (peer != nullptr) {
		peer->onChannelClosed(label);
	}
}

/**
 * Initializes a new RTC data channel for the given label.
 *
 * This initializer assumes the peer is the offerer of the data channel.
 *
 * @param parent    The parent RTC peer connection
 * @param label     The unique label for this data channel
 *
 * @return true if initialization was successful
 */
bool NetcodeChannel::init(const std::weak_ptr<NetcodePeer>& parent, std::string label) {
	auto p = parent.lock();
	if (p == nullptr) {
		return false;
	}

	_parent = parent;
	 std::shared_ptr<rtc::PeerConnection> connection;
	{
		// Locking "up" safe for raw variable access
		std::lock_guard<std::recursive_mutex> lock(p->_mutex);
		if (p->_active) {
			_grandparent = p->_parent;
			_uuid  = p->_uuid;
			bool debug = p->_debug;
			_debug = debug;
			connection = p->_connection;
		}
	}

	if (connection == nullptr) {
		return false;
	}
	
	_label = label;
	_active = true;

    if (_debug) {
        CULog("NETCODE: Offered data channel '%s' from %s",_label.c_str(),_uuid.c_str());
    }
	try {
		_channel = connection->createDataChannel(label);
		_channel->onOpen([this]() { onOpen(); });
		_channel->onClosed([this]() { onClosed(); });
		_channel->onMessage([this](auto data) { onMessage(data); });
		return true;
	} catch (const std::exception &e) {
		CULogError("NETCODE ERROR: %s",e.what());
		_active = false;
		return false;
	}	
}

/**
 * Initializes a new netcode wrapper for the given RTC data channel.
 *
 * This initializer assumes the peer is the recipient of the data channel.
 *
 * @param parent    The parent RTC peer connection
 * @param dc        The associated RTC data channel
 *
 * @return true if initialization was successful
 */
bool NetcodeChannel::init(const std::weak_ptr<NetcodePeer>& parent, const std::shared_ptr<rtc::DataChannel>& dc) {
	auto p = parent.lock();
	if (p == nullptr) {
		return false;
	} else if (dc == nullptr) {
		return false;
	}
	
	_parent = parent;
	{
		// Locking "up" safe for raw variable access
		std::lock_guard<std::recursive_mutex> lock(p->_mutex);
		if (p->_active) {
			bool debug = p->_debug;
			_debug = debug;
			_uuid  = p->_uuid;
			_grandparent = p->_parent;
		} else {
			return false;
		}
	}
	_label = dc->label();
    _active = true;
	if (_debug) {
		CULog("NETCODE: Received data channel '%s' from %s",_label.c_str(),_uuid.c_str());
	}

	_channel = dc;
	_channel->onOpen([this]() { onOpen(); });
	_channel->onClosed([this]() { onClosed(); });
	_channel->onMessage([this](auto data) { onMessage(data); });
	return true;
}

#pragma mark -
#pragma mark Internal Callbacks
/**
 * Called when the data channel first opens
 */
void NetcodeChannel::onOpen() {
	std::shared_ptr<NetcodePeer> parent = nullptr;
	std::string label;
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if (_debug) {
			CULog("NETCODE: Data channel '%s' to %s successfully opened.",_label.c_str(),_uuid.c_str());
		}
		parent = _parent.lock();
		label  = _label;
	}
    // Announce a successful connection
	if (parent != nullptr) {
		parent->onChannelOpened(label);
	}
	
}

/**
 * Called when the data channel closes
 */
void NetcodeChannel::onClosed() {
	if (_debug) {
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		CULog("NETCODE: Data channel '%s' to %s closed.",_label.c_str(),_uuid.c_str());
	}
	dispose();
}
	
/**
 * Responds to a data channel message
 *
 * This information will be forwarded to the {@link NetcodeConnection} associated
 * with this data channel.
 *
 * @param data  The message data (a string or byte vector) 
 */
void NetcodeChannel::onMessage(rtc::message_variant data) {
	std::shared_ptr<NetcodeConnection> grand = nullptr;
	std::string source = _uuid;
	
	// Critical section	
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if (_active && std::holds_alternative<rtc::binary>(data)) {
			grand  = _grandparent.lock();
			source = _uuid;
		}
	}
	
	// NEVER lock upwards
	if (grand != nullptr) {
		grand->append(source,std::get<rtc::binary>(data));	
	}
}

#pragma mark -
#pragma mark Communication
/**
 * Returns the parent {@link NetcodePeer} of this data channel
 *
 * Most users should never need to access this method. All communication
 * should take place using the associated {@link NetcodeConnection}. It is
 * provided for debugging purposes only.
 *
 * This method is not const because it requires a lock.
 *
 * @return the parent {@link NetcodePeer} of this data channel
 */
const std::shared_ptr<NetcodePeer> NetcodeChannel::getPeer() {
    std::lock_guard<std::recursive_mutex> lock(_mutex);
    return _parent.lock();
}

/**
 * Returns the {@link NetcodeConnection} associated with this data channel
 *
 * Most users should never need to access this method. It is provided for
 * debugging purposes only.
 *
 * This method is not const because it requires a lock.
 *
 * @return the {@link NetcodeConnection} associated with this data channel
 */
const std::shared_ptr<NetcodeConnection> NetcodeChannel::getConnection() {
    std::lock_guard<std::recursive_mutex> lock(_mutex);
    return _grandparent.lock();
}

/**
 * Closes this data channel
 *
 * The data channel will automatically be removed from its parent.
 *
 * @return true if the channel was successfully closed.
 */
bool NetcodeChannel::close()  {
	// Critical section	
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if (_active) {
			_open = false;
			_channel->close();
			return true;
		}
	}
	
	return false;
}

/**
 * Sends data along this data channel to its recipient
 *
 * Most users should never need  to access this method. All communication
 * should take place using the associated {@link NetcodeConnection}. It is
 * provided for debugging purposes only.
 *
 * @param data  The data to send
 *
 * @return true if transmission was (apparently) successful
 */
bool NetcodeChannel::send(const std::vector<std::byte>& data) {
	// Critical section	
	{
		std::lock_guard<std::recursive_mutex> lock(_mutex);
		if (_active) {
			_channel->send(data);
			return true;
		}
	}
	
	return false;		
}
