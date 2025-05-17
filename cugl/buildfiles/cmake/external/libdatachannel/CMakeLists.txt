cmake_minimum_required(VERSION 3.7)
project(libdatachannel
	VERSION 0.19.4
	LANGUAGES CXX)
set(PROJECT_DESCRIPTION "C/C++ WebRTC network library featuring Data Channels, Media Transport, and WebSockets")

include(GNUInstallDirs)
set(CMAKE_CURRENT_DEPTH "${PROJECT_SOURCE_DIR}")

# Define the relative position of this file in the CUGL project
# This is to make this file easily relocatable (for Android, etc.)
set(CMAKE_CURRENT_DEPTH "${PROJECT_SOURCE_DIR}/../..")

if(NOT DEFINED PROJECT_DIR_OFFSET)
	set(PROJECT_DIR_OFFSET "../..")
endif()

if(NOT DEFINED DATACHANNEL_PATH)
	set(DATACHANNEL_PATH "${CMAKE_CURRENT_DEPTH}/${PROJECT_DIR_OFFSET}/external/libdatachannel")
endif()

# Options (pared down)
option(NO_WEBSOCKET "Disable WebSocket support" OFF)
option(NO_MEDIA "Disable media transport support" OFF)
option(WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option(CAPI_STDCALL "Set calling convention of C API callbacks stdcall" OFF)
option(SCTP_DEBUG "Enable SCTP debugging output to verbose log" OFF)

option(BUILD_OPENSSL "Build OpenSSL from source" ON)
option(ENABLE_OPENSSL "Enable OpenSSL crypto engine for SRTP" ON)


list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/modules/Modules)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS OFF) # to force usrsctp to be built static

if(WIN32)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
	if(MSVC)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
		add_definitions(-DNOMINMAX)
		add_definitions(-D_CRT_SECURE_NO_WARNINGS)
		add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
		add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	endif()
endif()

set(LIBDATACHANNEL_SOURCES
	${DATACHANNEL_PATH}/src/candidate.cpp
	${DATACHANNEL_PATH}/src/channel.cpp
	${DATACHANNEL_PATH}/src/configuration.cpp
	${DATACHANNEL_PATH}/src/datachannel.cpp
	${DATACHANNEL_PATH}/src/description.cpp
	${DATACHANNEL_PATH}/src/mediahandler.cpp
	${DATACHANNEL_PATH}/src/global.cpp
	${DATACHANNEL_PATH}/src/message.cpp
	${DATACHANNEL_PATH}/src/peerconnection.cpp
	${DATACHANNEL_PATH}/src/rtcpreceivingsession.cpp
	${DATACHANNEL_PATH}/src/track.cpp
	${DATACHANNEL_PATH}/src/websocket.cpp
	${DATACHANNEL_PATH}/src/websocketserver.cpp
	${DATACHANNEL_PATH}/src/rtppacketizationconfig.cpp
	${DATACHANNEL_PATH}/src/rtcpsrreporter.cpp
	${DATACHANNEL_PATH}/src/rtppacketizer.cpp
	${DATACHANNEL_PATH}/src/rtpdepacketizer.cpp
	${DATACHANNEL_PATH}/src/h264rtppacketizer.cpp
	${DATACHANNEL_PATH}/src/h264rtpdepacketizer.cpp
	${DATACHANNEL_PATH}/src/nalunit.cpp
	${DATACHANNEL_PATH}/src/h265rtppacketizer.cpp
	${DATACHANNEL_PATH}/src/h265nalunit.cpp
	${DATACHANNEL_PATH}/src/av1rtppacketizer.cpp
	${DATACHANNEL_PATH}/src/rtcpnackresponder.cpp
	${DATACHANNEL_PATH}/src/rtp.cpp
	${DATACHANNEL_PATH}/src/capi.cpp
	${DATACHANNEL_PATH}/src/plihandler.cpp
	${DATACHANNEL_PATH}/src/pacinghandler.cpp
	${DATACHANNEL_PATH}/src/rembhandler.cpp
)

