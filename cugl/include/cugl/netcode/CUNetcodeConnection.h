//
//  CUNetcodeConnection.h
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
#ifndef __CU_NETCODE_CONNECTION_H__
#define __CU_NETCODE_CONNECTION_H__
#include <cugl/netcode/CUNetcodeConfig.h>

#include <rtc/rtc.hpp>
#include <unordered_map>
#include <unordered_set>
#include <functional>
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
class NetcodePeer;

/**
 * This class to supports a connection to other players with a peer-to-peer interface.
 *
 * The premise of this class is to make networking as simple as possible. Simply
 * call {@link #broadcast} with a byte vector, and then all others will receive
 * it when they call {@link #receive}. For more complex types, you can use the
 * classes {@link NetcodeSerializer} and {@link NetcodeDeserializer}.
 *
 * This class maintains a networked game using a peer-to-peer connections. One
 * player is designated as "host", but this is purely an organizational concept.
 * The host monitors the other players, allowing them join. But once the game
 * starts, all communication is peer-to-peer and the question of authority are
 * determined by the application layer.
 *
 * You can use this as a true client-server by replacing all calls to
 * {@link #broadcast} with calls to {@link #sendTo}. That way clients can send
 * to the host and the host can broadcast its responses.
 *
 * Using this class requires an external lobby websocket server to enable Web
 * RTC data channels. This server does not handle actual game data. In only
 * connects that players, an occasionally monitors for disconnects requiring
 * host migration. This reduces server costs significantly. We will provide you
 * a Docker package for a lobby server later in the semester.
 *
 * This class supports optional host migration should the host be lost. Upon
 * loss of the host, each surviving client will receive an invocation of the
 * callback function {@link #onPromotion}. if the callback exists and returns
 * true, that client will become a candidate to be the new host. If more than
 * one client is a candidate, the lobby will chose the first available. If any
 * client disconnects during the migration process, host migration will fail.
 * See {@link #onPromotion} for more details.
 *
 * It is completely unsafe for network connections to be used on the stack.
 * For that reason, this class hides the initialization methods (and the
 * constructors create uninitialized connections). You are forced to go through
 * the static allocator {@link #alloc} to create instances of this class.
 */
class NetcodeConnection : public std::enable_shared_from_this<NetcodeConnection> {
public:
    /**
     * An enum representing the current connection state.
     *
     * This state is the relationship of this connection to the lobby websocket
     * server. The peer connections and data channels have their own separate
     * states.
     */
    enum class State : int {
        /**
         * The connection is initialized, but {@link #open} has not yet been called.
         */
        INACTIVE     = -1,
        /**
         * The connection is in the initial connection phase.
         *
         * This represent the initial handshake with the game lobby server. This state
         * ends when the connection is officially marked as open.
         */
        CONNECTING   = 0,
        /**
         * The connection is negotiating its role with the server (host or client)
         *
         * This state ends when the connection receives a role acknowledgement from
         * the server.
         */
        NEGOTIATING  = 1,
        /**
         * The connection is complete and currently allowing players to join the room.
         *
         * This state ends when connection receives an acknowledgement that the host
         * called the method {@link #startSession}. At which point it will transition
         * to {@link #INSESSION}.
         */
        CONNECTED      = 2,
        /**
         * The connection is actively playing the game.
         *
         * This states ends when the player closes the connection or destroys the
         * socket.
         */
        INSESSION      = 3,
        /**
         * The connection is migrating to a new host.
         *
         * This state is caused when the host connection does not end "cleanly"
         * (e.g. with a call to {@link #close}. No messages can be sent during
         * this state.
         */
        MIGRATING    = 4,
        /**
         * The connection is disconnected.
         *
         * This state occurs when the connection to the lobby is lost. It
         * is typically the result of a call to {@link #close}.
         */
        DISCONNECTED = 5,
        // FAILURE CASES
        /**
         * The connection was denied the option to join a room.
         *
         * This error indicates that the room is full, or the game has started.
         */
        DENIED     = 6,
        /**
         * The connection did not match the host API version
         */
        MISMATCHED = 7,
        /**
         * The client connection specified a non-existent room
         */
        INVALID    = 8,
        /**
         * The connection failed with an unknown error.
         */
        FAILED     = 9,
        /**
         * This object has been disposed and is no longer available for use.
         */
        DISPOSED   = 10
    };

#pragma mark Callbacks
    /**
     * @typedef ConnectionCallback
     *
     * This type represents a callback for the {@link NetcodeConnection} class.
     *
     * This type refers to two different possible callbacks: one when a new peer
     * connection connects and another when it disconnects. This notification
     * goes to all connections, whether they are host or client (so there is no
     * guarantee of a direct connection to the peer). The UUID sent to the
     * callback indentifies the peer that connected/disconnected.
     *
     * Callback functions differ from listeners (found in the input classes) in
     * that only one callback of any type is allowed in a {@link NetcodeConnection}
     * class. Callback functions are guaranteed to be called at the start of an
     * animation frame, before the method {@link Application#update(float) }.
     *
     * The function type is equivalent to
     *
     *      std::function<void(const std::string uuid)>
     *
     * @param uuid  The unique identifier of the disconnecting peer
     */
    typedef std::function<void(const std::string uuid)> ConnectionCallback;

