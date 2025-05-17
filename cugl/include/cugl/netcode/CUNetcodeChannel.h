//
//  CUNetcodeChannel.h
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
#ifndef __CU_NETCODE_CHANNEL_H__
#define __CU_NETCODE_CHANNEL_H__
#include <cugl/core/CUBase.h>

#include <rtc/rtc.hpp>
#include <future>
#include <memory>
#include <string>
#include <atomic>
#include <cstddef>
#include <mutex>

namespace cugl {

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
class NetcodePeer;
class NetcodeConnection;

/**
 * This class represents a single data channel
 *
 * Netcode communicates between devices on the network using data channels. A
 * data channel is reliable, high speed communication that happens directly,
 * and does not take place through the lobby server. A data channel is a
 * relationship between two devices, providing bi-directional communication. It
 * is possible for two devices to have more than one data channel between them,
 * such as conversations marked private or public.
 *
 * Users should not create data channels directly, and as such all constructors
 * and allocators for this class are private.  All data channels are associated
 * with a {@link NetcodePeer} and should be constructed from them. We have only
 * exposed this class to simplify development.
 */
class NetcodeChannel {
private:
    /** The name of this data channel */
    std::string _label;
    /** The peer UUID (to prevent an unnecessary "join") */
    std::string _uuid;
    /** The NetcodePeer that owns this data channel. */
    std::weak_ptr<NetcodePeer> _parent;
    /** The NetcodeConnection ultimately associated with this data channel */
    std::weak_ptr<NetcodeConnection> _grandparent;
    /** The associated RTC data channel */
    std::shared_ptr<rtc::DataChannel> _channel;

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
// If I were a better C++ programmer I would do this:
// https://stackoverflow.com/questions/8147027/how-do-i-call-stdmake-shared-on-a-class-with-only-protected-or-private-const
public:
    /**
     * Creates a degenerate RTC data channel.
     *
     * This object has not been initialized by a {@link NetcodePeer} and cannot
     * be used.
     *
     * You should NEVER USE THIS CONSTRUCTOR. All data channels should be
     * created by a {@link NetcodeConnection} instead.
     */
    NetcodeChannel();

    /**
     * Deletes this RTC data channel, disposing all resources
     */
    ~NetcodeChannel();

private:
    /**
     * Disposes all of the resources used by this RTC data channel.
     *
     * While we never expect to reinitialize an RTC data channel, this method
     * allows for a "soft" deallocation, where internal resources are destroyed
     * as soon as a connection is terminated. This simplifies the cleanup process.
     */
    void dispose();

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
    bool init(const std::weak_ptr<NetcodePeer>& parent, std::string label);

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
    bool init(const std::weak_ptr<NetcodePeer>& parent, const std::shared_ptr<rtc::DataChannel>& dc);

    /**
     * Returns a newly allocated RTC data channel for the given label.
     *
     * This initializer assumes the peer is the offerer of the data channel.
     *
     * @param parent    The parent RTC peer connection
     * @param label     The unique label for this data channel
     *
     * @return a newly allocated RTC data channel for the given label.
     */
    static std::shared_ptr<NetcodeChannel> alloc(const std::weak_ptr<NetcodePeer>& parent, std::string label) {
        std::shared_ptr<NetcodeChannel> result = std::make_shared<NetcodeChannel>();
        return (result->init(parent,label) ? result : nullptr);
    }

    /**
     * Returns a newly allocated netcode wrapper for the given RTC data channel.
     *
     * This initializer assumes the peer is the recipient of the data channel.
     *
     * @param parent    The parent RTC peer connection
     * @param dc        The associated RTC data channel
     *
     * @return a newly allocated netcode wrapper for the given RTC data channel.
     */
    static std::shared_ptr<NetcodeChannel> alloc(const std::weak_ptr<NetcodePeer>& parent,
                                                 const std::shared_ptr<rtc::DataChannel>& dc) {
        std::shared_ptr<NetcodeChannel> result = std::make_shared<NetcodeChannel>();
        return (result->init(parent,dc) ? result : nullptr);
    }

#pragma mark Internal Callbacks
    /**
     * Called when the data channel first opens
     */
    void onOpen();

    /**
     * Called when the data channel closes
     */
    void onClosed();

    /**
     * Called when a data channel message is recieved
     *
     * This information will be forwarded to the {@link NetcodeConnection}
     * associated with this data channel. Our netcode classes do not use data
     * channels directly.
     *
     * @param data  The message data (a string or byte vector)
     */
    void onMessage(rtc::message_variant data);

    /** Allow access to the other netcode classes */
    friend class NetcodePeer;
    friend class NetcodeConnection;

#pragma mark Accessors
public:
    /**
     * Returns the label for this data channel.
     *
     * Each peer-to-peer pair has exactly one data channel with this label.
     *
     * @return the label for this data channel.
     */
    const std::string getLabel() const { return _label; }

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
    const std::shared_ptr<NetcodePeer> getPeer();

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
    const std::shared_ptr<NetcodeConnection> getConnection();

#pragma mark Communication
    /**
     * Closes this data channel
     *
     * The data channel will automatically be removed from its parent.
     *
     * @return true if the channel was successfully closed.
     */
    bool close();

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
    bool send(const std::vector<std::byte>& data);

#pragma mark Debugging
    /**
     * Toggles the debugging status of this channel.
     *
     * If debugging is active, connections will be quite verbose
     *
     * @param flag  Whether to activate debugging
     */
    void setDebug(bool flag) { _debug = flag; }

    /**
     * Returns the debugging status of this channel.
     *
     * If debugging is active, connections will be quite verbose
     *
     * @return the debugging status of this channel.
     */
    bool getDebug() const { return _debug; }
};

    }
}

#endif /* __CU_NETCODE_CHANNEL_H__ */
