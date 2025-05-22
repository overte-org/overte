# Copyright 2013-2019 High Fidelity, Inc.
# Copyright 2019-2021 Vircadia contributors.
# Copyright 2020-2025 Overte e.V.
# SPDX-License-Identifier: Apache-2.0

# 3.14 is the minimum version that supports symlinks on Windows
cmake_minimum_required(VERSION 3.14)

# This should allow using long paths on Windows
SET(CMAKE_NINJA_FORCE_RESPONSE_FILE 1 CACHE INTERNAL "")

include(SelectLibraryConfigurations)

# read USE_GLES enviroment variable and sets it as GLES option
# TODO still gets overwritten by "use GLES on linux aarch64"
set(GLES_OPTION "$ENV{USE_GLES}")

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Instruct CMake to run rcc automatically when needed
set(CMAKE_AUTORCC ON)

# use GLES on linux aarch64
if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    set(GLES_OPTION ON)
endif()

if (WIN32)
    # Building with webrtc-audio-processing on Windows fails on cppstd 14.
    set(CMAKE_CXX_STANDARD 17)
else ()
    set(CMAKE_CXX_STANDARD 14)
endif ()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set our OS X deployment target
# (needs to be set before first project() call and before prebuild.py)
if (APPLE)
  set(ENV{MACOSX_DEPLOYMENT_TARGET} 10.11)
endif()

set(EXTERNAL_BUILD_ASSETS "https://build-deps.overte.org")

set(RELEASE_TYPE "$ENV{RELEASE_TYPE}")
if ((NOT "${RELEASE_TYPE}" STREQUAL "PRODUCTION") AND (NOT "${RELEASE_TYPE}" STREQUAL "PR"))
    set(RELEASE_TYPE "DEV")
endif()

# OVERTE_OPTIMIZE
# Variable determining Overte optimization. If not set, it defaults to true.
# Should be set to false to get completely unoptimized build for easier line-by-line debugging

if( NOT WIN32 )
    if(NOT DEFINED OVERTE_OPTIMIZE)
        message("Enabling code optimization for Overte and compiled dependencies")
        set(OVERTE_OPTIMIZE true CACHE BOOL "Enable code optimization for Overte and compiled dependencies")
    endif()

    set(OVERTE_OPTIMIZE_FLAGS "")

    if(OVERTE_OPTIMIZE)
        if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            message("Clang compiler detected, adding -O3 -fPIC -g flags")
            set(OVERTE_OPTIMIZE_FLAGS "-O3 -fPIC -g")
        elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
            message("GCC compiler detected, adding -O3 -fPIC -ggdb flags")
            set(OVERTE_OPTIMIZE_FLAGS "-O3 -fPIC -ggdb")
        else()
            message("No predefined optimization flags for compiler ${CMAKE_CXX_COMPILER_ID}")
        endif()
    endif()
    MESSAGE(STATUS "OVERTE_OPTIMIZE: ${OVERTE_OPTIMIZE}")

    # OVERTE_CPU_ARCHITECTURE
    # Variable determining CPU architecture for which Overte will be built.
    # If defined, it's appended to CXXFLAGS and CFLAGS for Overte

    #Assume -march=native for compilers that allow it if architecture is not specified
    if(NOT DEFINED OVERTE_CPU_ARCHITECTURE)
        if(OVERTE_OPTIMIZE AND ( (CMAKE_CXX_COMPILER_ID MATCHES "Clang") OR (CMAKE_CXX_COMPILER_ID MATCHES "GNU") ) )
            message("Optimization is enabled, but architecture is not specified. Assuming native build")
            set(OVERTE_CPU_ARCHITECTURE "-march=native -mtune=native" CACHE STRING "Specify architecture dependent compiler flags here")
        endif()
    endif()

    if(DEFINED OVERTE_CPU_ARCHITECTURE)
        set(OVERTE_OPTIMIZE_FLAGS "${OVERTE_OPTIMIZE_FLAGS} ${OVERTE_CPU_ARCHITECTURE}")
        message("Adding CPU architecture flags: ${OVERTE_CPU_ARCHITECTURE}")
        MESSAGE(STATUS "OVERTE_CPU_ARCHITECTURE: ${OVERTE_CPU_ARCHITECTURE}")
    endif()

    # Function alignment is necessary for V8.
    # SetAlignedPointerInInternalField requires at least 2 byte alignment and -falign-functions will set alignment
    # to machine specific value which should be greater than 2.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OVERTE_OPTIMIZE_FLAGS} -falign-functions")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OVERTE_OPTIMIZE_FLAGS} -falign-functions")

    set(ENV{CXXFLAGS} "$ENV{CXXFLAGS} ${OVERTE_OPTIMIZE_FLAGS} -falign-functions")
    set(ENV{CFLAGS} "$ENV{CFLAGS} ${OVERTE_OPTIMIZE_FLAGS} -falign-functions")
    message($ENV{CXXFLAGS})
