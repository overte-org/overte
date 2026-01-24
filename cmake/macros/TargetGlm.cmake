# 
#  Copyright 2015 High Fidelity, Inc.
#  Copyright 2025 Overte e.V.
#  Created by Bradley Austin Davis on 2015/10/10
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
# 
macro(TARGET_GLM)
    if (OVERTE_USE_SYSTEM_LIBS)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(Glm REQUIRED glm)
        target_include_directories(${TARGET_NAME} SYSTEM PUBLIC ${Glm_INCLUDE_DIRS})
        target_link_libraries(${TARGET_NAME} ${Glm_LINK_LIBRARIES})
    else()
        find_package(glm CONFIG REQUIRED)
        target_link_libraries(${TARGET_NAME} glm::glm)
    endif()
    target_compile_definitions(${TARGET_NAME} PUBLIC GLM_FORCE_RADIANS)
    target_compile_definitions(${TARGET_NAME} PUBLIC GLM_ENABLE_EXPERIMENTAL)
    target_compile_definitions(${TARGET_NAME} PUBLIC GLM_FORCE_CTOR_INIT)
endmacro()
