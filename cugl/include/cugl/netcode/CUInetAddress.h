//
//  CUInetAddress.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a struct for a internet address-port pair, together
//  with some simple address validation. Because internet addresses are intended
//  to be on the stack, we do not provide explicit shared pointer support for
//  this class.
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
#ifndef __CU_INET_ADDRESS_H__
#define __CU_INET_ADDRESS_H__
#include <memory>
#include <vector>
#include <string>

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

/**
 * This class represents an internet address
 *
 * This class is effectively a simple struct. All attributes are publicly 
 * available and we do not use the standard CUGL shared pointer architecture.
 * Internet addresses are designed to be use on the stack, though you can
 * combine them with shared pointers if you wish.
 *
 * This class does have methods for validating an address, as well as determining 
 * its type (IPV4, IPV6 or Hostname). The latter is important for converting
 * the address to a string, as IPV6 addresses must be inclosed in brackets when
 * combined with the port. Because the attributes are publicly accessible, none
 * of this information is cached. Instead, it is computed on demand as necessary.
 */
class InetAddress {
public:
    /**
     * This enum represents the interet address type.
     *
     * This value allows us to have a single class that supports both IPV4 and IPV6
     * address values.
     */
    enum class Type : int {
        /** Indicates that the address is not one of the given types */
        INVALID = 0,
        /** Indicates that the IP address is formatted for IPV4. */
        IPV4 = 1,
        /** Indicates that the IP address is formatted for IPV6. */
        IPV6 = 2,
        /**
         * Indicates that the IP address refers to a host name.
         *
         * Like IPV4 addresses, hostnames are separated by dots. A hostname is 
         * identifiable because the toplevel domain *must* be alphabetic.
         */
        HOSTNAME = 3,
    };

    /** The internet address */
    std::string address;

    /** The address port */
    uint16_t port;

#pragma mark Constructors
    /**
     * Creates an internet address to refer to the localhost
     *
     * The address will be the hostname "localhost". The port will be 0. The
     * constructor does not perform any validation that the combined address
     * port are reachable.
     */
    InetAddress();
    
    /**
     * Creates an internet address to refer to the localhost
     *
     * The address will be the hostname "localhost". The constructor does not 
     * perform any validation that the combined address port are reachable.
     *
     * @param port  The address port
     */
    InetAddress(uint16_t port);
    
    /**
     * Creates an internet address for the given address.
     *
     * The constructor does not perform any validation that the combined
     * address and port are reachable.
     *
     * @param address   The internet address
     * @param port      The address port
     */
    InetAddress(const std::string address, uint16_t port=0);

    /**
     * Creates this internet address using a JSON entry.
     *
     * The JSON value should be an object with at least two keys: "address" and
     * "port". The "port" should be an integer.
     *
     * @param prefs      The address settings
     */
    InetAddress(const std::shared_ptr<JsonValue>& prefs);

    /**
     * Creates a copy of the given internet address.
     *
     * This copy constructor is provided so that internet addresses may be
     * safely used on the stack, without the use of pointers.
     *
     * @param src    The original internet address to copy
     */
    InetAddress(const InetAddress& src) = default;

    /**
     * Creates a new internet address with the resources of the given one.
     *
     * This move constructor is provided so that internet addresses may be
     * used efficiently on the stack, without the use of pointers.
     *
     * @param src    The original address contributing resources
     */
    InetAddress(InetAddress&& src) = default;

    /**
     * Deletes this internet address, disposing all resources
     */
    ~InetAddress() {}

#pragma mark Assignment
    /**
     * Assigns this address to be a copy of the given internet address.
     *
     * @param src   The address to copy
     *
     * @return a reference to this address for chaining purposes.
     */
    InetAddress& operator=(const InetAddress& src) = default;
    
    /**
     * Assigns this address to have the resources of the given internet address.
     *
     * @param src   The address to take resources from
     *
     * @return a reference to this address for chaining purposes.
     */
    InetAddress& operator=(InetAddress&& src) = default;
    
    /**
     * Assigns this address to be a copy of the given internet address.
     *
     * @param src   The address to copy
     *
     * @return a reference to this address for chaining purposes.
     */
    InetAddress& set(const InetAddress& src);

