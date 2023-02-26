# Copyright 2014-2019, High Fidelity, Inc.
# Copyright 2021-2023 Overte e.V.
# SPDX-License-Identifier: Apache-2.0

set(TARGET_NAME animation)
setup_hifi_library(Network)
link_hifi_libraries(shared graphics model-serializers script-engine)
include_hifi_library_headers(networking)
include_hifi_library_headers(gpu)
include_hifi_library_headers(hfm)
include_hifi_library_headers(image)

target_nsight()

if (WIN32)
  add_compile_definitions(_USE_MATH_DEFINES)
endif()
