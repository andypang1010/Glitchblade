# 3.1 is OK for most parts. However:
# 3.3 is needed in src/libFLAC
# 3.5 is needed in src/libFLAC/ia32
# 3.9 is needed in 'doc' because of doxygen_add_docs()
cmake_minimum_required(VERSION 3.6)

if(NOT (CMAKE_BUILD_TYPE OR CMAKE_CONFIGURATION_TYPES OR DEFINED ENV{CFLAGS} OR DEFINED ENV{CXXFLAGS}))
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo")
endif()

project(FLAC VERSION 1.4.3) # HOMEPAGE_URL "https://www.xiph.org/flac/")
if (POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif(POLICY CMP0077)

# Define the relative position of this file in the SDL App project
set(CMAKE_CURRENT_DEPTH "${PROJECT_SOURCE_DIR}/../../..")

if(NOT DEFINED PROJECT_DIR_OFFSET)
	set(PROJECT_DIR_OFFSET "../..")
endif()

if(NOT DEFINED SDL2_ATK_OFFSET)
	set(SDL2_ATK_OFFSET "")
endif()

if(NOT DEFINED FLAC_DIR)
	set(FLAC_DIR "${CMAKE_CURRENT_DEPTH}/${PROJECT_DIR_OFFSET}${SDL2_ATK_OFFSET}/external/flac")
endif()

# Resume the normal SDL CMake project

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/modules")

option(BUILD_CXXLIBS "Build libFLAC++" ON)
option(BUILD_PROGRAMS "Build and install programs" ON)
option(BUILD_EXAMPLES "Build and install examples" ON)
option(BUILD_TESTING "Build tests" ON)
option(BUILD_DOCS "Build and install doxygen documents" ON)
option(WITH_FORTIFY_SOURCE "Enable protection against buffer overflows" OFF) # Optimization is off for now
option(WITH_STACK_PROTECTOR "Enable GNU GCC stack smash protection" ON)
option(INSTALL_MANPAGES "Install MAN pages" ON)
option(INSTALL_PKGCONFIG_MODULES "Install PkgConfig modules" ON)
option(INSTALL_CMAKE_CONFIG_MODULE "Install CMake package-config module" ON)
option(WITH_OGG "ogg support (default: test for libogg)" ON)
option(BUILD_SHARED_LIBS "Build shared instead of static libraries" OFF)
option(ENABLE_MULTITHREADING "Enable multithreading if pthreads is available" ON)

set(VERSION ${PROJECT_VERSION})

if(NOT UNIX)
    # This is to make sure testing works when building with a DLL
    set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/objs)
    set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/objs)
endif()

if(WITH_OGG)
    if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/ogg")
        add_subdirectory("ogg")
        set(OGG_FOUND 1 CACHE INTERNAL "ogg has been added as subdirectory")
        set_target_properties(ogg PROPERTIES FOLDER Libraries)
        if(BUILD_TESTING)
            set_target_properties(test_bitwise test_framing PROPERTIES FOLDER Tests)
        endif()
    else()
        if(NOT TARGET Ogg::ogg)
            find_package(Ogg REQUIRED)
        else()
            set(OGG_FOUND 1 CACHE INTERNAL "ogg has already been built")
        endif()
        set(OGG_PACKAGE "ogg")
    endif()
endif()

if(NOT WIN32)
    find_package(Iconv)
    set(HAVE_ICONV ${Iconv_FOUND})
endif()

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "-Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -Waggregate-return -Wcast-align -Wnested-externs -Wshadow -Wundef -Wmissing-declarations -Winline ${CMAKE_C_FLAGS}")
    set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG ${CMAKE_C_FLAGS_RELEASE}")
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wcast-align -Wshadow -Wwrite-strings -Wctor-dtor-privacy -Wnon-virtual-dtor -Wreorder -Wsign-promo -Wundef ${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG ${CMAKE_CXX_FLAGS_RELEASE}")
endif()
if(MSVC)
    set(CMAKE_C_FLAGS_RELEASE "/O2 /Ob2 /Oi /Ot /Oy /DNDEBUG ${CMAKE_C_FLAGS_RELEASE}")
    add_compile_options(/wd4334)
    add_compile_options(/wd4244)
endif()

include(CMakePackageConfigHelpers)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckCSourceCompiles)
include(CheckCXXSourceCompiles)
include(CheckLibraryExists)
include(GNUInstallDirs)
include(UseSystemExtensions)
include(TestBigEndian)
enable_testing()

check_include_file("byteswap.h" HAVE_BYTESWAP_H)
check_include_file("inttypes.h" HAVE_INTTYPES_H)
check_include_file("stdint.h" HAVE_STDINT_H)
check_include_file("stdbool.h" HAVE_STDBOOL_H)
check_include_file("arm_neon.h" FLAC__HAS_NEONINTRIN)

if(NOT HAVE_STDINT_H OR NOT HAVE_STDBOOL_H)
    message(SEND_ERROR "Header stdint.h and/or stdbool.h not found")
endif()

if(MSVC)
    check_include_file("intrin.h" FLAC__HAS_X86INTRIN)
else()
    check_include_file("x86intrin.h" FLAC__HAS_X86INTRIN)
endif()

