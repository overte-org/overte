# Copyright 2013-2017, High Fidelity, Inc.
# Copyright 2023 Overte e.V.
# SPDX-License-Identifier: Apache-2.0

set(TARGET_NAME midi)
setup_hifi_library(Network)
link_hifi_libraries(shared networking)
include_hifi_library_headers(script-engine)

if (WIN32)
    target_link_libraries(${TARGET_NAME} winmm.lib)
endif ()
