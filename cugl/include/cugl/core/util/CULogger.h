//
//  CULogger.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a generalization of CULog that (1) creates multiple
//  channels to potentially log to and (2) simultaneously logs to a file and
//  to the output terminal. File logging is particularly useful for longer
//  running games.
//
//  This class is a singleton and should never be allocated directly.  It
//  should only be accessed via the static methods get() and open().
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
#ifndef __CU_LOGGER_H__
#define __CU_LOGGER_H__
#include <unordered_map>
#include <string>
#include <memory>

namespace cugl {

// Forward declaration
class TextWriter;

/**
 * This class provides an interface for fine-grained logging.
 *
 * This class is an alternative to `CULog` that provides a lot more features.
 * First of all it provides multiple log channels, each with its own settings.
 * In addition, each channel has its own associated log file. This allows you
 * to keep separate logs for the purposes of analysis and debugging.
 *
 * Logs are written to the save directory {@link Application#getSaveDirectory}.
 * They are named `<channel>.log` where `<channel>` is the name of the log
 * channel. Each line of the log file is prefixed by the time up to the nearest
 * microsecond. Messages are only written to the file if they have a log level
 * less than or equal to log level of the channel.
 *
 * It is possible to log to a file and the output console (e.g. the output
 * stream for CULog) at the same time. The console has its own log level
 * (defined as {@link #getConsoleLevel}) and will process messages accordingly.
 * Note that the console uses its own timestamps, and so there will be a few
 * microseconds difference between the log file and the console.
 */
class Logger {
public:
    /**
     * An enum to represent the logging state
     *
     * Log-levels are used to prioritize messages. Prioriy in this enumeration
     * is assigned higher priority to lower priority. So {@link Level#FATAL_MSG}
     * has highest priority while {@link Level#VERBOSE_MSG} has the lowest
     * priority. {@link Level#NO_MSG} is a special level indicating that no
     * logging should occur.
     *
     * For any given log channel, you can set it to ignore any messages below
     * a certain priority. For example, if the log channel has a level of
     * {@link Level#INFO_MSG}, then it will ignore any messages of the level
     * {@link Level#DEBUG_MSG}.
     */
    enum class Level : int {
        /** Do not log anything */
        NO_MSG  = 0,
        /** Log only fatal errors */
        FATAL_MSG = 1,
        /** Log all errors of any type */
        ERROR_MSG = 2,
        /** Log all errors and warnings */
        WARN_MSG = 3,
        /** Log useful information (DEFAULT) */
        INFO_MSG  = 4,
        /** Log detailed debugging information */
        DEBUG_MSG  = 5,
        /** Log all information available */
        VERBOSE_MSG = 6
    };
    
protected:
    /** The list of all active logs */
    static std::unordered_map<std::string, std::shared_ptr<Logger>> _channels;
    /** The SDL category to assign to the next allocated log */
    static int _nextcategory;
    
    /** The name of this log channel */
    std::string _name;
    /** The path to the log file */
    std::string _path;
    
    /** The file log level of this logger */
    Level _fileLevel;
    /** The console log level of this logger */
    Level _consLevel;
    
    /** The text writer for outputing to a file */
    std::shared_ptr<TextWriter> _writer;
    
    /** The SDL category for this logger */
    int _category;
    
    /** A buffer for writting timestamp information */
    char  _timestamp[64];
    /** The buffer for formatting log messages */
    char* _buffer;
    /** The capacity of the formatting buffer */
    size_t _capacity;
    
    /** Whether auto flush is active */
    bool _autof;
    
    /** Whether this channel is still open. */
    bool _open;
    
#pragma mark Constructors
public:
    /**
     * Constructs a new logger object.
     *
     * This constructor is only initializes the default values, and does not
     * create a usuable logger. It should never be accessed by the user. Use
     * the static method {@link #open} instead.
     */
    Logger();
    
