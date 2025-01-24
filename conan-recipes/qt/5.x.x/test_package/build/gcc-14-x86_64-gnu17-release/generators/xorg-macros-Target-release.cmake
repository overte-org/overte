# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(xorg-macros_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(xorg-macros_FRAMEWORKS_FOUND_RELEASE "${xorg-macros_FRAMEWORKS_RELEASE}" "${xorg-macros_FRAMEWORK_DIRS_RELEASE}")

set(xorg-macros_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET xorg-macros_DEPS_TARGET)
    add_library(xorg-macros_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET xorg-macros_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${xorg-macros_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${xorg-macros_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### xorg-macros_DEPS_TARGET to all of them
conan_package_library_targets("${xorg-macros_LIBS_RELEASE}"    # libraries
                              "${xorg-macros_LIB_DIRS_RELEASE}" # package_libdir
                              "${xorg-macros_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-macros_LIBRARY_TYPE_RELEASE}"
                              "${xorg-macros_IS_HOST_WINDOWS_RELEASE}"
                              xorg-macros_DEPS_TARGET
                              xorg-macros_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "xorg-macros"    # package_name
                              "${xorg-macros_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${xorg-macros_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Release ########################################
    set_property(TARGET xorg-macros::xorg-macros
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Release>:${xorg-macros_OBJECTS_RELEASE}>
                 $<$<CONFIG:Release>:${xorg-macros_LIBRARIES_TARGETS}>
                 )

    if("${xorg-macros_LIBS_RELEASE}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET xorg-macros::xorg-macros
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     xorg-macros_DEPS_TARGET)
    endif()

    set_property(TARGET xorg-macros::xorg-macros
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Release>:${xorg-macros_LINKER_FLAGS_RELEASE}>)
    set_property(TARGET xorg-macros::xorg-macros
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Release>:${xorg-macros_INCLUDE_DIRS_RELEASE}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET xorg-macros::xorg-macros
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Release>:${xorg-macros_LIB_DIRS_RELEASE}>)
    set_property(TARGET xorg-macros::xorg-macros
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Release>:${xorg-macros_COMPILE_DEFINITIONS_RELEASE}>)
    set_property(TARGET xorg-macros::xorg-macros
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Release>:${xorg-macros_COMPILE_OPTIONS_RELEASE}>)

########## For the modules (FindXXX)
set(xorg-macros_LIBRARIES_RELEASE xorg-macros::xorg-macros)
