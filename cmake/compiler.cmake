set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

if (NOT "${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
  message( FATAL_ERROR "Only 64 bit builds supported." )
endif()

if (WIN32)
  add_definitions(-DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)

  if (NOT WINDOW_SDK_PATH)
    set(DEBUG_DISCOVERED_SDK_PATH TRUE)
  endif()

  # sets path for Microsoft SDKs
  # if you get build error about missing 'glu32' this path is likely wrong
  if (MSVC_VERSION GREATER_EQUAL 1910) # VS 2017
    set(WINDOW_SDK_PATH "C:/Program Files (x86)/Windows Kits/10/Lib/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/x64" CACHE PATH "Windows SDK PATH")
  else()
    message( FATAL_ERROR "Visual Studio 2017 or higher required." )
  endif()

  if (DEBUG_DISCOVERED_SDK_PATH)
    message(STATUS "The discovered Windows SDK path is ${WINDOW_SDK_PATH}")
  endif ()

  list(APPEND CMAKE_PREFIX_PATH "${WINDOW_SDK_PATH}")

  # /wd4351 disables warning C4351: new behavior: elements of array will be default initialized
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP${HIFI_MAX_BUILD_CORES} /wd4351")
  # /LARGEADDRESSAWARE enables 32-bit apps to use more than 2GB of memory.
  # Caveats: http://stackoverflow.com/questions/2288728/drawbacks-of-using-largeaddressaware-for-32-bit-windows-executables
  # TODO: Remove when building 64-bit.
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE")
  # always produce symbols as PDB files
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG /OPT:REF /OPT:ICF")
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fno-strict-aliasing -Wno-unused-parameter")
  if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -Woverloaded-virtual -Wdouble-promotion -Wsuggest-override")
  endif ()
endif()

if ((APPLE) AND (CMAKE_GENERATOR STREQUAL "Xcode"))
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
  set(CMAKE_XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS[variant=Release] "YES")
  set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT[variant=Release] "dwarf-with-dsym")
  set(CMAKE_XCODE_ATTRIBUTE_DEPLOYMENT_POSTPROCESSING[variant=Release] "YES")
endif()


if (NOT ANDROID_LIB_DIR)
  set(ANDROID_LIB_DIR $ENV{ANDROID_LIB_DIR})
endif ()

if (APPLE)
  exec_program(sw_vers ARGS -productVersion  OUTPUT_VARIABLE OSX_VERSION)
  string(REGEX MATCH "^[0-9]+\\.[0-9]+" OSX_VERSION ${OSX_VERSION})
  message(STATUS "Detected OS X version = ${OSX_VERSION}")
  message(STATUS "OS X deployment target = ${CMAKE_OSX_DEPLOYMENT_TARGET}")

  set(OSX_SDK "${OSX_VERSION}" CACHE STRING "OS X SDK version to look for inside Xcode bundle or at OSX_SDK_PATH")

  # find the SDK path for the desired SDK
  find_path(
    _OSX_DESIRED_SDK_PATH
    NAME MacOSX${OSX_SDK}.sdk
    HINTS ${OSX_SDK_PATH}
    PATHS /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/
          /Applications/Xcode-beta.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/
  )

  if (NOT _OSX_DESIRED_SDK_PATH)
    message(STATUS "Could not find OS X ${OSX_SDK} SDK. Will fall back to default. If you want a specific SDK, please pass OSX_SDK and optionally OSX_SDK_PATH to CMake.")
  else ()
    message(STATUS "Found OS X ${OSX_SDK} SDK at ${_OSX_DESIRED_SDK_PATH}/MacOSX${OSX_SDK}.sdk")

    # set that as the SDK to use
    set(CMAKE_OSX_SYSROOT ${_OSX_DESIRED_SDK_PATH}/MacOSX${OSX_SDK}.sdk)
  endif ()
endif ()

message("CXXFLAGS: ${CXXFLAGS}")
