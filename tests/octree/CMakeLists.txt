# Copyright 2014-2018, High Fidelity, Inc.
# Copyright 2022-2023 Overte e.V.
# SPDX-License-Identifier: Apache-2.0

# Declare dependencies
macro (setup_testcase_dependencies)
  # link in the shared libraries
  link_hifi_libraries(shared test-utils octree gpu graphics networking entities avatars audio animation script-engine physics)
  target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_BINARY_DIR}/libraries/entities/src")
  target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/libraries/entities/src")

  package_libraries_for_deployment()
endmacro ()

setup_hifi_testcase(Network)
