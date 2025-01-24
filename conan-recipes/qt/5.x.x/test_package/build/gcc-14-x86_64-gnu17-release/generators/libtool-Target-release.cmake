# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(libtool_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(libtool_FRAMEWORKS_FOUND_RELEASE "${libtool_FRAMEWORKS_RELEASE}" "${libtool_FRAMEWORK_DIRS_RELEASE}")

set(libtool_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET libtool_DEPS_TARGET)
    add_library(libtool_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET libtool_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${libtool_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${libtool_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:automake::automake>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### libtool_DEPS_TARGET to all of them
conan_package_library_targets("${libtool_LIBS_RELEASE}"    # libraries
                              "${libtool_LIB_DIRS_RELEASE}" # package_libdir
                              "${libtool_BIN_DIRS_RELEASE}" # package_bindir
                              "${libtool_LIBRARY_TYPE_RELEASE}"
                              "${libtool_IS_HOST_WINDOWS_RELEASE}"
                              libtool_DEPS_TARGET
                              libtool_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "libtool"    # package_name
                              "${libtool_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${libtool_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Release ########################################
    set_property(TARGET libtool::libtool
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Release>:${libtool_OBJECTS_RELEASE}>
                 $<$<CONFIG:Release>:${libtool_LIBRARIES_TARGETS}>
                 )

    if("${libtool_LIBS_RELEASE}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET libtool::libtool
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     libtool_DEPS_TARGET)
    endif()

    set_property(TARGET libtool::libtool
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Release>:${libtool_LINKER_FLAGS_RELEASE}>)
    set_property(TARGET libtool::libtool
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Release>:${libtool_INCLUDE_DIRS_RELEASE}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET libtool::libtool
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Release>:${libtool_LIB_DIRS_RELEASE}>)
    set_property(TARGET libtool::libtool
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Release>:${libtool_COMPILE_DEFINITIONS_RELEASE}>)
    set_property(TARGET libtool::libtool
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Release>:${libtool_COMPILE_OPTIONS_RELEASE}>)

########## For the modules (FindXXX)
set(libtool_LIBRARIES_RELEASE libtool::libtool)
