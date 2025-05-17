//
//  CUWebsocketServer.h
//  Cornell University Game Library (CUGL)
//
//  The class NetcodeConnection is core feature of our networking library.
//  However, since websocket servers are available, it makes sense to expose
//  those as well. Unlike NetcodeConnection and the CUGL game lobby, this
//  class makes no assumptions about the connection protocol. It can be
//  anything the user wants.
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
#ifndef __CU_WEBSOCKET_SERVER_H__
#define __CU_WEBSOCKET_SERVER_H__
#include <cugl/netcode/CUWebSocketConfig.h>
#include <cugl/core/CUBase.h>
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
    
/** Forward reference to opaque class */
class WebSocketWrapper;

/**
 * This class to supports a long running websocket server.
 *
 * The class {@link NetcodeConnection} is built upon WebRTC for high speed
 * communication. A side effect of this is that CUGL has access to websockets,
 * an ubiquitous framework for creating servers. While not as performant as
 * WebRTC, websockets are relatively simple to use, which makes sense to expose
 * them to the rest of the engine. This particular class allows you to set up
 * a server inside of CUGL. When combined with headless mode, this make it
 * possible to deploy an instance of CUGL on the cloud.
 *
 * Websockets can still be a little difficult for beginners, as bi-directional
 * communication requires either a multi-threaded or an asynchronous (e.g.
 * coroutines) environment. While these are possible in CUGL, it makes much
 * more sense to synchronize messages receival and dispatch with the game loop,
 * in much the same way that we did for {@link NetcodeConnection}. Hence this
 * class shares a lot of the same features as that class.
 *
 * In particular, this class is as if the application were running an instance
 * of {@link NetcodeConnection} in host mode. It is periodically notified of
 * connections (and disconnections). And it can receive messages from each
 * connection and identify the source of the message. And it can send messages
 * either to a single connection, or broadcast to a group of them.
 *
 * There are some important difference from {@link NetcodeConnection}, however.
 * There is no lobby, as this class acts as the server lobby. That means this
 * device must have a publicly available address to connect to. This is unlikely
 * to be the case between mobile devices (our primary application).
 *
 * In addition, as a layer on top of TCP, the performance of this connection
 * will not be as high as that of {@link NetcodeConnection} which uses UDP
 * style communication.
 *
 * One last difference is how this class refers to its connections. Unlike
 * {@link NetcodeConnection}, we do not use UUIDs to reference connections.
 * Instead a connection is refered to by its remote internet address (address
 * and port) plus the connection timestamp in hexadecimal form.
 *
 * In addition, all connections have a path, which is a string resembling a
 * file name path. In {@link NetcodeConnection} we used these paths to
 * differentiate connections. However, paths do not have to be unique. They
 * are simply a way of grouping together connections of similar functionality.
 *
 * It is completely unsafe for network connections to be used on the stack.
 * For that reason, this class hides the initialization methods (and the
 * constructors create uninitialized connections). You are forced to go through
 * the static allocator {@link #alloc} to create instances of this class.
 */
class WebSocketServer : public std::enable_shared_from_this<WebSocketServer> {
public:
#pragma mark Callbacks
    /**
     * @typedef ConnectionCallback
     *
     * This type represents a callback for the {@link WebSocketServer} class.
     *
     * This type refers to two different possible callbacks: one when a client
     * connects and another when it disconnects. In each case the client id
     * and path are identified. The client id is a string built from its remote
     * internet address (as seen by the server) and the timestamp of connection
     * in hexadecimal form.
     *
     * Callback functions differ from listeners (found in the input classes) in
     * that only one callback of any type is allowed in a {@link WebSocketServer}
     * class. Callback functions are guaranteed to be called at the start of an
     * animation frame, before the method {@link Application#update(float) }.
     *
     * The function type is equivalent to
     *
     *      std::function<void(const std::string client, const std::string path)>
     *
     * @param client    The client identifier
     * @param path      The connection path
     */
    typedef std::function<void(const std::string client, const std::string path)> ConnectionCallback;
    
