# Copyright 2013-2019, High Fidelity, Inc.
# Copyright 2021-2023 Overte e.V.
# SPDX-License-Identifier: Apache-2.0

set(TARGET_NAME entities)
generate_entity_properties()
setup_hifi_library(Network)
target_include_directories(${TARGET_NAME} PRIVATE "${OPENSSL_INCLUDE_DIR}")
target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_BINARY_DIR}/libraries/entities/src")
target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/libraries/entities/src")
include_hifi_library_headers(hfm)
include_hifi_library_headers(model-serializers)
include_hifi_library_headers(gpu)
include_hifi_library_headers(image)
include_hifi_library_headers(ktx)
include_hifi_library_headers(material-networking)
include_hifi_library_headers(procedural)
link_hifi_libraries(audio shared shaders networking octree avatars graphics model-networking script-engine)

if (WIN32)
  add_compile_definitions(_USE_MATH_DEFINES)
endif()
