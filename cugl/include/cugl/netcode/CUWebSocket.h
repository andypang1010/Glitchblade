//
//  CUWebSocket.h
//  Cornell University Game Library (CUGL)
//
//  The class NetcodeConnection is core feature of our networking library.
//  However, since websockets are available, it makes sense to expose those
//  as well. Unlike NetcodeConnection and the CUGL game lobby, this class
//  makes no assumptions about the connection protocol. It can be anything
//  the user wants.
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
//  Note, however, that it is never safe to put this particular object on the
//  stack. therefore, everything except for the static constructors are private.
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
//  Version: 7/6/24
//
#ifndef __CU_WEBSOCKET_H__
#define __CU_WEBSOCKET_H__
#include <cugl/core/CUBase.h>
#include <cugl/netcode/CUInetAddress.h>
#include <functional>
#include <rtc/rtc.hpp>

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

/**
 * This class represents a connection to a central server.
 *
 * The class {@link NetcodeConnection} is built upon WebRTC for high speed
 * communication. A side effect of this is that CUGL has access to websockets,
 * an ubiquitous framework for creating servers. While not as performant as
 * WebRTC, websockets are relatively simple to use, which makes sense to expose
 * them to the rest of the engine.
 *
 * Websockets can still be a little difficult for beginners, as bi-directional
 * communication requires either a multi-threaded or an asynchronous (e.g.
 * coroutines) environment. While these are possible in CUGL, it makes much
 * more sense to synchronize messages receival and dispatch with the game loop,
 * in much the same way that we did for {@link NetcodeConnection}. Hence this
 * class shares a lot of the same features as that class.
 *
 * With that said, this class is much more limited than {@link NetcodeConnection}.
 * The websocket can only talk to one machine at a time. There is some basic
 * connection handling, but no concept of migration or game session management.
 * There is also no UUID for the connected server. The server is identified
 * solely by its URL.
 *
 * The biggest downside of this class is that the websocket server must have
 * a publicly available address to connect to. This is unlikely to be the case
 * between mobile devices (our primary application). That is why a lobby server
 * (like the one used by {@link NetcodeConnection} is so important.
 *
 * In addition, as a layer on top of TCP, the performance of this connection
 * will not be as high as that of {@link NetcodeConnection} which uses UDP
 * style communication.
 *
 * It is completely unsafe for network connections to be used on the stack. For
 * that reason, this class hides the initialization methods (and the constructors
 * create uninitialized connections). You are forced to go through the static
 * allocator {@link #alloc} to create instances of this class.
 */
class WebSocket : public std::enable_shared_from_this<WebSocket> {
public:
    /**
     * An enum representing the current connection state.
     *
     * This state is the relationship of this connection to the websocket server.
     * The peer connections and data channels have their own separate states.
     */
    enum class State : int {
        /**
         * The connection is initialized, but {@link #open} has not yet been called.
         */
        INACTIVE     = -1,
        /**
         * The connection is in the initial connection phase.
         *
         * This represent the initial handshake with the websocket server. This
         * state ends when the connection is officially marked as open.
         */
        CONNECTING   = 0,
        /**
         * The connection is complete and the web socket is ready for use.
         *
         * This state ends when either the connection is broken or the socket
         * is closed at either end (e.g at this end or by the server).
         */
        OPEN         = 2,
        /**
         * The connection is in the process of closing.
         *
         * This states marks the transition period between when a socket started
         * the process of closing, and when it actually marked as closed.
         */
        CLOSING      = 3,
        /**
         * The connection is disconnected.
         *
         * This state occurs when the socket has finished closing and is no
         * longer usable. It is possible to reopen a websocket after it has
         * closed.
         */
        CLOSED        = 4,
        // FAILURE CASES
        /**
         * The connection has failed with an unknown error.
         */
        FAILED        = 5
    };
    
#pragma mark Callbacks
    /**
     * @typedef StateCallback
     *
     * This type represents a callback for the {@link WebSocket} class.
     *
     * This callback is invoked when the connection state has changed. The
     * parameter marks the new connection state. This is particularly helpful
     * for monitoring host migrations.
     *
     * Callback functions differ from listeners (found in the input classes) in
     * that only one callback of any type is allowed in a {@link WebSocket}
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
     * from the message buffer. Unlike {@link NetcodeConnection}, this dispatcher
     * only does it relays the message data since there can only be one source.
     * However, we do include a timestamp of the number of microseconds that
     * have passed since the function {@link NetworkLayer#start} was called.
     *
     * The function type is equivalent to
     *
     *      const std::function<void(const std::vector<std::byte>& message)>
     *
     * @param message   The message data
     * @param time      The number of microseconds since {@link NetworkLayer#start}
     */
    typedef std::function<void(const std::vector<std::byte>& message, Uint64 time)> Dispatcher;
    
private:
    /**
     * A message envelope, storing the message as well as the timestamp of receipt
     */
    class Envelope {
    public:
        /** The message (relative) timestamp */
        Uint64 timestamp;
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
            timestamp  = env.timestamp;
            message = env.message;
        }
        
