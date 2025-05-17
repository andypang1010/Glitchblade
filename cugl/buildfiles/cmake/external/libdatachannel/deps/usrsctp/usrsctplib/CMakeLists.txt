#
# Copyright (C) 2015-2015 Oleg Alexeenkov
# Copyright (C) 2015-2020 Felix Weinrank
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the project nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#

# Define the relative position of this file in the CUGL project
# This is to make this file easily relocatable (for Android, etc.)
set(CMAKE_CURRENT_DEPTH "${PROJECT_SOURCE_DIR}/../../../..")

if(NOT DEFINED PROJECT_DIR_OFFSET)
	set(PROJECT_DIR_OFFSET "../..")
endif()

if(NOT DEFINED DATACHANNEL_PATH)
	set(DATACHANNEL_PATH "${CMAKE_CURRENT_DEPTH}/${PROJECT_DIR_OFFSET}/external/libdatachannel")
endif()

if(NOT DEFINED USRSCTP_PATH)
	set(USRSCTP_PATH "${DATACHANNEL_PATH}/deps/usrsctp")
endif()

set(USRSCTPLIB_PATH "${USRSCTP_PATH}/usrsctplib")

#################################################
# INCLUDE MODULES AND SETTINGS
#################################################

set(VERSION "0.9.5.0")

# Shared library API and ABI versions
# Notice: shared library version must be in X.Y.Z format only
set(SOVERSION_FULL "2.0.0")
set(SOVERSION_SHORT "2")

include(GNUInstallDirs)
set(prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix ${prefix})
set(libdir ${exec_prefix}/${CMAKE_INSTALL_LIBDIR})
set(includedir ${prefix}/${CMAKE_INSTALL_INCLUDEDIR})

set(CMAKE_REQUIRED_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_MACOSX_RPATH 1)

include(CheckCCompilerFlag)


option(SCTP_USE_MBEDTLS_SHA1 "Build with mbedtls sha1 support." OFF)

add_definitions(-D__Userspace__)
add_definitions(-DSCTP_SIMPLE_ALLOCATOR)
add_definitions(-DSCTP_PROCESS_LEVEL_LOCKS)

if(SCTP_USE_MBEDTLS_SHA1)
	add_definitions(-DSCTP_USE_MBEDTLS_SHA1)
	find_package(MbedTLS REQUIRED)
endif()


#################################################
# OS DEPENDENT
#################################################

check_c_compiler_flag(-Wno-address-of-packed-member has_wno_address_of_packed_member)
if (has_wno_address_of_packed_member)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-address-of-packed-member")
endif ()

check_c_compiler_flag(-Wno-deprecated-declarations has_wno_deprecated_declarations)
if (has_wno_deprecated_declarations)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations")
endif ()

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_definitions(-D_GNU_SOURCE)
endif ()

if (CMAKE_SYSTEM_NAME MATCHES "Darwin" OR CMAKE_SYSTEM_NAME MATCHES "iOS")
	add_definitions(-D__APPLE_USE_RFC_2292)
endif ()

#################################################
# MISC
#################################################

if (sctp_build_shared_lib)
	set(BUILD_SHARED_LIBS 1)
endif ()

find_package(Threads)

#################################################
# LIBRARY FILES
#################################################

include_directories(${USRSCTPLIB_PATH})

list(APPEND usrsctp_root_headers
	${USRSCTPLIB_PATH}/user_atomic.h
	${USRSCTPLIB_PATH}/user_environment.h
	${USRSCTPLIB_PATH}/user_inpcb.h
	${USRSCTPLIB_PATH}/user_ip_icmp.h
	${USRSCTPLIB_PATH}/user_ip6_var.h
	${USRSCTPLIB_PATH}/user_malloc.h
	${USRSCTPLIB_PATH}/user_mbuf.h
	${USRSCTPLIB_PATH}/user_queue.h
	${USRSCTPLIB_PATH}/user_recv_thread.h
	${USRSCTPLIB_PATH}/user_route.h
	${USRSCTPLIB_PATH}/user_socketvar.h
	${USRSCTPLIB_PATH}/user_uma.h
	${USRSCTPLIB_PATH}/usrsctp.h
)

