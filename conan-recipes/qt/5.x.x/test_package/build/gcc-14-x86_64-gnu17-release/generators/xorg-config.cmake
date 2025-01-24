########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(xorg_FIND_QUIETLY)
    set(xorg_MESSAGE_MODE VERBOSE)
else()
    set(xorg_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/xorgTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${xorg_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(xorg_VERSION_STRING "system")
set(xorg_INCLUDE_DIRS ${xorg_INCLUDE_DIRS_RELEASE} )
set(xorg_INCLUDE_DIR ${xorg_INCLUDE_DIRS_RELEASE} )
set(xorg_LIBRARIES ${xorg_LIBRARIES_RELEASE} )
set(xorg_DEFINITIONS ${xorg_DEFINITIONS_RELEASE} )


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${xorg_BUILD_MODULES_PATHS_RELEASE} )
    message(${xorg_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


