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
#include <cugl/netcode/CUInetAddress.h>
#include <cugl/core/assets/CUJsonValue.h>
#include <cugl/core/util/CUDebug.h>
#include <sstream>

using namespace cugl::netcode;


#pragma mark Constructors

/**
 * Creates an internet address to refer to the localhost
 *
 * The address will be the hostname "localhost". The port will be 0. The
 * constructor does not perform any validation that the combined address
 * port are reachable.
 */
InetAddress::InetAddress() {
    address = "localhost";
    port = 0;
}

/**
 * Creates an internet address to refer to the localhost
 *
 * The address will be the hostname "localhost". The constructor does not 
 * perform any validation that the combined address port are reachable.
 *
 * @param port  The address port
 */
InetAddress::InetAddress(uint16_t port) {
    address = "localhost";
    this->port = port;
}

/**
 * Creates an internet address for the given address.
 *
 * The constructor does not perform any validation that the combined
 * address and port are reachable.
 *
 * @param address   The internet address
 * @param port      The address port
 */
InetAddress::InetAddress(const std::string address, uint16_t port) {    
    this->address = address;
    this->port = port;
}

/**
 * Creates this internet address using a JSON entry.
 *
 * The JSON value should be an object with at least two keys: "address" and
 * "port". The "port" should be an integer.
 *
 * @param prefs      The address settings
 */
InetAddress::InetAddress(const std::shared_ptr<JsonValue>& prefs) {
    address = prefs->getString("address","localhost");
    port = prefs->getInt("port",0);
}

#pragma mark -
#pragma mark Assignment
/**
 * Assigns this address to be a copy of the given internet address.
 *
 * @param src   The address to copy
 *
 * @return a reference to this address for chaining purposes.
 */
InetAddress& InetAddress::set(const InetAddress& src) {
    address = src.address;
    port = src.port;
    return *this;
}

/**
 * Assigns this address to be a copy of the given internet address.
 *
 * @param src   The address to copy
 *
 * @return a reference to this address for chaining purposes.
 */
InetAddress& InetAddress::set(const std::shared_ptr<InetAddress>& src) {
    if (src == nullptr) {
        address = "localhost";
        port = 0;
        return *this;
    }
    
    address = src->address;
    port = src->port;
    return *this;
}

/**
 * Assigns this address to be a copy of given JSON object
 *
 * The JSON value should be an object with at least two keys: "address" and
 * "port". The "port" should be an integer.
 *
 * @param pref      The address settings
 *
 * @return a reference to this address for chaining purposes.
 */
InetAddress& InetAddress::set(const std::shared_ptr<JsonValue>& pref) {
    address = pref->getString("address","localhost");
    port = pref->getInt("port",0);
    return *this;
}


/**
 * Returns a string representation of this address.
 *
 * The string with combine the address string with the port, separated by 
 * a colon. No attempt is made to normalize IPV4 or IPV6 addresses.
 *
 * @return a string representation of this address.
 */
const std::string InetAddress::toString() const {
    if (getType() == Type::IPV6) {
        // We need to put IPV6 in brackets
        return "["+address+"]:"+std::to_string(port);
    }
    return address+":"+std::to_string(port);
}

#pragma mark -
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
InetAddress::Type InetAddress::getType() const {
    Type result = Type::INVALID;
    int colons = tokencount(address,':');
    if (colons >= 2 && colons <= 8) {
        std::vector<std::string> tokens = tokenize(address,':');
        if (isIPV6(tokens.begin(),tokens.end())) {
            result = Type::IPV6;
        }
    } 
    
    if (result == Type::INVALID) {
        std::vector<std::string> tokens = tokenize(address,'.');
        if (tokens.size() == 4 && isIPV4(tokens.begin(),tokens.end())) {
            result = Type::IPV4;
        } else if (isHostname(tokens.begin(),tokens.end())) {
            result = Type::HOSTNAME;        
        }
    }
    return result;
}

/**
 * Returns the number of potential tokens with respect to a separator.
 *
 * @param address    The address to tokenize
 * @param sep        The address separator
 *
 * @return the number of potential tokens with respect to a separator.
 */
int InetAddress::tokencount(std::string address, char sep) {
    int result = 1;
  
    for (auto it = address.begin(); it != address.end(); ++it) {
        if (*it == sep) { result++; }
    }
    
    return result;
}
    
