# Copyright 2023 Overte e.V.
# SPDX-License-Identifier: Apache-2.0

# Declare dependencies
macro (setup_testcase_dependencies)
  # V8TODO: replace most link_hifi_libraries with include_hifi_library_headers
  # link in the shared libraries
  link_hifi_libraries(shared test-utils script-engine networking octree avatars entities model-networking material-networking model-serializers graphics gpu ktx shaders hfm image procedural)

  package_libraries_for_deployment()
endmacro ()

setup_hifi_testcase(Network)
