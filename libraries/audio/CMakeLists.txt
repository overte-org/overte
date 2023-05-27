# Copyright 2013-2016, High Fidelity, Inc.
# Copyright 2022-2023 Overte e.V.
# SPDX-License-Identifier: Apache-2.0

set(TARGET_NAME audio)
setup_hifi_library(Network)

if (ANDROID)
  add_definitions("-D__STDC_CONSTANT_MACROS")
endif ()

link_hifi_libraries(networking shared plugins script-engine)