        /**
         * Creates a message envelope with the resources of the original
         *
         * @param env the message envelope to acquire
         */
        Envelope(Envelope&& env) {
            timestamp = env.timestamp;
            message = std::move(env.message);
        }
        
        /**
         * Copies the given message envelope
         *
         * @param env the message envelope to copy
         */
        Envelope& operator=(const Envelope& env) {
            timestamp = env.timestamp;
            message = env.message;
            return *this;
        }
        
        /**
         * Acquires the resources of the given message envelope
         *
         * @param env the message envelope to acquire
         */
        Envelope& operator=(Envelope&& env) {
            timestamp = env.timestamp;
            message = std::move(env.message);
            return *this;
        }
    };
    
    /** The internet address of the websocket server */
    InetAddress _address;
    /** The "path" defining this connection */
    std::string _path;
    
    /** The current connection state */
    std::atomic<State> _state;
    /** Whether this websocket connection is currently active (but maybe not yet open) */
    std::atomic<bool> _active;
    
    /** The associated RTC websocket */
    std::shared_ptr<rtc::WebSocket> _socket;
    
    /* A user defined callback to be invoked on state changes. */
    StateCallback _onStateChange;
    /** Alternatively make the dispatcher a callback */
    Dispatcher _onReceipt;
    
    /**
     * A data ring buffer for incoming messages
     *
     * We do not want to process data as soon as it is received, as that is difficult
     * to synchronize with the animation frame. Instead, we would like to call
     * {@link #receive} as the start of each {@link Application#update}. But this means
     * it is possible to receive multiple network messages before a read. This buffer
     * stores this messages.
     *
     * This is a classic ring buffer. It it fills up (because the application is too
     * slow to read), then the oldest messages are deleted first.
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
    
    /** A mutex to support locking. This class does not need a recursive one. */
    std::mutex _mutex;
    
#pragma mark Constructors
public:
    /**
     * Creates a degenerate websocket connection.
     *
     * This object has not been initialized with an address and cannot be used.
     *
     * You should NEVER USE THIS CONSTRUCTOR. All connections should be created
     * by the static constructor {@link #alloc} instead.
     */
    WebSocket();
    
    /**
     * Deletes this websocket connection, disposing all resources
     */
    ~WebSocket();
    
private:
    /**
     * Disposes all of the resources used by this websocket connection.
     *
     * While we never expect to reinitialize a new websocket connection, this
     * method allows for a "soft" deallocation, where internal resources are
     * destroyed as soon as a connection is terminated. This simplifies the
     * cleanup process.
     */
    void dispose();
    
    /**
     * Initializes a websocket connection to a server.
     *
     * This method initializes this websocket connection with all of the correct
     * settings. However, it does **not** connect to the server. You must call
     * the method {@link #open} to initiate connection. This design decision is
     * intended to give the user a chance to set the callback functions before
     * connection is established.
     *
     * Websocket servers typically reference their connections by a "path"
     * provided by the user. The path is not an necessary an identifier, as
     * multiple connections can use the same path. It is simply a way of
     * logically grouping connections This path can be any string, though
     * the socket applies a prefix of "/" if it is not already there. This
     * This version uses an empty path.
     *
     * This method will always return false if the {@link NetworkLayer} failed
     * to initialize.
     *
     * @param address   The server internet address
     *
     * @return true if initialization was successful
     */
    bool init(const InetAddress& address) {
        return initWithPath(address,"/");
    }
    
    /**
     * Initializes a websocket connection to a server.
     *
     * This method initializes this websocket connection with all of the correct
     * settings. However, it does **not** connect to the server. You must call
     * the method {@link #open} to initiate connection. This design decision is
     * intended to give the user a chance to set the callback functions before
     * connection is established.
     *
     * Websocket servers typically reference their connections by a "path"
     * provided by the user. The path is not an necessary an identifier, as
     * multiple connections can use the same path. It is simply a way of
     * logically grouping connections This path can be any string, though this
     * method will apply a prefix of "/" if it is not already there.
     *
     * This method will always return false if the {@link NetworkLayer} failed
     * to initialize.
     *
     * @param address   The server internet address
     * @param path      The connection path
     *
     * @return true if initialization was successful
     */
    bool initWithPath(const InetAddress& address, const std::string path);
    
    