    /**
     * Deletes this logger, releasing all resources.
     */
    ~Logger() { dispose(); }

private:
    /**
     * Disposes the resources associated with this logger.
     *
     * A disposed logger can be safely reinitialized.
     */
    void dispose();

    /**
     * Initializes a new logger with the given channel and log level.
     *
     * This method will fail if there is an existing logger for the same
     * channel. The level only applies to the log file. The console log level
     * is assumed to be {@link Level#NO_MSG} unless otherwise specified.
     *
     * @param channel   The log channel
     * @param level     The log level for the output file
     *
     * @return true if initialization was successful
     */
    bool init(const std::string channel, Level level);

    /**
     * Expands the size of the formatting buffer.
     *
     * @return size The new size requested.
     */
    void expand(size_t size);
    
#pragma mark Static Accessors
public:
    /**
     * Returns a new logger for the given channel.
     *
     * If this channel already exists, this method will return the existing
     * logger for that channel. A new logger will always start with level
     * {@link Level#INFO_MSG} for the log file and {@link Level#NO_MSG} for
     * the console.
     *
     * @param channel   The log channel
     *
     * @return a new logger for the given channel.
     */
    static std::shared_ptr<Logger> open(const std::string channel);

    /**
     * Returns a new logger for the given channel and log level.
     *
     * The level will apply to the log file. The console will always have
     * level {@link Level#NO_MSG} unless otherwise set by
     * {@link #setConsoleLevel}.
     *
     * If this channel already exists, this method will return the existing
     * logger for that channel. It will also update the log level of that
     * channel to the one specified.
     *
     * @param channel   The log channel
     * @param level     The log level
     *
     * @return a new logger for the given channel and log level.
     */
    static std::shared_ptr<Logger> open(const std::string channel, Level level);

    /**
     * Returns the logger for the given channel.
     *
     * If the specified channel is not open, this method returns nullptr.
     *
     * @param channel   The log channel
     *
     * @return the logger for the given channel.
     */
    static std::shared_ptr<Logger> get(const std::string channel);

    /**
     * Closes the log for the given channel.
     *
     * Once called, any references to the given log are invalid. If any
     * shared pointers to the log persist, any attempt to write to them will
     * fail.
     *
     * @param channel   The log channel
     *
     * @return true if the log channel was successfully closed
     */
    static bool close(const std::string channel);
    
#pragma mark Log Attributes
    /**
     * Returns the channel name for this logger
     *
     * @return the channel name for this logger
     */
    const std::string getName() const { return _name; }

    /**
     * Returns the absolute path to the file for this logger
     *
     * @return the absolute path to the file for this logger
     */
    const std::string getPath() const { return _path; }

    /**
     * Returns the log level for the file associate with this logger.
     *
     * Messages of a lower priority than this log level will not be written to
     * the file. If the level is set to {@link Level#NO_MSG} then no messages
     * will be written to the file.
     *
     * @return the log level for the file associate with this logger.
     */
    Level getLogLevel() const { return _fileLevel; }
    
    /**
     * Sets the log level for the file associate with this logger.
     *
     * Messages of a lower priority than this log level will not be written to
     * the file. If the level is set to {@link Level#NO_MSG} then no messages
     * will be written to the file.
     *
     * Changing the value always flushes any pending messages to the file.
     *
     * @param level the level for the file associate with this logger
     */
    void setLogLevel(Level level);
    
    /**
     * Returns the log level for the console.
     *
     * It is possible to simultaneously log to the log file and the console
     * (e.g. the output stream of CULog). However, messages of a lower priority
     * than this log level will not be written to the console. By default, the
     * console has level {@link Level#NO_MSG}, meaning no messages are written
     * to the console.
     *
     * Note that the console uses its own timestamps, and so there will be a
     * few microseconds difference between the timestamp of a message in the
     * log file and the timestamp in the console.
     *
     * @return the log level for the console.
     */
    Level getConsoleLevel() const { return _consLevel; }

