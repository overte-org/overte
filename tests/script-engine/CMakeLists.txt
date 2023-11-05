# Copyright 2023 Overte e.V.
# SPDX-License-Identifier: Apache-2.0

# Declare dependencies
macro (setup_testcase_dependencies)
  # V8TODO: replace most link_hifi_libraries with include_hifi_library_headers
  # link in the shared libraries
  link_hifi_libraries(shared test-utils script-engine networking octree avatars entities model-networking material-networking model-serializers graphics gpu ktx shaders hfm image procedural)

  package_libraries_for_deployment()

  # The test system is a bit unusual in how it works, and generates targets on its own.
  # This macro will be called for each of them, so we want to add stuff only to the
  # right targets.
  if("${TARGET_NAME}" STREQUAL "script-engine-ScriptEngineNetworkedTests")
    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/src/tests" DESTINATION  "${CMAKE_CURRENT_BINARY_DIR}/")
#    add_custom_command(TARGET "${TARGET_NAME}" POST_BUILD
#      COMMAND ${CMAKE_COMMAND} -E copy
#        "${CMAKE_CURRENT_SOURCE_DIR}/src/tests"
#        "${CMAKE_CURRENT_BINARY_DIR}/tests"
#    )
  endif()
endmacro ()

setup_hifi_testcase(Network)
