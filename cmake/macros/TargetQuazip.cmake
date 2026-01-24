# 
#  Copyright 2015 High Fidelity, Inc.
#  Created by Leonardo Murillo on 2015/11/20
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
# 
macro(TARGET_QUAZIP)
    if(OVERTE_USE_SYSTEM_LIBS)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(QuaZip REQUIRED quazip1-qt5)
        target_include_directories(${TARGET_NAME} SYSTEM PRIVATE ${QuaZip_INCLUDE_DIRS})
        target_link_libraries(${TARGET_NAME} ${QuaZip_LINK_LIBRARIES})
    else()
        find_package(QuaZip-Qt5 REQUIRED)
        target_link_libraries(${TARGET_NAME} QuaZip::QuaZip)
    endif()
endmacro()
