#
#  Copyright 2022-2023 Overte e.V.
#  Created by dr Karol Suprynowicz on 2022/09/03
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
#  SPDX-License-Identifier: Apache-2.0
#
macro(TARGET_V8)

find_package(V8 REQUIRED)
target_include_directories(${TARGET_NAME} PUBLIC ${V8_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} ${V8_LIBRARIES})
install_external_library("${V8_LIBRARIES}")


endmacro()