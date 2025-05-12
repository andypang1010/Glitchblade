cmake_minimum_required(VERSION 3.12)

project(libsrtp2 VERSION 2.5.0 LANGUAGES C)

# Define the relative position of this file in the CUGL project
# This is to make this file easily relocatable (for Android, etc.)
set(CMAKE_CURRENT_DEPTH "${PROJECT_SOURCE_DIR}/../../../..")

if(NOT DEFINED PROJECT_DIR_OFFSET)
	set(PROJECT_DIR_OFFSET "../..")
endif()

if(NOT DEFINED DATACHANNEL_PATH)
	set(DATACHANNEL_PATH "${CMAKE_CURRENT_DEPTH}/${PROJECT_DIR_OFFSET}/external/libdatachannel")
endif()

set(SRTP_PATH "${DATACHANNEL_PATH}/deps/libsrtp")

if(NOT MSVC)
    set(CMAKE_C_FLAGS "-fPIC")
    set(CMAKE_CXX_FLAGS "-fPIC")
endif()

set(PACKAGE_VERSION ${CMAKE_PROJECT_VERSION})
set(PACKAGE_STRING "${CMAKE_PROJECT_NAME} ${CMAKE_PROJECT_VERSION}")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/modules)

include(TestBigEndian)
include(CheckIncludeFile)
include(CheckFunctionExists)
include(CheckTypeSize)
include(CheckCSourceCompiles)

test_big_endian(WORDS_BIGENDIAN)

