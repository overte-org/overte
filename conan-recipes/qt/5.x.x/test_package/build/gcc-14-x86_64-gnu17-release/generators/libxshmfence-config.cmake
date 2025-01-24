########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(libxshmfence_FIND_QUIETLY)
    set(libxshmfence_MESSAGE_MODE VERBOSE)
else()
    set(libxshmfence_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/libxshmfenceTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${libxshmfence_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(libxshmfence_VERSION_STRING "1.3")
set(libxshmfence_INCLUDE_DIRS ${libxshmfence_INCLUDE_DIRS_RELEASE} )
set(libxshmfence_INCLUDE_DIR ${libxshmfence_INCLUDE_DIRS_RELEASE} )
set(libxshmfence_LIBRARIES ${libxshmfence_LIBRARIES_RELEASE} )
set(libxshmfence_DEFINITIONS ${libxshmfence_DEFINITIONS_RELEASE} )


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${libxshmfence_BUILD_MODULES_PATHS_RELEASE} )
    message(${libxshmfence_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


