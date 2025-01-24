########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(m4_FIND_QUIETLY)
    set(m4_MESSAGE_MODE VERBOSE)
else()
    set(m4_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/m4Targets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${m4_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(m4_VERSION_STRING "1.4.19")
set(m4_INCLUDE_DIRS ${m4_INCLUDE_DIRS_RELEASE} )
set(m4_INCLUDE_DIR ${m4_INCLUDE_DIRS_RELEASE} )
set(m4_LIBRARIES ${m4_LIBRARIES_RELEASE} )
set(m4_DEFINITIONS ${m4_DEFINITIONS_RELEASE} )


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${m4_BUILD_MODULES_PATHS_RELEASE} )
    message(${m4_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