if(ANDROID AND (CMAKE_SYSTEM_PROCESSOR MATCHES "i686" OR CMAKE_SYSTEM_PROCESSOR MATCHES "armv7-a"))
    # fseeko/ftello may link, but it's not usable before Android API 24 on 32-bit Android
    # https://android.googlesource.com/platform/bionic/+/main/docs/32-bit-abi.md
    # Unfortunately, we have to jump through some hoops to find the API Version
    # https://stackoverflow.com/questions/52839663/for-android-does-cmake-system-version-mean-the-minimum-api-level-or-the-target-a
    set(API_LEVEL 24)
    if (ANDROID_PLATFORM MATCHES "android-([^ ]+)")
        set(API_LEVEL CMAKE_MATCH_1)
    endif()
    if(API_LEVEL STRLESS_EQUAL 24)
        message(STATUS "Disabling fseeko/ftello for 32-bit Android before API 24")
        set(HAVE_FSEEKO 0 CACHE INTERNAL "")
    else()
        # TECHNICALLY we should search, but apparently fails on Android
        set(HAVE_FSEEKO 1 CACHE INTERNAL "")
    endif()
else()
    check_function_exists(fseeko HAVE_FSEEKO)
endif()

check_c_source_compiles("int main() { return __builtin_bswap16 (0) ; }" HAVE_BSWAP16)
check_c_source_compiles("int main() { return __builtin_bswap32 (0) ; }" HAVE_BSWAP32)
check_c_source_compiles("
    #include <langinfo.h>
    int main()
    {
        char* cs = nl_langinfo(CODESET);
        return !cs;
    }"
    HAVE_LANGINFO_CODESET)

test_big_endian(CPU_IS_BIG_ENDIAN)

check_c_compiler_flag(-Werror HAVE_WERROR_FLAG)
check_c_compiler_flag(-Wdeclaration-after-statement HAVE_DECL_AFTER_STMT_FLAG)
check_c_compiler_flag(-mstackrealign HAVE_STACKREALIGN_FLAG)
check_cxx_compiler_flag(-Weffc++ HAVE_WEFFCXX_FLAG)

if(MINGW AND (WITH_FORTIFY_SOURCE OR WITH_STACK_PROTECTOR))
  check_library_exists("ssp.a"  __stack_chk_fail "" HAVE_LIBSSP)
  if(NOT HAVE_LIBSSP)
    message(WARNING "Could not find libssp in MinGW, stack protection and/or FORTIFY_SOURCE are unavailable")
  else()
    link_libraries("ssp.a")
    # static libraries don't carry over other static libraries in mingw
    # we need to export it in the pkg-config
    set(FLAC_STATIC_LIBS "-lssp")
  endif()
elseif(NOT MSVC)
  set(HAVE_LIBSSP 1)
endif()

if(WITH_STACK_PROTECTOR)
  if(NOT MSVC AND NOT EMSCRIPTEN)
    check_c_compiler_flag("-fstack-protector-strong" HAVE_STACK_PROTECTOR_FLAG)
  endif()
endif()

if(HAVE_WERROR_FLAG)
    option(ENABLE_WERROR "Enable -Werror in all Makefiles" OFF)
endif()

add_compile_options(
    $<$<BOOL:${MSVC}>:/wd4267>
    $<$<BOOL:${MSVC}>:/wd4996>
    $<$<BOOL:${ENABLE_WERROR}>:-Werror>
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<BOOL:${HAVE_WEFFCXX_FLAG}>>:-Weffc++>
    $<$<AND:$<COMPILE_LANGUAGE:C>,$<BOOL:${HAVE_DECL_AFTER_STMT_FLAG}>>:-Wdeclaration-after-statement>)

if(WITH_FORTIFY_SOURCE AND HAVE_LIBSSP)
  add_definitions(-D_FORTIFY_SOURCE=2)
endif()

if(HAVE_STACK_PROTECTOR_FLAG AND HAVE_LIBSSP)
    add_compile_options($<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:-fstack-protector-strong>)
endif()

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "i686" AND HAVE_STACKREALIGN_FLAG)
    add_compile_options($<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:-mstackrealign>)
endif()

include_directories("${FLAC_DIR}/include")

include_directories("${CMAKE_CURRENT_BINARY_DIR}")
add_definitions(-DHAVE_CONFIG_H)

if(MSVC)
    add_definitions(
        -D_CRT_SECURE_NO_WARNINGS
        -D_USE_MATH_DEFINES)
endif()
if(CMAKE_BUILD_TYPE STREQUAL Debug OR CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)
    add_definitions(-DFLAC__OVERFLOW_DETECT)
endif()

if(ENABLE_MULTITHREADING)
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
    find_package(Threads)
    if(CMAKE_USE_PTHREADS_INIT)
        set(HAVE_PTHREAD 1)
    endif()
endif()

add_subdirectory("src")

# The following folder layout is mostly for MSVC
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set_target_properties(FLAC grabbag getopt replaygain_analysis replaygain_synthesis utf8 PROPERTIES FOLDER Libraries)
if(BUILD_CXXLIBS)
    set_target_properties(FLAC++ PROPERTIES FOLDER Libraries)
endif()
configure_file("${FLAC_DIR}/config.cmake.h.in" config.h)

if(INSTALL_CMAKE_CONFIG_MODULE)
    install(
        EXPORT targets
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
        NAMESPACE FLAC::)
    export(EXPORT targets NAMESPACE FLAC:: FILE FLACTargets.cmake)

    configure_package_config_file(
        ${FLAC_DIR}/flac-config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/flac-config.cmake
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/flac-config-version.cmake COMPATIBILITY AnyNewerVersion)

    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/flac-config.cmake ${CMAKE_CURRENT_BINARY_DIR}/flac-config-version.cmake
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
    )

    install(
        FILES
            "${CMAKE_CURRENT_BINARY_DIR}/flac-config.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/flac-config-version.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
endif()