    /**
     * @typedef PromotionCallback
     *
     * This type represents a callback for the {@link NetcodeConnection} class.
     *
     * This callback is invoked when the game lobby makes an offer to this
     * connection to become host, as part of host migration. If the callback
     * returns true, then this connection will become a candidate for the new
     * host. However, selection is not guaranteed, as the server polls all
     * clients simultaneously.
     *
     * If the connection is actually selected as the new host, this callback
     * will be invoked a second time with the parameter set to true. If the
     * callback returns false on the confirmation (because of a change of
     * heart), migration fails and all clients are disconnected.
     *
     * Callback functions differ from listeners (found in the input classes) in
     * that only one callback of any type is allowed in a {@link NetcodeConnection}
     * class. Callback functions are guaranteed to be called at the start of an
     * animation frame, before the method {@link Application#update(float) }.
     *
     * The function type is equivalent to
     *
     *      std::function<bool(bool)>
     *
     * @param confirmed Whether this connection is confirmed as the new host
     *
     * @return true if this connection is willing to become host
     */
    typedef std::function<bool(bool confirmed)> PromotionCallback;

    /**
     * @typedef StateCallback
     *
     * This type represents a callback for the {@link NetcodeConnection} class.
     *
     * This callback is invoked when the connection state has changed. The
     * parameter marks the new connection state. This is particularly helpful
     * for monitoring host migrations.
     *
     * Callback functions differ from listeners (found in the input classes) in
     * that only one callback of any type is allowed in a {@link NetcodeConnection}
     * class. Callback functions are guaranteed to be called at the start of an
     * animation frame, before the method {@link Application#update(float) }.
     *
     * The function type is equivalent to
     *
     *      std::function<void(State state)>
     *
     * @param state The new connection state
     */
    typedef std::function<void(State state)> StateCallback;

    /**
     * @typedef Dispatcher
     *
     * The dispatcher is called by the {@link #receive} function to consume data
     * from the message buffer. Not only does it relay the message data, but it
     * also communicates the "source". For broadcast messages, this will be the
     * value "broadcast". For private messages, it will be the UUID of the
     * sending client. Note that only the host can receive private messages.
     *
     * The function type is equivalent to
     *
     *      const std::function<void(const std::string source,
     *                               const std::vector<std::byte>& message)>
     *
     * @param source    The message source
     * @param message   The message data
     */
    typedef std::function<void(const std::string source, const std::vector<std::byte>& message)> Dispatcher;

private:
    /**
     * A message envelope, storing the message and its receipt
     *
     * As messages come from many different peers, it is helpful to know the
     * sender of each. This information is stored with the message in the ring
     * buffer
     */
    class Envelope {
    public:
        /** The message source */
        std::string source;
        /** The message (as a byte vector) */
        std::vector<std::byte> message;

        /** Creates an empty message envelope */
        Envelope() {}

        /**
         * Creates a copy of the given message envelope
         *
         * @param env the message envelope to copy
         */
        Envelope(const Envelope& env) {
            source  = env.source;
            message = env.message;
        }