    /**
     * Assigns this address to be a copy of the given internet address.
     *
     * @param src   The address to copy
     *
     * @return a reference to this address for chaining purposes.
     */
    InetAddress& set(const std::shared_ptr<InetAddress>& src);

    /**
     * Assigns this address according to the given JSON object
     *
     * The JSON value should be an object with at least two keys: "address" and
     * "port". The "port" should be an integer.
     *
     * @param pref      The address settings
     *
     * @return a reference to this address for chaining purposes.
     */
    virtual InetAddress& set(const std::shared_ptr<JsonValue>& pref);
    
    /**
     * Returns a string representation of this address.
     *
     * The string with combine the address string with the port, separated by 
     * a colon. No attempt is made to normalize IPV4 or IPV6 addresses.
     *
     * @return a string representation of this address.
     */
    virtual const std::string toString() const;
    
#pragma mark Validators
    
    /** 
     * Returns the type of this address.
     *
     * The method only checks the syntax of the address, and not whether the 
     * address is actually reachable. As the address attributes are publicly 
     * accessible, this value is not cached, but is instead recomputed each 
     * time this method is called.
     *
     * @return the type of this address.
     */
    Type getType() const;
    
    /** 
     * Returns the true if this address is syntactically valid.
     *
     * The method only checks the syntax of the address, and not whether the 
     * address is actually reachable. As the address attributes are publicly 
     * accessible, this value is not cached, but is instead recomputed each 
     * time this method is called.
     *
     * @return the true if this address is syntactically valid.
     */
    bool isValid() const { return getType() == Type::INVALID; }
    
private:
    // TODO: Move these to CUGL string tools
    /**
     * Returns the number of potential tokens with respect to a separator.
     *
     * @param address    The address to tokenize
     * @param sep        The address separator
     *
     * @return the number of potential tokens with respect to a separator.
     */
    static int tokencount(std::string address, char sep);

    /**
     * Returns the address broken into tokens with respect to a separator.
     *
     * @param address    The address to tokenize
     * @param sep        The address separator
     *
     * @return the address broken into tokens with respect to a separator.
     */
    static std::vector<std::string> tokenize(std::string address, char sep);

    /**
     * Returns true if s is a valid hexadecimal string.
     *
     * The letter components may either be lower or upper case to be valid.
     *
     * @param s    The string to check
     *
     * @return true if s is a valid hexadecimal string.
     */
    static bool isHexadecimal(std::string s);
    
    /**
     * Returns true if s is a valid hostname identifier
     *
     * Hostname identifiers include ASCII letters, numbers, and hyphens.
     * They may not start with a hyphen.
     *
     * @param s    The string to check
     *
     * @return true if s is valid hostname identifier
     */
    static bool isIdentifier(std::string s);

    /** 
     * Returns true if the given tokens form a valid IPV4 address.
     *
     * The method only checks the syntax of the address, and not whether the 
     * address is actually reachable.
     *
     * @param start    Start of the token iterator
     * @param end    End of the token iterator
     *
     * @return true if the given tokens form a valid IPV4 address.
     */
    static bool isIPV4(const std::vector<std::string>::iterator start,
                       const std::vector<std::string>::iterator end);    

    /** 
     * Returns true if the given tokens form a valid IPV6 address.
     *
     * The method only checks the syntax of the address, and not whether the 
     * address is actually reachable.
     *
     * @param start    Start of the token iterator
     * @param end    End of the token iterator
     *
     * @return true if the given tokens form a valid IPV6 address.
     */
    static bool isIPV6(const std::vector<std::string>::iterator start,
                       const std::vector<std::string>::iterator end);    

    /** 
     * Returns true if the given tokens form a valid hostname.
     *
     * The method only checks the syntax of the address, and not whether the address
     * is actually reachable.
     *
     * @param start    Start of the token iterator
     * @param end    End of the token iterator
     *
     * @return true if the given tokens form a valid hostname.
     */
    static bool isHostname(const std::vector<std::string>::iterator start,
                              const std::vector<std::string>::iterator end);    
    
};

    }
}
#endif
