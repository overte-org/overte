# Copyright 2013-2019 High Fidelity, Inc.
# Copyright 2019-2021 Vircadia contributors.
# Copyright 2020-2025 Overte e.V.
# SPDX-License-Identifier: Apache-2.0

# As the official CMake documentation is pretty bad for non-experts, Craig Scott's "Professional CMake: A Practical Guide" is a worthwhile investment,
# both as a guide and as a reference manual. https://crascit.com/professional-cmake/

# This should allow using long paths on Windows
SET(CMAKE_NINJA_FORCE_RESPONSE_FILE 1 CACHE INTERNAL "")
# 3.24 is the minimum version that supports COMPILE_WARNING_AS_ERROR
cmake_minimum_required(VERSION 3.24)

include(SelectLibraryConfigurations)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Instruct CMake to run rcc automatically when needed
set(CMAKE_AUTORCC ON)

# set our OS X deployment target
if (APPLE)
    # Needs to be set before the first project() call.
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.11" CACHE STRING "Specifies the minimum version of the target platform (e.g. macOS or iOS) on which the target binaries are to be deployed.")
endif()

set(RELEASE_TYPE "$ENV{RELEASE_TYPE}")
if ((NOT "${RELEASE_TYPE}" STREQUAL "PRODUCTION") AND (NOT "${RELEASE_TYPE}" STREQUAL "PR"))
    set(RELEASE_TYPE "DEV")
endif()


#
# Default compiler flags
#
# Compiler flags need to be set before calling project().
if( NOT WIN32 )
    # Function alignment is necessary for V8.
    # SetAlignedPointerInInternalField requires at least 2 byte alignment and -falign-functions will set alignment
    # to machine specific value which should be greater than 2.
    if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        set(CMAKE_CXX_FLAGS "-falign-functions -fPIC -m64 -mss3" CACHE STRING "C++ compiler flags for all build types.")
        set(CMAKE_C_FLAGS "-falign-functions -fPIC -m64 -mss3" CACHE STRING "C compiler flags for all build types.")
    elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
        # TODO: Find out which architecture optimizations to use on aarch64.
        # We should probably target the 64bit version of the Raspberry Pi 2 as minimum.
        set(CMAKE_CXX_FLAGS "-falign-functions -fPIC" CACHE STRING "C++ compiler flags for all build types.")
        set(CMAKE_C_FLAGS "-falign-functions -fPIC" CACHE STRING "C++ compiler flags for all build types.")
    endif()
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message("Clang compiler detected, setting default flags.")
        set(CMAKE_CXX_FLAGS_DEBUG "-Og -g" CACHE STRING "C++ compiler flags for Debug builds.")
        set(CMAKE_C_FLAGS_DEBUG "-Og -g" CACHE STRING "C compiler flags for Debug builds.")
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -DNDEBUG -g" CACHE STRING "C++ compiler flags for RelWithDebInfo builds.")
        set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -DNDEBUG -g" CACHE STRING "C compiler flags for RelWithDebInfo builds.")
        set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG" CACHE STRING "C++ compiler flags for Release builds.")
        set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG" CACHE STRING "C compiler flags for Release builds.")
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        message("GCC compiler detected, setting default flags")
        set(CMAKE_CXX_FLAGS_DEBUG "-Og -ggdb3" CACHE STRING "C++ compiler flags for Debug builds.")
        set(CMAKE_C_FLAGS_DEBUG "-Og -ggdb3" CACHE STRING "C compiler flags for Debug builds.")
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -DNDEBUG -ggdb2" CACHE STRING "C++ compiler flags for RelWithDebInfo builds.")
        set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -DNDEBUG -ggdb2" CACHE STRING "C compiler flags for RelWithDebInfo builds.")
        set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG" CACHE STRING "C++ compiler flags for Release builds.")
        set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG" CACHE STRING "C compiler flags for Release builds.")
    endif()
endif()


#
# OVERTE_WARNINGS
#
# Here we add the ability to allowlist warnings we've determined we can't fix, or are safe to
# ignore for one reason or another. The way of doing so is compiler-specific, so we deal with
# the detection of that in cmake, and just pass it down to the code from here.
#
# We can also treat warnings as errors. Without the allowlist this will almost certainly lead
# to a build failure.
set(OVERTE_WARNINGS_ALLOWLIST ON CACHE BOOL "Allowlist some warnings we can't currently fix.")

