cmake_minimum_required(VERSION 3.5)

# extract version out of configure.ac
file(READ "configure.ac" CONFIGURE_AC)
string(REGEX MATCH "AC_INIT\\(\\[libjpeg\\], \\[([0-9.]+)\\]\\)" ver_found "${CONFIGURE_AC}")
if(NOT ver_found)
	message(FATAL_ERROR "Could not extract version from configure.ac")
endif()
set(LIBJPEG_VERSION "${CMAKE_MATCH_1}")

project(jpeg LANGUAGES C VERSION ${LIBJPEG_VERSION})

# Define the relative position of this file in the SDL App project
set(CMAKE_CURRENT_DEPTH "${PROJECT_SOURCE_DIR}/../../..")

if(NOT DEFINED PROJECT_DIR_OFFSET)
	set(PROJECT_DIR_OFFSET "../..")
endif()

if(NOT DEFINED SDL2_IMAGE_OFFSET)
	set(SDL2_IMAGE_OFFSET "/components/SDL_image")
endif()

if(NOT DEFINED JPEG_DIR)
	set(JPEG_DIR "${CMAKE_CURRENT_DEPTH}/${PROJECT_DIR_OFFSET}${SDL2_IMAGE_OFFSET}/external/jpeg")
endif()

# Resume the normal SDL CMake project

include(GNUInstallDirs)

add_library(jpeg
	${JPEG_DIR}/jaricom.c 
	${JPEG_DIR}/jcapimin.c 
	${JPEG_DIR}/jcapistd.c 
	${JPEG_DIR}/jcarith.c 
	${JPEG_DIR}/jccoefct.c 
	${JPEG_DIR}/jccolor.c
	${JPEG_DIR}/jcdctmgr.c 
	${JPEG_DIR}/jchuff.c 
	${JPEG_DIR}/jcinit.c 
	${JPEG_DIR}/jcmainct.c 
	${JPEG_DIR}/jcmarker.c 
	${JPEG_DIR}/jcmaster.c
	${JPEG_DIR}/jcomapi.c 
	${JPEG_DIR}/jcparam.c 
	${JPEG_DIR}/jcprepct.c 
	${JPEG_DIR}/jcsample.c 
	${JPEG_DIR}/jctrans.c 
	${JPEG_DIR}/jdapimin.c
	${JPEG_DIR}/jdapistd.c 
	${JPEG_DIR}/jdarith.c 
	${JPEG_DIR}/jdatadst.c 
	${JPEG_DIR}/jdatasrc.c 
	${JPEG_DIR}/jdcoefct.c 
	${JPEG_DIR}/jdcolor.c
	${JPEG_DIR}/jddctmgr.c 
	${JPEG_DIR}/jdhuff.c 
	${JPEG_DIR}/jdinput.c 
	${JPEG_DIR}/jdmainct.c 
	${JPEG_DIR}/jdmarker.c 
	${JPEG_DIR}/jdmaster.c
	${JPEG_DIR}/jdmerge.c 
	${JPEG_DIR}/jdpostct.c 
	${JPEG_DIR}/jdsample.c 
	${JPEG_DIR}/jdtrans.c 
	${JPEG_DIR}/jerror.c 
	${JPEG_DIR}/jfdctflt.c
	${JPEG_DIR}/jfdctfst.c 
	${JPEG_DIR}/jfdctint.c 
	${JPEG_DIR}/jidctflt.c 
	${JPEG_DIR}/jquant1.c
	${JPEG_DIR}/jquant2.c 
	${JPEG_DIR}/jutils.c 
	${JPEG_DIR}/jmemmgr.c
)
add_library(JPEG::JPEG ALIAS jpeg)

set(jpeg_headers
    ${JPEG_DIR}/jconfig.h
    ${JPEG_DIR}/jerror.h
    ${JPEG_DIR}/jmorecfg.h
    ${JPEG_DIR}/jpeglib.h
)

if (ANDROID)
	target_sources(jpeg PRIVATE ${JPEG_DIR}/jmem-android.c)
else()
	target_sources(jpeg PRIVATE ${JPEG_DIR}/jmemansi.c)
endif()


target_sources(jpeg PRIVATE 
		${JPEG_DIR}/jidctint.c 
		${JPEG_DIR}/jidctfst.c)

# FIXME : include asm for ARM
# target_sources(jpeg PRIVATE 
#	${JPEG_DIR}/jidctint.c 
#	${JPEG_DIR}/jidctfst.S)

target_compile_definitions(jpeg PRIVATE AVOID_TABLES)

target_include_directories(jpeg PUBLIC
	$<BUILD_INTERFACE:${JPEG_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_target_properties(jpeg PROPERTIES
	SOVERSION "${PROJECT_VERSION_MAJOR}"
	VERSION "${PROJECT_VERSION}"
	# create libjpeg-9.dll on Windows
	RUNTIME_OUTPUT_NAME "jpeg-${PROJECT_VERSION_MAJOR}"
	PREFIX lib
)

if(MINGW AND BUILD_SHARED_LIBS)
	set_target_properties(jpeg PROPERTIES
		PREFIX lib
    )
endif()

install(TARGETS jpeg
	ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

install(FILES ${jpeg_headers}
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
