#
#  Copyright 2018 High Fidelity, Inc.
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
        if (ANDROID)
            set(INSTALL_DIR ${HIFI_ANDROID_PRECOMPILED}/etc2comp)
            set(ETC2COMP_INCLUDE_DIRS "${INSTALL_DIR}/include" "${INSTALL_DIR}/include/Etc" "${INSTALL_DIR}/include/EtcCodec")
            set(ETC2COMP_LIBRARY_DEBUG ${INSTALL_DIR}/lib/libEtcLib.a)
            set(ETC2COMP_LIBRARY_RELEASE ${INSTALL_DIR}/lib/libEtcLib.a)
            target_include_directories(${TARGET_NAME} PRIVATE ${ETC2COMP_INCLUDE_DIRS})
        else()
            find_package(etc2comp REQUIRED)
        endif()

        target_link_libraries(${TARGET_NAME} etc2comp::etc2comp)
    endif()
endmacro()