if(OVERTE_WARNINGS_ALLOWLIST)
    if (NOT WIN32)
      set(CMAKE_PLATFORM_INFO_DIR "${CMAKE_CURRENT_BINARY_DIR}")
      include(CMakeDetermineCXXCompiler)
    endif()

    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        message("GCC compiler detected, suppressing some unsolvable warnings.")
        add_compile_definitions(OVERTE_WARNINGS_ALLOWLIST_GCC)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message("Clang compiler detected, suppressing some unsolvable warnings.")
        add_compile_definitions(OVERTE_WARNINGS_ALLOWLIST_CLANG)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC" OR (CMAKE_CXX_COMPILER_ID MATCHES "" AND WIN32))
        message("Microsoft Visual Studio compiler detected, suppressing some unsolvable warnings.")
        add_compile_definitions(OVERTE_WARNINGS_ALLOWLIST_MSVC)
    else()
        message("We don't know yet how to allowlist warnings for ${CMAKE_CXX_COMPILER_ID}")
    endif()
endif()

# Enabling warnings-as-errors by default on our Linux target and on Windows.
# Our current Linux development target is Ubuntu 22.04, which uses GCC 11.2.
# TODO: Enable warnings-as-errors once we stop throwing warnings on the relevant platforms.
if ((CMAKE_CXX_COMPILER_ID MATCHES "GNU") AND (CMAKE_CXX_COMPILER_VERSION MATCHES "11"))
    set(COMPILE_WARNING_AS_ERROR OFF CACHE BOOL "")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC") # Windows
    set(COMPILE_WARNING_AS_ERROR OFF CACHE BOOL "")
endif()


project(overte)
include("cmake/init.cmake")
include("cmake/compiler.cmake")



