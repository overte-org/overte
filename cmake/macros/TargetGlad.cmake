# 
#  Copyright 2015 High Fidelity, Inc.
#  Copyright 2026 Overte e.V.
#  Created by Bradley Austin Davis on 2015/10/10
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
# 
macro(TARGET_GLAD)
    if (ANDROID)
        find_package(OpenGL COMPONENTS EGL GLES2 GLES3 REQUIRED)
        find_package(glad REQUIRED)
        target_link_libraries(${TARGET_NAME} OpenGL::EGL OpenGL::GLES2 OpenGL::GLES3 glad::glad)
    else()
        find_package(OpenGL REQUIRED)
        find_package(glad REQUIRED)
        target_link_libraries(${TARGET_NAME} OpenGL::GL glad::glad)
    endif()
endmacro()
