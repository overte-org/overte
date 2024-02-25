# Copyright 2015-2018, High Fidelity, Inc.
# Copyright 2021-2023 Overte e.V.
# SPDX-License-Identifier: Apache-2.0

set(TARGET_NAME "entities-test")

# This is not a testcase -- just set it up as a regular hifi project
setup_hifi_project(Network)
setup_memory_debugger()
setup_thread_debugger()
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Tests/manual-tests/")

# link in the shared libraries
link_hifi_libraries(
        entities hfm avatars script-engine image
        procedural material-networking ktx shaders
        model-networking shared octree gpu graphics
        model-serializers networking animation audio gl
)

if (WIN32)
    add_dependency_external_projects(wasapi)
endif ()

package_libraries_for_deployment()