#pragma mark Internal Callbacks
    /**
     * Called when the websocket first opens
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
     * Appends the given data to the ring buffer.
     *
     * This method is used to store an incoming message for later consumption.
     *
     * @param data      The message data
     * @param timestamp The number of microseconds since {@link NetcodeLayer#start}
     *
     * @return if the message was successfully added to the buffer.
     */
    bool append(const std::vector<std::byte>& data, Uint64 timestamp);
    
public:
#pragma mark Static Allocators
    /**
     * Returns a newly allocated websocket connection to a server.
     *
     * This method initializes this websocket connection with all of the correct
     * settings. However, it does **not** connect to the server. You must call
     * the method {@link #open} to initiate connection. This design decision is
     * intended to give the user a chance to set the callback functions before
     * connection is established.
     *
     * Websocket servers typically reference their connections by a "path"
     * provided by the user. The path is not an necessary an identifier, as
     * multiple connections can use the same path. It is simply a way of
     * logically grouping connections This path can be any string, though
     * the socket applies a prefix of "/" if it is not already there. This
     * This version uses an empty path.
     *
     * This method will always return false if the {@link NetworkLayer} failed
     * to initialize.
     *
     * @param address   The server internet address
     *
     * @return a newly allocated websocket connection to a server.
     */
    static std::shared_ptr<WebSocket> alloc(const InetAddress& address) {
        std::shared_ptr<WebSocket> result = std::make_shared<WebSocket>();
        return (result->init(address) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated websocket connection to a server.
     *
     * This method initializes this websocket connection with all of the correct
     * settings. However, it does **not** connect to the server. You must call
     * the method {@link #open} to initiate connection. This design decision is
     * intended to give the user a chance to set the callback functions before
     * connection is established.
     *
     * Websocket servers typically reference their connections by a "path"
     * provided by the user. The path is not an necessary an identifier, as
     * multiple connections can use the same path. It is simply a way of
     * logically grouping connections This path can be any string, though this
     * method will apply a prefix of "/" if it is not already there.
     *
     * This method will always return false if the {@link NetworkLayer} failed
     * to initialize.
     *
     * @param address   The server internet address
     * @param path      The connection path
     *
     * @return a newly allocated websocket connection to a server.
     */
    static std::shared_ptr<WebSocket> allocWithPath(const InetAddress& address, const std::string path) {
        std::shared_ptr<WebSocket> result = std::make_shared<WebSocket>();
        return (result->initWithPath(address,path) ? result : nullptr);
    }
    
#pragma mark Accessors
    /**
     * Returns the internet address of this connection
     *
     * @return the internet address of this connection
     */
    const InetAddress& getAddress() const { return _address; }
    
    /**
     * Returns the path for this connection.
     *
     * Websocket servers typically reference their connections by a "path"
     * provided by the user. The path is not an necessary an identifier, as
     * multiple connections can use the same path. It is simply a way of
     * logically grouping connections. This path can be any string, though
     * it must start with a "/".
     *
     * @return the path for this connection.
     */
    const std::string getPath() const { return _path; }
    
    /**
     * Returns true if this connection is open
     *
     * Technically a connection is not open if the state is CONNECTING.
     *
     * This method is not constant because it performs an internal query.
     *
     * @return true if this connection is open
     */
    bool isOpen();
    
    /**
     * Returns the current state of this connection.
     *
     * Monitoring state is one of the most important components of working with
     * a web socket. It is possible for a connection to close remotely, without
     * any input from this side.
     *
     * This method is not constant because it performs an internal query.
     *
     * State can either be monitored via polling with this method, or with a
     * callback set to {@link onStateChange}.
     */
    State getState();
    
    /**
     * Returns the message buffer capacity.
     *
     * It is possible for this connection to receive several messages over the
     * network before it has a chance to all {@link #receive}. This buffer
     * stores those messages to be read later. The capacity indicates the number
     * of messages that can be stored.
     *
     * This method is not const because it requires a lock.
     *
     * @return the message buffer capacity.
     */
    size_t getCapacity();
    
    /**
     * Sets the message buffer capacity.
     *
     * It is possible for this connection to recieve several messages over the
     * network before it has a chance to all {@link #receive}. This buffer stores
     * those messages to be read later. The capacity indicates the number of
     * messages that can be stored.
     *
     * @param capacity  The new message buffer capacity.
     */
    void setCapacity(size_t capacity);
    
    
#pragma mark Communication
    /**
     * Opens the connection to the websocket sever
     *
     * This process is **not** instantaneous. Upon calling this method, you
     * should wait for {@link #getState} or the callback {@link #onStateChange}
     * to return {@link State#OPEN}. Once that happens, it is possible to start
     * communicating with the serve.
     *
     * This method allows us to control the type of connection (ws:// or wss://).
     * Note that a server must have a SSL certificate to support a secure
     * connection.
     *
     * This method will be ignored if the socket is already open. However, it
     * is possible to use this method reopen a closed connection.
     *
     * @param secure Whether to connect to a server via SSL
     */
    void open(bool secure=false);
    
    /**
     * Closes this connection normally.
     *
     * Because this requires coordination with this connection, this method
     * does not close the connection immediately. Verify that the state is
     * {@link State#CLOSED} before destroying this object.
     */
    void close();
    
    /**
     * Sends a byte array to the server.
     *
     * Communication is guaranteed to be ordered. So if this socket sends two
     * messages to the server, the server will receive those messages in the
     * same order.
     *
     * You may choose to either send a byte array directly, or you can use the
     * {@link NetcodeSerializer} and {@link NetcodeDeserializer} classes to
     * encode more complex data.
     *
     * This method requires that the socket be open. Otherwise it will return false.
     *
     * @param data  The byte array to send.
     *
     * @return true if the message was (apparently) sent
     */
    bool send(const std::vector<std::byte>& data);
    
    /**
     * Sends a byte array to the server.
     *
     * Communication is guaranteed to be ordered. So if this socket sends two
     * messages to the server, the server will receive those messages in the
     * same order.
     *
     * You may choose to either send a byte array directly, or you can use the
     * {@link NetcodeSerializer} and {@link NetcodeDeserializer} classes to
     * encode more complex data.
     *
     * This method requires that the socket be open. Otherwise it will return false.
     *
     * @param data  The byte array to send.
     *
     * @return true if the message was (apparently) sent
     */
    bool send(std::vector<std::byte>&& data);
    
    /**
     * Receives incoming network messages.
     *
     * When executed, the function `dispatch` willl be called on every received
     * byte array since the last call to {@link #receive}. It is up to you to
     * interpret this data on your own or with {@link NetcodeDeserializer}
     *
     * A network frame can, but need not be, the same as a render frame. Your
     * dispatch function should be prepared to be called multiple times a render
     * frame, or even not at all.
     *
     * If a dispatcher callback has been registered with {@link #onReceipt},
     * this method will never do anything. In that case, messages are not
     * buffered and are processed as soon as they are received.
     *
     * @param dispatcher    The function to process received data
     */
    void receive(const Dispatcher& dispatcher);
    
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
     * Sets a callback function to invoke on state changes
     *
     * Monitoring state is one of the most important components of working with
     * a web socket. It is possible for a connection to close remotely, without
     * any input from this side.
     *
     * State can either be monitored via a callback with this method, or with a
     * polling the method {@link #getState}.
     *
     * @param callback  The state change callback
     */
    void onStateChange(StateCallback callback);
    
    
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

#endif /* __CU_WEBSOCKET_H__ */
