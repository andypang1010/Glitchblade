//
//  CUWebsocketServer.cpp
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
#include <cugl/netcode/CUWebSocketServer.h>
#include <cugl/netcode/CUNetworkLayer.h>
#include <cugl/core/util/CUDebug.h>
#include <cugl/core/util/CUStringTools.h>
#include <cugl/core/CUApplication.h>
#include <stdexcept>
#include <algorithm>
#include <cstring>

using namespace cugl::netcode;
using namespace std;

/** The buffer size for message envelopes */
#define DEFAULT_BUFFER 64

/**
 * Copies information from a CUGL configuration to an RTC configuration
 *
 * The purpose of this function is to hide libdatachannel from the students.
 *
 * @param src   The CUGL configuration
 * @param dst   The RTC configuration
 */
static void config2rtc(const WebSocketConfig& src, rtc::WebSocketServerConfiguration& dst) {
    dst.port = src.port;
    dst.enableTls = src.secure;
    dst.bindAddress = src.bindaddr.empty() ? nullopt : std::make_optional(src.bindaddr);
    dst.certificatePemFile = src.pemCertificate.empty() ? nullopt : std::make_optional(src.pemCertificate);
    dst.keyPemFile = src.pemKey.empty() ? nullopt : std::make_optional(src.pemKey);
    dst.keyPemPass = src.pemPass.empty() ? nullopt : std::make_optional(src.pemPass);

    if (src.timeout > 0) {
        dst.connectionTimeout = std::chrono::milliseconds(src.timeout);
    } else if (src.timeout < 0) {
        dst.connectionTimeout = std::chrono::milliseconds::zero(); // setting to 0 disables,
    }

    if (src.maxMessage != 0) {
        dst.maxMessageSize = src.maxMessage;
    }
}

#pragma mark -
#pragma mark WebSocketWrapper

/**
 * This class is a wrapper around an RTC socket and its relevant information.
 *
 * This is an internal class used by {@link WebSocketServer}. We add it to
 * reduce the number of redirects associating keys with sockets.
 */
class cugl::netcode::WebSocketWrapper {
public:
    size_t key;
    std::string address;
    std::string path;
    std::shared_ptr<rtc::WebSocket> socket;
    std::unordered_set<WebSocketWrapper*>* neighbors;
    
    /**
     * Creates a new wrapper with the given key and socket
     *
     * @param key       The wrapper key
     * @param socket    The wrapper socker
     */
    WebSocketWrapper(size_t key, const std::shared_ptr<rtc::WebSocket>& socket) :
        neighbors(nullptr),
        address(""),
        path("") {
        this->key = key;
        this->socket = socket;
    }
    
    /**
     * Deletes this wrapper, disposing all resources
     */
    ~WebSocketWrapper() {
        key = 0;
        address.clear();
        socket = nullptr;
        neighbors = nullptr; // This was handled externally
    }
};

#pragma mark -
#pragma mark WebSocketServer
/**
 * Creates a degenerate websocket server
 *
 * This object has not been initialized with a {@link WebSocketConfig} and
 * cannot be used.
 *
 * You should NEVER USE THIS CONSTRUCTOR. All connections should be created
 * by the static constructor {@link #alloc} instead.
 */
WebSocketServer::WebSocketServer() :
    _bufflimit(0),
    _buffsize(0),
    _buffhead(0),
    _bufftail(0),
    _debug(false),
    _active(false) {}

/**
 * Deletes this websocket server, disposing all resources
 */
WebSocketServer::~WebSocketServer() {
    dispose();
}

/**
 * Disposes all of the resources used by this websocket server.
 *
 * While we never expect to reinitialize a new websocket server, this
 * method allows for a "soft" deallocation, where internal resources are
 * destroyed as soon as a connection is terminated. This simplifies the
 * cleanup process.
 */
