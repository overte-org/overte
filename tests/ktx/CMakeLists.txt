# Declare dependencies
macro (SETUP_TESTCASE_DEPENDENCIES)
  # link in the shared libraries
  link_hifi_libraries(shared ktx gpu image)

  target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_BINARY_DIR}/libraries/entities/src")
  include_hifi_library_headers(entities)
  package_libraries_for_deployment()
endmacro ()

setup_hifi_testcase()
