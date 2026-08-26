#
#  AddSentry.cmake
#  cmake/macros
#
#  Created by Clement Brisset on 01/19/18.
#  Copyright 2018 High Fidelity, Inc.
#  Copyright 2025-2026 Overte e.V.
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http:#www.apache.org/licenses/LICENSE-2.0.html
#

macro(add_sentry)
  if (OVERTE_USE_SENTRY)
    if (OVERTE_BACKTRACE_URL STREQUAL "")
        message(FATAL_ERROR "Sentry is enabled, but -DOVERTE_BACKTRACE_URL is empty!")
    endif()
    if (OVERTE_BACKTRACE_TOKEN STREQUAL "")
        message(FATAL_ERROR "Sentry is enabled, but -DOVERTE_BACKTRACE_TOKEN is empty!")
    endif()

    find_package(Sentry QUIET REQUIRED)

    add_definitions(-DHAS_SENTRY)
    add_definitions(-DOVERTE_BACKTRACE_URL=\"${OVERTE_BACKTRACE_URL}\")
    add_definitions(-DOVERTE_BACKTRACE_TOKEN=\"${OVERTE_BACKTRACE_TOKEN}\")

    target_link_libraries(${TARGET_NAME} sentry-native::sentry-native)


    # if (WIN32)
    # set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS "/ignore:4099")
    # elseif (APPLE)
    # find_library(Security Security)
    # target_link_libraries(${TARGET_NAME} ${Security})
    # target_link_libraries(${TARGET_NAME} "-lbsm")
    # endif()


    # Find and pass crashpad_hander to CPack for packaging. crashpad_handler is used for sending crash reports to Sentry.
    find_program(CRASHPAD_HANDLER_EXECUTABLE crashpad_handler PATHS "${sentry_INCLUDE_DIR}/../bin/" NO_DEFAULT_PATH)
    if (NOT CRASHPAD_HANDLER_EXECUTABLE)
        message(FATAL_ERROR "Could not find Crashpad-handler at ${sentry_INCLUDE_DIR}/../bin/. Crashpad-handler is required for submitting crash reports to Sentry.")
    endif ()
    # TODO: Make sure we are packaging crashpad_handler into our AppImages/installers.
    set(CPACK_CRASHPAD_HANDLER_EXECUTABLE ${CRASHPAD_HANDLER_EXECUTABLE})

    add_custom_command(
      TARGET ${TARGET_NAME}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${CRASHPAD_HANDLER_EXECUTABLE} "$<TARGET_FILE_DIR:${TARGET_NAME}>/"
    )
  endif ()
endmacro()
