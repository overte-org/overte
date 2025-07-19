#
#  Created by Brad Davis on 2018/11/10
#  Copyright 2018 High Fidelity, Inc.
#  Copyright 2024-2025 Overte e.V.
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
#

set(TARGET_NAME gpu-vk)
setup_hifi_library()
link_hifi_libraries(shared shaders vk gl gpu)
GroupSources("src")

target_vulkan()