void WebSocketServer::dispose() {
    if (_active) {
        std::lock_guard<std::mutex> lock(_mutex);
        _active = false;    // Prevents cycles
        
        // This kills all connections
        _connections.clear();
        _paths.clear();
        
        _server->stop();
        _server = nullptr;

        _buffer.clear();
        return;
    }
}

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
bool WebSocketServer::init(const WebSocketConfig& config) {
    try {
        if (NetworkLayer::get() == nullptr) {
            CUAssertLog(false, "Network layer is not active");
            return false;
        }
        _debug  = NetworkLayer::get()->isDebug();
        
        _config = config;
        config2rtc(_config,_rtcconfig);
        _bufflimit = config.bufferSize == 0 ? DEFAULT_BUFFER : config.bufferSize;

        return true;
    } catch (const std::exception &e) {
        CULogError("NETCODE ERROR: %s",e.what());
        _server = nullptr;
        _active = false;
        return false;
    }
}

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
bool WebSocketServer::init(WebSocketConfig&& config) {
    try {
        if (NetworkLayer::get() == nullptr) {
            CUAssertLog(false, "Network layer is not active");
            return false;
        }
        _debug  = NetworkLayer::get()->isDebug();
        
        _config = std::move(config);
        config2rtc(_config,_rtcconfig);
        _bufflimit = config.bufferSize == 0 ? DEFAULT_BUFFER : config.bufferSize;

        return true;
    } catch (const std::exception &e) {
        CULogError("NETCODE ERROR: %s",e.what());
        _server = nullptr;
        _active = false;
        return false;
    }
}

#pragma mark -
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
void WebSocketServer::onClient(std::shared_ptr<rtc::WebSocket> socket) {
    size_t key = (size_t)socket.get();
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _connections.try_emplace(key,std::make_shared<WebSocketWrapper>(key,socket));
        
        // In case we die before a connection
        std::weak_ptr<WebSocketServer> wserver = shared_from_this();
        
        // Time to register some callbacks
        socket->onOpen([key,wserver]() {
            auto server = wserver.lock();
            if (server) {
                server->onOpen(key);
            }
        });
        socket->onError([key,wserver](std::string s) {
            auto server = wserver.lock();
            if (server) {
                server->onError(key,s);
            }
        });
        socket->onClosed([key,wserver]() {
            auto server = wserver.lock();
            if (server) {
                server->onClosed(key);
            }
        });
    }
    
    // JUST in case
    if (socket->isOpen()) {
        onOpen(key);
    }
}

/**
 * Called when the web socket opens.
 *
 * The key here is used to reference the pointer address of the socket.
 *
 * @param key   The pointer address key
 */
void WebSocketServer::onOpen(size_t key) {
    std::string addr = "UNKNOWN";
    std::string path = "/";

    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _connections.find(key);
        if (it == _connections.end()) {
            return;
        }
        auto wrapper = it->second;
        
        Uint64 time = NetworkLayer::get()->getTime();
        std::string hash = strtool::to_hexstring(time);
        
        auto ifaddr = wrapper->socket->remoteAddress();
        if (!ifaddr.has_value()) {
            _connections.erase(it);
            return;
        } else {
            addr = *ifaddr;
        }
        
        auto ifpath = wrapper->socket->path();
        if (ifpath.has_value()) {
            path = *ifpath;
        }
        
        addr += "x"+hash;
        wrapper->address = addr;
        _keymap[addr] = key;
        
        // Now handle paths
        wrapper->path = path;
        auto jt = _paths.find(path);
        if (jt == _paths.end()) {
            auto kt = _paths.try_emplace(path);
            wrapper->neighbors = &(kt.first->second);
        } else {
            wrapper->neighbors = &(jt->second);
        }
        wrapper->neighbors->emplace(wrapper.get());
        
        // Add the new callback
        std::weak_ptr<WebSocketServer> wserver = shared_from_this();
        wrapper->socket->onMessage([addr,wserver](auto data) {
            auto server = wserver.lock();
            if (server) {
                server->onMessage(addr,data);
            }
        });
        
        if (_debug) {
            CULog("SERVER: Client %s connected",addr.c_str());
        }
    }
    
    // Never hold locks during a user callback
    if (_onConnect) {
        _onConnect(addr,path);
    }
}

