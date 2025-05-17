cmake_minimum_required(VERSION 3.6)
project(vorbis)
if (POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif(POLICY CMP0077)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/modules")

# Define the relative position of this file in the SDL App project
set(CMAKE_CURRENT_DEPTH "${PROJECT_SOURCE_DIR}/../../..")

if(NOT DEFINED PROJECT_DIR_OFFSET)
	set(PROJECT_DIR_OFFSET "../..")
endif()

if(NOT DEFINED SDL2_ATK_OFFSET)
	set(SDL2_ATK_OFFSET "")
endif()

if(NOT DEFINED VORB_DIR)
	set(VORB_DIR "${CMAKE_CURRENT_DEPTH}/${PROJECT_DIR_OFFSET}${SDL2_ATK_OFFSET}/external/vorbis")
endif()

# Resume the normal SDL CMake project

# Required modules
include(CTest)
include(GNUInstallDirs)
include(CheckIncludeFiles)
include(CheckLibraryExists)

# Build options
option(BUILD_SHARED_LIBS "Build shared library" OFF)
if(APPLE)
    option(BUILD_FRAMEWORK "Build Framework bundle for OSX" OFF)
endif()

if(BUILD_FRAMEWORK)
	set(BUILD_SHARED_LIBS TRUE)
endif()

option(INSTALL_CMAKE_PACKAGE_MODULE "Install CMake package configuration module" ON)

# Extract project version from configure.ac
file(READ configure.ac CONFIGURE_AC_CONTENTS)
string(REGEX MATCH "AC_INIT\\(\\[libvorbis\\],\\[([0-9]*).([0-9]*).([0-9]*)" DUMMY ${CONFIGURE_AC_CONTENTS})
set(PROJECT_VERSION_MAJOR ${CMAKE_MATCH_1})
set(PROJECT_VERSION_MINOR ${CMAKE_MATCH_2})
set(PROJECT_VERSION_PATCH ${CMAKE_MATCH_3})
set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

# Helper function to get version-info
function(get_version_info result current_var_name age_var_name revision_var_name)
    string(REGEX MATCH "${current_var_name}=([0-9]*)" DUMMY ${CONFIGURE_AC_CONTENTS})
    set(VERSION_INFO_CURRENT ${CMAKE_MATCH_1})

    string(REGEX MATCH "${age_var_name}=([0-9]*)" DUMMY ${CONFIGURE_AC_CONTENTS})
    set(VERSION_INFO_AGE ${CMAKE_MATCH_1})

    string(REGEX MATCH "${revision_var_name}=([0-9]*)" DUMMY ${CONFIGURE_AC_CONTENTS})
    set(VERSION_INFO_REVISION ${CMAKE_MATCH_1})

    math(EXPR VERSION_INFO_CURRENT_MINUS_AGE "${VERSION_INFO_CURRENT} - ${VERSION_INFO_AGE}")

    set(${result} "${VERSION_INFO_CURRENT_MINUS_AGE}.${VERSION_INFO_AGE}.${VERSION_INFO_REVISION}" PARENT_SCOPE)
endfunction()

# Helper function to configure pkg-config files
function(configure_pkg_config_file pkg_config_file_in)
    set(prefix ${CMAKE_INSTALL_PREFIX})
    set(exec_prefix ${CMAKE_INSTALL_FULL_BINDIR})
    set(libdir ${CMAKE_INSTALL_FULL_LIBDIR})
    set(includedir ${CMAKE_INSTALL_FULL_INCLUDEDIR})
    set(VERSION ${PROJECT_VERSION})
    string(REPLACE ".in" "" pkg_config_file ${pkg_config_file_in})
    configure_file(${pkg_config_file_in} ${pkg_config_file} @ONLY)
endfunction()

message(STATUS "Configuring ${PROJECT_NAME} ${PROJECT_VERSION}")

# Find math library

check_library_exists(m floor "" HAVE_LIBM)
if(HAVE_LIBM)
    set(VORBIS_LIBS "-lm")
endif()

# Find ogg dependency
if(NOT TARGET Ogg::ogg)
    find_package(Ogg REQUIRED)
endif()

if(MSVC)
    add_compile_options(/wd4305)
    add_compile_options(/wd4334)
    add_compile_options(/wd4244)
    add_compile_options(/wd4267)
endif()

add_subdirectory(lib)

configure_pkg_config_file(${VORB_DIR}/vorbis.pc.in)
configure_pkg_config_file(${VORB_DIR}/vorbisenc.pc.in)
configure_pkg_config_file(${VORB_DIR}/vorbisfile.pc.in)
