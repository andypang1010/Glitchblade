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
#include <cugl/netcode/CUNetworkLayer.h>
#include <cugl/netcode/CUWebSocket.h>
#include <cugl/core/util/CUDebug.h>
#include <cugl/core/CUApplication.h>

using namespace cugl::netcode;
using namespace std;

/** The buffer size for message envelopes */
#define DEFAULT_BUFFER 64

/**
 * Returns the CUGL state equivalent of a RTC state
 *
 * The purpose of this function is to hide libdatachannel from the students.
 *
 * @param state The RTC state
 *
 * @return the CUGL state equivalent of a RTC state
 */
static WebSocket::State state2state(rtc::WebSocket::State state) {
    switch (state) {
        case rtc::WebSocket::State::Connecting:
            return WebSocket::State::CONNECTING;
        case rtc::WebSocket::State::Open:
            return WebSocket::State::OPEN;
        case rtc::WebSocket::State::Closing:
            return WebSocket::State::CLOSING;
        case rtc::WebSocket::State::Closed:
            return WebSocket::State::CLOSED;
        default:
            return WebSocket::State::FAILED;
    }
}

#pragma mark -
#pragma mark Constructors
/**
 * Creates a degenerate websocket connection.
 *
 * This object has not been initialized with a {@link NetcodeConfig} and cannot
 * be used.
 *
 * You should NEVER USE THIS CONSTRUCTOR. All connections should be created by
 * the static constructor {@link #alloc} instead.
 */
WebSocket::WebSocket() :
    _active(false),
    _path(""),
    _socket(nullptr),
    _buffsize(0),
    _buffhead(0),
    _bufftail(0),
    _debug(false),
    _state(State::INACTIVE) {}

/**
 * Deletes this websocket connection, disposing all resources
 */
WebSocket::~WebSocket() {
    dispose();
}

/**
 * Disposes all of the resources used by this websocket connection.
 *
 * While we never expect to reinitialize a new websocket connection, this method
 * allows for a "soft" deallocation, where internal resources are destroyed as
 * soon as a connection is terminated. This simplifies the cleanup process.
 */
void WebSocket::dispose() {
    if (_active) {
        std::lock_guard<std::mutex> lock(_mutex);
        _active = false;    // Prevents cycles

        _socket->close();
        _socket = nullptr;

        _path = "";
        _buffer.clear();
        
        // Leave other settings for debugging
    }
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
bool WebSocket::initWithPath(const InetAddress& address, const std::string path) {
    try {
        if (NetworkLayer::get() == nullptr) {
            CUAssertLog(false, "Network layer is not active");
            return false;
        }
        _debug  = NetworkLayer::get()->isDebug();
        _bufflimit = DEFAULT_BUFFER;
        _address  = address;
        _path = (path.empty() || path[0] != '/') ? "/"+path : path;
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
void WebSocket::onOpen() {
    if (_active) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_debug) {
            CULog("WEBSOCKET: WebSocket %s%s connected",
                  _address.toString().c_str(),_path.c_str());
        }
        _state = State::OPEN;
    }
    
    // Never hold locks on a user callback
    if (_onStateChange) {
        _onStateChange(_state);
    }
}

/**
 * Called when the websocket experiences an error
 *
 * @param s The error message
 */
void WebSocket::onError(std::string s) {
    if (_debug) {
        std::lock_guard<std::mutex> lock(_mutex);
        CULogError("WEBSOCKET: WebSocket error '%s' at %s%s",s.c_str(),
                   _address.toString().c_str(),_path.c_str());
    }
}

/**
 * Called when the web socket closes
 */
void WebSocket::onClosed() {
    if (_active) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_debug) {
            CULog("WEBSOCKET: WebSocket %s%s closed, ",
                  _address.toString().c_str(),_path.c_str());
        }
        _state = State::CLOSED;
    }
    
    // Never hold locks on a user callback
    if (_onStateChange) {
        _onStateChange(_state);
    }
    
    dispose();
}

/**
 * Called when this websocket (and not a peer channel) receives a message
 *
 * @param data  The message received
 */
