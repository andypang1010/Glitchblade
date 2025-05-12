# Directory for easier includes
# Anywhere you see include(...) you can check <root>/cmake for that file
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/modules)

#
# Extract version from Makefile
#

file(READ Makefile _MAKEFILE_CONTENTS)

string(REGEX MATCH "KFVER_MAJOR = ([0-9]+)\n" KFVER_MAJOR_MATCH "${_MAKEFILE_CONTENTS}")
if(NOT KFVER_MAJOR_MATCH)
    message(FATAL_ERROR "Cannot extract major (ABI) version from Makefile")
endif()
set(KFVER_MAJOR "${CMAKE_MATCH_1}")

string(REGEX MATCH "KFVER_MINOR = ([0-9]+)\n" KFVER_MINOR_MATCH "${_MAKEFILE_CONTENTS}")
if(NOT KFVER_MINOR_MATCH)
    message(FATAL_ERROR "Cannot extract minor version from Makefile")
endif()
set(KFVER_MINOR "${CMAKE_MATCH_1}")

string(REGEX MATCH "KFVER_PATCH = ([0-9]+)\n" KFVER_PATCH_MATCH "${_MAKEFILE_CONTENTS}")
if(NOT KFVER_PATCH_MATCH)
    message(FATAL_ERROR "Cannot extract patch version from Makefile")
endif()
set(KFVER_PATCH "${CMAKE_MATCH_1}")

set(MAKEFILE_EXTRACTED_VERSION "${KFVER_MAJOR}.${KFVER_MINOR}.${KFVER_PATCH}")

#
# Declare CMake project
#

cmake_minimum_required(VERSION 3.6)
project(kissfft VERSION "${MAKEFILE_EXTRACTED_VERSION}")

# Define the relative position of this file in the SDL App project
set(CMAKE_CURRENT_DEPTH "${PROJECT_SOURCE_DIR}/../../..")

if(NOT DEFINED PROJECT_DIR_OFFSET)
	set(PROJECT_DIR_OFFSET "../..")
endif()

if(NOT DEFINED SDL2_ATK_OFFSET)
	set(SDL2_ATK_OFFSET "")
endif()

if(NOT DEFINED KISS_DIR)
	set(KISS_DIR "${CMAKE_CURRENT_DEPTH}/${PROJECT_DIR_OFFSET}${SDL2_ATK_OFFSET}/external/kissfft")
endif()

# Resume the normal SDL CMake project

#
# CMake configuration options
#

# Principal datatype: double, float (default), int16_t, int32_t, simd

set(KISSFFT_DATATYPE "float" CACHE STRING "Principal datatype of kissfft: double, float (default), int16_t, int32_t, simd")

# Additional options

option(KISSFFT_STATIC "Build kissfft as static (ON) or shared library (OFF)" ON)
option(KISSFFT_OPENMP "Build kissfft with OpenMP support" OFF)
option(KISSFFT_PKGCONFIG "Build pkg-config files" ON)
option(KISSFFT_USE_ALLOCA "Use alloca instead of malloc" OFF)

#
# Validate datatype
#

if (NOT KISSFFT_DATATYPE MATCHES "^double$" AND
    NOT KISSFFT_DATATYPE MATCHES "^float$" AND
    NOT KISSFFT_DATATYPE MATCHES "^int16_t$" AND
    NOT KISSFFT_DATATYPE MATCHES "^int32_t$" AND
    NOT KISSFFT_DATATYPE MATCHES "^simd$")
    message(FATAL_ERROR "Incorrect value of KISSFFT_DATATYPE! It can be one of: double, float, int16_t, int32_t, simd")
endif()

#
# Print principal datatype
#

message(STATUS "Building KissFFT with datatype: ${KISSFFT_DATATYPE}")
set(KISSFFT_OUTPUT_NAME "kissfft-${KISSFFT_DATATYPE}")

