# 
#  IncludeHifiLibraryHeaders.cmake
#  cmake/macros
# 
#  Created by Stephen Birarda on August 8, 2014
#  Copyright 2014 High Fidelity, Inc.
#  Copyright 2024 Overte e.V.
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
# 

macro(include_hifi_library_headers LIBRARY)
  target_include_directories(${TARGET_NAME} PRIVATE "${HIFI_LIBRARY_DIR}/${LIBRARY}/src")
  if (${LIBRARY} STREQUAL "entities")
    target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/libraries/entities/src")
    target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_BINARY_DIR}/libraries/entities/src")
  endif()
endmacro(include_hifi_library_headers _library _root_dir)
