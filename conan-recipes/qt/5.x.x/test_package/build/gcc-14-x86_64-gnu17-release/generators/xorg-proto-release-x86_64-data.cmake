########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

list(APPEND xorg-proto_COMPONENT_NAMES xorg-proto::applewmproto xorg-proto::bigreqsproto xorg-proto::compositeproto xorg-proto::damageproto xorg-proto::dmxproto xorg-proto::dpmsproto xorg-proto::dri2proto xorg-proto::dri3proto xorg-proto::fixesproto xorg-proto::fontsproto xorg-proto::glproto xorg-proto::inputproto xorg-proto::kbproto xorg-proto::presentproto xorg-proto::randrproto xorg-proto::recordproto xorg-proto::renderproto xorg-proto::resourceproto xorg-proto::scrnsaverproto xorg-proto::videoproto xorg-proto::xcmiscproto xorg-proto::xextproto xorg-proto::xf86bigfontproto xorg-proto::xf86dgaproto xorg-proto::xf86driproto xorg-proto::xf86vidmodeproto xorg-proto::xineramaproto xorg-proto::xproto xorg-proto::xwaylandproto)
list(REMOVE_DUPLICATES xorg-proto_COMPONENT_NAMES)
if(DEFINED xorg-proto_FIND_DEPENDENCY_NAMES)
  list(APPEND xorg-proto_FIND_DEPENDENCY_NAMES xorg-macros)
  list(REMOVE_DUPLICATES xorg-proto_FIND_DEPENDENCY_NAMES)
else()
  set(xorg-proto_FIND_DEPENDENCY_NAMES xorg-macros)
endif()
set(xorg-macros_FIND_MODE "NO_MODULE")

########### VARIABLES #######################################################################
#############################################################################################
set(xorg-proto_PACKAGE_FOLDER_RELEASE "/home/juliangro/.conan2/p/xorg-7088df65c658e/p")
set(xorg-proto_BUILD_MODULES_PATHS_RELEASE )


