//
//  CUNetcodePeer.h
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
#ifndef __CU_NETCODE_PEER_H__
#define __CU_NETCODE_PEER_H__
#include <cugl/core/CUBase.h>

#include <rtc/rtc.hpp>
#include <unordered_map>
#include <future>
#include <memory>
#include <string>
#include <atomic>
#include <mutex>

namespace cugl {

/** Forward reference to JsonValues */
class JsonValue;

    /**
     * The classes to support CUGL networking.
     *
     * Currently CUGL supports ad-hoc game lobbies using web-sockets. The
     * sockets must connect connect to a CUGL game lobby server. However,
     * the actual network layer is supported by high speed WebRTC. See
     *
     *     https://libdatachannel.org
     *
     * for an explanation of our networking layer.
     */
    namespace netcode {

/** Forward reference to other netcode classes */
class NetcodeChannel;
class NetcodeConnection;

/**
 * This class represents a peer connection.
 *
 * While {@link NetcodeConnection} connects directly to the websocket server,
 * peer connections are used to connect individual devices P2P (peer-to-peer).
 * Each peer represents a connection between one device and another in the game.
 * In turn, peers have one or more {@link NetcodeChannel} objects to communicate
 * between them.
 *
 * Users should not create peer connections directly, and all constructors and
 * allocators for this class are private. All peer connections are associated
 * with a {@link NetcodeConnection} and should be constructed from them. We
 * have only exposed this class to simplify development.
 */
class NetcodePeer  : public std::enable_shared_from_this<NetcodePeer> {
private:
    /**
     * The globally unique identifier for this peer.
     *
     * This value corresponds to {@link NetcodeConnection#getUUID} of the device
     * that it is connected to.
     */
    std::string _uuid;
    /** Whether this is an offered (as opposed to received) connection */
    bool _offered;

    /** The NetcodePeer that owns this data channel. */
    std::weak_ptr<NetcodeConnection> _parent;
    /** The associated RTC peer connection */
    std::shared_ptr<rtc::PeerConnection> _connection;
    /** The data channels associated with this peer */
    std::unordered_map<std::string, std::shared_ptr<NetcodeChannel>> _channels;

    // To prevent race conditions
    /** Whether this data channel prints out debugging information */
    std::atomic<bool> _debug;
    /** Whether this channel is currently open */
    std::atomic<bool> _open;
    /** Whether this channel is currently active (but not maybe not yet open) */
    std::atomic<bool> _active;
    /**
     * A mutex to support locking
     *
     * A word on reentrant locks. This mutex is recursive, which means that
     * reentrant locks are permitted. However, we have three tightly-coupled
     * classes, each with their own locks. To prevent deadlock from holding
     * multiple locks, we only lock "downward", from NetcodeConnection to
     * NetcodePeer to NetcodeChannel. To lock upwards, a class must release
     * all of its own locks first.
     */
    std::recursive_mutex _mutex;

#pragma mark Constructors
public:
    /**
     * Creates a degenerate RTC peer connection.
     *
     * This object has not been initialized by a {@link NetcodeConnection} and
     * cannot be used.
     *
     * You should NEVER USE THIS CONSTRUCTOR.  Peer connections should be
     * created by a {@link NetcodeConnection} instead.
     */
    NetcodePeer();

    /**
     * Deletes this RTC peer connection, disposing all resources
     */
    ~NetcodePeer();

private:
    /**
     * Disposes all of the resources used by this RTC peer connection.
     *
     *
     * While we never expect to reinitialize an RTC peer connection, this method
     * allows for a "soft" deallocation, where internal resources are destroyed
     * as soon as a connection is terminated. This simplifies the cleanup process.
     */
    void dispose();

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
    bool init(const std::weak_ptr<NetcodeConnection>& parent, const std::string id, bool offered=false);

    /**
     * Returns a newly allocated RTC peer connection channel for the given id.
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
     * @return a newly allocated RTC peer connection channel for the given id.
     */
    static std::shared_ptr<NetcodePeer> alloc(const std::weak_ptr<NetcodeConnection>& parent,
                                              const std::string id, bool offered=false) {
        std::shared_ptr<NetcodePeer> result = std::make_shared<NetcodePeer>();
        return (result->init(parent,id,offered) ? result : nullptr);
    }

#pragma mark Internal Callbacks
    /**
     * Called when the peer state changes.
     *
     * @param state The new state
     */
    void onStateChange(rtc::PeerConnection::State state);

    /**
     * Called when the peer gathering state changes.
     *
     * @param state The new gathering state
     */
    void onGatheringStateChange(rtc::PeerConnection::GatheringState state);

    /**
     * Called when the local description changes (usually via websocket)
     *
     * @param description   The new description
     */
    void onLocalDescription(rtc::Description description);

    /**
     * Called when the local candidate changes (usually via websocket)
     *
     * @param candidate The new candidate
     */
    void onLocalCandidate(rtc::Candidate candidate);

    /**
     * Called when a new data channel is created
     *
     * There is only one data channel of any given label between two peers. But
     * either of the peers could instantiate this channel. This callback is to
     * notify the other peer of its creation.
     *
     * In our netcode, this is only called if this is a "received" peer
     * connection.
     *
     * @param candidate The new channel
     */
    void onDataChannel(const std::shared_ptr<rtc::DataChannel>& candidate);

#pragma mark Netcode Coordination
    /**
     * Called when a data channel is closed.
     *
     * This is used to notify the peer to stop tracking this data channel.
     *
     * @param label The data channel label
     */
    void onChannelClosed(const std::string label);

    /**
     * Called when a data channel is opened.
     *
     * In our experiments, it is only safe to open one data channel at a time.
     * This callback informs this peer when it is safe to make a new channel.
     *
     * @param label The data channel label
     */
    void onChannelOpened(const std::string label);

    /**
     * Creates a data channel with the given label
     *
     * There can only be one data channel of any label.
     *
     * @return true if creation was successful.
     */
    bool createChannel(const std::string label);

    /** Allow access to the other netcode classes */
    friend class NetcodeChannel;
    friend class NetcodeConnection;

public:
#pragma mark Accessors
    /**
     * Returns the UUID of this peer
     *
     * This value should correspond to {@link NetcodeConnection#getUUID} of the
     * connected peer.
     *
     * @return the UUID of this peer
     */
    std::string getUUID() const { return _uuid; }

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
    const std::shared_ptr<NetcodeChannel> getChannel(const std::string channel);

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
    const std::shared_ptr<NetcodeConnection> getConnection();

#pragma mark Communication
    /**
     * Closes this peer connection.
     *
     * All associated data channels will be destroyed. In addition, this
     * connection will be removed from this parent.
     *
     * @return true if the channel was successfully closed.
     */
    bool close();

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
    bool send(const std::string channel, const std::vector<std::byte>& data);

#pragma mark Debugging
    /**
     * Toggles the debugging status of this peer.
     *
     * If debugging is active, connections will be quite verbose
     *
     * @param flag  Whether to activate debugging
     */
    void setDebug(bool flag);

    /**
     * Returns the debugging status of this peer.
     *
     * If debugging is active, connections will be quite verbose
     *
     * @return the debugging status of this channel.
     */
    bool getDebug() const { return _debug; }

};

    }
}
#endif /* __CU_NETCODE_PEER_H__ */