#
# Overte build variables
#
set(OVERTE_BUILD_CLIENT ON CACHE BOOL "Build Interface.")
set(OVERTE_BUILD_SERVER ON CACHE BOOL "Build domain-server, assignment-client, and ice-server.")
set(OVERTE_BUILD_TESTS OFF CACHE BOOL "Build tests.")
set(OVERTE_BUILD_MANUAL_TESTS OFF CACHE BOOL "Build manual tests.")
set(OVERTE_BUILD_TOOLS ON CACHE BOOL "Build tools. Includes Oven, which is used for some server functionality.")
set(OVERTE_BUILD_INSTALLER ON CACHE BOOL "Build installer.")
set(OVERTE_DISABLE_QML OFF CACHE BOOL "Build without QML. (For graphics debugging.)")
set(OVERTE_DOWNLOAD_SERVERLESS_CONTENT OFF CACHE BOOL "Download and setup default serverless content beside Interface.")
# use OpenGL ES on Linux aarch64 and Android.
if ((CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64") OR ANDROID)
    set(OVERTE_RENDERING_BACKEND "GLES" CACHE STRING "Which rendering backend to compile with. Valid options are: 'OpenGL', 'GLES', and 'Vulkan'.")
else()
    set(OVERTE_RENDERING_BACKEND "OpenGL" CACHE STRING "Which rendering backend to compile with. Valid options are: 'OpenGL', 'GLES', and 'Vulkan'.")
endif()
set(OVERTE_EXTERNAL_BUILD_ASSETS "https://build-deps.overte.org" CACHE INTERNAL "")
set(OVERTE_TIMESERVER_URL "http://timestamp.comodoca.com?td=sha256" CACHE STRING "URL to timeserver used for signing executables.")
set(OVERTE_USE_KHR_ROBUSTNESS OFF CACHE BOOL "Use KHR_robustness.")


set(HIFI_USE_OPTIMIZED_IK_OPTION OFF)
set(OVERTE_BACKTRACE_URL "" CACHE STRING "URL to an endpoint for uploading crash-dumps. For example Sentry.")
set(OVERTE_BACKTRACE_TOKEN "" CACHE STRING "Token used to identify with release or build is uploading crash-dumps.")

if (ANDROID OR UWP)
    set(OVERTE_BUILD_SERVER OFF CACHE BOOL FORCE "Overwritten (mobile build)")
    set(OVERTE_BUILD_TOOLS OFF CACHE BOOL FORCE "Overwritten (mobile build)")
    set(OVERTE_BUILD_INSTALLER OFF CACHE BOOL FORCE "Overwritten (mobile build)")
endif()

if (ANDROID)
  set(PLATFORM_QT_COMPONENTS AndroidExtras WebView)
  add_definitions(-DHIFI_ANDROID_APP=\"${HIFI_ANDROID_APP}\")
  if (
      (${HIFI_ANDROID_APP} STREQUAL "questInterface") OR
      (${HIFI_ANDROID_APP} STREQUAL "questFramePlayer") OR
      (${HIFI_ANDROID_APP} STREQUAL "framePlayer")
    )
    # We know the quest hardware has this extension, so we can force the use of instanced stereo
    add_definitions(-DHAVE_EXT_clip_cull_distance)
    # We can also use multiview stereo techniques
    add_definitions(-DHAVE_OVR_multiview2)
    add_definitions(-DHAVE_OVR_multiview)
    # We can also use our own foveated textures
    add_definitions(-DHAVE_QCOM_texture_foveated)

    # if set, the application itself or some library it depends on MUST implement
    # `DisplayPluginList getDisplayPlugins()` and `InputPluginList getInputPlugins()`
    add_definitions(-DCUSTOM_INPUT_PLUGINS)
    add_definitions(-DCUSTOM_DISPLAY_PLUGINS)
    set(PLATFORM_PLUGIN_LIBRARIES oculusMobile oculusMobilePlugin)
  endif()

  # Allow client code to use preprocessor macros to distinguish between quest and non-quest builds
  if (${HIFI_ANDROID_APP} STREQUAL "questInterface")
    add_definitions(-DANDROID_APP_QUEST_INTERFACE)
  elseif(${HIFI_ANDROID_APP} STREQUAL "interface")
    add_definitions(-DANDROID_APP_INTERFACE)
  endif()
else ()
  set(PLATFORM_QT_COMPONENTS WebEngine Xml)
endif()

option(HIFI_USE_OPTIMIZED_IK "Use optimized IK" ${HIFI_USE_OPTIMIZED_IK_OPTION})
option(DISABLE_KTX_CACHE "Disable KTX Cache" OFF)

if (OVERTE_USE_KHR_ROBUSTNESS)
  add_definitions(-DUSE_KHR_ROBUSTNESS)
endif()

if (OVERTE_RENDERING_BACKEND STREQUAL "GLES")
    add_definitions(-DUSE_GLES)
    add_definitions(-DGPU_POINTER_STORAGE_SHARED)
    set(PLATFORM_GL_BACKEND gpu-gl-common gpu-gles)
elseif (OVERTE_RENDERING_BACKEND STREQUAL "OpenGL")
    add_definitions(-DGPU_POINTER_STORAGE_RAW)
    set(PLATFORM_GL_BACKEND gpu-gl-common gpu-gl)
elseif (OVERTE_RENDERING_BACKEND STREQUAL "Vulkan")
    # TODO
    message(FATAL_ERROR "Vulkan backend selected, but not implemented yet.")
else()
    message(FATAL_ERROR "Invalid OVERTE_RENDERING_BACKEND set. Expected 'GLES', 'OpenGL', or 'Vulkan'. Got '${OVERTE_RENDERING_BACKEND}'")
endif()

foreach(PLATFORM_QT_COMPONENT ${PLATFORM_QT_COMPONENTS})
  list(APPEND PLATFORM_QT_LIBRARIES "Qt5::${PLATFORM_QT_COMPONENT}")
endforeach()

MESSAGE(STATUS "Use optimized IK:      " ${HIFI_USE_OPTIMIZED_IK})
message(STATUS "Build server:          " ${OVERTE_BUILD_SERVER})
message(STATUS "Build client:          " ${OVERTE_BUILD_CLIENT})
message(STATUS "Build tests:           " ${OVERTE_BUILD_TESTS})
message(STATUS "Build tools:           " ${OVERTE_BUILD_TOOLS})
message(STATUS "Build installer:       " ${OVERTE_BUILD_INSTALLER})
message(STATUS "Rendering backend:     " ${OVERTE_RENDERING_BACKEND})
MESSAGE(STATUS "DL serverless content: " ${DOWNLOAD_SERVERLESS_CONTENT})

if (OVERTE_DISABLE_QML)
    message(STATUS "QML disabled!")
    add_definitions(-DDISABLE_QML)
endif()

if (DISABLE_KTX_CACHE)
  MESSAGE(STATUS "KTX cache disabled!")
  add_definitions(-DDISABLE_KTX_CACHE)
endif()

if (UNIX AND DEFINED ENV{HIFI_MEMORY_DEBUGGING})
  MESSAGE(STATUS "Memory debugging is enabled")
endif()

# Set SERVER_ONLY and CLIENT_ONLY for later packaging.
if ((NOT OVERTE_BUILD_CLIENT) AND OVERTE_BUILD_SERVER)
    set(SERVER_ONLY ON CACHE BOOL INTERNAL "")
elseif ((NOT OVERTE_BUILD_SERVER) AND OVERTE_BUILD_CLIENT)
    set(CLIENT_ONLY ON CACHE BOOL INTERNAL "")
endif()


#
# Helper projects
#
file(GLOB_RECURSE CMAKE_SRC cmake/*.cmake cmake/CMakeLists.txt)
add_custom_target(cmake SOURCES ${CMAKE_SRC})
GroupSources("cmake")
unset(CMAKE_SRC)

file(GLOB_RECURSE JS_SRC scripts/*.* unpublishedScripts/*.*)
add_custom_target(js SOURCES ${JS_SRC})
GroupSources("scripts")
GroupSources("unpublishedScripts")
unset(JS_SRC)

# Include Overte Web app files if cloned into a subdirectory.
if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/overte-web")
    file(GLOB_RECURSE WEB_APP_SRC overte-web/*.*)
    list(FILTER WEB_APP_SRC EXCLUDE REGEX "overte-web/(dist|node_modules|public)/*" )
    overte(overte-web SOURCES ${WEB_APP_SRC})
    GroupSources("overte-web")
    unset(WEB_APP_SRC)
endif()

# Include Overte Web SDK files if cloned into a subdirectory.
if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/overte-web-sdk")
    file(GLOB_RECURSE WEB_SDK_SRC overte-web-sdk/*.*)
    list(FILTER WEB_SDK_SRC EXCLUDE REGEX "overte-web-sdk/(dist|node_modules|public)/*" )
    add_custom_target(overte-web-sdk SOURCES ${WEB_SDK_SRC})
    GroupSources("overte-web-sdk")
    unset(WEB_SDK_SRC)
endif()

set_packaging_parameters()


find_package( Threads )

add_definitions(-DGLM_FORCE_RADIANS)
add_definitions(-DGLM_ENABLE_EXPERIMENTAL)
add_definitions(-DGLM_FORCE_CTOR_INIT)

if (HIFI_USE_OPTIMIZED_IK)
  MESSAGE(STATUS "SET THE USE IK DEFINITION ")
  add_definitions(-DHIFI_USE_OPTIMIZED_IK)
endif()
set(HIFI_LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libraries")

set(EXTERNAL_PROJECT_PREFIX "project")
set_property(DIRECTORY PROPERTY EP_PREFIX ${EXTERNAL_PROJECT_PREFIX})
setup_externals_binary_dir()

set(OVERTE_USE_NSIGHT ON CACHE BOOL "Attempt to find the Nvidia nSight libraries.")

# FIXME hack to work on the proper Android toolchain
if (ANDROID)
  add_subdirectory(android/apps/${HIFI_ANDROID_APP})
  return()
endif()

if (BUILD_GPU_FRAME_PLAYER_ONLY)
    # This is for CI build testing
    add_subdirectory(tools/gpu-frame-player)
else()

# OVERTE_BUILD_TOOLS option will be handled inside the tools's CMakeLists.txt because 'scribe' tool is required for build anyway
add_subdirectory(tools)

# add subdirectories for all targets
if (OVERTE_BUILD_SERVER)
  add_subdirectory(assignment-client)
  set_target_properties(assignment-client PROPERTIES FOLDER "Apps")
  add_subdirectory(domain-server)
  set_target_properties(domain-server PROPERTIES FOLDER "Apps")
  add_subdirectory(ice-server)
  set_target_properties(ice-server PROPERTIES FOLDER "Apps")
endif()

if (OVERTE_BUILD_CLIENT)
  add_subdirectory(interface)
  if (APPLE)
    set_target_properties(Overte PROPERTIES FOLDER "Apps")
  else()
    set_target_properties(interface PROPERTIES FOLDER "Apps")
  endif()

  option(USE_SIXENSE "Build Interface with sixense library/plugin" OFF)
  option(USE_NEURON "Build Interface with Neuron library/plugin" OFF)
endif()

if (OVERTE_BUILD_CLIENT OR OVERTE_BUILD_SERVER)
  add_subdirectory(plugins)
  add_subdirectory(server-console)
endif()

endif()

if (OVERTE_BUILD_TESTS)
  # Turn on testing so that add_test works
  # MUST be in the root cmake file for ctest to work
  include(CTest)
  enable_testing()
  add_subdirectory(tests)
  if (OVERTE_BUILD_MANUAL_TESTS)
    add_subdirectory(tests-manual)
  endif()
endif()

if (OVERTE_BUILD_INSTALLER)
  if (UNIX)
    install(
        DIRECTORY "${CMAKE_SOURCE_DIR}/scripts"
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/interface
        COMPONENT ${CLIENT_COMPONENT}
    )
  endif()
  generate_installers()
endif()

# QML import paths for Qt Creator and KDevelop code completion
set(QML_IMPORT_PATH ${CMAKE_SOURCE_DIR}/interface/resources/qml;${CMAKE_SOURCE_DIR}/launchers/qt/resources/qml CACHE PATH "Extra QML import paths for KDevelop and Qt Creator")
