# 
#  Created by Bradley Austin Davis on 2018/07/22
#  Copyright 2013-2018 High Fidelity, Inc.
#  Copyright 2025 Overte e.V.
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
# 
macro(TARGET_JSON)
    find_package(nlohmann_json REQUIRED)
    target_link_libraries(${TARGET_NAME} nlohmann_json::nlohmann_json)
endmacro()