set(LIBDATACHANNEL_HEADERS
	${DATACHANNEL_PATH}/include/rtc/candidate.hpp
	${DATACHANNEL_PATH}/include/rtc/channel.hpp
	${DATACHANNEL_PATH}/include/rtc/configuration.hpp
	${DATACHANNEL_PATH}/include/rtc/datachannel.hpp
	${DATACHANNEL_PATH}/include/rtc/description.hpp
	${DATACHANNEL_PATH}/include/rtc/mediahandler.hpp
	${DATACHANNEL_PATH}/include/rtc/rtcpreceivingsession.hpp
	${DATACHANNEL_PATH}/include/rtc/common.hpp
	${DATACHANNEL_PATH}/include/rtc/global.hpp
	${DATACHANNEL_PATH}/include/rtc/message.hpp
	${DATACHANNEL_PATH}/include/rtc/frameinfo.hpp
	${DATACHANNEL_PATH}/include/rtc/peerconnection.hpp
	${DATACHANNEL_PATH}/include/rtc/reliability.hpp
	${DATACHANNEL_PATH}/include/rtc/rtc.h
	${DATACHANNEL_PATH}/include/rtc/rtc.hpp
	${DATACHANNEL_PATH}/include/rtc/rtp.hpp
	${DATACHANNEL_PATH}/include/rtc/track.hpp
	${DATACHANNEL_PATH}/include/rtc/websocket.hpp
	${DATACHANNEL_PATH}/include/rtc/websocketserver.hpp
	${DATACHANNEL_PATH}/include/rtc/rtppacketizationconfig.hpp
	${DATACHANNEL_PATH}/include/rtc/rtcpsrreporter.hpp
	${DATACHANNEL_PATH}/include/rtc/rtppacketizer.hpp
	${DATACHANNEL_PATH}/include/rtc/rtpdepacketizer.hpp
	${DATACHANNEL_PATH}/include/rtc/h264rtppacketizer.hpp
	${DATACHANNEL_PATH}/include/rtc/h264rtpdepacketizer.hpp
	${DATACHANNEL_PATH}/include/rtc/nalunit.hpp
	${DATACHANNEL_PATH}/include/rtc/h265rtppacketizer.hpp
	${DATACHANNEL_PATH}/include/rtc/h265nalunit.hpp
	${DATACHANNEL_PATH}/include/rtc/av1rtppacketizer.hpp
	${DATACHANNEL_PATH}/include/rtc/rtcpnackresponder.hpp
	${DATACHANNEL_PATH}/include/rtc/utils.hpp
	${DATACHANNEL_PATH}/include/rtc/plihandler.hpp
	${DATACHANNEL_PATH}/include/rtc/pacinghandler.hpp
	${DATACHANNEL_PATH}/include/rtc/rembhandler.hpp
	${DATACHANNEL_PATH}/include/rtc/version.h
)

