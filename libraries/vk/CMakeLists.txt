#
#  Created by Bradley Austin Davis on 2018/10/29
#  Copyright 2018 High Fidelity, Inc.
#  Copyright 2024 Overte e.V.
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
#  SPDX-License-Identifier: Apache-2.0
#

set(TARGET_NAME vk)
setup_hifi_library(Gui)
GroupSources(src)
link_hifi_libraries(shared shaders gl)
find_package(Qt5Gui)
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

target_vulkan()
