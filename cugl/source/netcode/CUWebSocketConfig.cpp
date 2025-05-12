//
//  CUWebSocketConfig.h
//  Cornell University Game Library (CUGL)
//
//  This module provides the configuratiion for a WebSocket server. The
//  configuration is designed to be compatible with libdatachannels:
//
//      https://github.com/paullouisageneau/libdatachannel
//
//  Because configurations are intended to be on the stack, we do not provide
//  explicit shared pointer support for this class.
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
#include <cugl/netcode/CUWebSocketConfig.h>
#include <cugl/core/assets/CUJsonValue.h>
#include <cugl/core/util/CUDebug.h>

using namespace cugl::netcode;

#pragma mark Constructors
/**
 * Creates a new configuration.
 *
 * All values will be defaults. The lobby server will be set to 'localhost"
 * at port 8080.
 */
WebSocketConfig::WebSocketConfig() :
port(8080),
bindaddr(""),
secure(false),
pemCertificate(""),
pemKey(""),
pemPass(""),
timeout(0),
bufferSize(0),
maxMessage(0) {
}

/**
 * Creates a new configuration with the given port.
 *
 * All other values will be defaults.
 */
WebSocketConfig::WebSocketConfig(uint16_t port) : WebSocketConfig() {
    this->port = port;
}

/**
 * Creates a new configuration with the given bind address.
 *
 * All other values will be defaults.
 */
WebSocketConfig::WebSocketConfig(const InetAddress& address) : WebSocketConfig() {
    port = address.port;
    bindaddr = address.address;
}

/**
 * Creates this configuration using a JSON entry.
 *
 * The JSON value should be an object. While all keys are optional, it
 * supports the following entries:
 *
 *      "port":         The port to bind to
 *      "address":      The local address to bind to
 *      "secure":       A boolean indicating if the server uses SSL
 *      "certificate":  Either the PEM certficate, or a path to the certificate
 *      "pemkey":       Either the PEM key, or a path to the key
 *      "pempass":      The PEM pass phrase
 *      "timeout":      An int representing the connection timeout
 *      "buffer size":  An int respresenting the size of the message buffer
 *      "max message":  An int respresenting the maximum transmission size
 *
 * @param prefs     The configuration settings
 */
WebSocketConfig::WebSocketConfig(const std::shared_ptr<JsonValue>& prefs) {
    port = prefs->getInt("port",0);
    bindaddr = prefs->getString("address","");
    secure = prefs->getBool("secure",false);
    pemCertificate = prefs->getString("certificate","");
    pemKey = prefs->getString("pemkey","");
    pemPass = prefs->getString("pempass","");
    timeout = prefs->getInt("timeout",0);
    bufferSize = prefs->getInt("buffer size",0);
    maxMessage = prefs->getInt("max message",0);
}

/**
 * Deletes this configuration, disposing all resources
 */
WebSocketConfig::~WebSocketConfig() {}


#pragma mark -
#pragma mark Assignment
/**
 * Assigns this configuration to be a copy of the given configuration.
 *
 * @param src   The configuration to copy
 *
 * @return a reference to this configuration for chaining purposes.
 */
WebSocketConfig& WebSocketConfig::set(const WebSocketConfig& src) {
    port = src.port;
    bindaddr = src.bindaddr;
    secure = src.secure;
    pemCertificate = src.pemCertificate;
    pemKey = src.pemKey;
    pemPass = src.pemPass;
    timeout = src.timeout;
    bufferSize = src.bufferSize;
    maxMessage = src.maxMessage;
    return *this;
}

/**
 * Assigns this configuration to be a copy of the given configuration.
 *
 * @param src   The configuration to copy
 *
 * @return a reference to this configuration for chaining purposes.
 */
WebSocketConfig& WebSocketConfig::set(const std::shared_ptr<WebSocketConfig>& src) {
    port = src->port;
    bindaddr = src->bindaddr;
    secure = src->secure;
    pemCertificate = src->pemCertificate;
    pemKey = src->pemKey;
    pemPass = src->pemPass;
    timeout = src->timeout;
    bufferSize = src->bufferSize;
    maxMessage = src->maxMessage;
    return *this;
}

/**
 * Assigns this configuration according to the given JSON object
 *
 * The JSON value should be an object. While all keys are optional, it
 * supports the following entries:
 *
 *      "port":         The port to bind to
 *      "address":      The local address to bind to
 *      "secure":       A boolean indicating if the server uses SSL
 *      "certificate":  Either the PEM certficate, or a path to the certificate
 *      "pemkey":       Either the PEM key, or a path to the key
 *      "pempass":      The PEM pass phrase
 *      "timeout":      An int representing the connection timeout
 *      "max message":  An int respresenting the maximum transmission size
 *
 * @param prefs     The address settings
 *
 * @return a reference to this address for chaining purposes.
 */
WebSocketConfig& WebSocketConfig::set(const std::shared_ptr<JsonValue>& prefs) {
    port = prefs->getInt("port",0);
    bindaddr = prefs->getString("address","");
    secure = prefs->getBool("secure",false);
    pemCertificate = prefs->getString("certificate","");
    pemKey = prefs->getString("pemkey","");
    pemPass = prefs->getString("pempass","");
    timeout = prefs->getInt("timeout",0);
    bufferSize = prefs->getInt("buffer size",0);
    maxMessage = prefs->getInt("max message",0);

    return *this;
}
