/*
  Simple DirectMedia Layer Extensions
  Copyright (C) 2022 Walker White

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../APP_sysdisplay.h"
#include <cstdio>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/utsname.h>
#include <unordered_map>

#define LINE_SIZE 1024

/**
 * Reads the first line of file path into the buffer.
 *
 * At most len elements are read into buffer. In addition thie function
 * reads until it reaches a newline or carriage return. The data read is
 * guaranteed to be null terminated.
 *
 * @param path      The path name of the file to read
 * @param buffer    The buffer to store the data
 * @param len       The maximum number of characters to read
 *
 * @return the number of characters read into buffer
 */
static size_t read_first_line(const char* path, char* buffer, size_t len) {
    FILE *file = fopen(path,"r");
    if (file == NULL) {
        return 0;
    }

    int strend = -1;
    int ii;
    for (ii = 0; !feof(file) && ii < len && strend < 0; ii++) {
        buffer[ii] = fgetc(file);
        if (buffer[ii] <= 0 || buffer[ii] == '\n' || buffer[ii] == '\r') {
            strend = ii;
            buffer[ii] = 0;
        }
    }
    if (strend < 0) {
        strend = ii;
        buffer[ii] = 0;
    }
    return strend;
}


/**
 * This is a class to query hostnamectl for the local computer.
 *
 * As this is a fairly heavy-weight query, we only want to do this once. But
 * we also do not want to do it if the user does not need this information.
 * Hence this class queries all relevant information the first time that The
 * user asks for anything from hostnamectl. That information is then cached for
 * the life of the application.
 *
 * Note that the data stored by hostnamectl is OS dependent. Therefore, we
 * use backup methods like gethostname(), gethostid(), etc. if it does not
 * have the information we need.
 *
 * As this is an internal class we do not bother to encapulate anything.
 */
class HostInfo {
public:
    /** Whether we have performed our initial query */
    bool initialized;
    /** The display name of this windows device */
    std::string device_name;
    /** The device model (taken from the motherboard) */
    std::string device_model;
    /** The OS name (this typically includes the major version) */
    std::string os_name;
    /** The OS version */
    std::string os_version;
    /** The serial number for this installation of Windows */
    std::string device_id;

    /**
     * Creates a new HostInfo object
     *
     * The object is unintialized and has yet to perform a query of the
     * hostnamectl data. That is done with the method {@link #query}.
     */
    HostInfo() : initialized(false) {}

    /**
     * Deletes this HostInfo, releasing all resources
     */
    ~HostInfo() { }

    /**
     * Performs a query of this computer
     *
     * If this method has already (successfully) been called, then this
     * function does nothing. Otherwise, it queried hostnamectl and
     * aggregates the results. If this is not sufficient to get all of
     * the data that we need, we have a back-up method for each field.
     */
    void query() {
        if (initialized) {
            return;
        }

        hostnamectl();

        if (device_name.empty()) {
            backup_name();
        }
        if (device_model.empty()) {
            backup_model();
        }
        if (os_name.empty()) {
            backup_os();
        }
        if (os_version.empty()) {
            backup_version();
        }
        if (device_id.empty()) {
            backup_identifier();
        }

        initialized = true;
    }

private:

    /**
     * Extracts information from hostnamectl
     *
     * This function constructs a dictionary from hostnamectl, and
     * uses this dictionary to assign the relevant fields.
     */
    void hostnamectl() {
        FILE *fp;
        char line[LINE_SIZE];

        /* Open the command for reading. */
        fp = popen("/usr/bin/hostnamectl", "r");
        if (fp == NULL) {
            return;
        }

        std::unordered_map<std::string,std::string> dictionary;

        /* Read the output a line at a time  */
        while (fgets(line, LINE_SIZE, fp) != NULL) {
            // Extract the key
            size_t pos0 = 0;
            size_t pos1 = 0;

            while (pos0 < LINE_SIZE && line[pos0] == ' ') {
                pos0++;
            }
            pos1 = pos0;
            while (pos1 < LINE_SIZE && line[pos1] != ':') {
                pos1++;
            }
            if (pos1 < LINE_SIZE) {
                line[pos1] = 0;
            }
            std::string key(line+pos0);

            pos0 = pos1+1;
            while (pos0 < LINE_SIZE && line[pos0] == ' ') {
                pos0++;
            }
            while (pos1 < LINE_SIZE && line[pos1] != '\n') {
                pos1++;
            }
            if (pos1 < LINE_SIZE) {
                line[pos1] = 0;
            }
            std::string value(line+pos0);

            dictionary[key] = value;
        }

        // Now extract the information
        {
            auto it = dictionary.find("Static hostname");
            if (it != dictionary.end()) {
                device_name = it->second.c_str();
            }
        }

        {
            std::string model;
            std::string vendor;
            auto it = dictionary.find("Hardware Model");
            if (it != dictionary.end()) {
                model = it->second.c_str();
            }
            it = dictionary.find("Hardware Vendor");
            if (it != dictionary.end()) {
                vendor = it->second.c_str();
            }
            if (!vendor.empty() && !model.empty()) {
                device_model = model+" ("+vendor+")";
            } else if (!model.empty()) {
                device_model = model;
            } else {
                device_model = vendor;
            }
        }

        {
            auto it = dictionary.find("Operating System");
            if (it != dictionary.end()) {
                os_name = it->second.c_str();
            }
        }

        {
            auto it = dictionary.find("Kernel");
            if (it != dictionary.end()) {
                os_version = it->second.c_str();
            }
        }

        {
            auto it = dictionary.find("Machine ID");
            if (it != dictionary.end()) {
                device_id = it->second.c_str();
            } else {
                it = dictionary.find("Boot ID");
                if (it != dictionary.end()) {
                    device_id = it->second.c_str();
                }
            }
        }

        /* close */
        pclose(fp);
        return;
    }