set(xorg-proto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_RES_DIRS_RELEASE )
set(xorg-proto_DEFINITIONS_RELEASE )
set(xorg-proto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_OBJECTS_RELEASE )
set(xorg-proto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_COMPILE_OPTIONS_C_RELEASE )
set(xorg-proto_COMPILE_OPTIONS_CXX_RELEASE )
set(xorg-proto_LIB_DIRS_RELEASE )
set(xorg-proto_BIN_DIRS_RELEASE )
set(xorg-proto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_LIBS_RELEASE )
set(xorg-proto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_FRAMEWORKS_RELEASE )
set(xorg-proto_BUILD_DIRS_RELEASE )
set(xorg-proto_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(xorg-proto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_COMPILE_OPTIONS_C_RELEASE}>")
set(xorg-proto_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_EXE_LINK_FLAGS_RELEASE}>")


set(xorg-proto_COMPONENTS_RELEASE xorg-proto::applewmproto xorg-proto::bigreqsproto xorg-proto::compositeproto xorg-proto::damageproto xorg-proto::dmxproto xorg-proto::dpmsproto xorg-proto::dri2proto xorg-proto::dri3proto xorg-proto::fixesproto xorg-proto::fontsproto xorg-proto::glproto xorg-proto::inputproto xorg-proto::kbproto xorg-proto::presentproto xorg-proto::randrproto xorg-proto::recordproto xorg-proto::renderproto xorg-proto::resourceproto xorg-proto::scrnsaverproto xorg-proto::videoproto xorg-proto::xcmiscproto xorg-proto::xextproto xorg-proto::xf86bigfontproto xorg-proto::xf86dgaproto xorg-proto::xf86driproto xorg-proto::xf86vidmodeproto xorg-proto::xineramaproto xorg-proto::xproto xorg-proto::xwaylandproto)
########### COMPONENT xorg-proto::xwaylandproto VARIABLES ############################################

set(xorg-proto_xorg-proto_xwaylandproto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xwaylandproto_LIB_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xwaylandproto_BIN_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xwaylandproto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_xorg-proto_xwaylandproto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_xorg-proto_xwaylandproto_RES_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xwaylandproto_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_xwaylandproto_OBJECTS_RELEASE )
set(xorg-proto_xorg-proto_xwaylandproto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_xwaylandproto_COMPILE_OPTIONS_C_RELEASE "")
set(xorg-proto_xorg-proto_xwaylandproto_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg-proto_xorg-proto_xwaylandproto_LIBS_RELEASE )
set(xorg-proto_xorg-proto_xwaylandproto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_xorg-proto_xwaylandproto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xwaylandproto_FRAMEWORKS_RELEASE )
set(xorg-proto_xorg-proto_xwaylandproto_DEPENDENCIES_RELEASE xorg-macros::xorg-macros)
set(xorg-proto_xorg-proto_xwaylandproto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_xwaylandproto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_xwaylandproto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg-proto_xorg-proto_xwaylandproto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_xorg-proto_xwaylandproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_xorg-proto_xwaylandproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_xorg-proto_xwaylandproto_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg-proto_xorg-proto_xwaylandproto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_xorg-proto_xwaylandproto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_xorg-proto_xwaylandproto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg-proto::xproto VARIABLES ############################################

set(xorg-proto_xorg-proto_xproto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xproto_LIB_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xproto_BIN_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xproto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_xorg-proto_xproto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_xorg-proto_xproto_RES_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xproto_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_xproto_OBJECTS_RELEASE )
set(xorg-proto_xorg-proto_xproto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_xproto_COMPILE_OPTIONS_C_RELEASE "")
set(xorg-proto_xorg-proto_xproto_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg-proto_xorg-proto_xproto_LIBS_RELEASE )
set(xorg-proto_xorg-proto_xproto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_xorg-proto_xproto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xproto_FRAMEWORKS_RELEASE )
set(xorg-proto_xorg-proto_xproto_DEPENDENCIES_RELEASE xorg-macros::xorg-macros)
set(xorg-proto_xorg-proto_xproto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_xproto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_xproto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg-proto_xorg-proto_xproto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_xorg-proto_xproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_xorg-proto_xproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_xorg-proto_xproto_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg-proto_xorg-proto_xproto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_xorg-proto_xproto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_xorg-proto_xproto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg-proto::xineramaproto VARIABLES ############################################

set(xorg-proto_xorg-proto_xineramaproto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xineramaproto_LIB_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xineramaproto_BIN_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xineramaproto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_xorg-proto_xineramaproto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_xorg-proto_xineramaproto_RES_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xineramaproto_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_xineramaproto_OBJECTS_RELEASE )
set(xorg-proto_xorg-proto_xineramaproto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_xineramaproto_COMPILE_OPTIONS_C_RELEASE "")
set(xorg-proto_xorg-proto_xineramaproto_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg-proto_xorg-proto_xineramaproto_LIBS_RELEASE )
set(xorg-proto_xorg-proto_xineramaproto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_xorg-proto_xineramaproto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xineramaproto_FRAMEWORKS_RELEASE )
set(xorg-proto_xorg-proto_xineramaproto_DEPENDENCIES_RELEASE xorg-macros::xorg-macros)
set(xorg-proto_xorg-proto_xineramaproto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_xineramaproto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_xineramaproto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg-proto_xorg-proto_xineramaproto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_xorg-proto_xineramaproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_xorg-proto_xineramaproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_xorg-proto_xineramaproto_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg-proto_xorg-proto_xineramaproto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_xorg-proto_xineramaproto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_xorg-proto_xineramaproto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg-proto::xf86vidmodeproto VARIABLES ############################################

set(xorg-proto_xorg-proto_xf86vidmodeproto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xf86vidmodeproto_LIB_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xf86vidmodeproto_BIN_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xf86vidmodeproto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_xorg-proto_xf86vidmodeproto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_xorg-proto_xf86vidmodeproto_RES_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xf86vidmodeproto_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_xf86vidmodeproto_OBJECTS_RELEASE )
set(xorg-proto_xorg-proto_xf86vidmodeproto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_xf86vidmodeproto_COMPILE_OPTIONS_C_RELEASE "")
set(xorg-proto_xorg-proto_xf86vidmodeproto_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg-proto_xorg-proto_xf86vidmodeproto_LIBS_RELEASE )
set(xorg-proto_xorg-proto_xf86vidmodeproto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_xorg-proto_xf86vidmodeproto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xf86vidmodeproto_FRAMEWORKS_RELEASE )
set(xorg-proto_xorg-proto_xf86vidmodeproto_DEPENDENCIES_RELEASE xorg-macros::xorg-macros)
set(xorg-proto_xorg-proto_xf86vidmodeproto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_xf86vidmodeproto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_xf86vidmodeproto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg-proto_xorg-proto_xf86vidmodeproto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_xorg-proto_xf86vidmodeproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_xorg-proto_xf86vidmodeproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_xorg-proto_xf86vidmodeproto_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg-proto_xorg-proto_xf86vidmodeproto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_xorg-proto_xf86vidmodeproto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_xorg-proto_xf86vidmodeproto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg-proto::xf86driproto VARIABLES ############################################

set(xorg-proto_xorg-proto_xf86driproto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xf86driproto_LIB_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xf86driproto_BIN_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xf86driproto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_xorg-proto_xf86driproto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_xorg-proto_xf86driproto_RES_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xf86driproto_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_xf86driproto_OBJECTS_RELEASE )
set(xorg-proto_xorg-proto_xf86driproto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_xf86driproto_COMPILE_OPTIONS_C_RELEASE "")
set(xorg-proto_xorg-proto_xf86driproto_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg-proto_xorg-proto_xf86driproto_LIBS_RELEASE )
set(xorg-proto_xorg-proto_xf86driproto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_xorg-proto_xf86driproto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xf86driproto_FRAMEWORKS_RELEASE )
set(xorg-proto_xorg-proto_xf86driproto_DEPENDENCIES_RELEASE xorg-macros::xorg-macros)
set(xorg-proto_xorg-proto_xf86driproto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_xf86driproto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_xf86driproto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg-proto_xorg-proto_xf86driproto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_xorg-proto_xf86driproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_xorg-proto_xf86driproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_xorg-proto_xf86driproto_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg-proto_xorg-proto_xf86driproto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_xorg-proto_xf86driproto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_xorg-proto_xf86driproto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg-proto::xf86dgaproto VARIABLES ############################################

set(xorg-proto_xorg-proto_xf86dgaproto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xf86dgaproto_LIB_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xf86dgaproto_BIN_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xf86dgaproto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_xorg-proto_xf86dgaproto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_xorg-proto_xf86dgaproto_RES_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xf86dgaproto_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_xf86dgaproto_OBJECTS_RELEASE )
set(xorg-proto_xorg-proto_xf86dgaproto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_xf86dgaproto_COMPILE_OPTIONS_C_RELEASE "")
set(xorg-proto_xorg-proto_xf86dgaproto_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg-proto_xorg-proto_xf86dgaproto_LIBS_RELEASE )
set(xorg-proto_xorg-proto_xf86dgaproto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_xorg-proto_xf86dgaproto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xf86dgaproto_FRAMEWORKS_RELEASE )
set(xorg-proto_xorg-proto_xf86dgaproto_DEPENDENCIES_RELEASE xorg-macros::xorg-macros)
set(xorg-proto_xorg-proto_xf86dgaproto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_xf86dgaproto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_xf86dgaproto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg-proto_xorg-proto_xf86dgaproto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_xorg-proto_xf86dgaproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_xorg-proto_xf86dgaproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_xorg-proto_xf86dgaproto_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg-proto_xorg-proto_xf86dgaproto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_xorg-proto_xf86dgaproto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_xorg-proto_xf86dgaproto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg-proto::xf86bigfontproto VARIABLES ############################################

set(xorg-proto_xorg-proto_xf86bigfontproto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xf86bigfontproto_LIB_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xf86bigfontproto_BIN_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xf86bigfontproto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_xorg-proto_xf86bigfontproto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_xorg-proto_xf86bigfontproto_RES_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xf86bigfontproto_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_xf86bigfontproto_OBJECTS_RELEASE )
set(xorg-proto_xorg-proto_xf86bigfontproto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_xf86bigfontproto_COMPILE_OPTIONS_C_RELEASE "")
set(xorg-proto_xorg-proto_xf86bigfontproto_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg-proto_xorg-proto_xf86bigfontproto_LIBS_RELEASE )
set(xorg-proto_xorg-proto_xf86bigfontproto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_xorg-proto_xf86bigfontproto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xf86bigfontproto_FRAMEWORKS_RELEASE )
set(xorg-proto_xorg-proto_xf86bigfontproto_DEPENDENCIES_RELEASE xorg-macros::xorg-macros)
set(xorg-proto_xorg-proto_xf86bigfontproto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_xf86bigfontproto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_xf86bigfontproto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg-proto_xorg-proto_xf86bigfontproto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_xorg-proto_xf86bigfontproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_xorg-proto_xf86bigfontproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_xorg-proto_xf86bigfontproto_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg-proto_xorg-proto_xf86bigfontproto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_xorg-proto_xf86bigfontproto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_xorg-proto_xf86bigfontproto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg-proto::xextproto VARIABLES ############################################

set(xorg-proto_xorg-proto_xextproto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xextproto_LIB_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xextproto_BIN_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xextproto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_xorg-proto_xextproto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_xorg-proto_xextproto_RES_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xextproto_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_xextproto_OBJECTS_RELEASE )
set(xorg-proto_xorg-proto_xextproto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_xextproto_COMPILE_OPTIONS_C_RELEASE "")
set(xorg-proto_xorg-proto_xextproto_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg-proto_xorg-proto_xextproto_LIBS_RELEASE )
set(xorg-proto_xorg-proto_xextproto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_xorg-proto_xextproto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xextproto_FRAMEWORKS_RELEASE )
set(xorg-proto_xorg-proto_xextproto_DEPENDENCIES_RELEASE xorg-macros::xorg-macros)
set(xorg-proto_xorg-proto_xextproto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_xextproto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_xextproto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg-proto_xorg-proto_xextproto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_xorg-proto_xextproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_xorg-proto_xextproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_xorg-proto_xextproto_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg-proto_xorg-proto_xextproto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_xorg-proto_xextproto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_xorg-proto_xextproto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg-proto::xcmiscproto VARIABLES ############################################

set(xorg-proto_xorg-proto_xcmiscproto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xcmiscproto_LIB_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xcmiscproto_BIN_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xcmiscproto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_xorg-proto_xcmiscproto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_xorg-proto_xcmiscproto_RES_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xcmiscproto_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_xcmiscproto_OBJECTS_RELEASE )
set(xorg-proto_xorg-proto_xcmiscproto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_xcmiscproto_COMPILE_OPTIONS_C_RELEASE "")
set(xorg-proto_xorg-proto_xcmiscproto_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg-proto_xorg-proto_xcmiscproto_LIBS_RELEASE )
set(xorg-proto_xorg-proto_xcmiscproto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_xorg-proto_xcmiscproto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_xorg-proto_xcmiscproto_FRAMEWORKS_RELEASE )
set(xorg-proto_xorg-proto_xcmiscproto_DEPENDENCIES_RELEASE xorg-macros::xorg-macros)
set(xorg-proto_xorg-proto_xcmiscproto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_xcmiscproto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_xcmiscproto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg-proto_xorg-proto_xcmiscproto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_xorg-proto_xcmiscproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_xorg-proto_xcmiscproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_xorg-proto_xcmiscproto_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg-proto_xorg-proto_xcmiscproto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_xorg-proto_xcmiscproto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_xorg-proto_xcmiscproto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg-proto::videoproto VARIABLES ############################################

set(xorg-proto_xorg-proto_videoproto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_xorg-proto_videoproto_LIB_DIRS_RELEASE )
set(xorg-proto_xorg-proto_videoproto_BIN_DIRS_RELEASE )
set(xorg-proto_xorg-proto_videoproto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_xorg-proto_videoproto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_xorg-proto_videoproto_RES_DIRS_RELEASE )
set(xorg-proto_xorg-proto_videoproto_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_videoproto_OBJECTS_RELEASE )
set(xorg-proto_xorg-proto_videoproto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_videoproto_COMPILE_OPTIONS_C_RELEASE "")
set(xorg-proto_xorg-proto_videoproto_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg-proto_xorg-proto_videoproto_LIBS_RELEASE )
set(xorg-proto_xorg-proto_videoproto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_xorg-proto_videoproto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_xorg-proto_videoproto_FRAMEWORKS_RELEASE )
set(xorg-proto_xorg-proto_videoproto_DEPENDENCIES_RELEASE xorg-macros::xorg-macros)
set(xorg-proto_xorg-proto_videoproto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_videoproto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_videoproto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg-proto_xorg-proto_videoproto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_xorg-proto_videoproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_xorg-proto_videoproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_xorg-proto_videoproto_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg-proto_xorg-proto_videoproto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_xorg-proto_videoproto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_xorg-proto_videoproto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg-proto::scrnsaverproto VARIABLES ############################################

set(xorg-proto_xorg-proto_scrnsaverproto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_xorg-proto_scrnsaverproto_LIB_DIRS_RELEASE )
set(xorg-proto_xorg-proto_scrnsaverproto_BIN_DIRS_RELEASE )
set(xorg-proto_xorg-proto_scrnsaverproto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_xorg-proto_scrnsaverproto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_xorg-proto_scrnsaverproto_RES_DIRS_RELEASE )
set(xorg-proto_xorg-proto_scrnsaverproto_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_scrnsaverproto_OBJECTS_RELEASE )
set(xorg-proto_xorg-proto_scrnsaverproto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_scrnsaverproto_COMPILE_OPTIONS_C_RELEASE "")
set(xorg-proto_xorg-proto_scrnsaverproto_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg-proto_xorg-proto_scrnsaverproto_LIBS_RELEASE )
set(xorg-proto_xorg-proto_scrnsaverproto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_xorg-proto_scrnsaverproto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_xorg-proto_scrnsaverproto_FRAMEWORKS_RELEASE )
set(xorg-proto_xorg-proto_scrnsaverproto_DEPENDENCIES_RELEASE xorg-macros::xorg-macros)
set(xorg-proto_xorg-proto_scrnsaverproto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_scrnsaverproto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_scrnsaverproto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg-proto_xorg-proto_scrnsaverproto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_xorg-proto_scrnsaverproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_xorg-proto_scrnsaverproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_xorg-proto_scrnsaverproto_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg-proto_xorg-proto_scrnsaverproto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_xorg-proto_scrnsaverproto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_xorg-proto_scrnsaverproto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg-proto::resourceproto VARIABLES ############################################

set(xorg-proto_xorg-proto_resourceproto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_xorg-proto_resourceproto_LIB_DIRS_RELEASE )
set(xorg-proto_xorg-proto_resourceproto_BIN_DIRS_RELEASE )
set(xorg-proto_xorg-proto_resourceproto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_xorg-proto_resourceproto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_xorg-proto_resourceproto_RES_DIRS_RELEASE )
set(xorg-proto_xorg-proto_resourceproto_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_resourceproto_OBJECTS_RELEASE )
set(xorg-proto_xorg-proto_resourceproto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_resourceproto_COMPILE_OPTIONS_C_RELEASE "")
set(xorg-proto_xorg-proto_resourceproto_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg-proto_xorg-proto_resourceproto_LIBS_RELEASE )
set(xorg-proto_xorg-proto_resourceproto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_xorg-proto_resourceproto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_xorg-proto_resourceproto_FRAMEWORKS_RELEASE )
set(xorg-proto_xorg-proto_resourceproto_DEPENDENCIES_RELEASE xorg-macros::xorg-macros)
set(xorg-proto_xorg-proto_resourceproto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_resourceproto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_resourceproto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg-proto_xorg-proto_resourceproto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_xorg-proto_resourceproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_xorg-proto_resourceproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_xorg-proto_resourceproto_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg-proto_xorg-proto_resourceproto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_xorg-proto_resourceproto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_xorg-proto_resourceproto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg-proto::renderproto VARIABLES ############################################

set(xorg-proto_xorg-proto_renderproto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_xorg-proto_renderproto_LIB_DIRS_RELEASE )
set(xorg-proto_xorg-proto_renderproto_BIN_DIRS_RELEASE )
set(xorg-proto_xorg-proto_renderproto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_xorg-proto_renderproto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_xorg-proto_renderproto_RES_DIRS_RELEASE )
set(xorg-proto_xorg-proto_renderproto_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_renderproto_OBJECTS_RELEASE )
set(xorg-proto_xorg-proto_renderproto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_renderproto_COMPILE_OPTIONS_C_RELEASE "")
set(xorg-proto_xorg-proto_renderproto_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg-proto_xorg-proto_renderproto_LIBS_RELEASE )
set(xorg-proto_xorg-proto_renderproto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_xorg-proto_renderproto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_xorg-proto_renderproto_FRAMEWORKS_RELEASE )
set(xorg-proto_xorg-proto_renderproto_DEPENDENCIES_RELEASE xorg-macros::xorg-macros)
set(xorg-proto_xorg-proto_renderproto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_renderproto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_renderproto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg-proto_xorg-proto_renderproto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_xorg-proto_renderproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_xorg-proto_renderproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_xorg-proto_renderproto_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg-proto_xorg-proto_renderproto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_xorg-proto_renderproto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_xorg-proto_renderproto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg-proto::recordproto VARIABLES ############################################

set(xorg-proto_xorg-proto_recordproto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_xorg-proto_recordproto_LIB_DIRS_RELEASE )
set(xorg-proto_xorg-proto_recordproto_BIN_DIRS_RELEASE )
set(xorg-proto_xorg-proto_recordproto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_xorg-proto_recordproto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_xorg-proto_recordproto_RES_DIRS_RELEASE )
set(xorg-proto_xorg-proto_recordproto_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_recordproto_OBJECTS_RELEASE )
set(xorg-proto_xorg-proto_recordproto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_recordproto_COMPILE_OPTIONS_C_RELEASE "")
set(xorg-proto_xorg-proto_recordproto_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg-proto_xorg-proto_recordproto_LIBS_RELEASE )
set(xorg-proto_xorg-proto_recordproto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_xorg-proto_recordproto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_xorg-proto_recordproto_FRAMEWORKS_RELEASE )
set(xorg-proto_xorg-proto_recordproto_DEPENDENCIES_RELEASE xorg-macros::xorg-macros)
set(xorg-proto_xorg-proto_recordproto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_recordproto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_recordproto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg-proto_xorg-proto_recordproto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_xorg-proto_recordproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_xorg-proto_recordproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_xorg-proto_recordproto_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg-proto_xorg-proto_recordproto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_xorg-proto_recordproto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_xorg-proto_recordproto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg-proto::randrproto VARIABLES ############################################

set(xorg-proto_xorg-proto_randrproto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_xorg-proto_randrproto_LIB_DIRS_RELEASE )
set(xorg-proto_xorg-proto_randrproto_BIN_DIRS_RELEASE )
set(xorg-proto_xorg-proto_randrproto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_xorg-proto_randrproto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_xorg-proto_randrproto_RES_DIRS_RELEASE )
set(xorg-proto_xorg-proto_randrproto_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_randrproto_OBJECTS_RELEASE )
set(xorg-proto_xorg-proto_randrproto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_randrproto_COMPILE_OPTIONS_C_RELEASE "")
set(xorg-proto_xorg-proto_randrproto_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg-proto_xorg-proto_randrproto_LIBS_RELEASE )
set(xorg-proto_xorg-proto_randrproto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_xorg-proto_randrproto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_xorg-proto_randrproto_FRAMEWORKS_RELEASE )
set(xorg-proto_xorg-proto_randrproto_DEPENDENCIES_RELEASE xorg-macros::xorg-macros)
set(xorg-proto_xorg-proto_randrproto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_randrproto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_randrproto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg-proto_xorg-proto_randrproto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_xorg-proto_randrproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_xorg-proto_randrproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_xorg-proto_randrproto_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg-proto_xorg-proto_randrproto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_xorg-proto_randrproto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_xorg-proto_randrproto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg-proto::presentproto VARIABLES ############################################

set(xorg-proto_xorg-proto_presentproto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_xorg-proto_presentproto_LIB_DIRS_RELEASE )
set(xorg-proto_xorg-proto_presentproto_BIN_DIRS_RELEASE )
set(xorg-proto_xorg-proto_presentproto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_xorg-proto_presentproto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_xorg-proto_presentproto_RES_DIRS_RELEASE )
set(xorg-proto_xorg-proto_presentproto_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_presentproto_OBJECTS_RELEASE )
set(xorg-proto_xorg-proto_presentproto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_presentproto_COMPILE_OPTIONS_C_RELEASE "")
set(xorg-proto_xorg-proto_presentproto_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg-proto_xorg-proto_presentproto_LIBS_RELEASE )
set(xorg-proto_xorg-proto_presentproto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_xorg-proto_presentproto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_xorg-proto_presentproto_FRAMEWORKS_RELEASE )
set(xorg-proto_xorg-proto_presentproto_DEPENDENCIES_RELEASE xorg-macros::xorg-macros)
set(xorg-proto_xorg-proto_presentproto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_presentproto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_presentproto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg-proto_xorg-proto_presentproto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_xorg-proto_presentproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_xorg-proto_presentproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_xorg-proto_presentproto_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg-proto_xorg-proto_presentproto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_xorg-proto_presentproto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_xorg-proto_presentproto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg-proto::kbproto VARIABLES ############################################

set(xorg-proto_xorg-proto_kbproto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_xorg-proto_kbproto_LIB_DIRS_RELEASE )
set(xorg-proto_xorg-proto_kbproto_BIN_DIRS_RELEASE )
set(xorg-proto_xorg-proto_kbproto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_xorg-proto_kbproto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_xorg-proto_kbproto_RES_DIRS_RELEASE )
set(xorg-proto_xorg-proto_kbproto_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_kbproto_OBJECTS_RELEASE )
set(xorg-proto_xorg-proto_kbproto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_kbproto_COMPILE_OPTIONS_C_RELEASE "")
set(xorg-proto_xorg-proto_kbproto_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg-proto_xorg-proto_kbproto_LIBS_RELEASE )
set(xorg-proto_xorg-proto_kbproto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_xorg-proto_kbproto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_xorg-proto_kbproto_FRAMEWORKS_RELEASE )
set(xorg-proto_xorg-proto_kbproto_DEPENDENCIES_RELEASE xorg-macros::xorg-macros)
set(xorg-proto_xorg-proto_kbproto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_kbproto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_kbproto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg-proto_xorg-proto_kbproto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_xorg-proto_kbproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_xorg-proto_kbproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_xorg-proto_kbproto_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg-proto_xorg-proto_kbproto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_xorg-proto_kbproto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_xorg-proto_kbproto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg-proto::inputproto VARIABLES ############################################

set(xorg-proto_xorg-proto_inputproto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_xorg-proto_inputproto_LIB_DIRS_RELEASE )
set(xorg-proto_xorg-proto_inputproto_BIN_DIRS_RELEASE )
set(xorg-proto_xorg-proto_inputproto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_xorg-proto_inputproto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_xorg-proto_inputproto_RES_DIRS_RELEASE )
set(xorg-proto_xorg-proto_inputproto_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_inputproto_OBJECTS_RELEASE )
set(xorg-proto_xorg-proto_inputproto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_inputproto_COMPILE_OPTIONS_C_RELEASE "")
set(xorg-proto_xorg-proto_inputproto_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg-proto_xorg-proto_inputproto_LIBS_RELEASE )
set(xorg-proto_xorg-proto_inputproto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_xorg-proto_inputproto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_xorg-proto_inputproto_FRAMEWORKS_RELEASE )
set(xorg-proto_xorg-proto_inputproto_DEPENDENCIES_RELEASE xorg-macros::xorg-macros)
set(xorg-proto_xorg-proto_inputproto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_inputproto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_inputproto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg-proto_xorg-proto_inputproto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_xorg-proto_inputproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_xorg-proto_inputproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_xorg-proto_inputproto_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg-proto_xorg-proto_inputproto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_xorg-proto_inputproto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_xorg-proto_inputproto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg-proto::glproto VARIABLES ############################################

set(xorg-proto_xorg-proto_glproto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_xorg-proto_glproto_LIB_DIRS_RELEASE )
set(xorg-proto_xorg-proto_glproto_BIN_DIRS_RELEASE )
set(xorg-proto_xorg-proto_glproto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_xorg-proto_glproto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_xorg-proto_glproto_RES_DIRS_RELEASE )
set(xorg-proto_xorg-proto_glproto_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_glproto_OBJECTS_RELEASE )
set(xorg-proto_xorg-proto_glproto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_glproto_COMPILE_OPTIONS_C_RELEASE "")
set(xorg-proto_xorg-proto_glproto_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg-proto_xorg-proto_glproto_LIBS_RELEASE )
set(xorg-proto_xorg-proto_glproto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_xorg-proto_glproto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_xorg-proto_glproto_FRAMEWORKS_RELEASE )
set(xorg-proto_xorg-proto_glproto_DEPENDENCIES_RELEASE xorg-macros::xorg-macros)
set(xorg-proto_xorg-proto_glproto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_glproto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_glproto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg-proto_xorg-proto_glproto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_xorg-proto_glproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_xorg-proto_glproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_xorg-proto_glproto_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg-proto_xorg-proto_glproto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_xorg-proto_glproto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_xorg-proto_glproto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg-proto::fontsproto VARIABLES ############################################

set(xorg-proto_xorg-proto_fontsproto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_xorg-proto_fontsproto_LIB_DIRS_RELEASE )
set(xorg-proto_xorg-proto_fontsproto_BIN_DIRS_RELEASE )
set(xorg-proto_xorg-proto_fontsproto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_xorg-proto_fontsproto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_xorg-proto_fontsproto_RES_DIRS_RELEASE )
set(xorg-proto_xorg-proto_fontsproto_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_fontsproto_OBJECTS_RELEASE )
set(xorg-proto_xorg-proto_fontsproto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_fontsproto_COMPILE_OPTIONS_C_RELEASE "")
set(xorg-proto_xorg-proto_fontsproto_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg-proto_xorg-proto_fontsproto_LIBS_RELEASE )
set(xorg-proto_xorg-proto_fontsproto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_xorg-proto_fontsproto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_xorg-proto_fontsproto_FRAMEWORKS_RELEASE )
set(xorg-proto_xorg-proto_fontsproto_DEPENDENCIES_RELEASE xorg-macros::xorg-macros)
set(xorg-proto_xorg-proto_fontsproto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_fontsproto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_fontsproto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg-proto_xorg-proto_fontsproto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_xorg-proto_fontsproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_xorg-proto_fontsproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_xorg-proto_fontsproto_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg-proto_xorg-proto_fontsproto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_xorg-proto_fontsproto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_xorg-proto_fontsproto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg-proto::fixesproto VARIABLES ############################################

set(xorg-proto_xorg-proto_fixesproto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_xorg-proto_fixesproto_LIB_DIRS_RELEASE )
set(xorg-proto_xorg-proto_fixesproto_BIN_DIRS_RELEASE )
set(xorg-proto_xorg-proto_fixesproto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_xorg-proto_fixesproto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_xorg-proto_fixesproto_RES_DIRS_RELEASE )
set(xorg-proto_xorg-proto_fixesproto_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_fixesproto_OBJECTS_RELEASE )
set(xorg-proto_xorg-proto_fixesproto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_fixesproto_COMPILE_OPTIONS_C_RELEASE "")
set(xorg-proto_xorg-proto_fixesproto_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg-proto_xorg-proto_fixesproto_LIBS_RELEASE )
set(xorg-proto_xorg-proto_fixesproto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_xorg-proto_fixesproto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_xorg-proto_fixesproto_FRAMEWORKS_RELEASE )
set(xorg-proto_xorg-proto_fixesproto_DEPENDENCIES_RELEASE xorg-macros::xorg-macros)
set(xorg-proto_xorg-proto_fixesproto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_fixesproto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_fixesproto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg-proto_xorg-proto_fixesproto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_xorg-proto_fixesproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_xorg-proto_fixesproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_xorg-proto_fixesproto_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg-proto_xorg-proto_fixesproto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_xorg-proto_fixesproto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_xorg-proto_fixesproto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg-proto::dri3proto VARIABLES ############################################

set(xorg-proto_xorg-proto_dri3proto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_xorg-proto_dri3proto_LIB_DIRS_RELEASE )
set(xorg-proto_xorg-proto_dri3proto_BIN_DIRS_RELEASE )
set(xorg-proto_xorg-proto_dri3proto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_xorg-proto_dri3proto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_xorg-proto_dri3proto_RES_DIRS_RELEASE )
set(xorg-proto_xorg-proto_dri3proto_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_dri3proto_OBJECTS_RELEASE )
set(xorg-proto_xorg-proto_dri3proto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_dri3proto_COMPILE_OPTIONS_C_RELEASE "")
set(xorg-proto_xorg-proto_dri3proto_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg-proto_xorg-proto_dri3proto_LIBS_RELEASE )
set(xorg-proto_xorg-proto_dri3proto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_xorg-proto_dri3proto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_xorg-proto_dri3proto_FRAMEWORKS_RELEASE )
set(xorg-proto_xorg-proto_dri3proto_DEPENDENCIES_RELEASE xorg-macros::xorg-macros)
set(xorg-proto_xorg-proto_dri3proto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_dri3proto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_dri3proto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg-proto_xorg-proto_dri3proto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_xorg-proto_dri3proto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_xorg-proto_dri3proto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_xorg-proto_dri3proto_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg-proto_xorg-proto_dri3proto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_xorg-proto_dri3proto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_xorg-proto_dri3proto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg-proto::dri2proto VARIABLES ############################################

set(xorg-proto_xorg-proto_dri2proto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_xorg-proto_dri2proto_LIB_DIRS_RELEASE )
set(xorg-proto_xorg-proto_dri2proto_BIN_DIRS_RELEASE )
set(xorg-proto_xorg-proto_dri2proto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_xorg-proto_dri2proto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_xorg-proto_dri2proto_RES_DIRS_RELEASE )
set(xorg-proto_xorg-proto_dri2proto_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_dri2proto_OBJECTS_RELEASE )
set(xorg-proto_xorg-proto_dri2proto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_dri2proto_COMPILE_OPTIONS_C_RELEASE "")
set(xorg-proto_xorg-proto_dri2proto_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg-proto_xorg-proto_dri2proto_LIBS_RELEASE )
set(xorg-proto_xorg-proto_dri2proto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_xorg-proto_dri2proto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_xorg-proto_dri2proto_FRAMEWORKS_RELEASE )
set(xorg-proto_xorg-proto_dri2proto_DEPENDENCIES_RELEASE xorg-macros::xorg-macros)
set(xorg-proto_xorg-proto_dri2proto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_dri2proto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_dri2proto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg-proto_xorg-proto_dri2proto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_xorg-proto_dri2proto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_xorg-proto_dri2proto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_xorg-proto_dri2proto_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg-proto_xorg-proto_dri2proto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_xorg-proto_dri2proto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_xorg-proto_dri2proto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg-proto::dpmsproto VARIABLES ############################################

set(xorg-proto_xorg-proto_dpmsproto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_xorg-proto_dpmsproto_LIB_DIRS_RELEASE )
set(xorg-proto_xorg-proto_dpmsproto_BIN_DIRS_RELEASE )
set(xorg-proto_xorg-proto_dpmsproto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_xorg-proto_dpmsproto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_xorg-proto_dpmsproto_RES_DIRS_RELEASE )
set(xorg-proto_xorg-proto_dpmsproto_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_dpmsproto_OBJECTS_RELEASE )
set(xorg-proto_xorg-proto_dpmsproto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_dpmsproto_COMPILE_OPTIONS_C_RELEASE "")
set(xorg-proto_xorg-proto_dpmsproto_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg-proto_xorg-proto_dpmsproto_LIBS_RELEASE )
set(xorg-proto_xorg-proto_dpmsproto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_xorg-proto_dpmsproto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_xorg-proto_dpmsproto_FRAMEWORKS_RELEASE )
set(xorg-proto_xorg-proto_dpmsproto_DEPENDENCIES_RELEASE xorg-macros::xorg-macros)
set(xorg-proto_xorg-proto_dpmsproto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_dpmsproto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_dpmsproto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg-proto_xorg-proto_dpmsproto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_xorg-proto_dpmsproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_xorg-proto_dpmsproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_xorg-proto_dpmsproto_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg-proto_xorg-proto_dpmsproto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_xorg-proto_dpmsproto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_xorg-proto_dpmsproto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg-proto::dmxproto VARIABLES ############################################

set(xorg-proto_xorg-proto_dmxproto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_xorg-proto_dmxproto_LIB_DIRS_RELEASE )
set(xorg-proto_xorg-proto_dmxproto_BIN_DIRS_RELEASE )
set(xorg-proto_xorg-proto_dmxproto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_xorg-proto_dmxproto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_xorg-proto_dmxproto_RES_DIRS_RELEASE )
set(xorg-proto_xorg-proto_dmxproto_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_dmxproto_OBJECTS_RELEASE )
set(xorg-proto_xorg-proto_dmxproto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_dmxproto_COMPILE_OPTIONS_C_RELEASE "")
set(xorg-proto_xorg-proto_dmxproto_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg-proto_xorg-proto_dmxproto_LIBS_RELEASE )
set(xorg-proto_xorg-proto_dmxproto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_xorg-proto_dmxproto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_xorg-proto_dmxproto_FRAMEWORKS_RELEASE )
set(xorg-proto_xorg-proto_dmxproto_DEPENDENCIES_RELEASE xorg-macros::xorg-macros)
set(xorg-proto_xorg-proto_dmxproto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_dmxproto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_dmxproto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg-proto_xorg-proto_dmxproto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_xorg-proto_dmxproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_xorg-proto_dmxproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_xorg-proto_dmxproto_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg-proto_xorg-proto_dmxproto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_xorg-proto_dmxproto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_xorg-proto_dmxproto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg-proto::damageproto VARIABLES ############################################

set(xorg-proto_xorg-proto_damageproto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_xorg-proto_damageproto_LIB_DIRS_RELEASE )
set(xorg-proto_xorg-proto_damageproto_BIN_DIRS_RELEASE )
set(xorg-proto_xorg-proto_damageproto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_xorg-proto_damageproto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_xorg-proto_damageproto_RES_DIRS_RELEASE )
set(xorg-proto_xorg-proto_damageproto_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_damageproto_OBJECTS_RELEASE )
set(xorg-proto_xorg-proto_damageproto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_damageproto_COMPILE_OPTIONS_C_RELEASE "")
set(xorg-proto_xorg-proto_damageproto_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg-proto_xorg-proto_damageproto_LIBS_RELEASE )
set(xorg-proto_xorg-proto_damageproto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_xorg-proto_damageproto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_xorg-proto_damageproto_FRAMEWORKS_RELEASE )
set(xorg-proto_xorg-proto_damageproto_DEPENDENCIES_RELEASE xorg-macros::xorg-macros)
set(xorg-proto_xorg-proto_damageproto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_damageproto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_damageproto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg-proto_xorg-proto_damageproto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_xorg-proto_damageproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_xorg-proto_damageproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_xorg-proto_damageproto_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg-proto_xorg-proto_damageproto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_xorg-proto_damageproto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_xorg-proto_damageproto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg-proto::compositeproto VARIABLES ############################################

set(xorg-proto_xorg-proto_compositeproto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_xorg-proto_compositeproto_LIB_DIRS_RELEASE )
set(xorg-proto_xorg-proto_compositeproto_BIN_DIRS_RELEASE )
set(xorg-proto_xorg-proto_compositeproto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_xorg-proto_compositeproto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_xorg-proto_compositeproto_RES_DIRS_RELEASE )
set(xorg-proto_xorg-proto_compositeproto_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_compositeproto_OBJECTS_RELEASE )
set(xorg-proto_xorg-proto_compositeproto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_compositeproto_COMPILE_OPTIONS_C_RELEASE "")
set(xorg-proto_xorg-proto_compositeproto_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg-proto_xorg-proto_compositeproto_LIBS_RELEASE )
set(xorg-proto_xorg-proto_compositeproto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_xorg-proto_compositeproto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_xorg-proto_compositeproto_FRAMEWORKS_RELEASE )
set(xorg-proto_xorg-proto_compositeproto_DEPENDENCIES_RELEASE xorg-macros::xorg-macros)
set(xorg-proto_xorg-proto_compositeproto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_compositeproto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_compositeproto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg-proto_xorg-proto_compositeproto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_xorg-proto_compositeproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_xorg-proto_compositeproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_xorg-proto_compositeproto_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg-proto_xorg-proto_compositeproto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_xorg-proto_compositeproto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_xorg-proto_compositeproto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg-proto::bigreqsproto VARIABLES ############################################

set(xorg-proto_xorg-proto_bigreqsproto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_xorg-proto_bigreqsproto_LIB_DIRS_RELEASE )
set(xorg-proto_xorg-proto_bigreqsproto_BIN_DIRS_RELEASE )
set(xorg-proto_xorg-proto_bigreqsproto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_xorg-proto_bigreqsproto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_xorg-proto_bigreqsproto_RES_DIRS_RELEASE )
set(xorg-proto_xorg-proto_bigreqsproto_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_bigreqsproto_OBJECTS_RELEASE )
set(xorg-proto_xorg-proto_bigreqsproto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_bigreqsproto_COMPILE_OPTIONS_C_RELEASE "")
set(xorg-proto_xorg-proto_bigreqsproto_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg-proto_xorg-proto_bigreqsproto_LIBS_RELEASE )
set(xorg-proto_xorg-proto_bigreqsproto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_xorg-proto_bigreqsproto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_xorg-proto_bigreqsproto_FRAMEWORKS_RELEASE )
set(xorg-proto_xorg-proto_bigreqsproto_DEPENDENCIES_RELEASE xorg-macros::xorg-macros)
set(xorg-proto_xorg-proto_bigreqsproto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_bigreqsproto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_bigreqsproto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg-proto_xorg-proto_bigreqsproto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_xorg-proto_bigreqsproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_xorg-proto_bigreqsproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_xorg-proto_bigreqsproto_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg-proto_xorg-proto_bigreqsproto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_xorg-proto_bigreqsproto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_xorg-proto_bigreqsproto_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg-proto::applewmproto VARIABLES ############################################

set(xorg-proto_xorg-proto_applewmproto_INCLUDE_DIRS_RELEASE )
set(xorg-proto_xorg-proto_applewmproto_LIB_DIRS_RELEASE )
set(xorg-proto_xorg-proto_applewmproto_BIN_DIRS_RELEASE )
set(xorg-proto_xorg-proto_applewmproto_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-proto_xorg-proto_applewmproto_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-proto_xorg-proto_applewmproto_RES_DIRS_RELEASE )
set(xorg-proto_xorg-proto_applewmproto_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_applewmproto_OBJECTS_RELEASE )
set(xorg-proto_xorg-proto_applewmproto_COMPILE_DEFINITIONS_RELEASE )
set(xorg-proto_xorg-proto_applewmproto_COMPILE_OPTIONS_C_RELEASE "")
set(xorg-proto_xorg-proto_applewmproto_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg-proto_xorg-proto_applewmproto_LIBS_RELEASE )
set(xorg-proto_xorg-proto_applewmproto_SYSTEM_LIBS_RELEASE )
set(xorg-proto_xorg-proto_applewmproto_FRAMEWORK_DIRS_RELEASE )
set(xorg-proto_xorg-proto_applewmproto_FRAMEWORKS_RELEASE )
set(xorg-proto_xorg-proto_applewmproto_DEPENDENCIES_RELEASE xorg-macros::xorg-macros)
set(xorg-proto_xorg-proto_applewmproto_SHARED_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_applewmproto_EXE_LINK_FLAGS_RELEASE )
set(xorg-proto_xorg-proto_applewmproto_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg-proto_xorg-proto_applewmproto_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-proto_xorg-proto_applewmproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-proto_xorg-proto_applewmproto_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-proto_xorg-proto_applewmproto_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg-proto_xorg-proto_applewmproto_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-proto_xorg-proto_applewmproto_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-proto_xorg-proto_applewmproto_COMPILE_OPTIONS_C_RELEASE}>")