########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(Fontconfig_FIND_QUIETLY)
    set(Fontconfig_MESSAGE_MODE VERBOSE)
else()
    set(Fontconfig_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/module-FontconfigTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${fontconfig_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(Fontconfig_VERSION_STRING "2.15.0")
set(Fontconfig_INCLUDE_DIRS ${fontconfig_INCLUDE_DIRS_RELEASE} )
set(Fontconfig_INCLUDE_DIR ${fontconfig_INCLUDE_DIRS_RELEASE} )
set(Fontconfig_LIBRARIES ${fontconfig_LIBRARIES_RELEASE} )
set(Fontconfig_DEFINITIONS ${fontconfig_DEFINITIONS_RELEASE} )


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${fontconfig_BUILD_MODULES_PATHS_RELEASE} )
    message(${Fontconfig_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


include(FindPackageHandleStandardArgs)
set(Fontconfig_FOUND 1)
set(Fontconfig_VERSION "2.15.0")

find_package_handle_standard_args(Fontconfig
                                  REQUIRED_VARS Fontconfig_VERSION
                                  VERSION_VAR Fontconfig_VERSION)
mark_as_advanced(Fontconfig_FOUND Fontconfig_VERSION)
