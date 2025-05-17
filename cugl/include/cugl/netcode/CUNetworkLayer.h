//
//  CUNetworkLayer.h
//  Cornell University Game Library (CUGL)
//
//  This module provides the global initialization (and tear-down) for CUGL 
//  network communications. It must be activated before you can access any
//  network connections.
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
#ifndef __CU_NETWORK_LAYER_H__
#define __CU_NETWORK_LAYER_H__
#include <cugl/core/util/CUTimestamp.h>
#include <unordered_map>
#include <string>
#include <memory>
#include <mutex>
#include <atomic>

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

/** Forward reference to NetcodeConnection*/
class NetcodeConnection;

/**
 * This class represents the networking subsystem.
 *
 * We had originally hoped to do away with such a class. All devices have
 * networking these days, right? However, CUGL does not use the built-in
 * networking API. Instead it uses Web RTC to provide reliable high-speed
 * communication between these devices. As an external subsystem, it must be
 * initialized before use, and shutdown when finished. That is the primary
 * purpose of this class.
 *
 * As a singleton, this class has a private constructor. You should only access 
 * the singleton via the static method {@link #get}. Furthermore, you create
 * and deallocate the singleton with the {@link #start} and {@link #stop}
 * methods appropriately.
 *
 * While it is safe to construct internet addresses without this subsystem, 
 * you must initialize this system before using any of the connection classes:
 * {@link NetcodeConnection}, {@link WebSocket} and {@link WebSocketServer}.
 */
class NetworkLayer {
public:
    /**
     * This enum represents the desired logging for network debugging.
     *
     * Setting a value of {@link #NETCODE} or higher will cause the method
     * {@link #isDebug} to return true.
     */
	enum class Log : int {
		/** Disable logging */
		NONE  = 0,
		/** Log only fatal errors */
		FATAL = 1,
		/** Log all errors of any type */
		ERRORS = 2,
		/** Log all errors and warnings */
		WARNINGS = 3,
        /**
         * Log all errors, warnings, and netcode specific messages
         *
         * This setting (and anything higher), causes {@link #isDebug} to return
         * true. This level will cause netcode-specific debug messages to show
         * but will not show anything other than warnings or errors for the
         * underlying RTC layer.
         */
        NETCODE = 4,
        /**
         * Log all important connection information
         *
         * This level will set {@link #isDebug} to true, showing all of the
         * netcode-specific debug messages. In addition, it will show general
         * info messages from the underlying RTC layer.
         */
		INFO  = 5,
        /**
         * Log all important developer information
         *
         * This level will set {@link #isDebug} to true, showing all of the
         * netcode-specific debug messages. In addition, it will show detailed
         * developer messages from the underlying RTC layer.
         */
		DEVELOPER = 6,
		/** Log all information available */
		VERBOSE = 7
	};

    /**
     * Returns a reference to the networking layer singleton.
     *
     * If the method {@link #start} has not yet been called (or if the system
     * has been shutdown with the method {@link #stop}), this method will return
     * nullptr.
     *
     * @return a reference to the networking layer singleton.
     */
    static NetworkLayer* get() { return _singleton; }

    /**
     * Starts up the RTC networking layer
     *
     * Once this method is called, the {@link #get} method will no longer return
     * nullptr. The classes {@link NetcodeConnection}, {@link WebSocket}, and
     * {@link WebSocketServer} all require this method before they can properly
     * be used.
     *
     * @param level	The desired logging level
     *
     * @return true if the network sublayer was successfully initialized
     */
    static bool start(Log level=Log::NONE);

    /**
     * Shuts down the RTC networking layer
     *
     * Once this method is called, the {@link #get} method will always return 
     * nullptr. Any existing connection instances -- {@link NetcodeConnection},
     * {@link WebSocket}, and {@link WebSocketServer} -- will immediately be
     * disconnected, and any further connection attempts will fail.
     *
     * With that said, it is generally a good idea to dispose of all connection
     * instances **before** calling this method, as this forced cleanup process 
     * can take up to 10 seconds, hanging the application.
     *
     * @return true if the network sublayer was successfully shut down
     */
    static bool stop();

    /**
     * Returns true if the networking layer is in debug mode.
     *
     * @return true if the networking layer is in debug mode.
     */
    bool isDebug() const { return _debug; }

    /**
     * Returns the number of microseconds since the network layer started
     *
     * This value can be used for more fine-grained measurement of message
     * time than render framerate.
     *
     * @return the number of microseconds since the network layer started
     */
    Uint64 getTime() const;
    
private:
    /** The networking layer singleton */
    static NetworkLayer* _singleton;

    /** Whether this manager is in debug mode */
    bool _debug;
    
    /** A timestamp to mark the start of networking */
    Timestamp _timestamp;

    /**
     * Creates the RTC networking layer
     *
     * @param level	The desired logging level
     *
     * This constructor is a private and should never be access by the user.
     */
    NetworkLayer(Log level);

    /**
     * Deallocates the networking layer, releasing all resources
     */
    ~NetworkLayer();
};

	}
}

#endif /* __CU_NETWORK_LAYER_H__ */
