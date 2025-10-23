#
#  Copyright 2015 High Fidelity, Inc.
#  Copyright 2025 Overte e.V.
#  Created by Bradley Austin Davis on 2015/10/10
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
#
macro(TARGET_BULLET)
    if(OVERTE_USE_SYSTEM_LIBS)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(BULLET REQUIRED bullet)
        target_include_directories(${TARGET_NAME} SYSTEM PRIVATE ${BULLET_INCLUDE_DIRS})
        target_link_libraries(${TARGET_NAME} ${BULLET_LINK_LIBRARIES})
    else()
        find_package(Bullet REQUIRED)
        target_link_libraries(${TARGET_NAME} Bullet::Bullet)
    endif()
endmacro()


