########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(ODBC_FIND_QUIETLY)
    set(ODBC_MESSAGE_MODE VERBOSE)
else()
    set(ODBC_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/module-ODBCTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${odbc_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(ODBC_VERSION_STRING "2.3.11")
set(ODBC_INCLUDE_DIRS ${odbc_INCLUDE_DIRS_RELEASE} )
set(ODBC_INCLUDE_DIR ${odbc_INCLUDE_DIRS_RELEASE} )
set(ODBC_LIBRARIES ${odbc_LIBRARIES_RELEASE} )
set(ODBC_DEFINITIONS ${odbc_DEFINITIONS_RELEASE} )


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${odbc_BUILD_MODULES_PATHS_RELEASE} )
    message(${ODBC_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


include(FindPackageHandleStandardArgs)
set(ODBC_FOUND 1)
set(ODBC_VERSION "2.3.11")

find_package_handle_standard_args(ODBC
                                  REQUIRED_VARS ODBC_VERSION
                                  VERSION_VAR ODBC_VERSION)
mark_as_advanced(ODBC_FOUND ODBC_VERSION)
