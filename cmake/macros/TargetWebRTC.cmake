#
#  Copyright 2019 High Fidelity, Inc.
#  Copyright 2025 Overte e.V.
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
#
macro(TARGET_WEBRTC)
    if (OVERTE_USE_SYSTEM_LIBS)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(WebRTC REQUIRED webrtc-audio-processing-2)
        target_include_directories(${TARGET_NAME} SYSTEM PUBLIC ${WebRTC_INCLUDE_DIRS})
        target_link_libraries(${TARGET_NAME} ${WebRTC_LINK_LIBRARIES})
    else()
        find_package(webrtc-audio-processing REQUIRED)
        target_link_libraries(${TARGET_NAME} webrtc-audio-processing::webrtc-audio-processing)
    endif()
endmacro()
