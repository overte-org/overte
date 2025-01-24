# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(libxshmfence_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(libxshmfence_FRAMEWORKS_FOUND_RELEASE "${libxshmfence_FRAMEWORKS_RELEASE}" "${libxshmfence_FRAMEWORK_DIRS_RELEASE}")

set(libxshmfence_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET libxshmfence_DEPS_TARGET)
    add_library(libxshmfence_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET libxshmfence_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${libxshmfence_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${libxshmfence_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:xorg-proto::xorg-proto>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### libxshmfence_DEPS_TARGET to all of them
conan_package_library_targets("${libxshmfence_LIBS_RELEASE}"    # libraries
                              "${libxshmfence_LIB_DIRS_RELEASE}" # package_libdir
                              "${libxshmfence_BIN_DIRS_RELEASE}" # package_bindir
                              "${libxshmfence_LIBRARY_TYPE_RELEASE}"
                              "${libxshmfence_IS_HOST_WINDOWS_RELEASE}"
                              libxshmfence_DEPS_TARGET
                              libxshmfence_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "libxshmfence"    # package_name
                              "${libxshmfence_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${libxshmfence_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Release ########################################
    set_property(TARGET libxshmfence::libxshmfence
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Release>:${libxshmfence_OBJECTS_RELEASE}>
                 $<$<CONFIG:Release>:${libxshmfence_LIBRARIES_TARGETS}>
                 )

    if("${libxshmfence_LIBS_RELEASE}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET libxshmfence::libxshmfence
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     libxshmfence_DEPS_TARGET)
    endif()

    set_property(TARGET libxshmfence::libxshmfence
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Release>:${libxshmfence_LINKER_FLAGS_RELEASE}>)
    set_property(TARGET libxshmfence::libxshmfence
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Release>:${libxshmfence_INCLUDE_DIRS_RELEASE}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET libxshmfence::libxshmfence
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Release>:${libxshmfence_LIB_DIRS_RELEASE}>)
    set_property(TARGET libxshmfence::libxshmfence
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Release>:${libxshmfence_COMPILE_DEFINITIONS_RELEASE}>)
    set_property(TARGET libxshmfence::libxshmfence
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Release>:${libxshmfence_COMPILE_OPTIONS_RELEASE}>)

########## For the modules (FindXXX)
set(libxshmfence_LIBRARIES_RELEASE libxshmfence::libxshmfence)
