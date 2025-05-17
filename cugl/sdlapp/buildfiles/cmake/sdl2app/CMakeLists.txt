# Primary CMake file for SDL_app
# This file is actually the code for SDL_app
# We have pulled it out as a subproject for interface reasons
cmake_minimum_required(VERSION 3.10)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/modules")
set(MAJOR_VERSION 2)
set(MINOR_VERSION 1)
set(MICRO_VERSION 0)
set(SDL_REQUIRED_VERSION 2.0.9)

if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()

include(PrivateSdlFunctions)
sdl_calculate_derived_version_variables()

set(DYLIB_COMPATIBILITY_VERSION "1.0.0")

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the SDL_app source code and call cmake from there")
endif()

project(SDL2_app
    LANGUAGES C
    VERSION "${FULL_VERSION}"
)

message(STATUS "Configuring ${PROJECT_NAME} ${PROJECT_VERSION}")

# Define the relative position of this file in the SDL App project
set(CMAKE_CURRENT_DEPTH "${PROJECT_SOURCE_DIR}/..")

if(NOT DEFINED PROJECT_DIR_OFFSET)
	set(PROJECT_DIR_OFFSET "../..")
endif()

if(NOT DEFINED SDL2_APP_OFFSET)
	set(SDL2_APP_OFFSET "")
endif()

if(NOT DEFINED SDL2_APP_DIR)
	set(SDL2_APP_DIR "${CMAKE_CURRENT_DEPTH}/${PROJECT_DIR_OFFSET}${SDL2_APP_OFFSET}")
endif()
set(SDL2_APP_SRC   "${SDL2_APP_DIR}/src/app")

# Resume the normal SDL CMake project

# Set defaults preventing destination file conflicts
set(SDL2APP_DEBUG_POSTFIX "d"
    CACHE STRING "Name suffix for debug builds")
mark_as_advanced(SDL2APP_DEBUG_POSTFIX)

include(CMakeDependentOption)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

option(CMAKE_POSITION_INDEPENDENT_CODE "Build static libraries with -fPIC" ON)
option(SDL2APP_BUILD_SHARED_LIBS 	"Build the library as a shared library" ON)
set(BUILD_SHARED_LIBS ${SDL2APP_BUILD_SHARED_LIBS})

if(SDL2APP_BUILD_SHARED_LIBS)
    set(sdl2_app_export_name SDL2_app)
    set(sdl2_app_install_name_infix shared)
    set(sdl2_target_name SDL2::SDL2)
else()
    set(sdl2_app_export_name SDL2_app-static)
    set(sdl2_app_install_name_infix static)
    set(sdl2_target_name SDL2::SDL2-static)
endif()

sdl_find_sdl2(${sdl2_target_name} ${SDL_REQUIRED_VERSION})

# Set PROJECT_VERSION of subprojects to "" if it's project call does not set VERSION
cmake_policy(SET CMP0048 NEW)

# Allow cmake_dependent_option to use "Full Condition Syntax"
if(POLICY CMP0127)
    cmake_policy(SET CMP0127 NEW)
endif()

set(APP_SOURCES
    ${SDL2_APP_SRC}/APP_device.c
    ${SDL2_APP_SRC}/APP_display.c
    ${SDL2_APP_SRC}/APP_version.c
)

if(ANDROID)
	list(APPEND APP_SOURCES ${SDL2_APP_SRC}/android/APP_sysdevice.c)
	list(APPEND APP_SOURCES ${SDL2_APP_SRC}/android/APP_sysdisplay.c)
elseif(DARWIN OR MACOSX)
	list(APPEND APP_SOURCES ${SDL2_APP_SRC}/macosx/APP_sysdevice.m)
	list(APPEND APP_SOURCES ${SDL2_APP_SRC}/macosx/APP_sysdisplay.m)
elseif(IOS OR TVOS)
	list(APPEND APP_SOURCES ${SDL2_APP_SRC}/ios/APP_sysdevice.m)
	list(APPEND APP_SOURCES ${SDL2_APP_SRC}/ios/APP_sysdisplay.c)
elseif(WIN32)
    list(APPEND APP_SOURCES ${SDL2_APP_SRC}/windows/APP_sysdevice.cpp)
    list(APPEND APP_SOURCES ${SDL2_APP_SRC}/dummy/APP_sysdisplay.c)
elseif(LINUX)
    list(APPEND APP_SOURCES ${SDL2_APP_SRC}/linux/APP_sysdevice.cpp)
    list(APPEND APP_SOURCES ${SDL2_APP_SRC}/dummy/APP_sysdisplay.c)
else()
	list(APPEND APP_SOURCES ${SDL2_APP_SRC}/dummy/APP_sysdevice.c)
	list(APPEND APP_SOURCES ${SDL2_APP_SRC}/dummy/APP_sysdisplay.c)