    /**
     * Sets the log level for the console.
     *
     * It is possible to simultaneously log to the log file and the console
     * (e.g. the output stream of CULog). However, messages of a lower priority
     * than this log level will not be written to the console. By default, the
     * console has level {@link Level#NO_MSG}, meaning no messages are written
     * to the console.
     *
     * Note that the console uses its own timestamps, and so there will be a
     * few microseconds difference between the timestamp of a message in the
     * log file and the timestamp in the console.
     *
     * @param level the log level for the console.
     */
    void setConsoleLevel(Level level);
    
    /**
     * Returns true if this logger autoflushes
     *
     * If a logger does not have autoflush, the messages are not guaranteed to
     * be written to the file until {@link #flush} is called. Otherwise, the
     * file is written after every message. To improve performance, you may
     * wish to disable this feature if you are writting a large number of
     * messages per animation frame.
     *
     * @return true if this logger autoflushes
     */
    bool doesAutoFlush() const { return _autof; }
    
    /**
     * Sets whether this logger should autoflush
     *
     * If a logger does not have autoflush, the messages are not guaranteed to
     * be written to the file until {@link #flush} is called. Otherwise, the
     * file is written after every message. To improve performance, you may
     * wish to disable this feature if you are writting a large number of
     * messages per animation frame.
     *
     * @param value whether this logger should autoflush
     */
    void setAutoFlush(bool value);
    
#pragma mark Message Logging
    /**
     * Sends a message to this logger.
     *
     * This method has the same structure as printf and CULog. It consists of
     * a format string followed by zero or more arguments to substititute into
     * the string.
     *
     * The message will be logged with level {@link #getLogLevel} to the file
     * and {@link #getConsoleLevel} to the console.
     *
     * @param format    The formatting string
     * @param ...       The printf-style subsitution arguments
     */
    void log(const std::string format, ...);

    /**
     * Sends a message to this logger.
     *
     * This method has the same structure as printf and CULog. It consists of
     * a format string followed by zero or more arguments to substititute into
     * the string.
     *
     * The message will be logged with level {@link #getLogLevel} to the file
     * and {@link #getConsoleLevel} to the console.
     *
     * @param format    The formatting string
     * @param ...       The printf-style subsitution arguments
     */
    void log(const char* format, ...);

    /**
     * Sends a message to this logger.
     *
     * This method has the same structure as printf and CULog. It consists of
     * a format string followed by zero or more arguments to substititute into
     * the string.
     *
     * The message will be logged with given level to both the file and the
     * console. The message must be of equal priority or higher than
     * {@link #getLogLevel} or {@link #getConsoleLevel} to appear in the file
     * or on the console, respectively.
     *
     * @param level     The priority for this log message
     * @param format    The formatting string
     * @param ...       The printf-style subsitution arguments
     */
    void log(Level level, const std::string format, ...);

    /**
     * Sends a message to this logger.
     *
     * This method has the same structure as printf and CULog. It consists of
     * a format string followed by zero or more arguments to substititute into
     * the string.
     *
     * The message will be logged with given level to both the file and the
     * console. The message must be of equal priority or higher than
     * {@link #getLogLevel} or {@link #getConsoleLevel} to appear in the file
     * or on the console, respectively.
     *
     * @param level     The priority for this log message
     * @param format    The formatting string
     * @param ...       The printf-style subsitution arguments
     */
    void log(Level level, const char* format, ...);

    /**
     * Flushes any pending messages to the log file.
     *
     * If a logger has not set {@link #doesAutoFlush}, the messages are not
     * guaranteed to be written to the file until this method is called.
     * Otherwise, the file is written after every message. To improve
     * performance, you may wish to disable auto flush if you are writting a
     * large number of messages per animation frame.
     */
    void flush();

};
}

#endif /* __CU_LOGGER_H__ */
