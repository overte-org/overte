# Copyright 2015-2018, High Fidelity, Inc.
# Copyright 2021-2023 Overte e.V.
# SPDX-License-Identifier: Apache-2.0

set(TARGET_NAME gpu-textures-tests)
# This is not a testcase -- just set it up as a regular hifi project
setup_hifi_project(Quick Gui)
setup_memory_debugger()
setup_thread_debugger()
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Tests/manual-tests/")
link_hifi_libraries(
    shared shaders task networking gl 
    ktx gpu octree
    ${PLATFORM_GL_BACKEND}
)

set(EXTRA_DEPLOY_OPTIONS "--qmldir  \"${PROJECT_SOURCE_DIR}/qml\"")
package_libraries_for_deployment()

target_nsight()
