# Declare dependencies
macro (SETUP_TESTCASE_DEPENDENCIES)
  # link in the shared libraries
  link_hifi_libraries(shared ktx gpu image)

  target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_BINARY_DIR}/libraries/entities/src")
  include_hifi_library_headers(entities)
  package_libraries_for_deployment()

  if("${TARGET_NAME}" STREQUAL "ktx-KtxTests")
    # The test wants the cube_texture.png for testing.
    file(INSTALL "${CMAKE_SOURCE_DIR}/scripts/developer/tests/cube_texture.png" DESTINATION "${CMAKE_BINARY_DIR}/tests/ktx")

  endif()
endmacro ()

setup_hifi_testcase()