    /**
     * @typedef Dispatcher
     *
     * The dispatcher is called by the {@link #receive} function to consume data
     * from the message buffer. Not only does it relay the message data, but it
     * also communicates the client that sent it. We also include a timestamp
     * of the number of microseconds that have passed since the function
     * {@link NetworkLayer#start} was called.
     *
     * The function type is equivalent to
     *
     *      const std::function<void(const std::string client,
     *                               const std::vector<std::byte>& message
     *                               Uint64 time)>
     *
     * @param client    The client identifier
     * @param message   The message data
     * @param time      The number of microseconds since {@link NetworkLayer#start}
     */
    typedef std::function<void(const std::string source, const std::vector<std::byte>& message, Uint64 time)> Dispatcher;
     
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
        /** The message (relative) timestamp */
        Uint64 timestamp;
        /** The client identifier */
        std::string client;
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
            timestamp = env.timestamp;
            client  = env.client;
            message = env.message;
        }
        
        /**
         * Creates a message envelope with the resources of the original
         *
         * @param env the message envelope to acquire
         */
        Envelope(Envelope&& env) {
            timestamp = env.timestamp;
            client  = std::move(env.client);
            message = std::move(env.message);
         }
        
        /**
         * Copies the given message envelope
         *
         * @param env the message envelope to copy
         */
        Envelope& operator=(const Envelope& env) {
            timestamp = env.timestamp;
            client  = env.client;
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
            client  = std::move(env.client);
            message = std::move(env.message);
            return *this;
        }
    };
    
    /** The configuration of this connection */
    WebSocketConfig _config;
    /** The RTC equivalent */
    rtc::WebSocketServerConfiguration _rtcconfig;

    /** The associated RTC websocket server */
    std::shared_ptr<rtc::WebSocketServer> _server;
    /** The associated RTC socket connections */
    std::unordered_map<size_t, std::shared_ptr<WebSocketWrapper>> _connections;
    /** The connection paths */
    std::unordered_map<std::string,std::unordered_set<WebSocketWrapper*>> _paths;
    /** The connection keys */
    std::unordered_map<std::string,size_t> _keymap;

    /* A user defined callback to be invoked when a peer connects. */
    ConnectionCallback _onConnect;
    /* A user defined callback to be invoked when a peer disconnects. */
    ConnectionCallback _onDisconnect;
    /** Alternatively make the dispatcher a callback */
    Dispatcher _onReceipt;

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
    /** Whether this websocket server is currently active (but maybe not yet started) */
    std::atomic<bool> _active;
    
    /** A mutex to support locking. This class does not need a recursive one. */
    std::mutex _mutex;
    
#pragma mark Constructors
public:
    /**
     * Creates a degenerate websocket server
     *
     * This object has not been initialized with a {@link WebSocketConfig} and
     * cannot be used.
     *
     * You should NEVER USE THIS CONSTRUCTOR. All connections should be created
     * by the static constructor {@link #alloc} instead.
     */
    WebSocketServer();
    
    /**
     * Deletes this websocket server, disposing all resources
     */
    ~WebSocketServer();
    
private:
    /**
     * Disposes all of the resources used by this websocket server.
     *
     * While we never expect to reinitialize a new websocket server, this
     * method allows for a "soft" deallocation, where internal resources are
     * destroyed as soon as a connection is terminated. This simplifies the
     * cleanup process.
     */
    void dispose();

    /**
     * Initializes a new websocket server.
     *
     * This method initializes this websocket server with all of the correct
     * settings. However, it does **not** actually start the server. You must
     * call the method {@link #start} to allow incoming connections. This design
     * decision is intended to give the user a chance to set the callback
     * functions before connection is established.
     *
     * This method will always return false if the {@link NetworkLayer} failed
     * to initialize.
     *
     * @param config    The server configuration
     *
     * @return true if initialization was successful
     */
    bool init(const WebSocketConfig& config);
    
    /**
     * Initializes a new websocket server.
     *
     * This method initializes this websocket server with all of the correct
     * settings. However, it does **not** actually start the server. You must
     * call the method {@link #start} to allow incoming connections. This design
     * decision is intended to give the user a chance to set the callback
     * functions before connection is established.
     *
     * This method will always return false if the {@link NetworkLayer} failed
     * to initialize.
     *
     * @param config    The server configuration
     *
     * @return true if initialization was successful
     */
    bool init(WebSocketConfig&& config);


#pragma mark Internal Callbacks
    /**
     * Called when a client connects to this server.
     *
     * This function appends the socket to the connection set using its
     * pointer address as key (as it is too early to have any other information)
     * That means it will not register as a connection to the user yet.
     *
     * @param socket    The socket to register
     */
    void onClient(std::shared_ptr<rtc::WebSocket> socket);

    /**
     * Called when the web socket opens.
     *
     * The key here is used to reference the pointer address of the socket.
     *
     * @param key   The pointer address key
     */
    void onOpen(size_t key);
    
    /**
     * Called when a websocket experiences an error.
     *
     * The key here is used to reference the pointer address of the socket.
     *
     * @param key   The pointer address key
     * @param s     The error message
     */
    void onError(size_t key, const std::string s);
    
    /**
     * Called when a web socket closes
     *
     * The key here is used to reference the pointer address of the socket.
     *
     * @param key   The pointer address key
     */
    void onClosed(size_t key);
    
    /**
     * Called when this websocket (and not a peer channel) receives a message
     *
     * @param client    The messaging client
     * @param data      The message received
     */
    void onMessage(const std::string client, rtc::message_variant data);
    
    /**
     * Appends the given data to the ring buffer.
     *
     * This method is used to store an incoming message for later consumption.
     *
     * @param client    The message client
     * @param data      The message data
     * @param timestamp The number of microseconds since {@link NetcodeLayer#start}
     *
     * @return if the message was successfully added to the buffer.
     */
    bool append(const std::string client, const std::vector<std::byte>& data, Uint64 timestamp);
    
