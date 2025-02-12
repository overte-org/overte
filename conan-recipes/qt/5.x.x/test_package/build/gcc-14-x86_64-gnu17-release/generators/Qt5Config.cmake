########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(Qt5_FIND_QUIETLY)
    set(Qt5_MESSAGE_MODE VERBOSE)
else()
    set(Qt5_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/Qt5Targets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${qt_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(Qt5_VERSION_STRING "5.15.16-2025.01.23")
set(Qt5_INCLUDE_DIRS ${qt_INCLUDE_DIRS_RELEASE} )
set(Qt5_INCLUDE_DIR ${qt_INCLUDE_DIRS_RELEASE} )
set(Qt5_LIBRARIES ${qt_LIBRARIES_RELEASE} )
set(Qt5_DEFINITIONS ${qt_DEFINITIONS_RELEASE} )


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${qt_BUILD_MODULES_PATHS_RELEASE} )
    message(${Qt5_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


