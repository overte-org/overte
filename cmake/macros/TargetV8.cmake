#
#  Copyright 2022-2023 Overte e.V.
#  Created by dr Karol Suprynowicz on 2022/09/03
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
#  SPDX-License-Identifier: Apache-2.0
#
macro(TARGET_V8)
    if(OVERTE_USE_SYSTEM_LIBS)
        # NOTE: this is configured for NixOS specifically
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(libv8 REQUIRED v8)
        target_include_directories(${TARGET_NAME} SYSTEM PRIVATE ${libv8_INCLUDE_DIRS})
        target_link_libraries(${TARGET_NAME} ${libv8_LINK_LIBRARIES})
    else()
        find_package(libnode REQUIRED)
        target_link_libraries(${TARGET_NAME} libnode::libnode)
    endif()
endmacro()
