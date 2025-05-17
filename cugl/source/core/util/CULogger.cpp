//
//  CULogger.cpp
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
#include <cugl/core/util/CULogger.h>
#include <cugl/core/util/CUDebug.h>
#include <cugl/core/util/CUFiletools.h>
#include <cugl/core/CUApplication.h>
#include <cugl/core/io/CUTextWriter.h>
#include <chrono>
#include <ctime>
#include <cstring>
#include <iomanip>
#include <sstream>

using namespace cugl;

// The buffer size allocated for time stampes
#define STAMP_SIZE 64

/** The list of all active logs */
std::unordered_map<std::string, std::shared_ptr<Logger>> Logger::_channels;

/** The SDL category to assign to the next allocated log */
int Logger::_nextcategory = SDL_LOG_CATEGORY_CUSTOM;

/**
 * Returns the string representation of the given level
 *
 * @param level The log level
 *
 * @return the string representation of the given level
 */
static const char* level2name(Logger::Level level) {
    switch (level) {
        case Logger::Level::NO_MSG:
            return "NONE";
        case Logger::Level::FATAL_MSG:
            return "FATAL";
        case Logger::Level::ERROR_MSG:
            return "ERROR";
        case Logger::Level::WARN_MSG:
            return "WARN";
        case Logger::Level::INFO_MSG:
            return "INFO";
        case Logger::Level::DEBUG_MSG:
            return "DEBUG";
        case Logger::Level::VERBOSE_MSG:
            return "VERBOSE";
    }
    return "INFO";
}

/**
 * Returns the SDL equivalent of the given level
 *
 * @param level The log level
 *
 * @return the SDL equivalent of the given level
 */
static SDL_LogPriority level2sdl(Logger::Level level) {
    switch (level) {
        case Logger::Level::NO_MSG:
            return SDL_LOG_PRIORITY_CRITICAL;
        case Logger::Level::FATAL_MSG:
            return SDL_LOG_PRIORITY_CRITICAL;
        case Logger::Level::ERROR_MSG:
            return SDL_LOG_PRIORITY_ERROR;
        case Logger::Level::WARN_MSG:
            return SDL_LOG_PRIORITY_WARN;
        case Logger::Level::INFO_MSG:
            return SDL_LOG_PRIORITY_INFO;
        case Logger::Level::DEBUG_MSG:
            return SDL_LOG_PRIORITY_DEBUG;
        case Logger::Level::VERBOSE_MSG:
            return SDL_LOG_PRIORITY_VERBOSE;
    }
    return SDL_LOG_PRIORITY_INFO;
}

/**
 * Stores the time stamp in the given buffer.
 *
 * The times stamp includes the date and time up to the nearest microsecond.
 *
 * @param buffer    The buffer to store the time stamp
 * @param size      The size of the buffer
 *
 * @return the length of the string written to the buffer
 */
static size_t stamp_time(char* buffer, size_t size) {
    auto now = std::chrono::system_clock::now();
    auto micro = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()) % 1000000;

    auto timer = std::chrono::system_clock::to_time_t(now);
    std::tm parse = *std::localtime(&timer);
#ifdef __WINDOWS__
    // Windows is apparently not C++11 conformant on strftime
    size_t limit = std::strftime(buffer, size, "%y-%m-%d %H:%M:%S", &parse);
#else
    size_t limit = std::strftime(buffer, size, "%y-%m-%d %0H:%M:%S", &parse);
#endif
    if (limit+9 < size) {
        limit += std::snprintf(buffer+limit, 9, ".%06d",(int)(micro.count()));
    }
    return limit;
}

#pragma mark -
#pragma mark Constructors
/**
 * Constructs a new logger object.
 *
 * This constructor is private and should never be accessed by the user.
 * Use the static method {@link #open} instead.
 */
Logger::Logger() :
_fileLevel(Level::NO_MSG),
_consLevel(Level::NO_MSG),
_buffer(nullptr),
_capacity(0),
_autof(false),
_open(false) {
}

/**
 * Disposes the resources associated with this logger.
 *
 * A disposed logger can be safely reinitialized.
 */
void Logger::dispose() {
    _writer->close();
    _writer = nullptr;
    _open  = false;
    _autof = false;
    _fileLevel = Level::NO_MSG;
    _consLevel = Level::NO_MSG;
}

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
bool Logger::init(const std::string channel, Level level) {
    auto exists = _channels.find(channel);
    if (exists != _channels.end()) {
        CUAssertLog(false, "Channel '%s' already exists.",channel.c_str());
        return false;
    }

    _name = channel;
    
    std::string items[2];
    items[0] = Application::get()->getSaveDirectory();
    items[1] = channel+".log";
    _path = cugl::filetool::join_path(items,2);
    _path = cugl::filetool::canonicalize_path(_path);
    
    _writer = TextWriter::alloc(_path);
    _fileLevel = level;
    if (_writer != nullptr) {
        _open = true;
        _capacity = 256;
        _buffer = (char*)std::malloc(_capacity*sizeof(char));
        _category = _nextcategory++;
        setConsoleLevel(Level::NO_MSG);
        return _buffer != nullptr;
    }
    return false;
}