set(LIBDATACHANNEL_IMPL_SOURCES
	${DATACHANNEL_PATH}/src/impl/certificate.cpp
	${DATACHANNEL_PATH}/src/impl/channel.cpp
	${DATACHANNEL_PATH}/src/impl/datachannel.cpp
	${DATACHANNEL_PATH}/src/impl/dtlssrtptransport.cpp
	${DATACHANNEL_PATH}/src/impl/dtlstransport.cpp
	${DATACHANNEL_PATH}/src/impl/icetransport.cpp
	${DATACHANNEL_PATH}/src/impl/init.cpp
	${DATACHANNEL_PATH}/src/impl/peerconnection.cpp
	${DATACHANNEL_PATH}/src/impl/logcounter.cpp
	${DATACHANNEL_PATH}/src/impl/sctptransport.cpp
	${DATACHANNEL_PATH}/src/impl/threadpool.cpp
	${DATACHANNEL_PATH}/src/impl/tls.cpp
	${DATACHANNEL_PATH}/src/impl/track.cpp
	${DATACHANNEL_PATH}/src/impl/utils.cpp
	${DATACHANNEL_PATH}/src/impl/processor.cpp
	${DATACHANNEL_PATH}/src/impl/sha.cpp
	${DATACHANNEL_PATH}/src/impl/pollinterrupter.cpp
	${DATACHANNEL_PATH}/src/impl/pollservice.cpp
	${DATACHANNEL_PATH}/src/impl/http.cpp
	${DATACHANNEL_PATH}/src/impl/httpproxytransport.cpp
	${DATACHANNEL_PATH}/src/impl/tcpserver.cpp
	${DATACHANNEL_PATH}/src/impl/tcptransport.cpp
	${DATACHANNEL_PATH}/src/impl/tlstransport.cpp
	${DATACHANNEL_PATH}/src/impl/transport.cpp
	${DATACHANNEL_PATH}/src/impl/verifiedtlstransport.cpp
	${DATACHANNEL_PATH}/src/impl/websocket.cpp
	${DATACHANNEL_PATH}/src/impl/websocketserver.cpp
	${DATACHANNEL_PATH}/src/impl/wstransport.cpp
	${DATACHANNEL_PATH}/src/impl/wshandshake.cpp
)

set(LIBDATACHANNEL_IMPL_HEADERS
	${DATACHANNEL_PATH}/src/impl/certificate.hpp
	${DATACHANNEL_PATH}/src/impl/channel.hpp
	${DATACHANNEL_PATH}/src/impl/datachannel.hpp
	${DATACHANNEL_PATH}/src/impl/dtlssrtptransport.hpp
	${DATACHANNEL_PATH}/src/impl/dtlstransport.hpp
	${DATACHANNEL_PATH}/src/impl/icetransport.hpp
	${DATACHANNEL_PATH}/src/impl/init.hpp
	${DATACHANNEL_PATH}/src/impl/internals.hpp
	${DATACHANNEL_PATH}/src/impl/peerconnection.hpp
	${DATACHANNEL_PATH}/src/impl/queue.hpp
	${DATACHANNEL_PATH}/src/impl/logcounter.hpp
	${DATACHANNEL_PATH}/src/impl/sctptransport.hpp
	${DATACHANNEL_PATH}/src/impl/threadpool.hpp
	${DATACHANNEL_PATH}/src/impl/tls.hpp
	${DATACHANNEL_PATH}/src/impl/track.hpp
	${DATACHANNEL_PATH}/src/impl/utils.hpp
	${DATACHANNEL_PATH}/src/impl/processor.hpp
	${DATACHANNEL_PATH}/src/impl/sha.hpp
	${DATACHANNEL_PATH}/src/impl/pollinterrupter.hpp
	${DATACHANNEL_PATH}/src/impl/pollservice.hpp
	${DATACHANNEL_PATH}/src/impl/http.hpp
	${DATACHANNEL_PATH}/src/impl/httpproxytransport.hpp
	${DATACHANNEL_PATH}/src/impl/tcpserver.hpp
	${DATACHANNEL_PATH}/src/impl/tcptransport.hpp
	${DATACHANNEL_PATH}/src/impl/tlstransport.hpp
	${DATACHANNEL_PATH}/src/impl/transport.hpp
	${DATACHANNEL_PATH}/src/impl/verifiedtlstransport.hpp
	${DATACHANNEL_PATH}/src/impl/websocket.hpp
	${DATACHANNEL_PATH}/src/impl/websocketserver.hpp
	${DATACHANNEL_PATH}/src/impl/wstransport.hpp
	${DATACHANNEL_PATH}/src/impl/wshandshake.hpp
)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# PLOG
set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)
add_subdirectory(deps/plog EXCLUDE_FROM_ALL)