#
# Validate KISSFFT_STATIC
#

if (BUILD_SHARED_LIBS AND KISSFFT_STATIC)
    message(FATAL_ERROR "Conflicting CMake configuration: -DBUILD_SHARED_LIBS=ON and -DKISSFFT_STATIC=ON")
endif()

#
# Enable BUILD_SHARED_LIBS for shared library build before
# kissfft library is declared
#

if (NOT KISSFFT_STATIC)
    set(BUILD_SHARED_LIBS ON)
    message(STATUS "Building shared library")
else()
    message(STATUS "Building static library")
endif()

#
# Detect C compiler and pass appropriate flags
#

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    add_compile_options(-ffast-math -fomit-frame-pointer
        -W -Wall -Wno-cast-align -Wcast-qual -Wshadow -Wwrite-strings
        "$<$<COMPILE_LANGUAGE:C>:-Wstrict-prototypes;-Wmissing-prototypes;-Wnested-externs;-Wbad-function-cast>")
endif()

if(MSVC)
    add_compile_options(/wd4267)
endif()

#
# Add GNUInstallDirs for GNU infrastructure before target)include_directories
#

if(CMAKE_SYSTEM_NAME MATCHES "^(Linux|kFreeBSD|GNU)$" AND NOT CMAKE_CROSSCOMPILING)
    include(GNUInstallDirs)
endif()

#
# Declare PKGINCLUDEDIR for kissfft include path
#

set(PKGINCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}/kissfft")
message(STATUS "PKGINCLUDEDIR is ${PKGINCLUDEDIR}")

#
# Declare kissfft library ( libkissfft.a / libkissfft-${KISSFFT_DATATYPE}.so.${MAKEFILE_EXTRACTED_VERSION} )
#

add_library(kissfft
  ${KISS_DIR}/kiss_fft.c
  ${KISS_DIR}/kfc.c
  ${KISS_DIR}/kiss_fftnd.c
  ${KISS_DIR}/kiss_fftndr.c
  ${KISS_DIR}/kiss_fftr.c)

target_include_directories(kissfft PUBLIC
    $<BUILD_INTERFACE:${KISS_DIR}>
    $<INSTALL_INTERFACE:${PKGINCLUDEDIR}>)

#
# Set compile definitions based on datatype and additional support flags
#

set(KISSFFT_COMPILE_DEFINITIONS)

#
# double / float
#

if(KISSFFT_DATATYPE MATCHES "^float$" OR KISSFFT_DATATYPE MATCHES "^double$")
    list(APPEND KISSFFT_COMPILE_DEFINITIONS kiss_fft_scalar=${KISSFFT_DATATYPE})
else()

    #
    # int16_t
    #

    if(KISSFFT_DATATYPE MATCHES "^int16_t$")
        list(APPEND KISSFFT_COMPILE_DEFINITIONS FIXED_POINT=16)
    else()

        #
        # int32_t
        #

        if(KISSFFT_DATATYPE MATCHES "^int32_t$")
            list(APPEND KISSFFT_COMPILE_DEFINITIONS FIXED_POINT=32)
        else()

            #
            # simd
            #

            if(KISSFFT_DATATYPE MATCHES "^simd$")
                list(APPEND KISSFFT_COMPILE_DEFINITIONS USE_SIMD)
                if (NOT MSVC)
                    target_compile_options(kissfft PRIVATE -msse)
                else()
                    target_compile_options(kissfft PRIVATE "/arch:SSE")
                endif()
            endif()
        endif()
    endif()
endif()

#
# OpenMP support
#

