# Primary CMake file for SDL_atk
# The format is adopted from SDL_image/SDL_ttf
cmake_minimum_required(VERSION 3.16)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/modules")
set(MAJOR_VERSION 2)
set(MINOR_VERSION 1)
set(MICRO_VERSION 0)
set(SDL_REQUIRED_VERSION 2.0.9)

include(PrivateSdlFunctions)
sdl_calculate_derived_version_variables()

set(DYLIB_COMPATIBILITY_VERSION "1.0.0")

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the SDL_atk source code and call cmake from there")
endif()

project(SDL2_atk
    LANGUAGES C
    VERSION "${FULL_VERSION}"
)

message(STATUS "Configuring ${PROJECT_NAME} ${PROJECT_VERSION}")

# Define the relative position of this file in the SDL App project
# This is to make this file easily relocatable (for Android, etc.)
set(CMAKE_CURRENT_DEPTH "${PROJECT_SOURCE_DIR}/..")

if(NOT DEFINED PROJECT_DIR_OFFSET)
	set(PROJECT_DIR_OFFSET "../..")
endif()

if(NOT DEFINED SDL2_ATK_OFFSET)
	set(SDL2_ATK_OFFSET "")
endif()

if(NOT DEFINED SDL2_ATK_DIR)
	set(SDL2_ATK_DIR "${CMAKE_CURRENT_DEPTH}/${PROJECT_DIR_OFFSET}${SDL2_ATK_OFFSET}")
endif()

set(SDL2_ATK_SRC "${SDL2_ATK_DIR}/src/atk")
set(OGG_DIR  "${SDL2_ATK_DIR}/external/ogg")
set(FLAC_DIR "${SDL2_ATK_DIR}/external/flac")
set(VORB_DIR "${SDL2_ATK_DIR}/external/vorbis")
set(KISS_DIR "${SDL2_ATK_DIR}/external/kissfft")

# Resume the normal SDL CMake project


# Set defaults preventing destination file conflicts
set(SDL2ATK_DEBUG_POSTFIX "d"
    CACHE STRING "Name suffix for debug builds")
mark_as_advanced(SDL2ATK_DEBUG_POSTFIX)


include(CMakeDependentOption)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

option(CMAKE_POSITION_INDEPENDENT_CODE "Build static libraries with -fPIC" ON)
option(SDL2ATK_BUILD_SHARED_LIBS "Build the library as a shared library" ON)
option(SDL2ATK_INSTALL "Enable SDL2_atk install target" ON)
option(SDL2ATK_VENDORED "Use vendored third-party libraries" ON)
set(BUILD_SHARED_LIBS ${SDL2ATK_BUILD_SHARED_LIBS})

option(SDL2ATK_TESTS "Build unit tests?" ON)
cmake_dependent_option(SDL2ATK_TESTS_INSTALL "Install unit tests?" OFF "SDL2ATK_TESTS;SDL2ATK_INSTALL" OFF)

option(SDL2ATK_FLAC "Support streaming FLAC files" ON)
option(SDL2ATK_VORB "Support streaming OGG Vorbis files" ON)
option(SDL2ATK_MP3  "Support streaming MP3 files" ON)
option(SDL2ATK_WAV  "Support streaming WAV files" ON)
option(SDL2ATK_WAV_SAVE  "Support saving WAV files" ON)
option(SDL2ATK_FLAC_SAVE "Support saving FLAC files" ON)
option(SDL2ATK_VORB_SAVE "Support saving OGG Vorbis files" ON)

set(LIBOGG_MINIMUM_VERSION "1.3.2")
set(SDL2ATK_OGG_VENDORED OFF)

set(LIBFLAC_MINIMUM_VERSION "1.3.2")
if(SDL2ATK_VENDORED AND SDL2ATK_FLAC)
    set(SDL2ATK_FLAC_VENDORED ON)
    set(SDL2ATK_OGG_VENDORED ON)
else()
    set(SDL2ATK_FLAC_VENDORED OFF)
endif()

set(LIBVORBIS_MINIMUM_VERSION "1.3.4")
if(SDL2ATK_VENDORED AND SDL2ATK_VORB)
    set(SDL2ATK_VORB_VENDORED ON)
    set(SDL2ATK_OGG_VENDORED ON)
else()
    set(SDL2ATK_VORB_VENDORED OFF)
endif()

