# Copyright 2013-2020, High Fidelity, Inc.
# Copyright 2021-2026 Overte e.V.
# SPDX-License-Identifier: Apache-2.0

set(TARGET_NAME shared)

include_directories("${QT_DIR}/include/QtCore/${QT_VERSION}/QtCore" "${QT_DIR}/include/QtCore/${QT_VERSION}")

setup_hifi_library(Gui Network)

if (OVERTE_DISABLE_WEBRTC)
    message(WARNING "WebRTC has been disabled for this build.")
endif()

if (WIN32)
    target_link_libraries(${TARGET_NAME} Wbemuuid.lib)
    add_compile_definitions(_USE_MATH_DEFINES)
endif()

if (ANDROID)
    target_link_libraries(${TARGET_NAME} android)
endif()

if (APPLE)
    find_library(FRAMEWORK_IOKIT IOKit)
    find_library(CORE_FOUNDATION CoreFoundation)
    find_library(OpenGL OpenGL)
    target_link_libraries(${TARGET_NAME} ${FRAMEWORK_IOKIT} ${CORE_FOUNDATION} ${OpenGL})
endif()

if (UNIX AND NOT APPLE)
    find_package(Journald)

    if (${JOURNALD_FOUND})
        target_link_libraries(${TARGET_NAME} ${JOURNALD_LIBRARIES})
        target_include_directories(${TARGET_NAME} PRIVATE ${JOURNALD_INCLUDE_DIR})
        target_compile_definitions(${TARGET_NAME} PUBLIC HAS_JOURNALD)
    endif()
endif()

target_zlib()
target_nsight()
target_json()
target_glm()
