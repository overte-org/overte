# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(xorg_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(xorg_FRAMEWORKS_FOUND_RELEASE "${xorg_FRAMEWORKS_RELEASE}" "${xorg_FRAMEWORK_DIRS_RELEASE}")

set(xorg_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET xorg_DEPS_TARGET)
    add_library(xorg_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET xorg_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${xorg_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${xorg_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:xorg::uuid>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### xorg_DEPS_TARGET to all of them
conan_package_library_targets("${xorg_LIBS_RELEASE}"    # libraries
                              "${xorg_LIB_DIRS_RELEASE}" # package_libdir
                              "${xorg_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_LIBRARY_TYPE_RELEASE}"
                              "${xorg_IS_HOST_WINDOWS_RELEASE}"
                              xorg_DEPS_TARGET
                              xorg_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "xorg"    # package_name
                              "${xorg_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${xorg_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## COMPONENTS TARGET PROPERTIES Release ########################################

    ########## COMPONENT xorg::sm #############

        set(xorg_xorg_sm_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_sm_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_sm_FRAMEWORKS_RELEASE}" "${xorg_xorg_sm_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_sm_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_sm_DEPS_TARGET)
            add_library(xorg_xorg_sm_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_sm_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_sm_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_sm_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_sm_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_sm_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_sm_LIBS_RELEASE}"
                              "${xorg_xorg_sm_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_sm_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_sm_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_sm_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_sm_DEPS_TARGET
                              xorg_xorg_sm_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_sm"
                              "${xorg_xorg_sm_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::sm
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_sm_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_sm_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_sm_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::sm
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_sm_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::sm APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_sm_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::sm APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_sm_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::sm APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_sm_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::sm APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_sm_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::sm APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_sm_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::uuid #############

        set(xorg_xorg_uuid_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_uuid_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_uuid_FRAMEWORKS_RELEASE}" "${xorg_xorg_uuid_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_uuid_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_uuid_DEPS_TARGET)
            add_library(xorg_xorg_uuid_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_uuid_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_uuid_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_uuid_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_uuid_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_uuid_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_uuid_LIBS_RELEASE}"
                              "${xorg_xorg_uuid_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_uuid_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_uuid_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_uuid_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_uuid_DEPS_TARGET
                              xorg_xorg_uuid_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_uuid"
                              "${xorg_xorg_uuid_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::uuid
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_uuid_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_uuid_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_uuid_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::uuid
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_uuid_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::uuid APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_uuid_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::uuid APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_uuid_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::uuid APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_uuid_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::uuid APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_uuid_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::uuid APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_uuid_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xcb-res #############

        set(xorg_xorg_xcb-res_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xcb-res_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xcb-res_FRAMEWORKS_RELEASE}" "${xorg_xorg_xcb-res_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xcb-res_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xcb-res_DEPS_TARGET)
            add_library(xorg_xorg_xcb-res_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xcb-res_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-res_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-res_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-res_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xcb-res_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xcb-res_LIBS_RELEASE}"
                              "${xorg_xorg_xcb-res_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xcb-res_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xcb-res_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xcb-res_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xcb-res_DEPS_TARGET
                              xorg_xorg_xcb-res_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xcb-res"
                              "${xorg_xorg_xcb-res_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xcb-res
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-res_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-res_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xcb-res_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xcb-res
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xcb-res_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xcb-res APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-res_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xcb-res APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-res_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-res APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-res_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-res APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-res_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xcb-res APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-res_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xcb-ewmh #############

        set(xorg_xorg_xcb-ewmh_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xcb-ewmh_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xcb-ewmh_FRAMEWORKS_RELEASE}" "${xorg_xorg_xcb-ewmh_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xcb-ewmh_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xcb-ewmh_DEPS_TARGET)
            add_library(xorg_xorg_xcb-ewmh_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xcb-ewmh_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-ewmh_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-ewmh_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-ewmh_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xcb-ewmh_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xcb-ewmh_LIBS_RELEASE}"
                              "${xorg_xorg_xcb-ewmh_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xcb-ewmh_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xcb-ewmh_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xcb-ewmh_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xcb-ewmh_DEPS_TARGET
                              xorg_xorg_xcb-ewmh_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xcb-ewmh"
                              "${xorg_xorg_xcb-ewmh_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xcb-ewmh
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-ewmh_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-ewmh_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xcb-ewmh_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xcb-ewmh
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xcb-ewmh_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xcb-ewmh APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-ewmh_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xcb-ewmh APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-ewmh_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-ewmh APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-ewmh_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-ewmh APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-ewmh_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xcb-ewmh APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-ewmh_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xcb-composite #############

        set(xorg_xorg_xcb-composite_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xcb-composite_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xcb-composite_FRAMEWORKS_RELEASE}" "${xorg_xorg_xcb-composite_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xcb-composite_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xcb-composite_DEPS_TARGET)
            add_library(xorg_xorg_xcb-composite_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xcb-composite_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-composite_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-composite_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-composite_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xcb-composite_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xcb-composite_LIBS_RELEASE}"
                              "${xorg_xorg_xcb-composite_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xcb-composite_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xcb-composite_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xcb-composite_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xcb-composite_DEPS_TARGET
                              xorg_xorg_xcb-composite_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xcb-composite"
                              "${xorg_xorg_xcb-composite_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xcb-composite
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-composite_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-composite_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xcb-composite_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xcb-composite
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xcb-composite_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xcb-composite APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-composite_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xcb-composite APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-composite_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-composite APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-composite_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-composite APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-composite_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xcb-composite APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-composite_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xcb-present #############

        set(xorg_xorg_xcb-present_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xcb-present_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xcb-present_FRAMEWORKS_RELEASE}" "${xorg_xorg_xcb-present_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xcb-present_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xcb-present_DEPS_TARGET)
            add_library(xorg_xorg_xcb-present_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xcb-present_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-present_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-present_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-present_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xcb-present_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xcb-present_LIBS_RELEASE}"
                              "${xorg_xorg_xcb-present_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xcb-present_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xcb-present_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xcb-present_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xcb-present_DEPS_TARGET
                              xorg_xorg_xcb-present_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xcb-present"
                              "${xorg_xorg_xcb-present_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xcb-present
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-present_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-present_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xcb-present_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xcb-present
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xcb-present_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xcb-present APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-present_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xcb-present APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-present_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-present APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-present_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-present APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-present_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xcb-present APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-present_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xcb-glx #############

        set(xorg_xorg_xcb-glx_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xcb-glx_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xcb-glx_FRAMEWORKS_RELEASE}" "${xorg_xorg_xcb-glx_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xcb-glx_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xcb-glx_DEPS_TARGET)
            add_library(xorg_xorg_xcb-glx_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xcb-glx_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-glx_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-glx_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-glx_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xcb-glx_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xcb-glx_LIBS_RELEASE}"
                              "${xorg_xorg_xcb-glx_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xcb-glx_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xcb-glx_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xcb-glx_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xcb-glx_DEPS_TARGET
                              xorg_xorg_xcb-glx_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xcb-glx"
                              "${xorg_xorg_xcb-glx_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xcb-glx
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-glx_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-glx_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xcb-glx_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xcb-glx
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xcb-glx_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xcb-glx APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-glx_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xcb-glx APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-glx_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-glx APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-glx_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-glx APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-glx_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xcb-glx APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-glx_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xcb-dri2 #############

        set(xorg_xorg_xcb-dri2_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xcb-dri2_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xcb-dri2_FRAMEWORKS_RELEASE}" "${xorg_xorg_xcb-dri2_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xcb-dri2_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xcb-dri2_DEPS_TARGET)
            add_library(xorg_xorg_xcb-dri2_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xcb-dri2_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-dri2_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-dri2_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-dri2_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xcb-dri2_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xcb-dri2_LIBS_RELEASE}"
                              "${xorg_xorg_xcb-dri2_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xcb-dri2_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xcb-dri2_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xcb-dri2_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xcb-dri2_DEPS_TARGET
                              xorg_xorg_xcb-dri2_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xcb-dri2"
                              "${xorg_xorg_xcb-dri2_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xcb-dri2
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-dri2_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-dri2_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xcb-dri2_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xcb-dri2
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xcb-dri2_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xcb-dri2 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-dri2_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xcb-dri2 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-dri2_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-dri2 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-dri2_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-dri2 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-dri2_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xcb-dri2 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-dri2_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xcb-cursor #############

        set(xorg_xorg_xcb-cursor_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xcb-cursor_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xcb-cursor_FRAMEWORKS_RELEASE}" "${xorg_xorg_xcb-cursor_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xcb-cursor_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xcb-cursor_DEPS_TARGET)
            add_library(xorg_xorg_xcb-cursor_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xcb-cursor_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-cursor_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-cursor_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-cursor_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xcb-cursor_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xcb-cursor_LIBS_RELEASE}"
                              "${xorg_xorg_xcb-cursor_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xcb-cursor_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xcb-cursor_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xcb-cursor_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xcb-cursor_DEPS_TARGET
                              xorg_xorg_xcb-cursor_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xcb-cursor"
                              "${xorg_xorg_xcb-cursor_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xcb-cursor
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-cursor_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-cursor_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xcb-cursor_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xcb-cursor
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xcb-cursor_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xcb-cursor APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-cursor_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xcb-cursor APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-cursor_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-cursor APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-cursor_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-cursor APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-cursor_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xcb-cursor APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-cursor_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xcb-dri3 #############

        set(xorg_xorg_xcb-dri3_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xcb-dri3_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xcb-dri3_FRAMEWORKS_RELEASE}" "${xorg_xorg_xcb-dri3_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xcb-dri3_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xcb-dri3_DEPS_TARGET)
            add_library(xorg_xorg_xcb-dri3_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xcb-dri3_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-dri3_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-dri3_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-dri3_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xcb-dri3_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xcb-dri3_LIBS_RELEASE}"
                              "${xorg_xorg_xcb-dri3_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xcb-dri3_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xcb-dri3_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xcb-dri3_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xcb-dri3_DEPS_TARGET
                              xorg_xorg_xcb-dri3_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xcb-dri3"
                              "${xorg_xorg_xcb-dri3_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xcb-dri3
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-dri3_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-dri3_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xcb-dri3_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xcb-dri3
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xcb-dri3_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xcb-dri3 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-dri3_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xcb-dri3 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-dri3_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-dri3 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-dri3_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-dri3 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-dri3_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xcb-dri3 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-dri3_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xcb-util #############

        set(xorg_xorg_xcb-util_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xcb-util_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xcb-util_FRAMEWORKS_RELEASE}" "${xorg_xorg_xcb-util_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xcb-util_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xcb-util_DEPS_TARGET)
            add_library(xorg_xorg_xcb-util_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xcb-util_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-util_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-util_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-util_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xcb-util_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xcb-util_LIBS_RELEASE}"
                              "${xorg_xorg_xcb-util_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xcb-util_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xcb-util_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xcb-util_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xcb-util_DEPS_TARGET
                              xorg_xorg_xcb-util_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xcb-util"
                              "${xorg_xorg_xcb-util_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xcb-util
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-util_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-util_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xcb-util_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xcb-util
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xcb-util_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xcb-util APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-util_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xcb-util APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-util_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-util APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-util_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-util APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-util_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xcb-util APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-util_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xcb-event #############

        set(xorg_xorg_xcb-event_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xcb-event_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xcb-event_FRAMEWORKS_RELEASE}" "${xorg_xorg_xcb-event_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xcb-event_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xcb-event_DEPS_TARGET)
            add_library(xorg_xorg_xcb-event_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xcb-event_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-event_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-event_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-event_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xcb-event_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xcb-event_LIBS_RELEASE}"
                              "${xorg_xorg_xcb-event_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xcb-event_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xcb-event_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xcb-event_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xcb-event_DEPS_TARGET
                              xorg_xorg_xcb-event_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xcb-event"
                              "${xorg_xorg_xcb-event_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xcb-event
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-event_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-event_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xcb-event_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xcb-event
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xcb-event_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xcb-event APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-event_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xcb-event APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-event_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-event APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-event_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-event APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-event_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xcb-event APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-event_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xcb-aux #############

        set(xorg_xorg_xcb-aux_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xcb-aux_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xcb-aux_FRAMEWORKS_RELEASE}" "${xorg_xorg_xcb-aux_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xcb-aux_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xcb-aux_DEPS_TARGET)
            add_library(xorg_xorg_xcb-aux_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xcb-aux_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-aux_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-aux_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-aux_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xcb-aux_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xcb-aux_LIBS_RELEASE}"
                              "${xorg_xorg_xcb-aux_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xcb-aux_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xcb-aux_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xcb-aux_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xcb-aux_DEPS_TARGET
                              xorg_xorg_xcb-aux_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xcb-aux"
                              "${xorg_xorg_xcb-aux_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xcb-aux
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-aux_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-aux_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xcb-aux_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xcb-aux
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xcb-aux_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xcb-aux APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-aux_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xcb-aux APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-aux_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-aux APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-aux_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-aux APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-aux_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xcb-aux APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-aux_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xcb-atom #############

        set(xorg_xorg_xcb-atom_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xcb-atom_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xcb-atom_FRAMEWORKS_RELEASE}" "${xorg_xorg_xcb-atom_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xcb-atom_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xcb-atom_DEPS_TARGET)
            add_library(xorg_xorg_xcb-atom_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xcb-atom_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-atom_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-atom_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-atom_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xcb-atom_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xcb-atom_LIBS_RELEASE}"
                              "${xorg_xorg_xcb-atom_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xcb-atom_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xcb-atom_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xcb-atom_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xcb-atom_DEPS_TARGET
                              xorg_xorg_xcb-atom_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xcb-atom"
                              "${xorg_xorg_xcb-atom_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xcb-atom
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-atom_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-atom_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xcb-atom_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xcb-atom
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xcb-atom_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xcb-atom APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-atom_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xcb-atom APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-atom_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-atom APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-atom_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-atom APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-atom_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xcb-atom APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-atom_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xcb #############

        set(xorg_xorg_xcb_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xcb_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xcb_FRAMEWORKS_RELEASE}" "${xorg_xorg_xcb_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xcb_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xcb_DEPS_TARGET)
            add_library(xorg_xorg_xcb_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xcb_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xcb_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xcb_LIBS_RELEASE}"
                              "${xorg_xorg_xcb_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xcb_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xcb_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xcb_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xcb_DEPS_TARGET
                              xorg_xorg_xcb_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xcb"
                              "${xorg_xorg_xcb_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xcb
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xcb_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xcb
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xcb_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xcb APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xcb APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xcb APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xcb-xinerama #############

        set(xorg_xorg_xcb-xinerama_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xcb-xinerama_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xcb-xinerama_FRAMEWORKS_RELEASE}" "${xorg_xorg_xcb-xinerama_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xcb-xinerama_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xcb-xinerama_DEPS_TARGET)
            add_library(xorg_xorg_xcb-xinerama_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xcb-xinerama_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xinerama_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xinerama_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xinerama_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xcb-xinerama_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xcb-xinerama_LIBS_RELEASE}"
                              "${xorg_xorg_xcb-xinerama_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xcb-xinerama_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xcb-xinerama_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xcb-xinerama_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xcb-xinerama_DEPS_TARGET
                              xorg_xorg_xcb-xinerama_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xcb-xinerama"
                              "${xorg_xorg_xcb-xinerama_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xcb-xinerama
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xinerama_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xinerama_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xcb-xinerama_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xcb-xinerama
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xcb-xinerama_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xcb-xinerama APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xinerama_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xcb-xinerama APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xinerama_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-xinerama APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xinerama_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-xinerama APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xinerama_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xcb-xinerama APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xinerama_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xcb-xfixes #############

        set(xorg_xorg_xcb-xfixes_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xcb-xfixes_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xcb-xfixes_FRAMEWORKS_RELEASE}" "${xorg_xorg_xcb-xfixes_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xcb-xfixes_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xcb-xfixes_DEPS_TARGET)
            add_library(xorg_xorg_xcb-xfixes_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xcb-xfixes_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xfixes_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xfixes_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xfixes_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xcb-xfixes_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xcb-xfixes_LIBS_RELEASE}"
                              "${xorg_xorg_xcb-xfixes_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xcb-xfixes_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xcb-xfixes_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xcb-xfixes_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xcb-xfixes_DEPS_TARGET
                              xorg_xorg_xcb-xfixes_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xcb-xfixes"
                              "${xorg_xorg_xcb-xfixes_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xcb-xfixes
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xfixes_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xfixes_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xcb-xfixes_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xcb-xfixes
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xcb-xfixes_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xcb-xfixes APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xfixes_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xcb-xfixes APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xfixes_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-xfixes APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xfixes_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-xfixes APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xfixes_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xcb-xfixes APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xfixes_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xcb-sync #############

        set(xorg_xorg_xcb-sync_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xcb-sync_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xcb-sync_FRAMEWORKS_RELEASE}" "${xorg_xorg_xcb-sync_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xcb-sync_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xcb-sync_DEPS_TARGET)
            add_library(xorg_xorg_xcb-sync_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xcb-sync_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-sync_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-sync_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-sync_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xcb-sync_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xcb-sync_LIBS_RELEASE}"
                              "${xorg_xorg_xcb-sync_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xcb-sync_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xcb-sync_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xcb-sync_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xcb-sync_DEPS_TARGET
                              xorg_xorg_xcb-sync_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xcb-sync"
                              "${xorg_xorg_xcb-sync_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xcb-sync
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-sync_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-sync_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xcb-sync_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xcb-sync
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xcb-sync_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xcb-sync APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-sync_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xcb-sync APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-sync_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-sync APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-sync_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-sync APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-sync_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xcb-sync APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-sync_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xcb-shm #############

        set(xorg_xorg_xcb-shm_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xcb-shm_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xcb-shm_FRAMEWORKS_RELEASE}" "${xorg_xorg_xcb-shm_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xcb-shm_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xcb-shm_DEPS_TARGET)
            add_library(xorg_xorg_xcb-shm_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xcb-shm_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-shm_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-shm_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-shm_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xcb-shm_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xcb-shm_LIBS_RELEASE}"
                              "${xorg_xorg_xcb-shm_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xcb-shm_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xcb-shm_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xcb-shm_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xcb-shm_DEPS_TARGET
                              xorg_xorg_xcb-shm_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xcb-shm"
                              "${xorg_xorg_xcb-shm_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xcb-shm
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-shm_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-shm_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xcb-shm_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xcb-shm
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xcb-shm_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xcb-shm APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-shm_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xcb-shm APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-shm_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-shm APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-shm_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-shm APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-shm_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xcb-shm APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-shm_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xcb-shape #############

        set(xorg_xorg_xcb-shape_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xcb-shape_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xcb-shape_FRAMEWORKS_RELEASE}" "${xorg_xorg_xcb-shape_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xcb-shape_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xcb-shape_DEPS_TARGET)
            add_library(xorg_xorg_xcb-shape_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xcb-shape_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-shape_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-shape_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-shape_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xcb-shape_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xcb-shape_LIBS_RELEASE}"
                              "${xorg_xorg_xcb-shape_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xcb-shape_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xcb-shape_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xcb-shape_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xcb-shape_DEPS_TARGET
                              xorg_xorg_xcb-shape_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xcb-shape"
                              "${xorg_xorg_xcb-shape_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xcb-shape
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-shape_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-shape_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xcb-shape_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xcb-shape
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xcb-shape_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xcb-shape APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-shape_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xcb-shape APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-shape_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-shape APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-shape_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-shape APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-shape_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xcb-shape APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-shape_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xcb-renderutil #############

        set(xorg_xorg_xcb-renderutil_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xcb-renderutil_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xcb-renderutil_FRAMEWORKS_RELEASE}" "${xorg_xorg_xcb-renderutil_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xcb-renderutil_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xcb-renderutil_DEPS_TARGET)
            add_library(xorg_xorg_xcb-renderutil_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xcb-renderutil_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-renderutil_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-renderutil_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-renderutil_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xcb-renderutil_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xcb-renderutil_LIBS_RELEASE}"
                              "${xorg_xorg_xcb-renderutil_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xcb-renderutil_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xcb-renderutil_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xcb-renderutil_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xcb-renderutil_DEPS_TARGET
                              xorg_xorg_xcb-renderutil_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xcb-renderutil"
                              "${xorg_xorg_xcb-renderutil_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xcb-renderutil
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-renderutil_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-renderutil_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xcb-renderutil_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xcb-renderutil
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xcb-renderutil_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xcb-renderutil APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-renderutil_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xcb-renderutil APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-renderutil_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-renderutil APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-renderutil_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-renderutil APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-renderutil_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xcb-renderutil APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-renderutil_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xcb-render #############

        set(xorg_xorg_xcb-render_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xcb-render_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xcb-render_FRAMEWORKS_RELEASE}" "${xorg_xorg_xcb-render_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xcb-render_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xcb-render_DEPS_TARGET)
            add_library(xorg_xorg_xcb-render_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xcb-render_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-render_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-render_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-render_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xcb-render_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xcb-render_LIBS_RELEASE}"
                              "${xorg_xorg_xcb-render_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xcb-render_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xcb-render_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xcb-render_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xcb-render_DEPS_TARGET
                              xorg_xorg_xcb-render_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xcb-render"
                              "${xorg_xorg_xcb-render_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xcb-render
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-render_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-render_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xcb-render_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xcb-render
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xcb-render_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xcb-render APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-render_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xcb-render APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-render_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-render APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-render_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-render APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-render_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xcb-render APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-render_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xcb-randr #############

        set(xorg_xorg_xcb-randr_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xcb-randr_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xcb-randr_FRAMEWORKS_RELEASE}" "${xorg_xorg_xcb-randr_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xcb-randr_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xcb-randr_DEPS_TARGET)
            add_library(xorg_xorg_xcb-randr_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xcb-randr_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-randr_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-randr_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-randr_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xcb-randr_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xcb-randr_LIBS_RELEASE}"
                              "${xorg_xorg_xcb-randr_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xcb-randr_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xcb-randr_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xcb-randr_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xcb-randr_DEPS_TARGET
                              xorg_xorg_xcb-randr_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xcb-randr"
                              "${xorg_xorg_xcb-randr_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xcb-randr
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-randr_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-randr_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xcb-randr_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xcb-randr
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xcb-randr_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xcb-randr APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-randr_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xcb-randr APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-randr_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-randr APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-randr_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-randr APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-randr_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xcb-randr APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-randr_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xcb-keysyms #############

        set(xorg_xorg_xcb-keysyms_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xcb-keysyms_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xcb-keysyms_FRAMEWORKS_RELEASE}" "${xorg_xorg_xcb-keysyms_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xcb-keysyms_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xcb-keysyms_DEPS_TARGET)
            add_library(xorg_xorg_xcb-keysyms_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xcb-keysyms_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-keysyms_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-keysyms_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-keysyms_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xcb-keysyms_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xcb-keysyms_LIBS_RELEASE}"
                              "${xorg_xorg_xcb-keysyms_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xcb-keysyms_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xcb-keysyms_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xcb-keysyms_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xcb-keysyms_DEPS_TARGET
                              xorg_xorg_xcb-keysyms_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xcb-keysyms"
                              "${xorg_xorg_xcb-keysyms_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xcb-keysyms
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-keysyms_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-keysyms_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xcb-keysyms_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xcb-keysyms
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xcb-keysyms_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xcb-keysyms APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-keysyms_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xcb-keysyms APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-keysyms_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-keysyms APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-keysyms_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-keysyms APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-keysyms_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xcb-keysyms APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-keysyms_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xcb-image #############

        set(xorg_xorg_xcb-image_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xcb-image_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xcb-image_FRAMEWORKS_RELEASE}" "${xorg_xorg_xcb-image_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xcb-image_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xcb-image_DEPS_TARGET)
            add_library(xorg_xorg_xcb-image_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xcb-image_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-image_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-image_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-image_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xcb-image_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xcb-image_LIBS_RELEASE}"
                              "${xorg_xorg_xcb-image_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xcb-image_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xcb-image_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xcb-image_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xcb-image_DEPS_TARGET
                              xorg_xorg_xcb-image_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xcb-image"
                              "${xorg_xorg_xcb-image_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xcb-image
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-image_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-image_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xcb-image_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xcb-image
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xcb-image_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xcb-image APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-image_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xcb-image APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-image_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-image APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-image_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-image APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-image_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xcb-image APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-image_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xcb-icccm #############

        set(xorg_xorg_xcb-icccm_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xcb-icccm_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xcb-icccm_FRAMEWORKS_RELEASE}" "${xorg_xorg_xcb-icccm_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xcb-icccm_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xcb-icccm_DEPS_TARGET)
            add_library(xorg_xorg_xcb-icccm_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xcb-icccm_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-icccm_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-icccm_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-icccm_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xcb-icccm_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xcb-icccm_LIBS_RELEASE}"
                              "${xorg_xorg_xcb-icccm_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xcb-icccm_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xcb-icccm_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xcb-icccm_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xcb-icccm_DEPS_TARGET
                              xorg_xorg_xcb-icccm_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xcb-icccm"
                              "${xorg_xorg_xcb-icccm_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xcb-icccm
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-icccm_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-icccm_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xcb-icccm_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xcb-icccm
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xcb-icccm_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xcb-icccm APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-icccm_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xcb-icccm APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-icccm_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-icccm APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-icccm_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-icccm APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-icccm_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xcb-icccm APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-icccm_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xcb-xkb #############

        set(xorg_xorg_xcb-xkb_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xcb-xkb_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xcb-xkb_FRAMEWORKS_RELEASE}" "${xorg_xorg_xcb-xkb_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xcb-xkb_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xcb-xkb_DEPS_TARGET)
            add_library(xorg_xorg_xcb-xkb_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xcb-xkb_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xkb_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xkb_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xkb_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xcb-xkb_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xcb-xkb_LIBS_RELEASE}"
                              "${xorg_xorg_xcb-xkb_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xcb-xkb_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xcb-xkb_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xcb-xkb_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xcb-xkb_DEPS_TARGET
                              xorg_xorg_xcb-xkb_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xcb-xkb"
                              "${xorg_xorg_xcb-xkb_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xcb-xkb
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xkb_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xkb_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xcb-xkb_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xcb-xkb
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xcb-xkb_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xcb-xkb APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xkb_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xcb-xkb APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xkb_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-xkb APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xkb_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcb-xkb APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xkb_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xcb-xkb APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcb-xkb_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xxf86vm #############

        set(xorg_xorg_xxf86vm_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xxf86vm_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xxf86vm_FRAMEWORKS_RELEASE}" "${xorg_xorg_xxf86vm_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xxf86vm_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xxf86vm_DEPS_TARGET)
            add_library(xorg_xorg_xxf86vm_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xxf86vm_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xxf86vm_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xxf86vm_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xxf86vm_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xxf86vm_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xxf86vm_LIBS_RELEASE}"
                              "${xorg_xorg_xxf86vm_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xxf86vm_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xxf86vm_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xxf86vm_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xxf86vm_DEPS_TARGET
                              xorg_xorg_xxf86vm_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xxf86vm"
                              "${xorg_xorg_xxf86vm_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xxf86vm
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xxf86vm_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xxf86vm_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xxf86vm_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xxf86vm
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xxf86vm_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xxf86vm APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xxf86vm_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xxf86vm APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xxf86vm_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xxf86vm APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xxf86vm_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xxf86vm APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xxf86vm_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xxf86vm APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xxf86vm_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xv #############

        set(xorg_xorg_xv_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xv_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xv_FRAMEWORKS_RELEASE}" "${xorg_xorg_xv_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xv_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xv_DEPS_TARGET)
            add_library(xorg_xorg_xv_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xv_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xv_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xv_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xv_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xv_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xv_LIBS_RELEASE}"
                              "${xorg_xorg_xv_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xv_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xv_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xv_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xv_DEPS_TARGET
                              xorg_xorg_xv_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xv"
                              "${xorg_xorg_xv_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xv
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xv_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xv_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xv_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xv
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xv_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xv APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xv_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xv APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xv_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xv APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xv_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xv APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xv_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xv APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xv_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xtst #############

        set(xorg_xorg_xtst_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xtst_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xtst_FRAMEWORKS_RELEASE}" "${xorg_xorg_xtst_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xtst_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xtst_DEPS_TARGET)
            add_library(xorg_xorg_xtst_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xtst_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xtst_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xtst_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xtst_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xtst_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xtst_LIBS_RELEASE}"
                              "${xorg_xorg_xtst_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xtst_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xtst_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xtst_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xtst_DEPS_TARGET
                              xorg_xorg_xtst_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xtst"
                              "${xorg_xorg_xtst_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xtst
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xtst_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xtst_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xtst_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xtst
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xtst_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xtst APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xtst_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xtst APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xtst_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xtst APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xtst_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xtst APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xtst_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xtst APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xtst_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xt #############

        set(xorg_xorg_xt_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xt_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xt_FRAMEWORKS_RELEASE}" "${xorg_xorg_xt_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xt_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xt_DEPS_TARGET)
            add_library(xorg_xorg_xt_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xt_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xt_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xt_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xt_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xt_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xt_LIBS_RELEASE}"
                              "${xorg_xorg_xt_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xt_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xt_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xt_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xt_DEPS_TARGET
                              xorg_xorg_xt_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xt"
                              "${xorg_xorg_xt_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xt
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xt_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xt_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xt_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xt
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xt_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xt APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xt_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xt APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xt_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xt APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xt_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xt APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xt_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xt APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xt_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xscrnsaver #############

        set(xorg_xorg_xscrnsaver_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xscrnsaver_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xscrnsaver_FRAMEWORKS_RELEASE}" "${xorg_xorg_xscrnsaver_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xscrnsaver_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xscrnsaver_DEPS_TARGET)
            add_library(xorg_xorg_xscrnsaver_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xscrnsaver_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xscrnsaver_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xscrnsaver_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xscrnsaver_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xscrnsaver_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xscrnsaver_LIBS_RELEASE}"
                              "${xorg_xorg_xscrnsaver_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xscrnsaver_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xscrnsaver_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xscrnsaver_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xscrnsaver_DEPS_TARGET
                              xorg_xorg_xscrnsaver_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xscrnsaver"
                              "${xorg_xorg_xscrnsaver_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xscrnsaver
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xscrnsaver_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xscrnsaver_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xscrnsaver_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xscrnsaver
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xscrnsaver_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xscrnsaver APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xscrnsaver_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xscrnsaver APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xscrnsaver_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xscrnsaver APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xscrnsaver_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xscrnsaver APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xscrnsaver_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xscrnsaver APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xscrnsaver_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xres #############

        set(xorg_xorg_xres_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xres_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xres_FRAMEWORKS_RELEASE}" "${xorg_xorg_xres_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xres_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xres_DEPS_TARGET)
            add_library(xorg_xorg_xres_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xres_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xres_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xres_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xres_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xres_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xres_LIBS_RELEASE}"
                              "${xorg_xorg_xres_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xres_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xres_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xres_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xres_DEPS_TARGET
                              xorg_xorg_xres_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xres"
                              "${xorg_xorg_xres_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xres
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xres_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xres_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xres_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xres
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xres_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xres APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xres_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xres APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xres_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xres APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xres_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xres APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xres_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xres APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xres_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xrender #############

        set(xorg_xorg_xrender_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xrender_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xrender_FRAMEWORKS_RELEASE}" "${xorg_xorg_xrender_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xrender_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xrender_DEPS_TARGET)
            add_library(xorg_xorg_xrender_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xrender_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xrender_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xrender_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xrender_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xrender_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xrender_LIBS_RELEASE}"
                              "${xorg_xorg_xrender_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xrender_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xrender_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xrender_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xrender_DEPS_TARGET
                              xorg_xorg_xrender_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xrender"
                              "${xorg_xorg_xrender_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xrender
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xrender_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xrender_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xrender_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xrender
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xrender_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xrender APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xrender_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xrender APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xrender_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xrender APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xrender_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xrender APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xrender_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xrender APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xrender_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xrandr #############

        set(xorg_xorg_xrandr_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xrandr_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xrandr_FRAMEWORKS_RELEASE}" "${xorg_xorg_xrandr_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xrandr_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xrandr_DEPS_TARGET)
            add_library(xorg_xorg_xrandr_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xrandr_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xrandr_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xrandr_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xrandr_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xrandr_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xrandr_LIBS_RELEASE}"
                              "${xorg_xorg_xrandr_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xrandr_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xrandr_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xrandr_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xrandr_DEPS_TARGET
                              xorg_xorg_xrandr_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xrandr"
                              "${xorg_xorg_xrandr_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xrandr
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xrandr_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xrandr_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xrandr_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xrandr
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xrandr_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xrandr APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xrandr_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xrandr APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xrandr_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xrandr APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xrandr_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xrandr APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xrandr_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xrandr APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xrandr_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xpm #############

        set(xorg_xorg_xpm_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xpm_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xpm_FRAMEWORKS_RELEASE}" "${xorg_xorg_xpm_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xpm_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xpm_DEPS_TARGET)
            add_library(xorg_xorg_xpm_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xpm_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xpm_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xpm_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xpm_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xpm_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xpm_LIBS_RELEASE}"
                              "${xorg_xorg_xpm_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xpm_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xpm_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xpm_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xpm_DEPS_TARGET
                              xorg_xorg_xpm_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xpm"
                              "${xorg_xorg_xpm_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xpm
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xpm_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xpm_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xpm_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xpm
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xpm_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xpm APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xpm_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xpm APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xpm_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xpm APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xpm_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xpm APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xpm_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xpm APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xpm_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xmuu #############

        set(xorg_xorg_xmuu_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xmuu_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xmuu_FRAMEWORKS_RELEASE}" "${xorg_xorg_xmuu_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xmuu_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xmuu_DEPS_TARGET)
            add_library(xorg_xorg_xmuu_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xmuu_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xmuu_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xmuu_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xmuu_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xmuu_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xmuu_LIBS_RELEASE}"
                              "${xorg_xorg_xmuu_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xmuu_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xmuu_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xmuu_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xmuu_DEPS_TARGET
                              xorg_xorg_xmuu_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xmuu"
                              "${xorg_xorg_xmuu_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xmuu
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xmuu_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xmuu_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xmuu_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xmuu
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xmuu_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xmuu APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xmuu_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xmuu APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xmuu_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xmuu APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xmuu_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xmuu APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xmuu_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xmuu APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xmuu_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xmu #############

        set(xorg_xorg_xmu_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xmu_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xmu_FRAMEWORKS_RELEASE}" "${xorg_xorg_xmu_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xmu_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xmu_DEPS_TARGET)
            add_library(xorg_xorg_xmu_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xmu_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xmu_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xmu_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xmu_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xmu_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xmu_LIBS_RELEASE}"
                              "${xorg_xorg_xmu_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xmu_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xmu_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xmu_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xmu_DEPS_TARGET
                              xorg_xorg_xmu_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xmu"
                              "${xorg_xorg_xmu_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xmu
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xmu_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xmu_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xmu_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xmu
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xmu_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xmu APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xmu_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xmu APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xmu_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xmu APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xmu_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xmu APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xmu_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xmu APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xmu_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xkbfile #############

        set(xorg_xorg_xkbfile_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xkbfile_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xkbfile_FRAMEWORKS_RELEASE}" "${xorg_xorg_xkbfile_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xkbfile_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xkbfile_DEPS_TARGET)
            add_library(xorg_xorg_xkbfile_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xkbfile_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xkbfile_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xkbfile_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xkbfile_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xkbfile_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xkbfile_LIBS_RELEASE}"
                              "${xorg_xorg_xkbfile_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xkbfile_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xkbfile_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xkbfile_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xkbfile_DEPS_TARGET
                              xorg_xorg_xkbfile_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xkbfile"
                              "${xorg_xorg_xkbfile_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xkbfile
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xkbfile_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xkbfile_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xkbfile_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xkbfile
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xkbfile_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xkbfile APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xkbfile_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xkbfile APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xkbfile_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xkbfile APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xkbfile_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xkbfile APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xkbfile_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xkbfile APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xkbfile_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xinerama #############

        set(xorg_xorg_xinerama_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xinerama_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xinerama_FRAMEWORKS_RELEASE}" "${xorg_xorg_xinerama_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xinerama_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xinerama_DEPS_TARGET)
            add_library(xorg_xorg_xinerama_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xinerama_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xinerama_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xinerama_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xinerama_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xinerama_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xinerama_LIBS_RELEASE}"
                              "${xorg_xorg_xinerama_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xinerama_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xinerama_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xinerama_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xinerama_DEPS_TARGET
                              xorg_xorg_xinerama_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xinerama"
                              "${xorg_xorg_xinerama_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xinerama
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xinerama_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xinerama_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xinerama_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xinerama
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xinerama_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xinerama APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xinerama_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xinerama APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xinerama_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xinerama APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xinerama_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xinerama APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xinerama_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xinerama APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xinerama_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xi #############

        set(xorg_xorg_xi_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xi_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xi_FRAMEWORKS_RELEASE}" "${xorg_xorg_xi_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xi_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xi_DEPS_TARGET)
            add_library(xorg_xorg_xi_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xi_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xi_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xi_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xi_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xi_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xi_LIBS_RELEASE}"
                              "${xorg_xorg_xi_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xi_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xi_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xi_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xi_DEPS_TARGET
                              xorg_xorg_xi_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xi"
                              "${xorg_xorg_xi_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xi
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xi_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xi_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xi_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xi
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xi_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xi APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xi_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xi APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xi_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xi APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xi_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xi APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xi_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xi APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xi_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xfixes #############

        set(xorg_xorg_xfixes_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xfixes_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xfixes_FRAMEWORKS_RELEASE}" "${xorg_xorg_xfixes_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xfixes_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xfixes_DEPS_TARGET)
            add_library(xorg_xorg_xfixes_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xfixes_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xfixes_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xfixes_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xfixes_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xfixes_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xfixes_LIBS_RELEASE}"
                              "${xorg_xorg_xfixes_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xfixes_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xfixes_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xfixes_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xfixes_DEPS_TARGET
                              xorg_xorg_xfixes_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xfixes"
                              "${xorg_xorg_xfixes_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xfixes
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xfixes_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xfixes_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xfixes_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xfixes
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xfixes_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xfixes APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xfixes_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xfixes APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xfixes_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xfixes APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xfixes_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xfixes APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xfixes_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xfixes APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xfixes_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xext #############

        set(xorg_xorg_xext_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xext_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xext_FRAMEWORKS_RELEASE}" "${xorg_xorg_xext_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xext_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xext_DEPS_TARGET)
            add_library(xorg_xorg_xext_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xext_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xext_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xext_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xext_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xext_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xext_LIBS_RELEASE}"
                              "${xorg_xorg_xext_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xext_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xext_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xext_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xext_DEPS_TARGET
                              xorg_xorg_xext_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xext"
                              "${xorg_xorg_xext_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xext
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xext_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xext_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xext_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xext
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xext_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xext APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xext_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xext APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xext_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xext APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xext_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xext APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xext_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xext APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xext_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xdmcp #############

        set(xorg_xorg_xdmcp_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xdmcp_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xdmcp_FRAMEWORKS_RELEASE}" "${xorg_xorg_xdmcp_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xdmcp_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xdmcp_DEPS_TARGET)
            add_library(xorg_xorg_xdmcp_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xdmcp_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xdmcp_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xdmcp_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xdmcp_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xdmcp_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xdmcp_LIBS_RELEASE}"
                              "${xorg_xorg_xdmcp_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xdmcp_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xdmcp_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xdmcp_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xdmcp_DEPS_TARGET
                              xorg_xorg_xdmcp_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xdmcp"
                              "${xorg_xorg_xdmcp_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xdmcp
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xdmcp_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xdmcp_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xdmcp_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xdmcp
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xdmcp_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xdmcp APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xdmcp_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xdmcp APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xdmcp_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xdmcp APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xdmcp_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xdmcp APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xdmcp_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xdmcp APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xdmcp_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xdamage #############

        set(xorg_xorg_xdamage_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xdamage_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xdamage_FRAMEWORKS_RELEASE}" "${xorg_xorg_xdamage_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xdamage_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xdamage_DEPS_TARGET)
            add_library(xorg_xorg_xdamage_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xdamage_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xdamage_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xdamage_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xdamage_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xdamage_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xdamage_LIBS_RELEASE}"
                              "${xorg_xorg_xdamage_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xdamage_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xdamage_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xdamage_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xdamage_DEPS_TARGET
                              xorg_xorg_xdamage_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xdamage"
                              "${xorg_xorg_xdamage_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xdamage
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xdamage_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xdamage_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xdamage_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xdamage
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xdamage_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xdamage APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xdamage_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xdamage APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xdamage_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xdamage APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xdamage_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xdamage APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xdamage_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xdamage APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xdamage_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xcursor #############

        set(xorg_xorg_xcursor_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xcursor_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xcursor_FRAMEWORKS_RELEASE}" "${xorg_xorg_xcursor_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xcursor_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xcursor_DEPS_TARGET)
            add_library(xorg_xorg_xcursor_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xcursor_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcursor_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcursor_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcursor_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xcursor_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xcursor_LIBS_RELEASE}"
                              "${xorg_xorg_xcursor_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xcursor_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xcursor_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xcursor_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xcursor_DEPS_TARGET
                              xorg_xorg_xcursor_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xcursor"
                              "${xorg_xorg_xcursor_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xcursor
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcursor_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcursor_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xcursor_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xcursor
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xcursor_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xcursor APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcursor_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xcursor APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcursor_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcursor APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcursor_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcursor APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcursor_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xcursor APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcursor_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xcomposite #############

        set(xorg_xorg_xcomposite_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xcomposite_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xcomposite_FRAMEWORKS_RELEASE}" "${xorg_xorg_xcomposite_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xcomposite_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xcomposite_DEPS_TARGET)
            add_library(xorg_xorg_xcomposite_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xcomposite_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcomposite_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcomposite_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcomposite_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xcomposite_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xcomposite_LIBS_RELEASE}"
                              "${xorg_xorg_xcomposite_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xcomposite_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xcomposite_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xcomposite_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xcomposite_DEPS_TARGET
                              xorg_xorg_xcomposite_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xcomposite"
                              "${xorg_xorg_xcomposite_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xcomposite
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcomposite_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xcomposite_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xcomposite_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xcomposite
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xcomposite_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xcomposite APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcomposite_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xcomposite APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcomposite_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcomposite APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xcomposite_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xcomposite APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcomposite_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xcomposite APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xcomposite_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xaw7 #############

        set(xorg_xorg_xaw7_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xaw7_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xaw7_FRAMEWORKS_RELEASE}" "${xorg_xorg_xaw7_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xaw7_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xaw7_DEPS_TARGET)
            add_library(xorg_xorg_xaw7_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xaw7_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xaw7_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xaw7_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xaw7_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xaw7_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xaw7_LIBS_RELEASE}"
                              "${xorg_xorg_xaw7_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xaw7_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xaw7_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xaw7_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xaw7_DEPS_TARGET
                              xorg_xorg_xaw7_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xaw7"
                              "${xorg_xorg_xaw7_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xaw7
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xaw7_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xaw7_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xaw7_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xaw7
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xaw7_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xaw7 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xaw7_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xaw7 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xaw7_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xaw7 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xaw7_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xaw7 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xaw7_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xaw7 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xaw7_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::xau #############

        set(xorg_xorg_xau_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_xau_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_xau_FRAMEWORKS_RELEASE}" "${xorg_xorg_xau_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_xau_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_xau_DEPS_TARGET)
            add_library(xorg_xorg_xau_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_xau_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xau_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xau_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xau_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_xau_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_xau_LIBS_RELEASE}"
                              "${xorg_xorg_xau_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_xau_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_xau_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_xau_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_xau_DEPS_TARGET
                              xorg_xorg_xau_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_xau"
                              "${xorg_xorg_xau_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::xau
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_xau_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_xau_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_xau_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::xau
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_xau_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::xau APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xau_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::xau APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xau_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::xau APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_xau_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::xau APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xau_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::xau APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_xau_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::ice #############

        set(xorg_xorg_ice_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_ice_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_ice_FRAMEWORKS_RELEASE}" "${xorg_xorg_ice_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_ice_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_ice_DEPS_TARGET)
            add_library(xorg_xorg_ice_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_ice_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_ice_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_ice_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_ice_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_ice_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_ice_LIBS_RELEASE}"
                              "${xorg_xorg_ice_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_ice_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_ice_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_ice_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_ice_DEPS_TARGET
                              xorg_xorg_ice_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_ice"
                              "${xorg_xorg_ice_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::ice
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_ice_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_ice_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_ice_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::ice
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_ice_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::ice APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_ice_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::ice APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_ice_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::ice APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_ice_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::ice APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_ice_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::ice APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_ice_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::fontenc #############

        set(xorg_xorg_fontenc_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_fontenc_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_fontenc_FRAMEWORKS_RELEASE}" "${xorg_xorg_fontenc_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_fontenc_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_fontenc_DEPS_TARGET)
            add_library(xorg_xorg_fontenc_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_fontenc_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_fontenc_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_fontenc_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_fontenc_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_fontenc_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_fontenc_LIBS_RELEASE}"
                              "${xorg_xorg_fontenc_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_fontenc_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_fontenc_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_fontenc_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_fontenc_DEPS_TARGET
                              xorg_xorg_fontenc_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_fontenc"
                              "${xorg_xorg_fontenc_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::fontenc
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_fontenc_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_fontenc_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_fontenc_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::fontenc
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_fontenc_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::fontenc APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_fontenc_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::fontenc APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_fontenc_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::fontenc APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_fontenc_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::fontenc APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_fontenc_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::fontenc APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_fontenc_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::x11-xcb #############

        set(xorg_xorg_x11-xcb_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_x11-xcb_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_x11-xcb_FRAMEWORKS_RELEASE}" "${xorg_xorg_x11-xcb_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_x11-xcb_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_x11-xcb_DEPS_TARGET)
            add_library(xorg_xorg_x11-xcb_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_x11-xcb_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_x11-xcb_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_x11-xcb_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_x11-xcb_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_x11-xcb_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_x11-xcb_LIBS_RELEASE}"
                              "${xorg_xorg_x11-xcb_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_x11-xcb_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_x11-xcb_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_x11-xcb_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_x11-xcb_DEPS_TARGET
                              xorg_xorg_x11-xcb_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_x11-xcb"
                              "${xorg_xorg_x11-xcb_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::x11-xcb
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_x11-xcb_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_x11-xcb_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_x11-xcb_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::x11-xcb
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_x11-xcb_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::x11-xcb APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_x11-xcb_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::x11-xcb APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_x11-xcb_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::x11-xcb APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_x11-xcb_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::x11-xcb APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_x11-xcb_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::x11-xcb APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_x11-xcb_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT xorg::x11 #############

        set(xorg_xorg_x11_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(xorg_xorg_x11_FRAMEWORKS_FOUND_RELEASE "${xorg_xorg_x11_FRAMEWORKS_RELEASE}" "${xorg_xorg_x11_FRAMEWORK_DIRS_RELEASE}")

        set(xorg_xorg_x11_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET xorg_xorg_x11_DEPS_TARGET)
            add_library(xorg_xorg_x11_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET xorg_xorg_x11_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_x11_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_x11_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_x11_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'xorg_xorg_x11_DEPS_TARGET' to all of them
        conan_package_library_targets("${xorg_xorg_x11_LIBS_RELEASE}"
                              "${xorg_xorg_x11_LIB_DIRS_RELEASE}"
                              "${xorg_xorg_x11_BIN_DIRS_RELEASE}" # package_bindir
                              "${xorg_xorg_x11_LIBRARY_TYPE_RELEASE}"
                              "${xorg_xorg_x11_IS_HOST_WINDOWS_RELEASE}"
                              xorg_xorg_x11_DEPS_TARGET
                              xorg_xorg_x11_LIBRARIES_TARGETS
                              "_RELEASE"
                              "xorg_xorg_x11"
                              "${xorg_xorg_x11_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET xorg::x11
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${xorg_xorg_x11_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${xorg_xorg_x11_LIBRARIES_TARGETS}>
                     )

        if("${xorg_xorg_x11_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET xorg::x11
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         xorg_xorg_x11_DEPS_TARGET)
        endif()

        set_property(TARGET xorg::x11 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_x11_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET xorg::x11 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_x11_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET xorg::x11 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${xorg_xorg_x11_LIB_DIRS_RELEASE}>)
        set_property(TARGET xorg::x11 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${xorg_xorg_x11_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET xorg::x11 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${xorg_xorg_x11_COMPILE_OPTIONS_RELEASE}>)

    ########## AGGREGATED GLOBAL TARGET WITH THE COMPONENTS #####################
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::sm)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::uuid)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xcb-res)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xcb-ewmh)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xcb-composite)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xcb-present)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xcb-glx)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xcb-dri2)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xcb-cursor)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xcb-dri3)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xcb-util)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xcb-event)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xcb-aux)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xcb-atom)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xcb)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xcb-xinerama)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xcb-xfixes)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xcb-sync)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xcb-shm)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xcb-shape)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xcb-renderutil)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xcb-render)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xcb-randr)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xcb-keysyms)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xcb-image)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xcb-icccm)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xcb-xkb)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xxf86vm)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xv)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xtst)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xt)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xscrnsaver)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xres)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xrender)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xrandr)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xpm)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xmuu)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xmu)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xkbfile)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xinerama)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xi)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xfixes)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xext)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xdmcp)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xdamage)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xcursor)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xcomposite)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xaw7)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::xau)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::ice)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::fontenc)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::x11-xcb)
    set_property(TARGET xorg::xorg APPEND PROPERTY INTERFACE_LINK_LIBRARIES xorg::x11)

########## For the modules (FindXXX)
set(xorg_LIBRARIES_RELEASE xorg::xorg)