set(LIBKISSFFT_MINIMUM_VERSION "131.1.0")
if(SDL2ATK_VENDORED)
    set(SDL2ATK_KISSFFT_VENDORED ON)
else()
    set(SDL2ATK_KISSFFT_VENDORED OFF)
endif()


# Save BUILD_SHARED_LIBS variable
set(SDL2ATK_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})

if(SDL2ATK_BUILD_SHARED_LIBS)
    set(sdl2_atk_export_name SDL2_atk)
    set(sdl2_atk_install_name_infix shared)
    set(sdl2_target_name SDL2::SDL2)
else()
    set(sdl2_atk_export_name SDL2_atk-static)
    set(sdl2_atk_install_name_infix static)
    set(sdl2_target_name SDL2::SDL2-static)
endif()


sdl_find_sdl2(${sdl2_target_name} ${SDL_REQUIRED_VERSION})

# Set PROJECT_VERSION of subprojects to "" if it's project call does not set VERSION
cmake_policy(SET CMP0048 NEW)

# Allow cmake_dependent_option to use "Full Condition Syntax"
if(POLICY CMP0127)
    cmake_policy(SET CMP0127 NEW)
endif()

# Compile the interface (we only support static for now)
file(GLOB CODEC_SOURCES
    ${SDL2_ATK_SRC}/audio/*.c
    ${SDL2_ATK_SRC}/codec/*.c
    ${SDL2_ATK_SRC}/dsp/*.c
    ${SDL2_ATK_SRC}/file/*.c
    ${SDL2_ATK_SRC}/math/*.c
    ${SDL2_ATK_SRC}/rand/*.c
)

if (MSVC)
    set(SIZEOF_VOIDP ${CMAKE_SIZEOF_VOID_P})
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(ARCH_64 TRUE)
      set(PROCESSOR_ARCH "x64")
    else()
      set(ARCH_64 FALSE)
      set(PROCESSOR_ARCH "x86")
    endif()
    add_compile_options(/wd4068)
    add_compile_options(/wd4244)
    add_compile_options(/wd4267)
    add_compile_options(/wd4996)
endif()


add_library(SDL2_atk ${CODEC_SOURCES})
set_target_properties(SDL2_atk PROPERTIES PUBLIC_HEADER "SDL_atk.h")
add_library(SDL2_atk::${sdl2_atk_export_name} ALIAS SDL2_atk)
target_include_directories(SDL2_atk PUBLIC
    "$<BUILD_INTERFACE:${SDL2_ATK_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/SDL2>"
)
target_compile_definitions(SDL2_atk PRIVATE
    BUILD_SDL
    SDL_BUILD_MAJOR_VERSION=${MAJOR_VERSION}
    SDL_BUILD_MINOR_VERSION=${MINOR_VERSION}
    SDL_BUILD_MICRO_VERSION=${MICRO_VERSION}
)
target_link_libraries(SDL2_atk PRIVATE ${sdl2_target_name})
if(WIN32 AND SDL2ATK_BUILD_SHARED_LIBS)
    target_sources(SDL2_atk PRIVATE
        ${SDL2_ATK_DIR}/version.rc
    )
endif()
set_target_properties(SDL2_atk PROPERTIES
    DEFINE_SYMBOL DLL_EXPORT
    PUBLIC_HEADER SDL_atk.h
    EXPORT_NAME ${sdl2_atk_export_name}
    C_VISIBILITY_PRESET "hidden"
)
if(NOT ANDROID)
    set_target_properties(SDL2_atk PROPERTIES
        DEBUG_POSTFIX "${SDL2ATK_DEBUG_POSTFIX}"
    )
    if(APPLE)
        # the SOVERSION property corresponds to the compatibility version and VERSION corresponds to the current version
        # https://cmake.org/cmake/help/latest/prop_tgt/SOVERSION.html#mach-o-versions
        set_target_properties(SDL2_atk PROPERTIES
            SOVERSION "${DYLIB_COMPATIBILITY_VERSION}"
            VERSION "${DYLIB_CURRENT_VERSION}"
        )
    else()
        set_target_properties(SDL2_atk PROPERTIES
            SOVERSION "${LT_MAJOR}"
            VERSION "${LT_VERSION}"
        )
    endif()
endif()
if(SDL2ATK_BUILD_SHARED_LIBS AND (APPLE OR (UNIX AND NOT ANDROID)))
    add_custom_command(TARGET SDL2_atk POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E create_symlink "$<TARGET_SONAME_FILE_NAME:SDL2_atk>" "libSDL2_atk$<$<CONFIG:Debug>:${SDL2ATK_DEBUG_POSTFIX}>$<TARGET_FILE_SUFFIX:SDL2_atk>"
        # BYPRODUCTS "libSDL2_atk$<$<CONFIG:Debug>:${SDL2ATK_DEBUG_POSTFIX}>$<TARGET_FILE_SUFFIX:SDL2_atk>" # Needs CMake 3.20
        WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
    )
endif()
if(SDL2ATLK_BUILD_SHARED_LIBS)
    if(WIN32 OR OS2)
        set_target_properties(SDL2_atk PROPERTIES
            PREFIX ""
        )
    endif()
    if(OS2)
        # OS/2 doesn't support a DLL name longer than 8 characters.
        set_target_properties(SDL2_atk PROPERTIES
            OUTPUT_NAME "SDL2atk"
        )
    elseif(UNIX AND NOT ANDROID)
        set_target_properties(SDL2_atk PROPERTIES
            OUTPUT_NAME "SDL2_atk-${LT_RELEASE}"
        )
    endif()
else()
    if(MSVC OR (WATCOM AND (WIN32 OR OS2)))
        set_target_properties(SDL2_atk PROPERTIES
            OUTPUT_NAME "SDL2_atk-static"
        )
    endif()
endif()

# Use `Compatible Interface Properties` to ensure a shared SDL2_atk is built with a shared SDL2
if(SDL2ATK_BUILD_SHARED_LIBS)
    set_property(TARGET SDL2_atk PROPERTY INTERFACE_SDL2_SHARED ${SDL2ATK_BUILD_SHARED_LIBS})
    set_property(TARGET SDL2_atk APPEND PROPERTY COMPATIBLE_INTERFACE_BOOL SDL2_SHARED)
endif()

if(SDL2ATK_BUILD_SHARED_LIBS)
    sdl_target_link_options_no_undefined(SDL2_atk)
endif()

if(SDL2ATK_BUILD_SHARED_LIBS)
    # Make sure static library dependencies are built with -fPIC when building a shared SDL2_atk
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

set(INSTALL_EXTRA_TARGETS)
set(PC_LIBS)
set(PC_REQUIRES)

# Build the support codecs as static libraries
set(BUILD_SHARED_LIBS OFF)

# All the libraries are statically linked in
if(SDL2ATK_FLAC OR SDL2ATK_VORB)
    if(SDL2ATK_OGG_VENDORED)
        message(STATUS "${PROJECT_NAME}: Using vendored libogg")
        sdl_check_project_in_subfolder(external/ogg ogg SDL2ATK_VENDORED)

        add_subdirectory(external/ogg EXCLUDE_FROM_ALL)

        list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:ogg>)
    else()
        message(STATUS "${PROJECT_NAME}: Using system libogg")
        find_package(ogg ${LIBOGG_MINIMUM_VERSION} REQUIRED)
        list(APPEND PC_REQUIRES libogg)
    endif()
	target_link_libraries(SDL2_atk PRIVATE Ogg::ogg)
endif()

if(SDL2ATK_FLAC)
    target_compile_definitions(SDL2_atk PRIVATE LOAD_FLAC)
    if(SDL2ATK_FLAC_VENDORED)
          message(STATUS "${PROJECT_NAME}: Using vendored libFLAC")
          sdl_check_project_in_subfolder(external/flac FLAC SDL2ATK_VENDORED)

          option(BUILD_CXXLIBS "Build libFLAC++" OFF)
          option(BUILD_PROGRAMS "Build and install programs" OFF)
          option(BUILD_EXAMPLES "Build and install examples" OFF)
          option(BUILD_TESTING "Build tests" OFF)
          option(BUILD_DOCS "Build and install doxygen documents" OFF)
          option(WITH_STACK_PROTECTOR "Enable GNU GCC stack smash protection" OFF)
          option(INSTALL_MANPAGES "Install MAN pages" OFF)
          option(INSTALL_PKGCONFIG_MODULES "Install PkgConfig modules" OFF)
          add_subdirectory(external/flac EXCLUDE_FROM_ALL)

          list(APPEND INSTALL_EXTRA_TARGETS FLAC)
          list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:FLAC>)
    else()
        message(STATUS "${PROJECT_NAME}: Using system libFLAC")
        find_package(FLAC ${LIBFLAC_MINIMUM_VERSION} REQUIRED)
        list(APPEND PC_REQUIRES libFLAC)
    endif()
    target_link_libraries(SDL2_atk PRIVATE FLAC)
    # Necessary definition for Windows
    target_compile_definitions(SDL2_atk PRIVATE FLAC__NO_DLL)
endif()

if(SDL2ATK_FLAC_SAVE)
    target_compile_definitions(SDL2_atk PRIVATE SAVE_FLAC)
endif()

if(SDL2ATK_VORB)
	set(CMAKE_POLICY_DEFAULT_CMP0048 OLD)
    target_compile_definitions(SDL2_atk PRIVATE LOAD_VORB)
    if(SDL2ATK_VORB_VENDORED)
        message(STATUS "${PROJECT_NAME}: Using vendored libvorbis")
        sdl_check_project_in_subfolder(external/vorbis vorbis SDL2ATK_VENDORED)

        add_subdirectory(external/vorbis EXCLUDE_FROM_ALL)
        list(APPEND INSTALL_EXTRA_TARGETS vorbis)
        list(APPEND INSTALL_EXTRA_TARGETS vorbisenc)
        list(APPEND INSTALL_EXTRA_TARGETS vorbisfile)
        list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:vorbis>)
        list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:vorbisenc>)
        list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:vorbisfile>)
    else()
        message(STATUS "${PROJECT_NAME}: Using system libvorbis")
        find_package(vorbis ${LIBVORBIS_MINIMUM_VERSION} REQUIRED)
        list(APPEND PC_REQUIRES libvorbis)
        list(APPEND PC_REQUIRES libvorbisenc)
        list(APPEND PC_REQUIRES libvorbisencfile)
    endif()
    target_link_libraries(SDL2_atk PRIVATE vorbis)
    target_link_libraries(SDL2_atk PRIVATE vorbisenc)
    target_link_libraries(SDL2_atk PRIVATE vorbisfile)
endif()

if(SDL2ATK_VORB_SAVE)
    target_compile_definitions(SDL2_atk PRIVATE SAVE_VORB)
endif()

if(SDL2ATK_MP3)
    target_compile_definitions(SDL2_atk PRIVATE LOAD_MP3)
    target_include_directories(SDL2_atk PUBLIC
        "$<BUILD_INTERFACE:${SDL2_ATK_DIR}/external/minimp3>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/SDL2_atk>"
    )
endif()

if(SDL2ATK_WAV)
    target_compile_definitions(SDL2_atk PRIVATE LOAD_WAV)
endif()

if(SDL2ATK_WAV_SAVE)
    target_compile_definitions(SDL2_atk PRIVATE SAVE_WAV)
endif()

if(SDL2ATK_KISSFFT_VENDORED)
    message(STATUS "${PROJECT_NAME}: Using vendored libkissfft")
    sdl_check_project_in_subfolder(external/kissfft kissfft SDL2ATK_VENDORED)

    option(KISSFFT_TEST "Build kissfft tests" OFF)
    option(KISSFFT_TOOLS "Build kissfft command-line tools" OFF)
    option(KISSFFT_STATIC "Build kissfft as static (ON) or shared library (OFF)" ON)
    add_subdirectory(external/kissfft EXCLUDE_FROM_ALL)
    list(APPEND INSTALL_EXTRA_TARGETS kissfft)
    list(APPEND PC_LIBS -l$<TARGET_FILE_BASE_NAME:kissfft>)
else()
    message(STATUS "${PROJECT_NAME}: Using system libkissfft")
    find_package(kissfft ${LIBVORBIS_MINIMUM_VERSION} REQUIRED)
    list(APPEND PC_REQUIRES libkissfft)
endif()
target_link_libraries(SDL2_atk PRIVATE kissfft)


# Restore BUILD_SHARED_LIBS
set(BUILD_SHARED_LIBS ${SDL2ATK_BUILD_SHARED_LIBS})

if(SDL2ATK_INSTALL)
    install(
        TARGETS SDL2_atk
        EXPORT SDL2ATKExports
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT devel
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT library
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT library
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/SDL2" COMPONENT devel
    )

    if(INSTALL_EXTRA_TARGETS)
        install(
            TARGETS ${INSTALL_EXTRA_TARGETS}
            EXPORT SDL2ATKExports
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT devel
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT library
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT library
            PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}" COMPONENT devel
        )
    endif()

    if(APPLE)
        set(PKG_PREFIX "SDL2_atk.framework/Resources")
    elseif(WIN32 AND NOT MINGW)
        set(PKG_PREFIX "cmake")
    else()
        set(PKG_PREFIX "${CMAKE_INSTALL_LIBDIR}/cmake/SDL2_atk")
    endif()

    configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/modules/SDL2_atkConfig.cmake.in SDL2_atkConfig.cmake
        INSTALL_DESTINATION "${PKG_PREFIX}"
    )
    write_basic_package_version_file("${PROJECT_BINARY_DIR}/SDL2_atkConfigVersion.cmake"
        VERSION ${FULL_VERSION}
        COMPATIBILITY AnyNewerVersion
    )
    install(
        FILES
            "${CMAKE_CURRENT_BINARY_DIR}/SDL2_atkConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/SDL2_atkConfigVersion.cmake"
        DESTINATION "${PKG_PREFIX}"
        COMPONENT devel
    )
    install(EXPORT SDL2ATKExports
        FILE SDL2_atk-${sdl2_atk_install_name_infix}-targets.cmake
        NAMESPACE SDL2_atk::
        DESTINATION "${PKG_PREFIX}"
        COMPONENT devel
    )

    if(SDL2ATK_BUILD_SHARED_LIBS)
        # Only create a .pc file for a shared SDL2_atk
        set(prefix "${CMAKE_INSTALL_PREFIX}")
        set(exec_prefix "\${prefix}")
        set(libdir "\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}")
        set(includedir "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
        set(PACKAGE "${PROJECT_NAME}")
        set(VERSION "${FULL_VERSION}")
        set(SDL_VERSION "${SDL_REQUIRED_VERSION}")
        string(JOIN " " PC_REQUIRES ${PC_REQUIRES})
        string(JOIN " " PC_LIBS ${PC_LIBS})
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/modules/SDL2_atk.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/SDL2_atk.pc.intermediate" @ONLY)
        file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/SDL2_atk-$<CONFIG>.pc" INPUT "${CMAKE_CURRENT_BINARY_DIR}/SDL2_atk.pc.intermediate")

        set(PC_DESTDIR)
        if(CMAKE_SYSTEM_NAME MATCHES FreeBSD)
            # FreeBSD uses ${PREFIX}/libdata/pkgconfig
            set(PC_DESTDIR "libdata/pkgconfig")
        else()
            set(PC_DESTDIR "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
        endif()
        # Only install a SDL2_atk.pc file in Release mode
        install(CODE "
            execute_process(COMMAND \"\${CMAKE_COMMAND}\" -E copy_if_different
                \"${CMAKE_CURRENT_BINARY_DIR}/SDL2_atk-$<CONFIG>.pc\"
                \"${CMAKE_CURRENT_BINARY_DIR}/SDL2_atk.pc\")
            file(INSTALL DESTINATION \"\${CMAKE_INSTALL_PREFIX}/${PC_DESTDIR}\"
                TYPE FILE
                FILES \"${CMAKE_CURRENT_BINARY_DIR}/SDL2_atk.pc\")" CONFIG Release COMPONENT devel)
    endif()

    if(SDL2ATK_BUILD_SHARED_LIBS AND (APPLE OR (UNIX AND NOT ANDROID)))
        install(
            FILES
                "${PROJECT_BINARY_DIR}/libSDL2_atk$<$<CONFIG:Debug>:${SDL2ATK_DEBUG_POSTFIX}>$<TARGET_FILE_SUFFIX:SDL2_atk>"
            DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            COMPONENT devel
        )
    endif()

    install(FILES "LICENSE.txt"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/licenses/${PROJECT_NAME}"
        COMPONENT library
    )
endif()

add_library(SDL2::atk INTERFACE IMPORTED GLOBAL)
set_target_properties(SDL2::atk PROPERTIES
    INTERFACE_LINK_LIBRARIES "SDL2_atk"
)

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.17")
    set_target_properties(SDL2::atk PROPERTIES
        DEPRECATION "Use SDL2_atk::SDL2_atk or SDL2_atk::SDL2_atk-static instead"
    )
endif()
