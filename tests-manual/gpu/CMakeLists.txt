# Copyright 2015-2019, High Fidelity, Inc.
# Copyright 2021-2023 Overte e.V.
# SPDX-License-Identifier: Apache-2.0

set(TARGET_NAME gpu-test)
# This is not a testcase -- just set it up as a regular hifi project
setup_hifi_project(Quick Gui)
setup_memory_debugger()
setup_thread_debugger()
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Tests/manual-tests/")
link_hifi_libraries(
        shared task networking gl
        ktx shaders gpu procedural octree image
        material-networking ktx shaders model-networking
        graphics model-networking model-serializers hfm animation material-networking
        script-engine render render-utils
        ${PLATFORM_GL_BACKEND}
)

package_libraries_for_deployment()

target_nsight()
