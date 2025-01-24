# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(automake_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(automake_FRAMEWORKS_FOUND_RELEASE "${automake_FRAMEWORKS_RELEASE}" "${automake_FRAMEWORK_DIRS_RELEASE}")

set(automake_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET automake_DEPS_TARGET)
    add_library(automake_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET automake_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${automake_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${automake_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:autoconf::autoconf>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### automake_DEPS_TARGET to all of them
conan_package_library_targets("${automake_LIBS_RELEASE}"    # libraries
                              "${automake_LIB_DIRS_RELEASE}" # package_libdir
                              "${automake_BIN_DIRS_RELEASE}" # package_bindir
                              "${automake_LIBRARY_TYPE_RELEASE}"
                              "${automake_IS_HOST_WINDOWS_RELEASE}"
                              automake_DEPS_TARGET
                              automake_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "automake"    # package_name
                              "${automake_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${automake_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Release ########################################
    set_property(TARGET automake::automake
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Release>:${automake_OBJECTS_RELEASE}>
                 $<$<CONFIG:Release>:${automake_LIBRARIES_TARGETS}>
                 )

    if("${automake_LIBS_RELEASE}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET automake::automake
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     automake_DEPS_TARGET)
    endif()

    set_property(TARGET automake::automake
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Release>:${automake_LINKER_FLAGS_RELEASE}>)
    set_property(TARGET automake::automake
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Release>:${automake_INCLUDE_DIRS_RELEASE}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET automake::automake
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Release>:${automake_LIB_DIRS_RELEASE}>)
    set_property(TARGET automake::automake
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Release>:${automake_COMPILE_DEFINITIONS_RELEASE}>)
    set_property(TARGET automake::automake
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Release>:${automake_COMPILE_OPTIONS_RELEASE}>)

########## For the modules (FindXXX)
set(automake_LIBRARIES_RELEASE automake::automake)
