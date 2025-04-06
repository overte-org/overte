#
# Copyright 2024 Lubosz Sarnecki
# Copyright 2024 Overte e.V.
#
# SPDX-License-Identifier: Apache-2.0
#

set(TARGET_NAME openxr)
setup_hifi_plugin(Gui Qml Multimedia)
link_hifi_libraries(shared task gl qml networking controllers ui
    plugins display-plugins ui-plugins input-plugins
    audio-client render-utils graphics shaders gpu render
    material-networking model-networking model-baker hfm
    model-serializers ktx image procedural ${PLATFORM_GL_BACKEND})
include_hifi_library_headers(octree)
include_hifi_library_headers(script-engine)

target_openxr()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    # Silence GCC warnings
    target_compile_options(openxr PRIVATE -Wno-missing-field-initializers)

    # Fix build issue where lerp is already defined on C++20 / GCC
    target_compile_definitions(openxr PRIVATE -DDONT_REDEFINE_LERP)
endif()

set_property(TARGET openxr PROPERTY CXX_STANDARD 20)