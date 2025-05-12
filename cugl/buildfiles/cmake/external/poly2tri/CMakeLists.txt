cmake_minimum_required(VERSION 3.12)

project(poly2tri LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)

option(P2T_BUILD_TESTS "Build tests" OFF)
option(P2T_BUILD_TESTBED "Build the testbed application" OFF)
set(POLY2_DIR "../../../../external/poly2tri")

file(GLOB SOURCES 
	${POLY2_DIR}/poly2tri/common/*.cc 
	${POLY2_DIR}/poly2tri/sweep/*.cc)
file(GLOB HEADERS 
	${POLY2_DIR}/poly2tri/*.h 
	${POLY2_DIR}/poly2tri/common/*.h 
	${POLY2_DIR}/poly2tri/sweep/*.h)

add_library(poly2tri ${SOURCES} ${HEADERS})
target_include_directories(poly2tri INTERFACE ${POLY2_DIR})

get_target_property(poly2tri_target_type poly2tri TYPE)
if(poly2tri_target_type STREQUAL SHARED_LIBRARY)
  target_compile_definitions(poly2tri PRIVATE P2T_SHARED_EXPORTS)
else()
  target_compile_definitions(poly2tri PUBLIC P2T_STATIC_EXPORTS)
endif()