########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(nspr_FIND_QUIETLY)
    set(nspr_MESSAGE_MODE VERBOSE)
else()
    set(nspr_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/nsprTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${nspr_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(nspr_VERSION_STRING "4.35")
set(nspr_INCLUDE_DIRS ${nspr_INCLUDE_DIRS_RELEASE} )
set(nspr_INCLUDE_DIR ${nspr_INCLUDE_DIRS_RELEASE} )
set(nspr_LIBRARIES ${nspr_LIBRARIES_RELEASE} )
set(nspr_DEFINITIONS ${nspr_DEFINITIONS_RELEASE} )


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${nspr_BUILD_MODULES_PATHS_RELEASE} )
    message(${nspr_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


