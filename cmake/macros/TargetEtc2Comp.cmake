#
#  Copyright 2018 High Fidelity, Inc.
#  Copyright 2026 Overte e.V.
#  Created by Sam Gondelman on 5/2/2018
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
#
macro(TARGET_ETC2COMP)
    if (OVERTE_USE_SYSTEM_LIBS)
        find_path(ETC2COMP_INCLUDE_DIRS "EtcLib/Etc/Etc.h")

        target_include_directories(${TARGET_NAME} SYSTEM PRIVATE ${ETC2COMP_INCLUDE_DIRS} "${ETC2COMP_INCLUDE_DIRS}/EtcLib/Etc" "${ETC2COMP_INCLUDE_DIRS}/EtcLib/EtcCodec")
        target_link_libraries(${TARGET_NAME} libEtcLib.a)
    else()
        find_package(etc2comp REQUIRED)
        target_link_libraries(${TARGET_NAME} etc2comp::etc2comp)
    endif()
endmacro()

