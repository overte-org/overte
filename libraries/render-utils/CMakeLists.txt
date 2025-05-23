# Copyright 2013-2019, High Fidelity, Inc.
# Copyright 2021-2023 Overte e.V.
# SPDX-License-Identifier: Apache-2.0

set(TARGET_NAME render-utils)
generate_render_pipelines()
# pull in the resources.qrc file
qt5_add_resources(QT_RESOURCES_FILE "${CMAKE_CURRENT_SOURCE_DIR}/res/fonts/fonts.qrc")
setup_hifi_library(Gui Network Qml Quick)
target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_BINARY_DIR}/libraries/render-utils/src")
target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/libraries/render-utils/src")
link_hifi_libraries(shared task ktx gpu shaders graphics graphics-scripting material-networking model-networking render animation model-serializers image procedural)
include_hifi_library_headers(audio)
include_hifi_library_headers(networking)
include_hifi_library_headers(octree)
include_hifi_library_headers(script-engine)
include_hifi_library_headers(hfm)

# Artery Font Format
target_aff()

# tell CMake to exclude qrc_fonts.cpp for policy CMP0071
set_property(SOURCE qrc_fonts.cpp PROPERTY SKIP_AUTOMOC ON)

if (NOT ANDROID)
  target_nsight()
endif ()

if (WIN32)
  add_compile_definitions(_USE_MATH_DEFINES)
endif()