endif()

if (MSVC)
    add_compile_options(/wd4068)
endif()

add_library(SDL2_app ${APP_SOURCES})
target_include_directories(SDL2_app PUBLIC
    "$<BUILD_INTERFACE:${SDL2_APP_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/SDL2>"
)
target_compile_definitions(SDL2_app PRIVATE
    BUILD_SDL
    SDL_BUILD_MAJOR_VERSION=${MAJOR_VERSION}
    SDL_BUILD_MINOR_VERSION=${MINOR_VERSION}
    SDL_BUILD_MICRO_VERSION=${MICRO_VERSION}
)

if (SDL2APP_BUILD_SHARED_LIBS AND (DARWIN OR MACOSX))
    list(APPEND EXTRA_LDFLAGS "-Wl,-framework,AppKit")
endif()

target_link_libraries(SDL2_app PRIVATE ${sdl2_target_name} ${EXTRA_LDFLAGS})
set_target_properties(SDL2_app PROPERTIES
    DEFINE_SYMBOL DLL_EXPORT
    PUBLIC_HEADER SDL_app.h
    EXPORT_NAME ${sdl2_app_export_name}
    C_VISIBILITY_PRESET "hidden"
)
if(NOT ANDROID)
    set_target_properties(SDL2_app PROPERTIES
        DEBUG_POSTFIX "${SDL2APP_DEBUG_POSTFIX}"
    )
    if(APPLE)
        # the SOVERSION property corresponds to the compatibility version and VERSION corresponds to the current version
        # https://cmake.org/cmake/help/latest/prop_tgt/SOVERSION.html#mach-o-versions
        set_target_properties(SDL2_app PROPERTIES
            SOVERSION "${DYLIB_COMPATIBILITY_VERSION}"
            VERSION "${DYLIB_CURRENT_VERSION}"
        )
    else()
        set_target_properties(SDL2_app PROPERTIES
            SOVERSION "${LT_MAJOR}"
            VERSION "${LT_VERSION}"
        )
    endif()
endif()
if(SDL2APP_BUILD_SHARED_LIBS AND (APPLE OR (UNIX AND NOT ANDROID)))
    add_custom_command(TARGET SDL2_app POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E create_symlink "$<TARGET_SONAME_FILE_NAME:SDL2_app>" "libSDL2_app$<$<CONFIG:Debug>:${SDL2APP_DEBUG_POSTFIX}>$<TARGET_FILE_SUFFIX:SDL2_app>"
        # BYPRODUCTS "libSDL2_app$<$<CONFIG:Debug>:${SDL2APP_DEBUG_POSTFIX}>$<TARGET_FILE_SUFFIX:SDL2_app>" # Needs CMake 3.20
        WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
    )
endif()
if(SDL2ATLK_BUILD_SHARED_LIBS)
    if(WIN32 OR OS2)
        set_target_properties(SDL2_app PROPERTIES
            PREFIX ""
        )
    endif()
    if(OS2)
        # OS/2 doesn't support a DLL name longer than 8 characters.
        set_target_properties(SDL2_app PROPERTIES
            OUTPUT_NAME "SDL2app"
        )
    elseif(UNIX AND NOT ANDROID)
        set_target_properties(SDL2_app PROPERTIES
            OUTPUT_NAME "SDL2_app-${LT_RELEASE}"
        )
    endif()
else()
    if(MSVC OR (WATCOM AND (WIN32 OR OS2)))
        set_target_properties(SDL2_app PROPERTIES
            OUTPUT_NAME "SDL2_app-static"
        )
    endif()
endif()

# Use `Compatible Interface Properties` to ensure a shared SDL2_app is built with a shared SDL2
if(SDL2APP_BUILD_SHARED_LIBS)
    set_property(TARGET SDL2_app PROPERTY INTERFACE_SDL2_SHARED ${SDL2APP_BUILD_SHARED_LIBS})
    set_property(TARGET SDL2_app APPEND PROPERTY COMPATIBLE_INTERFACE_BOOL SDL2_SHARED)
endif()

if(SDL2APP_BUILD_SHARED_LIBS)
    sdl_target_link_options_no_undefined(SDL2_app)
endif()

add_library(SDL2_app::${sdl2_app_export_name} ALIAS SDL2_app)

add_library(SDL2::app INTERFACE IMPORTED GLOBAL)
set_target_properties(SDL2::app PROPERTIES
    INTERFACE_LINK_LIBRARIES "SDL2_app"
)
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.17")
    set_target_properties(SDL2::app PROPERTIES
        DEPRECATION "Use SDL2_app::SDL2_app or SDL2_app::SDL2_app-static instead"
    )
endif()
