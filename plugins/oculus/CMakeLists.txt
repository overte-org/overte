#
#  Created by Bradley Austin Davis on 2015/10/25
#  Copyright 2015 High Fidelity, Inc.
#  Copyright 2023 Overte e.V.
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http:#www.apache.org/licenses/LICENSE-2.0.html
#  SPDX-License-Identifier: Apache-2.0
#

if (WIN32 AND (NOT USE_GLES))

  # if we were passed an Oculus App ID for entitlement checks, send that along
  if (DEFINED ENV{OCULUS_APP_ID})
    add_definitions(-DOCULUS_APP_ID="$ENV{OCULUS_APP_ID}")
  endif ()

  set(TARGET_NAME oculus)
  setup_hifi_plugin(Multimedia)
  link_hifi_libraries(
    shared task gl shaders gpu ${PLATFORM_GL_BACKEND} controllers ui qml
    plugins ui-plugins display-plugins input-plugins
    audio-client networking render-utils graphics
    ${PLATFORM_GL_BACKEND}
  )
  include_hifi_library_headers(octree)
  include_hifi_library_headers(script-engine)

  find_package(OVR REQUIRED)
  find_package(OVRPlatform REQUIRED)

  target_link_libraries(${TARGET_NAME} OVR::SDK OVR::Platform)
  target_link_libraries(${TARGET_NAME} Winmm.lib)

endif()
