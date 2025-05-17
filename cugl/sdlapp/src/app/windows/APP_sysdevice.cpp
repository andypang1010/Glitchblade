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

#define _WIN32_DCOM
#include <comdef.h>
#include <Wbemidl.h>
#include <iostream>
#include <sstream>

using namespace std;

#pragma comment(lib, "wbemuuid.lib")

/**
 * Returns a UTF8 string from a Windows BSTR
 *
 * Code adapated from
 *
 *     https://stackoverflow.com/questions/52000140/bstr-conversion-to-utf-8
 *
 * @param bstr  The BSTR
 *
 * @return a UTF8 string from a Windows BSTR
 */
static std::string BSTRtoUTF8(BSTR bstr) {
    int len = SysStringLen(bstr);
    if(len == 0) return "";

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, bstr, len, NULL, 0, NULL, NULL);
    char* utf8Char = (char*)malloc(size_needed + 1);
    WideCharToMultiByte(CP_UTF8, 0, bstr, len, utf8Char, size_needed, NULL, NULL);
    utf8Char[size_needed] = 0; // Not guaranteed
    std::string result(utf8Char);
    free(utf8Char);
    return result;
}

/**
 * This is a class to query the WMI information for the local computer.
 *
 * As this is a fairly heavy-weight query, we only want to do this once. But
 * we also do not want to do it if the user does not need this information.
 * Hence this class queries all relevant information the first time that The
 * user asks for anything from the WMI. That information is then cached for
 * the life of the application.
 *
 * As this is an internal class we do not bother to encapulate anything.
 */
