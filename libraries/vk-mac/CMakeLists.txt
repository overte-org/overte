#
#  Created by Julian Groß on 2026-05-15
#  Copyright 2026 Overte e.V.
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
#

set(TARGET_NAME vk-mac)
setup_hifi_library()
find_package(Qt5 COMPONENTS Core Gui REQUIRED)
target_link_libraries(${TARGET_NAME} Qt5::GuiPrivate Qt5::CorePrivate)
