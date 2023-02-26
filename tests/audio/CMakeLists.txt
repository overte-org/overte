# Copyright 2015, High Fidelity, Inc.
# Copyright 2022-2023 Overte e.V.
# SPDX-License-Identifier: Apache-2.0

# Declare dependencies
macro (SETUP_TESTCASE_DEPENDENCIES)
  # link in the shared libraries
  link_hifi_libraries(shared audio audio-client plugins networking script-engine)

  package_libraries_for_deployment()
endmacro ()

setup_hifi_testcase()