if (NOT APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
  set (HAVE_X86 TRUE)
else ()
  set (HAVE_X86 FALSE)
endif ()

check_include_file(arpa/inet.h HAVE_ARPA_INET_H)
check_include_file(byteswap.h HAVE_BYTESWAP_H)
check_include_file(inttypes.h HAVE_INTTYPES_H)
check_include_file(machine/types.h HAVE_MACHINE_TYPES_H)
check_include_file(netinet/in.h HAVE_NETINET_IN_H)
check_include_file(stdint.h HAVE_STDINT_H)
check_include_file(stdlib.h HAVE_STDLIB_H)
check_include_file(sys/int_types.h HAVE_SYS_INT_TYPES_H)
check_include_file(sys/socket.h HAVE_SYS_SOCKET_H)
check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(unistd.h HAVE_UNISTD_H)
check_include_file(windows.h HAVE_WINDOWS_H)
check_include_file(winsock2.h HAVE_WINSOCK2_H)

check_function_exists(sigaction HAVE_SIGACTION)
check_function_exists(inet_aton HAVE_INET_ATON)
check_function_exists(inet_pton HAVE_INET_PTON)
check_function_exists(usleep HAVE_USLEEP)

check_type_size(uint8_t UINT8_T)
check_type_size(uint16_t UINT16_T)
check_type_size(uint32_t UINT32_T)
check_type_size(uint64_t UINT64_T)
check_type_size(int32_t INT32_T)
check_type_size("unsigned long" SIZEOF_UNSIGNED_LONG)
check_type_size("unsigned long long" SIZEOF_UNSIGNED_LONG_LONG)

check_c_source_compiles("inline void func(); void func() { } int main() { func(); return 0; }" HAVE_INLINE)
if(NOT HAVE_INLINE)
  check_c_source_compiles("__inline void func(); void func() { } int main() { func(); return 0; }" HAVE___INLINE)
endif()

set(ENABLE_DEBUG_LOGGING OFF CACHE BOOL "Enable debug logging in all modules")
set(ERR_REPORTING_STDOUT OFF CACHE BOOL "Enable logging to stdout")
set(ERR_REPORTING_FILE "" CACHE FILEPATH "Use file for logging")
set(ENABLE_OPENSSL ON CACHE BOOL "Enable OpenSSL crypto engine")
set(BUILD_WITH_SANITIZERS OFF CACHE BOOL "Adds sanitizer flags to compiler")
set(BUILD_WITH_WARNINGS ON CACHE BOOL "Adds warnings and warning as error flags to compiler")
option(BUILD_SHARED_LIBS "Build shared library" OFF)

if(ENABLE_OPENSSL)
  set(USE_EXTERNAL_CRYPTO TRUE)
else()
  set(USE_EXTERNAL_CRYPTO FALSE)
endif()

if(ENABLE_OPENSSL)
  find_package(OpenSSL REQUIRED)
  set(OPENSSL ${ENABLE_OPENSSL} CACHE BOOL INTERNAL)
  set(GCM ${ENABLE_OPENSSL} CACHE BOOL INTERNAL)
endif()

set(CONFIG_FILE_DIR ${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CONFIG_FILE_DIR})

configure_file(config_in_cmake.h ${CONFIG_FILE_DIR}/config.h)
add_definitions(-DHAVE_CONFIG_H)

if(BUILD_WITH_SANITIZERS AND NOT WIN32)
  if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(SANITIZERS address,undefined)
  else()
    set(SANITIZERS leak,address,undefined)
  endif()
  message(STATUS "Using sanitizers: ${SANITIZERS}")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-omit-frame-pointer -fsanitize=${SANITIZERS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=${SANITIZERS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=${SANITIZERS}")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=${SANITIZERS}")
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=${SANITIZERS}")
endif()


if(BUILD_WITH_WARNINGS)
  if(CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID MATCHES "GNU")
    set(WARNINGS -Wall -pedantic -Wextra -Werror)
  elseif(MSVC)
    set(WARNINGS /W4 /WX)
  endif()
  message(STATUS "Using warnings: ${WARNINGS}")
  add_compile_options(${WARNINGS})
endif()

set(SOURCES_C
	${SRTP_PATH}/srtp/srtp.c
)

set(CIPHERS_SOURCES_C
	${SRTP_PATH}/crypto/cipher/cipher.c
	${SRTP_PATH}/crypto/cipher/cipher_test_cases.c
	${SRTP_PATH}/crypto/cipher/cipher_test_cases.h
	${SRTP_PATH}/crypto/cipher/null_cipher.c
)

if(ENABLE_OPENSSL)
  list(APPEND CIPHERS_SOURCES_C
	${SRTP_PATH}/crypto/cipher/aes_icm_ossl.c
	${SRTP_PATH}/crypto/cipher/aes_gcm_ossl.c
  )
elseif(ENABLE_MBEDTLS)
  list(APPEND CIPHERS_SOURCES_C
	${SRTP_PATH}/crypto/cipher/aes_icm_mbedtls.c
	${SRTP_PATH}/crypto/cipher/aes_gcm_mbedtls.c
  )
elseif(ENABLE_NSS)
  list(APPEND CIPHERS_SOURCES_C
	${SRTP_PATH}/crypto/cipher/aes_icm_nss.c
	${SRTP_PATH}/crypto/cipher/aes_gcm_nss.c
  )
else()
  list(APPEND CIPHERS_SOURCES_C
	${SRTP_PATH}/crypto/cipher/aes.c
	${SRTP_PATH}/crypto/cipher/aes_icm.c
  )
endif()

set(HASHES_SOURCES_C
	${SRTP_PATH}/crypto/hash/auth.c
	${SRTP_PATH}/crypto/hash/auth_test_cases.c
	${SRTP_PATH}/crypto/hash/auth_test_cases.h
	${SRTP_PATH}/crypto/hash/null_auth.c
)

if(ENABLE_OPENSSL)
  list(APPEND HASHES_SOURCES_C
	${SRTP_PATH}/crypto/hash/hmac_ossl.c
  )
elseif(ENABLE_MBEDTLS)
  list(APPEND HASHES_SOURCES_C
	${SRTP_PATH}/crypto/hash/hmac_mbedtls.c
  )
elseif(ENABLE_NSS)
  list(APPEND HASHES_SOURCES_C
	${SRTP_PATH}/crypto/hash/hmac_nss.c
  )
else()
  list(APPEND HASHES_SOURCES_C
	${SRTP_PATH}/crypto/hash/hmac.c
	${SRTP_PATH}/crypto/hash/sha1.c
  )
endif()

set(KERNEL_SOURCES_C
	${SRTP_PATH}/crypto/kernel/alloc.c
	${SRTP_PATH}/crypto/kernel/crypto_kernel.c
	${SRTP_PATH}/crypto/kernel/err.c
	${SRTP_PATH}/crypto/kernel/key.c
)

set(MATH_SOURCES_C
	${SRTP_PATH}/crypto/math/datatypes.c
)

set(REPLAY_SOURCES_C
	${SRTP_PATH}/crypto/replay/rdb.c
	${SRTP_PATH}/crypto/replay/rdbx.c
)

set(SOURCES_H
	${SRTP_PATH}/crypto/include/aes.h
	${SRTP_PATH}/crypto/include/aes_icm.h
	${SRTP_PATH}/crypto/include/alloc.h
	${SRTP_PATH}/crypto/include/auth.h
	${SRTP_PATH}/crypto/include/cipher.h
	${SRTP_PATH}/crypto/include/cipher_types.h
	${SRTP_PATH}/crypto/include/crypto_kernel.h
	${SRTP_PATH}/crypto/include/crypto_types.h
	${SRTP_PATH}/crypto/include/datatypes.h
	${SRTP_PATH}/crypto/include/err.h
	${SRTP_PATH}/crypto/include/hmac.h
	${SRTP_PATH}/crypto/include/integers.h
	${SRTP_PATH}/crypto/include/key.h
	${SRTP_PATH}/crypto/include/null_auth.h
	${SRTP_PATH}/crypto/include/null_cipher.h
	${SRTP_PATH}/crypto/include/rdb.h
	${SRTP_PATH}/crypto/include/rdbx.h
	${SRTP_PATH}/crypto/include/sha1.h
	${SRTP_PATH}/include/srtp.h
	${SRTP_PATH}/include/srtp_priv.h
	${CONFIG_FILE_DIR}/config.h
)

source_group("src" FILES ${SOURCES_C})
source_group("src\\Ciphers" FILES ${CIPHERS_SOURCES_C})
source_group("src\\Hashes" FILES ${HASHES_SOURCES_C})
source_group("src\\Kernel" FILES ${KERNEL_SOURCES_C})
source_group("src\\Math" FILES ${MATH_SOURCES_C})
source_group("src\\Replay" FILES ${REPLAY_SOURCES_C})
source_group("include" FILES ${SOURCES_H})

add_library(srtp2
  ${SOURCES_C}
  ${CIPHERS_SOURCES_C}
  ${HASHES_SOURCES_C}
  ${KERNEL_SOURCES_C}
  ${MATH_SOURCES_C}
  ${REPLAY_SOURCES_C}
  ${SOURCES_H}
)
add_library(libSRTP::srtp2 ALIAS srtp2)

set_target_properties(srtp2 PROPERTIES VERSION ${CMAKE_PROJECT_VERSION})

target_include_directories(srtp2 PUBLIC
  $<BUILD_INTERFACE:${SRTP_PATH}/crypto/include>
  $<BUILD_INTERFACE:${SRTP_PATH}/include>
  $<INSTALL_INTERFACE:include>
)
if(ENABLE_OPENSSL)
  target_include_directories(srtp2 PRIVATE ${OPENSSL_INCLUDE_DIR})
  # target_link_libraries(srtp2 OpenSSL::Crypto)	# We handle upstream
endif()

if(WIN32)
  target_link_libraries(srtp2 ws2_32)
  target_compile_definitions(srtp2 PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()