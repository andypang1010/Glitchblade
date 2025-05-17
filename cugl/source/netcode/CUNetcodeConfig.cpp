//
//  CUNetcodeConfig.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides the configuratiion for a Web RTC implementation. The
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
//  Version: 7/3/24 (CUGL 3.0 reorganization)
//
#include <cugl/netcode/CUNetcodeConfig.h>
#include <cugl/core/assets/CUJsonValue.h>
#include <cugl/core/util/CUDebug.h>

using namespace cugl::netcode;

#pragma mark Constructors
/**
 * Creates a new configuration.
 *
 * All values will be defaults. The lobby server will be set to 'localhost"
 * at port 8000 (e.g. the Django port).
 */
NetcodeConfig::NetcodeConfig() {
	lobby.port = 8000;
    secure = false;
    multiplex = false;
    portRangeBegin = 1024;
    portRangeEnd = 65535;
    mtu = 0;
    bufferSize = 0;
    maxMessage = 0;
    maxPlayers = 2;
    apiVersion = 0;
    uuidSeed = "";

}

/**
 * Creates a new configuration with the given lobby server
 *
 * All other values will be defaults. No ICE servers will be specified.
 */
NetcodeConfig::NetcodeConfig(const InetAddress& lobby) {
	this->lobby = lobby;
    secure = false;
    multiplex = false;
    portRangeBegin = 1024;
    portRangeEnd = 65535;
    mtu = 0;
    bufferSize = 0;
    maxMessage = 0;
    maxPlayers = 2;
    apiVersion = 0;
    uuidSeed = "";
}

/**
 * Creates a new configuration with the given lobby and ICE server
 *
 * All other values will be defaults. No ICE servers will be specified.
 */
NetcodeConfig::NetcodeConfig(const InetAddress& lobby, const ICEAddress& iceServer) {
	this->lobby = lobby;
	iceServers.push_back(iceServer);
    secure = false;
    multiplex = false;
    portRangeBegin = 1024;
    portRangeEnd = 65535;
    mtu = 0;
    bufferSize = 0;
	maxMessage = 0;
	maxPlayers = 2;
	apiVersion = 0;
    uuidSeed = "";
}

/**
 * Creates this configuration using a JSON entry.
 *
 * The JSON value should be an object with at least one key -- "lobby" --
 * which is the JSON for an {@link InetAddress}. All other keys are optional.
 * They include:
 *
 *      "secure":       A boolean indicating if the lobby uses SSL
 *      "ICE servers":  A list of {@link ICEAddress} JSONs
 *      "multiplex":    A boolean specifying whether to use UDP multiplexing
 *      "port range":   A list pair of the ports to scan
 *      "MTU":          An int representing the maximum transmission unit
 *      "buffer size":  An int respresenting the size of the message buffer
 *      "max message":  An int respresenting the maximum transmission size
 *      "max players":  An int respresenting the maximum number of players
 *      "UUID seed":    A string providing a potential UUID seed
 *      "API version":  An int respresenting the API version
 *
 * @param pref      The configuration settings
 */
NetcodeConfig::NetcodeConfig(const std::shared_ptr<JsonValue>& prefs) {
	if (prefs->has("lobby")) {
		lobby.set(prefs->get("lobby"));
	}
	if (prefs->has("ice servers")) {
		auto child = prefs->get("ice servers");
		for(int ii = 0; ii < child->size(); ii++) {
			iceServers.emplace_back(child->get(ii));
		}
	}
    secure = prefs->getBool("secure",false);
	multiplex = prefs->getBool("multiplex",false);
	if (prefs->has("port range") && prefs->get("port range")->size() >= 2) {
		auto child = prefs->get("port range");
		portRangeBegin = child->get(0)->asInt(1024);
		portRangeEnd = child->get(1)->asInt(65535);
	} else {
		portRangeBegin = 1024;
		portRangeEnd = 65535;
	}
    mtu = prefs->getInt("MTU",0);
    bufferSize = prefs->getInt("buffer size",0);
	maxMessage = prefs->getInt("max message",0);
	maxPlayers = prefs->getInt("max players",2);
	apiVersion = prefs->getInt("API version",0);
    uuidSeed = prefs->getString("UUID seed","");
}

