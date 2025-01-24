########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(xkbcommon_FIND_QUIETLY)
    set(xkbcommon_MESSAGE_MODE VERBOSE)
else()
    set(xkbcommon_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/xkbcommonTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${xkbcommon_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(xkbcommon_VERSION_STRING "1.5.0")
set(xkbcommon_INCLUDE_DIRS ${xkbcommon_INCLUDE_DIRS_RELEASE} )
set(xkbcommon_INCLUDE_DIR ${xkbcommon_INCLUDE_DIRS_RELEASE} )
set(xkbcommon_LIBRARIES ${xkbcommon_LIBRARIES_RELEASE} )
set(xkbcommon_DEFINITIONS ${xkbcommon_DEFINITIONS_RELEASE} )


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${xkbcommon_BUILD_MODULES_PATHS_RELEASE} )
    message(${xkbcommon_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


