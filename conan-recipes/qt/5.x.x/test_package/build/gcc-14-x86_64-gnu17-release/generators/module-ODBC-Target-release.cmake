# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(odbc_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(odbc_FRAMEWORKS_FOUND_RELEASE "${odbc_FRAMEWORKS_RELEASE}" "${odbc_FRAMEWORK_DIRS_RELEASE}")

set(odbc_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET odbc_DEPS_TARGET)
    add_library(odbc_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET odbc_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${odbc_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${odbc_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:libtool::libtool>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### odbc_DEPS_TARGET to all of them
conan_package_library_targets("${odbc_LIBS_RELEASE}"    # libraries
                              "${odbc_LIB_DIRS_RELEASE}" # package_libdir
                              "${odbc_BIN_DIRS_RELEASE}" # package_bindir
                              "${odbc_LIBRARY_TYPE_RELEASE}"
                              "${odbc_IS_HOST_WINDOWS_RELEASE}"
                              odbc_DEPS_TARGET
                              odbc_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "odbc"    # package_name
                              "${odbc_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${odbc_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## COMPONENTS TARGET PROPERTIES Release ########################################

    ########## COMPONENT odbc::odbccr #############

        set(odbc_odbc_odbccr_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(odbc_odbc_odbccr_FRAMEWORKS_FOUND_RELEASE "${odbc_odbc_odbccr_FRAMEWORKS_RELEASE}" "${odbc_odbc_odbccr_FRAMEWORK_DIRS_RELEASE}")

        set(odbc_odbc_odbccr_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET odbc_odbc_odbccr_DEPS_TARGET)
            add_library(odbc_odbc_odbccr_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET odbc_odbc_odbccr_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${odbc_odbc_odbccr_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${odbc_odbc_odbccr_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${odbc_odbc_odbccr_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'odbc_odbc_odbccr_DEPS_TARGET' to all of them
        conan_package_library_targets("${odbc_odbc_odbccr_LIBS_RELEASE}"
                              "${odbc_odbc_odbccr_LIB_DIRS_RELEASE}"
                              "${odbc_odbc_odbccr_BIN_DIRS_RELEASE}" # package_bindir
                              "${odbc_odbc_odbccr_LIBRARY_TYPE_RELEASE}"
                              "${odbc_odbc_odbccr_IS_HOST_WINDOWS_RELEASE}"
                              odbc_odbc_odbccr_DEPS_TARGET
                              odbc_odbc_odbccr_LIBRARIES_TARGETS
                              "_RELEASE"
                              "odbc_odbc_odbccr"
                              "${odbc_odbc_odbccr_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET odbc::odbccr
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${odbc_odbc_odbccr_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${odbc_odbc_odbccr_LIBRARIES_TARGETS}>
                     )

        if("${odbc_odbc_odbccr_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET odbc::odbccr
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         odbc_odbc_odbccr_DEPS_TARGET)
        endif()

        set_property(TARGET odbc::odbccr APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${odbc_odbc_odbccr_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET odbc::odbccr APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${odbc_odbc_odbccr_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET odbc::odbccr APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${odbc_odbc_odbccr_LIB_DIRS_RELEASE}>)
        set_property(TARGET odbc::odbccr APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${odbc_odbc_odbccr_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET odbc::odbccr APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${odbc_odbc_odbccr_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT odbc::odbcinst #############

        set(odbc_odbc_odbcinst_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(odbc_odbc_odbcinst_FRAMEWORKS_FOUND_RELEASE "${odbc_odbc_odbcinst_FRAMEWORKS_RELEASE}" "${odbc_odbc_odbcinst_FRAMEWORK_DIRS_RELEASE}")

        set(odbc_odbc_odbcinst_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET odbc_odbc_odbcinst_DEPS_TARGET)
            add_library(odbc_odbc_odbcinst_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET odbc_odbc_odbcinst_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${odbc_odbc_odbcinst_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${odbc_odbc_odbcinst_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${odbc_odbc_odbcinst_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'odbc_odbc_odbcinst_DEPS_TARGET' to all of them
        conan_package_library_targets("${odbc_odbc_odbcinst_LIBS_RELEASE}"
                              "${odbc_odbc_odbcinst_LIB_DIRS_RELEASE}"
                              "${odbc_odbc_odbcinst_BIN_DIRS_RELEASE}" # package_bindir
                              "${odbc_odbc_odbcinst_LIBRARY_TYPE_RELEASE}"
                              "${odbc_odbc_odbcinst_IS_HOST_WINDOWS_RELEASE}"
                              odbc_odbc_odbcinst_DEPS_TARGET
                              odbc_odbc_odbcinst_LIBRARIES_TARGETS
                              "_RELEASE"
                              "odbc_odbc_odbcinst"
                              "${odbc_odbc_odbcinst_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET odbc::odbcinst
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${odbc_odbc_odbcinst_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${odbc_odbc_odbcinst_LIBRARIES_TARGETS}>
                     )

        if("${odbc_odbc_odbcinst_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET odbc::odbcinst
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         odbc_odbc_odbcinst_DEPS_TARGET)
        endif()

        set_property(TARGET odbc::odbcinst APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${odbc_odbc_odbcinst_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET odbc::odbcinst APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${odbc_odbc_odbcinst_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET odbc::odbcinst APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${odbc_odbc_odbcinst_LIB_DIRS_RELEASE}>)
        set_property(TARGET odbc::odbcinst APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${odbc_odbc_odbcinst_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET odbc::odbcinst APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${odbc_odbc_odbcinst_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT odbc::_odbc #############

        set(odbc_odbc__odbc_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(odbc_odbc__odbc_FRAMEWORKS_FOUND_RELEASE "${odbc_odbc__odbc_FRAMEWORKS_RELEASE}" "${odbc_odbc__odbc_FRAMEWORK_DIRS_RELEASE}")

        set(odbc_odbc__odbc_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET odbc_odbc__odbc_DEPS_TARGET)
            add_library(odbc_odbc__odbc_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET odbc_odbc__odbc_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${odbc_odbc__odbc_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${odbc_odbc__odbc_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${odbc_odbc__odbc_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'odbc_odbc__odbc_DEPS_TARGET' to all of them
        conan_package_library_targets("${odbc_odbc__odbc_LIBS_RELEASE}"
                              "${odbc_odbc__odbc_LIB_DIRS_RELEASE}"
                              "${odbc_odbc__odbc_BIN_DIRS_RELEASE}" # package_bindir
                              "${odbc_odbc__odbc_LIBRARY_TYPE_RELEASE}"
                              "${odbc_odbc__odbc_IS_HOST_WINDOWS_RELEASE}"
                              odbc_odbc__odbc_DEPS_TARGET
                              odbc_odbc__odbc_LIBRARIES_TARGETS
                              "_RELEASE"
                              "odbc_odbc__odbc"
                              "${odbc_odbc__odbc_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET odbc::_odbc
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${odbc_odbc__odbc_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${odbc_odbc__odbc_LIBRARIES_TARGETS}>
                     )

        if("${odbc_odbc__odbc_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET odbc::_odbc
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         odbc_odbc__odbc_DEPS_TARGET)
        endif()

        set_property(TARGET odbc::_odbc APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${odbc_odbc__odbc_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET odbc::_odbc APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${odbc_odbc__odbc_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET odbc::_odbc APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${odbc_odbc__odbc_LIB_DIRS_RELEASE}>)
        set_property(TARGET odbc::_odbc APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${odbc_odbc__odbc_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET odbc::_odbc APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${odbc_odbc__odbc_COMPILE_OPTIONS_RELEASE}>)

    ########## AGGREGATED GLOBAL TARGET WITH THE COMPONENTS #####################
    set_property(TARGET ODBC::ODBC APPEND PROPERTY INTERFACE_LINK_LIBRARIES odbc::odbccr)
    set_property(TARGET ODBC::ODBC APPEND PROPERTY INTERFACE_LINK_LIBRARIES odbc::odbcinst)
    set_property(TARGET ODBC::ODBC APPEND PROPERTY INTERFACE_LINK_LIBRARIES odbc::_odbc)

########## For the modules (FindXXX)
set(odbc_LIBRARIES_RELEASE ODBC::ODBC)
