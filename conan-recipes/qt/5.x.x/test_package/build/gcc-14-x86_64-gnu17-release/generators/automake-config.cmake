########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(automake_FIND_QUIETLY)
    set(automake_MESSAGE_MODE VERBOSE)
else()
    set(automake_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/automakeTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${automake_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(automake_VERSION_STRING "1.16.5")
set(automake_INCLUDE_DIRS ${automake_INCLUDE_DIRS_RELEASE} )
set(automake_INCLUDE_DIR ${automake_INCLUDE_DIRS_RELEASE} )
set(automake_LIBRARIES ${automake_LIBRARIES_RELEASE} )
set(automake_DEFINITIONS ${automake_DEFINITIONS_RELEASE} )


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${automake_BUILD_MODULES_PATHS_RELEASE} )
    message(${automake_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