/**
 * Called when a websocket experiences an error.
 *
 * The key here is used to reference the pointer address of the socket.
 *
 * @param key   The pointer address key
 * @param s     The error message
 */
void WebSocketServer::onError(size_t key, const std::string s) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_debug) {
        auto it = _connections.find(key);
        if (it == _connections.end()) {
            return;
        }
        
        if (it->second->address.empty()) {
            CULog("SERVER: Pending client error %s",s.c_str());
        } else {
            CULog("SERVER: Client[%s] error %s",it->second->address.c_str(),s.c_str());
        }
    }
}

/**
 * Called when a web socket closes
 *
 * The key here is used to reference the pointer address of the socket.
 *
 * @param key   The pointer address key
 */
void WebSocketServer::onClosed(size_t key) {
    std::string addr = "";
    std::string path = "/";
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _connections.find(key);
        if (it == _connections.end()) {
            return;
        }
        
        auto wrapper = it->second;
        addr = wrapper->address;
        path = wrapper->path;
        
        auto jt = _keymap.find(addr);
        if (jt != _keymap.end()) {
            _keymap.erase(jt);
        }
        
        if (wrapper->neighbors != nullptr) {
            auto kt = wrapper->neighbors->find(wrapper.get());
            if (kt != wrapper->neighbors->end()) {
                wrapper->neighbors->erase(kt);
            }
            if (wrapper->neighbors->empty()) {
                _paths.erase(path);
            }
            wrapper->neighbors = nullptr;
        }
        
        _connections.erase(it);
        
        if (_debug) {
            CULog("SERVER: Client %s disconnected",addr.c_str());
        }
    }
    
    // Never hold locks during a user callback
    if (_onDisconnect) {
        _onDisconnect(addr,path);
    }
}

/**
 * Called when this websocket (and not a peer channel) receives a message
 *
 * @param client    The messaging client
 * @param data      The message received
 */
void WebSocketServer::onMessage(const std::string client, rtc::message_variant data) {
    // data holds either std::string or rtc::binary
    if (!std::holds_alternative<rtc::binary>(data)) {
        return;
    }
    
    append(client,std::get<rtc::binary>(data),NetworkLayer::get()->getTime());
}

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
bool WebSocketServer::append(const std::string client, const std::vector<std::byte>& data,
                             Uint64 timestamp) {
    std::function<bool()> callback;
    bool success = false;
    
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_active) {
            if (_onReceipt) {
                callback = [=,this]() {
                    _onReceipt(client,data,timestamp);
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
                env->client  = client;
                env->message = data;
                env->timestamp = timestamp;

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
 * Returns the port for this server
 *
 * @return the port for this server
 */
uint16_t WebSocketServer::getPort() const {
    if (_active) {
        return _server->port();
    }
    return _config.port;
}

/**
 * Returns the message buffer capacity.
 *
 * It is possible for this connection to receive several messages over the network
 * before it has a chance to all {@link #receive}. This buffer stores those messages
 * to be read later. The capacity indicates the number of messages that can be
 * stored.
 *
 * Note that this is NOT the same as the capacity of a single message. That value
 * was set as part of the initial {@link NetcodeConfig}.
 *
 * This method is not const because it requires a lock.
 *
 * @return the message buffer capacity.
 */
size_t WebSocketServer::getCapacity() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _buffer.size();
}

/**
 * Sets the message buffer capacity.
 *
 * It is possible for this connection to recieve several messages over the network
 * before it has a chance to all {@link #receive}. This buffer stores those messages
 * to be read later. The capacity indicates the number of messages that can be
 * stored.
 *
 * Note that this is NOT the same as the capacity of a single message. That value
 * was set as part of the initial {@link NetcodeConfig}.
 *
 * @paran capacity  The new message buffer capacity.
 */
void WebSocketServer::setCapacity(size_t capacity) {
    std::lock_guard<std::mutex> lock(_mutex);
    
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
const std::unordered_set<std::string> WebSocketServer::getConnections() {
    std::lock_guard<std::mutex> lock(_mutex);
    std::unordered_set<std::string> result;
    for(auto it = _keymap.begin(); it != _keymap.end(); ++it) {
        result.emplace(it->first);
    }
    return result;
}

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
const std::unordered_set<std::string> WebSocketServer::getConnections(std::string path) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _paths.find(path);
    
    std::unordered_set<std::string> result;
    if (it != _paths.end()) {
        for(auto jt = it->second.begin(); jt != it->second.end(); ++jt) {
            auto wrapper = *jt;
            if (wrapper != nullptr) {
                result.emplace(wrapper->address);
            }
        }
    }
    
    return result;
}
    
/**
 * Returns true if the given client is currently connected.
 *
 * A client id is a string built from its remote internet address (as seen
 * by the server) and the timestamp of connection in hexadecimal form.
 *
 * This method is not const because it requires a lock.
 *
 * @param player    The player to test for connection
 *
 * @return true if the given client is currently connected.
 */
bool WebSocketServer::isConnected(const std::string client) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _keymap.find(client);
    return (it != _keymap.end());
}

/**
 * Returns the number of clients currently connected to this server
 *
 * This does not include any clients that have been disconnected.
 *
 * This method is not const because it requires a lock.
 *
 * @return the number of clients currently connected to this server
 */
size_t WebSocketServer::getNumConnections() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _keymap.size();
}

