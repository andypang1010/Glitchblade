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
#ifndef __CU_ICE_ADDRESS_H__
#define __CU_ICE_ADDRESS_H__
#include <cugl/netcode/CUInetAddress.h>
#include <memory>
#include <vector>
#include <string>

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
 * This class represents an ICE internet address (with a username and password).
 *
 * An ICE (Interactive Connectivity Establishment) server is used to provide NAT
 * punchthrough services. This allows players to connect across different networks,
 * or even in the same network when the game lobby is located in another network.
 *
 * ICE servers are categorized as STUN (Session Traversal Utilities for NAT) or
 * TURN (Traversal Using Relays around NAT) servers. STUN servers use simple UDP
 * rerouting to help players find each other. While they are successful on most
 * networks, they can be blocked by aggressive firewalls. TURN servers act as
 * a communication middleman for getting around firewalls.
 *
 * Because STUN servers perform simple rerouting, they are freely available and
 * do not require log-in credentials. Google has several available, such as
 * "stun.l.google.com:19302" or "stun4.l.google.com:19302".
 *
 * TURN servers must actively communicate in a session, and therefore are 
 * unlikely to be free. Most require user accounts and passwords. But the issue
 * is entirely the compute cost. CUGL is compatible with open source TURN
 * servers such as Violet:
 *
 *   https://github.com/paullouisageneau/violet
 *
 * For this reason, this class will always associate a username and password 
 * with a TURN server.
 *
 * Like its parent class, this class is effectively a simple struct. All 
 * attributes are publicly available and we do not use the standard CUGL shared
 * pointer architecture. Internet addresses are designed to be use on the stack,
 * though you can combine them with shared pointers if you wish.
 */
class ICEAddress : public InetAddress {
public:
    /** Whether this is a TURN server (false for STUN) */
    bool turn;

    /** The ICE username (ignored for STUN servers) */
    std::string username;

    /** The ICE password (ignored for STUN servers) */
    std::string password;

#pragma mark Constructors
    /**
     * Creates an ICE address to refer to the localhost
     *
     * The address will be the hostname "localhost". It will be categorized as
     * a STUN server with default port 3478. The constructor does not perform any
     * validation that the combined address port are reachable.
     */
    ICEAddress();

    /**
     * Creates an ICE address to refer to the localhost
     *
     * The address will be the hostname "localhost". It will be categorized as
     * a STUN server. The constructor does not perform any validation that the
     * combined address port are reachable.
     *
     * @param port  The address port
     */
    ICEAddress(uint16_t port);

    /**
     * Creates an ICE address for the given address.
     *
     * The address will be categorized as a STUN server. The constructor does not
     * perform any validation that the combined address and port are reachable.
     *
     * @param address   The address of the STUN server
     * @param port      The address port
     */
    ICEAddress(const std::string address, uint16_t port=3478);

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
    ICEAddress(const std::string address, const std::string username,
    		   const std::string password, uint16_t port=3478);


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
    ICEAddress(const std::shared_ptr<JsonValue>& prefs);

    /**
     * Creates a copy of the given ICE address.
     *
     * This copy constructor is provided so that internet addresses may be
     * used safely on the stack, without the use of pointers.
     *
     * @param src    The original internet address to copy
     */
    ICEAddress(const ICEAddress& src) = default;

    /**
     * Creates a new ICE address with the resources of the given one.
     *
     * This move constructor is provided so that internet addresses may be
     * used efficiently on the stack, without the use of pointers.
     *
     * @param src    The original address contributing resources
     */
    ICEAddress(ICEAddress&& src) = default;

    /**
     * Deletes this ICE address, disposing all resources
     */
    ~ICEAddress() {}

#pragma mark Assignment
    /**
     * Assigns this address to be a copy of the given ICE address.
     *
     * @param src   The address to copy
     *
     * @return a reference to this address for chaining purposes.
     */
    ICEAddress& operator=(const ICEAddress& src) = default;

    /**
     * Assigns this address to have the resources of the given ICE address.
     *
     * @param src   The address to take resources from
     *
     * @return a reference to this address for chaining purposes.
     */
    ICEAddress& operator=(ICEAddress&& src) = default;

    /**
     * Assigns this address to be a copy of the given ICE address.
     *
     * @param src   The address to copy
     *
     * @return a reference to this address for chaining purposes.
     */
    ICEAddress& set(const ICEAddress& src);

    /**
     * Assigns this address to be a copy of the given ICE address.
     *
     * @param src   The address to copy
     *
     * @return a reference to this address for chaining purposes.
     */
    ICEAddress& set(const std::shared_ptr<ICEAddress>& src);

     /**
     * Assigns this address according to the given JSON object
     *
     * The JSON value should be an object with at least three keys: "address",
     * "port", and "turn". The "port" should be an integer, while "turn" is
     * a boolean.  If "turn" is true, this constructor will search for additional
     * keys "username" and "password".
     *
     * @param pref      The address settings
     *
     * @return a reference to this address for chaining purposes.
     */
    virtual ICEAddress& set(const std::shared_ptr<JsonValue>& pref) override;

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
    virtual const std::string toString() const override;

};

    }
}
#endif
