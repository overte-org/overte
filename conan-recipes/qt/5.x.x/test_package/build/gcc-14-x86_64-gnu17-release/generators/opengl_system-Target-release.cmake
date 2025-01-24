# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(opengl_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(opengl_FRAMEWORKS_FOUND_RELEASE "${opengl_FRAMEWORKS_RELEASE}" "${opengl_FRAMEWORK_DIRS_RELEASE}")

set(opengl_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET opengl_DEPS_TARGET)
    add_library(opengl_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET opengl_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${opengl_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${opengl_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### opengl_DEPS_TARGET to all of them
conan_package_library_targets("${opengl_LIBS_RELEASE}"    # libraries
                              "${opengl_LIB_DIRS_RELEASE}" # package_libdir
                              "${opengl_BIN_DIRS_RELEASE}" # package_bindir
                              "${opengl_LIBRARY_TYPE_RELEASE}"
                              "${opengl_IS_HOST_WINDOWS_RELEASE}"
                              opengl_DEPS_TARGET
                              opengl_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "opengl"    # package_name
                              "${opengl_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${opengl_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Release ########################################
    set_property(TARGET opengl::opengl
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Release>:${opengl_OBJECTS_RELEASE}>
                 $<$<CONFIG:Release>:${opengl_LIBRARIES_TARGETS}>
                 )

    if("${opengl_LIBS_RELEASE}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET opengl::opengl
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     opengl_DEPS_TARGET)
    endif()

    set_property(TARGET opengl::opengl
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Release>:${opengl_LINKER_FLAGS_RELEASE}>)
    set_property(TARGET opengl::opengl
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Release>:${opengl_INCLUDE_DIRS_RELEASE}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET opengl::opengl
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Release>:${opengl_LIB_DIRS_RELEASE}>)
    set_property(TARGET opengl::opengl
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Release>:${opengl_COMPILE_DEFINITIONS_RELEASE}>)
    set_property(TARGET opengl::opengl
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Release>:${opengl_COMPILE_OPTIONS_RELEASE}>)

########## For the modules (FindXXX)
set(opengl_LIBRARIES_RELEASE opengl::opengl)
