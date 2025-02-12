# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(xkeyboard-config_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(xkeyboard-config_FRAMEWORKS_FOUND_RELEASE "${xkeyboard-config_FRAMEWORKS_RELEASE}" "${xkeyboard-config_FRAMEWORK_DIRS_RELEASE}")

set(xkeyboard-config_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET xkeyboard-config_DEPS_TARGET)
    add_library(xkeyboard-config_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET xkeyboard-config_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${xkeyboard-config_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${xkeyboard-config_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### xkeyboard-config_DEPS_TARGET to all of them
conan_package_library_targets("${xkeyboard-config_LIBS_RELEASE}"    # libraries
                              "${xkeyboard-config_LIB_DIRS_RELEASE}" # package_libdir
                              "${xkeyboard-config_BIN_DIRS_RELEASE}" # package_bindir
                              "${xkeyboard-config_LIBRARY_TYPE_RELEASE}"
                              "${xkeyboard-config_IS_HOST_WINDOWS_RELEASE}"
                              xkeyboard-config_DEPS_TARGET
                              xkeyboard-config_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "xkeyboard-config"    # package_name
                              "${xkeyboard-config_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${xkeyboard-config_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Release ########################################
    set_property(TARGET xkeyboard-config::xkeyboard-config
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Release>:${xkeyboard-config_OBJECTS_RELEASE}>
                 $<$<CONFIG:Release>:${xkeyboard-config_LIBRARIES_TARGETS}>
                 )

    if("${xkeyboard-config_LIBS_RELEASE}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET xkeyboard-config::xkeyboard-config
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     xkeyboard-config_DEPS_TARGET)
    endif()

    set_property(TARGET xkeyboard-config::xkeyboard-config
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Release>:${xkeyboard-config_LINKER_FLAGS_RELEASE}>)
    set_property(TARGET xkeyboard-config::xkeyboard-config
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Release>:${xkeyboard-config_INCLUDE_DIRS_RELEASE}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET xkeyboard-config::xkeyboard-config
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Release>:${xkeyboard-config_LIB_DIRS_RELEASE}>)
    set_property(TARGET xkeyboard-config::xkeyboard-config
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Release>:${xkeyboard-config_COMPILE_DEFINITIONS_RELEASE}>)
    set_property(TARGET xkeyboard-config::xkeyboard-config
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Release>:${xkeyboard-config_COMPILE_OPTIONS_RELEASE}>)

########## For the modules (FindXXX)
set(xkeyboard-config_LIBRARIES_RELEASE xkeyboard-config::xkeyboard-config)