    /**
     * Acquire the device name from gethostname.
     *
     * This method is used only if hostnamectl fails.
     */
    void backup_name() {
        char buffer[LINE_SIZE];

        gethostname(buffer, LINE_SIZE);
        buffer[LINE_SIZE-1] = 0; // Because this is undefined
        device_name = buffer;
    }

    /**
     * Acquire the device model from /sys/devices/virtual/dmi.
     *
     * This method is used only if hostnamectl fails.
     */
    void backup_model() {
        char buffer[LINE_SIZE];

        size_t amt = read_first_line("/sys/devices/virtual/dmi/id/product_name",buffer,LINE_SIZE);
        if (amt < LINE_SIZE-4) {
            size_t ext = LINE_SIZE-amt-4;
            ext = read_first_line("/sys/devices/virtual/dmi/id/sys_vendor",buffer+amt+2,ext);
            if (ext > 0) {
                // Glue them together
                buffer[amt] = ' ';
                buffer[amt+1] = '(';
                buffer[amt+ext+2] = ')';
                buffer[amt+ext+3] = 0;
                amt += ext+3;
            }
        }

        device_model = amt == 0 ? "UNKNOWN" : buffer;
    }

    /**
     * Acquire the os name from uname.
     *
     * This method is used only if hostnamectl fails.
     */
    void backup_os() {
        struct utsname buffer;
        if (uname(&buffer) == 0) {
            os_name = buffer.sysname;
        } else {
            os_name = "Linux";
        }
    }

    /**
     * Acquire the os version from uname.
     *
     * This method is used only if hostnamectl fails.
     */
    void backup_version() {
        struct utsname buffer;
        if (uname(&buffer) == 0) {
            os_name = buffer.version;
        } else {
            os_name = "UNKNOWN";
        }
    }

    /**
     * Acquire the device id from gethostid.
     *
     * This method is used only if hostnamectl fails.
     */
    void backup_identifier() {
        char buffer[LINE_SIZE];

        long value = gethostid();
        if (value >= 0) {
            sprintf(buffer, "%0lx", (unsigned long)value);
            device_id = buffer;
        }
    }

};

/** The HostInfo Singleton */
static HostInfo g_hostinfo;

// C encapulation of C++ function
extern "C" const char* APP_SYS_GetDeviceName(void);

/**
 * System dependent version of APP_GetDeviceName
 *
 * @return the name of this device
 */
const char* APP_SYS_GetDeviceName(void) {
    g_hostinfo.query();
    return g_hostinfo.device_name.c_str();
}

// C encapulation of C++ function
extern "C" const char* APP_SYS_GetDeviceModel(void);

/**
 * System dependent version of APP_GetDeviceModel
 *
 * @return the model of this device
 */
const char* APP_SYS_GetDeviceModel(void) {
    g_hostinfo.query();
    return g_hostinfo.device_model.c_str();
}

// C encapulation of C++ function
extern "C" const char* APP_SYS_GetDeviceOS(void);

/**
 * System dependent version of APP_GetDeviceOS
 *
 * @return the operating system running this device
 */
const char* APP_SYS_GetDeviceOS(void) {
    g_hostinfo.query();
    return g_hostinfo.os_name.c_str();

}

// C encapulation of C++ function
extern "C" const char* APP_SYS_GetDeviceOSVersion(void);

/**
 * System dependent version of APP_GetDeviceOSVersion
 *
 * @return the operating system version of this device
 */
const char* APP_SYS_GetDeviceOSVersion(void) {
    g_hostinfo.query();
    return g_hostinfo.os_version.c_str();
}

// C encapulation of C++ function
extern "C" const char* APP_SYS_GetDeviceID(void);

/**
 * System dependent version of APP_GetDeviceID
 *
 * @return a unique identifier for this device
 */
const char* APP_SYS_GetDeviceID(void) {
    g_hostinfo.query();
    return g_hostinfo.device_id.c_str();

}
