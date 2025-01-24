# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(fontconfig_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(fontconfig_FRAMEWORKS_FOUND_RELEASE "${fontconfig_FRAMEWORKS_RELEASE}" "${fontconfig_FRAMEWORK_DIRS_RELEASE}")

set(fontconfig_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET fontconfig_DEPS_TARGET)
    add_library(fontconfig_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET fontconfig_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${fontconfig_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${fontconfig_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### fontconfig_DEPS_TARGET to all of them
conan_package_library_targets("${fontconfig_LIBS_RELEASE}"    # libraries
                              "${fontconfig_LIB_DIRS_RELEASE}" # package_libdir
                              "${fontconfig_BIN_DIRS_RELEASE}" # package_bindir
                              "${fontconfig_LIBRARY_TYPE_RELEASE}"
                              "${fontconfig_IS_HOST_WINDOWS_RELEASE}"
                              fontconfig_DEPS_TARGET
                              fontconfig_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "fontconfig"    # package_name
                              "${fontconfig_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${fontconfig_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Release ########################################
    set_property(TARGET Fontconfig::Fontconfig
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Release>:${fontconfig_OBJECTS_RELEASE}>
                 $<$<CONFIG:Release>:${fontconfig_LIBRARIES_TARGETS}>
                 )

    if("${fontconfig_LIBS_RELEASE}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET Fontconfig::Fontconfig
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     fontconfig_DEPS_TARGET)
    endif()

    set_property(TARGET Fontconfig::Fontconfig
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Release>:${fontconfig_LINKER_FLAGS_RELEASE}>)
    set_property(TARGET Fontconfig::Fontconfig
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Release>:${fontconfig_INCLUDE_DIRS_RELEASE}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET Fontconfig::Fontconfig
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Release>:${fontconfig_LIB_DIRS_RELEASE}>)
    set_property(TARGET Fontconfig::Fontconfig
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Release>:${fontconfig_COMPILE_DEFINITIONS_RELEASE}>)
    set_property(TARGET Fontconfig::Fontconfig
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Release>:${fontconfig_COMPILE_OPTIONS_RELEASE}>)

########## For the modules (FindXXX)
set(fontconfig_LIBRARIES_RELEASE Fontconfig::Fontconfig)
