#
#  Copyright 2017-2020 High Fidelity, Inc.
#  Copyright 2023-2026 Overte e.V.
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
#
macro(TARGET_DRACO)
    if (OVERTE_USE_SYSTEM_LIBS)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(Draco REQUIRED draco)
        target_include_directories(${TARGET_NAME} SYSTEM PUBLIC ${Draco_INCLUDE_DIRS})
        target_link_libraries(${TARGET_NAME} ${Draco_LINK_LIBRARIES})
    else()
        find_package(draco REQUIRED)
        target_link_libraries(${TARGET_NAME} draco::draco)
    endif()
endmacro()
