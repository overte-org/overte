########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(xorg-proto_FIND_QUIETLY)
    set(xorg-proto_MESSAGE_MODE VERBOSE)
else()
    set(xorg-proto_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/xorg-protoTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${xorg-proto_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(xorg-proto_VERSION_STRING "2022.2")
set(xorg-proto_INCLUDE_DIRS ${xorg-proto_INCLUDE_DIRS_RELEASE} )
set(xorg-proto_INCLUDE_DIR ${xorg-proto_INCLUDE_DIRS_RELEASE} )
set(xorg-proto_LIBRARIES ${xorg-proto_LIBRARIES_RELEASE} )
set(xorg-proto_DEFINITIONS ${xorg-proto_DEFINITIONS_RELEASE} )


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${xorg-proto_BUILD_MODULES_PATHS_RELEASE} )
    message(${xorg-proto_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


