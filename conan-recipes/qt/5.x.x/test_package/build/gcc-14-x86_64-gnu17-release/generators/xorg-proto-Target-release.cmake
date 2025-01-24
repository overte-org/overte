# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(xorg-proto_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(xorg-proto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_FRAMEWORKS_RELEASE}" "${xorg-proto_FRAMEWORK_DIRS_RELEASE}")

set(xorg-proto_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET xorg-proto_DEPS_TARGET)
    add_library(xorg-proto_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET xorg-proto_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${xorg-proto_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${xorg-proto_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:xorg-macros::xorg-macros>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### xorg-proto_DEPS_TARGET to all of them
conan_package_library_targets("${xorg-proto_LIBS_RELEASE}"    # libraries
                              "${xorg-proto_LIB_DIRS_RELEASE}" # package_libdir
                              "${xorg-proto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_DEPS_TARGET
                              xorg-proto_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "xorg-proto"    # package_name
                              "${xorg-proto_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${xorg-proto_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## COMPONENTS TARGET PROPERTIES Release ########################################

    ########## COMPONENT xorg-proto::xwaylandproto #############

        set(xorg-proto_xorg-proto_xwaylandproto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg-proto_xorg-proto_xwaylandproto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_xorg-proto_xwaylandproto_FRAMEWORKS_RELEASE}" "${xorg-proto_xorg-proto_xwaylandproto_FRAMEWORK_DIRS_RELEASE}")

        set(xorg-proto_xorg-proto_xwaylandproto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg-proto_xorg-proto_xwaylandproto_DEPS_TARGET)
            add_library(xorg-proto_xorg-proto_xwaylandproto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg-proto_xorg-proto_xwaylandproto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xwaylandproto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xwaylandproto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xwaylandproto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg-proto_xorg-proto_xwaylandproto_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg-proto_xorg-proto_xwaylandproto_LIBS_RELEASE}"
                              "${xorg-proto_xorg-proto_xwaylandproto_LIB_DIRS_RELEASE}"
                              "${xorg-proto_xorg-proto_xwaylandproto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_xorg-proto_xwaylandproto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_xorg-proto_xwaylandproto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_xorg-proto_xwaylandproto_DEPS_TARGET
                              xorg-proto_xorg-proto_xwaylandproto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg-proto_xorg-proto_xwaylandproto"
                              "${xorg-proto_xorg-proto_xwaylandproto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg-proto::xwaylandproto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xwaylandproto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xwaylandproto_LIBRARIES_TARGETS}>
                     )

        if("${xorg-proto_xorg-proto_xwaylandproto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg-proto::xwaylandproto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg-proto_xorg-proto_xwaylandproto_DEPS_TARGET)
        endif()

        set_property(TARGET xorg-proto::xwaylandproto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xwaylandproto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg-proto::xwaylandproto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xwaylandproto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::xwaylandproto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xwaylandproto_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::xwaylandproto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xwaylandproto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg-proto::xwaylandproto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xwaylandproto_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg-proto::xproto #############

        set(xorg-proto_xorg-proto_xproto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg-proto_xorg-proto_xproto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_xorg-proto_xproto_FRAMEWORKS_RELEASE}" "${xorg-proto_xorg-proto_xproto_FRAMEWORK_DIRS_RELEASE}")

        set(xorg-proto_xorg-proto_xproto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg-proto_xorg-proto_xproto_DEPS_TARGET)
            add_library(xorg-proto_xorg-proto_xproto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg-proto_xorg-proto_xproto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xproto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xproto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xproto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg-proto_xorg-proto_xproto_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg-proto_xorg-proto_xproto_LIBS_RELEASE}"
                              "${xorg-proto_xorg-proto_xproto_LIB_DIRS_RELEASE}"
                              "${xorg-proto_xorg-proto_xproto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_xorg-proto_xproto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_xorg-proto_xproto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_xorg-proto_xproto_DEPS_TARGET
                              xorg-proto_xorg-proto_xproto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg-proto_xorg-proto_xproto"
                              "${xorg-proto_xorg-proto_xproto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg-proto::xproto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xproto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xproto_LIBRARIES_TARGETS}>
                     )

        if("${xorg-proto_xorg-proto_xproto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg-proto::xproto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg-proto_xorg-proto_xproto_DEPS_TARGET)
        endif()

        set_property(TARGET xorg-proto::xproto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xproto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg-proto::xproto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xproto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::xproto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xproto_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::xproto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xproto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg-proto::xproto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xproto_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg-proto::xineramaproto #############

        set(xorg-proto_xorg-proto_xineramaproto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg-proto_xorg-proto_xineramaproto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_xorg-proto_xineramaproto_FRAMEWORKS_RELEASE}" "${xorg-proto_xorg-proto_xineramaproto_FRAMEWORK_DIRS_RELEASE}")

        set(xorg-proto_xorg-proto_xineramaproto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg-proto_xorg-proto_xineramaproto_DEPS_TARGET)
            add_library(xorg-proto_xorg-proto_xineramaproto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg-proto_xorg-proto_xineramaproto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xineramaproto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xineramaproto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xineramaproto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg-proto_xorg-proto_xineramaproto_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg-proto_xorg-proto_xineramaproto_LIBS_RELEASE}"
                              "${xorg-proto_xorg-proto_xineramaproto_LIB_DIRS_RELEASE}"
                              "${xorg-proto_xorg-proto_xineramaproto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_xorg-proto_xineramaproto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_xorg-proto_xineramaproto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_xorg-proto_xineramaproto_DEPS_TARGET
                              xorg-proto_xorg-proto_xineramaproto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg-proto_xorg-proto_xineramaproto"
                              "${xorg-proto_xorg-proto_xineramaproto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg-proto::xineramaproto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xineramaproto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xineramaproto_LIBRARIES_TARGETS}>
                     )

        if("${xorg-proto_xorg-proto_xineramaproto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg-proto::xineramaproto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg-proto_xorg-proto_xineramaproto_DEPS_TARGET)
        endif()

        set_property(TARGET xorg-proto::xineramaproto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xineramaproto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg-proto::xineramaproto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xineramaproto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::xineramaproto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xineramaproto_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::xineramaproto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xineramaproto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg-proto::xineramaproto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xineramaproto_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg-proto::xf86vidmodeproto #############

        set(xorg-proto_xorg-proto_xf86vidmodeproto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg-proto_xorg-proto_xf86vidmodeproto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_xorg-proto_xf86vidmodeproto_FRAMEWORKS_RELEASE}" "${xorg-proto_xorg-proto_xf86vidmodeproto_FRAMEWORK_DIRS_RELEASE}")

        set(xorg-proto_xorg-proto_xf86vidmodeproto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg-proto_xorg-proto_xf86vidmodeproto_DEPS_TARGET)
            add_library(xorg-proto_xorg-proto_xf86vidmodeproto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg-proto_xorg-proto_xf86vidmodeproto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86vidmodeproto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86vidmodeproto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86vidmodeproto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg-proto_xorg-proto_xf86vidmodeproto_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg-proto_xorg-proto_xf86vidmodeproto_LIBS_RELEASE}"
                              "${xorg-proto_xorg-proto_xf86vidmodeproto_LIB_DIRS_RELEASE}"
                              "${xorg-proto_xorg-proto_xf86vidmodeproto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_xorg-proto_xf86vidmodeproto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_xorg-proto_xf86vidmodeproto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_xorg-proto_xf86vidmodeproto_DEPS_TARGET
                              xorg-proto_xorg-proto_xf86vidmodeproto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg-proto_xorg-proto_xf86vidmodeproto"
                              "${xorg-proto_xorg-proto_xf86vidmodeproto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg-proto::xf86vidmodeproto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86vidmodeproto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86vidmodeproto_LIBRARIES_TARGETS}>
                     )

        if("${xorg-proto_xorg-proto_xf86vidmodeproto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg-proto::xf86vidmodeproto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg-proto_xorg-proto_xf86vidmodeproto_DEPS_TARGET)
        endif()

        set_property(TARGET xorg-proto::xf86vidmodeproto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86vidmodeproto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg-proto::xf86vidmodeproto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86vidmodeproto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::xf86vidmodeproto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86vidmodeproto_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::xf86vidmodeproto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86vidmodeproto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg-proto::xf86vidmodeproto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86vidmodeproto_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg-proto::xf86driproto #############

        set(xorg-proto_xorg-proto_xf86driproto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg-proto_xorg-proto_xf86driproto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_xorg-proto_xf86driproto_FRAMEWORKS_RELEASE}" "${xorg-proto_xorg-proto_xf86driproto_FRAMEWORK_DIRS_RELEASE}")

        set(xorg-proto_xorg-proto_xf86driproto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg-proto_xorg-proto_xf86driproto_DEPS_TARGET)
            add_library(xorg-proto_xorg-proto_xf86driproto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg-proto_xorg-proto_xf86driproto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86driproto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86driproto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86driproto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg-proto_xorg-proto_xf86driproto_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg-proto_xorg-proto_xf86driproto_LIBS_RELEASE}"
                              "${xorg-proto_xorg-proto_xf86driproto_LIB_DIRS_RELEASE}"
                              "${xorg-proto_xorg-proto_xf86driproto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_xorg-proto_xf86driproto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_xorg-proto_xf86driproto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_xorg-proto_xf86driproto_DEPS_TARGET
                              xorg-proto_xorg-proto_xf86driproto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg-proto_xorg-proto_xf86driproto"
                              "${xorg-proto_xorg-proto_xf86driproto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg-proto::xf86driproto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86driproto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86driproto_LIBRARIES_TARGETS}>
                     )

        if("${xorg-proto_xorg-proto_xf86driproto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg-proto::xf86driproto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg-proto_xorg-proto_xf86driproto_DEPS_TARGET)
        endif()

        set_property(TARGET xorg-proto::xf86driproto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86driproto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg-proto::xf86driproto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86driproto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::xf86driproto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86driproto_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::xf86driproto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86driproto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg-proto::xf86driproto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86driproto_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg-proto::xf86dgaproto #############

        set(xorg-proto_xorg-proto_xf86dgaproto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg-proto_xorg-proto_xf86dgaproto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_xorg-proto_xf86dgaproto_FRAMEWORKS_RELEASE}" "${xorg-proto_xorg-proto_xf86dgaproto_FRAMEWORK_DIRS_RELEASE}")

        set(xorg-proto_xorg-proto_xf86dgaproto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg-proto_xorg-proto_xf86dgaproto_DEPS_TARGET)
            add_library(xorg-proto_xorg-proto_xf86dgaproto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg-proto_xorg-proto_xf86dgaproto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86dgaproto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86dgaproto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86dgaproto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg-proto_xorg-proto_xf86dgaproto_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg-proto_xorg-proto_xf86dgaproto_LIBS_RELEASE}"
                              "${xorg-proto_xorg-proto_xf86dgaproto_LIB_DIRS_RELEASE}"
                              "${xorg-proto_xorg-proto_xf86dgaproto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_xorg-proto_xf86dgaproto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_xorg-proto_xf86dgaproto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_xorg-proto_xf86dgaproto_DEPS_TARGET
                              xorg-proto_xorg-proto_xf86dgaproto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg-proto_xorg-proto_xf86dgaproto"
                              "${xorg-proto_xorg-proto_xf86dgaproto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg-proto::xf86dgaproto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86dgaproto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86dgaproto_LIBRARIES_TARGETS}>
                     )

        if("${xorg-proto_xorg-proto_xf86dgaproto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg-proto::xf86dgaproto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg-proto_xorg-proto_xf86dgaproto_DEPS_TARGET)
        endif()

        set_property(TARGET xorg-proto::xf86dgaproto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86dgaproto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg-proto::xf86dgaproto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86dgaproto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::xf86dgaproto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86dgaproto_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::xf86dgaproto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86dgaproto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg-proto::xf86dgaproto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86dgaproto_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg-proto::xf86bigfontproto #############

        set(xorg-proto_xorg-proto_xf86bigfontproto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg-proto_xorg-proto_xf86bigfontproto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_xorg-proto_xf86bigfontproto_FRAMEWORKS_RELEASE}" "${xorg-proto_xorg-proto_xf86bigfontproto_FRAMEWORK_DIRS_RELEASE}")

        set(xorg-proto_xorg-proto_xf86bigfontproto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg-proto_xorg-proto_xf86bigfontproto_DEPS_TARGET)
            add_library(xorg-proto_xorg-proto_xf86bigfontproto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg-proto_xorg-proto_xf86bigfontproto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86bigfontproto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86bigfontproto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86bigfontproto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg-proto_xorg-proto_xf86bigfontproto_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg-proto_xorg-proto_xf86bigfontproto_LIBS_RELEASE}"
                              "${xorg-proto_xorg-proto_xf86bigfontproto_LIB_DIRS_RELEASE}"
                              "${xorg-proto_xorg-proto_xf86bigfontproto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_xorg-proto_xf86bigfontproto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_xorg-proto_xf86bigfontproto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_xorg-proto_xf86bigfontproto_DEPS_TARGET
                              xorg-proto_xorg-proto_xf86bigfontproto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg-proto_xorg-proto_xf86bigfontproto"
                              "${xorg-proto_xorg-proto_xf86bigfontproto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg-proto::xf86bigfontproto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86bigfontproto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86bigfontproto_LIBRARIES_TARGETS}>
                     )

        if("${xorg-proto_xorg-proto_xf86bigfontproto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg-proto::xf86bigfontproto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg-proto_xorg-proto_xf86bigfontproto_DEPS_TARGET)
        endif()

        set_property(TARGET xorg-proto::xf86bigfontproto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86bigfontproto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg-proto::xf86bigfontproto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86bigfontproto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::xf86bigfontproto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86bigfontproto_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::xf86bigfontproto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86bigfontproto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg-proto::xf86bigfontproto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xf86bigfontproto_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg-proto::xextproto #############

        set(xorg-proto_xorg-proto_xextproto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg-proto_xorg-proto_xextproto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_xorg-proto_xextproto_FRAMEWORKS_RELEASE}" "${xorg-proto_xorg-proto_xextproto_FRAMEWORK_DIRS_RELEASE}")

        set(xorg-proto_xorg-proto_xextproto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg-proto_xorg-proto_xextproto_DEPS_TARGET)
            add_library(xorg-proto_xorg-proto_xextproto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg-proto_xorg-proto_xextproto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xextproto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xextproto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xextproto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg-proto_xorg-proto_xextproto_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg-proto_xorg-proto_xextproto_LIBS_RELEASE}"
                              "${xorg-proto_xorg-proto_xextproto_LIB_DIRS_RELEASE}"
                              "${xorg-proto_xorg-proto_xextproto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_xorg-proto_xextproto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_xorg-proto_xextproto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_xorg-proto_xextproto_DEPS_TARGET
                              xorg-proto_xorg-proto_xextproto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg-proto_xorg-proto_xextproto"
                              "${xorg-proto_xorg-proto_xextproto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg-proto::xextproto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xextproto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xextproto_LIBRARIES_TARGETS}>
                     )

        if("${xorg-proto_xorg-proto_xextproto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg-proto::xextproto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg-proto_xorg-proto_xextproto_DEPS_TARGET)
        endif()

        set_property(TARGET xorg-proto::xextproto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xextproto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg-proto::xextproto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xextproto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::xextproto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xextproto_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::xextproto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xextproto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg-proto::xextproto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xextproto_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg-proto::xcmiscproto #############

        set(xorg-proto_xorg-proto_xcmiscproto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg-proto_xorg-proto_xcmiscproto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_xorg-proto_xcmiscproto_FRAMEWORKS_RELEASE}" "${xorg-proto_xorg-proto_xcmiscproto_FRAMEWORK_DIRS_RELEASE}")

        set(xorg-proto_xorg-proto_xcmiscproto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg-proto_xorg-proto_xcmiscproto_DEPS_TARGET)
            add_library(xorg-proto_xorg-proto_xcmiscproto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg-proto_xorg-proto_xcmiscproto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xcmiscproto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xcmiscproto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xcmiscproto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg-proto_xorg-proto_xcmiscproto_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg-proto_xorg-proto_xcmiscproto_LIBS_RELEASE}"
                              "${xorg-proto_xorg-proto_xcmiscproto_LIB_DIRS_RELEASE}"
                              "${xorg-proto_xorg-proto_xcmiscproto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_xorg-proto_xcmiscproto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_xorg-proto_xcmiscproto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_xorg-proto_xcmiscproto_DEPS_TARGET
                              xorg-proto_xorg-proto_xcmiscproto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg-proto_xorg-proto_xcmiscproto"
                              "${xorg-proto_xorg-proto_xcmiscproto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg-proto::xcmiscproto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xcmiscproto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xcmiscproto_LIBRARIES_TARGETS}>
                     )

        if("${xorg-proto_xorg-proto_xcmiscproto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg-proto::xcmiscproto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg-proto_xorg-proto_xcmiscproto_DEPS_TARGET)
        endif()

        set_property(TARGET xorg-proto::xcmiscproto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xcmiscproto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg-proto::xcmiscproto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xcmiscproto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::xcmiscproto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xcmiscproto_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::xcmiscproto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xcmiscproto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg-proto::xcmiscproto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_xcmiscproto_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg-proto::videoproto #############

        set(xorg-proto_xorg-proto_videoproto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg-proto_xorg-proto_videoproto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_xorg-proto_videoproto_FRAMEWORKS_RELEASE}" "${xorg-proto_xorg-proto_videoproto_FRAMEWORK_DIRS_RELEASE}")

        set(xorg-proto_xorg-proto_videoproto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg-proto_xorg-proto_videoproto_DEPS_TARGET)
            add_library(xorg-proto_xorg-proto_videoproto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg-proto_xorg-proto_videoproto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_videoproto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_videoproto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_videoproto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg-proto_xorg-proto_videoproto_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg-proto_xorg-proto_videoproto_LIBS_RELEASE}"
                              "${xorg-proto_xorg-proto_videoproto_LIB_DIRS_RELEASE}"
                              "${xorg-proto_xorg-proto_videoproto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_xorg-proto_videoproto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_xorg-proto_videoproto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_xorg-proto_videoproto_DEPS_TARGET
                              xorg-proto_xorg-proto_videoproto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg-proto_xorg-proto_videoproto"
                              "${xorg-proto_xorg-proto_videoproto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg-proto::videoproto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_videoproto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_videoproto_LIBRARIES_TARGETS}>
                     )

        if("${xorg-proto_xorg-proto_videoproto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg-proto::videoproto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg-proto_xorg-proto_videoproto_DEPS_TARGET)
        endif()

        set_property(TARGET xorg-proto::videoproto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_videoproto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg-proto::videoproto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_videoproto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::videoproto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_videoproto_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::videoproto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_videoproto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg-proto::videoproto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_videoproto_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg-proto::scrnsaverproto #############

        set(xorg-proto_xorg-proto_scrnsaverproto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg-proto_xorg-proto_scrnsaverproto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_xorg-proto_scrnsaverproto_FRAMEWORKS_RELEASE}" "${xorg-proto_xorg-proto_scrnsaverproto_FRAMEWORK_DIRS_RELEASE}")

        set(xorg-proto_xorg-proto_scrnsaverproto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg-proto_xorg-proto_scrnsaverproto_DEPS_TARGET)
            add_library(xorg-proto_xorg-proto_scrnsaverproto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg-proto_xorg-proto_scrnsaverproto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_scrnsaverproto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_scrnsaverproto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_scrnsaverproto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg-proto_xorg-proto_scrnsaverproto_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg-proto_xorg-proto_scrnsaverproto_LIBS_RELEASE}"
                              "${xorg-proto_xorg-proto_scrnsaverproto_LIB_DIRS_RELEASE}"
                              "${xorg-proto_xorg-proto_scrnsaverproto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_xorg-proto_scrnsaverproto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_xorg-proto_scrnsaverproto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_xorg-proto_scrnsaverproto_DEPS_TARGET
                              xorg-proto_xorg-proto_scrnsaverproto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg-proto_xorg-proto_scrnsaverproto"
                              "${xorg-proto_xorg-proto_scrnsaverproto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg-proto::scrnsaverproto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_scrnsaverproto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_scrnsaverproto_LIBRARIES_TARGETS}>
                     )

        if("${xorg-proto_xorg-proto_scrnsaverproto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg-proto::scrnsaverproto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg-proto_xorg-proto_scrnsaverproto_DEPS_TARGET)
        endif()

        set_property(TARGET xorg-proto::scrnsaverproto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_scrnsaverproto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg-proto::scrnsaverproto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_scrnsaverproto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::scrnsaverproto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_scrnsaverproto_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::scrnsaverproto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_scrnsaverproto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg-proto::scrnsaverproto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_scrnsaverproto_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg-proto::resourceproto #############

        set(xorg-proto_xorg-proto_resourceproto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg-proto_xorg-proto_resourceproto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_xorg-proto_resourceproto_FRAMEWORKS_RELEASE}" "${xorg-proto_xorg-proto_resourceproto_FRAMEWORK_DIRS_RELEASE}")

        set(xorg-proto_xorg-proto_resourceproto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg-proto_xorg-proto_resourceproto_DEPS_TARGET)
            add_library(xorg-proto_xorg-proto_resourceproto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg-proto_xorg-proto_resourceproto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_resourceproto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_resourceproto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_resourceproto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg-proto_xorg-proto_resourceproto_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg-proto_xorg-proto_resourceproto_LIBS_RELEASE}"
                              "${xorg-proto_xorg-proto_resourceproto_LIB_DIRS_RELEASE}"
                              "${xorg-proto_xorg-proto_resourceproto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_xorg-proto_resourceproto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_xorg-proto_resourceproto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_xorg-proto_resourceproto_DEPS_TARGET
                              xorg-proto_xorg-proto_resourceproto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg-proto_xorg-proto_resourceproto"
                              "${xorg-proto_xorg-proto_resourceproto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg-proto::resourceproto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_resourceproto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_resourceproto_LIBRARIES_TARGETS}>
                     )

        if("${xorg-proto_xorg-proto_resourceproto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg-proto::resourceproto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg-proto_xorg-proto_resourceproto_DEPS_TARGET)
        endif()

        set_property(TARGET xorg-proto::resourceproto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_resourceproto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg-proto::resourceproto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_resourceproto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::resourceproto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_resourceproto_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::resourceproto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_resourceproto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg-proto::resourceproto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_resourceproto_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg-proto::renderproto #############

        set(xorg-proto_xorg-proto_renderproto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg-proto_xorg-proto_renderproto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_xorg-proto_renderproto_FRAMEWORKS_RELEASE}" "${xorg-proto_xorg-proto_renderproto_FRAMEWORK_DIRS_RELEASE}")

        set(xorg-proto_xorg-proto_renderproto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg-proto_xorg-proto_renderproto_DEPS_TARGET)
            add_library(xorg-proto_xorg-proto_renderproto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg-proto_xorg-proto_renderproto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_renderproto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_renderproto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_renderproto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg-proto_xorg-proto_renderproto_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg-proto_xorg-proto_renderproto_LIBS_RELEASE}"
                              "${xorg-proto_xorg-proto_renderproto_LIB_DIRS_RELEASE}"
                              "${xorg-proto_xorg-proto_renderproto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_xorg-proto_renderproto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_xorg-proto_renderproto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_xorg-proto_renderproto_DEPS_TARGET
                              xorg-proto_xorg-proto_renderproto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg-proto_xorg-proto_renderproto"
                              "${xorg-proto_xorg-proto_renderproto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg-proto::renderproto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_renderproto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_renderproto_LIBRARIES_TARGETS}>
                     )

        if("${xorg-proto_xorg-proto_renderproto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg-proto::renderproto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg-proto_xorg-proto_renderproto_DEPS_TARGET)
        endif()

        set_property(TARGET xorg-proto::renderproto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_renderproto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg-proto::renderproto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_renderproto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::renderproto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_renderproto_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::renderproto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_renderproto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg-proto::renderproto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_renderproto_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg-proto::recordproto #############

        set(xorg-proto_xorg-proto_recordproto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg-proto_xorg-proto_recordproto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_xorg-proto_recordproto_FRAMEWORKS_RELEASE}" "${xorg-proto_xorg-proto_recordproto_FRAMEWORK_DIRS_RELEASE}")

        set(xorg-proto_xorg-proto_recordproto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg-proto_xorg-proto_recordproto_DEPS_TARGET)
            add_library(xorg-proto_xorg-proto_recordproto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg-proto_xorg-proto_recordproto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_recordproto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_recordproto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_recordproto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg-proto_xorg-proto_recordproto_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg-proto_xorg-proto_recordproto_LIBS_RELEASE}"
                              "${xorg-proto_xorg-proto_recordproto_LIB_DIRS_RELEASE}"
                              "${xorg-proto_xorg-proto_recordproto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_xorg-proto_recordproto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_xorg-proto_recordproto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_xorg-proto_recordproto_DEPS_TARGET
                              xorg-proto_xorg-proto_recordproto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg-proto_xorg-proto_recordproto"
                              "${xorg-proto_xorg-proto_recordproto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg-proto::recordproto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_recordproto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_recordproto_LIBRARIES_TARGETS}>
                     )

        if("${xorg-proto_xorg-proto_recordproto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg-proto::recordproto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg-proto_xorg-proto_recordproto_DEPS_TARGET)
        endif()

        set_property(TARGET xorg-proto::recordproto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_recordproto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg-proto::recordproto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_recordproto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::recordproto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_recordproto_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::recordproto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_recordproto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg-proto::recordproto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_recordproto_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg-proto::randrproto #############

        set(xorg-proto_xorg-proto_randrproto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg-proto_xorg-proto_randrproto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_xorg-proto_randrproto_FRAMEWORKS_RELEASE}" "${xorg-proto_xorg-proto_randrproto_FRAMEWORK_DIRS_RELEASE}")

        set(xorg-proto_xorg-proto_randrproto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg-proto_xorg-proto_randrproto_DEPS_TARGET)
            add_library(xorg-proto_xorg-proto_randrproto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg-proto_xorg-proto_randrproto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_randrproto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_randrproto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_randrproto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg-proto_xorg-proto_randrproto_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg-proto_xorg-proto_randrproto_LIBS_RELEASE}"
                              "${xorg-proto_xorg-proto_randrproto_LIB_DIRS_RELEASE}"
                              "${xorg-proto_xorg-proto_randrproto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_xorg-proto_randrproto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_xorg-proto_randrproto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_xorg-proto_randrproto_DEPS_TARGET
                              xorg-proto_xorg-proto_randrproto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg-proto_xorg-proto_randrproto"
                              "${xorg-proto_xorg-proto_randrproto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg-proto::randrproto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_randrproto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_randrproto_LIBRARIES_TARGETS}>
                     )

        if("${xorg-proto_xorg-proto_randrproto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg-proto::randrproto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg-proto_xorg-proto_randrproto_DEPS_TARGET)
        endif()

        set_property(TARGET xorg-proto::randrproto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_randrproto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg-proto::randrproto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_randrproto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::randrproto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_randrproto_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::randrproto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_randrproto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg-proto::randrproto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_randrproto_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg-proto::presentproto #############

        set(xorg-proto_xorg-proto_presentproto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg-proto_xorg-proto_presentproto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_xorg-proto_presentproto_FRAMEWORKS_RELEASE}" "${xorg-proto_xorg-proto_presentproto_FRAMEWORK_DIRS_RELEASE}")

        set(xorg-proto_xorg-proto_presentproto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg-proto_xorg-proto_presentproto_DEPS_TARGET)
            add_library(xorg-proto_xorg-proto_presentproto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg-proto_xorg-proto_presentproto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_presentproto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_presentproto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_presentproto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg-proto_xorg-proto_presentproto_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg-proto_xorg-proto_presentproto_LIBS_RELEASE}"
                              "${xorg-proto_xorg-proto_presentproto_LIB_DIRS_RELEASE}"
                              "${xorg-proto_xorg-proto_presentproto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_xorg-proto_presentproto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_xorg-proto_presentproto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_xorg-proto_presentproto_DEPS_TARGET
                              xorg-proto_xorg-proto_presentproto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg-proto_xorg-proto_presentproto"
                              "${xorg-proto_xorg-proto_presentproto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg-proto::presentproto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_presentproto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_presentproto_LIBRARIES_TARGETS}>
                     )

        if("${xorg-proto_xorg-proto_presentproto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg-proto::presentproto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg-proto_xorg-proto_presentproto_DEPS_TARGET)
        endif()

        set_property(TARGET xorg-proto::presentproto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_presentproto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg-proto::presentproto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_presentproto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::presentproto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_presentproto_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::presentproto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_presentproto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg-proto::presentproto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_presentproto_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg-proto::kbproto #############

        set(xorg-proto_xorg-proto_kbproto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg-proto_xorg-proto_kbproto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_xorg-proto_kbproto_FRAMEWORKS_RELEASE}" "${xorg-proto_xorg-proto_kbproto_FRAMEWORK_DIRS_RELEASE}")

        set(xorg-proto_xorg-proto_kbproto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg-proto_xorg-proto_kbproto_DEPS_TARGET)
            add_library(xorg-proto_xorg-proto_kbproto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg-proto_xorg-proto_kbproto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_kbproto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_kbproto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_kbproto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg-proto_xorg-proto_kbproto_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg-proto_xorg-proto_kbproto_LIBS_RELEASE}"
                              "${xorg-proto_xorg-proto_kbproto_LIB_DIRS_RELEASE}"
                              "${xorg-proto_xorg-proto_kbproto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_xorg-proto_kbproto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_xorg-proto_kbproto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_xorg-proto_kbproto_DEPS_TARGET
                              xorg-proto_xorg-proto_kbproto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg-proto_xorg-proto_kbproto"
                              "${xorg-proto_xorg-proto_kbproto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg-proto::kbproto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_kbproto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_kbproto_LIBRARIES_TARGETS}>
                     )

        if("${xorg-proto_xorg-proto_kbproto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg-proto::kbproto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg-proto_xorg-proto_kbproto_DEPS_TARGET)
        endif()

        set_property(TARGET xorg-proto::kbproto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_kbproto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg-proto::kbproto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_kbproto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::kbproto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_kbproto_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::kbproto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_kbproto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg-proto::kbproto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_kbproto_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg-proto::inputproto #############

        set(xorg-proto_xorg-proto_inputproto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg-proto_xorg-proto_inputproto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_xorg-proto_inputproto_FRAMEWORKS_RELEASE}" "${xorg-proto_xorg-proto_inputproto_FRAMEWORK_DIRS_RELEASE}")

        set(xorg-proto_xorg-proto_inputproto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg-proto_xorg-proto_inputproto_DEPS_TARGET)
            add_library(xorg-proto_xorg-proto_inputproto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg-proto_xorg-proto_inputproto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_inputproto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_inputproto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_inputproto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg-proto_xorg-proto_inputproto_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg-proto_xorg-proto_inputproto_LIBS_RELEASE}"
                              "${xorg-proto_xorg-proto_inputproto_LIB_DIRS_RELEASE}"
                              "${xorg-proto_xorg-proto_inputproto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_xorg-proto_inputproto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_xorg-proto_inputproto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_xorg-proto_inputproto_DEPS_TARGET
                              xorg-proto_xorg-proto_inputproto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg-proto_xorg-proto_inputproto"
                              "${xorg-proto_xorg-proto_inputproto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg-proto::inputproto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_inputproto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_inputproto_LIBRARIES_TARGETS}>
                     )

        if("${xorg-proto_xorg-proto_inputproto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg-proto::inputproto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg-proto_xorg-proto_inputproto_DEPS_TARGET)
        endif()

        set_property(TARGET xorg-proto::inputproto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_inputproto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg-proto::inputproto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_inputproto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::inputproto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_inputproto_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::inputproto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_inputproto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg-proto::inputproto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_inputproto_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg-proto::glproto #############

        set(xorg-proto_xorg-proto_glproto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg-proto_xorg-proto_glproto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_xorg-proto_glproto_FRAMEWORKS_RELEASE}" "${xorg-proto_xorg-proto_glproto_FRAMEWORK_DIRS_RELEASE}")

        set(xorg-proto_xorg-proto_glproto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg-proto_xorg-proto_glproto_DEPS_TARGET)
            add_library(xorg-proto_xorg-proto_glproto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg-proto_xorg-proto_glproto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_glproto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_glproto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_glproto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg-proto_xorg-proto_glproto_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg-proto_xorg-proto_glproto_LIBS_RELEASE}"
                              "${xorg-proto_xorg-proto_glproto_LIB_DIRS_RELEASE}"
                              "${xorg-proto_xorg-proto_glproto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_xorg-proto_glproto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_xorg-proto_glproto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_xorg-proto_glproto_DEPS_TARGET
                              xorg-proto_xorg-proto_glproto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg-proto_xorg-proto_glproto"
                              "${xorg-proto_xorg-proto_glproto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg-proto::glproto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_glproto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_glproto_LIBRARIES_TARGETS}>
                     )

        if("${xorg-proto_xorg-proto_glproto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg-proto::glproto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg-proto_xorg-proto_glproto_DEPS_TARGET)
        endif()

        set_property(TARGET xorg-proto::glproto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_glproto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg-proto::glproto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_glproto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::glproto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_glproto_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::glproto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_glproto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg-proto::glproto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_glproto_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg-proto::fontsproto #############

        set(xorg-proto_xorg-proto_fontsproto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg-proto_xorg-proto_fontsproto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_xorg-proto_fontsproto_FRAMEWORKS_RELEASE}" "${xorg-proto_xorg-proto_fontsproto_FRAMEWORK_DIRS_RELEASE}")

        set(xorg-proto_xorg-proto_fontsproto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg-proto_xorg-proto_fontsproto_DEPS_TARGET)
            add_library(xorg-proto_xorg-proto_fontsproto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg-proto_xorg-proto_fontsproto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_fontsproto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_fontsproto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_fontsproto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg-proto_xorg-proto_fontsproto_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg-proto_xorg-proto_fontsproto_LIBS_RELEASE}"
                              "${xorg-proto_xorg-proto_fontsproto_LIB_DIRS_RELEASE}"
                              "${xorg-proto_xorg-proto_fontsproto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_xorg-proto_fontsproto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_xorg-proto_fontsproto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_xorg-proto_fontsproto_DEPS_TARGET
                              xorg-proto_xorg-proto_fontsproto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg-proto_xorg-proto_fontsproto"
                              "${xorg-proto_xorg-proto_fontsproto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg-proto::fontsproto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_fontsproto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_fontsproto_LIBRARIES_TARGETS}>
                     )

        if("${xorg-proto_xorg-proto_fontsproto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg-proto::fontsproto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg-proto_xorg-proto_fontsproto_DEPS_TARGET)
        endif()

        set_property(TARGET xorg-proto::fontsproto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_fontsproto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg-proto::fontsproto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_fontsproto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::fontsproto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_fontsproto_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::fontsproto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_fontsproto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg-proto::fontsproto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_fontsproto_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg-proto::fixesproto #############

        set(xorg-proto_xorg-proto_fixesproto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg-proto_xorg-proto_fixesproto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_xorg-proto_fixesproto_FRAMEWORKS_RELEASE}" "${xorg-proto_xorg-proto_fixesproto_FRAMEWORK_DIRS_RELEASE}")

        set(xorg-proto_xorg-proto_fixesproto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg-proto_xorg-proto_fixesproto_DEPS_TARGET)
            add_library(xorg-proto_xorg-proto_fixesproto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg-proto_xorg-proto_fixesproto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_fixesproto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_fixesproto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_fixesproto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg-proto_xorg-proto_fixesproto_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg-proto_xorg-proto_fixesproto_LIBS_RELEASE}"
                              "${xorg-proto_xorg-proto_fixesproto_LIB_DIRS_RELEASE}"
                              "${xorg-proto_xorg-proto_fixesproto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_xorg-proto_fixesproto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_xorg-proto_fixesproto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_xorg-proto_fixesproto_DEPS_TARGET
                              xorg-proto_xorg-proto_fixesproto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg-proto_xorg-proto_fixesproto"
                              "${xorg-proto_xorg-proto_fixesproto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg-proto::fixesproto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_fixesproto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_fixesproto_LIBRARIES_TARGETS}>
                     )

        if("${xorg-proto_xorg-proto_fixesproto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg-proto::fixesproto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg-proto_xorg-proto_fixesproto_DEPS_TARGET)
        endif()

        set_property(TARGET xorg-proto::fixesproto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_fixesproto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg-proto::fixesproto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_fixesproto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::fixesproto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_fixesproto_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::fixesproto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_fixesproto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg-proto::fixesproto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_fixesproto_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg-proto::dri3proto #############

        set(xorg-proto_xorg-proto_dri3proto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg-proto_xorg-proto_dri3proto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_xorg-proto_dri3proto_FRAMEWORKS_RELEASE}" "${xorg-proto_xorg-proto_dri3proto_FRAMEWORK_DIRS_RELEASE}")

        set(xorg-proto_xorg-proto_dri3proto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg-proto_xorg-proto_dri3proto_DEPS_TARGET)
            add_library(xorg-proto_xorg-proto_dri3proto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg-proto_xorg-proto_dri3proto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dri3proto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dri3proto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dri3proto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg-proto_xorg-proto_dri3proto_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg-proto_xorg-proto_dri3proto_LIBS_RELEASE}"
                              "${xorg-proto_xorg-proto_dri3proto_LIB_DIRS_RELEASE}"
                              "${xorg-proto_xorg-proto_dri3proto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_xorg-proto_dri3proto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_xorg-proto_dri3proto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_xorg-proto_dri3proto_DEPS_TARGET
                              xorg-proto_xorg-proto_dri3proto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg-proto_xorg-proto_dri3proto"
                              "${xorg-proto_xorg-proto_dri3proto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg-proto::dri3proto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dri3proto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dri3proto_LIBRARIES_TARGETS}>
                     )

        if("${xorg-proto_xorg-proto_dri3proto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg-proto::dri3proto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg-proto_xorg-proto_dri3proto_DEPS_TARGET)
        endif()

        set_property(TARGET xorg-proto::dri3proto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dri3proto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg-proto::dri3proto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dri3proto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::dri3proto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dri3proto_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::dri3proto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dri3proto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg-proto::dri3proto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dri3proto_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg-proto::dri2proto #############

        set(xorg-proto_xorg-proto_dri2proto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg-proto_xorg-proto_dri2proto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_xorg-proto_dri2proto_FRAMEWORKS_RELEASE}" "${xorg-proto_xorg-proto_dri2proto_FRAMEWORK_DIRS_RELEASE}")

        set(xorg-proto_xorg-proto_dri2proto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg-proto_xorg-proto_dri2proto_DEPS_TARGET)
            add_library(xorg-proto_xorg-proto_dri2proto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg-proto_xorg-proto_dri2proto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dri2proto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dri2proto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dri2proto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg-proto_xorg-proto_dri2proto_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg-proto_xorg-proto_dri2proto_LIBS_RELEASE}"
                              "${xorg-proto_xorg-proto_dri2proto_LIB_DIRS_RELEASE}"
                              "${xorg-proto_xorg-proto_dri2proto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_xorg-proto_dri2proto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_xorg-proto_dri2proto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_xorg-proto_dri2proto_DEPS_TARGET
                              xorg-proto_xorg-proto_dri2proto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg-proto_xorg-proto_dri2proto"
                              "${xorg-proto_xorg-proto_dri2proto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg-proto::dri2proto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dri2proto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dri2proto_LIBRARIES_TARGETS}>
                     )

        if("${xorg-proto_xorg-proto_dri2proto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg-proto::dri2proto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg-proto_xorg-proto_dri2proto_DEPS_TARGET)
        endif()

        set_property(TARGET xorg-proto::dri2proto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dri2proto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg-proto::dri2proto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dri2proto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::dri2proto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dri2proto_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::dri2proto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dri2proto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg-proto::dri2proto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dri2proto_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg-proto::dpmsproto #############

        set(xorg-proto_xorg-proto_dpmsproto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg-proto_xorg-proto_dpmsproto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_xorg-proto_dpmsproto_FRAMEWORKS_RELEASE}" "${xorg-proto_xorg-proto_dpmsproto_FRAMEWORK_DIRS_RELEASE}")

        set(xorg-proto_xorg-proto_dpmsproto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg-proto_xorg-proto_dpmsproto_DEPS_TARGET)
            add_library(xorg-proto_xorg-proto_dpmsproto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg-proto_xorg-proto_dpmsproto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dpmsproto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dpmsproto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dpmsproto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg-proto_xorg-proto_dpmsproto_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg-proto_xorg-proto_dpmsproto_LIBS_RELEASE}"
                              "${xorg-proto_xorg-proto_dpmsproto_LIB_DIRS_RELEASE}"
                              "${xorg-proto_xorg-proto_dpmsproto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_xorg-proto_dpmsproto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_xorg-proto_dpmsproto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_xorg-proto_dpmsproto_DEPS_TARGET
                              xorg-proto_xorg-proto_dpmsproto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg-proto_xorg-proto_dpmsproto"
                              "${xorg-proto_xorg-proto_dpmsproto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg-proto::dpmsproto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dpmsproto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dpmsproto_LIBRARIES_TARGETS}>
                     )

        if("${xorg-proto_xorg-proto_dpmsproto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg-proto::dpmsproto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg-proto_xorg-proto_dpmsproto_DEPS_TARGET)
        endif()

        set_property(TARGET xorg-proto::dpmsproto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dpmsproto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg-proto::dpmsproto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dpmsproto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::dpmsproto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dpmsproto_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::dpmsproto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dpmsproto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg-proto::dpmsproto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dpmsproto_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg-proto::dmxproto #############

        set(xorg-proto_xorg-proto_dmxproto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg-proto_xorg-proto_dmxproto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_xorg-proto_dmxproto_FRAMEWORKS_RELEASE}" "${xorg-proto_xorg-proto_dmxproto_FRAMEWORK_DIRS_RELEASE}")

        set(xorg-proto_xorg-proto_dmxproto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg-proto_xorg-proto_dmxproto_DEPS_TARGET)
            add_library(xorg-proto_xorg-proto_dmxproto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg-proto_xorg-proto_dmxproto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dmxproto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dmxproto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dmxproto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg-proto_xorg-proto_dmxproto_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg-proto_xorg-proto_dmxproto_LIBS_RELEASE}"
                              "${xorg-proto_xorg-proto_dmxproto_LIB_DIRS_RELEASE}"
                              "${xorg-proto_xorg-proto_dmxproto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_xorg-proto_dmxproto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_xorg-proto_dmxproto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_xorg-proto_dmxproto_DEPS_TARGET
                              xorg-proto_xorg-proto_dmxproto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg-proto_xorg-proto_dmxproto"
                              "${xorg-proto_xorg-proto_dmxproto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg-proto::dmxproto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dmxproto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dmxproto_LIBRARIES_TARGETS}>
                     )

        if("${xorg-proto_xorg-proto_dmxproto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg-proto::dmxproto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg-proto_xorg-proto_dmxproto_DEPS_TARGET)
        endif()

        set_property(TARGET xorg-proto::dmxproto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dmxproto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg-proto::dmxproto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dmxproto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::dmxproto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dmxproto_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::dmxproto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dmxproto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg-proto::dmxproto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_dmxproto_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg-proto::damageproto #############

        set(xorg-proto_xorg-proto_damageproto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg-proto_xorg-proto_damageproto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_xorg-proto_damageproto_FRAMEWORKS_RELEASE}" "${xorg-proto_xorg-proto_damageproto_FRAMEWORK_DIRS_RELEASE}")

        set(xorg-proto_xorg-proto_damageproto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg-proto_xorg-proto_damageproto_DEPS_TARGET)
            add_library(xorg-proto_xorg-proto_damageproto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg-proto_xorg-proto_damageproto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_damageproto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_damageproto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_damageproto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg-proto_xorg-proto_damageproto_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg-proto_xorg-proto_damageproto_LIBS_RELEASE}"
                              "${xorg-proto_xorg-proto_damageproto_LIB_DIRS_RELEASE}"
                              "${xorg-proto_xorg-proto_damageproto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_xorg-proto_damageproto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_xorg-proto_damageproto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_xorg-proto_damageproto_DEPS_TARGET
                              xorg-proto_xorg-proto_damageproto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg-proto_xorg-proto_damageproto"
                              "${xorg-proto_xorg-proto_damageproto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg-proto::damageproto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_damageproto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_damageproto_LIBRARIES_TARGETS}>
                     )

        if("${xorg-proto_xorg-proto_damageproto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg-proto::damageproto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg-proto_xorg-proto_damageproto_DEPS_TARGET)
        endif()

        set_property(TARGET xorg-proto::damageproto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_damageproto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg-proto::damageproto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_damageproto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::damageproto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_damageproto_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::damageproto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_damageproto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg-proto::damageproto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_damageproto_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg-proto::compositeproto #############

        set(xorg-proto_xorg-proto_compositeproto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg-proto_xorg-proto_compositeproto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_xorg-proto_compositeproto_FRAMEWORKS_RELEASE}" "${xorg-proto_xorg-proto_compositeproto_FRAMEWORK_DIRS_RELEASE}")

        set(xorg-proto_xorg-proto_compositeproto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg-proto_xorg-proto_compositeproto_DEPS_TARGET)
            add_library(xorg-proto_xorg-proto_compositeproto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg-proto_xorg-proto_compositeproto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_compositeproto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_compositeproto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_compositeproto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg-proto_xorg-proto_compositeproto_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg-proto_xorg-proto_compositeproto_LIBS_RELEASE}"
                              "${xorg-proto_xorg-proto_compositeproto_LIB_DIRS_RELEASE}"
                              "${xorg-proto_xorg-proto_compositeproto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_xorg-proto_compositeproto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_xorg-proto_compositeproto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_xorg-proto_compositeproto_DEPS_TARGET
                              xorg-proto_xorg-proto_compositeproto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg-proto_xorg-proto_compositeproto"
                              "${xorg-proto_xorg-proto_compositeproto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg-proto::compositeproto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_compositeproto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_compositeproto_LIBRARIES_TARGETS}>
                     )

        if("${xorg-proto_xorg-proto_compositeproto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg-proto::compositeproto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg-proto_xorg-proto_compositeproto_DEPS_TARGET)
        endif()

        set_property(TARGET xorg-proto::compositeproto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_compositeproto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg-proto::compositeproto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_compositeproto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::compositeproto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_compositeproto_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::compositeproto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_compositeproto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg-proto::compositeproto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_compositeproto_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg-proto::bigreqsproto #############

        set(xorg-proto_xorg-proto_bigreqsproto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg-proto_xorg-proto_bigreqsproto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_xorg-proto_bigreqsproto_FRAMEWORKS_RELEASE}" "${xorg-proto_xorg-proto_bigreqsproto_FRAMEWORK_DIRS_RELEASE}")

        set(xorg-proto_xorg-proto_bigreqsproto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg-proto_xorg-proto_bigreqsproto_DEPS_TARGET)
            add_library(xorg-proto_xorg-proto_bigreqsproto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg-proto_xorg-proto_bigreqsproto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_bigreqsproto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_bigreqsproto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_bigreqsproto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg-proto_xorg-proto_bigreqsproto_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg-proto_xorg-proto_bigreqsproto_LIBS_RELEASE}"
                              "${xorg-proto_xorg-proto_bigreqsproto_LIB_DIRS_RELEASE}"
                              "${xorg-proto_xorg-proto_bigreqsproto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_xorg-proto_bigreqsproto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_xorg-proto_bigreqsproto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_xorg-proto_bigreqsproto_DEPS_TARGET
                              xorg-proto_xorg-proto_bigreqsproto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg-proto_xorg-proto_bigreqsproto"
                              "${xorg-proto_xorg-proto_bigreqsproto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg-proto::bigreqsproto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_bigreqsproto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_bigreqsproto_LIBRARIES_TARGETS}>
                     )

        if("${xorg-proto_xorg-proto_bigreqsproto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg-proto::bigreqsproto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg-proto_xorg-proto_bigreqsproto_DEPS_TARGET)
        endif()

        set_property(TARGET xorg-proto::bigreqsproto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_bigreqsproto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg-proto::bigreqsproto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_bigreqsproto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::bigreqsproto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_bigreqsproto_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::bigreqsproto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_bigreqsproto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg-proto::bigreqsproto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_bigreqsproto_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg-proto::applewmproto #############

        set(xorg-proto_xorg-proto_applewmproto_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg-proto_xorg-proto_applewmproto_FRAMEWORKS_FOUND_RELEASE "${xorg-proto_xorg-proto_applewmproto_FRAMEWORKS_RELEASE}" "${xorg-proto_xorg-proto_applewmproto_FRAMEWORK_DIRS_RELEASE}")

        set(xorg-proto_xorg-proto_applewmproto_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg-proto_xorg-proto_applewmproto_DEPS_TARGET)
            add_library(xorg-proto_xorg-proto_applewmproto_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg-proto_xorg-proto_applewmproto_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_applewmproto_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_applewmproto_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_applewmproto_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg-proto_xorg-proto_applewmproto_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg-proto_xorg-proto_applewmproto_LIBS_RELEASE}"
                              "${xorg-proto_xorg-proto_applewmproto_LIB_DIRS_RELEASE}"
                              "${xorg-proto_xorg-proto_applewmproto_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg-proto_xorg-proto_applewmproto_LIBRARY_TYPE_RELEASE}"
                              "${xorg-proto_xorg-proto_applewmproto_IS_HOST_WINDOWS_RELEASE}"
                              xorg-proto_xorg-proto_applewmproto_DEPS_TARGET
                              xorg-proto_xorg-proto_applewmproto_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg-proto_xorg-proto_applewmproto"
                              "${xorg-proto_xorg-proto_applewmproto_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg-proto::applewmproto
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_applewmproto_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_applewmproto_LIBRARIES_TARGETS}>
                     )

        if("${xorg-proto_xorg-proto_applewmproto_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg-proto::applewmproto
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg-proto_xorg-proto_applewmproto_DEPS_TARGET)
        endif()

        set_property(TARGET xorg-proto::applewmproto APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_applewmproto_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg-proto::applewmproto APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_applewmproto_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::applewmproto APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_applewmproto_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg-proto::applewmproto APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_applewmproto_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg-proto::applewmproto APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg-proto_xorg-proto_applewmproto_COMPILE_OPTIONS_RELEASE}>)

    ########## AGGREGATED GLOBAL TARGET WITH THE COMPONENTS #####################
    set_property(TARGET xorg-proto::xorg-proto APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg-proto::xwaylandproto)
    set_property(TARGET xorg-proto::xorg-proto APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg-proto::xproto)
    set_property(TARGET xorg-proto::xorg-proto APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg-proto::xineramaproto)
    set_property(TARGET xorg-proto::xorg-proto APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg-proto::xf86vidmodeproto)
    set_property(TARGET xorg-proto::xorg-proto APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg-proto::xf86driproto)
    set_property(TARGET xorg-proto::xorg-proto APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg-proto::xf86dgaproto)
    set_property(TARGET xorg-proto::xorg-proto APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg-proto::xf86bigfontproto)
    set_property(TARGET xorg-proto::xorg-proto APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg-proto::xextproto)
    set_property(TARGET xorg-proto::xorg-proto APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg-proto::xcmiscproto)
    set_property(TARGET xorg-proto::xorg-proto APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg-proto::videoproto)
    set_property(TARGET xorg-proto::xorg-proto APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg-proto::scrnsaverproto)
    set_property(TARGET xorg-proto::xorg-proto APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg-proto::resourceproto)
    set_property(TARGET xorg-proto::xorg-proto APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg-proto::renderproto)
    set_property(TARGET xorg-proto::xorg-proto APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg-proto::recordproto)
    set_property(TARGET xorg-proto::xorg-proto APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg-proto::randrproto)
    set_property(TARGET xorg-proto::xorg-proto APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg-proto::presentproto)
    set_property(TARGET xorg-proto::xorg-proto APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg-proto::kbproto)
    set_property(TARGET xorg-proto::xorg-proto APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg-proto::inputproto)
    set_property(TARGET xorg-proto::xorg-proto APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg-proto::glproto)
    set_property(TARGET xorg-proto::xorg-proto APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg-proto::fontsproto)
    set_property(TARGET xorg-proto::xorg-proto APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg-proto::fixesproto)
    set_property(TARGET xorg-proto::xorg-proto APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg-proto::dri3proto)
    set_property(TARGET xorg-proto::xorg-proto APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg-proto::dri2proto)
    set_property(TARGET xorg-proto::xorg-proto APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg-proto::dpmsproto)
    set_property(TARGET xorg-proto::xorg-proto APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg-proto::dmxproto)
    set_property(TARGET xorg-proto::xorg-proto APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg-proto::damageproto)
    set_property(TARGET xorg-proto::xorg-proto APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg-proto::compositeproto)
    set_property(TARGET xorg-proto::xorg-proto APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg-proto::bigreqsproto)
    set_property(TARGET xorg-proto::xorg-proto APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg-proto::applewmproto)

########## For the modules (FindXXX)
set(xorg-proto_LIBRARIES_RELEASE xorg-proto::xorg-proto)
