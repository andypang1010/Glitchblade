//
//  CUICEAddress.h
//  Cornell University Game Library (CUGL)
//
//  This module provides an extension of internet addresses for ICE (STUN and TURN)
//  servers.  Those addresses need extra information such as username and password.
//  Because internet addresses are intended to be on the stack, we do not provide
//  explicit shared pointer support for this class.
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
#include <cugl/netcode/CUICEAddress.h>
#include <cugl/core/assets/CUJsonValue.h>
#include <cugl/core/util/CUDebug.h>
#include <sstream>

using namespace cugl::netcode;

/**
 * Creates an ICE address to refer to the localhost
 *
 * The address will be the hostname "localhost". It will be categorized as 
 * a STUN server with default port 3478. The constructor does not perform any 
 * validation that the combined address port are reachable.
 */
ICEAddress::ICEAddress() : InetAddress() {
	turn = false;
	username = "";
	password = "";
}

/**
 * Creates an ICE address to refer to the localhost
 *
 * The address will be the hostname "localhost". It will be categorized as 
 * a STUN server. The constructor does not perform any validation that the 
 * combined address port are reachable.
 *
 * @param port  The address port
 */
ICEAddress::ICEAddress(uint16_t port) : InetAddress(port) {
	turn = false;
	username = "";
	password = "";
}

/**
 * Creates an ICE address for the given address.
 *
 * The address will be categorized as a STUN server. The constructor does not 
 * perform any validation that the combined address and port are reachable.
 *
 * @param address   The address of the STUN server
 * @param port      The address port
 */
ICEAddress::ICEAddress(const std::string address, uint16_t port) : InetAddress(address, port) {
	turn = false;
	username = "";
	password = "";
}

/**
 * Creates an ICE address for the given address, username and password
 *
 * The address will be categorized as a TURN server. The constructor does not 
 * perform any validation that the combined address and port are reachable.
 *
 * @param address   The address of the TURN server
 * @param username  The user name for the TURN server
 * @param password  The password for the TURN server
 * @param port      The address port
 */
ICEAddress::ICEAddress(const std::string address, const std::string username, 
		   const std::string password, uint16_t port) : InetAddress(address, port) {
	turn = true;
	this->username = username;
	this->password = password;
}

/**
 * Creates this ICE address using a JSON entry.
 *
 * The JSON value should be an object with at least three keys: "address",
 * "port", and "turn". The "port" should be an integer, while "turn" is
 * a boolean.  If "turn" is true, this constructor will search for additional
 * keys "username" and "password".
 *
 * @param prefs      The address settings
 */
ICEAddress::ICEAddress(const std::shared_ptr<JsonValue>& prefs) : InetAddress(prefs) {
	turn = prefs->getBool("turn",false);
    username = prefs->getString("username","");
    password = prefs->getString("password","");
}
    
#pragma mark -
#pragma mark Assignment
/**
 * Assigns this address to be a copy of the given ICE address.
 *
 * @param src   The address to copy
 *
 * @return a reference to this address for chaining purposes.
 */
ICEAddress& ICEAddress::set(const ICEAddress& src) {
	address = src.address;
	username = src.username;
	password = src.password;
	turn = src.turn;
	port = src.port;
	return *this;
}

/**
 * Assigns this address to be a copy of the given ICE address.
 *
 * @param src   The address to copy
 *
 * @return a reference to this address for chaining purposes.
 */
ICEAddress& ICEAddress::set(const std::shared_ptr<ICEAddress>& src) {
	address = src->address;
	username = src->username;
	password = src->password;
	turn = src->turn;
	port = src->port;
	return *this;
}

/**
 * Assigns this address to be a copy of given JSON object
 *
 * The JSON value should be an object with at least three keys: "address",
 * "port", and "turn". The "port" should be an integer, while "turn" is
 * a boolean.  If "turn" is true, this constructor will search for additional
 * keys "username" and "password".
 *
 * @param prefs      The address settings
 *
 * @return a reference to this address for chaining purposes.
 */
ICEAddress& ICEAddress::set(const std::shared_ptr<JsonValue>& prefs) {
	InetAddress::set(prefs);
	turn = prefs->getBool("turn",false);
    username = prefs->getString("username","");
    password = prefs->getString("password","");
    return *this;
}

/**
 * Returns a string representation of this address.
 *
 * The string with present the address in a form usable by Web RTC
 * communication. The format is 
 *
 *    [("stun"|"turn") "://"][username ":" password "@"]hostname[":" port]
 *
 * The username and password will only be visible for a TURN server. If these
 * values are blank, even though the address is for a TURN server, the strings
 * "username" and "password" will be used literally.
 *
 * @return a string representation of this address.
 */
const std::string ICEAddress::toString() const {
	std::string suffix = InetAddress::toString();
	if (turn) {
		std::string u = username.empty() ? "username" : username;
		std::string p = password.empty() ? "password" : password;
		return "turn://"+u+":"+p+"@"+suffix;
	} else {
		return "stun://"+suffix;
	}
}