/**
 * Toggles the debugging status of this connection.
 *
 * If debugging is active, connections will be quite verbose
 *
 * @param flag  Whether to activate debugging
 */
void WebSocketServer::setDebug(bool flag) {
    std::lock_guard<std::mutex> lock(_mutex);
    _debug = flag;
}

#pragma mark -
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
void WebSocketServer::start() {
    if (_active) {
        return;
    }
    
    if (_debug) {
        CULog("SERVER: Server activated at port %d",_config.port);
    }

    _server = std::make_shared<rtc::WebSocketServer>(_rtcconfig);
    _server->onClient([this](std::shared_ptr<rtc::WebSocket> socket) {
        this->onClient(socket);
    });
    
    _buffer.resize(_bufflimit);
    
    // Start the connection
    _active = true;
}

/**
 * Stops this websocket server, closing all connections.
 *
 * Shutdown of a websocket server is immediate, and all connections are
 * closed. However, it is possible to restart the server (with no initial
 * connections) using {@link #start}.
 */
void WebSocketServer::stop() {
    if (!_active) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(_mutex);
    _active = false;
    _connections.clear();
    _paths.clear();
    _keymap.clear();
    _server->stop();
    _server = nullptr;
}

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
bool WebSocketServer::sendTo(const std::string dst, const std::vector<std::byte>& data) {
    std::shared_ptr<rtc::WebSocket> socket;

    // Critical section
    if (_active) {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _keymap.find(dst);
        if (it == _keymap.end()) {
            return false;
        }
        
        auto wrapper = _connections.find(it->second)->second;
        socket = wrapper->socket;
    }
    
    // Do not hold locks on send
    if (socket) {
        socket->send(data);
    } else {
        return false;
    }
    return true;
}

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
bool WebSocketServer::sendTo(const std::string dst, std::vector<std::byte>&& data) {
    std::shared_ptr<rtc::WebSocket> socket;

    // Critical section
    if (_active) {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _keymap.find(dst);
        if (it == _keymap.end()) {
            return false;
        }
        
        auto wrapper = _connections.find(it->second)->second;
        socket = wrapper->socket;
    }
    
    // Do not hold locks on send
    if (socket) {
        socket->send(std::move(data));
    } else {
        return false;
    }
    return true;
}

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
bool WebSocketServer::broadcast(const std::string path,
                                const std::vector<std::byte>& data) {
    std::vector<std::shared_ptr<rtc::WebSocket>> sockets;
    bool success = true;

    // Critical section
    if (_active) {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _paths.find(path);
        if (it == _paths.end()) {
            return false;
        }
        
        sockets.reserve(it->second.size());
        for (auto jt = it->second.begin(); jt != it->second.end(); ++jt) {
            sockets.push_back((*jt)->socket);
        }
    }
        
    // Do not hold locks on send
    for(auto it = sockets.begin(); it != sockets.end(); ++it) {
        success = (*it)->send(data) && success;
    }
        
    return success;
}

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
bool WebSocketServer::broadcast(const std::vector<std::byte>& data) {
    std::vector<std::shared_ptr<rtc::WebSocket>> sockets;
    bool success = true;

    // Critical section
    if (_active) {
        std::lock_guard<std::mutex> lock(_mutex);
        sockets.reserve(_connections.size());
        for (auto jt = _connections.begin(); jt != _connections.end(); ++jt) {
            if (jt->second->socket->isOpen()) {
                sockets.push_back(jt->second->socket);
            }
        }
    }
        
    // Do not hold locks on send
    for(auto it = sockets.begin(); it != sockets.end(); ++it) {
        success = (*it)->send(data) && success;
    }
        
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
void WebSocketServer::receive(const Dispatcher& dispatcher) {
    if (dispatcher == nullptr || !_active || _buffsize == 0) {
        return;
    }
    
    // Sigh. Dispatch is also a callback. Cannot hold a lock when we use.
    std::vector<Envelope> messages;
    messages.reserve(_buffsize);
    {
        std::lock_guard<std::mutex> lock(_mutex);
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
        dispatcher(it->client,it->message,it->timestamp);
    }
}
    
#pragma mark -
#pragma mark Callbacks
/**
 * Sets a callback function to invoke on message receipt
 *
 * This callback is alternative to the method {@link #receive}. Instead of buffering
 * messages and calling that method each frame, this callback function will be
 * invoked as soon as the message is received.
 *
 * All callback functions are guaranteed to be called on the main thread. They
 * are called at the start of an animation frame, before the method
 * {@link Application#update(float) }.
 *
 * @param callback  The dispatcher callback
 */
void WebSocketServer::onReceipt(Dispatcher callback) {
    std::lock_guard<std::mutex> lock(_mutex);
    _onReceipt = callback;
}

/**
 * Sets a callback function to invoke on player connections
 *
 * The websocket will keep a player aware of any connections that may happen.
 * This callback will update {@link #getPlayers} after any such connection.
 * Hence connections can be detected through polling or this callback interface.
 * If this information is important to you, the callback interface is preferred.
 *
 * All callback functions are guaranteed to be called on the main thread. They
 * are called at the start of an animation frame, before the method
 * {@link Application#update(float) }.
 *
 * @param callback  The connection callback
 */
void WebSocketServer::onConnect(ConnectionCallback callback) {
    std::lock_guard<std::mutex> lock(_mutex);
    _onConnect = callback;
}

/**
 * Sets a callback function to invoke on player disconnections
 *
 * The websocket will keep a player aware of any disconnections that may happen.
 * This callback will update {@link #getPlayers} after any such disconnection.
 * Hence disconnections can be detected through polling or this callback interface.
 * If this information is important to you, the callback interface is preferred.
 *
 * All callback functions are guaranteed to be called on the main thread. They
 * are called at the start of an animation frame, before the method
 * {@link Application#update(float) }.
 *
 * @param callback  The disconnection callback
 */
void WebSocketServer::onDisconnect(ConnectionCallback callback) {
    std::lock_guard<std::mutex> lock(_mutex);
    _onDisconnect = callback;
}