        /**
         * Creates a message envelope with the resources of the original
         *
         * @param env the message envelope to acquire
         */
        Envelope(Envelope&& env) {
            source  = std::move(env.source);
            message = std::move(env.message);
         }

        /**
         * Copies the given message envelope
         *
         * @param env the message envelope to copy
         */
        Envelope& operator=(const Envelope& env) {
            source  = env.source;
            message = env.message;
            return *this;
        }

        /**
         * Acquires the resources of the given message envelope
         *
         * @param env the message envelope to acquire
         */
        Envelope& operator=(Envelope&& env) {
            source  = std::move(env.source);
            message = std::move(env.message);
            return *this;
        }
    };

    /** The configuration of this connection */
    NetcodeConfig _config;
    /** The RTC equivalent */
    rtc::Configuration _rtcconfig;

    /** The globally unique identify for this connection */
    std::string _uuid;

    /** The current connection state */
    std::atomic<State> _state;
    /** The previous state (as part of a migration) */
    State _previous;

    /** Whether or not this connection is the host in our ad-hoc server setup */
    std::atomic<bool> _ishost;
    /** The globally unique identifer for the host connection (which may or may not be this) */
    std::string _host;
    /** The room identifier, as assigned by the game lobby */
    std::string _room;

    /** The associated RTC websocket */
    std::shared_ptr<rtc::WebSocket> _socket;
    /** The associated RTC peer connections */
    std::unordered_map<std::string, std::shared_ptr<NetcodePeer>> _peers;
    /** The active connection UUIDs (including this connection) */
    std::unordered_set<std::string> _players;
    /** The total number of players when the game started */
    uint16_t _initialPlayers;

    /* A user defined callback to be invoked when a peer connects. */
    ConnectionCallback _onConnect;
    /* A user defined callback to be invoked when a peer disconnects. */
    ConnectionCallback _onDisconnect;
    /* A user defined callback to be invoked on state changes. */
    StateCallback _onStateChange;
    /* A user defined callback to be invoked if a player is asked to become host. */
    PromotionCallback _onPromotion;
    /** Alternatively make the dispatcher a callback */
    Dispatcher _onReceipt;
    /** A counter to indicate when host migration is complete */
    size_t _migration;

    /**
     * A data ring buffer for incoming messages
     *
     * We do not want to process data as soon as it is received, as that is
     * difficult to synchronize with the animation frame. Instead, we would
     * like to call {@link #receive} as the start of each {@link Application#update}.
     * But this means it is possible to receive multiple network messages
     * before a read. This buffer stores this messages.
     *
     * This is a classic ring buffer. It it fills up (because the application
     * is too slow to read), then the oldest messages are deleted first.
     */
    std::vector<Envelope> _buffer;
    /** The number of messages in the data ring buffer */
    std::atomic<size_t> _buffsize;
    /** The capacity of the data ring buffer */
    size_t _bufflimit;
    /** The head of the data ring buffer */
    size_t _buffhead;
    /** The tail of the data ring buffer */
    size_t _bufftail;