if(KISSFFT_OPENMP)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        if (NOT MSVC)
            target_compile_options(kissfft PRIVATE -fopenmp)
            if(${CMAKE_VERSION} VERSION_LESS "3.13.0")
                target_link_libraries(kissfft PRIVATE "-fopenmp")
            else()
                target_link_options(kissfft PRIVATE -fopenmp)
            endif()
        else()
            target_compile_options(kissfft PRIVATE "/openmp")
            if(${CMAKE_VERSION} VERSION_LESS "3.13.0")
                target_link_libraries(kissfft PRIVATE "/openmp")
            else()
                target_link_options(kissfft PRIVATE "/openmp")
            endif()
        endif()
        set(KISSFFT_EXPORT_SUFFIX "-openmp")
        set(KISSFFT_OUTPUT_NAME "kissfft-${KISSFFT_DATATYPE}-openmp")
    else()
        message(FATAL_ERROR "Don't know how to enable OpenMP for this compiler")
    endif()
endif()

#
# Shared / static library
#

if(NOT KISSFFT_STATIC)
    list(APPEND KISSFFT_COMPILE_DEFINITIONS KISS_FFT_SHARED)
    set_target_properties(kissfft PROPERTIES
        C_VISIBILITY_PRESET hidden)
    set(KISSFFT_EXPORT_SUFFIX "${KISSFFT_EXPORT_SUFFIX}-shared")
else()
    set(KISSFFT_EXPORT_SUFFIX "${KISSFFT_EXPORT_SUFFIX}-static")
endif()

#
# Alloca support
#

if(KISSFFT_USE_ALLOCA)
    list(APPEND KISSFFT_COMPILE_DEFINITIONS KISS_FFT_USE_ALLOCA)
endif()

# Set library name, version, soversion and aliases

target_compile_definitions(kissfft PUBLIC ${KISSFFT_COMPILE_DEFINITIONS})
set_target_properties(kissfft PROPERTIES
    OUTPUT_NAME "${KISSFFT_OUTPUT_NAME}"
    DEFINE_SYMBOL KISS_FFT_BUILD
    EXPORT_NAME "${KISSFFT_OUTPUT_NAME}"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${KFVER_MAJOR})
add_library(kissfft::kissfft ALIAS kissfft)
add_library(kissfft::kissfft-${KISSFFT_DATATYPE} ALIAS kissfft)

#
# Build with libm (-lm) on Linux and kFreeBSD
#

if(CMAKE_SYSTEM_NAME MATCHES "^(Linux|kFreeBSD|GNU)$" AND NOT CMAKE_CROSSCOMPILING)
    target_link_libraries(kissfft PRIVATE m)
endif()

#
# Define a helper function to define executable file
#

function(add_kissfft_executable NAME)
    add_executable(${NAME} ${ARGN})
    target_link_libraries(${NAME} PRIVATE kissfft::kissfft)

    #
    # Build with libm (-lm) on Linux and kFreeBSD
    #

    if(CMAKE_SYSTEM_NAME MATCHES "^(Linux|kFreeBSD|GNU)$" AND NOT CMAKE_CROSSCOMPILING)
        target_link_libraries(${NAME} PRIVATE m)
    endif()

    if (NOT KISSFFT_OPENMP)
        set_target_properties(${NAME} PROPERTIES
            OUTPUT_NAME "${NAME}-${KISSFFT_DATATYPE}")
    else()
        if (NOT MSVC)
            target_compile_options(${NAME} PRIVATE -fopenmp)
            if(${CMAKE_VERSION} VERSION_LESS "3.13.0")
                target_link_libraries(${NAME} PRIVATE "-fopenmp")
            else()
                target_link_options(${NAME} PRIVATE -fopenmp)
            endif()
        else()
            target_compile_options(${NAME} PRIVATE "/openmp")
            if(${CMAKE_VERSION} VERSION_LESS "3.13.0")
                target_link_libraries(${NAME} PRIVATE "/openmp")
            else()
                target_link_options(${NAME} PRIVATE "/openmp")
            endif()
        endif()
        set_target_properties(${NAME} PROPERTIES
            OUTPUT_NAME "${NAME}-${KISSFFT_DATATYPE}-openmp")
    endif()
endfunction()