class WMIInfo {
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
     * Creates a new WMIInfo object
     *
     * The object is unintialized and has yet to perform a query of the
     * WMI data. That is done with the method {@link #query}.
     */
    WMIInfo() : initialized(false) {}

    /**
     * Deletes this WMIInfo, releasing all resources
     */
    ~WMIInfo() {}

    /**
     * Performs a query of the WMI data.
     *
     * If this method has already (successfully) been called, then this
     * function does nothing.  Otherwise, it connects to the COM system and
     * queries the relevant data for this class.
     *
     * This code is adapted from
     *
     *     https://learn.microsoft.com/en-us/windows/win32/wmisdk/example--getting-wmi-data-from-the-local-computer
     *
     * @return true if the query was successful
     */
    bool query() {
        if (initialized) {
            return true;
        }

        HRESULT hres;
        stringstream stream;

       // Step 1: Initialize COM
       hres =  CoInitializeEx(0, COINIT_APARTMENTTHREADED);
       if (FAILED(hres)) {
           stream   << "Failed to initialize COM library. Error code = 0x"
                    << hex << hres << endl;
           SDL_SetError(stream.str().c_str());
           return false;
       }

       // Step 2: Set general COM security levels
       hres =  CoInitializeSecurity(
           NULL,
           -1,                          // COM authentication
           NULL,                        // Authentication services
           NULL,                        // Reserved
           RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
           RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
           NULL,                        // Authentication info
           EOAC_NONE,                   // Additional capabilities
           NULL                         // Reserved
       );

       if (FAILED(hres)) {
           stream   << "Failed to initialize COM security. Error code = 0x"
                    << hex << hres << endl;
           SDL_SetError(stream.str().c_str());
           CoUninitialize();
           return false;
       }

       // Step 3:  Obtain the initial locator to WMI
       IWbemLocator *pLoc = NULL;
       hres = CoCreateInstance(
           CLSID_WbemLocator,
           0,
           CLSCTX_INPROC_SERVER,
           IID_IWbemLocator, (LPVOID *) &pLoc);

       if (FAILED(hres)) {
           stream   << "Failed to create IWbemLocator object. Error code = 0x"
                    << hex << hres << endl;
           SDL_SetError(stream.str().c_str());
           CoUninitialize();
           return false;
       }

       // Step 4: Connect to WMI through the IWbemLocator::ConnectServer method
       IWbemServices *pSvc = NULL;

       // Connect to the root\cimv2 namespace with
       // the current user and obtain pointer pSvc
       // to make IWbemServices calls.
       hres = pLoc->ConnectServer(
            _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
            NULL,                    // User name. NULL = current user
            NULL,                    // User password. NULL = current
            0,                       // Locale. NULL indicates current
            NULL,                    // Security flags.
            0,                       // Authority (for example, Kerberos)
            0,                       // Context object
            &pSvc                    // pointer to IWbemServices proxy
            );

       if (FAILED(hres)) {
           stream   << "Could not connect to WMI. Error code = 0x"
                    << hex << hres << endl;
           SDL_SetError(stream.str().c_str());
           pLoc->Release();
           CoUninitialize();
           return false;
       }

       // Step 5:  Set security levels on the proxy
       hres = CoSetProxyBlanket(
          pSvc,                        // Indicates the proxy to set
          RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
          RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
          NULL,                        // Server principal name
          RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
          RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
          NULL,                        // client identity
          EOAC_NONE                    // proxy capabilities
       );

       if (FAILED(hres)) {
           stream   << "Could not set WMI proxy blanket. Error code = 0x"
                    << hex << hres << endl;
           SDL_SetError(stream.str().c_str());
           pSvc->Release();
           pLoc->Release();
           CoUninitialize();
           return false;
       }

       // Steps 6 and 7 broken out into functions
       queryOS(pSvc);
       queryBaseBoard(pSvc);

        // Cleanup
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        initialized = true;
        return true;
    }

private:
    /**
     * Queries entries in the Win32_OperatingSystem class.
     *
     * This method is used to acquire the OS name and version, device name,
     * and device id.
     *
     * @param pSvc  A reference to IWbemServices
     *
     * @return true if the query was successful
     */
    bool  queryOS(IWbemServices *pSvc) {
        HRESULT hres;

        // Query the operating system
        IEnumWbemClassObject* pEnumerator = NULL;
        hres = pSvc->ExecQuery(
            bstr_t("WQL"),
            bstr_t("SELECT * FROM Win32_OperatingSystem"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            NULL,
            &pEnumerator);

        if (FAILED(hres)) {
            std::stringstream stream;
            stream   << "Query for operating system failed. Error code = 0x"
                     << hex << hres << endl;
            SDL_SetError(stream.str().c_str());
            return false;
        }

        IWbemClassObject* pclsObj = NULL;
        ULONG uReturn = 0;

        // Extrac the relevant information
        while (pEnumerator) {
            HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
            if(0 == uReturn) { break; }

            VARIANT vtProp;
            VariantInit(&vtProp);

            // DEVICE NAME
            hr = pclsObj->Get(L"CSName", 0, &vtProp, 0, 0);
            if (vtProp.vt == VT_BSTR) {
                device_name = BSTRtoUTF8(vtProp.bstrVal);
            }
            VariantClear(&vtProp);

            // OS NAME
            hr = pclsObj->Get(L"Caption", 0, &vtProp, 0, 0);
            if (vtProp.vt == VT_BSTR) {
                os_name = BSTRtoUTF8(vtProp.bstrVal);
            }
            VariantClear(&vtProp);

            // OS VERSION
            hr = pclsObj->Get(L"Version", 0, &vtProp, 0, 0);
            if (vtProp.vt == VT_BSTR) {
                os_version = BSTRtoUTF8(vtProp.bstrVal);
            }
            VariantClear(&vtProp);

            // IDENTIFIER
            hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);
            if (vtProp.vt == VT_BSTR) {
                device_id = BSTRtoUTF8(vtProp.bstrVal);
            }
            VariantClear(&vtProp);

            pclsObj->Release();
         }
         pEnumerator->Release();
         return true;
    }

    /**
     * Queries entries in the Win32_BaseBoard class.
     *
     * This method is used to acquire the device model.
     *
     * @param pSvc  A reference to IWbemServices
     *
     * @return true if the query was successful
     */
    bool  queryBaseBoard(IWbemServices *pSvc) {
        HRESULT hres;

        // Query the base board
        IEnumWbemClassObject* pEnumerator = NULL;
        hres = pSvc->ExecQuery(
            bstr_t("WQL"),
            bstr_t("SELECT * FROM Win32_Baseboard"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            NULL,
            &pEnumerator);

        if (FAILED(hres)) {
            std::stringstream stream;
            stream   << "Query for base board failed. Error code = 0x"
                     << hex << hres << endl;
            SDL_SetError(stream.str().c_str());
            return false;
        }

        IWbemClassObject* pclsObj = NULL;
        ULONG uReturn = 0;

        // Extrac the relevant information
        while (pEnumerator) {
            HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
            if(0 == uReturn) { break; }

            VARIANT vtProp;
            VariantInit(&vtProp);

            std::string manufacturer;
            std::string model;

            hr = pclsObj->Get(L"Manufacturer", 0, &vtProp, 0, 0);
            if (vtProp.vt == VT_BSTR) {
                manufacturer = BSTRtoUTF8(vtProp.bstrVal);
            }
            VariantClear(&vtProp);

            // OS NAME
            hr = pclsObj->Get(L"Model", 0, &vtProp, 0, 0);
            if (vtProp.vt == VT_BSTR) {
                model = BSTRtoUTF8(vtProp.bstrVal);
            }
            VariantClear(&vtProp);

            if (!manufacturer.empty() && !model.empty()) {
                device_model = manufacturer+" "+model;
            } else if (!model.empty()) {
                device_model = model;
            } else {
                device_model = manufacturer;
            }

            pclsObj->Release();
         }
         pEnumerator->Release();
         return true;
    }

};

/** The WMIInfo Singleton */
static WMIInfo g_wmiinfo;

// C encapulation of C++ function
extern "C" const char* APP_SYS_GetDeviceName(void);

/**
 * System dependent version of APP_GetDeviceName
 *
 * @return the name of this device
 */
const char* APP_SYS_GetDeviceName(void) {
    g_wmiinfo.query();
    return g_wmiinfo.device_name.c_str();
}

// C encapulation of C++ function
extern "C" const char* APP_SYS_GetDeviceModel(void);

/**
 * System dependent version of APP_GetDeviceModel
 *
 * @return the model of this device
 */
const char* APP_SYS_GetDeviceModel(void) {
    if (g_wmiinfo.query()) {
        return g_wmiinfo.device_model.c_str();
    }
    return "UNKNOWN";
}

// C encapulation of C++ function
extern "C" const char* APP_SYS_GetDeviceOS(void);


/**
 * System dependent version of APP_GetDeviceOS
 *
 * @return the operating system running this device
 */
const char* APP_SYS_GetDeviceOS(void) {
    if (g_wmiinfo.query()) {
        return g_wmiinfo.os_name.c_str();
    }
    return "Windows";
}

// C encapulation of C++ function
extern "C" const char* APP_SYS_GetDeviceOSVersion(void);

/**
 * System dependent version of APP_GetDeviceOSVersion
 *
 * @return the operating system version of this device
 */
const char* APP_SYS_GetDeviceOSVersion(void) {
    if (g_wmiinfo.query()) {
        return g_wmiinfo.os_version.c_str();
    }
    return "UNKNOWN";
}

// C encapulation of C++ function
extern "C" const char* APP_SYS_GetDeviceID(void);

/**
 * System dependent version of APP_GetDeviceID
 *
 * @return a unique identifier for this device
 */
const char* APP_SYS_GetDeviceID(void) {
    if (g_wmiinfo.query()) {
        return g_wmiinfo.device_id.c_str();
    }
    return "";
}
