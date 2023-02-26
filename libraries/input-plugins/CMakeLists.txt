# Copyright 2013-2018, High Fidelity, Inc.
# Copyright 2023 Overte e.V.
# SPDX-License-Identifier: Apache-2.0

set(TARGET_NAME input-plugins)
setup_hifi_library()
link_hifi_libraries(shared plugins ui-plugins controllers ui)
include_hifi_library_headers(script-engine)

GroupSources("src/input-plugins")
