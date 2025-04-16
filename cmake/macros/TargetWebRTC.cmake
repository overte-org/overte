#
#  Copyright 2019 High Fidelity, Inc.
#  Copyright 2025 Overte e.V.
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
#
macro(TARGET_WEBRTC)
    find_package(webrtc-audio-processing REQUIRED)
    target_link_libraries(${TARGET_NAME} webrtc-audio-processing::webrtc-audio-processing)
endmacro()
