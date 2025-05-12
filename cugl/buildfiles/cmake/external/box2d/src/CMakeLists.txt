set(BOX2_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../external/box2d/src")
set(BOX2_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../external/box2d/include")

set(BOX2D_SOURCE_FILES
	${BOX2_SRC_DIR}/collision/b2_broad_phase.cpp
	${BOX2_SRC_DIR}/collision/b2_chain_shape.cpp
	${BOX2_SRC_DIR}/collision/b2_circle_shape.cpp
	${BOX2_SRC_DIR}/collision/b2_collide_circle.cpp
	${BOX2_SRC_DIR}/collision/b2_collide_edge.cpp
	${BOX2_SRC_DIR}/collision/b2_collide_polygon.cpp
	${BOX2_SRC_DIR}/collision/b2_collision.cpp
	${BOX2_SRC_DIR}/collision/b2_distance.cpp
	${BOX2_SRC_DIR}/collision/b2_dynamic_tree.cpp
	${BOX2_SRC_DIR}/collision/b2_edge_shape.cpp
	${BOX2_SRC_DIR}/collision/b2_polygon_shape.cpp
	${BOX2_SRC_DIR}/collision/b2_time_of_impact.cpp
	${BOX2_SRC_DIR}/common/b2_block_allocator.cpp
	${BOX2_SRC_DIR}/common/b2_draw.cpp
	${BOX2_SRC_DIR}/common/b2_math.cpp
	${BOX2_SRC_DIR}/common/b2_settings.cpp
	${BOX2_SRC_DIR}/common/b2_stack_allocator.cpp
	${BOX2_SRC_DIR}/common/b2_timer.cpp
	${BOX2_SRC_DIR}/dynamics/b2_body.cpp
	${BOX2_SRC_DIR}/dynamics/b2_chain_circle_contact.cpp
	${BOX2_SRC_DIR}/dynamics/b2_chain_circle_contact.h
	${BOX2_SRC_DIR}/dynamics/b2_chain_polygon_contact.cpp
	${BOX2_SRC_DIR}/dynamics/b2_chain_polygon_contact.h
	${BOX2_SRC_DIR}/dynamics/b2_circle_contact.cpp
	${BOX2_SRC_DIR}/dynamics/b2_circle_contact.h
	${BOX2_SRC_DIR}/dynamics/b2_contact.cpp
	${BOX2_SRC_DIR}/dynamics/b2_contact_manager.cpp
	${BOX2_SRC_DIR}/dynamics/b2_contact_solver.cpp
	${BOX2_SRC_DIR}/dynamics/b2_contact_solver.h
	${BOX2_SRC_DIR}/dynamics/b2_distance_joint.cpp
	${BOX2_SRC_DIR}/dynamics/b2_edge_circle_contact.cpp
	${BOX2_SRC_DIR}/dynamics/b2_edge_circle_contact.h
	${BOX2_SRC_DIR}/dynamics/b2_edge_polygon_contact.cpp
	${BOX2_SRC_DIR}/dynamics/b2_edge_polygon_contact.h
	${BOX2_SRC_DIR}/dynamics/b2_fixture.cpp
	${BOX2_SRC_DIR}/dynamics/b2_friction_joint.cpp
	${BOX2_SRC_DIR}/dynamics/b2_gear_joint.cpp
	${BOX2_SRC_DIR}/dynamics/b2_island.cpp
	${BOX2_SRC_DIR}/dynamics/b2_island.h
	${BOX2_SRC_DIR}/dynamics/b2_joint.cpp
	${BOX2_SRC_DIR}/dynamics/b2_motor_joint.cpp
	${BOX2_SRC_DIR}/dynamics/b2_mouse_joint.cpp
	${BOX2_SRC_DIR}/dynamics/b2_polygon_circle_contact.cpp
	${BOX2_SRC_DIR}/dynamics/b2_polygon_circle_contact.h
	${BOX2_SRC_DIR}/dynamics/b2_polygon_contact.cpp
	${BOX2_SRC_DIR}/dynamics/b2_polygon_contact.h
	${BOX2_SRC_DIR}/dynamics/b2_prismatic_joint.cpp
	${BOX2_SRC_DIR}/dynamics/b2_pulley_joint.cpp
	${BOX2_SRC_DIR}/dynamics/b2_revolute_joint.cpp
	${BOX2_SRC_DIR}/dynamics/b2_weld_joint.cpp
	${BOX2_SRC_DIR}/dynamics/b2_wheel_joint.cpp
	${BOX2_SRC_DIR}/dynamics/b2_world.cpp
	${BOX2_SRC_DIR}/dynamics/b2_world_callbacks.cpp
	${BOX2_SRC_DIR}/rope/b2_rope.cpp)

set(BOX2D_HEADER_FILES
	${BOX2_INC_DIR}/box2d/b2_api.h
	${BOX2_INC_DIR}/box2d/b2_block_allocator.h
	${BOX2_INC_DIR}/box2d/b2_body.h
	${BOX2_INC_DIR}/box2d/b2_broad_phase.h
	${BOX2_INC_DIR}/box2d/b2_chain_shape.h
	${BOX2_INC_DIR}/box2d/b2_circle_shape.h
	${BOX2_INC_DIR}/box2d/b2_collision.h
	${BOX2_INC_DIR}/box2d/b2_common.h
	${BOX2_INC_DIR}/box2d/b2_contact.h
	${BOX2_INC_DIR}/box2d/b2_contact_manager.h
	${BOX2_INC_DIR}/box2d/b2_distance.h
	${BOX2_INC_DIR}/box2d/b2_distance_joint.h
	${BOX2_INC_DIR}/box2d/b2_draw.h
	${BOX2_INC_DIR}/box2d/b2_dynamic_tree.h
	${BOX2_INC_DIR}/box2d/b2_edge_shape.h
	${BOX2_INC_DIR}/box2d/b2_fixture.h
	${BOX2_INC_DIR}/box2d/b2_friction_joint.h
	${BOX2_INC_DIR}/box2d/b2_gear_joint.h
	${BOX2_INC_DIR}/box2d/b2_growable_stack.h
	${BOX2_INC_DIR}/box2d/b2_joint.h
	${BOX2_INC_DIR}/box2d/b2_math.h
	${BOX2_INC_DIR}/box2d/b2_motor_joint.h
	${BOX2_INC_DIR}/box2d/b2_mouse_joint.h
	${BOX2_INC_DIR}/box2d/b2_polygon_shape.h
	${BOX2_INC_DIR}/box2d/b2_prismatic_joint.h
	${BOX2_INC_DIR}/box2d/b2_pulley_joint.h
	${BOX2_INC_DIR}/box2d/b2_revolute_joint.h
	${BOX2_INC_DIR}/box2d/b2_rope.h
	${BOX2_INC_DIR}/box2d/b2_settings.h
	${BOX2_INC_DIR}/box2d/b2_shape.h
	${BOX2_INC_DIR}/box2d/b2_stack_allocator.h
	${BOX2_INC_DIR}/box2d/b2_time_of_impact.h
	${BOX2_INC_DIR}/box2d/b2_timer.h
	${BOX2_INC_DIR}/box2d/b2_time_step.h
	${BOX2_INC_DIR}/box2d/b2_types.h
	${BOX2_INC_DIR}/box2d/b2_weld_joint.h
	${BOX2_INC_DIR}/box2d/b2_wheel_joint.h
	${BOX2_INC_DIR}/box2d/b2_world.h
	${BOX2_INC_DIR}/box2d/b2_world_callbacks.h
	${BOX2_INC_DIR}/box2d/box2d.h)

add_library(box2d ${BOX2D_SOURCE_FILES} ${BOX2D_HEADER_FILES})
target_include_directories(box2d
  PUBLIC
    $<BUILD_INTERFACE:${BOX2_INC_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    ${BOX2_SRC_DIR}
)

set_target_properties(box2d PROPERTIES
	CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

if(MSVC)
  set_target_properties(box2d PROPERTIES
    COMPILE_PDB_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMPILE_PDB_NAME box2d
  )

  install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/box2d.pdb"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    OPTIONAL
  )
endif()

if (BUILD_SHARED_LIBS)
  target_compile_definitions(box2d
    PUBLIC
      B2_SHARED
  )
endif()

source_group(TREE "${BOX2_SRC_DIR}" PREFIX "src" FILES ${BOX2D_SOURCE_FILES})
source_group(TREE "${BOX2_INC_DIR}" PREFIX "include" FILES ${BOX2D_HEADER_FILES})

install(
  TARGETS box2d
  EXPORT box2dConfig
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
  EXPORT box2dConfig
  NAMESPACE box2d::
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/box2d"
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/box2dConfigVersion.cmake"
  COMPATIBILITY SameMajorVersion
)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/box2dConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/box2d"
)