/**
 * Returns the address broken into tokens with respect to a separator.
 *
 * @param address    The address to tokenize
 * @param sep        The address separator
 *
 * @return the address broken into tokens with respect to a separator.
 */
std::vector<std::string> InetAddress::tokenize(std::string address, char sep) {
    std::vector<std::string> tokens;
    std::stringstream check1(address);
    std::string intermediate;
  
    while (getline(check1, intermediate, sep)) {
        tokens.push_back(intermediate);
    }
    
    return tokens;
}

/**
 * Returns true if s is a valid hexadecimal string.
 *
 * The letter components may either be lower or upper case to be valid.
 *
 * @param s    The string to check
 *
 * @return true if s is a valid hexadecimal string.
 */
bool InetAddress::isHexadecimal(std::string s) {
    for (auto it = s.begin(); it != s.end(); ++it) {
        char ch = *it;
        if ((ch < '0' || ch > '9') && (ch < 'A' || ch > 'F') && (ch < 'a' || ch > 'f')) {
            return false;
        }
    }
  
    return s.size() > 0;
}

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
bool InetAddress::isIdentifier(std::string s) {
    for (auto it = s.begin(); it != s.end(); ++it) {
        char ch = *it;
        if ((ch < '0' || ch > '9') && (ch < 'A' || ch > 'Z') && (ch < 'a' || ch > 'z') && (ch != '-')) {
            return false;
        }
    }
  
    return s.size() > 0 && s[0] != '-';
}

/** 
 * Returns true if the given tokens form a valid IPV4 address.
 *
 * The method only checks the syntax of the address, and not whether the address
 * is actually reachable.
 *
 * @param start    Start of the token iterator
 * @param end    End of the token iterator
 *
 * @return true if the given tokens form a valid IPV4 address.
 */
bool InetAddress::isIPV4(const std::vector<std::string>::iterator start,
                         const std::vector<std::string>::iterator end) {    
       // Check if all the tokenized strings lies in the range [0, 255]
    for (auto it = start; it != end; ++it) {
          // Easy cases
          if (*it == "0") { continue; }
        if (it->size() == 0) { return false; }

        int num = 0;
        for (auto jt = it->begin(); jt != it->end(); ++jt) {
            char digit = *jt;
            if (digit > '9'|| digit < '0') {
                return false;
            }
  
            num *= 10;
            num += digit - '0';
            
            if (num == 0) {
                return false;
            }
        }
  
        // Range check for num
        if (num > 255 || num < 0) {
            return false;
        }
    }
  
    return true;
}

/** 
 * Returns true if the given tokens form a valid IPV6 address.
 *
 * The method only checks the syntax of the address, and not whether the address
 * is actually reachable.
 *
 * @param start    Start of the token iterator
 * @param end    End of the token iterator
 *
 * @return true if the given tokens form a valid IPV6 address.
 */
bool InetAddress::isIPV6(const std::vector<std::string>::iterator start,
                         const std::vector<std::string>::iterator end) {
      // Check if all the tokenized strings are in hexadecimal format
    for (auto it = start; it != end; ++it) {
        size_t len = it->size();
  
        if (len > 4 || !(len == 0 || isHexadecimal(*it))) {
            // Check if dual
            if (tokencount(*it,'.') == 4) {
                std::vector<std::string> tokens = tokenize(*it,'.');
                return isIPV4(tokens.begin(),tokens.end());
            } else {
                return false;
            }
        }
    }
    return true;
}

/** 
 * Returns true if the given tokens form a valid hostname.
 *
 * The method only checks the syntax of the address, and not whether the
 * address is actually reachable.
 *
 * @param start    Start of the token iterator
 * @param end    End of the token iterator
 *
 * @return true if the given tokens form a valid hostname.
 */
bool InetAddress::isHostname(const std::vector<std::string>::iterator start,
                             const std::vector<std::string>::iterator end) {
       // Check if all the tokenized strings are identifiers
    for (auto it = start; it != end; ++it) {
        if (!isIdentifier(*it)) {
            return false;
        }
    }
    
    // Ensure final domain is alphabetic
    if (start != end) {
        std::string last = *(end-1);
        for (auto it = last.begin(); it != last.end(); ++it) {
            char ch = *it;
            if ((ch < 'A' || ch > 'Z') && (ch < 'a' || ch > 'z')) {
                return false;
            }
        } 
        return true;
    }
      
    return false;
}
