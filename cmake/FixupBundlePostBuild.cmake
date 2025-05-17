#
#  FixupBundlePostBuild.cmake
#  cmake
#
#  Copyright 2015 High Fidelity, Inc.
#  Created by Stephen Birarda on February 13, 2014
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
#

include(BundleUtilities)

if (APPLE)
  set(PLUGIN_EXTENSION "dylib")
elseif (WIN32)
  set(PLUGIN_EXTENSION "dll")
else()
  set(PLUGIN_EXTENSION "so")
endif()

file(GLOB EXTRA_PLUGINS "${BUNDLE_PLUGIN_DIR}/*.${PLUGIN_EXTENSION}")
fixup_bundle("${BUNDLE_EXECUTABLE}" "${EXTRA_PLUGINS}" "${LIB_PATHS}" IGNORE_ITEM "vcredist_x86.exe;vcredist_x64.exe")