public:
#pragma mark Static Allocators
    /**
     * Returns a newly allocated websocket server.
     *
     * This method initializes this websocket server with all of the correct
     * settings. However, it does **not** actually start the server. You must
     * call the method {@link #start} to allow incoming connections. This design
     * decision is intended to give the user a chance to set the callback
     * functions before connection is established.
     *
     * This method will always return false if the {@link NetworkLayer} failed
     * to initialize.
     *
     * @param config    The server configuration
     *
     * @return a newly allocated websocket server.
     */
    static std::shared_ptr<WebSocketServer> alloc(const WebSocketConfig& config) {
        std::shared_ptr<WebSocketServer> result = std::make_shared<WebSocketServer>();
        return (result->init(config) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated websocket server.
     *
     * This method initializes this websocket server with all of the correct
     * settings. However, it does **not** actually start the server. You must
     * call the method {@link #start} to allow incoming connections. This design
     * decision is intended to give the user a chance to set the callback
     * functions before connection is established.
     *
     * This method will always return false if the {@link NetworkLayer} failed
     * to initialize.
     *
     * @param config    The server configuration
     *
     * @return a newly allocated websocket server.
     */
    static std::shared_ptr<WebSocketServer> alloc(WebSocketConfig&& config) {
        std::shared_ptr<WebSocketServer> result = std::make_shared<WebSocketServer>();
        return (result->init(std::move(config)) ? result : nullptr);
    }

#pragma mark Accessors
    /**
     * Returns true if this server is active and accepting connections.
     *
     * @return true if this server is active and accepting connections.
     */
    bool isActive() const { return _active; }
    
    /**
     * Returns the port for this server
     *
     * @return the port for this server
     */
    uint16_t getPort() const;
    
    /**
     * Returns the message buffer capacity.
     *
     * It is possible for this connection to receive several messages over the
     * network before it has a chance to all {@link #receive}. This buffer
     * stores those messages to be read later. The capacity indicates the number
     * of messages that can be stored.
     *
     * Note that this is NOT the same as the capacity of a single message. That 
     * value was set as part of the initial {@link WebSocketConfig}.
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
     * stores those messages to be read later. The capacity indicates the number
     * of messages that can be stored.
     *
     * Note that this is NOT the same as the capacity of a single message. That
     * value was set as part of the initial {@link WebSocketConfig}.
     *
     * @param capacity  The new message buffer capacity.
     */
    void setCapacity(size_t capacity);
    
    /**
     * Returns the list of active connections
     *
     * This vector stores the identifiers of all the connections. These
     * identifiers are a string built from the connection remote internet
     * address (as seen by the server) and the timestamp of connection in
     * hexadecimal form.
     *
     * This method is not const because it requires a lock.
     *
     * @return the list of active connections
     */
    const std::unordered_set<std::string> getConnections();

    /**
     * Returns the list of active connections for the given path
     *
     * This vector stores the identifiers of all the connections. These
     * identifiers are a string built from the connection remote internet
     * address (as seen by the server) and the timestamp of connection in
     * hexadecimal form.
     *
     * This method is not const because it requires a lock.
     *
     * @param path  The path to query
     *
     * @return the list of active connections for the given path
     */
    const std::unordered_set<std::string> getConnections(std::string path);

    /**
     * Returns true if the given client is currently connected.
     *
     * A client id is a string built from its remote internet address (as seen
     * by the server) and the timestamp of connection in hexadecimal form.
     *
     * This method is not const because it requires a lock.
     *
     * @param client    The player to test for connection
     *
     * @return true if the given client is currently connected.
     */
    bool isConnected(const std::string client);

    /**
     * Returns the number of clients currently connected to this server
     *
     * This does not include any clients that have been disconnected.
     *
     * This method is not const because it requires a lock.
     *
     * @return the number of clients currently connected to this server
     */
    size_t getNumConnections();
    
#pragma mark Communication
    /**
     * Starts this websocket server, allowing it to receive incoming connections.
     *
     * The server start is instantaneous. It is not like a {@link WebSocket} or
     * {@link NetcodeConnection} where we have to wait for a negotitation to
     * complete.
     *
     * Calling this method on an active websocket server does nothing. However,
     * it is possible to use this method to restart a server that previously
     * was shutdown by {@link #stop}.
     */
    void start();
    
    /**
     * Stops this websocket server, closing all connections.
     *
     * Shutdown of a websocket server is immediate, and all connections are
     * closed. However, it is possible to restart the server (with no initial
     * connections) using {@link #start}.
     */
    void stop();
    
    /**
     * Sends a byte array to the specified connection.
     *
     * As a websocket server can have multiple connections, this method i sused
     * to send a communication to a specific client. Communication from this
     * server is guaranteed to be ordered for an individual client. So if this
     * server sends to messages to client B, client B will receive those messages
     * in the same order. However, there is no relationship between the messages
     * sent to different clients.
     *
     * You may choose to either send a byte array directly, or you can use the
     * {@link NetcodeSerializer} and {@link NetcodeDeserializer} classes to 
     * encode more complex data.
     *
     * This requires that the destination be connected. Otherwise it will return
     * false.
     *
     * @param dst   The identifier of the client to send to
     * @param data  The byte array to send.
     *
     * @return true if the message was (apparently) sent
     */
    bool sendTo(const std::string dst, const std::vector<std::byte>& data);

    /**
     * Sends a byte array to the specified connection.
     *
     * As a websocket server can have multiple connections, this method i sused
     * to send a communication to a specific client. Communication from this
     * server is guaranteed to be ordered for an individual client. So if this
     * server sends to messages to client B, client B will receive those messages
     * in the same order. However, there is no relationship between the messages
     * sent to different clients.
     *
     * You may choose to either send a byte array directly, or you can use the
     * {@link NetcodeSerializer} and {@link NetcodeDeserializer} classes to
     * encode more complex data.
     *
     * This requires that the destination be connected. Otherwise it will return
     * false.
     *
     * @param dst   The identifier of the client to send to
     * @param data  The byte array to send.
     *
     * @return true if the message was (apparently) sent
     */
    bool sendTo(const std::string dst, std::vector<std::byte>&& data);
    
    /**
     * Sends a byte array to all other connections on the given path.
     *
     * This message will be sent to all clients that connected via the given
     * path. As with {@link #sendTo}, communication to a particular client
     * is guaranteed to be ordered. So if we broadcasts two messages, all
     * matching clients will receive those messages in the same order. However,
     * there is no relationship between the messages arriving at different
     * clients. This method also respects the order of {@link #sendTo}.
     *
     * You may choose to either send a byte array directly, or you can use the
     * {@link NetcodeSerializer} and {@link NetcodeDeserializer} classes to
     * encode more complex data.
     *
     * This requires that there be at least one connection on the given path.
     * Otherwise it will return false. In fact, to return true, it must succeed
     * on sending to all of the connections.
     *
     * @param path  The path to broadcast to
     * @param data  The byte array to send.
     *
     * @return true if the message was (apparently) sent
     */
    bool broadcast(const std::string path, const std::vector<std::byte>& data);

    /**
     * Sends a byte array to all connections.
     *
     * This message will be sent to all connected clients. As with {@link #sendTo}, 
     * communication to a particular client is guaranteed to be ordered. So if
     * we broadcasts two messages, all clients will receive those messages in
     * the same order. However, there is no relationship between the messages
     * arriving at different clients. This method also respects the order of
     * {@link #sendTo}.
     *
     * You may choose to either send a byte array directly, or you can use the
     * {@link NetcodeSerializer} and {@link NetcodeDeserializer} classes to
     * encode more complex data.
     *
     * This requires that there be at least one connection on the given path.
     * Otherwise it will return false. In fact, to return true, it must succeed
     * on sending to all of the connections.
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
     * dispatch function should be prepared to be called multiple times a render
     * frame, or even not at all.
     *
     * If a dispatcher callback has been registered with {@link #onReceipt},
     * this method will never do anything. In that case, messages are not
     * buffered and are processed as soon as they are received. However, this
     * method has the advantage that it can be read on a separate thread.
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
     * Sets a callback function to invoke on player connections
     *
     * The websocket will keep a player aware of any connections that may 
     * happen. This callback will update {@link #getConnections} after any such
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
     * happen. This callback will update {@link #getConnections} after any such
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

#endif /* __CU_WEBSOCKET_SERVER_H__ */