void WebSocket::onMessage(rtc::message_variant data) {
    // data holds either std::string or rtc::binary
    if (!std::holds_alternative<rtc::binary>(data)) {
        return;
    }
    
    append(std::get<rtc::binary>(data),NetworkLayer::get()->getTime());
}

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
bool WebSocket::append(const std::vector<std::byte>& data, Uint64 timestamp) {
    std::function<bool()> callback;
    bool success = false;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_active) {
            if (_onReceipt) {
                callback = [=,this]() {
                    _onReceipt(data,timestamp);
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
                env->timestamp = timestamp;
                env->message = data;

                _bufftail = ((_bufftail + 1) % _buffer.size());
                _buffsize++;
            }
            success = true;
        }
    }
    
    if (callback) {
        cugl::Application::get()->schedule(callback);
    }
    
    return success;
}

#pragma mark -
#pragma mark Accessors
/**
 * Returns true if this connection is open
 *
 * Technically a connection is not open if the state is CONNECTING.
 *
 * This method is not constant because it performs an internal query.
 *
 * @return true if this connection is open
 */
bool WebSocket::isOpen() { return getState() == WebSocket::State::OPEN; }

/**
 * Returns the current state of this connection.
 *
 * Monitoring state is one of the most important components of working with
 * a {@link Websocket}. It is possible for a connection to close remotely,
 * without any input from this side.
 *
 * This method is not constant because it performs an internal query.
 *
 * State can either be monitored via polling with this method, or with a
 * callback set to {@link onStateChange}.
 */
WebSocket::State WebSocket::getState() {
    _state = state2state(_socket->readyState());
    return _state;
}
    
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
size_t WebSocket::getCapacity() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _buffer.size();
}

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
void WebSocket::setCapacity(size_t capacity) {
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
 * Toggles the debugging status of this connection.
 *
 * If debugging is active, connections will be quite verbose.
 *
 * @param flag  Whether to activate debugging
 */
void WebSocket::setDebug(bool flag) {
    std::lock_guard<std::mutex> lock(_mutex);
    _debug = flag;
}

#pragma mark -
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
void WebSocket::open(bool secure) {
    if (_debug) {
        CULog("WEBSOCKET: Socket connection %s%s allocated",
              _address.toString().c_str(),_path.c_str());
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

    const std::string prefix = secure ? "wss://" : "ws://";
    const std::string url = prefix + _address.toString() + _path;
    CULog("%s",url.c_str());
    
    if (_debug) {
        CULog("WEBSOCKET: Connecting to %s",url.c_str());
    }
    
    _socket->open(url);
    if (_debug) {
        CULog("WEBSOCKET: Waiting for '%s' to connect",url.c_str());
    }
}

/**
 * Closes this connection normally.
 *
 * Because this requires coordination with this connection, this method
 * does not close the connection immediately. Verify that the state is
 * {@link State#CLOSED} before destroying this object.
 */
void WebSocket::close() {
    if (_active) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _socket->close();
            _state = WebSocket::State::CLOSING;
        }

        // Never hold locks on a user callback
        if (_onStateChange) {
            _onStateChange(_state);
        }
    }    
}

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
bool WebSocket::send(const std::vector<std::byte>& data) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_active && _state == WebSocket::State::OPEN) {
        return _socket->send(data);
    }
    return false;
}

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
bool WebSocket::send(std::vector<std::byte>&& data) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_active && _state == WebSocket::State::OPEN) {
        return _socket->send(std::move(data));
    }
    return false;
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
 * buffered and are processed as soon as they are received.
 *
 * @param dispatcher    The function to process received data
 */
void WebSocket::receive(const Dispatcher& dispatcher) {
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
        dispatcher(it->message,it->timestamp);
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
void WebSocket::onReceipt(Dispatcher callback) {
    std::lock_guard<std::mutex> lock(_mutex);
    _onReceipt = callback;
}

/**
 * Sets a callback function to invoke on state changes
 *
 * Monitoring state is one of the most important components of working with
 * a {@link Websocket}. It is possible for a connection to close remotely,
 * without any input from this side.
 *
 * State can either be monitored via a callback with this method, or with a
 * polling the method {@link #getState}.
 *
 * @param callback  The state change callback
 */
void WebSocket::onStateChange(StateCallback callback) {
    std::lock_guard<std::mutex> lock(_mutex);
    _onStateChange = callback;
}
