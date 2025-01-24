# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(autoconf_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(autoconf_FRAMEWORKS_FOUND_RELEASE "${autoconf_FRAMEWORKS_RELEASE}" "${autoconf_FRAMEWORK_DIRS_RELEASE}")

set(autoconf_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET autoconf_DEPS_TARGET)
    add_library(autoconf_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET autoconf_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${autoconf_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${autoconf_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:m4::m4>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### autoconf_DEPS_TARGET to all of them
conan_package_library_targets("${autoconf_LIBS_RELEASE}"    # libraries
                              "${autoconf_LIB_DIRS_RELEASE}" # package_libdir
                              "${autoconf_BIN_DIRS_RELEASE}" # package_bindir
                              "${autoconf_LIBRARY_TYPE_RELEASE}"
                              "${autoconf_IS_HOST_WINDOWS_RELEASE}"
                              autoconf_DEPS_TARGET
                              autoconf_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "autoconf"    # package_name
                              "${autoconf_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${autoconf_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Release ########################################
    set_property(TARGET autoconf::autoconf
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Release>:${autoconf_OBJECTS_RELEASE}>
                 $<$<CONFIG:Release>:${autoconf_LIBRARIES_TARGETS}>
                 )

    if("${autoconf_LIBS_RELEASE}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET autoconf::autoconf
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     autoconf_DEPS_TARGET)
    endif()

    set_property(TARGET autoconf::autoconf
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Release>:${autoconf_LINKER_FLAGS_RELEASE}>)
    set_property(TARGET autoconf::autoconf
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Release>:${autoconf_INCLUDE_DIRS_RELEASE}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET autoconf::autoconf
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Release>:${autoconf_LIB_DIRS_RELEASE}>)
    set_property(TARGET autoconf::autoconf
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Release>:${autoconf_COMPILE_DEFINITIONS_RELEASE}>)
    set_property(TARGET autoconf::autoconf
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Release>:${autoconf_COMPILE_OPTIONS_RELEASE}>)

########## For the modules (FindXXX)
set(autoconf_LIBRARIES_RELEASE autoconf::autoconf)
