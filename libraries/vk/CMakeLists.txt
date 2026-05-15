#
#  Created by Bradley Austin Davis on 2018/10/29
#  Copyright 2018 High Fidelity, Inc.
#  Copyright 2024-2026 Overte e.V.
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
#  SPDX-License-Identifier: Apache-2.0
#

set(TARGET_NAME vk)
setup_hifi_library(Gui)
GroupSources(src)
link_hifi_libraries(shared shaders gl vk-mac)
find_package(Qt5 COMPONENTS Core Gui REQUIRED)
target_link_libraries(${TARGET_NAME} Qt5::GuiPrivate Qt5::CorePrivate)

#if(APPLE)
#    # separate library created just to build the Objective-C++ code
#    add_library(vulkan_mac
#            layerForWindow.h
#            layerForWindow.mm
#    )
#
##    target_include_directories(vulkan_mac
##            PUBLIC
##    )
#endif()

#if(APPLE)
#    # when building on macOS, also link the Objective-C++ lib.
#    target_link_libraries(${TARGET_NAME} vk-mac)
#endif()

target_vulkan()