if(SCTP_DEBUG)
	add_definitions(-DSCTP_DEBUG)
endif()

# USRSCTP
option(sctp_build_shared_lib OFF)
option(sctp_build_programs OFF)
option(sctp_inet OFF)
option(sctp_inet6 OFF)
option(sctp_werror OFF)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
add_subdirectory(deps/usrsctp EXCLUDE_FROM_ALL)
if (MSYS OR MINGW)
	target_compile_definitions(usrsctp PUBLIC -DSCTP_STDINT_INCLUDE=<stdint.h>)
endif()
add_library(Usrsctp::Usrsctp ALIAS usrsctp)

add_library(datachannel STATIC
	${LIBDATACHANNEL_SOURCES}
	${LIBDATACHANNEL_HEADERS}
	${LIBDATACHANNEL_IMPL_SOURCES}
	${LIBDATACHANNEL_IMPL_HEADERS})
set_target_properties(datachannel PROPERTIES
	VERSION ${PROJECT_VERSION}
	CXX_STANDARD 17)
target_compile_definitions(datachannel PRIVATE RTC_EXPORTS)
target_compile_definitions(datachannel PUBLIC RTC_STATIC)

target_include_directories(datachannel PUBLIC
    $<BUILD_INTERFACE:${DATACHANNEL_PATH}/include>
    $<INSTALL_INTERFACE:include>)
target_include_directories(datachannel PRIVATE ${DATACHANNEL_PATH}/include/rtc)
target_include_directories(datachannel PRIVATE ${DATACHANNEL_PATH}/src)
target_link_libraries(datachannel PRIVATE Threads::Threads)
target_link_libraries(datachannel PRIVATE Usrsctp::Usrsctp plog::plog)

if(WIN32)
	target_link_libraries(datachannel PUBLIC ws2_32) # winsock2
endif()

if (NO_WEBSOCKET)
	target_compile_definitions(datachannel PUBLIC RTC_ENABLE_WEBSOCKET=0)
else()
	target_compile_definitions(datachannel PUBLIC RTC_ENABLE_WEBSOCKET=1)
endif()

# OPENSSL
target_compile_definitions(datachannel PRIVATE USE_GNUTLS=0)
if (BUILD_OPENSSL)
	add_subdirectory(deps/openssl)
endif()
find_package(OpenSSL REQUIRED)
target_link_libraries(datachannel PRIVATE OpenSSL::SSL)

# SRTP2
if(NO_MEDIA)
	target_compile_definitions(datachannel PUBLIC RTC_ENABLE_MEDIA=0)
else()
	target_compile_definitions(datachannel PUBLIC RTC_ENABLE_MEDIA=1)
	add_subdirectory(deps/libsrtp EXCLUDE_FROM_ALL)
	target_compile_definitions(datachannel PRIVATE RTC_SYSTEM_SRTP=0)
	target_link_libraries(datachannel PRIVATE srtp2)
endif()

# JUICE

target_compile_definitions(datachannel PRIVATE USE_NICE=0)
add_subdirectory(deps/libjuice EXCLUDE_FROM_ALL)
target_compile_definitions(datachannel PRIVATE RTC_SYSTEM_JUICE=0)
target_link_libraries(datachannel PRIVATE LibJuice::LibJuice)

if(CAPI_STDCALL)
	target_compile_definitions(datachannel PUBLIC CAPI_STDCALL)
endif()

set_target_properties(datachannel PROPERTIES EXPORT_NAME LibDataChannel)
add_library(LibDataChannel::LibDataChannel ALIAS datachannel)

if(NOT MSVC)
	target_compile_options(datachannel PRIVATE -Wall -Wextra)
endif()

if(WARNINGS_AS_ERRORS)
	if(MSVC)
		target_compile_options(datachannel PRIVATE /WX)
	else()
		target_compile_options(datachannel PRIVATE -Werror)
	endif()
endif()