/**
 * Expands the size of the formatting buffer.
 *
 * @return size The new size requested.
 */
void Logger::expand(size_t size) {
    if (!_open || size < _capacity) {
        return;
    }
    
    if (_buffer) {
        free(_buffer);
        _buffer = nullptr;
    }
    
    while (size > _capacity) {
        _capacity *= 2;
    }
    _buffer = (char*)std::malloc(_capacity*sizeof(char));
    if (_buffer == nullptr) {
        CUAssertLog(false,"Log channel '%s' failed unexpectedly.", _name.c_str());
        dispose();
    }
}

#pragma mark -
#pragma mark Static Accessors
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
std::shared_ptr<Logger> Logger::open(const std::string channel) {
    auto exists = _channels.find(channel);
    if (exists != _channels.end()) {
        return exists->second;
    }
    
    std::shared_ptr<Logger> result = std::make_shared<Logger>();
    
    if (result->init(channel, Level::INFO_MSG)) {
        _channels[channel] = result;
        return result;
    }
    
    return nullptr;
}

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
std::shared_ptr<Logger> Logger::open(const std::string channel,
                                     Logger::Level level) {
    auto exists = _channels.find(channel);
    if (exists != _channels.end()) {
        exists->second->setLogLevel(level);
        return exists->second;
    }
    
    std::shared_ptr<Logger> result = std::make_shared<Logger>();

    if (result->init(channel, level)) {
        _channels[channel] = result;
        return result;
    }
    
    return nullptr;
}

/**
 * Returns the logger for the given channel.
 *
 * If the specified channel is not open, this method returns nullptr.
 *
 * @param channel   The log channel
 *
 * @return the logger for the given channel.
 */
std::shared_ptr<Logger> Logger::get(const std::string channel) {
    auto log = _channels.find(channel);
    if (log == _channels.end()) {
        return nullptr;
    }
    return log->second;

}

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
bool Logger::close(const std::string channel) {
    auto log = _channels.find(channel);
    if (log == _channels.end()) {
        return false;
    }
    auto logger = log->second;
    logger->dispose();
    _channels.erase(log);
    return true;
}

#pragma mark -
#pragma mark Log Attributes
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
void Logger::setLogLevel(Level level) {
    if (_open) {
        _writer->flush();
        _fileLevel = level;
    }
}

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
void Logger::setConsoleLevel(Level level) {
    if (_open) {
        SDL_LogSetPriority(_category, level2sdl(level));
        _consLevel = level;
    }
}

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
void Logger::setAutoFlush(bool value) {
    if (_open) {
        _autof = value;
        if (value) {
            _writer->flush();
        }
    }
}

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
void Logger::log(const std::string format, ...) {
    if (!_open) {
        CUAssertLog(_open, "Channel '%s' is closed.",_name.c_str());
        return;
    }
    
    va_list args;
    va_start (args, format);
    size_t size = vsnprintf(nullptr, 0, format.c_str(), args)+1;
    va_end(args);
    if (size < _capacity) {
        expand(size);
    }
    
    _buffer[0] = '[';
    strcpy(_buffer+1,_name.c_str());
    _buffer[_name.size()+1] = ']';
    _buffer[_name.size()+2] = ' ';

    va_start (args, format);
    size_t off = _name.size()+3;
    vsnprintf(_buffer+off, _capacity-off, format.c_str(), args);
    va_end(args);
    if ((int)_fileLevel > (int)Level::NO_MSG) {
        stamp_time(_timestamp,STAMP_SIZE);
        _writer->write(_timestamp);
        _writer->write(' ');
        _writer->write(level2name(_fileLevel));
        _writer->write(": ");
        _writer->write(_buffer);
        _writer->write('\n');
        if (_autof) {
            _writer->flush();
        }
    }
    if ((int)_consLevel > (int)Level::NO_MSG) {
        SDL_LogMessage(SDL_LOG_CATEGORY_CUSTOM, level2sdl(_consLevel),
                       "%s",_buffer);
    }
}

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
 * @param format    The formatting string
 * @param ...       The printf-style subsitution arguments
 */