list(APPEND usrsctp_netinet_headers
	${USRSCTPLIB_PATH}/netinet/sctp_asconf.h
	${USRSCTPLIB_PATH}/netinet/sctp_auth.h
	${USRSCTPLIB_PATH}/netinet/sctp_bsd_addr.h
	${USRSCTPLIB_PATH}/netinet/sctp_callout.h
	${USRSCTPLIB_PATH}/netinet/sctp_constants.h
	${USRSCTPLIB_PATH}/netinet/sctp_crc32.h
	${USRSCTPLIB_PATH}/netinet/sctp_header.h
	${USRSCTPLIB_PATH}/netinet/sctp_indata.h
	${USRSCTPLIB_PATH}/netinet/sctp_input.h
	${USRSCTPLIB_PATH}/netinet/sctp_lock_userspace.h
	${USRSCTPLIB_PATH}/netinet/sctp_os_userspace.h
	${USRSCTPLIB_PATH}/netinet/sctp_os.h
	${USRSCTPLIB_PATH}/netinet/sctp_output.h
	${USRSCTPLIB_PATH}/netinet/sctp_pcb.h
	${USRSCTPLIB_PATH}/netinet/sctp_peeloff.h
	${USRSCTPLIB_PATH}/netinet/sctp_process_lock.h
	${USRSCTPLIB_PATH}/netinet/sctp_sha1.h
	${USRSCTPLIB_PATH}/netinet/sctp_structs.h
	${USRSCTPLIB_PATH}/netinet/sctp_sysctl.h
	${USRSCTPLIB_PATH}/netinet/sctp_timer.h
	${USRSCTPLIB_PATH}/netinet/sctp_uio.h
	${USRSCTPLIB_PATH}/netinet/sctp_var.h
	${USRSCTPLIB_PATH}/netinet/sctputil.h
	${USRSCTPLIB_PATH}/netinet/sctp.h
)

list(APPEND usrsctp_netinet6_headers
	${USRSCTPLIB_PATH}/netinet6/sctp6_var.h
)

list(APPEND usrsctp_headers
	${usrsctp_root_headers}
	${usrsctp_netinet_headers}
	${usrsctp_netinet6_headers}
)

list(APPEND usrsctp_sources
	${USRSCTPLIB_PATH}/netinet/sctp_asconf.c
	${USRSCTPLIB_PATH}/netinet/sctp_auth.c
	${USRSCTPLIB_PATH}/netinet/sctp_bsd_addr.c
	${USRSCTPLIB_PATH}/netinet/sctp_callout.c
	${USRSCTPLIB_PATH}/netinet/sctp_cc_functions.c
	${USRSCTPLIB_PATH}/netinet/sctp_crc32.c
	${USRSCTPLIB_PATH}/netinet/sctp_indata.c
	${USRSCTPLIB_PATH}/netinet/sctp_input.c
	${USRSCTPLIB_PATH}/netinet/sctp_output.c
	${USRSCTPLIB_PATH}/netinet/sctp_pcb.c
	${USRSCTPLIB_PATH}/netinet/sctp_peeloff.c
	${USRSCTPLIB_PATH}/netinet/sctp_sha1.c
	${USRSCTPLIB_PATH}/netinet/sctp_ss_functions.c
	${USRSCTPLIB_PATH}/netinet/sctp_sysctl.c
	${USRSCTPLIB_PATH}/netinet/sctp_timer.c
	${USRSCTPLIB_PATH}/netinet/sctp_userspace.c
	${USRSCTPLIB_PATH}/netinet/sctp_usrreq.c
	${USRSCTPLIB_PATH}/netinet/sctputil.c
	${USRSCTPLIB_PATH}/netinet6/sctp6_usrreq.c
	${USRSCTPLIB_PATH}/user_environment.c
	${USRSCTPLIB_PATH}/user_mbuf.c
	${USRSCTPLIB_PATH}/user_recv_thread.c
	${USRSCTPLIB_PATH}/user_socket.c
)

add_library(usrsctp ${usrsctp_sources} ${usrsctp_headers})

target_include_directories(usrsctp PUBLIC ${USRSCTPLIB_PATH})

if(SCTP_USE_MBEDTLS_SHA1)
	target_include_directories(usrsctp PRIVATE ${MBEDTLS_INCLUDE_DIRS})
endif()

target_link_libraries(usrsctp ${CMAKE_THREAD_LIBS_INIT})

if (WIN32)
	message(STATUS "link library: ws2_32")
	target_link_libraries(usrsctp ws2_32 iphlpapi)
endif ()

if(SCTP_USE_MBEDTLS_SHA1)
	target_link_libraries(usrsctp PRIVATE ${MBEDTLS_LIBRARIES})
endif()


set_target_properties(usrsctp PROPERTIES IMPORT_SUFFIX "_import.lib")
set_target_properties(usrsctp PROPERTIES SOVERSION ${SOVERSION_SHORT} VERSION ${SOVERSION_FULL})
