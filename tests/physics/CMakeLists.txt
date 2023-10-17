# Copyright 2014-2018, High Fidelity, Inc.
# Copyright 2023 Overte e.V.
# SPDX-License-Identifier: Apache-2.0

# Declare dependencies
macro (SETUP_TESTCASE_DEPENDENCIES)
  target_bullet()
  link_hifi_libraries(shared test-utils physics gpu graphics)
  package_libraries_for_deployment()
endmacro ()

setup_hifi_testcase()
