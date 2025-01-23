# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(nss_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(nss_FRAMEWORKS_FOUND_RELEASE "${nss_FRAMEWORKS_RELEASE}" "${nss_FRAMEWORK_DIRS_RELEASE}")

set(nss_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET nss_DEPS_TARGET)
    add_library(nss_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET nss_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${nss_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${nss_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:nss::libnss;nss::ssl;nss::smime;nss::util;nspr::nspr;nss::freebl>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### nss_DEPS_TARGET to all of them
conan_package_library_targets("${nss_LIBS_RELEASE}"    # libraries
                              "${nss_LIB_DIRS_RELEASE}" # package_libdir
                              "${nss_BIN_DIRS_RELEASE}" # package_bindir
                              "${nss_LIBRARY_TYPE_RELEASE}"
                              "${nss_IS_HOST_WINDOWS_RELEASE}"
                              nss_DEPS_TARGET
                              nss_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "nss"    # package_name
                              "${nss_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${nss_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## COMPONENTS TARGET PROPERTIES Release ########################################

    ########## COMPONENT nss::nss_pc #############

        set(nss_nss_nss_pc_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(nss_nss_nss_pc_FRAMEWORKS_FOUND_RELEASE "${nss_nss_nss_pc_FRAMEWORKS_RELEASE}" "${nss_nss_nss_pc_FRAMEWORK_DIRS_RELEASE}")

        set(nss_nss_nss_pc_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET nss_nss_nss_pc_DEPS_TARGET)
            add_library(nss_nss_nss_pc_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET nss_nss_nss_pc_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${nss_nss_nss_pc_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${nss_nss_nss_pc_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${nss_nss_nss_pc_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'nss_nss_nss_pc_DEPS_TARGET' to all of them
        conan_package_library_targets("${nss_nss_nss_pc_LIBS_RELEASE}"
                              "${nss_nss_nss_pc_LIB_DIRS_RELEASE}"
                              "${nss_nss_nss_pc_BIN_DIRS_RELEASE}" # package_bindir
                              "${nss_nss_nss_pc_LIBRARY_TYPE_RELEASE}"
                              "${nss_nss_nss_pc_IS_HOST_WINDOWS_RELEASE}"
                              nss_nss_nss_pc_DEPS_TARGET
                              nss_nss_nss_pc_LIBRARIES_TARGETS
                              "_RELEASE"
                              "nss_nss_nss_pc"
                              "${nss_nss_nss_pc_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET nss::nss_pc
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${nss_nss_nss_pc_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${nss_nss_nss_pc_LIBRARIES_TARGETS}>
                     )

        if("${nss_nss_nss_pc_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET nss::nss_pc
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         nss_nss_nss_pc_DEPS_TARGET)
        endif()

        set_property(TARGET nss::nss_pc APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${nss_nss_nss_pc_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET nss::nss_pc APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${nss_nss_nss_pc_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET nss::nss_pc APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${nss_nss_nss_pc_LIB_DIRS_RELEASE}>)
        set_property(TARGET nss::nss_pc APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${nss_nss_nss_pc_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET nss::nss_pc APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${nss_nss_nss_pc_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT nss::smime #############

        set(nss_nss_smime_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(nss_nss_smime_FRAMEWORKS_FOUND_RELEASE "${nss_nss_smime_FRAMEWORKS_RELEASE}" "${nss_nss_smime_FRAMEWORK_DIRS_RELEASE}")

        set(nss_nss_smime_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET nss_nss_smime_DEPS_TARGET)
            add_library(nss_nss_smime_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET nss_nss_smime_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${nss_nss_smime_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${nss_nss_smime_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${nss_nss_smime_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'nss_nss_smime_DEPS_TARGET' to all of them
        conan_package_library_targets("${nss_nss_smime_LIBS_RELEASE}"
                              "${nss_nss_smime_LIB_DIRS_RELEASE}"
                              "${nss_nss_smime_BIN_DIRS_RELEASE}" # package_bindir
                              "${nss_nss_smime_LIBRARY_TYPE_RELEASE}"
                              "${nss_nss_smime_IS_HOST_WINDOWS_RELEASE}"
                              nss_nss_smime_DEPS_TARGET
                              nss_nss_smime_LIBRARIES_TARGETS
                              "_RELEASE"
                              "nss_nss_smime"
                              "${nss_nss_smime_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET nss::smime
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${nss_nss_smime_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${nss_nss_smime_LIBRARIES_TARGETS}>
                     )

        if("${nss_nss_smime_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET nss::smime
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         nss_nss_smime_DEPS_TARGET)
        endif()

        set_property(TARGET nss::smime APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${nss_nss_smime_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET nss::smime APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${nss_nss_smime_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET nss::smime APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${nss_nss_smime_LIB_DIRS_RELEASE}>)
        set_property(TARGET nss::smime APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${nss_nss_smime_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET nss::smime APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${nss_nss_smime_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT nss::ssl #############

        set(nss_nss_ssl_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(nss_nss_ssl_FRAMEWORKS_FOUND_RELEASE "${nss_nss_ssl_FRAMEWORKS_RELEASE}" "${nss_nss_ssl_FRAMEWORK_DIRS_RELEASE}")

        set(nss_nss_ssl_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET nss_nss_ssl_DEPS_TARGET)
            add_library(nss_nss_ssl_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET nss_nss_ssl_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${nss_nss_ssl_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${nss_nss_ssl_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${nss_nss_ssl_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'nss_nss_ssl_DEPS_TARGET' to all of them
        conan_package_library_targets("${nss_nss_ssl_LIBS_RELEASE}"
                              "${nss_nss_ssl_LIB_DIRS_RELEASE}"
                              "${nss_nss_ssl_BIN_DIRS_RELEASE}" # package_bindir
                              "${nss_nss_ssl_LIBRARY_TYPE_RELEASE}"
                              "${nss_nss_ssl_IS_HOST_WINDOWS_RELEASE}"
                              nss_nss_ssl_DEPS_TARGET
                              nss_nss_ssl_LIBRARIES_TARGETS
                              "_RELEASE"
                              "nss_nss_ssl"
                              "${nss_nss_ssl_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET nss::ssl
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${nss_nss_ssl_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${nss_nss_ssl_LIBRARIES_TARGETS}>
                     )

        if("${nss_nss_ssl_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET nss::ssl
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         nss_nss_ssl_DEPS_TARGET)
        endif()

        set_property(TARGET nss::ssl APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${nss_nss_ssl_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET nss::ssl APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${nss_nss_ssl_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET nss::ssl APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${nss_nss_ssl_LIB_DIRS_RELEASE}>)
        set_property(TARGET nss::ssl APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${nss_nss_ssl_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET nss::ssl APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${nss_nss_ssl_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT nss::softokn #############

        set(nss_nss_softokn_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(nss_nss_softokn_FRAMEWORKS_FOUND_RELEASE "${nss_nss_softokn_FRAMEWORKS_RELEASE}" "${nss_nss_softokn_FRAMEWORK_DIRS_RELEASE}")

        set(nss_nss_softokn_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET nss_nss_softokn_DEPS_TARGET)
            add_library(nss_nss_softokn_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET nss_nss_softokn_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${nss_nss_softokn_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${nss_nss_softokn_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${nss_nss_softokn_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'nss_nss_softokn_DEPS_TARGET' to all of them
        conan_package_library_targets("${nss_nss_softokn_LIBS_RELEASE}"
                              "${nss_nss_softokn_LIB_DIRS_RELEASE}"
                              "${nss_nss_softokn_BIN_DIRS_RELEASE}" # package_bindir
                              "${nss_nss_softokn_LIBRARY_TYPE_RELEASE}"
                              "${nss_nss_softokn_IS_HOST_WINDOWS_RELEASE}"
                              nss_nss_softokn_DEPS_TARGET
                              nss_nss_softokn_LIBRARIES_TARGETS
                              "_RELEASE"
                              "nss_nss_softokn"
                              "${nss_nss_softokn_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET nss::softokn
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${nss_nss_softokn_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${nss_nss_softokn_LIBRARIES_TARGETS}>
                     )

        if("${nss_nss_softokn_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET nss::softokn
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         nss_nss_softokn_DEPS_TARGET)
        endif()

        set_property(TARGET nss::softokn APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${nss_nss_softokn_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET nss::softokn APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${nss_nss_softokn_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET nss::softokn APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${nss_nss_softokn_LIB_DIRS_RELEASE}>)
        set_property(TARGET nss::softokn APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${nss_nss_softokn_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET nss::softokn APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${nss_nss_softokn_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT nss::libnss #############

        set(nss_nss_libnss_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(nss_nss_libnss_FRAMEWORKS_FOUND_RELEASE "${nss_nss_libnss_FRAMEWORKS_RELEASE}" "${nss_nss_libnss_FRAMEWORK_DIRS_RELEASE}")

        set(nss_nss_libnss_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET nss_nss_libnss_DEPS_TARGET)
            add_library(nss_nss_libnss_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET nss_nss_libnss_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${nss_nss_libnss_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${nss_nss_libnss_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${nss_nss_libnss_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'nss_nss_libnss_DEPS_TARGET' to all of them
        conan_package_library_targets("${nss_nss_libnss_LIBS_RELEASE}"
                              "${nss_nss_libnss_LIB_DIRS_RELEASE}"
                              "${nss_nss_libnss_BIN_DIRS_RELEASE}" # package_bindir
                              "${nss_nss_libnss_LIBRARY_TYPE_RELEASE}"
                              "${nss_nss_libnss_IS_HOST_WINDOWS_RELEASE}"
                              nss_nss_libnss_DEPS_TARGET
                              nss_nss_libnss_LIBRARIES_TARGETS
                              "_RELEASE"
                              "nss_nss_libnss"
                              "${nss_nss_libnss_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET nss::libnss
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${nss_nss_libnss_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${nss_nss_libnss_LIBRARIES_TARGETS}>
                     )

        if("${nss_nss_libnss_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET nss::libnss
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         nss_nss_libnss_DEPS_TARGET)
        endif()

        set_property(TARGET nss::libnss APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${nss_nss_libnss_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET nss::libnss APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${nss_nss_libnss_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET nss::libnss APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${nss_nss_libnss_LIB_DIRS_RELEASE}>)
        set_property(TARGET nss::libnss APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${nss_nss_libnss_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET nss::libnss APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${nss_nss_libnss_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT nss::tools #############

        set(nss_nss_tools_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(nss_nss_tools_FRAMEWORKS_FOUND_RELEASE "${nss_nss_tools_FRAMEWORKS_RELEASE}" "${nss_nss_tools_FRAMEWORK_DIRS_RELEASE}")

        set(nss_nss_tools_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET nss_nss_tools_DEPS_TARGET)
            add_library(nss_nss_tools_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET nss_nss_tools_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${nss_nss_tools_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${nss_nss_tools_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${nss_nss_tools_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'nss_nss_tools_DEPS_TARGET' to all of them
        conan_package_library_targets("${nss_nss_tools_LIBS_RELEASE}"
                              "${nss_nss_tools_LIB_DIRS_RELEASE}"
                              "${nss_nss_tools_BIN_DIRS_RELEASE}" # package_bindir
                              "${nss_nss_tools_LIBRARY_TYPE_RELEASE}"
                              "${nss_nss_tools_IS_HOST_WINDOWS_RELEASE}"
                              nss_nss_tools_DEPS_TARGET
                              nss_nss_tools_LIBRARIES_TARGETS
                              "_RELEASE"
                              "nss_nss_tools"
                              "${nss_nss_tools_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET nss::tools
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${nss_nss_tools_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${nss_nss_tools_LIBRARIES_TARGETS}>
                     )

        if("${nss_nss_tools_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET nss::tools
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         nss_nss_tools_DEPS_TARGET)
        endif()

        set_property(TARGET nss::tools APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${nss_nss_tools_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET nss::tools APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${nss_nss_tools_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET nss::tools APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${nss_nss_tools_LIB_DIRS_RELEASE}>)
        set_property(TARGET nss::tools APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${nss_nss_tools_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET nss::tools APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${nss_nss_tools_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT nss::freebl #############

        set(nss_nss_freebl_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(nss_nss_freebl_FRAMEWORKS_FOUND_RELEASE "${nss_nss_freebl_FRAMEWORKS_RELEASE}" "${nss_nss_freebl_FRAMEWORK_DIRS_RELEASE}")

        set(nss_nss_freebl_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET nss_nss_freebl_DEPS_TARGET)
            add_library(nss_nss_freebl_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET nss_nss_freebl_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${nss_nss_freebl_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${nss_nss_freebl_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${nss_nss_freebl_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'nss_nss_freebl_DEPS_TARGET' to all of them
        conan_package_library_targets("${nss_nss_freebl_LIBS_RELEASE}"
                              "${nss_nss_freebl_LIB_DIRS_RELEASE}"
                              "${nss_nss_freebl_BIN_DIRS_RELEASE}" # package_bindir
                              "${nss_nss_freebl_LIBRARY_TYPE_RELEASE}"
                              "${nss_nss_freebl_IS_HOST_WINDOWS_RELEASE}"
                              nss_nss_freebl_DEPS_TARGET
                              nss_nss_freebl_LIBRARIES_TARGETS
                              "_RELEASE"
                              "nss_nss_freebl"
                              "${nss_nss_freebl_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET nss::freebl
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${nss_nss_freebl_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${nss_nss_freebl_LIBRARIES_TARGETS}>
                     )

        if("${nss_nss_freebl_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET nss::freebl
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         nss_nss_freebl_DEPS_TARGET)
        endif()

        set_property(TARGET nss::freebl APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${nss_nss_freebl_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET nss::freebl APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${nss_nss_freebl_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET nss::freebl APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${nss_nss_freebl_LIB_DIRS_RELEASE}>)
        set_property(TARGET nss::freebl APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${nss_nss_freebl_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET nss::freebl APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${nss_nss_freebl_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT nss::util #############

        set(nss_nss_util_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(nss_nss_util_FRAMEWORKS_FOUND_RELEASE "${nss_nss_util_FRAMEWORKS_RELEASE}" "${nss_nss_util_FRAMEWORK_DIRS_RELEASE}")

        set(nss_nss_util_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET nss_nss_util_DEPS_TARGET)
            add_library(nss_nss_util_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET nss_nss_util_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${nss_nss_util_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${nss_nss_util_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${nss_nss_util_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'nss_nss_util_DEPS_TARGET' to all of them
        conan_package_library_targets("${nss_nss_util_LIBS_RELEASE}"
                              "${nss_nss_util_LIB_DIRS_RELEASE}"
                              "${nss_nss_util_BIN_DIRS_RELEASE}" # package_bindir
                              "${nss_nss_util_LIBRARY_TYPE_RELEASE}"
                              "${nss_nss_util_IS_HOST_WINDOWS_RELEASE}"
                              nss_nss_util_DEPS_TARGET
                              nss_nss_util_LIBRARIES_TARGETS
                              "_RELEASE"
                              "nss_nss_util"
                              "${nss_nss_util_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET nss::util
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${nss_nss_util_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${nss_nss_util_LIBRARIES_TARGETS}>
                     )

        if("${nss_nss_util_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET nss::util
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         nss_nss_util_DEPS_TARGET)
        endif()

        set_property(TARGET nss::util APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${nss_nss_util_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET nss::util APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${nss_nss_util_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET nss::util APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${nss_nss_util_LIB_DIRS_RELEASE}>)
        set_property(TARGET nss::util APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${nss_nss_util_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET nss::util APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${nss_nss_util_COMPILE_OPTIONS_RELEASE}>)

    ########## AGGREGATED GLOBAL TARGET WITH THE COMPONENTS #####################
    set_property(TARGET nss::nss APPEND PROPERTY INTERFACE_LINK_LIBRARIES nss::nss_pc)
    set_property(TARGET nss::nss APPEND PROPERTY INTERFACE_LINK_LIBRARIES nss::smime)
    set_property(TARGET nss::nss APPEND PROPERTY INTERFACE_LINK_LIBRARIES nss::ssl)
    set_property(TARGET nss::nss APPEND PROPERTY INTERFACE_LINK_LIBRARIES nss::softokn)
    set_property(TARGET nss::nss APPEND PROPERTY INTERFACE_LINK_LIBRARIES nss::libnss)
    set_property(TARGET nss::nss APPEND PROPERTY INTERFACE_LINK_LIBRARIES nss::tools)
    set_property(TARGET nss::nss APPEND PROPERTY INTERFACE_LINK_LIBRARIES nss::freebl)
    set_property(TARGET nss::nss APPEND PROPERTY INTERFACE_LINK_LIBRARIES nss::util)

########## For the modules (FindXXX)
set(nss_LIBRARIES_RELEASE nss::nss)
