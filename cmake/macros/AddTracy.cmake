#
#  AddTracy.cmake
#  cmake/macros
#
#  Created by Julian Groß on 2026-05-09.
#  Copyright 2026 Overte e.V.
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http:#www.apache.org/licenses/LICENSE-2.0.html
#

macro(ADD_TRACY)
    if(OVERTE_USE_TRACY)
        option(TRACY_ENABLE "Enable the Tracy CPU/GPU/memory profiler." ON)
        option(TRACY_ON_DEMAND "" ON)

        include(FetchContent)

        FetchContent_Declare(
            tracy
            GIT_REPOSITORY https://github.com/wolfpld/tracy.git
            GIT_TAG master
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
        )
        FetchContent_MakeAvailable(tracy)

        target_link_libraries(${TARGET_NAME} Tracy::TracyClient)
    endif()
endmacro()