/**
 * Deletes this configuration, disposing all resources
 */
NetcodeConfig::~NetcodeConfig() {}


#pragma mark -    
#pragma mark Assignment
/**
 * Assigns this configuration to be a copy of the given configuration.
 *
 * @param src   The configuration to copy
 *
 * @return a reference to this configuration for chaining purposes.
 */
NetcodeConfig& NetcodeConfig::set(const NetcodeConfig& src) {
	lobby = src.lobby;
	iceServers = src.iceServers;
	multiplex = src.multiplex;
    portRangeBegin = src.portRangeBegin;
    portRangeEnd = src.portRangeEnd;
    mtu = src.mtu;
    bufferSize = src.bufferSize;
	maxMessage = src.maxMessage;
	maxPlayers = src.maxPlayers;
	apiVersion = src.apiVersion;
    uuidSeed = src.uuidSeed;
	return *this;
}

/**
 * Assigns this configuration to be a copy of the given configuration.
 *
 * @param src   The configuration to copy
 *
 * @return a reference to this configuration for chaining purposes.
 */
NetcodeConfig& NetcodeConfig::set(const std::shared_ptr<NetcodeConfig>& src) {
	lobby = src->lobby;
	iceServers = src->iceServers;
	multiplex = src->multiplex;
    portRangeBegin = src->portRangeBegin;
    portRangeEnd = src->portRangeEnd;
    mtu = src->mtu;
    bufferSize = src->bufferSize;
    maxMessage = src->maxMessage;
	maxPlayers = src->maxPlayers;
	apiVersion = src->apiVersion;
    uuidSeed = src->uuidSeed;
	return *this;
}

/**
 * Assigns this configuration according to the given JSON object
 *
 * The JSON value should be an object with at least one key -- "lobby" --
 * which is the JSON for an {@link InetAddress}. All other keys are optional.
 * They include:
 *
 *      "secure":       A boolean indicating if the lobby uses SSL
 *      "ICE servers":  A list of {@link ICEAddress} JSONs
 *      "multiplex":    A boolean specifying whether to use UDP multiplexing
 *      "port range":   A list pair of the ports to scan
 *      "MTU":          An int representing the maximum transmission unit
 *      "buffer size":  An int respresenting the size of the message buffer
 *      "max message":  An int respresenting the maximum transmission size
 *      "max players":  An int respresenting the maximum number of players
 *      "UUID seed":    A string providing a potential UUID seed
 *      "API version":  An int respresenting the API version
 *
 * @param pref      The address settings
 *
 * @return a reference to this address for chaining purposes.
 */
NetcodeConfig& NetcodeConfig::set(const std::shared_ptr<JsonValue>& prefs) {
	if (prefs->has("lobby")) {
		lobby.set(prefs->get("lobby"));
	}
	iceServers.clear();
	if (prefs->has("ice servers")) {
		auto child = prefs->get("ice servers");
		for(int ii = 0; ii < child->size(); ii++) {
			iceServers.emplace_back(child->get(ii));
		}
	}
    secure = prefs->getBool("secure",false);
	multiplex = prefs->getBool("multiplex",false);
	if (prefs->has("port range") && prefs->get("port range")->size() >= 2) {
		auto child = prefs->get("port range");
		portRangeBegin = child->get(0)->asInt(1024);
		portRangeEnd = child->get(1)->asInt(65535);
	} else {
		portRangeBegin = 1024;
		portRangeEnd = 65535;
	}
    mtu = prefs->getInt("MTU",0);
    bufferSize = prefs->getInt("buffer size",0);
    maxMessage = prefs->getInt("max message",0);
    maxPlayers = prefs->getInt("max players",2);
    apiVersion = prefs->getInt("API version",0);
    uuidSeed = prefs->getString("UUID seed","");
	return *this;
}
