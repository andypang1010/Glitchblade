# Primary CMake file for SDL_app
# This compiles all of the individual SDL components as an interface
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(SDL2_pckg VERSION 2.0)

# Define the relative position of this file in the SDL App project
# This is to make this file easily relocatable (for Android, etc.)
set(CMAKE_CURRENT_DEPTH "${PROJECT_SOURCE_DIR}")

if(NOT DEFINED PROJECT_DIR_OFFSET)
	set(PROJECT_DIR_OFFSET "../../..")
endif()

if(NOT DEFINED SDL2_PCKG_DIR)
	set(SDL2_PCKG_DIR "${CMAKE_CURRENT_DEPTH}/${PROJECT_DIR_OFFSET}")
endif()

if(NOT DEFINED SDL2_MAKE_DIR)
	set(SDL2_MAKE_DIR "${CMAKE_CURRENT_DEPTH}/../../cmake")
endif()

set(SDL2_APP_DIR   "${SDL2_PCKG_DIR}")
set(SDL2_BASE_DIR  "${SDL2_PCKG_DIR}")
set(SDL2_IMAGE_DIR "${SDL2_PCKG_DIR}")
set(SDL2_TTF_DIR   "${SDL2_PCKG_DIR}")
set(SDL2_ATK_DIR   "${SDL2_PCKG_DIR}")

# Resume the normal SDL CMake project

# Get the platform
include("${CMAKE_CURRENT_LIST_DIR}/modules/sdlplatform.cmake")
SDL_DetectCMakePlatform()

option(CMAKE_POSITION_INDEPENDENT_CODE "Build static libraries with -fPIC" ON)
option(SDL2PCKG_BUILD_SHARED_LIBS  "Build all components as a shared library" ON)
set(BUILD_SHARED_LIBS ${SDL2PCKG_BUILD_SHARED_LIBS})

set(COMPONENT_LIBS)

if(SDL2PCKG_BUILD_SHARED_LIBS)
    set(sdl2_pckg_export_name SDL2_pckg)
    set(sdl2_pckg_install_name_infix shared)
else()
    set(sdl2_pckg_export_name SDL2_pckg-static)
    set(sdl2_pckg_install_name_infix static)
endif()

add_library(SDL2_pckg INTERFACE)
add_library(SDL2_pckg::${sdl2_pckg_export_name} ALIAS SDL2_pckg)

## Base SDL
set(SDL_SHARED ${SDL2PCKG_BUILD_SHARED_LIBS})
set(SDL_TEST   OFF)
add_subdirectory(${SDL2_MAKE_DIR}/sdl2 "sdl2.dir")
if (SDL2PCKG_BUILD_SHARED_LIBS)
	list(APPEND COMPONENT_LIBS SDL2)
else()
	list(APPEND COMPONENT_LIBS SDL2-static)
endif()
if (MSVC)
	list(APPEND COMPONENT_LIBS SDL2::SDL2main)
endif()

## SDL Image
set(SDL2IMAGE_BUILD_SHARED_LIBS ${SDL2PCKG_BUILD_SHARED_LIBS})
set(SDL2IMAGE_INSTALL OFF)
set(SDL2IMAGE_VENDORED ON)
add_subdirectory(${SDL2_MAKE_DIR}/sdl2image "sdl2image.dir")
if (SDL2PCKG_BUILD_SHARED_LIBS)
	list(APPEND COMPONENT_LIBS SDL2_image::SDL2_image)
else()
	list(APPEND COMPONENT_LIBS SDL2_image::SDL2_image-static)
endif()

## SDL TTF
set(SDL2TTF_BUILD_SHARED_LIBS ${SDL2PCKG_BUILD_SHARED_LIBS})
set(SDL2TTF_INSTALL OFF)
set(SDL2TTF_VENDORED ON)
add_subdirectory(${SDL2_MAKE_DIR}/sdl2ttf "sdl2ttf.dir")
if (SDL2PCKG_BUILD_SHARED_LIBS)
	list(APPEND COMPONENT_LIBS SDL2_ttf::SDL2_ttf)
else()
	list(APPEND COMPONENT_LIBS SDL2_ttf::SDL2_ttf-static)
endif()

## SDL ATK
set(SDL2ATK_BUILD_SHARED_LIBS ${SDL2PCKG_BUILD_SHARED_LIBS})
set(SDL2ATK_INSTALL OFF)
set(SDL2ATK_TESTS OFF)
set(SDL2ATK_VENDORED ON)
add_subdirectory(${SDL2_MAKE_DIR}/sdl2atk "sdl2atk.dir")
if (SDL2PCKG_BUILD_SHARED_LIBS)
	list(APPEND COMPONENT_LIBS SDL2_atk::SDL2_atk)
else()
	list(APPEND COMPONENT_LIBS SDL2_atk::SDL2_atk-static)
endif()

## SDL APP
set(SDL2APP_BUILD_SHARED_LIBS ${SDL2PCKG_BUILD_SHARED_LIBS})
add_subdirectory(${SDL2_MAKE_DIR}/sdl2app "sdl2app.dir")
if (SDL2PCKG_BUILD_SHARED_LIBS)
	list(APPEND COMPONENT_LIBS SDL2_app::SDL2_app)
else()
	list(APPEND COMPONENT_LIBS SDL2_app::SDL2_app-static)
endif()


## Put it all together
target_link_libraries(SDL2_pckg INTERFACE ${COMPONENT_LIBS})

add_library(SDL2::pckg INTERFACE IMPORTED GLOBAL)
set_target_properties(SDL2::pckg PROPERTIES
    INTERFACE_LINK_LIBRARIES "SDL2_pckg"
)
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.17")
    set_target_properties(SDL2::pckg PROPERTIES
        DEPRECATION "Use SDL2_pckg::SDL2_pckgp or SDL2_pckg::SDL2_pckg-static instead"
    )
endif()