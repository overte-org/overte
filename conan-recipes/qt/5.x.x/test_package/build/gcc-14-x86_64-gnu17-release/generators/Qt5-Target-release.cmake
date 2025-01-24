# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(qt_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(qt_FRAMEWORKS_FOUND_RELEASE "${qt_FRAMEWORKS_RELEASE}" "${qt_FRAMEWORK_DIRS_RELEASE}")

set(qt_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET qt_DEPS_TARGET)
    add_library(qt_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET qt_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${qt_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${qt_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:Fontconfig::Fontconfig;xkbcommon::xkbcommon;xorg::xorg;opengl::opengl;Qt5::Core;Qt5::Gui;Qt5::Widgets;xkbcommon::libxkbcommon-x11;Qt5::ServiceSupport;Qt5::ThemeSupport;Qt5::FontDatabaseSupport;Qt5::EdidSupport;Qt5::XkbCommonSupport;Qt5::XcbQpa;ODBC::ODBC;Qt5::Network;Qt5::Qml;Qt5::QmlModels;Qt5::Quick;Qt5::Test;Qt5::WebChannel;Qt5::Positioning;xorg-proto::xorg-proto;libxshmfence::libxshmfence;nss::nss;egl::egl;Qt5::WebEngineCore;Qt5::PrintSupport;Qt5::Multimedia>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### qt_DEPS_TARGET to all of them
conan_package_library_targets("${qt_LIBS_RELEASE}"    # libraries
                              "${qt_LIB_DIRS_RELEASE}" # package_libdir
                              "${qt_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_LIBRARY_TYPE_RELEASE}"
                              "${qt_IS_HOST_WINDOWS_RELEASE}"
                              qt_DEPS_TARGET
                              qt_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "qt"    # package_name
                              "${qt_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${qt_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## COMPONENTS TARGET PROPERTIES Release ########################################

    ########## COMPONENT Qt5::WebEngineWidgets #############

        set(qt_Qt5_WebEngineWidgets_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_WebEngineWidgets_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_WebEngineWidgets_FRAMEWORKS_RELEASE}" "${qt_Qt5_WebEngineWidgets_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_WebEngineWidgets_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_WebEngineWidgets_DEPS_TARGET)
            add_library(qt_Qt5_WebEngineWidgets_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_WebEngineWidgets_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngineWidgets_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngineWidgets_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngineWidgets_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_WebEngineWidgets_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_WebEngineWidgets_LIBS_RELEASE}"
                              "${qt_Qt5_WebEngineWidgets_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_WebEngineWidgets_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_WebEngineWidgets_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_WebEngineWidgets_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_WebEngineWidgets_DEPS_TARGET
                              qt_Qt5_WebEngineWidgets_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_WebEngineWidgets"
                              "${qt_Qt5_WebEngineWidgets_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::WebEngineWidgets
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngineWidgets_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngineWidgets_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_WebEngineWidgets_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::WebEngineWidgets
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_WebEngineWidgets_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::WebEngineWidgets APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngineWidgets_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::WebEngineWidgets APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngineWidgets_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::WebEngineWidgets APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngineWidgets_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::WebEngineWidgets APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngineWidgets_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::WebEngineWidgets APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngineWidgets_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::WebEngine #############

        set(qt_Qt5_WebEngine_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_WebEngine_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_WebEngine_FRAMEWORKS_RELEASE}" "${qt_Qt5_WebEngine_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_WebEngine_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_WebEngine_DEPS_TARGET)
            add_library(qt_Qt5_WebEngine_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_WebEngine_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngine_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngine_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngine_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_WebEngine_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_WebEngine_LIBS_RELEASE}"
                              "${qt_Qt5_WebEngine_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_WebEngine_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_WebEngine_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_WebEngine_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_WebEngine_DEPS_TARGET
                              qt_Qt5_WebEngine_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_WebEngine"
                              "${qt_Qt5_WebEngine_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::WebEngine
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngine_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngine_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_WebEngine_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::WebEngine
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_WebEngine_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::WebEngine APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngine_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::WebEngine APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngine_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::WebEngine APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngine_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::WebEngine APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngine_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::WebEngine APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngine_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::WebView #############

        set(qt_Qt5_WebView_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_WebView_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_WebView_FRAMEWORKS_RELEASE}" "${qt_Qt5_WebView_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_WebView_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_WebView_DEPS_TARGET)
            add_library(qt_Qt5_WebView_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_WebView_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_WebView_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_WebView_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_WebView_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_WebView_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_WebView_LIBS_RELEASE}"
                              "${qt_Qt5_WebView_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_WebView_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_WebView_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_WebView_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_WebView_DEPS_TARGET
                              qt_Qt5_WebView_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_WebView"
                              "${qt_Qt5_WebView_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::WebView
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_WebView_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_WebView_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_WebView_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::WebView
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_WebView_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::WebView APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_WebView_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::WebView APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_WebView_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::WebView APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_WebView_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::WebView APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_WebView_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::WebView APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_WebView_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::MultimediaQuick #############

        set(qt_Qt5_MultimediaQuick_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_MultimediaQuick_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_MultimediaQuick_FRAMEWORKS_RELEASE}" "${qt_Qt5_MultimediaQuick_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_MultimediaQuick_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_MultimediaQuick_DEPS_TARGET)
            add_library(qt_Qt5_MultimediaQuick_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_MultimediaQuick_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_MultimediaQuick_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_MultimediaQuick_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_MultimediaQuick_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_MultimediaQuick_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_MultimediaQuick_LIBS_RELEASE}"
                              "${qt_Qt5_MultimediaQuick_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_MultimediaQuick_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_MultimediaQuick_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_MultimediaQuick_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_MultimediaQuick_DEPS_TARGET
                              qt_Qt5_MultimediaQuick_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_MultimediaQuick"
                              "${qt_Qt5_MultimediaQuick_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::MultimediaQuick
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_MultimediaQuick_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_MultimediaQuick_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_MultimediaQuick_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::MultimediaQuick
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_MultimediaQuick_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::MultimediaQuick APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_MultimediaQuick_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::MultimediaQuick APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_MultimediaQuick_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::MultimediaQuick APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_MultimediaQuick_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::MultimediaQuick APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_MultimediaQuick_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::MultimediaQuick APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_MultimediaQuick_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::WebEngineCore #############

        set(qt_Qt5_WebEngineCore_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_WebEngineCore_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_WebEngineCore_FRAMEWORKS_RELEASE}" "${qt_Qt5_WebEngineCore_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_WebEngineCore_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_WebEngineCore_DEPS_TARGET)
            add_library(qt_Qt5_WebEngineCore_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_WebEngineCore_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngineCore_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngineCore_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngineCore_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_WebEngineCore_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_WebEngineCore_LIBS_RELEASE}"
                              "${qt_Qt5_WebEngineCore_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_WebEngineCore_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_WebEngineCore_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_WebEngineCore_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_WebEngineCore_DEPS_TARGET
                              qt_Qt5_WebEngineCore_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_WebEngineCore"
                              "${qt_Qt5_WebEngineCore_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::WebEngineCore
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngineCore_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngineCore_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_WebEngineCore_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::WebEngineCore
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_WebEngineCore_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::WebEngineCore APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngineCore_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::WebEngineCore APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngineCore_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::WebEngineCore APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngineCore_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::WebEngineCore APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngineCore_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::WebEngineCore APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_WebEngineCore_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::Location #############

        set(qt_Qt5_Location_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_Location_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_Location_FRAMEWORKS_RELEASE}" "${qt_Qt5_Location_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_Location_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_Location_DEPS_TARGET)
            add_library(qt_Qt5_Location_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_Location_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Location_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Location_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Location_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_Location_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_Location_LIBS_RELEASE}"
                              "${qt_Qt5_Location_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_Location_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_Location_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_Location_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_Location_DEPS_TARGET
                              qt_Qt5_Location_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_Location"
                              "${qt_Qt5_Location_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::Location
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Location_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Location_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_Location_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::Location
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_Location_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::Location APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Location_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::Location APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Location_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Location APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Location_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Location APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Location_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::Location APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Location_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::QuickTemplates2 #############

        set(qt_Qt5_QuickTemplates2_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_QuickTemplates2_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_QuickTemplates2_FRAMEWORKS_RELEASE}" "${qt_Qt5_QuickTemplates2_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_QuickTemplates2_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_QuickTemplates2_DEPS_TARGET)
            add_library(qt_Qt5_QuickTemplates2_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_QuickTemplates2_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QuickTemplates2_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QuickTemplates2_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QuickTemplates2_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_QuickTemplates2_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_QuickTemplates2_LIBS_RELEASE}"
                              "${qt_Qt5_QuickTemplates2_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_QuickTemplates2_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_QuickTemplates2_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_QuickTemplates2_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_QuickTemplates2_DEPS_TARGET
                              qt_Qt5_QuickTemplates2_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_QuickTemplates2"
                              "${qt_Qt5_QuickTemplates2_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::QuickTemplates2
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QuickTemplates2_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QuickTemplates2_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_QuickTemplates2_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::QuickTemplates2
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_QuickTemplates2_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::QuickTemplates2 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QuickTemplates2_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::QuickTemplates2 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QuickTemplates2_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QuickTemplates2 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QuickTemplates2_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QuickTemplates2 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QuickTemplates2_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::QuickTemplates2 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QuickTemplates2_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::QuickControls2 #############

        set(qt_Qt5_QuickControls2_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_QuickControls2_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_QuickControls2_FRAMEWORKS_RELEASE}" "${qt_Qt5_QuickControls2_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_QuickControls2_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_QuickControls2_DEPS_TARGET)
            add_library(qt_Qt5_QuickControls2_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_QuickControls2_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QuickControls2_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QuickControls2_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QuickControls2_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_QuickControls2_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_QuickControls2_LIBS_RELEASE}"
                              "${qt_Qt5_QuickControls2_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_QuickControls2_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_QuickControls2_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_QuickControls2_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_QuickControls2_DEPS_TARGET
                              qt_Qt5_QuickControls2_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_QuickControls2"
                              "${qt_Qt5_QuickControls2_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::QuickControls2
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QuickControls2_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QuickControls2_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_QuickControls2_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::QuickControls2
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_QuickControls2_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::QuickControls2 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QuickControls2_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::QuickControls2 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QuickControls2_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QuickControls2 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QuickControls2_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QuickControls2 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QuickControls2_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::QuickControls2 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QuickControls2_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::QuickShapes #############

        set(qt_Qt5_QuickShapes_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_QuickShapes_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_QuickShapes_FRAMEWORKS_RELEASE}" "${qt_Qt5_QuickShapes_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_QuickShapes_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_QuickShapes_DEPS_TARGET)
            add_library(qt_Qt5_QuickShapes_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_QuickShapes_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QuickShapes_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QuickShapes_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QuickShapes_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_QuickShapes_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_QuickShapes_LIBS_RELEASE}"
                              "${qt_Qt5_QuickShapes_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_QuickShapes_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_QuickShapes_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_QuickShapes_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_QuickShapes_DEPS_TARGET
                              qt_Qt5_QuickShapes_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_QuickShapes"
                              "${qt_Qt5_QuickShapes_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::QuickShapes
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QuickShapes_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QuickShapes_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_QuickShapes_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::QuickShapes
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_QuickShapes_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::QuickShapes APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QuickShapes_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::QuickShapes APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QuickShapes_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QuickShapes APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QuickShapes_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QuickShapes APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QuickShapes_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::QuickShapes APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QuickShapes_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::QuickWidgets #############

        set(qt_Qt5_QuickWidgets_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_QuickWidgets_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_QuickWidgets_FRAMEWORKS_RELEASE}" "${qt_Qt5_QuickWidgets_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_QuickWidgets_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_QuickWidgets_DEPS_TARGET)
            add_library(qt_Qt5_QuickWidgets_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_QuickWidgets_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QuickWidgets_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QuickWidgets_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QuickWidgets_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_QuickWidgets_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_QuickWidgets_LIBS_RELEASE}"
                              "${qt_Qt5_QuickWidgets_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_QuickWidgets_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_QuickWidgets_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_QuickWidgets_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_QuickWidgets_DEPS_TARGET
                              qt_Qt5_QuickWidgets_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_QuickWidgets"
                              "${qt_Qt5_QuickWidgets_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::QuickWidgets
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QuickWidgets_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QuickWidgets_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_QuickWidgets_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::QuickWidgets
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_QuickWidgets_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::QuickWidgets APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QuickWidgets_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::QuickWidgets APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QuickWidgets_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QuickWidgets APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QuickWidgets_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QuickWidgets APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QuickWidgets_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::QuickWidgets APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QuickWidgets_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::Quick #############

        set(qt_Qt5_Quick_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_Quick_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_Quick_FRAMEWORKS_RELEASE}" "${qt_Qt5_Quick_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_Quick_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_Quick_DEPS_TARGET)
            add_library(qt_Qt5_Quick_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_Quick_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Quick_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Quick_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Quick_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_Quick_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_Quick_LIBS_RELEASE}"
                              "${qt_Qt5_Quick_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_Quick_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_Quick_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_Quick_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_Quick_DEPS_TARGET
                              qt_Qt5_Quick_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_Quick"
                              "${qt_Qt5_Quick_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::Quick
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Quick_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Quick_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_Quick_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::Quick
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_Quick_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::Quick APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Quick_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::Quick APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Quick_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Quick APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Quick_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Quick APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Quick_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::Quick APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Quick_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::QXcbIntegrationPlugin #############

        set(qt_Qt5_QXcbIntegrationPlugin_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_QXcbIntegrationPlugin_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_QXcbIntegrationPlugin_FRAMEWORKS_RELEASE}" "${qt_Qt5_QXcbIntegrationPlugin_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_QXcbIntegrationPlugin_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_QXcbIntegrationPlugin_DEPS_TARGET)
            add_library(qt_Qt5_QXcbIntegrationPlugin_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_QXcbIntegrationPlugin_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QXcbIntegrationPlugin_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QXcbIntegrationPlugin_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QXcbIntegrationPlugin_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_QXcbIntegrationPlugin_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_QXcbIntegrationPlugin_LIBS_RELEASE}"
                              "${qt_Qt5_QXcbIntegrationPlugin_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_QXcbIntegrationPlugin_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_QXcbIntegrationPlugin_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_QXcbIntegrationPlugin_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_QXcbIntegrationPlugin_DEPS_TARGET
                              qt_Qt5_QXcbIntegrationPlugin_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_QXcbIntegrationPlugin"
                              "${qt_Qt5_QXcbIntegrationPlugin_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::QXcbIntegrationPlugin
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QXcbIntegrationPlugin_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QXcbIntegrationPlugin_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_QXcbIntegrationPlugin_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::QXcbIntegrationPlugin
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_QXcbIntegrationPlugin_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::QXcbIntegrationPlugin APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QXcbIntegrationPlugin_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::QXcbIntegrationPlugin APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QXcbIntegrationPlugin_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QXcbIntegrationPlugin APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QXcbIntegrationPlugin_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QXcbIntegrationPlugin APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QXcbIntegrationPlugin_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::QXcbIntegrationPlugin APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QXcbIntegrationPlugin_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::MultimediaWidgets #############

        set(qt_Qt5_MultimediaWidgets_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_MultimediaWidgets_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_MultimediaWidgets_FRAMEWORKS_RELEASE}" "${qt_Qt5_MultimediaWidgets_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_MultimediaWidgets_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_MultimediaWidgets_DEPS_TARGET)
            add_library(qt_Qt5_MultimediaWidgets_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_MultimediaWidgets_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_MultimediaWidgets_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_MultimediaWidgets_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_MultimediaWidgets_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_MultimediaWidgets_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_MultimediaWidgets_LIBS_RELEASE}"
                              "${qt_Qt5_MultimediaWidgets_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_MultimediaWidgets_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_MultimediaWidgets_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_MultimediaWidgets_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_MultimediaWidgets_DEPS_TARGET
                              qt_Qt5_MultimediaWidgets_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_MultimediaWidgets"
                              "${qt_Qt5_MultimediaWidgets_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::MultimediaWidgets
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_MultimediaWidgets_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_MultimediaWidgets_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_MultimediaWidgets_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::MultimediaWidgets
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_MultimediaWidgets_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::MultimediaWidgets APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_MultimediaWidgets_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::MultimediaWidgets APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_MultimediaWidgets_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::MultimediaWidgets APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_MultimediaWidgets_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::MultimediaWidgets APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_MultimediaWidgets_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::MultimediaWidgets APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_MultimediaWidgets_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::Scxml #############

        set(qt_Qt5_Scxml_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_Scxml_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_Scxml_FRAMEWORKS_RELEASE}" "${qt_Qt5_Scxml_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_Scxml_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_Scxml_DEPS_TARGET)
            add_library(qt_Qt5_Scxml_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_Scxml_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Scxml_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Scxml_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Scxml_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_Scxml_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_Scxml_LIBS_RELEASE}"
                              "${qt_Qt5_Scxml_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_Scxml_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_Scxml_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_Scxml_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_Scxml_DEPS_TARGET
                              qt_Qt5_Scxml_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_Scxml"
                              "${qt_Qt5_Scxml_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::Scxml
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Scxml_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Scxml_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_Scxml_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::Scxml
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_Scxml_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::Scxml APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Scxml_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::Scxml APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Scxml_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Scxml APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Scxml_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Scxml APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Scxml_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::Scxml APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Scxml_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::WebChannel #############

        set(qt_Qt5_WebChannel_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_WebChannel_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_WebChannel_FRAMEWORKS_RELEASE}" "${qt_Qt5_WebChannel_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_WebChannel_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_WebChannel_DEPS_TARGET)
            add_library(qt_Qt5_WebChannel_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_WebChannel_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_WebChannel_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_WebChannel_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_WebChannel_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_WebChannel_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_WebChannel_LIBS_RELEASE}"
                              "${qt_Qt5_WebChannel_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_WebChannel_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_WebChannel_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_WebChannel_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_WebChannel_DEPS_TARGET
                              qt_Qt5_WebChannel_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_WebChannel"
                              "${qt_Qt5_WebChannel_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::WebChannel
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_WebChannel_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_WebChannel_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_WebChannel_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::WebChannel
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_WebChannel_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::WebChannel APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_WebChannel_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::WebChannel APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_WebChannel_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::WebChannel APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_WebChannel_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::WebChannel APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_WebChannel_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::WebChannel APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_WebChannel_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::QmlWorkerScript #############

        set(qt_Qt5_QmlWorkerScript_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_QmlWorkerScript_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_QmlWorkerScript_FRAMEWORKS_RELEASE}" "${qt_Qt5_QmlWorkerScript_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_QmlWorkerScript_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_QmlWorkerScript_DEPS_TARGET)
            add_library(qt_Qt5_QmlWorkerScript_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_QmlWorkerScript_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QmlWorkerScript_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QmlWorkerScript_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QmlWorkerScript_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_QmlWorkerScript_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_QmlWorkerScript_LIBS_RELEASE}"
                              "${qt_Qt5_QmlWorkerScript_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_QmlWorkerScript_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_QmlWorkerScript_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_QmlWorkerScript_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_QmlWorkerScript_DEPS_TARGET
                              qt_Qt5_QmlWorkerScript_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_QmlWorkerScript"
                              "${qt_Qt5_QmlWorkerScript_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::QmlWorkerScript
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QmlWorkerScript_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QmlWorkerScript_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_QmlWorkerScript_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::QmlWorkerScript
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_QmlWorkerScript_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::QmlWorkerScript APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QmlWorkerScript_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::QmlWorkerScript APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QmlWorkerScript_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QmlWorkerScript APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QmlWorkerScript_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QmlWorkerScript APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QmlWorkerScript_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::QmlWorkerScript APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QmlWorkerScript_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::QmlImportScanner #############

        set(qt_Qt5_QmlImportScanner_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_QmlImportScanner_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_QmlImportScanner_FRAMEWORKS_RELEASE}" "${qt_Qt5_QmlImportScanner_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_QmlImportScanner_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_QmlImportScanner_DEPS_TARGET)
            add_library(qt_Qt5_QmlImportScanner_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_QmlImportScanner_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QmlImportScanner_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QmlImportScanner_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QmlImportScanner_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_QmlImportScanner_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_QmlImportScanner_LIBS_RELEASE}"
                              "${qt_Qt5_QmlImportScanner_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_QmlImportScanner_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_QmlImportScanner_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_QmlImportScanner_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_QmlImportScanner_DEPS_TARGET
                              qt_Qt5_QmlImportScanner_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_QmlImportScanner"
                              "${qt_Qt5_QmlImportScanner_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::QmlImportScanner
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QmlImportScanner_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QmlImportScanner_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_QmlImportScanner_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::QmlImportScanner
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_QmlImportScanner_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::QmlImportScanner APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QmlImportScanner_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::QmlImportScanner APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QmlImportScanner_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QmlImportScanner APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QmlImportScanner_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QmlImportScanner APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QmlImportScanner_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::QmlImportScanner APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QmlImportScanner_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::QmlModels #############

        set(qt_Qt5_QmlModels_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_QmlModels_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_QmlModels_FRAMEWORKS_RELEASE}" "${qt_Qt5_QmlModels_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_QmlModels_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_QmlModels_DEPS_TARGET)
            add_library(qt_Qt5_QmlModels_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_QmlModels_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QmlModels_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QmlModels_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QmlModels_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_QmlModels_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_QmlModels_LIBS_RELEASE}"
                              "${qt_Qt5_QmlModels_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_QmlModels_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_QmlModels_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_QmlModels_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_QmlModels_DEPS_TARGET
                              qt_Qt5_QmlModels_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_QmlModels"
                              "${qt_Qt5_QmlModels_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::QmlModels
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QmlModels_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QmlModels_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_QmlModels_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::QmlModels
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_QmlModels_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::QmlModels APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QmlModels_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::QmlModels APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QmlModels_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QmlModels APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QmlModels_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QmlModels APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QmlModels_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::QmlModels APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QmlModels_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::XcbQpa #############

        set(qt_Qt5_XcbQpa_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_XcbQpa_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_XcbQpa_FRAMEWORKS_RELEASE}" "${qt_Qt5_XcbQpa_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_XcbQpa_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_XcbQpa_DEPS_TARGET)
            add_library(qt_Qt5_XcbQpa_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_XcbQpa_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_XcbQpa_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_XcbQpa_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_XcbQpa_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_XcbQpa_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_XcbQpa_LIBS_RELEASE}"
                              "${qt_Qt5_XcbQpa_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_XcbQpa_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_XcbQpa_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_XcbQpa_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_XcbQpa_DEPS_TARGET
                              qt_Qt5_XcbQpa_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_XcbQpa"
                              "${qt_Qt5_XcbQpa_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::XcbQpa
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_XcbQpa_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_XcbQpa_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_XcbQpa_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::XcbQpa
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_XcbQpa_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::XcbQpa APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_XcbQpa_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::XcbQpa APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_XcbQpa_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::XcbQpa APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_XcbQpa_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::XcbQpa APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_XcbQpa_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::XcbQpa APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_XcbQpa_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::PrintSupport #############

        set(qt_Qt5_PrintSupport_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_PrintSupport_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_PrintSupport_FRAMEWORKS_RELEASE}" "${qt_Qt5_PrintSupport_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_PrintSupport_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_PrintSupport_DEPS_TARGET)
            add_library(qt_Qt5_PrintSupport_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_PrintSupport_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_PrintSupport_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_PrintSupport_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_PrintSupport_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_PrintSupport_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_PrintSupport_LIBS_RELEASE}"
                              "${qt_Qt5_PrintSupport_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_PrintSupport_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_PrintSupport_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_PrintSupport_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_PrintSupport_DEPS_TARGET
                              qt_Qt5_PrintSupport_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_PrintSupport"
                              "${qt_Qt5_PrintSupport_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::PrintSupport
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_PrintSupport_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_PrintSupport_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_PrintSupport_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::PrintSupport
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_PrintSupport_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::PrintSupport APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_PrintSupport_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::PrintSupport APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_PrintSupport_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::PrintSupport APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_PrintSupport_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::PrintSupport APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_PrintSupport_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::PrintSupport APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_PrintSupport_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::XmlPatterns #############

        set(qt_Qt5_XmlPatterns_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_XmlPatterns_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_XmlPatterns_FRAMEWORKS_RELEASE}" "${qt_Qt5_XmlPatterns_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_XmlPatterns_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_XmlPatterns_DEPS_TARGET)
            add_library(qt_Qt5_XmlPatterns_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_XmlPatterns_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_XmlPatterns_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_XmlPatterns_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_XmlPatterns_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_XmlPatterns_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_XmlPatterns_LIBS_RELEASE}"
                              "${qt_Qt5_XmlPatterns_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_XmlPatterns_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_XmlPatterns_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_XmlPatterns_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_XmlPatterns_DEPS_TARGET
                              qt_Qt5_XmlPatterns_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_XmlPatterns"
                              "${qt_Qt5_XmlPatterns_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::XmlPatterns
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_XmlPatterns_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_XmlPatterns_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_XmlPatterns_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::XmlPatterns
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_XmlPatterns_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::XmlPatterns APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_XmlPatterns_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::XmlPatterns APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_XmlPatterns_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::XmlPatterns APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_XmlPatterns_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::XmlPatterns APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_XmlPatterns_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::XmlPatterns APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_XmlPatterns_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::WebSockets #############

        set(qt_Qt5_WebSockets_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_WebSockets_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_WebSockets_FRAMEWORKS_RELEASE}" "${qt_Qt5_WebSockets_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_WebSockets_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_WebSockets_DEPS_TARGET)
            add_library(qt_Qt5_WebSockets_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_WebSockets_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_WebSockets_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_WebSockets_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_WebSockets_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_WebSockets_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_WebSockets_LIBS_RELEASE}"
                              "${qt_Qt5_WebSockets_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_WebSockets_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_WebSockets_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_WebSockets_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_WebSockets_DEPS_TARGET
                              qt_Qt5_WebSockets_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_WebSockets"
                              "${qt_Qt5_WebSockets_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::WebSockets
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_WebSockets_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_WebSockets_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_WebSockets_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::WebSockets
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_WebSockets_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::WebSockets APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_WebSockets_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::WebSockets APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_WebSockets_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::WebSockets APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_WebSockets_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::WebSockets APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_WebSockets_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::WebSockets APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_WebSockets_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::Multimedia #############

        set(qt_Qt5_Multimedia_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_Multimedia_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_Multimedia_FRAMEWORKS_RELEASE}" "${qt_Qt5_Multimedia_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_Multimedia_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_Multimedia_DEPS_TARGET)
            add_library(qt_Qt5_Multimedia_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_Multimedia_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Multimedia_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Multimedia_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Multimedia_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_Multimedia_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_Multimedia_LIBS_RELEASE}"
                              "${qt_Qt5_Multimedia_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_Multimedia_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_Multimedia_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_Multimedia_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_Multimedia_DEPS_TARGET
                              qt_Qt5_Multimedia_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_Multimedia"
                              "${qt_Qt5_Multimedia_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::Multimedia
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Multimedia_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Multimedia_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_Multimedia_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::Multimedia
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_Multimedia_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::Multimedia APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Multimedia_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::Multimedia APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Multimedia_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Multimedia APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Multimedia_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Multimedia APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Multimedia_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::Multimedia APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Multimedia_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::Svg #############

        set(qt_Qt5_Svg_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_Svg_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_Svg_FRAMEWORKS_RELEASE}" "${qt_Qt5_Svg_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_Svg_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_Svg_DEPS_TARGET)
            add_library(qt_Qt5_Svg_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_Svg_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Svg_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Svg_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Svg_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_Svg_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_Svg_LIBS_RELEASE}"
                              "${qt_Qt5_Svg_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_Svg_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_Svg_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_Svg_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_Svg_DEPS_TARGET
                              qt_Qt5_Svg_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_Svg"
                              "${qt_Qt5_Svg_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::Svg
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Svg_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Svg_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_Svg_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::Svg
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_Svg_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::Svg APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Svg_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::Svg APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Svg_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Svg APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Svg_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Svg APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Svg_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::Svg APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Svg_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::QuickTest #############

        set(qt_Qt5_QuickTest_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_QuickTest_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_QuickTest_FRAMEWORKS_RELEASE}" "${qt_Qt5_QuickTest_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_QuickTest_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_QuickTest_DEPS_TARGET)
            add_library(qt_Qt5_QuickTest_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_QuickTest_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QuickTest_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QuickTest_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QuickTest_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_QuickTest_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_QuickTest_LIBS_RELEASE}"
                              "${qt_Qt5_QuickTest_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_QuickTest_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_QuickTest_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_QuickTest_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_QuickTest_DEPS_TARGET
                              qt_Qt5_QuickTest_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_QuickTest"
                              "${qt_Qt5_QuickTest_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::QuickTest
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QuickTest_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QuickTest_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_QuickTest_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::QuickTest
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_QuickTest_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::QuickTest APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QuickTest_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::QuickTest APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QuickTest_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QuickTest APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QuickTest_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QuickTest APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QuickTest_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::QuickTest APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QuickTest_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::Qml #############

        set(qt_Qt5_Qml_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_Qml_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_Qml_FRAMEWORKS_RELEASE}" "${qt_Qt5_Qml_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_Qml_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_Qml_DEPS_TARGET)
            add_library(qt_Qt5_Qml_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_Qml_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Qml_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Qml_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Qml_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_Qml_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_Qml_LIBS_RELEASE}"
                              "${qt_Qt5_Qml_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_Qml_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_Qml_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_Qml_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_Qml_DEPS_TARGET
                              qt_Qt5_Qml_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_Qml"
                              "${qt_Qt5_Qml_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::Qml
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Qml_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Qml_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_Qml_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::Qml
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_Qml_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::Qml APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Qml_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::Qml APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Qml_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Qml APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Qml_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Qml APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Qml_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::Qml APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Qml_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::OpenGLExtensions #############

        set(qt_Qt5_OpenGLExtensions_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_OpenGLExtensions_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_OpenGLExtensions_FRAMEWORKS_RELEASE}" "${qt_Qt5_OpenGLExtensions_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_OpenGLExtensions_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_OpenGLExtensions_DEPS_TARGET)
            add_library(qt_Qt5_OpenGLExtensions_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_OpenGLExtensions_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_OpenGLExtensions_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_OpenGLExtensions_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_OpenGLExtensions_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_OpenGLExtensions_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_OpenGLExtensions_LIBS_RELEASE}"
                              "${qt_Qt5_OpenGLExtensions_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_OpenGLExtensions_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_OpenGLExtensions_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_OpenGLExtensions_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_OpenGLExtensions_DEPS_TARGET
                              qt_Qt5_OpenGLExtensions_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_OpenGLExtensions"
                              "${qt_Qt5_OpenGLExtensions_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::OpenGLExtensions
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_OpenGLExtensions_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_OpenGLExtensions_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_OpenGLExtensions_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::OpenGLExtensions
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_OpenGLExtensions_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::OpenGLExtensions APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_OpenGLExtensions_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::OpenGLExtensions APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_OpenGLExtensions_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::OpenGLExtensions APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_OpenGLExtensions_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::OpenGLExtensions APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_OpenGLExtensions_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::OpenGLExtensions APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_OpenGLExtensions_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::OpenGL #############

        set(qt_Qt5_OpenGL_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_OpenGL_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_OpenGL_FRAMEWORKS_RELEASE}" "${qt_Qt5_OpenGL_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_OpenGL_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_OpenGL_DEPS_TARGET)
            add_library(qt_Qt5_OpenGL_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_OpenGL_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_OpenGL_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_OpenGL_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_OpenGL_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_OpenGL_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_OpenGL_LIBS_RELEASE}"
                              "${qt_Qt5_OpenGL_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_OpenGL_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_OpenGL_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_OpenGL_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_OpenGL_DEPS_TARGET
                              qt_Qt5_OpenGL_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_OpenGL"
                              "${qt_Qt5_OpenGL_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::OpenGL
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_OpenGL_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_OpenGL_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_OpenGL_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::OpenGL
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_OpenGL_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::OpenGL APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_OpenGL_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::OpenGL APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_OpenGL_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::OpenGL APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_OpenGL_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::OpenGL APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_OpenGL_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::OpenGL APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_OpenGL_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::XkbCommonSupport #############

        set(qt_Qt5_XkbCommonSupport_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_XkbCommonSupport_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_XkbCommonSupport_FRAMEWORKS_RELEASE}" "${qt_Qt5_XkbCommonSupport_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_XkbCommonSupport_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_XkbCommonSupport_DEPS_TARGET)
            add_library(qt_Qt5_XkbCommonSupport_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_XkbCommonSupport_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_XkbCommonSupport_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_XkbCommonSupport_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_XkbCommonSupport_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_XkbCommonSupport_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_XkbCommonSupport_LIBS_RELEASE}"
                              "${qt_Qt5_XkbCommonSupport_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_XkbCommonSupport_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_XkbCommonSupport_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_XkbCommonSupport_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_XkbCommonSupport_DEPS_TARGET
                              qt_Qt5_XkbCommonSupport_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_XkbCommonSupport"
                              "${qt_Qt5_XkbCommonSupport_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::XkbCommonSupport
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_XkbCommonSupport_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_XkbCommonSupport_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_XkbCommonSupport_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::XkbCommonSupport
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_XkbCommonSupport_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::XkbCommonSupport APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_XkbCommonSupport_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::XkbCommonSupport APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_XkbCommonSupport_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::XkbCommonSupport APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_XkbCommonSupport_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::XkbCommonSupport APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_XkbCommonSupport_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::XkbCommonSupport APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_XkbCommonSupport_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::ServiceSupport #############

        set(qt_Qt5_ServiceSupport_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_ServiceSupport_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_ServiceSupport_FRAMEWORKS_RELEASE}" "${qt_Qt5_ServiceSupport_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_ServiceSupport_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_ServiceSupport_DEPS_TARGET)
            add_library(qt_Qt5_ServiceSupport_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_ServiceSupport_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_ServiceSupport_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_ServiceSupport_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_ServiceSupport_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_ServiceSupport_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_ServiceSupport_LIBS_RELEASE}"
                              "${qt_Qt5_ServiceSupport_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_ServiceSupport_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_ServiceSupport_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_ServiceSupport_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_ServiceSupport_DEPS_TARGET
                              qt_Qt5_ServiceSupport_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_ServiceSupport"
                              "${qt_Qt5_ServiceSupport_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::ServiceSupport
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_ServiceSupport_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_ServiceSupport_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_ServiceSupport_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::ServiceSupport
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_ServiceSupport_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::ServiceSupport APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_ServiceSupport_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::ServiceSupport APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_ServiceSupport_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::ServiceSupport APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_ServiceSupport_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::ServiceSupport APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_ServiceSupport_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::ServiceSupport APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_ServiceSupport_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::Widgets #############

        set(qt_Qt5_Widgets_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_Widgets_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_Widgets_FRAMEWORKS_RELEASE}" "${qt_Qt5_Widgets_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_Widgets_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_Widgets_DEPS_TARGET)
            add_library(qt_Qt5_Widgets_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_Widgets_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Widgets_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Widgets_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Widgets_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_Widgets_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_Widgets_LIBS_RELEASE}"
                              "${qt_Qt5_Widgets_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_Widgets_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_Widgets_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_Widgets_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_Widgets_DEPS_TARGET
                              qt_Qt5_Widgets_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_Widgets"
                              "${qt_Qt5_Widgets_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::Widgets
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Widgets_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Widgets_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_Widgets_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::Widgets
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_Widgets_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::Widgets APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Widgets_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::Widgets APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Widgets_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Widgets APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Widgets_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Widgets APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Widgets_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::Widgets APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Widgets_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::AccessibilitySupport #############

        set(qt_Qt5_AccessibilitySupport_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_AccessibilitySupport_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_AccessibilitySupport_FRAMEWORKS_RELEASE}" "${qt_Qt5_AccessibilitySupport_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_AccessibilitySupport_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_AccessibilitySupport_DEPS_TARGET)
            add_library(qt_Qt5_AccessibilitySupport_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_AccessibilitySupport_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_AccessibilitySupport_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_AccessibilitySupport_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_AccessibilitySupport_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_AccessibilitySupport_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_AccessibilitySupport_LIBS_RELEASE}"
                              "${qt_Qt5_AccessibilitySupport_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_AccessibilitySupport_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_AccessibilitySupport_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_AccessibilitySupport_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_AccessibilitySupport_DEPS_TARGET
                              qt_Qt5_AccessibilitySupport_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_AccessibilitySupport"
                              "${qt_Qt5_AccessibilitySupport_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::AccessibilitySupport
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_AccessibilitySupport_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_AccessibilitySupport_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_AccessibilitySupport_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::AccessibilitySupport
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_AccessibilitySupport_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::AccessibilitySupport APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_AccessibilitySupport_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::AccessibilitySupport APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_AccessibilitySupport_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::AccessibilitySupport APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_AccessibilitySupport_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::AccessibilitySupport APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_AccessibilitySupport_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::AccessibilitySupport APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_AccessibilitySupport_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::ThemeSupport #############

        set(qt_Qt5_ThemeSupport_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_ThemeSupport_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_ThemeSupport_FRAMEWORKS_RELEASE}" "${qt_Qt5_ThemeSupport_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_ThemeSupport_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_ThemeSupport_DEPS_TARGET)
            add_library(qt_Qt5_ThemeSupport_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_ThemeSupport_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_ThemeSupport_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_ThemeSupport_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_ThemeSupport_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_ThemeSupport_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_ThemeSupport_LIBS_RELEASE}"
                              "${qt_Qt5_ThemeSupport_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_ThemeSupport_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_ThemeSupport_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_ThemeSupport_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_ThemeSupport_DEPS_TARGET
                              qt_Qt5_ThemeSupport_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_ThemeSupport"
                              "${qt_Qt5_ThemeSupport_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::ThemeSupport
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_ThemeSupport_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_ThemeSupport_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_ThemeSupport_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::ThemeSupport
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_ThemeSupport_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::ThemeSupport APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_ThemeSupport_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::ThemeSupport APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_ThemeSupport_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::ThemeSupport APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_ThemeSupport_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::ThemeSupport APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_ThemeSupport_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::ThemeSupport APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_ThemeSupport_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::FontDatabaseSupport #############

        set(qt_Qt5_FontDatabaseSupport_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_FontDatabaseSupport_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_FontDatabaseSupport_FRAMEWORKS_RELEASE}" "${qt_Qt5_FontDatabaseSupport_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_FontDatabaseSupport_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_FontDatabaseSupport_DEPS_TARGET)
            add_library(qt_Qt5_FontDatabaseSupport_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_FontDatabaseSupport_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_FontDatabaseSupport_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_FontDatabaseSupport_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_FontDatabaseSupport_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_FontDatabaseSupport_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_FontDatabaseSupport_LIBS_RELEASE}"
                              "${qt_Qt5_FontDatabaseSupport_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_FontDatabaseSupport_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_FontDatabaseSupport_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_FontDatabaseSupport_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_FontDatabaseSupport_DEPS_TARGET
                              qt_Qt5_FontDatabaseSupport_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_FontDatabaseSupport"
                              "${qt_Qt5_FontDatabaseSupport_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::FontDatabaseSupport
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_FontDatabaseSupport_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_FontDatabaseSupport_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_FontDatabaseSupport_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::FontDatabaseSupport
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_FontDatabaseSupport_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::FontDatabaseSupport APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_FontDatabaseSupport_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::FontDatabaseSupport APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_FontDatabaseSupport_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::FontDatabaseSupport APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_FontDatabaseSupport_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::FontDatabaseSupport APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_FontDatabaseSupport_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::FontDatabaseSupport APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_FontDatabaseSupport_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::EventDispatcherSupport #############

        set(qt_Qt5_EventDispatcherSupport_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_EventDispatcherSupport_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_EventDispatcherSupport_FRAMEWORKS_RELEASE}" "${qt_Qt5_EventDispatcherSupport_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_EventDispatcherSupport_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_EventDispatcherSupport_DEPS_TARGET)
            add_library(qt_Qt5_EventDispatcherSupport_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_EventDispatcherSupport_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_EventDispatcherSupport_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_EventDispatcherSupport_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_EventDispatcherSupport_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_EventDispatcherSupport_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_EventDispatcherSupport_LIBS_RELEASE}"
                              "${qt_Qt5_EventDispatcherSupport_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_EventDispatcherSupport_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_EventDispatcherSupport_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_EventDispatcherSupport_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_EventDispatcherSupport_DEPS_TARGET
                              qt_Qt5_EventDispatcherSupport_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_EventDispatcherSupport"
                              "${qt_Qt5_EventDispatcherSupport_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::EventDispatcherSupport
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_EventDispatcherSupport_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_EventDispatcherSupport_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_EventDispatcherSupport_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::EventDispatcherSupport
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_EventDispatcherSupport_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::EventDispatcherSupport APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_EventDispatcherSupport_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::EventDispatcherSupport APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_EventDispatcherSupport_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::EventDispatcherSupport APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_EventDispatcherSupport_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::EventDispatcherSupport APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_EventDispatcherSupport_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::EventDispatcherSupport APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_EventDispatcherSupport_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::QM3uPlaylistPlugin #############

        set(qt_Qt5_QM3uPlaylistPlugin_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_QM3uPlaylistPlugin_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_QM3uPlaylistPlugin_FRAMEWORKS_RELEASE}" "${qt_Qt5_QM3uPlaylistPlugin_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_QM3uPlaylistPlugin_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_QM3uPlaylistPlugin_DEPS_TARGET)
            add_library(qt_Qt5_QM3uPlaylistPlugin_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_QM3uPlaylistPlugin_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QM3uPlaylistPlugin_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QM3uPlaylistPlugin_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QM3uPlaylistPlugin_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_QM3uPlaylistPlugin_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_QM3uPlaylistPlugin_LIBS_RELEASE}"
                              "${qt_Qt5_QM3uPlaylistPlugin_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_QM3uPlaylistPlugin_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_QM3uPlaylistPlugin_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_QM3uPlaylistPlugin_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_QM3uPlaylistPlugin_DEPS_TARGET
                              qt_Qt5_QM3uPlaylistPlugin_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_QM3uPlaylistPlugin"
                              "${qt_Qt5_QM3uPlaylistPlugin_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::QM3uPlaylistPlugin
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QM3uPlaylistPlugin_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QM3uPlaylistPlugin_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_QM3uPlaylistPlugin_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::QM3uPlaylistPlugin
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_QM3uPlaylistPlugin_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::QM3uPlaylistPlugin APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QM3uPlaylistPlugin_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::QM3uPlaylistPlugin APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QM3uPlaylistPlugin_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QM3uPlaylistPlugin APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QM3uPlaylistPlugin_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QM3uPlaylistPlugin APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QM3uPlaylistPlugin_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::QM3uPlaylistPlugin APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QM3uPlaylistPlugin_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::QGeoPositionInfoSourceFactorySerialNmea #############

        set(qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_FRAMEWORKS_RELEASE}" "${qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_DEPS_TARGET)
            add_library(qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_LIBS_RELEASE}"
                              "${qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_DEPS_TARGET
                              qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea"
                              "${qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::QGeoPositionInfoSourceFactorySerialNmea
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::QGeoPositionInfoSourceFactorySerialNmea
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::QGeoPositionInfoSourceFactorySerialNmea APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::QGeoPositionInfoSourceFactorySerialNmea APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QGeoPositionInfoSourceFactorySerialNmea APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QGeoPositionInfoSourceFactorySerialNmea APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::QGeoPositionInfoSourceFactorySerialNmea APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactorySerialNmea_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::QGeoPositionInfoSourceFactoryPoll #############

        set(qt_Qt5_QGeoPositionInfoSourceFactoryPoll_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_QGeoPositionInfoSourceFactoryPoll_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_QGeoPositionInfoSourceFactoryPoll_FRAMEWORKS_RELEASE}" "${qt_Qt5_QGeoPositionInfoSourceFactoryPoll_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_QGeoPositionInfoSourceFactoryPoll_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_QGeoPositionInfoSourceFactoryPoll_DEPS_TARGET)
            add_library(qt_Qt5_QGeoPositionInfoSourceFactoryPoll_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_QGeoPositionInfoSourceFactoryPoll_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryPoll_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryPoll_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryPoll_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_QGeoPositionInfoSourceFactoryPoll_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_QGeoPositionInfoSourceFactoryPoll_LIBS_RELEASE}"
                              "${qt_Qt5_QGeoPositionInfoSourceFactoryPoll_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_QGeoPositionInfoSourceFactoryPoll_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_QGeoPositionInfoSourceFactoryPoll_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_QGeoPositionInfoSourceFactoryPoll_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_QGeoPositionInfoSourceFactoryPoll_DEPS_TARGET
                              qt_Qt5_QGeoPositionInfoSourceFactoryPoll_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_QGeoPositionInfoSourceFactoryPoll"
                              "${qt_Qt5_QGeoPositionInfoSourceFactoryPoll_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::QGeoPositionInfoSourceFactoryPoll
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryPoll_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryPoll_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_QGeoPositionInfoSourceFactoryPoll_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::QGeoPositionInfoSourceFactoryPoll
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_QGeoPositionInfoSourceFactoryPoll_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::QGeoPositionInfoSourceFactoryPoll APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryPoll_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::QGeoPositionInfoSourceFactoryPoll APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryPoll_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QGeoPositionInfoSourceFactoryPoll APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryPoll_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QGeoPositionInfoSourceFactoryPoll APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryPoll_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::QGeoPositionInfoSourceFactoryPoll APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryPoll_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::QGeoPositionInfoSourceFactoryGeoclue2 #############

        set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_FRAMEWORKS_RELEASE}" "${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_DEPS_TARGET)
            add_library(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_LIBS_RELEASE}"
                              "${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_DEPS_TARGET
                              qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2"
                              "${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::QGeoPositionInfoSourceFactoryGeoclue2
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::QGeoPositionInfoSourceFactoryGeoclue2
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::QGeoPositionInfoSourceFactoryGeoclue2 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::QGeoPositionInfoSourceFactoryGeoclue2 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QGeoPositionInfoSourceFactoryGeoclue2 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QGeoPositionInfoSourceFactoryGeoclue2 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::QGeoPositionInfoSourceFactoryGeoclue2 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue2_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::QGeoPositionInfoSourceFactoryGeoclue #############

        set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_FRAMEWORKS_RELEASE}" "${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_DEPS_TARGET)
            add_library(qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_LIBS_RELEASE}"
                              "${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_DEPS_TARGET
                              qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue"
                              "${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::QGeoPositionInfoSourceFactoryGeoclue
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::QGeoPositionInfoSourceFactoryGeoclue
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::QGeoPositionInfoSourceFactoryGeoclue APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::QGeoPositionInfoSourceFactoryGeoclue APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QGeoPositionInfoSourceFactoryGeoclue APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QGeoPositionInfoSourceFactoryGeoclue APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::QGeoPositionInfoSourceFactoryGeoclue APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoPositionInfoSourceFactoryGeoclue_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::QGeoServiceProviderFactoryOsm #############

        set(qt_Qt5_QGeoServiceProviderFactoryOsm_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_QGeoServiceProviderFactoryOsm_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_QGeoServiceProviderFactoryOsm_FRAMEWORKS_RELEASE}" "${qt_Qt5_QGeoServiceProviderFactoryOsm_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_QGeoServiceProviderFactoryOsm_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_QGeoServiceProviderFactoryOsm_DEPS_TARGET)
            add_library(qt_Qt5_QGeoServiceProviderFactoryOsm_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_QGeoServiceProviderFactoryOsm_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryOsm_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryOsm_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryOsm_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_QGeoServiceProviderFactoryOsm_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_QGeoServiceProviderFactoryOsm_LIBS_RELEASE}"
                              "${qt_Qt5_QGeoServiceProviderFactoryOsm_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_QGeoServiceProviderFactoryOsm_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_QGeoServiceProviderFactoryOsm_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_QGeoServiceProviderFactoryOsm_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_QGeoServiceProviderFactoryOsm_DEPS_TARGET
                              qt_Qt5_QGeoServiceProviderFactoryOsm_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_QGeoServiceProviderFactoryOsm"
                              "${qt_Qt5_QGeoServiceProviderFactoryOsm_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::QGeoServiceProviderFactoryOsm
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryOsm_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryOsm_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_QGeoServiceProviderFactoryOsm_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::QGeoServiceProviderFactoryOsm
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_QGeoServiceProviderFactoryOsm_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::QGeoServiceProviderFactoryOsm APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryOsm_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::QGeoServiceProviderFactoryOsm APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryOsm_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QGeoServiceProviderFactoryOsm APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryOsm_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QGeoServiceProviderFactoryOsm APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryOsm_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::QGeoServiceProviderFactoryOsm APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryOsm_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::QGeoServiceProviderFactoryNokia #############

        set(qt_Qt5_QGeoServiceProviderFactoryNokia_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_QGeoServiceProviderFactoryNokia_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_QGeoServiceProviderFactoryNokia_FRAMEWORKS_RELEASE}" "${qt_Qt5_QGeoServiceProviderFactoryNokia_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_QGeoServiceProviderFactoryNokia_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_QGeoServiceProviderFactoryNokia_DEPS_TARGET)
            add_library(qt_Qt5_QGeoServiceProviderFactoryNokia_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_QGeoServiceProviderFactoryNokia_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryNokia_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryNokia_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryNokia_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_QGeoServiceProviderFactoryNokia_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_QGeoServiceProviderFactoryNokia_LIBS_RELEASE}"
                              "${qt_Qt5_QGeoServiceProviderFactoryNokia_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_QGeoServiceProviderFactoryNokia_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_QGeoServiceProviderFactoryNokia_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_QGeoServiceProviderFactoryNokia_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_QGeoServiceProviderFactoryNokia_DEPS_TARGET
                              qt_Qt5_QGeoServiceProviderFactoryNokia_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_QGeoServiceProviderFactoryNokia"
                              "${qt_Qt5_QGeoServiceProviderFactoryNokia_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::QGeoServiceProviderFactoryNokia
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryNokia_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryNokia_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_QGeoServiceProviderFactoryNokia_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::QGeoServiceProviderFactoryNokia
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_QGeoServiceProviderFactoryNokia_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::QGeoServiceProviderFactoryNokia APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryNokia_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::QGeoServiceProviderFactoryNokia APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryNokia_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QGeoServiceProviderFactoryNokia APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryNokia_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QGeoServiceProviderFactoryNokia APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryNokia_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::QGeoServiceProviderFactoryNokia APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryNokia_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::QGeoServiceProviderFactoryItemsOverlay #############

        set(qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_FRAMEWORKS_RELEASE}" "${qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_DEPS_TARGET)
            add_library(qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_LIBS_RELEASE}"
                              "${qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_DEPS_TARGET
                              qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_QGeoServiceProviderFactoryItemsOverlay"
                              "${qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::QGeoServiceProviderFactoryItemsOverlay
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::QGeoServiceProviderFactoryItemsOverlay
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::QGeoServiceProviderFactoryItemsOverlay APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::QGeoServiceProviderFactoryItemsOverlay APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QGeoServiceProviderFactoryItemsOverlay APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QGeoServiceProviderFactoryItemsOverlay APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::QGeoServiceProviderFactoryItemsOverlay APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryItemsOverlay_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::GeoServiceProviderFactoryEsri #############

        set(qt_Qt5_GeoServiceProviderFactoryEsri_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_GeoServiceProviderFactoryEsri_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_GeoServiceProviderFactoryEsri_FRAMEWORKS_RELEASE}" "${qt_Qt5_GeoServiceProviderFactoryEsri_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_GeoServiceProviderFactoryEsri_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_GeoServiceProviderFactoryEsri_DEPS_TARGET)
            add_library(qt_Qt5_GeoServiceProviderFactoryEsri_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_GeoServiceProviderFactoryEsri_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_GeoServiceProviderFactoryEsri_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_GeoServiceProviderFactoryEsri_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_GeoServiceProviderFactoryEsri_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_GeoServiceProviderFactoryEsri_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_GeoServiceProviderFactoryEsri_LIBS_RELEASE}"
                              "${qt_Qt5_GeoServiceProviderFactoryEsri_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_GeoServiceProviderFactoryEsri_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_GeoServiceProviderFactoryEsri_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_GeoServiceProviderFactoryEsri_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_GeoServiceProviderFactoryEsri_DEPS_TARGET
                              qt_Qt5_GeoServiceProviderFactoryEsri_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_GeoServiceProviderFactoryEsri"
                              "${qt_Qt5_GeoServiceProviderFactoryEsri_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::GeoServiceProviderFactoryEsri
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_GeoServiceProviderFactoryEsri_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_GeoServiceProviderFactoryEsri_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_GeoServiceProviderFactoryEsri_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::GeoServiceProviderFactoryEsri
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_GeoServiceProviderFactoryEsri_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::GeoServiceProviderFactoryEsri APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_GeoServiceProviderFactoryEsri_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::GeoServiceProviderFactoryEsri APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_GeoServiceProviderFactoryEsri_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::GeoServiceProviderFactoryEsri APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_GeoServiceProviderFactoryEsri_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::GeoServiceProviderFactoryEsri APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_GeoServiceProviderFactoryEsri_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::GeoServiceProviderFactoryEsri APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_GeoServiceProviderFactoryEsri_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::QGeoServiceProviderFactoryMapboxGL #############

        set(qt_Qt5_QGeoServiceProviderFactoryMapboxGL_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_QGeoServiceProviderFactoryMapboxGL_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_QGeoServiceProviderFactoryMapboxGL_FRAMEWORKS_RELEASE}" "${qt_Qt5_QGeoServiceProviderFactoryMapboxGL_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_QGeoServiceProviderFactoryMapboxGL_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_QGeoServiceProviderFactoryMapboxGL_DEPS_TARGET)
            add_library(qt_Qt5_QGeoServiceProviderFactoryMapboxGL_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_QGeoServiceProviderFactoryMapboxGL_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryMapboxGL_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryMapboxGL_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryMapboxGL_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_QGeoServiceProviderFactoryMapboxGL_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_QGeoServiceProviderFactoryMapboxGL_LIBS_RELEASE}"
                              "${qt_Qt5_QGeoServiceProviderFactoryMapboxGL_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_QGeoServiceProviderFactoryMapboxGL_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_QGeoServiceProviderFactoryMapboxGL_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_QGeoServiceProviderFactoryMapboxGL_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_QGeoServiceProviderFactoryMapboxGL_DEPS_TARGET
                              qt_Qt5_QGeoServiceProviderFactoryMapboxGL_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_QGeoServiceProviderFactoryMapboxGL"
                              "${qt_Qt5_QGeoServiceProviderFactoryMapboxGL_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::QGeoServiceProviderFactoryMapboxGL
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryMapboxGL_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryMapboxGL_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_QGeoServiceProviderFactoryMapboxGL_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::QGeoServiceProviderFactoryMapboxGL
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_QGeoServiceProviderFactoryMapboxGL_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::QGeoServiceProviderFactoryMapboxGL APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryMapboxGL_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::QGeoServiceProviderFactoryMapboxGL APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryMapboxGL_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QGeoServiceProviderFactoryMapboxGL APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryMapboxGL_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QGeoServiceProviderFactoryMapboxGL APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryMapboxGL_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::QGeoServiceProviderFactoryMapboxGL APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryMapboxGL_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::QGeoServiceProviderFactoryMapbox #############

        set(qt_Qt5_QGeoServiceProviderFactoryMapbox_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_QGeoServiceProviderFactoryMapbox_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_QGeoServiceProviderFactoryMapbox_FRAMEWORKS_RELEASE}" "${qt_Qt5_QGeoServiceProviderFactoryMapbox_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_QGeoServiceProviderFactoryMapbox_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_QGeoServiceProviderFactoryMapbox_DEPS_TARGET)
            add_library(qt_Qt5_QGeoServiceProviderFactoryMapbox_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_QGeoServiceProviderFactoryMapbox_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryMapbox_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryMapbox_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryMapbox_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_QGeoServiceProviderFactoryMapbox_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_QGeoServiceProviderFactoryMapbox_LIBS_RELEASE}"
                              "${qt_Qt5_QGeoServiceProviderFactoryMapbox_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_QGeoServiceProviderFactoryMapbox_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_QGeoServiceProviderFactoryMapbox_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_QGeoServiceProviderFactoryMapbox_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_QGeoServiceProviderFactoryMapbox_DEPS_TARGET
                              qt_Qt5_QGeoServiceProviderFactoryMapbox_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_QGeoServiceProviderFactoryMapbox"
                              "${qt_Qt5_QGeoServiceProviderFactoryMapbox_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::QGeoServiceProviderFactoryMapbox
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryMapbox_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryMapbox_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_QGeoServiceProviderFactoryMapbox_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::QGeoServiceProviderFactoryMapbox
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_QGeoServiceProviderFactoryMapbox_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::QGeoServiceProviderFactoryMapbox APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryMapbox_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::QGeoServiceProviderFactoryMapbox APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryMapbox_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QGeoServiceProviderFactoryMapbox APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryMapbox_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QGeoServiceProviderFactoryMapbox APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryMapbox_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::QGeoServiceProviderFactoryMapbox APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QGeoServiceProviderFactoryMapbox_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::Positioning #############

        set(qt_Qt5_Positioning_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_Positioning_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_Positioning_FRAMEWORKS_RELEASE}" "${qt_Qt5_Positioning_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_Positioning_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_Positioning_DEPS_TARGET)
            add_library(qt_Qt5_Positioning_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_Positioning_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Positioning_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Positioning_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Positioning_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_Positioning_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_Positioning_LIBS_RELEASE}"
                              "${qt_Qt5_Positioning_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_Positioning_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_Positioning_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_Positioning_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_Positioning_DEPS_TARGET
                              qt_Qt5_Positioning_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_Positioning"
                              "${qt_Qt5_Positioning_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::Positioning
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Positioning_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Positioning_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_Positioning_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::Positioning
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_Positioning_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::Positioning APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Positioning_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::Positioning APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Positioning_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Positioning APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Positioning_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Positioning APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Positioning_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::Positioning APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Positioning_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::QSvgPlugin #############

        set(qt_Qt5_QSvgPlugin_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_QSvgPlugin_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_QSvgPlugin_FRAMEWORKS_RELEASE}" "${qt_Qt5_QSvgPlugin_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_QSvgPlugin_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_QSvgPlugin_DEPS_TARGET)
            add_library(qt_Qt5_QSvgPlugin_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_QSvgPlugin_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QSvgPlugin_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QSvgPlugin_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QSvgPlugin_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_QSvgPlugin_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_QSvgPlugin_LIBS_RELEASE}"
                              "${qt_Qt5_QSvgPlugin_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_QSvgPlugin_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_QSvgPlugin_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_QSvgPlugin_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_QSvgPlugin_DEPS_TARGET
                              qt_Qt5_QSvgPlugin_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_QSvgPlugin"
                              "${qt_Qt5_QSvgPlugin_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::QSvgPlugin
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QSvgPlugin_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QSvgPlugin_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_QSvgPlugin_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::QSvgPlugin
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_QSvgPlugin_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::QSvgPlugin APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QSvgPlugin_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::QSvgPlugin APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QSvgPlugin_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QSvgPlugin APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QSvgPlugin_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QSvgPlugin APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QSvgPlugin_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::QSvgPlugin APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QSvgPlugin_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::QSvgIconPlugin #############

        set(qt_Qt5_QSvgIconPlugin_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_QSvgIconPlugin_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_QSvgIconPlugin_FRAMEWORKS_RELEASE}" "${qt_Qt5_QSvgIconPlugin_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_QSvgIconPlugin_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_QSvgIconPlugin_DEPS_TARGET)
            add_library(qt_Qt5_QSvgIconPlugin_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_QSvgIconPlugin_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QSvgIconPlugin_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QSvgIconPlugin_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QSvgIconPlugin_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_QSvgIconPlugin_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_QSvgIconPlugin_LIBS_RELEASE}"
                              "${qt_Qt5_QSvgIconPlugin_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_QSvgIconPlugin_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_QSvgIconPlugin_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_QSvgIconPlugin_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_QSvgIconPlugin_DEPS_TARGET
                              qt_Qt5_QSvgIconPlugin_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_QSvgIconPlugin"
                              "${qt_Qt5_QSvgIconPlugin_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::QSvgIconPlugin
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QSvgIconPlugin_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QSvgIconPlugin_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_QSvgIconPlugin_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::QSvgIconPlugin
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_QSvgIconPlugin_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::QSvgIconPlugin APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QSvgIconPlugin_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::QSvgIconPlugin APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QSvgIconPlugin_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QSvgIconPlugin APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QSvgIconPlugin_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QSvgIconPlugin APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QSvgIconPlugin_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::QSvgIconPlugin APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QSvgIconPlugin_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::Xml #############

        set(qt_Qt5_Xml_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_Xml_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_Xml_FRAMEWORKS_RELEASE}" "${qt_Qt5_Xml_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_Xml_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_Xml_DEPS_TARGET)
            add_library(qt_Qt5_Xml_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_Xml_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Xml_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Xml_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Xml_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_Xml_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_Xml_LIBS_RELEASE}"
                              "${qt_Qt5_Xml_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_Xml_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_Xml_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_Xml_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_Xml_DEPS_TARGET
                              qt_Qt5_Xml_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_Xml"
                              "${qt_Qt5_Xml_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::Xml
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Xml_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Xml_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_Xml_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::Xml
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_Xml_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::Xml APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Xml_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::Xml APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Xml_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Xml APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Xml_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Xml APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Xml_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::Xml APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Xml_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::Concurrent #############

        set(qt_Qt5_Concurrent_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_Concurrent_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_Concurrent_FRAMEWORKS_RELEASE}" "${qt_Qt5_Concurrent_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_Concurrent_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_Concurrent_DEPS_TARGET)
            add_library(qt_Qt5_Concurrent_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_Concurrent_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Concurrent_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Concurrent_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Concurrent_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_Concurrent_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_Concurrent_LIBS_RELEASE}"
                              "${qt_Qt5_Concurrent_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_Concurrent_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_Concurrent_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_Concurrent_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_Concurrent_DEPS_TARGET
                              qt_Qt5_Concurrent_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_Concurrent"
                              "${qt_Qt5_Concurrent_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::Concurrent
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Concurrent_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Concurrent_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_Concurrent_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::Concurrent
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_Concurrent_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::Concurrent APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Concurrent_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::Concurrent APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Concurrent_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Concurrent APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Concurrent_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Concurrent APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Concurrent_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::Concurrent APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Concurrent_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::Test #############

        set(qt_Qt5_Test_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_Test_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_Test_FRAMEWORKS_RELEASE}" "${qt_Qt5_Test_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_Test_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_Test_DEPS_TARGET)
            add_library(qt_Qt5_Test_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_Test_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Test_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Test_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Test_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_Test_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_Test_LIBS_RELEASE}"
                              "${qt_Qt5_Test_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_Test_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_Test_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_Test_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_Test_DEPS_TARGET
                              qt_Qt5_Test_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_Test"
                              "${qt_Qt5_Test_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::Test
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Test_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Test_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_Test_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::Test
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_Test_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::Test APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Test_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::Test APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Test_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Test APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Test_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Test APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Test_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::Test APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Test_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::Sql #############

        set(qt_Qt5_Sql_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_Sql_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_Sql_FRAMEWORKS_RELEASE}" "${qt_Qt5_Sql_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_Sql_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_Sql_DEPS_TARGET)
            add_library(qt_Qt5_Sql_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_Sql_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Sql_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Sql_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Sql_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_Sql_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_Sql_LIBS_RELEASE}"
                              "${qt_Qt5_Sql_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_Sql_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_Sql_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_Sql_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_Sql_DEPS_TARGET
                              qt_Qt5_Sql_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_Sql"
                              "${qt_Qt5_Sql_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::Sql
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Sql_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Sql_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_Sql_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::Sql
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_Sql_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::Sql APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Sql_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::Sql APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Sql_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Sql APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Sql_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Sql APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Sql_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::Sql APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Sql_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::Network #############

        set(qt_Qt5_Network_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_Network_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_Network_FRAMEWORKS_RELEASE}" "${qt_Qt5_Network_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_Network_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_Network_DEPS_TARGET)
            add_library(qt_Qt5_Network_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_Network_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Network_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Network_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Network_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_Network_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_Network_LIBS_RELEASE}"
                              "${qt_Qt5_Network_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_Network_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_Network_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_Network_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_Network_DEPS_TARGET
                              qt_Qt5_Network_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_Network"
                              "${qt_Qt5_Network_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::Network
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Network_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Network_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_Network_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::Network
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_Network_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::Network APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Network_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::Network APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Network_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Network APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Network_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Network APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Network_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::Network APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Network_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::QODBCDriverPlugin #############

        set(qt_Qt5_QODBCDriverPlugin_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_QODBCDriverPlugin_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_QODBCDriverPlugin_FRAMEWORKS_RELEASE}" "${qt_Qt5_QODBCDriverPlugin_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_QODBCDriverPlugin_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_QODBCDriverPlugin_DEPS_TARGET)
            add_library(qt_Qt5_QODBCDriverPlugin_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_QODBCDriverPlugin_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QODBCDriverPlugin_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QODBCDriverPlugin_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QODBCDriverPlugin_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_QODBCDriverPlugin_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_QODBCDriverPlugin_LIBS_RELEASE}"
                              "${qt_Qt5_QODBCDriverPlugin_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_QODBCDriverPlugin_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_QODBCDriverPlugin_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_QODBCDriverPlugin_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_QODBCDriverPlugin_DEPS_TARGET
                              qt_Qt5_QODBCDriverPlugin_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_QODBCDriverPlugin"
                              "${qt_Qt5_QODBCDriverPlugin_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::QODBCDriverPlugin
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QODBCDriverPlugin_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QODBCDriverPlugin_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_QODBCDriverPlugin_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::QODBCDriverPlugin
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_QODBCDriverPlugin_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::QODBCDriverPlugin APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QODBCDriverPlugin_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::QODBCDriverPlugin APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QODBCDriverPlugin_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QODBCDriverPlugin APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QODBCDriverPlugin_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QODBCDriverPlugin APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QODBCDriverPlugin_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::QODBCDriverPlugin APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QODBCDriverPlugin_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::QMySQLDriverPlugin #############

        set(qt_Qt5_QMySQLDriverPlugin_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_QMySQLDriverPlugin_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_QMySQLDriverPlugin_FRAMEWORKS_RELEASE}" "${qt_Qt5_QMySQLDriverPlugin_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_QMySQLDriverPlugin_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_QMySQLDriverPlugin_DEPS_TARGET)
            add_library(qt_Qt5_QMySQLDriverPlugin_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_QMySQLDriverPlugin_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QMySQLDriverPlugin_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QMySQLDriverPlugin_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QMySQLDriverPlugin_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_QMySQLDriverPlugin_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_QMySQLDriverPlugin_LIBS_RELEASE}"
                              "${qt_Qt5_QMySQLDriverPlugin_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_QMySQLDriverPlugin_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_QMySQLDriverPlugin_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_QMySQLDriverPlugin_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_QMySQLDriverPlugin_DEPS_TARGET
                              qt_Qt5_QMySQLDriverPlugin_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_QMySQLDriverPlugin"
                              "${qt_Qt5_QMySQLDriverPlugin_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::QMySQLDriverPlugin
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QMySQLDriverPlugin_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QMySQLDriverPlugin_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_QMySQLDriverPlugin_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::QMySQLDriverPlugin
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_QMySQLDriverPlugin_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::QMySQLDriverPlugin APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QMySQLDriverPlugin_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::QMySQLDriverPlugin APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QMySQLDriverPlugin_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QMySQLDriverPlugin APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QMySQLDriverPlugin_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QMySQLDriverPlugin APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QMySQLDriverPlugin_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::QMySQLDriverPlugin APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QMySQLDriverPlugin_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::QPSQLDriverPlugin #############

        set(qt_Qt5_QPSQLDriverPlugin_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_QPSQLDriverPlugin_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_QPSQLDriverPlugin_FRAMEWORKS_RELEASE}" "${qt_Qt5_QPSQLDriverPlugin_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_QPSQLDriverPlugin_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_QPSQLDriverPlugin_DEPS_TARGET)
            add_library(qt_Qt5_QPSQLDriverPlugin_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_QPSQLDriverPlugin_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QPSQLDriverPlugin_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QPSQLDriverPlugin_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QPSQLDriverPlugin_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_QPSQLDriverPlugin_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_QPSQLDriverPlugin_LIBS_RELEASE}"
                              "${qt_Qt5_QPSQLDriverPlugin_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_QPSQLDriverPlugin_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_QPSQLDriverPlugin_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_QPSQLDriverPlugin_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_QPSQLDriverPlugin_DEPS_TARGET
                              qt_Qt5_QPSQLDriverPlugin_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_QPSQLDriverPlugin"
                              "${qt_Qt5_QPSQLDriverPlugin_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::QPSQLDriverPlugin
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QPSQLDriverPlugin_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QPSQLDriverPlugin_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_QPSQLDriverPlugin_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::QPSQLDriverPlugin
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_QPSQLDriverPlugin_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::QPSQLDriverPlugin APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QPSQLDriverPlugin_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::QPSQLDriverPlugin APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QPSQLDriverPlugin_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QPSQLDriverPlugin APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QPSQLDriverPlugin_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QPSQLDriverPlugin APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QPSQLDriverPlugin_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::QPSQLDriverPlugin APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QPSQLDriverPlugin_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::QSQLiteDriverPlugin #############

        set(qt_Qt5_QSQLiteDriverPlugin_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_QSQLiteDriverPlugin_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_QSQLiteDriverPlugin_FRAMEWORKS_RELEASE}" "${qt_Qt5_QSQLiteDriverPlugin_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_QSQLiteDriverPlugin_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_QSQLiteDriverPlugin_DEPS_TARGET)
            add_library(qt_Qt5_QSQLiteDriverPlugin_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_QSQLiteDriverPlugin_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QSQLiteDriverPlugin_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QSQLiteDriverPlugin_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QSQLiteDriverPlugin_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_QSQLiteDriverPlugin_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_QSQLiteDriverPlugin_LIBS_RELEASE}"
                              "${qt_Qt5_QSQLiteDriverPlugin_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_QSQLiteDriverPlugin_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_QSQLiteDriverPlugin_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_QSQLiteDriverPlugin_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_QSQLiteDriverPlugin_DEPS_TARGET
                              qt_Qt5_QSQLiteDriverPlugin_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_QSQLiteDriverPlugin"
                              "${qt_Qt5_QSQLiteDriverPlugin_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::QSQLiteDriverPlugin
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_QSQLiteDriverPlugin_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_QSQLiteDriverPlugin_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_QSQLiteDriverPlugin_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::QSQLiteDriverPlugin
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_QSQLiteDriverPlugin_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::QSQLiteDriverPlugin APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QSQLiteDriverPlugin_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::QSQLiteDriverPlugin APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QSQLiteDriverPlugin_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QSQLiteDriverPlugin APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_QSQLiteDriverPlugin_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::QSQLiteDriverPlugin APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QSQLiteDriverPlugin_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::QSQLiteDriverPlugin APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_QSQLiteDriverPlugin_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::EdidSupport #############

        set(qt_Qt5_EdidSupport_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_EdidSupport_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_EdidSupport_FRAMEWORKS_RELEASE}" "${qt_Qt5_EdidSupport_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_EdidSupport_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_EdidSupport_DEPS_TARGET)
            add_library(qt_Qt5_EdidSupport_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_EdidSupport_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_EdidSupport_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_EdidSupport_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_EdidSupport_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_EdidSupport_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_EdidSupport_LIBS_RELEASE}"
                              "${qt_Qt5_EdidSupport_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_EdidSupport_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_EdidSupport_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_EdidSupport_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_EdidSupport_DEPS_TARGET
                              qt_Qt5_EdidSupport_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_EdidSupport"
                              "${qt_Qt5_EdidSupport_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::EdidSupport
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_EdidSupport_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_EdidSupport_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_EdidSupport_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::EdidSupport
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_EdidSupport_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::EdidSupport APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_EdidSupport_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::EdidSupport APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_EdidSupport_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::EdidSupport APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_EdidSupport_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::EdidSupport APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_EdidSupport_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::EdidSupport APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_EdidSupport_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::Gui #############

        set(qt_Qt5_Gui_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_Gui_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_Gui_FRAMEWORKS_RELEASE}" "${qt_Qt5_Gui_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_Gui_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_Gui_DEPS_TARGET)
            add_library(qt_Qt5_Gui_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_Gui_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Gui_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Gui_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Gui_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_Gui_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_Gui_LIBS_RELEASE}"
                              "${qt_Qt5_Gui_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_Gui_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_Gui_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_Gui_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_Gui_DEPS_TARGET
                              qt_Qt5_Gui_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_Gui"
                              "${qt_Qt5_Gui_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::Gui
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Gui_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Gui_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_Gui_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::Gui
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_Gui_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::Gui APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Gui_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::Gui APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Gui_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Gui APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Gui_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Gui APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Gui_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::Gui APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Gui_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT qt::WebEngineCore #############

        set(qt_qt_WebEngineCore_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_qt_WebEngineCore_FRAMEWORKS_FOUND_RELEASE "${qt_qt_WebEngineCore_FRAMEWORKS_RELEASE}" "${qt_qt_WebEngineCore_FRAMEWORK_DIRS_RELEASE}")

        set(qt_qt_WebEngineCore_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_qt_WebEngineCore_DEPS_TARGET)
            add_library(qt_qt_WebEngineCore_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_qt_WebEngineCore_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_qt_WebEngineCore_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_qt_WebEngineCore_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_qt_WebEngineCore_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_qt_WebEngineCore_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_qt_WebEngineCore_LIBS_RELEASE}"
                              "${qt_qt_WebEngineCore_LIB_DIRS_RELEASE}"
                              "${qt_qt_WebEngineCore_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_qt_WebEngineCore_LIBRARY_TYPE_RELEASE}"
                              "${qt_qt_WebEngineCore_IS_HOST_WINDOWS_RELEASE}"
                              qt_qt_WebEngineCore_DEPS_TARGET
                              qt_qt_WebEngineCore_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_qt_WebEngineCore"
                              "${qt_qt_WebEngineCore_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET qt::WebEngineCore
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_qt_WebEngineCore_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_qt_WebEngineCore_LIBRARIES_TARGETS}>
                     )

        if("${qt_qt_WebEngineCore_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET qt::WebEngineCore
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_qt_WebEngineCore_DEPS_TARGET)
        endif()

        set_property(TARGET qt::WebEngineCore APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_qt_WebEngineCore_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET qt::WebEngineCore APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_qt_WebEngineCore_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET qt::WebEngineCore APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_qt_WebEngineCore_LIB_DIRS_RELEASE}>)
        set_property(TARGET qt::WebEngineCore APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_qt_WebEngineCore_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET qt::WebEngineCore APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_qt_WebEngineCore_COMPILE_OPTIONS_RELEASE}>)

    ########## COMPONENT Qt5::Core #############

        set(qt_Qt5_Core_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(qt_Qt5_Core_FRAMEWORKS_FOUND_RELEASE "${qt_Qt5_Core_FRAMEWORKS_RELEASE}" "${qt_Qt5_Core_FRAMEWORK_DIRS_RELEASE}")

        set(qt_Qt5_Core_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET qt_Qt5_Core_DEPS_TARGET)
            add_library(qt_Qt5_Core_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET qt_Qt5_Core_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Core_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Core_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Core_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'qt_Qt5_Core_DEPS_TARGET' to all of them
        conan_package_library_targets("${qt_Qt5_Core_LIBS_RELEASE}"
                              "${qt_Qt5_Core_LIB_DIRS_RELEASE}"
                              "${qt_Qt5_Core_BIN_DIRS_RELEASE}" # package_bindir
                              "${qt_Qt5_Core_LIBRARY_TYPE_RELEASE}"
                              "${qt_Qt5_Core_IS_HOST_WINDOWS_RELEASE}"
                              qt_Qt5_Core_DEPS_TARGET
                              qt_Qt5_Core_LIBRARIES_TARGETS
                              "_RELEASE"
                              "qt_Qt5_Core"
                              "${qt_Qt5_Core_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Qt5::Core
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${qt_Qt5_Core_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${qt_Qt5_Core_LIBRARIES_TARGETS}>
                     )

        if("${qt_Qt5_Core_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Qt5::Core
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         qt_Qt5_Core_DEPS_TARGET)
        endif()

        set_property(TARGET Qt5::Core APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Core_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Qt5::Core APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Core_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Core APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${qt_Qt5_Core_LIB_DIRS_RELEASE}>)
        set_property(TARGET Qt5::Core APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Core_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Qt5::Core APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${qt_Qt5_Core_COMPILE_OPTIONS_RELEASE}>)

    ########## AGGREGATED GLOBAL TARGET WITH THE COMPONENTS #####################
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::WebEngineWidgets)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::WebEngine)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::WebView)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::MultimediaQuick)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::WebEngineCore)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::Location)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::QuickTemplates2)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::QuickControls2)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::QuickShapes)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::QuickWidgets)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::Quick)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::QXcbIntegrationPlugin)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::MultimediaWidgets)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::Scxml)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::WebChannel)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::QmlWorkerScript)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::QmlImportScanner)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::QmlModels)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::XcbQpa)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::PrintSupport)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::XmlPatterns)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::WebSockets)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::Multimedia)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::Svg)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::QuickTest)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::Qml)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::OpenGLExtensions)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::OpenGL)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::XkbCommonSupport)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::ServiceSupport)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::Widgets)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::AccessibilitySupport)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::ThemeSupport)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::FontDatabaseSupport)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::EventDispatcherSupport)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::QM3uPlaylistPlugin)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::QGeoPositionInfoSourceFactorySerialNmea)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::QGeoPositionInfoSourceFactoryPoll)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::QGeoPositionInfoSourceFactoryGeoclue2)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::QGeoPositionInfoSourceFactoryGeoclue)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::QGeoServiceProviderFactoryOsm)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::QGeoServiceProviderFactoryNokia)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::QGeoServiceProviderFactoryItemsOverlay)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::GeoServiceProviderFactoryEsri)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::QGeoServiceProviderFactoryMapboxGL)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::QGeoServiceProviderFactoryMapbox)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::Positioning)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::QSvgPlugin)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::QSvgIconPlugin)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::Xml)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::Concurrent)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::Test)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::Sql)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::Network)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::QODBCDriverPlugin)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::QMySQLDriverPlugin)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::QPSQLDriverPlugin)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::QSQLiteDriverPlugin)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::EdidSupport)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::Gui)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES qt::WebEngineCore)
    set_property(TARGET qt::qt APPEND PROPERTY INTERFACE_LINK_LIBRARIES Qt5::Core)

########## For the modules (FindXXX)
set(qt_LIBRARIES_RELEASE qt::qt)
