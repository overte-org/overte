# Copyright 2023 Overte contributors
# Copyright 2023 Overte e.V.
# SPDX-License-Identifier: Apache-2.0

set(TARGET_NAME JSAPIExample)
setup_hifi_client_server_plugin(scripting)
link_hifi_libraries(shared plugins script-engine)
include_hifi_library_headers(networking)
