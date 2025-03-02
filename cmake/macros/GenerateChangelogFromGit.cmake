#
#  GetVersionFromGit.cmake
#  cmake/macros
#
#  Created by Dale Glass on 08/12/24
#  Copyright 2024 Overte e.V.
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http:#www.apache.org/licenses/LICENSE-2.0.html
#

# Generate a changelog from git.
# First, we find the closest tag (eg, the current release). Then we backtrack one more level, and find the tag
# that came before that. This way we generate a changelog starting from the release before the current one.

function(generate_changelog_from_git)
    message(STATUS "Generating changelog from git...")

    find_package(Git)
    if (Git_FOUND)
        message(STATUS "Git is at ${GIT_EXECUTABLE}")
        message(STATUS "Source dir is ${CMAKE_CURRENT_LIST_DIR}")

        EXECUTE_PROCESS(
            COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0
            WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
            OUTPUT_VARIABLE GIT_CLOSEST_TAG
            RESULT_VARIABLE GIT_DESCRIBE_RESULT
            ERROR_VARIABLE GIT_DESCRIBE_ERROR
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        if (${GIT_DESCRIBE_ERROR})
            message(WARNING "Git describe result code: ${GIT_DESCRIBE_RESULT}")
            message(WARNING "Git describe error: ${GIT_DESCRIBE_ERROR}")
            return()
        endif()

        EXECUTE_PROCESS(
            COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0 ${GIT_CLOSEST_TAG}^
            WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
            OUTPUT_VARIABLE GIT_PREVIOUS_TAG
            RESULT_VARIABLE GIT_DESCRIBE_RESULT
            ERROR_VARIABLE GIT_DESCRIBE_ERROR
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        if (${GIT_DESCRIBE_ERROR})
            message(WARNING "Git describe result code: ${GIT_DESCRIBE_RESULT}")
            message(WARNING "Git describe error: ${GIT_DESCRIBE_ERROR}")
            return()
        endif()


        message(STATUS "Closest tag : ${GIT_CLOSEST_TAG}")
        message(STATUS "Previous tag: ${GIT_PREVIOUS_TAG}")
    else()
        message(WARNING "Git not found!")
    endif()

endfunction()
