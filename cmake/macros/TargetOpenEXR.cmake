#
#  Copyright 2015 High Fidelity, Inc.
#  Created by Olivier Prat on 2019/03/26
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
#
macro(TARGET_OPENEXR)
    if (NOT ANDROID)
        find_package(OpenEXR REQUIRED)
        if(OVERTE_USE_SYSTEM_LIBS)
            find_package(PkgConfig REQUIRED)
            pkg_check_modules(Imath REQUIRED Imath)
            message("${Imath_INCLUDE_DIRS}")
            target_include_directories(${TARGET_NAME} SYSTEM PRIVATE ${Imath_INCLUDE_DIRS})
            target_link_libraries(${TARGET_NAME} OpenEXR::OpenEXR)
        else()
            target_link_libraries(${TARGET_NAME} openexr::openexr)
        endif()
    endif()
endmacro()