void Logger::log(const char* format, ...) {
    if (!_open) {
        CUAssertLog(_open, "Channel '%s' is closed.",_name.c_str());
        return;
    }

    va_list args;
    va_start (args, format);
    size_t size = vsnprintf(nullptr, 0, format, args)+1;
    va_end(args);
    if (size < _capacity) {
        expand(size);
    }
    
    _buffer[0] = '[';
    strcpy(_buffer+1,_name.c_str());
    _buffer[_name.size()+1] = ']';
    _buffer[_name.size()+2] = ' ';

    va_start (args, format);
    size_t off = _name.size()+3;
    vsnprintf(_buffer+off, _capacity-off, format, args);
    va_end(args);
    if ((int)_fileLevel > (int)Level::NO_MSG) {
        stamp_time(_timestamp,STAMP_SIZE);
        _writer->write(_timestamp);
        _writer->write(' ');
        _writer->write(level2name(_fileLevel));
        _writer->write(": ");
        _writer->write(_buffer);
        _writer->write('\n');
        if (_autof) {
            _writer->flush();
        }
    }
    if ((int)_consLevel > (int)Level::NO_MSG) {
        SDL_LogMessage(SDL_LOG_CATEGORY_CUSTOM, level2sdl(_consLevel),
                       "%s",_buffer);
    }
}

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
 * @param format    The formatting string
 * @param ...       The printf-style subsitution arguments
 */
void Logger::log(Level level, const std::string format, ...) {
    if (!_open) {
        CUAssertLog(_open, "Channel '%s' is closed.",_name.c_str());
        return;
    }

    va_list args;
    va_start (args, format);
    size_t size = vsnprintf(nullptr, 0, format.c_str(), args)+_name.size()+4;
    va_end(args);
    if (size < _capacity) {
        expand(size);
    }
    
    _buffer[0] = '[';
    strcpy(_buffer+1,_name.c_str());
    _buffer[_name.size()+1] = ']';
    _buffer[_name.size()+2] = ' ';

    va_start (args, format);
    size_t off = _name.size()+3;
    vsnprintf(_buffer+off, _capacity-off, format.c_str(), args);
    va_end(args);
    if ((int)_fileLevel > (int)Level::NO_MSG &&
        (int)level > (int)Level::NO_MSG &&
        (int)level <= (int)_fileLevel) {
        stamp_time(_timestamp,STAMP_SIZE);
        _writer->write(_timestamp);
        _writer->write(' ');
        _writer->write(level2name(level));
        _writer->write(": ");
        _writer->write(_buffer);
        _writer->write('\n');
        if (_autof) {
            _writer->flush();
        }
    }
    if ((int)_consLevel > (int)Level::NO_MSG &&
        (int)level > (int)Level::NO_MSG) {
        SDL_LogMessage(SDL_LOG_CATEGORY_CUSTOM, level2sdl(level),
                       "%s",_buffer);
    }
}

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
 * @param format    The formatting string
 * @param ...       The printf-style subsitution arguments
 */
void Logger::log(Level level, const char* format, ...) {
    if (!_open) {
        CUAssertLog(_open, "Channel '%s' is closed.",_name.c_str());
        return;
    }

    va_list args;
    va_start (args, format);
    size_t size = vsnprintf(nullptr, 0, format, args)+_name.size()+4;
    va_end(args);
    if (size < _capacity) {
        expand(size);
    }
    
    _buffer[0] = '[';
    strcpy(_buffer+1,_name.c_str());
    _buffer[_name.size()+1] = ']';
    _buffer[_name.size()+2] = ' ';

    va_start (args, format);
    size_t off = _name.size()+3;
    vsnprintf(_buffer+off, _capacity-off, format, args);
    va_end(args);
    if ((int)_fileLevel > (int)Level::NO_MSG &&
        (int)level > (int)Level::NO_MSG &&
        (int)level <= (int)_fileLevel) {
        stamp_time(_timestamp,STAMP_SIZE);
        _writer->write(_timestamp);
        _writer->write(' ');
        _writer->write(level2name(level));
        _writer->write(": ");
        _writer->write(_buffer);
        _writer->write('\n');
        if (_autof) {
            _writer->flush();
        }
    }
    if ((int)_consLevel > (int)Level::NO_MSG &&
        (int)level > (int)Level::NO_MSG) {
        SDL_LogMessage(SDL_LOG_CATEGORY_CUSTOM, level2sdl(level),
                       "%s",_buffer);
    }
}

/**
 * Flushes any pending messages to the log file.
 *
 * If a logger has not set {@link #doesAutoFlush}, the messages are not
 * guaranteed to be written to the file until this method is called.
 * Otherwise, the file is written after every message. To improve
 * performance, you may wish to disable auto flush if you are writting a
 * large number of messages per animation frame.
 */
void Logger::flush() {
    if (_open) {
        _writer->flush();
    }
}
