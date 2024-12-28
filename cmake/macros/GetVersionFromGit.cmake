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

function(get_version_from_git)
    message(STATUS "Getting version from git...")

    find_package(Git)
    if (Git_FOUND)
        message(STATUS "Git is at ${GIT_EXECUTABLE}")
        message(STATUS "Source dir is ${CMAKE_CURRENT_LIST_DIR}")

        EXECUTE_PROCESS(
            COMMAND ${GIT_EXECUTABLE} describe --tags --dirty=-modified
            WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
            OUTPUT_VARIABLE GIT_VERSION_RAW
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
            COMMAND ${GIT_EXECUTABLE} rev-parse --verify HEAD
            WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
            OUTPUT_VARIABLE GIT_COMMIT
            RESULT_VARIABLE GIT_DESCRIBE_RESULT
            ERROR_VARIABLE GIT_DESCRIBE_ERROR
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        if (${GIT_DESCRIBE_ERROR})
            message(WARNING "Git rev-parse result code: ${GIT_DESCRIBE_RESULT}")
            message(WARNING "Git rev-parse error: ${GIT_DESCRIBE_ERROR}")
            return()
        endif()

        # Git returns something like this:
        # 2024.11.1-52-g6648ea8187-modified
        #
        # It's a bit much for packaging, so we simplify things a bit by parsing the components out.

        message(STATUS "Raw version is: ${GIT_VERSION_RAW}")
        message(STATUS "Git commit is : ${GIT_COMMIT}")

        STRING(REGEX REPLACE "^([0-9]+)\.([0-9]+)\.([0-9]+)-([0-9]+).*"    "\\1.\\2.\\3.\\4" GIT_VERSION_FULL "${GIT_VERSION_RAW}")

        message(STATUS "Version is    : ${GIT_VERSION_FULL}")
        STRING(REGEX REPLACE "^([0-9]+)\.([0-9]+)\.([0-9]+)-([0-9]+).*"    "\\1" GIT_VERSION_MAJOR "${GIT_VERSION_RAW}")
        STRING(REGEX REPLACE "^([0-9]+)\.([0-9]+)\.([0-9]+)-([0-9]+).*"    "\\2" GIT_VERSION_MINOR "${GIT_VERSION_RAW}")
        STRING(REGEX REPLACE "^([0-9]+)\.([0-9]+)\.([0-9]+)-([0-9]+).*"    "\\3" GIT_VERSION_PATCH "${GIT_VERSION_RAW}")
        STRING(REGEX REPLACE "^([0-9]+)\.([0-9]+)\.([0-9]+)-([0-9]+).*"    "\\4" GIT_VERSION_TWEAK "${GIT_VERSION_RAW}")


        #message(STATUS "Major: ${GIT_VERSION_MAJOR}")
        #message(STATUS "Minor: ${GIT_VERSION_MINOR}")
        #message(STATUS "Patch: ${GIT_VERSION_PATCH}")
        #message(STATUS "Tweak: ${GIT_VERSION_TWEAK}")

        set(GIT_COMMIT        "${GIT_COMMIT}"       PARENT_SCOPE)

        set(GIT_VERSION_FULL  "${GIT_VERSION_FULL}" PARENT_SCOPE)
        set(GIT_VERSION_MAJOR "${GIT_VERSION_MAJOR}" PARENT_SCOPE)
        set(GIT_VERSION_MINOR "${GIT_VERSION_MINOR}" PARENT_SCOPE)
        set(GIT_VERSION_PATCH "${GIT_VERSION_PATCH}" PARENT_SCOPE)
        set(GIT_VERSION_TWEAK "${GIT_VERSION_TWEAK}" PARENT_SCOPE)
    else()
        message(WARNING "Git not found!")
    endif()

endfunction()
