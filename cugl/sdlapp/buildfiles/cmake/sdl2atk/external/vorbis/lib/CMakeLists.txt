set(VORBIS_PUBLIC_HEADERS
    ${VORB_DIR}/include/vorbis/codec.h
    ${VORB_DIR}/include/vorbis/vorbisenc.h
    ${VORB_DIR}/include/vorbis/vorbisfile.h
)

set(VORBIS_HEADERS
    ${VORB_DIR}/lib/envelope.h
    ${VORB_DIR}/lib/lpc.h
    ${VORB_DIR}/lib/lsp.h
    ${VORB_DIR}/lib/codebook.h
    ${VORB_DIR}/lib/misc.h
    ${VORB_DIR}/lib/psy.h
    ${VORB_DIR}/lib/masking.h
    ${VORB_DIR}/lib/os.h
    ${VORB_DIR}/lib/mdct.h
    ${VORB_DIR}/lib/smallft.h
    ${VORB_DIR}/lib/highlevel.h
    ${VORB_DIR}/lib/registry.h
    ${VORB_DIR}/lib/scales.h
    ${VORB_DIR}/lib/window.h
    ${VORB_DIR}/lib/lookup.h
    ${VORB_DIR}/lib/lookup_data.h
    ${VORB_DIR}/lib/codec_internal.h
    ${VORB_DIR}/lib/backends.h
    ${VORB_DIR}/lib/bitrate.h
)

set(VORBIS_SOURCES
    ${VORB_DIR}/lib/mdct.c
    ${VORB_DIR}/lib/smallft.c
    ${VORB_DIR}/lib/block.c
    ${VORB_DIR}/lib/envelope.c
    ${VORB_DIR}/lib/window.c
    ${VORB_DIR}/lib/lsp.c
    ${VORB_DIR}/lib/lpc.c
    ${VORB_DIR}/lib/analysis.c
    ${VORB_DIR}/lib/synthesis.c
    ${VORB_DIR}/lib/psy.c
    ${VORB_DIR}/lib/info.c
    ${VORB_DIR}/lib/floor1.c
    ${VORB_DIR}/lib/floor0.c
    ${VORB_DIR}/lib/res0.c
    ${VORB_DIR}/lib/mapping0.c
    ${VORB_DIR}/lib/registry.c
    ${VORB_DIR}/lib/codebook.c
    ${VORB_DIR}/lib/sharedbook.c
    ${VORB_DIR}/lib/lookup.c
    ${VORB_DIR}/lib/bitrate.c
)

set(VORBISFILE_SOURCES
    ${VORB_DIR}/lib/vorbisfile.c
)

set(VORBISENC_SOURCES
    ${VORB_DIR}/lib/vorbisenc.c
)

if(WIN32)
    list(APPEND VORBIS_SOURCES ${VORB_DIR}/lib/vorbisenc.c)
endif()

if(WIN32)
    list(APPEND VORBIS_SOURCES ${VORB_DIR}/win32/vorbis.def)
    list(APPEND VORBISENC_SOURCES ${VORB_DIR}/win32/vorbisenc.def)
    list(APPEND VORBISFILE_SOURCES ${VORB_DIR}/win32/vorbisfile.def)
endif()

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
endif()

if (NOT BUILD_FRAMEWORK)
    add_library(vorbis ${VORBIS_HEADERS} ${VORBIS_SOURCES})
    add_library(vorbisenc ${VORBISENC_SOURCES})
    add_library(vorbisfile ${VORBISFILE_SOURCES})

    add_library(Vorbis::vorbis ALIAS vorbis)
    add_library(Vorbis::vorbisenc ALIAS vorbisenc)
    add_library(Vorbis::vorbisfile ALIAS vorbisfile)

    get_version_info(VORBIS_VERSION_INFO "V_LIB_CURRENT" "V_LIB_AGE" "V_LIB_REVISION")
    set_target_properties(vorbis PROPERTIES SOVERSION ${VORBIS_VERSION_INFO})
    get_version_info(VORBISENC_VERSION_INFO "VE_LIB_CURRENT" "VE_LIB_AGE" "VE_LIB_REVISION")
    set_target_properties(vorbisenc PROPERTIES SOVERSION ${VORBISENC_VERSION_INFO})
    get_version_info(VORBISFILE_VERSION_INFO "VF_LIB_CURRENT" "VF_LIB_AGE" "VF_LIB_REVISION")
    set_target_properties(vorbisfile PROPERTIES SOVERSION ${VORBISFILE_VERSION_INFO})

    target_include_directories(vorbis
        PUBLIC
            $<BUILD_INTERFACE:${VORB_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
       PRIVATE
            ${VORB_DIR}/lib
    )
    target_include_directories(vorbisenc
        PUBLIC
            $<BUILD_INTERFACE:${VORB_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        PRIVATE
            ${VORB_DIR}/lib
    )
    target_include_directories(vorbisfile
        PUBLIC
            $<BUILD_INTERFACE:${VORB_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    target_link_libraries(vorbis
        PUBLIC Ogg::ogg
        PRIVATE $<$<BOOL:${HAVE_LIBM}>:m>
    )
    target_link_libraries(vorbisenc PUBLIC vorbis)
    target_link_libraries(vorbisfile PUBLIC vorbis)

    install(FILES ${VORBIS_PUBLIC_HEADERS} DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/vorbis)

    install(TARGETS vorbis vorbisenc vorbisfile
        EXPORT VorbisTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
    export(EXPORT VorbisTargets NAMESPACE Vorbis:: FILE VorbisTargets.cmake)

    if(INSTALL_CMAKE_PACKAGE_MODULE)

    set(CMAKE_INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/Vorbis)

        install(EXPORT VorbisTargets
            DESTINATION ${CMAKE_INSTALL_CONFIGDIR}
            NAMESPACE Vorbis::
        )


        include(CMakePackageConfigHelpers)

        configure_package_config_file(${PROJECT_SOURCE_DIR}/modules/VorbisConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/VorbisConfig.cmake
            INSTALL_DESTINATION ${CMAKE_INSTALL_CONFIGDIR}
        )

        write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/VorbisConfigVersion.cmake
            COMPATIBILITY SameMajorVersion
        )

        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/VorbisConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/VorbisConfigVersion.cmake
            DESTINATION ${CMAKE_INSTALL_CONFIGDIR}
        )

    endif()
else()
    add_library(vorbis ${VORBIS_PUBLIC_HEADERS} ${VORBIS_HEADERS} ${VORBIS_SOURCES} ${VORBISFILE_SOURCES} ${VORBISENC_SOURCES})
    set_target_properties(vorbis PROPERTIES
        FRAMEWORK TRUE
        FRAMEWORK_VERSION ${PROJECT_VERSION}
        MACOSX_FRAMEWORK_IDENTIFIER org.xiph.vorbis
        MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${PROJECT_VERSION}
        MACOSX_FRAMEWORK_BUNDLE_VERSION ${PROJECT_VERSION}
        XCODE_ATTRIBUTE_INSTALL_PATH "@rpath"
        PUBLIC_HEADER "${VORBIS_PUBLIC_HEADERS}"
        OUTPUT_NAME Vorbis
    )
    target_link_libraries(vorbis ${OGG_LIBRARIES})
endif()
