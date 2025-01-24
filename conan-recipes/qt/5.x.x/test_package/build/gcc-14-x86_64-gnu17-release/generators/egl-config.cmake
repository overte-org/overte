########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(egl_FIND_QUIETLY)
    set(egl_MESSAGE_MODE VERBOSE)
else()
    set(egl_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/eglTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${egl_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(egl_VERSION_STRING "system")
set(egl_INCLUDE_DIRS ${egl_INCLUDE_DIRS_RELEASE} )
set(egl_INCLUDE_DIR ${egl_INCLUDE_DIRS_RELEASE} )
set(egl_LIBRARIES ${egl_LIBRARIES_RELEASE} )
set(egl_DEFINITIONS ${egl_DEFINITIONS_RELEASE} )


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${egl_BUILD_MODULES_PATHS_RELEASE} )
    message(${egl_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


