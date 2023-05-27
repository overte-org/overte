# Copyright 2013-2019, High Fidelity, Inc.
# Copyright 2021-2023 Overte e.V.
# SPDX-License-Identifier: Apache-2.0

set(TARGET_NAME script-engine)
# FIXME Move undo scripting interface to application and remove Widgets
setup_hifi_library(Network Script WebSockets)

target_zlib()
target_v8()
if (NOT ANDROID)
  target_quazip()
endif ()

link_hifi_libraries(networking)
include_hifi_library_headers(octree)
include_hifi_library_headers(shared)
