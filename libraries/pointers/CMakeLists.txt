# Copyright 2013-2017, High Fidelity, Inc.
# Copyright 2023 Overte e.V.
# SPDX-License-Identifier: Apache-2.0

set(TARGET_NAME pointers)
setup_hifi_library()
GroupSources(src)
link_hifi_libraries(shared controllers)
include_hifi_library_headers(script-engine)
