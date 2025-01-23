# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(nspr_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(nspr_FRAMEWORKS_FOUND_RELEASE "${nspr_FRAMEWORKS_RELEASE}" "${nspr_FRAMEWORK_DIRS_RELEASE}")

set(nspr_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET nspr_DEPS_TARGET)
    add_library(nspr_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET nspr_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${nspr_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${nspr_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### nspr_DEPS_TARGET to all of them
conan_package_library_targets("${nspr_LIBS_RELEASE}"    # libraries
                              "${nspr_LIB_DIRS_RELEASE}" # package_libdir
                              "${nspr_BIN_DIRS_RELEASE}" # package_bindir
                              "${nspr_LIBRARY_TYPE_RELEASE}"
                              "${nspr_IS_HOST_WINDOWS_RELEASE}"
                              nspr_DEPS_TARGET
                              nspr_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "nspr"    # package_name
                              "${nspr_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${nspr_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Release ########################################
    set_property(TARGET nspr::nspr
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Release>:${nspr_OBJECTS_RELEASE}>
                 $<$<CONFIG:Release>:${nspr_LIBRARIES_TARGETS}>
                 )

    if("${nspr_LIBS_RELEASE}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET nspr::nspr
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     nspr_DEPS_TARGET)
    endif()

    set_property(TARGET nspr::nspr
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Release>:${nspr_LINKER_FLAGS_RELEASE}>)
    set_property(TARGET nspr::nspr
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Release>:${nspr_INCLUDE_DIRS_RELEASE}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET nspr::nspr
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Release>:${nspr_LIB_DIRS_RELEASE}>)
    set_property(TARGET nspr::nspr
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Release>:${nspr_COMPILE_DEFINITIONS_RELEASE}>)
    set_property(TARGET nspr::nspr
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Release>:${nspr_COMPILE_OPTIONS_RELEASE}>)

########## For the modules (FindXXX)
set(nspr_LIBRARIES_RELEASE nspr::nspr)
