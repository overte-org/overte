########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(nss_FIND_QUIETLY)
    set(nss_MESSAGE_MODE VERBOSE)
else()
    set(nss_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/nssTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${nss_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(nss_VERSION_STRING "3.107")
set(nss_INCLUDE_DIRS ${nss_INCLUDE_DIRS_RELEASE} )
set(nss_INCLUDE_DIR ${nss_INCLUDE_DIRS_RELEASE} )
set(nss_LIBRARIES ${nss_LIBRARIES_RELEASE} )
set(nss_DEFINITIONS ${nss_DEFINITIONS_RELEASE} )


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${nss_BUILD_MODULES_PATHS_RELEASE} )
    message(${nss_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


