# 
#  Created by Bradley Austin Davis on 2017/11/28
#  Copyright 2013-2017 High Fidelity, Inc.
#  Copyright 2026 Overte e.V.
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
# 
macro(TARGET_POLYVOX)
    if(OVERTE_USE_SYSTEM_LIBS)
        find_package(PolyVox REQUIRED)
        target_include_directories(${TARGET_NAME} SYSTEM PRIVATE ${PolyVox_INCLUDE_DIRS})
        target_link_libraries(${TARGET_NAME} ${PolyVox_LIBRARIES})
    else()
        find_package(polyvox REQUIRED)
        target_link_libraries(${TARGET_NAME} polyvox::polyvox)
    endif()
endmacro()
