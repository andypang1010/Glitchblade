cmake_minimum_required (VERSION 3.7)
project (libjuice
	VERSION 1.3.3
	LANGUAGES C)
set(PROJECT_DESCRIPTION "UDP Interactive Connectivity Establishment (ICE) library")

option(USE_NETTLE "Use Nettle for hash functions" OFF)
option(NO_SERVER "Disable server support" OFF)
option(NO_EXPORT_HEADER "Disable export header" OFF)
option(WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option(CLANG_TIDY "Enable clang-tidy" OFF)

# Mitigations
option(DISABLE_CONSENT_FRESHNESS "Disable RFC 7675 Consent Freshness" OFF)
option(ENABLE_LOCALHOST_ADDRESS "List localhost addresses in candidates" OFF)
option(ENABLE_LOCAL_ADDRESS_TRANSLATION "Translate local addresses to localhost" OFF)

set(C_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/modules/Modules)

include(GNUInstallDirs)

# Define the relative position of this file in the CUGL project
# This is to make this file easily relocatable (for Android, etc.)
set(CMAKE_CURRENT_DEPTH "${PROJECT_SOURCE_DIR}/../../../..")

if(NOT DEFINED PROJECT_DIR_OFFSET)
	set(PROJECT_DIR_OFFSET "../..")
endif()

if(NOT DEFINED DATACHANNEL_PATH)
	set(DATACHANNEL_PATH "${CMAKE_CURRENT_DEPTH}/${PROJECT_DIR_OFFSET}/external/libdatachannel")
endif()

set(JUICE_PATH "${DATACHANNEL_PATH}/deps/libjuice")

if(WIN32)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
	if (MSVC)
		add_definitions(-DNOMINMAX)
		add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	endif()
endif()

if(CLANG_TIDY)
	set(CMAKE_C_CLANG_TIDY clang-tidy)
endif()

set(LIBJUICE_SOURCES
	${JUICE_PATH}/src/addr.c
	${JUICE_PATH}/src/agent.c
	${JUICE_PATH}/src/crc32.c
	${JUICE_PATH}/src/const_time.c
	${JUICE_PATH}/src/conn.c
	${JUICE_PATH}/src/conn_poll.c
	${JUICE_PATH}/src/conn_thread.c
	${JUICE_PATH}/src/conn_mux.c
	${JUICE_PATH}/src/base64.c
	${JUICE_PATH}/src/hash.c
	${JUICE_PATH}/src/hmac.c
	${JUICE_PATH}/src/ice.c
	${JUICE_PATH}/src/juice.c
	${JUICE_PATH}/src/log.c
	${JUICE_PATH}/src/random.c
	${JUICE_PATH}/src/server.c
	${JUICE_PATH}/src/stun.c
	${JUICE_PATH}/src/timestamp.c
	${JUICE_PATH}/src/turn.c
	${JUICE_PATH}/src/udp.c
)
source_group("Source Files" FILES "${LIBJUICE_SOURCES}")

set(LIBJUICE_HEADERS
	${JUICE_PATH}/include/juice/juice.h
)
source_group("Header Files" FILES "${LIBJUICE_HEADERS}")


set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library(juice STATIC ${LIBJUICE_SOURCES})
set_target_properties(juice PROPERTIES VERSION ${PROJECT_VERSION})

target_include_directories(juice PUBLIC
    $<BUILD_INTERFACE:${JUICE_PATH}/include>
    $<INSTALL_INTERFACE:include>)
target_include_directories(juice PRIVATE ${JUICE_PATH}/include/juice)
target_include_directories(juice PRIVATE ${JUICE_PATH}/src)
target_compile_definitions(juice PRIVATE $<$<CONFIG:Release>:RELEASE=1>)
target_compile_definitions(juice PUBLIC JUICE_STATIC)
target_link_libraries(juice PRIVATE Threads::Threads)

if(WIN32)
	target_link_libraries(juice PRIVATE
		ws2_32 # winsock2
		bcrypt)
endif()

target_compile_definitions(juice PRIVATE USE_NETTLE=0)
if(NOT MSVC)
	target_compile_options(juice PRIVATE -Wno-deprecated-pragma)
endif()


if (NO_SERVER)
	target_compile_definitions(juice PRIVATE NO_SERVER)
endif()

if(APPLE)
	# This seems to be necessary on MacOS
	target_include_directories(juice PRIVATE /usr/local/include)
endif()

set_target_properties(juice PROPERTIES EXPORT_NAME LibJuice)
add_library(LibJuice::LibJuice ALIAS juice)

target_compile_definitions(juice PRIVATE JUICE_EXPORTS)

if(NOT MSVC)
	target_compile_options(juice PRIVATE -Wall -Wextra)
endif()

if(WARNINGS_AS_ERRORS)
	if(MSVC)
		target_compile_options(juice PRIVATE /WX)
	else()
		target_compile_options(juice PRIVATE -Werror)
	endif()
endif()

if(DISABLE_CONSENT_FRESHNESS)
	#target_compile_definitions(juice PRIVATE JUICE_DISABLE_CONSENT_FRESHNESS=1)
endif()

if(ENABLE_LOCALHOST_ADDRESS)
	target_compile_definitions(juice PRIVATE JUICE_ENABLE_LOCALHOST_ADDRESS=1)
endif()

if(ENABLE_LOCAL_ADDRESS_TRANSLATION)
	target_compile_definitions(juice PRIVATE JUICE_ENABLE_LOCAL_ADDRESS_TRANSLATION=1)
endif()