endif()

# OVERTE_WARNINGS
#
# Here we add the ability to allowlist warnings we've determined we can't fix, or are safe to
# ignore for one reason or another. The way of doing so is compiler-specific, so we deal with
# the detection of that in cmake, and just pass it down to the code from here.
#
# We can also treat warnings as errors. Without the allowlist this will almost certainly lead
# to a build failure.

if(NOT DEFINED OVERTE_WARNINGS_ALLOWLIST)
    set(OVERTE_WARNINGS_ALLOWLIST true CACHE BOOL "Allowlist some warnings we can't currently fix")
endif()

if(NOT DEFINED OVERTE_WARNINGS_AS_ERRORS)
    set(OVERTE_WARNINGS_AS_ERRORS false CACHE BOOL "Count warnings as errors")
endif()

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

if(OVERTE_WARNINGS_AS_ERRORS)
    if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC" OR (CMAKE_CXX_COMPILER_ID MATCHES "" AND WIN32))
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
      set(CMAKE_CFLAGS "${CMAKE_CFLAGS} /WX")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
      set(CMAKE_CFLAGS "${CMAKE_CFLAGS} -Werror")
    endif()
endif()


project(overte)
include("cmake/init.cmake")
include("cmake/compiler.cmake")

if (NOT DEFINED CLIENT_ONLY)
  set(CLIENT_ONLY 0)
endif()

if (NOT DEFINED SERVER_ONLY)
  set(SERVER_ONLY 0)
endif()

if (ANDROID OR UWP)
  set(MOBILE 1)
else()
  set(MOBILE 0)
endif()



# Use default time server if none defined in environment
set_from_env(TIMESERVER_URL TIMESERVER_URL "http://timestamp.comodoca.com?td=sha256")

set(HIFI_USE_OPTIMIZED_IK_OPTION OFF)
set(BUILD_CLIENT_OPTION ON)
set(BUILD_SERVER_OPTION ON)
set(BUILD_TESTS_OPTION OFF)
set(BUILD_MANUAL_TESTS_OPTION ${BUILD_TESTS_OPTION})
set(BUILD_TOOLS_OPTION ON)
set(BUILD_INSTALLER_OPTION ON)
set(DISABLE_QML_OPTION OFF)
set(DOWNLOAD_SERVERLESS_CONTENT_OPTION OFF)

if (ANDROID OR UWP)
  set(BUILD_SERVER_OPTION OFF)
  set(BUILD_TOOLS_OPTION OFF)
  set(BUILD_INSTALLER OFF)
endif()

if (CLIENT_ONLY)
  set(BUILD_SERVER_OPTION OFF)
endif()

if (SERVER_ONLY)
  set(BUILD_CLIENT_OPTION OFF)
  set(BUILD_TESTS_OPTION OFF)
endif()

if (ANDROID)
  set(GLES_OPTION ON)
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
endif ()

if (USE_GLES AND (NOT ANDROID AND NOT UNIX))
  set(DISABLE_QML_OPTION ON)
endif()

option(HIFI_USE_OPTIMIZED_IK "Use optimized IK" ${HIFI_USE_OPTIMIZED_IK_OPTION})
option(BUILD_CLIENT "Build client components" ${BUILD_CLIENT_OPTION})
option(BUILD_SERVER "Build server components" ${BUILD_SERVER_OPTION})
option(BUILD_TESTS "Build tests" ${BUILD_TESTS_OPTION})
option(BUILD_MANUAL_TESTS "Build manual tests" ${BUILD_MANUAL_TESTS_OPTION})
option(BUILD_TOOLS "Build tools" ${BUILD_TOOLS_OPTION})
option(BUILD_INSTALLER "Build installer" ${BUILD_INSTALLER_OPTION})
option(USE_GLES "Use OpenGL ES" ${GLES_OPTION})
option(USE_KHR_ROBUSTNESS "Use KHR_robustness" OFF)
option(DISABLE_QML "Disable QML" ${DISABLE_QML_OPTION})
option(DISABLE_KTX_CACHE "Disable KTX Cache" OFF)
option(
  DOWNLOAD_SERVERLESS_CONTENT
  "Download and setup default serverless content beside Interface"
  ${DOWNLOAD_SERVERLESS_CONTENT_OPTION}
)

