# 
#  Copyright 2015 High Fidelity, Inc.
#  Created by Bradley Austin Davis on 2015/10/10
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
# 
macro(TARGET_GLAD)
    if (ANDROID)
        include(SelectLibraryConfigurations)
        set(INSTALL_DIR ${HIFI_ANDROID_PRECOMPILED}/glad)
        set(GLAD_INCLUDE_DIRS "${INSTALL_DIR}/include")
        set(GLAD_LIBRARY_DEBUG ${INSTALL_DIR}/lib/libglad_d.a)
        set(GLAD_LIBRARY_RELEASE ${INSTALL_DIR}/lib/libglad.a)
        select_library_configurations(GLAD)
        find_library(EGL EGL)
        target_link_libraries(${TARGET_NAME} ${EGL})
    else()
        find_package(OpenGL REQUIRED)
        find_package(glad REQUIRED)
    endif()

    target_link_libraries(${TARGET_NAME} OpenGL::GL glad::glad)
    # target_link_libraries(${TARGET_NAME} ${GLAD_EXTRA_LIBRARIES})       
endmacro()
