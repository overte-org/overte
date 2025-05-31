#
#  SetPackagingParameters.cmake
#  cmake/macros
#
#  Created by Leonardo Murillo on 07/14/2015.
#  Copyright 2015 High Fidelity, Inc.
#  Copyright 2020 Vircadia contributors.
#  Copyright 2025 Overte e.V.
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html

# This macro checks some Jenkins defined environment variables to determine the origin of this build
# and decides how targets should be packaged.

macro(SET_PACKAGING_PARAMETERS)
  set(PR_BUILD 0)
  set(PRODUCTION_BUILD 0)
  set(DEV_BUILD 0)
  set(BUILD_GLOBAL_SERVICES "DEVELOPMENT")
  set(USE_STABLE_GLOBAL_SERVICES 0)
  set(OVERTE_GIT_COMMIT_SHORT 0 CACHE STRING "Short Git commit hash to use for versioning.")

  set(APP_USER_MODEL_ID "com.highfidelity.console-dev")

  set_from_env(STABLE_BUILD STABLE_BUILD 0)
  set(OVERTE_RELEASE_TYPE "DEV" CACHE STRING "Valid options are: 'PRODUCTION', 'PR', 'NIGHTLY', and 'DEV'.")
  set(OVERTE_RELEASE_NUMBER "0000.00.0" CACHE STRING "Release version number. E.g. 2025.05.1-rc1 for the first release candidate of the first release in May 2025.")

  set_from_env(PRELOADED_STARTUP_LOCATION PRELOADED_STARTUP_LOCATION "")
  set_from_env(PRELOADED_SCRIPT_ALLOWLIST PRELOADED_SCRIPT_ALLOWLIST "")

  set_from_env(BYPASS_SIGNING BYPASS_SIGNING 0)

  message(STATUS "The OVERTE_RELEASE_TYPE variable is: ${OVERTE_RELEASE_TYPE}")

  # setup component categories for installer
  set(DDE_COMPONENT dde)
  set(CLIENT_COMPONENT client)
  set(SERVER_COMPONENT server)

  if (APPLE)
    set(INTERFACE_BUNDLE_NAME "Overte")
  else()
    set(INTERFACE_BUNDLE_NAME "interface")
  endif()

  string(TIMESTAMP BUILD_DATE "%Y-%m-%d" UTC)

  if (OVERTE_RELEASE_TYPE STREQUAL "PRODUCTION")
    set(PRODUCTION_BUILD 1)
    set(BUILD_VERSION ${OVERTE_RELEASE_NUMBER})
    set(BUILD_ORGANIZATION "Overte")
    set(HIGH_FIDELITY_PROTOCOL "hifi")
    set(HIGH_FIDELITY_APP_PROTOCOL "hifiapp")
    set(INTERFACE_ICON_PREFIX "interface")

    # add definition for this release type
    add_definitions(-DPRODUCTION_BUILD)

    # if the build is a PRODUCTION_BUILD from the "stable" branch
    # then use the STABLE gobal services
    if (STABLE_BUILD)
      message(STATUS "The RELEASE_TYPE is PRODUCTION and STABLE_BUILD is 1")
      set(BUILD_GLOBAL_SERVICES "STABLE")
      set(USE_STABLE_GLOBAL_SERVICES 1)
    endif ()

    if (NOT BYPASS_SIGNING)
      set(BYPASS_SIGNING 0)
    endif ()      

  elseif (OVERTE_RELEASE_TYPE STREQUAL "PR")
    set(PR_BUILD 1)
    set(BUILD_VERSION "PR${OVERTE_RELEASE_NUMBER}-${BUILD_DATE}")
    set(BUILD_ORGANIZATION "Overte - ${BUILD_VERSION}")
    set(INTERFACE_ICON_PREFIX "interface-beta")

    # add definition for this release type
    add_definitions(-DPR_BUILD)

  elseif (OVERTE_RELEASE_TYPE STREQUAL "NIGHTLY")
    set(NIGHTLY_BUILD 1)
    set(BUILD_VERSION "Nightly-${BUILD_DATE}")
    set(BUILD_ORGANIZATION "Overte - ${BUILD_VERSION}")
    set(INTERFACE_ICON_PREFIX "interface-beta")

    # add definition for this release type
    add_definitions(-DPR_BUILD)

  elseif (OVERTE_RELEASE_TYPE STREQUAL "DEV")
    set(DEV_BUILD 1)
    set(BUILD_VERSION "Dev-${BUILD_DATE}")
    set(BUILD_ORGANIZATION "Overte - ${BUILD_VERSION}")
    set(INTERFACE_ICON_PREFIX "interface-beta")

    # add definition for this release type
    add_definitions(-DDEV_BUILD)

  else()
    message(FATAL_ERROR "OVERTE_RELEASE_TYPE invalid. Expected: 'RELEASE', 'PR', 'NIGHTLY', or 'DEV'. Got: '${OVERTE_RELEASE_TYPE}'")
  endif()

  set(NITPICK_BUNDLE_NAME "nitpick")
  if (PRODUCTION_BUILD)
    set(NITPICK_ICON_PREFIX "nitpick")
  else ()
    set(NITPICK_ICON_PREFIX "nitpick-beta")
  endif ()

  # if STABLE_BUILD is 1, PRODUCTION_BUILD must be 1 and
  # DEV_BUILD and PR_BUILD must be 0
  if (STABLE_BUILD)
    if ((NOT PRODUCTION_BUILD) OR PR_BUILD OR DEV_BUILD)
      message(FATAL_ERROR "Cannot produce STABLE_BUILD without PRODUCTION_BUILD")
    endif ()
  endif ()

  set(BUILD_VERSION_NO_SHA ${BUILD_VERSION})
  if (NOT PRODUCTION_BUILD)
    # append the abbreviated commit SHA to the build version
    # since this is a PR build or master/nightly build
    set(BUILD_VERSION "${BUILD_VERSION}-${OVERTE_GIT_COMMIT_SHORT}")
  endif ()

  if (APPLE)
    set(DMG_SUBFOLDER_NAME "${BUILD_ORGANIZATION}")

    set(ESCAPED_DMG_SUBFOLDER_NAME "")
    string(REPLACE " " "\\ " ESCAPED_DMG_SUBFOLDER_NAME ${DMG_SUBFOLDER_NAME})

    set(DMG_SUBFOLDER_ICON "${HF_CMAKE_DIR}/installer/install-folder.rsrc")

    set(CONSOLE_INSTALL_DIR       ".")
    set(INTERFACE_INSTALL_DIR     ".")
    set(NITPICK_INSTALL_DIR       ".")

    if (NOT OVERTE_BUILD_SERVER)
        set(CONSOLE_EXEC_NAME "Console.app")
    else ()
        set(CONSOLE_EXEC_NAME "Sandbox.app")
    endif()
    set(CONSOLE_INSTALL_APP_PATH "${CONSOLE_INSTALL_DIR}/${CONSOLE_EXEC_NAME}")

    set(CONSOLE_APP_CONTENTS "${CONSOLE_INSTALL_APP_PATH}/Contents")
    set(COMPONENT_APP_PATH "${CONSOLE_APP_CONTENTS}/MacOS/Components.app")
    set(COMPONENT_INSTALL_DIR "${COMPONENT_APP_PATH}/Contents/MacOS")
    set(CONSOLE_PLUGIN_INSTALL_DIR "${COMPONENT_APP_PATH}/Contents/PlugIns")

    set(INTERFACE_INSTALL_APP_PATH "${INTERFACE_INSTALL_DIR}/${INTERFACE_BUNDLE_NAME}.app")
    set(INTERFACE_ICON_FILENAME "${INTERFACE_ICON_PREFIX}.icns")
    set(NITPICK_ICON_FILENAME "${NITPICK_ICON_PREFIX}.icns")
  else ()
    if (WIN32)
      set(CONSOLE_INSTALL_DIR "server-console")
      set(NITPICK_INSTALL_DIR "nitpick")
    else ()
      set(CONSOLE_INSTALL_DIR ".")
      set(NITPICK_INSTALL_DIR ".")
    endif ()

    set(COMPONENT_INSTALL_DIR ".")
    set(INTERFACE_INSTALL_DIR ".")
  endif ()

  if (WIN32)
    set(INTERFACE_EXEC_PREFIX "interface")
    set(INTERFACE_ICON_FILENAME "${INTERFACE_ICON_PREFIX}.ico")
    set(NITPICK_ICON_FILENAME "${NITPICK_ICON_PREFIX}.ico")

    set(CONSOLE_EXEC_NAME "server-console.exe")

    set(DS_EXEC_NAME "domain-server.exe")
    set(AC_EXEC_NAME "assignment-client.exe")

    # shortcut names
    if (PRODUCTION_BUILD)
      set(INTERFACE_SHORTCUT_NAME "Overte")
      set(CONSOLE_SHORTCUT_NAME "Console")
      set(SANDBOX_SHORTCUT_NAME "Server")
      set(APP_USER_MODEL_ID "org.overte.console")
    else ()
      set(INTERFACE_SHORTCUT_NAME "Overte - ${BUILD_VERSION_NO_SHA}")
      set(CONSOLE_SHORTCUT_NAME "Console - ${BUILD_VERSION_NO_SHA}")
      set(SANDBOX_SHORTCUT_NAME "Server - ${BUILD_VERSION_NO_SHA}")
    endif ()

    set(INTERFACE_HF_SHORTCUT_NAME "${INTERFACE_SHORTCUT_NAME}")
    set(CONSOLE_HF_SHORTCUT_NAME "Overte ${CONSOLE_SHORTCUT_NAME}")
    set(SANDBOX_HF_SHORTCUT_NAME "Overte ${SANDBOX_SHORTCUT_NAME}")

    set(PRE_SANDBOX_INTERFACE_SHORTCUT_NAME "Overte")
    set(PRE_SANDBOX_CONSOLE_SHORTCUT_NAME "Server Console")

    # check if we need to find signtool
    if (PRODUCTION_BUILD OR PR_BUILD)
      if (MSVC_VERSION GREATER_EQUAL 1910) # VS 2017
        find_program(SIGNTOOL_EXECUTABLE signtool PATHS "C:/Program Files (x86)/Windows Kits/10" PATH_SUFFIXES "bin/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/x64")
      elseif (MSVC_VERSION GREATER_EQUAL 1800) # VS 2013
        find_program(SIGNTOOL_EXECUTABLE signtool PATHS "C:/Program Files (x86)/Windows Kits/8.1" PATH_SUFFIXES "bin/x64")
      else()
        message( FATAL_ERROR "Visual Studio 2013 or higher required." )
      endif()

      if (NOT SIGNTOOL_EXECUTABLE)
        message(FATAL_ERROR "Code signing of executables was requested but signtool.exe could not be found.")
      endif ()
    endif ()

    set(GENERATED_UNINSTALLER_EXEC_NAME "Uninstall.exe")
    set(REGISTRY_HKLM_INSTALL_ROOT "Software")
    set(POST_INSTALL_OPTIONS_REG_GROUP "PostInstallOptions")
    set(CLIENT_DESKTOP_SHORTCUT_REG_KEY "ClientDesktopShortcut")
    set(CONSOLE_DESKTOP_SHORTCUT_REG_KEY "ConsoleDesktopShortcut")
    set(CONSOLE_STARTUP_REG_KEY "ConsoleStartupShortcut")
    set(CLIENT_LAUNCH_NOW_REG_KEY "ClientLaunchAfterInstall")
    set(SERVER_LAUNCH_NOW_REG_KEY "ServerLaunchAfterInstall")
    set(CUSTOM_INSTALL_REG_KEY "CustomInstall")
    set(CLIENT_ID_REG_KEY "ClientGUID")
    set(GA_TRACKING_ID $ENV{GA_TRACKING_ID})
  endif ()

  # print out some results for testing this new build feature
  message(STATUS "The BUILD_GLOBAL_SERVICES variable is: ${BUILD_GLOBAL_SERVICES}")
  message(STATUS "The USE_STABLE_GLOBAL_SERVICES variable is: ${USE_STABLE_GLOBAL_SERVICES}")

  # create a header file our targets can use to find out the application version
  file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/includes")
  configure_file("${HF_CMAKE_DIR}/templates/BuildInfo.h.in" "${CMAKE_BINARY_DIR}/includes/BuildInfo.h")
  include_directories("${CMAKE_BINARY_DIR}/includes")

endmacro(SET_PACKAGING_PARAMETERS)