set(PLATFORM_QT_GL OpenGL)

if (USE_KHR_ROBUSTNESS)
  add_definitions(-DUSE_KHR_ROBUSTNESS)
endif()

if (USE_GLES)
  add_definitions(-DUSE_GLES)
  add_definitions(-DGPU_POINTER_STORAGE_SHARED)
  set(PLATFORM_GL_BACKEND gpu-gl-common gpu-gles)
else()
  add_definitions(-DGPU_POINTER_STORAGE_RAW)
  set(PLATFORM_GL_BACKEND gpu-gl-common gpu-gl)
endif()

foreach(PLATFORM_QT_COMPONENT ${PLATFORM_QT_COMPONENTS})
  list(APPEND PLATFORM_QT_LIBRARIES "Qt5::${PLATFORM_QT_COMPONENT}")
endforeach()

MESSAGE(STATUS "Use optimized IK:      " ${HIFI_USE_OPTIMIZED_IK})
MESSAGE(STATUS "Build server:          " ${BUILD_SERVER})
MESSAGE(STATUS "Build client:          " ${BUILD_CLIENT})
MESSAGE(STATUS "Build tests:           " ${BUILD_TESTS})
MESSAGE(STATUS "Build tools:           " ${BUILD_TOOLS})
MESSAGE(STATUS "Build installer:       " ${BUILD_INSTALLER})
MESSAGE(STATUS "GL ES:                 " ${USE_GLES})
MESSAGE(STATUS "DL serverless content: " ${DOWNLOAD_SERVERLESS_CONTENT})

if (DISABLE_QML)
  MESSAGE(STATUS "QML disabled!")
  add_definitions(-DDISABLE_QML)
endif()

if (DISABLE_KTX_CACHE)
  MESSAGE(STATUS "KTX cache disabled!")
  add_definitions(-DDISABLE_KTX_CACHE)
endif()

if (UNIX AND DEFINED ENV{HIFI_MEMORY_DEBUGGING})
  MESSAGE(STATUS "Memory debugging is enabled")
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

if (WIN32)
    # Deal with fakakta Visual Studo 2017 bug
    add_definitions(-DQT_NO_FLOAT16_OPERATORS -DWIN32)
endif()

if (HIFI_USE_OPTIMIZED_IK)
  MESSAGE(STATUS "SET THE USE IK DEFINITION ")
  add_definitions(-DHIFI_USE_OPTIMIZED_IK)
endif()
set(HIFI_LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libraries")

set(EXTERNAL_PROJECT_PREFIX "project")
set_property(DIRECTORY PROPERTY EP_PREFIX ${EXTERNAL_PROJECT_PREFIX})
setup_externals_binary_dir()

option(USE_NSIGHT "Attempt to find the nSight libraries" 1)

# FIXME hack to work on the proper Android toolchain
if (ANDROID)
  add_subdirectory(android/apps/${HIFI_ANDROID_APP})
  return()
endif()

if (BUILD_GPU_FRAME_PLAYER_ONLY)
    # This is for CI build testing
    add_subdirectory(tools/gpu-frame-player)
else()

# BUILD_TOOLS option will be handled inside the tools's CMakeLists.txt because 'scribe' tool is required for build anyway
add_subdirectory(tools)

# add subdirectories for all targets
if (BUILD_SERVER)
  add_subdirectory(assignment-client)
  set_target_properties(assignment-client PROPERTIES FOLDER "Apps")
  add_subdirectory(domain-server)
  set_target_properties(domain-server PROPERTIES FOLDER "Apps")
  add_subdirectory(ice-server)
  set_target_properties(ice-server PROPERTIES FOLDER "Apps")
endif()

if (BUILD_CLIENT)
  add_subdirectory(interface)
  if (APPLE)
    set_target_properties(Overte PROPERTIES FOLDER "Apps")
  else()
    set_target_properties(interface PROPERTIES FOLDER "Apps")
  endif()

  option(USE_SIXENSE "Build Interface with sixense library/plugin" OFF)
  option(USE_NEURON "Build Interface with Neuron library/plugin" OFF)
endif()

if (BUILD_CLIENT OR BUILD_SERVER)
  add_subdirectory(plugins)
  add_subdirectory(server-console)
endif()

endif()

if (BUILD_TESTS)
  # Turn on testing so that add_test works
  # MUST be in the root cmake file for ctest to work
  include(CTest)
  enable_testing()
  add_subdirectory(tests)
  if (BUILD_MANUAL_TESTS)
    add_subdirectory(tests-manual)
  endif()
endif()

if (BUILD_INSTALLER)
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