    // To prevent race conditions
    /** Whether this websocket connection prints out debugging information */
    std::atomic<bool> _debug;
    /** Whether this websocket connection is currently open */
    std::atomic<bool> _open;
    /** Whether this websocket connection is currently active (but maybe not yet open) */
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
     * Creates a degenerate netcode connection.
     *
     * This object has not been initialized with a {@link NetcodeConfig} and
     * cannot be used.
     *
     * You should NEVER USE THIS CONSTRUCTOR. All connections should be created
     * by the static constructor {@link #alloc} instead.
     */
    NetcodeConnection();

    /**
     * Deletes this netcode connection, disposing all resources
     */
    ~NetcodeConnection();

private:
    /**
     * Disposes all of the resources used by this netcode connection.
     *
     * While we never expect to reinitialize a new netcode connection, this
     * method allows for a "soft" deallocation, where internal resources are
     * destroyed as soon as a connection is terminated. This simplifies the
     * cleanup process.
     */
    void dispose();

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
    bool init(const NetcodeConfig& config);

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
    bool init(const NetcodeConfig& config, const std::string room);

#pragma mark Internal Callbacks
    /**
     * Called when the web socket first opens
     */
    void onOpen();

    /**
     * Called when the websocket experiences an error
     *
     * @param s The error message
     */
    void onError(const std::string s);

    /**
     * Called when the web socket closes
     */
    void onClosed();

    /**
     * Called when this websocket (and not a peer channel) receives a message
     *
     * @param data  The message received
     */
    void onMessage(rtc::message_variant data);

    /**
     * Called when a peer has established BOTH data channels
     *
     * @param uuid  The UUID of the peer connection
     */
    void onPeerEstablished(const std::string uuid);

    /**
     * Called when a peer connection closes
     *
     * @param uuid  The UUID of the peer connection
     */
    void onPeerClosed(const std::string uuid);

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
    bool offerPeer(const std::string uuid);

    /**
     * Processes a JSON message that is part of the initial room negotiation
     *
     * All websocket messages are JSON objects. Once we parse the bytes into
     * JSON, we can determine the type of message. These messages are custom
     * to our game lobby server, and not part of a typical signaling server.
     *
     * @param json  The message to handle
     */
    void handleNegotiation(const std::shared_ptr<JsonValue>&  json);

    /**
     * Processes a JSON message that is part of an ongoing game session
     *
     * All websocket messages are JSON objects. Once we parse the bytes into
     * JSON, we can determine the type of message. These messages are custom
     * to our game lobby server, and not part of a typical signaling server.
     *
     * @param json  The message to handle
     */
    void handleSession(const std::shared_ptr<JsonValue>&  json);

    /**
     * Processes a JSON message that is part of host migration
     *
     * All websocket messages are JSON objects. Once we parse the bytes into
     * JSON, we can determine the type of message. These messages are custom
     * to our game lobby server, and not part of a typical signaling server.
     *
     * @param json  The message to handle
     */
    void handleMigration(const std::shared_ptr<JsonValue>&  json);

    /**
     * Processes a JSON message that comes from a peer connection
     *
     * All websocket messages are JSON objects. Once we parse the bytes into
     * JSON, we can determine the type of message. These messages are standard
     * signaling operations as part of the RTC specification.
     *
     * @param json  The message to handle
     */
    void handleSignal(const std::shared_ptr<JsonValue>&  json);

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
    bool append(const std::string source, const std::vector<std::byte>& data);

    /** Allow access to the other netcode classes */
    friend class NetcodeManager;
    friend class NetcodeChannel;
    friend class NetcodePeer;

public:
#pragma mark Static Allocators
    /**
     * Returns a newly allocated network connection as host.
     *
     * This method initializes this websocket connection with all of the correct
     * settings. However, it does **not** connect to the game lobby. You must call
     * the method {@link #open} to initiate connection. This design decision is
     * intended to give the user a chance to set the callback functions before
     * connection is established.
     *
     * This method will always return nullptr if the {@link NetworkLayer} failed to
     * initialize.
     *
     * @param config    The connection configuration
     *
     * @return a newly allocated network connection as host.
     */
    static std::shared_ptr<NetcodeConnection> alloc(const NetcodeConfig& config) {
        std::shared_ptr<NetcodeConnection> result = std::make_shared<NetcodeConnection>();
        return (result->init(config) ? result : nullptr);
    }

    /**
     * Returns a newly allocated network connection as a client.
     *
     * This method initializes this websocket connection with all of the correct
     * settings. However, it does **not** connect to the game lobby. You must call
     * the method {@link #open} to initiate connection. This design decision is
     * intended to give the user a chance to set the callback functions before
     * connection is established.
     *
     * The room should match one specified by the host. If you are using the
     * traditional CUGL lobby server, this will be a hexadecimal string.
     *
     * This method will always return nullptr if the {@link NetworkLayer} failed to
     * initialize.
     *
     * @param config    The connection configuration
     * @param room      The host's assigned room id
     *
     * @return a newly allocated network connection as a client.
     */
    static std::shared_ptr<NetcodeConnection> alloc(const NetcodeConfig& config, const std::string room) {
        std::shared_ptr<NetcodeConnection> result = std::make_shared<NetcodeConnection>();
        return (result->init(config,room) ? result : nullptr);
    }

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
    std::string getUUID();

    /**
     * Returns true if this connection is (currently) the game host
     *
     * @return true if this connection is (currently) the game host
     */
    bool isHost() const { return _ishost; }

    /**
     * Returns the UUID for the (current) game host
     *
     * This method is not const because it requires a lock.
     *
     * @return the UUID for the (current) game host
     */
    const std::string getHost();

    /**
     * Returns true if this connection is open
     *
     * Technically a connection is not open if the state is CONNECTING.
     *
     * @return true if this connection is open
     */
    bool isOpen() const { return _open; }

    /**
     * Returns the current state of this connection.
     *
     * Monitoring state is one of the most important components of working
     * with a {@link NetcodeConnection}. Many state changes (moving from
     * {@link State#CONNECTED} to {@link State#INSESSION}, or {@link State#INSESSION}
     * to {@link State#MIGRATING}) can happen due to circumstances beyond the
     * control of this connection. It is particularly important to pay
     * attention to the {@link State#MIGRATING} state, as no messages can be
     * sent during that time.
     *
     * State can either be monitored via polling with this method, or with a
     * callback set to {@link onStateChange}.
     */
    State getState() const { return _state; }

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
    size_t getCapacity();

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
    void setCapacity(size_t capacity);

    /**
     * Returns the room ID or empty string.
     *
     * If this player is a client, this will return the room ID this object was
     * constructed with. Otherwise, as host, this will return the empty string
     * until {@link #getState} is CONNECTED.
     *
     * If this connection is used with the standard CUGL lobby server, then the
     * string will represent a hexadecimal number.
     *
     * @returns the room ID or empty string.
     */
    const std::string getRoom() const { return _room; }

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
    const std::unordered_set<std::string> getPlayers();

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
    const std::unordered_map<std::string, std::shared_ptr<NetcodePeer>> getPeers();

    /**
     * Returns true if the given player UUID is currently connected to the game.
     *
     * This method is not const because it requires a lock.
     *
     * @param player    The player to test for connection
     *
     * @return true if the given player UUID is currently connected to the game.
     */
    bool isPlayerActive(const std::string player);

    /**
     * Returns the number of players currently connected to this game
     *
     * This does not include any players that have been disconnected.
     *
     * This method is not const because it requires a lock.
     *
     * @return the number of players currently connected to this game
     */
    size_t getNumPlayers();

    /**
     * Returns the number of players present when the game was started
     *
     * This includes any players that may have disconnected. It returns 0 if the
     * game has not yet started (e.g. the state is not {@link State#INSESSION}).
     *
     * This method is not const because it requires a lock.
     *
     * @return the number of players present when the game was started
     */
    size_t getTotalPlayers();

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
    void open();

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
    void close();

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
    bool sendTo(const std::string dst, const std::vector<std::byte>& data);

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
    bool sendToHost(const std::vector<std::byte>& data);

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
    bool broadcast(const std::vector<std::byte>& data);

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
    void receive(const Dispatcher& dispatcher);

    /**
     * Marks the game as started and bans incoming connections.
     *
     * Note: This can only be called by the host. This method is ignored for
     * clients.
     */
    void startSession();

    /**
     * Marks the game as completed.
     *
     * This will issue shutdown commands to call clients, disconnecting them
     * from the game.
     *
     * Note: This can only be called by the host. This method is ignored for
     * clients.
     */
    void endSession();

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
    void onReceipt(Dispatcher callback);

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
    void onConnect(ConnectionCallback callback);

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
    void onDisconnect(ConnectionCallback callback);

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
    void onStateChange(StateCallback callback);

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
    void onPromotion(PromotionCallback callback);


#pragma mark Debugging
    /**
     * Toggles the debugging status of this connection.
     *
     * If debugging is active, connections will be quite verbose
     *
     * @param flag  Whether to activate debugging
     */
    void setDebug(bool flag);

    /**
     * Returns the debugging status of this connection.
     *
     * If debugging is active, connections will be quite verbose
     *
     * @return the debugging status of this connection.
     */
    bool getDebug() const { return _debug; }
};

    }
}
#endif
