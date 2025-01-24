########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(libtool_FIND_QUIETLY)
    set(libtool_MESSAGE_MODE VERBOSE)
else()
    set(libtool_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/libtoolTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${libtool_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(libtool_VERSION_STRING "2.4.7")
set(libtool_INCLUDE_DIRS ${libtool_INCLUDE_DIRS_RELEASE} )
set(libtool_INCLUDE_DIR ${libtool_INCLUDE_DIRS_RELEASE} )
set(libtool_LIBRARIES ${libtool_LIBRARIES_RELEASE} )
set(libtool_DEFINITIONS ${libtool_DEFINITIONS_RELEASE} )


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${libtool_BUILD_MODULES_PATHS_RELEASE} )
    message(${libtool_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


