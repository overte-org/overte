#
#  Copyright 2015 High Fidelity, Inc.
#  Created by Bradley Austin Davis on 2015/10/10
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
#
macro(TARGET_NVTT)
    if (ANDROID)
        set(NVTT_INSTALL_DIR ${HIFI_ANDROID_PRECOMPILED}/nvtt)
        set(NVTT_LIB_DIR "${NVTT_INSTALL_DIR}/lib")
        set(NVTT_INCLUDE_DIRS "${NVTT_INSTALL_DIR}/include" CACHE STRING INTERNAL)
        list(APPEND NVTT_LIBS "${NVTT_LIB_DIR}/libnvcore.so")
        list(APPEND NVTT_LIBS "${NVTT_LIB_DIR}/libnvmath.so")
        list(APPEND NVTT_LIBS "${NVTT_LIB_DIR}/libnvimage.so")
        list(APPEND NVTT_LIBS "${NVTT_LIB_DIR}/libnvtt.so")
        set(NVTT_LIBRARIES ${NVTT_LIBS} CACHE STRING INTERNAL)
        target_include_directories(${TARGET_NAME} PRIVATE ${NVTT_INCLUDE_DIRS})
    else()
        find_package(nvidia-texture-tools REQUIRED)
    endif()

    target_link_libraries(${TARGET_NAME} nvidia-texture-tools::nvidia-texture-tools)

    if ((NOT WIN32) AND (NOT ANDROID) AND (NOT APPLE))
        find_package(OpenMP)
        target_link_libraries(${TARGET_NAME} OpenMP::OpenMP_C OpenMP::OpenMP_CXX)
    endif()

endmacro()
