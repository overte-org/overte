########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(autoconf_FIND_QUIETLY)
    set(autoconf_MESSAGE_MODE VERBOSE)
else()
    set(autoconf_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/autoconfTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${autoconf_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(autoconf_VERSION_STRING "2.71")
set(autoconf_INCLUDE_DIRS ${autoconf_INCLUDE_DIRS_RELEASE} )
set(autoconf_INCLUDE_DIR ${autoconf_INCLUDE_DIRS_RELEASE} )
set(autoconf_LIBRARIES ${autoconf_LIBRARIES_RELEASE} )
set(autoconf_DEFINITIONS ${autoconf_DEFINITIONS_RELEASE} )


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${autoconf_BUILD_MODULES_PATHS_RELEASE} )
    message(${autoconf_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


