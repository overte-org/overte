########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

list(APPEND xorg_COMPONENT_NAMES xorg::x11 xorg::x11-xcb xorg::fontenc xorg::ice xorg::xau xorg::xaw7 xorg::xcomposite xorg::xcursor xorg::xdamage xorg::xdmcp xorg::xext xorg::xfixes xorg::xi xorg::xinerama xorg::xkbfile xorg::xmu xorg::xmuu xorg::xpm xorg::xrandr xorg::xrender xorg::xres xorg::xscrnsaver xorg::xt xorg::xtst xorg::xv xorg::xxf86vm xorg::xcb-xkb xorg::xcb-icccm xorg::xcb-image xorg::xcb-keysyms xorg::xcb-randr xorg::xcb-render xorg::xcb-renderutil xorg::xcb-shape xorg::xcb-shm xorg::xcb-sync xorg::xcb-xfixes xorg::xcb-xinerama xorg::xcb xorg::xcb-atom xorg::xcb-aux xorg::xcb-event xorg::xcb-util xorg::xcb-dri3 xorg::xcb-cursor xorg::xcb-dri2 xorg::xcb-glx xorg::xcb-present xorg::xcb-composite xorg::xcb-ewmh xorg::xcb-res xorg::uuid xorg::sm)
list(REMOVE_DUPLICATES xorg_COMPONENT_NAMES)
if(DEFINED xorg_FIND_DEPENDENCY_NAMES)
  list(APPEND xorg_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES xorg_FIND_DEPENDENCY_NAMES)
else()
  set(xorg_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(xorg_PACKAGE_FOLDER_RELEASE "/home/juliangro/.conan2/p/xorg454a2dd81d5aa/p")
set(xorg_BUILD_MODULES_PATHS_RELEASE )


set(xorg_INCLUDE_DIRS_RELEASE )
set(xorg_RES_DIRS_RELEASE )
set(xorg_DEFINITIONS_RELEASE )
set(xorg_SHARED_LINK_FLAGS_RELEASE )
set(xorg_EXE_LINK_FLAGS_RELEASE )
set(xorg_OBJECTS_RELEASE )
set(xorg_COMPILE_DEFINITIONS_RELEASE )
set(xorg_COMPILE_OPTIONS_C_RELEASE )
set(xorg_COMPILE_OPTIONS_CXX_RELEASE )
set(xorg_LIB_DIRS_RELEASE )
set(xorg_BIN_DIRS_RELEASE )
set(xorg_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_LIBS_RELEASE )
set(xorg_SYSTEM_LIBS_RELEASE SM uuid xcb-res xcb-ewmh xcb xcb-composite xcb-present xcb-glx xcb-dri2 xcb-cursor xcb-dri3 xcb-util xcb-xinerama xcb-xfixes xcb-sync xcb-shm xcb-shape xcb-render-util xcb-render xcb-randr xcb-keysyms xcb-image xcb-icccm xcb-xkb Xxf86vm Xv Xtst Xt X11 Xss XRes Xrender Xrandr Xpm Xmuu Xmu xkbfile Xinerama Xi Xfixes Xext Xdmcp Xdamage Xcursor Xcomposite Xaw7 Xau ICE fontenc X11-xcb)
set(xorg_FRAMEWORK_DIRS_RELEASE )
set(xorg_FRAMEWORKS_RELEASE )
set(xorg_BUILD_DIRS_RELEASE )
set(xorg_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(xorg_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_COMPILE_OPTIONS_C_RELEASE}>")
set(xorg_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_EXE_LINK_FLAGS_RELEASE}>")


set(xorg_COMPONENTS_RELEASE xorg::x11 xorg::x11-xcb xorg::fontenc xorg::ice xorg::xau xorg::xaw7 xorg::xcomposite xorg::xcursor xorg::xdamage xorg::xdmcp xorg::xext xorg::xfixes xorg::xi xorg::xinerama xorg::xkbfile xorg::xmu xorg::xmuu xorg::xpm xorg::xrandr xorg::xrender xorg::xres xorg::xscrnsaver xorg::xt xorg::xtst xorg::xv xorg::xxf86vm xorg::xcb-xkb xorg::xcb-icccm xorg::xcb-image xorg::xcb-keysyms xorg::xcb-randr xorg::xcb-render xorg::xcb-renderutil xorg::xcb-shape xorg::xcb-shm xorg::xcb-sync xorg::xcb-xfixes xorg::xcb-xinerama xorg::xcb xorg::xcb-atom xorg::xcb-aux xorg::xcb-event xorg::xcb-util xorg::xcb-dri3 xorg::xcb-cursor xorg::xcb-dri2 xorg::xcb-glx xorg::xcb-present xorg::xcb-composite xorg::xcb-ewmh xorg::xcb-res xorg::uuid xorg::sm)
########### COMPONENT xorg::sm VARIABLES ############################################

set(xorg_xorg_sm_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_sm_LIB_DIRS_RELEASE )
set(xorg_xorg_sm_BIN_DIRS_RELEASE )
set(xorg_xorg_sm_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_sm_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_sm_RES_DIRS_RELEASE )
set(xorg_xorg_sm_DEFINITIONS_RELEASE )
set(xorg_xorg_sm_OBJECTS_RELEASE )
set(xorg_xorg_sm_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_sm_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_sm_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_sm_LIBS_RELEASE )
set(xorg_xorg_sm_SYSTEM_LIBS_RELEASE SM)
set(xorg_xorg_sm_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_sm_FRAMEWORKS_RELEASE )
set(xorg_xorg_sm_DEPENDENCIES_RELEASE xorg::uuid)
set(xorg_xorg_sm_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_sm_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_sm_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_sm_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_sm_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_sm_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_sm_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_sm_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_sm_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_sm_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::uuid VARIABLES ############################################

set(xorg_xorg_uuid_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_uuid_LIB_DIRS_RELEASE )
set(xorg_xorg_uuid_BIN_DIRS_RELEASE )
set(xorg_xorg_uuid_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_uuid_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_uuid_RES_DIRS_RELEASE )
set(xorg_xorg_uuid_DEFINITIONS_RELEASE )
set(xorg_xorg_uuid_OBJECTS_RELEASE )
set(xorg_xorg_uuid_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_uuid_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_uuid_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_uuid_LIBS_RELEASE )
set(xorg_xorg_uuid_SYSTEM_LIBS_RELEASE uuid)
set(xorg_xorg_uuid_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_uuid_FRAMEWORKS_RELEASE )
set(xorg_xorg_uuid_DEPENDENCIES_RELEASE )
set(xorg_xorg_uuid_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_uuid_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_uuid_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_uuid_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_uuid_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_uuid_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_uuid_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_uuid_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_uuid_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_uuid_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xcb-res VARIABLES ############################################

set(xorg_xorg_xcb-res_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xcb-res_LIB_DIRS_RELEASE )
set(xorg_xorg_xcb-res_BIN_DIRS_RELEASE )
set(xorg_xorg_xcb-res_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xcb-res_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xcb-res_RES_DIRS_RELEASE )
set(xorg_xorg_xcb-res_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-res_OBJECTS_RELEASE )
set(xorg_xorg_xcb-res_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-res_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xcb-res_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xcb-res_LIBS_RELEASE )
set(xorg_xorg_xcb-res_SYSTEM_LIBS_RELEASE xcb-res)
set(xorg_xorg_xcb-res_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xcb-res_FRAMEWORKS_RELEASE )
set(xorg_xorg_xcb-res_DEPENDENCIES_RELEASE )
set(xorg_xorg_xcb-res_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-res_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-res_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xcb-res_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xcb-res_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xcb-res_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xcb-res_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xcb-res_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xcb-res_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xcb-res_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xcb-ewmh VARIABLES ############################################

set(xorg_xorg_xcb-ewmh_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xcb-ewmh_LIB_DIRS_RELEASE )
set(xorg_xorg_xcb-ewmh_BIN_DIRS_RELEASE )
set(xorg_xorg_xcb-ewmh_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xcb-ewmh_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xcb-ewmh_RES_DIRS_RELEASE )
set(xorg_xorg_xcb-ewmh_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-ewmh_OBJECTS_RELEASE )
set(xorg_xorg_xcb-ewmh_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-ewmh_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xcb-ewmh_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xcb-ewmh_LIBS_RELEASE )
set(xorg_xorg_xcb-ewmh_SYSTEM_LIBS_RELEASE xcb-ewmh xcb)
set(xorg_xorg_xcb-ewmh_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xcb-ewmh_FRAMEWORKS_RELEASE )
set(xorg_xorg_xcb-ewmh_DEPENDENCIES_RELEASE )
set(xorg_xorg_xcb-ewmh_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-ewmh_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-ewmh_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xcb-ewmh_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xcb-ewmh_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xcb-ewmh_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xcb-ewmh_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xcb-ewmh_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xcb-ewmh_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xcb-ewmh_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xcb-composite VARIABLES ############################################

set(xorg_xorg_xcb-composite_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xcb-composite_LIB_DIRS_RELEASE )
set(xorg_xorg_xcb-composite_BIN_DIRS_RELEASE )
set(xorg_xorg_xcb-composite_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xcb-composite_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xcb-composite_RES_DIRS_RELEASE )
set(xorg_xorg_xcb-composite_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-composite_OBJECTS_RELEASE )
set(xorg_xorg_xcb-composite_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-composite_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xcb-composite_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xcb-composite_LIBS_RELEASE )
set(xorg_xorg_xcb-composite_SYSTEM_LIBS_RELEASE xcb-composite)
set(xorg_xorg_xcb-composite_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xcb-composite_FRAMEWORKS_RELEASE )
set(xorg_xorg_xcb-composite_DEPENDENCIES_RELEASE )
set(xorg_xorg_xcb-composite_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-composite_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-composite_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xcb-composite_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xcb-composite_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xcb-composite_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xcb-composite_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xcb-composite_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xcb-composite_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xcb-composite_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xcb-present VARIABLES ############################################

set(xorg_xorg_xcb-present_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xcb-present_LIB_DIRS_RELEASE )
set(xorg_xorg_xcb-present_BIN_DIRS_RELEASE )
set(xorg_xorg_xcb-present_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xcb-present_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xcb-present_RES_DIRS_RELEASE )
set(xorg_xorg_xcb-present_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-present_OBJECTS_RELEASE )
set(xorg_xorg_xcb-present_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-present_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xcb-present_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xcb-present_LIBS_RELEASE )
set(xorg_xorg_xcb-present_SYSTEM_LIBS_RELEASE xcb-present)
set(xorg_xorg_xcb-present_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xcb-present_FRAMEWORKS_RELEASE )
set(xorg_xorg_xcb-present_DEPENDENCIES_RELEASE )
set(xorg_xorg_xcb-present_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-present_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-present_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xcb-present_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xcb-present_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xcb-present_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xcb-present_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xcb-present_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xcb-present_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xcb-present_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xcb-glx VARIABLES ############################################

set(xorg_xorg_xcb-glx_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xcb-glx_LIB_DIRS_RELEASE )
set(xorg_xorg_xcb-glx_BIN_DIRS_RELEASE )
set(xorg_xorg_xcb-glx_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xcb-glx_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xcb-glx_RES_DIRS_RELEASE )
set(xorg_xorg_xcb-glx_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-glx_OBJECTS_RELEASE )
set(xorg_xorg_xcb-glx_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-glx_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xcb-glx_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xcb-glx_LIBS_RELEASE )
set(xorg_xorg_xcb-glx_SYSTEM_LIBS_RELEASE xcb-glx)
set(xorg_xorg_xcb-glx_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xcb-glx_FRAMEWORKS_RELEASE )
set(xorg_xorg_xcb-glx_DEPENDENCIES_RELEASE )
set(xorg_xorg_xcb-glx_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-glx_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-glx_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xcb-glx_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xcb-glx_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xcb-glx_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xcb-glx_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xcb-glx_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xcb-glx_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xcb-glx_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xcb-dri2 VARIABLES ############################################

set(xorg_xorg_xcb-dri2_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xcb-dri2_LIB_DIRS_RELEASE )
set(xorg_xorg_xcb-dri2_BIN_DIRS_RELEASE )
set(xorg_xorg_xcb-dri2_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xcb-dri2_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xcb-dri2_RES_DIRS_RELEASE )
set(xorg_xorg_xcb-dri2_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-dri2_OBJECTS_RELEASE )
set(xorg_xorg_xcb-dri2_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-dri2_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xcb-dri2_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xcb-dri2_LIBS_RELEASE )
set(xorg_xorg_xcb-dri2_SYSTEM_LIBS_RELEASE xcb-dri2)
set(xorg_xorg_xcb-dri2_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xcb-dri2_FRAMEWORKS_RELEASE )
set(xorg_xorg_xcb-dri2_DEPENDENCIES_RELEASE )
set(xorg_xorg_xcb-dri2_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-dri2_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-dri2_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xcb-dri2_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xcb-dri2_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xcb-dri2_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xcb-dri2_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xcb-dri2_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xcb-dri2_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xcb-dri2_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xcb-cursor VARIABLES ############################################

set(xorg_xorg_xcb-cursor_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xcb-cursor_LIB_DIRS_RELEASE )
set(xorg_xorg_xcb-cursor_BIN_DIRS_RELEASE )
set(xorg_xorg_xcb-cursor_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xcb-cursor_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xcb-cursor_RES_DIRS_RELEASE )
set(xorg_xorg_xcb-cursor_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-cursor_OBJECTS_RELEASE )
set(xorg_xorg_xcb-cursor_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-cursor_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xcb-cursor_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xcb-cursor_LIBS_RELEASE )
set(xorg_xorg_xcb-cursor_SYSTEM_LIBS_RELEASE xcb-cursor xcb)
set(xorg_xorg_xcb-cursor_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xcb-cursor_FRAMEWORKS_RELEASE )
set(xorg_xorg_xcb-cursor_DEPENDENCIES_RELEASE )
set(xorg_xorg_xcb-cursor_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-cursor_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-cursor_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xcb-cursor_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xcb-cursor_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xcb-cursor_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xcb-cursor_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xcb-cursor_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xcb-cursor_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xcb-cursor_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xcb-dri3 VARIABLES ############################################

set(xorg_xorg_xcb-dri3_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xcb-dri3_LIB_DIRS_RELEASE )
set(xorg_xorg_xcb-dri3_BIN_DIRS_RELEASE )
set(xorg_xorg_xcb-dri3_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xcb-dri3_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xcb-dri3_RES_DIRS_RELEASE )
set(xorg_xorg_xcb-dri3_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-dri3_OBJECTS_RELEASE )
set(xorg_xorg_xcb-dri3_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-dri3_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xcb-dri3_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xcb-dri3_LIBS_RELEASE )
set(xorg_xorg_xcb-dri3_SYSTEM_LIBS_RELEASE xcb-dri3)
set(xorg_xorg_xcb-dri3_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xcb-dri3_FRAMEWORKS_RELEASE )
set(xorg_xorg_xcb-dri3_DEPENDENCIES_RELEASE )
set(xorg_xorg_xcb-dri3_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-dri3_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-dri3_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xcb-dri3_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xcb-dri3_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xcb-dri3_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xcb-dri3_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xcb-dri3_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xcb-dri3_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xcb-dri3_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xcb-util VARIABLES ############################################

set(xorg_xorg_xcb-util_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xcb-util_LIB_DIRS_RELEASE )
set(xorg_xorg_xcb-util_BIN_DIRS_RELEASE )
set(xorg_xorg_xcb-util_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xcb-util_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xcb-util_RES_DIRS_RELEASE )
set(xorg_xorg_xcb-util_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-util_OBJECTS_RELEASE )
set(xorg_xorg_xcb-util_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-util_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xcb-util_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xcb-util_LIBS_RELEASE )
set(xorg_xorg_xcb-util_SYSTEM_LIBS_RELEASE xcb-util xcb)
set(xorg_xorg_xcb-util_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xcb-util_FRAMEWORKS_RELEASE )
set(xorg_xorg_xcb-util_DEPENDENCIES_RELEASE )
set(xorg_xorg_xcb-util_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-util_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-util_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xcb-util_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xcb-util_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xcb-util_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xcb-util_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xcb-util_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xcb-util_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xcb-util_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xcb-event VARIABLES ############################################

set(xorg_xorg_xcb-event_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xcb-event_LIB_DIRS_RELEASE )
set(xorg_xorg_xcb-event_BIN_DIRS_RELEASE )
set(xorg_xorg_xcb-event_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xcb-event_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xcb-event_RES_DIRS_RELEASE )
set(xorg_xorg_xcb-event_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-event_OBJECTS_RELEASE )
set(xorg_xorg_xcb-event_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-event_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xcb-event_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xcb-event_LIBS_RELEASE )
set(xorg_xorg_xcb-event_SYSTEM_LIBS_RELEASE xcb-util xcb)
set(xorg_xorg_xcb-event_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xcb-event_FRAMEWORKS_RELEASE )
set(xorg_xorg_xcb-event_DEPENDENCIES_RELEASE )
set(xorg_xorg_xcb-event_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-event_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-event_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xcb-event_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xcb-event_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xcb-event_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xcb-event_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xcb-event_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xcb-event_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xcb-event_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xcb-aux VARIABLES ############################################

set(xorg_xorg_xcb-aux_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xcb-aux_LIB_DIRS_RELEASE )
set(xorg_xorg_xcb-aux_BIN_DIRS_RELEASE )
set(xorg_xorg_xcb-aux_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xcb-aux_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xcb-aux_RES_DIRS_RELEASE )
set(xorg_xorg_xcb-aux_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-aux_OBJECTS_RELEASE )
set(xorg_xorg_xcb-aux_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-aux_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xcb-aux_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xcb-aux_LIBS_RELEASE )
set(xorg_xorg_xcb-aux_SYSTEM_LIBS_RELEASE xcb-util xcb)
set(xorg_xorg_xcb-aux_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xcb-aux_FRAMEWORKS_RELEASE )
set(xorg_xorg_xcb-aux_DEPENDENCIES_RELEASE )
set(xorg_xorg_xcb-aux_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-aux_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-aux_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xcb-aux_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xcb-aux_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xcb-aux_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xcb-aux_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xcb-aux_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xcb-aux_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xcb-aux_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xcb-atom VARIABLES ############################################

set(xorg_xorg_xcb-atom_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xcb-atom_LIB_DIRS_RELEASE )
set(xorg_xorg_xcb-atom_BIN_DIRS_RELEASE )
set(xorg_xorg_xcb-atom_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xcb-atom_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xcb-atom_RES_DIRS_RELEASE )
set(xorg_xorg_xcb-atom_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-atom_OBJECTS_RELEASE )
set(xorg_xorg_xcb-atom_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-atom_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xcb-atom_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xcb-atom_LIBS_RELEASE )
set(xorg_xorg_xcb-atom_SYSTEM_LIBS_RELEASE xcb-util xcb)
set(xorg_xorg_xcb-atom_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xcb-atom_FRAMEWORKS_RELEASE )
set(xorg_xorg_xcb-atom_DEPENDENCIES_RELEASE )
set(xorg_xorg_xcb-atom_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-atom_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-atom_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xcb-atom_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xcb-atom_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xcb-atom_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xcb-atom_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xcb-atom_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xcb-atom_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xcb-atom_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xcb VARIABLES ############################################

set(xorg_xorg_xcb_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xcb_LIB_DIRS_RELEASE )
set(xorg_xorg_xcb_BIN_DIRS_RELEASE )
set(xorg_xorg_xcb_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xcb_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xcb_RES_DIRS_RELEASE )
set(xorg_xorg_xcb_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb_OBJECTS_RELEASE )
set(xorg_xorg_xcb_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xcb_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xcb_LIBS_RELEASE )
set(xorg_xorg_xcb_SYSTEM_LIBS_RELEASE xcb)
set(xorg_xorg_xcb_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xcb_FRAMEWORKS_RELEASE )
set(xorg_xorg_xcb_DEPENDENCIES_RELEASE )
set(xorg_xorg_xcb_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xcb_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xcb_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xcb_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xcb_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xcb_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xcb_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xcb_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xcb-xinerama VARIABLES ############################################

set(xorg_xorg_xcb-xinerama_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xcb-xinerama_LIB_DIRS_RELEASE )
set(xorg_xorg_xcb-xinerama_BIN_DIRS_RELEASE )
set(xorg_xorg_xcb-xinerama_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xcb-xinerama_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xcb-xinerama_RES_DIRS_RELEASE )
set(xorg_xorg_xcb-xinerama_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-xinerama_OBJECTS_RELEASE )
set(xorg_xorg_xcb-xinerama_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-xinerama_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xcb-xinerama_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xcb-xinerama_LIBS_RELEASE )
set(xorg_xorg_xcb-xinerama_SYSTEM_LIBS_RELEASE xcb-xinerama)
set(xorg_xorg_xcb-xinerama_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xcb-xinerama_FRAMEWORKS_RELEASE )
set(xorg_xorg_xcb-xinerama_DEPENDENCIES_RELEASE )
set(xorg_xorg_xcb-xinerama_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-xinerama_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-xinerama_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xcb-xinerama_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xcb-xinerama_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xcb-xinerama_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xcb-xinerama_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xcb-xinerama_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xcb-xinerama_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xcb-xinerama_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xcb-xfixes VARIABLES ############################################

set(xorg_xorg_xcb-xfixes_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xcb-xfixes_LIB_DIRS_RELEASE )
set(xorg_xorg_xcb-xfixes_BIN_DIRS_RELEASE )
set(xorg_xorg_xcb-xfixes_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xcb-xfixes_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xcb-xfixes_RES_DIRS_RELEASE )
set(xorg_xorg_xcb-xfixes_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-xfixes_OBJECTS_RELEASE )
set(xorg_xorg_xcb-xfixes_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-xfixes_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xcb-xfixes_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xcb-xfixes_LIBS_RELEASE )
set(xorg_xorg_xcb-xfixes_SYSTEM_LIBS_RELEASE xcb-xfixes)
set(xorg_xorg_xcb-xfixes_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xcb-xfixes_FRAMEWORKS_RELEASE )
set(xorg_xorg_xcb-xfixes_DEPENDENCIES_RELEASE )
set(xorg_xorg_xcb-xfixes_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-xfixes_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-xfixes_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xcb-xfixes_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xcb-xfixes_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xcb-xfixes_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xcb-xfixes_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xcb-xfixes_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xcb-xfixes_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xcb-xfixes_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xcb-sync VARIABLES ############################################

set(xorg_xorg_xcb-sync_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xcb-sync_LIB_DIRS_RELEASE )
set(xorg_xorg_xcb-sync_BIN_DIRS_RELEASE )
set(xorg_xorg_xcb-sync_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xcb-sync_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xcb-sync_RES_DIRS_RELEASE )
set(xorg_xorg_xcb-sync_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-sync_OBJECTS_RELEASE )
set(xorg_xorg_xcb-sync_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-sync_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xcb-sync_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xcb-sync_LIBS_RELEASE )
set(xorg_xorg_xcb-sync_SYSTEM_LIBS_RELEASE xcb-sync)
set(xorg_xorg_xcb-sync_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xcb-sync_FRAMEWORKS_RELEASE )
set(xorg_xorg_xcb-sync_DEPENDENCIES_RELEASE )
set(xorg_xorg_xcb-sync_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-sync_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-sync_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xcb-sync_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xcb-sync_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xcb-sync_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xcb-sync_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xcb-sync_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xcb-sync_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xcb-sync_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xcb-shm VARIABLES ############################################

set(xorg_xorg_xcb-shm_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xcb-shm_LIB_DIRS_RELEASE )
set(xorg_xorg_xcb-shm_BIN_DIRS_RELEASE )
set(xorg_xorg_xcb-shm_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xcb-shm_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xcb-shm_RES_DIRS_RELEASE )
set(xorg_xorg_xcb-shm_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-shm_OBJECTS_RELEASE )
set(xorg_xorg_xcb-shm_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-shm_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xcb-shm_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xcb-shm_LIBS_RELEASE )
set(xorg_xorg_xcb-shm_SYSTEM_LIBS_RELEASE xcb-shm)
set(xorg_xorg_xcb-shm_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xcb-shm_FRAMEWORKS_RELEASE )
set(xorg_xorg_xcb-shm_DEPENDENCIES_RELEASE )
set(xorg_xorg_xcb-shm_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-shm_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-shm_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xcb-shm_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xcb-shm_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xcb-shm_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xcb-shm_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xcb-shm_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xcb-shm_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xcb-shm_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xcb-shape VARIABLES ############################################

set(xorg_xorg_xcb-shape_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xcb-shape_LIB_DIRS_RELEASE )
set(xorg_xorg_xcb-shape_BIN_DIRS_RELEASE )
set(xorg_xorg_xcb-shape_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xcb-shape_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xcb-shape_RES_DIRS_RELEASE )
set(xorg_xorg_xcb-shape_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-shape_OBJECTS_RELEASE )
set(xorg_xorg_xcb-shape_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-shape_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xcb-shape_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xcb-shape_LIBS_RELEASE )
set(xorg_xorg_xcb-shape_SYSTEM_LIBS_RELEASE xcb-shape)
set(xorg_xorg_xcb-shape_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xcb-shape_FRAMEWORKS_RELEASE )
set(xorg_xorg_xcb-shape_DEPENDENCIES_RELEASE )
set(xorg_xorg_xcb-shape_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-shape_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-shape_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xcb-shape_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xcb-shape_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xcb-shape_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xcb-shape_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xcb-shape_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xcb-shape_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xcb-shape_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xcb-renderutil VARIABLES ############################################

set(xorg_xorg_xcb-renderutil_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xcb-renderutil_LIB_DIRS_RELEASE )
set(xorg_xorg_xcb-renderutil_BIN_DIRS_RELEASE )
set(xorg_xorg_xcb-renderutil_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xcb-renderutil_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xcb-renderutil_RES_DIRS_RELEASE )
set(xorg_xorg_xcb-renderutil_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-renderutil_OBJECTS_RELEASE )
set(xorg_xorg_xcb-renderutil_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-renderutil_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xcb-renderutil_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xcb-renderutil_LIBS_RELEASE )
set(xorg_xorg_xcb-renderutil_SYSTEM_LIBS_RELEASE xcb-render-util xcb xcb-render)
set(xorg_xorg_xcb-renderutil_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xcb-renderutil_FRAMEWORKS_RELEASE )
set(xorg_xorg_xcb-renderutil_DEPENDENCIES_RELEASE )
set(xorg_xorg_xcb-renderutil_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-renderutil_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-renderutil_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xcb-renderutil_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xcb-renderutil_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xcb-renderutil_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xcb-renderutil_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xcb-renderutil_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xcb-renderutil_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xcb-renderutil_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xcb-render VARIABLES ############################################

set(xorg_xorg_xcb-render_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xcb-render_LIB_DIRS_RELEASE )
set(xorg_xorg_xcb-render_BIN_DIRS_RELEASE )
set(xorg_xorg_xcb-render_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xcb-render_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xcb-render_RES_DIRS_RELEASE )
set(xorg_xorg_xcb-render_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-render_OBJECTS_RELEASE )
set(xorg_xorg_xcb-render_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-render_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xcb-render_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xcb-render_LIBS_RELEASE )
set(xorg_xorg_xcb-render_SYSTEM_LIBS_RELEASE xcb-render)
set(xorg_xorg_xcb-render_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xcb-render_FRAMEWORKS_RELEASE )
set(xorg_xorg_xcb-render_DEPENDENCIES_RELEASE )
set(xorg_xorg_xcb-render_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-render_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-render_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xcb-render_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xcb-render_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xcb-render_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xcb-render_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xcb-render_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xcb-render_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xcb-render_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xcb-randr VARIABLES ############################################

set(xorg_xorg_xcb-randr_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xcb-randr_LIB_DIRS_RELEASE )
set(xorg_xorg_xcb-randr_BIN_DIRS_RELEASE )
set(xorg_xorg_xcb-randr_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xcb-randr_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xcb-randr_RES_DIRS_RELEASE )
set(xorg_xorg_xcb-randr_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-randr_OBJECTS_RELEASE )
set(xorg_xorg_xcb-randr_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-randr_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xcb-randr_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xcb-randr_LIBS_RELEASE )
set(xorg_xorg_xcb-randr_SYSTEM_LIBS_RELEASE xcb-randr)
set(xorg_xorg_xcb-randr_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xcb-randr_FRAMEWORKS_RELEASE )
set(xorg_xorg_xcb-randr_DEPENDENCIES_RELEASE )
set(xorg_xorg_xcb-randr_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-randr_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-randr_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xcb-randr_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xcb-randr_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xcb-randr_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xcb-randr_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xcb-randr_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xcb-randr_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xcb-randr_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xcb-keysyms VARIABLES ############################################

set(xorg_xorg_xcb-keysyms_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xcb-keysyms_LIB_DIRS_RELEASE )
set(xorg_xorg_xcb-keysyms_BIN_DIRS_RELEASE )
set(xorg_xorg_xcb-keysyms_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xcb-keysyms_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xcb-keysyms_RES_DIRS_RELEASE )
set(xorg_xorg_xcb-keysyms_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-keysyms_OBJECTS_RELEASE )
set(xorg_xorg_xcb-keysyms_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-keysyms_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xcb-keysyms_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xcb-keysyms_LIBS_RELEASE )
set(xorg_xorg_xcb-keysyms_SYSTEM_LIBS_RELEASE xcb-keysyms xcb)
set(xorg_xorg_xcb-keysyms_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xcb-keysyms_FRAMEWORKS_RELEASE )
set(xorg_xorg_xcb-keysyms_DEPENDENCIES_RELEASE )
set(xorg_xorg_xcb-keysyms_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-keysyms_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-keysyms_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xcb-keysyms_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xcb-keysyms_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xcb-keysyms_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xcb-keysyms_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xcb-keysyms_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xcb-keysyms_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xcb-keysyms_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xcb-image VARIABLES ############################################

set(xorg_xorg_xcb-image_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xcb-image_LIB_DIRS_RELEASE )
set(xorg_xorg_xcb-image_BIN_DIRS_RELEASE )
set(xorg_xorg_xcb-image_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xcb-image_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xcb-image_RES_DIRS_RELEASE )
set(xorg_xorg_xcb-image_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-image_OBJECTS_RELEASE )
set(xorg_xorg_xcb-image_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-image_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xcb-image_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xcb-image_LIBS_RELEASE )
set(xorg_xorg_xcb-image_SYSTEM_LIBS_RELEASE xcb-image xcb xcb-shm)
set(xorg_xorg_xcb-image_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xcb-image_FRAMEWORKS_RELEASE )
set(xorg_xorg_xcb-image_DEPENDENCIES_RELEASE )
set(xorg_xorg_xcb-image_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-image_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-image_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xcb-image_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xcb-image_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xcb-image_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xcb-image_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xcb-image_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xcb-image_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xcb-image_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xcb-icccm VARIABLES ############################################

set(xorg_xorg_xcb-icccm_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xcb-icccm_LIB_DIRS_RELEASE )
set(xorg_xorg_xcb-icccm_BIN_DIRS_RELEASE )
set(xorg_xorg_xcb-icccm_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xcb-icccm_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xcb-icccm_RES_DIRS_RELEASE )
set(xorg_xorg_xcb-icccm_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-icccm_OBJECTS_RELEASE )
set(xorg_xorg_xcb-icccm_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-icccm_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xcb-icccm_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xcb-icccm_LIBS_RELEASE )
set(xorg_xorg_xcb-icccm_SYSTEM_LIBS_RELEASE xcb-icccm xcb)
set(xorg_xorg_xcb-icccm_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xcb-icccm_FRAMEWORKS_RELEASE )
set(xorg_xorg_xcb-icccm_DEPENDENCIES_RELEASE )
set(xorg_xorg_xcb-icccm_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-icccm_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-icccm_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xcb-icccm_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xcb-icccm_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xcb-icccm_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xcb-icccm_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xcb-icccm_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xcb-icccm_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xcb-icccm_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xcb-xkb VARIABLES ############################################

set(xorg_xorg_xcb-xkb_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xcb-xkb_LIB_DIRS_RELEASE )
set(xorg_xorg_xcb-xkb_BIN_DIRS_RELEASE )
set(xorg_xorg_xcb-xkb_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xcb-xkb_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xcb-xkb_RES_DIRS_RELEASE )
set(xorg_xorg_xcb-xkb_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-xkb_OBJECTS_RELEASE )
set(xorg_xorg_xcb-xkb_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xcb-xkb_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xcb-xkb_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xcb-xkb_LIBS_RELEASE )
set(xorg_xorg_xcb-xkb_SYSTEM_LIBS_RELEASE xcb-xkb)
set(xorg_xorg_xcb-xkb_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xcb-xkb_FRAMEWORKS_RELEASE )
set(xorg_xorg_xcb-xkb_DEPENDENCIES_RELEASE )
set(xorg_xorg_xcb-xkb_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-xkb_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcb-xkb_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xcb-xkb_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xcb-xkb_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xcb-xkb_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xcb-xkb_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xcb-xkb_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xcb-xkb_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xcb-xkb_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xxf86vm VARIABLES ############################################

set(xorg_xorg_xxf86vm_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xxf86vm_LIB_DIRS_RELEASE )
set(xorg_xorg_xxf86vm_BIN_DIRS_RELEASE )
set(xorg_xorg_xxf86vm_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xxf86vm_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xxf86vm_RES_DIRS_RELEASE )
set(xorg_xorg_xxf86vm_DEFINITIONS_RELEASE )
set(xorg_xorg_xxf86vm_OBJECTS_RELEASE )
set(xorg_xorg_xxf86vm_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xxf86vm_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xxf86vm_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xxf86vm_LIBS_RELEASE )
set(xorg_xorg_xxf86vm_SYSTEM_LIBS_RELEASE Xxf86vm)
set(xorg_xorg_xxf86vm_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xxf86vm_FRAMEWORKS_RELEASE )
set(xorg_xorg_xxf86vm_DEPENDENCIES_RELEASE )
set(xorg_xorg_xxf86vm_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xxf86vm_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xxf86vm_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xxf86vm_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xxf86vm_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xxf86vm_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xxf86vm_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xxf86vm_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xxf86vm_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xxf86vm_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xv VARIABLES ############################################

set(xorg_xorg_xv_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xv_LIB_DIRS_RELEASE )
set(xorg_xorg_xv_BIN_DIRS_RELEASE )
set(xorg_xorg_xv_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xv_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xv_RES_DIRS_RELEASE )
set(xorg_xorg_xv_DEFINITIONS_RELEASE )
set(xorg_xorg_xv_OBJECTS_RELEASE )
set(xorg_xorg_xv_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xv_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xv_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xv_LIBS_RELEASE )
set(xorg_xorg_xv_SYSTEM_LIBS_RELEASE Xv)
set(xorg_xorg_xv_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xv_FRAMEWORKS_RELEASE )
set(xorg_xorg_xv_DEPENDENCIES_RELEASE )
set(xorg_xorg_xv_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xv_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xv_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xv_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xv_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xv_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xv_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xv_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xv_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xv_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xtst VARIABLES ############################################

set(xorg_xorg_xtst_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xtst_LIB_DIRS_RELEASE )
set(xorg_xorg_xtst_BIN_DIRS_RELEASE )
set(xorg_xorg_xtst_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xtst_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xtst_RES_DIRS_RELEASE )
set(xorg_xorg_xtst_DEFINITIONS_RELEASE )
set(xorg_xorg_xtst_OBJECTS_RELEASE )
set(xorg_xorg_xtst_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xtst_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xtst_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xtst_LIBS_RELEASE )
set(xorg_xorg_xtst_SYSTEM_LIBS_RELEASE Xtst)
set(xorg_xorg_xtst_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xtst_FRAMEWORKS_RELEASE )
set(xorg_xorg_xtst_DEPENDENCIES_RELEASE )
set(xorg_xorg_xtst_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xtst_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xtst_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xtst_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xtst_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xtst_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xtst_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xtst_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xtst_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xtst_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xt VARIABLES ############################################

set(xorg_xorg_xt_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xt_LIB_DIRS_RELEASE )
set(xorg_xorg_xt_BIN_DIRS_RELEASE )
set(xorg_xorg_xt_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xt_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xt_RES_DIRS_RELEASE )
set(xorg_xorg_xt_DEFINITIONS_RELEASE )
set(xorg_xorg_xt_OBJECTS_RELEASE )
set(xorg_xorg_xt_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xt_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xt_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xt_LIBS_RELEASE )
set(xorg_xorg_xt_SYSTEM_LIBS_RELEASE Xt X11)
set(xorg_xorg_xt_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xt_FRAMEWORKS_RELEASE )
set(xorg_xorg_xt_DEPENDENCIES_RELEASE )
set(xorg_xorg_xt_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xt_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xt_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xt_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xt_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xt_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xt_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xt_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xt_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xt_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xscrnsaver VARIABLES ############################################

set(xorg_xorg_xscrnsaver_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xscrnsaver_LIB_DIRS_RELEASE )
set(xorg_xorg_xscrnsaver_BIN_DIRS_RELEASE )
set(xorg_xorg_xscrnsaver_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xscrnsaver_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xscrnsaver_RES_DIRS_RELEASE )
set(xorg_xorg_xscrnsaver_DEFINITIONS_RELEASE )
set(xorg_xorg_xscrnsaver_OBJECTS_RELEASE )
set(xorg_xorg_xscrnsaver_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xscrnsaver_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xscrnsaver_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xscrnsaver_LIBS_RELEASE )
set(xorg_xorg_xscrnsaver_SYSTEM_LIBS_RELEASE Xss)
set(xorg_xorg_xscrnsaver_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xscrnsaver_FRAMEWORKS_RELEASE )
set(xorg_xorg_xscrnsaver_DEPENDENCIES_RELEASE )
set(xorg_xorg_xscrnsaver_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xscrnsaver_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xscrnsaver_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xscrnsaver_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xscrnsaver_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xscrnsaver_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xscrnsaver_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xscrnsaver_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xscrnsaver_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xscrnsaver_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xres VARIABLES ############################################

set(xorg_xorg_xres_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xres_LIB_DIRS_RELEASE )
set(xorg_xorg_xres_BIN_DIRS_RELEASE )
set(xorg_xorg_xres_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xres_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xres_RES_DIRS_RELEASE )
set(xorg_xorg_xres_DEFINITIONS_RELEASE )
set(xorg_xorg_xres_OBJECTS_RELEASE )
set(xorg_xorg_xres_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xres_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xres_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xres_LIBS_RELEASE )
set(xorg_xorg_xres_SYSTEM_LIBS_RELEASE XRes)
set(xorg_xorg_xres_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xres_FRAMEWORKS_RELEASE )
set(xorg_xorg_xres_DEPENDENCIES_RELEASE )
set(xorg_xorg_xres_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xres_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xres_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xres_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xres_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xres_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xres_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xres_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xres_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xres_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xrender VARIABLES ############################################

set(xorg_xorg_xrender_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xrender_LIB_DIRS_RELEASE )
set(xorg_xorg_xrender_BIN_DIRS_RELEASE )
set(xorg_xorg_xrender_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xrender_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xrender_RES_DIRS_RELEASE )
set(xorg_xorg_xrender_DEFINITIONS_RELEASE )
set(xorg_xorg_xrender_OBJECTS_RELEASE )
set(xorg_xorg_xrender_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xrender_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xrender_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xrender_LIBS_RELEASE )
set(xorg_xorg_xrender_SYSTEM_LIBS_RELEASE Xrender X11)
set(xorg_xorg_xrender_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xrender_FRAMEWORKS_RELEASE )
set(xorg_xorg_xrender_DEPENDENCIES_RELEASE )
set(xorg_xorg_xrender_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xrender_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xrender_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xrender_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xrender_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xrender_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xrender_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xrender_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xrender_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xrender_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xrandr VARIABLES ############################################

set(xorg_xorg_xrandr_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xrandr_LIB_DIRS_RELEASE )
set(xorg_xorg_xrandr_BIN_DIRS_RELEASE )
set(xorg_xorg_xrandr_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xrandr_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xrandr_RES_DIRS_RELEASE )
set(xorg_xorg_xrandr_DEFINITIONS_RELEASE )
set(xorg_xorg_xrandr_OBJECTS_RELEASE )
set(xorg_xorg_xrandr_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xrandr_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xrandr_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xrandr_LIBS_RELEASE )
set(xorg_xorg_xrandr_SYSTEM_LIBS_RELEASE Xrandr)
set(xorg_xorg_xrandr_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xrandr_FRAMEWORKS_RELEASE )
set(xorg_xorg_xrandr_DEPENDENCIES_RELEASE )
set(xorg_xorg_xrandr_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xrandr_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xrandr_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xrandr_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xrandr_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xrandr_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xrandr_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xrandr_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xrandr_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xrandr_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xpm VARIABLES ############################################

set(xorg_xorg_xpm_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xpm_LIB_DIRS_RELEASE )
set(xorg_xorg_xpm_BIN_DIRS_RELEASE )
set(xorg_xorg_xpm_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xpm_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xpm_RES_DIRS_RELEASE )
set(xorg_xorg_xpm_DEFINITIONS_RELEASE )
set(xorg_xorg_xpm_OBJECTS_RELEASE )
set(xorg_xorg_xpm_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xpm_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xpm_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xpm_LIBS_RELEASE )
set(xorg_xorg_xpm_SYSTEM_LIBS_RELEASE Xpm X11)
set(xorg_xorg_xpm_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xpm_FRAMEWORKS_RELEASE )
set(xorg_xorg_xpm_DEPENDENCIES_RELEASE )
set(xorg_xorg_xpm_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xpm_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xpm_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xpm_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xpm_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xpm_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xpm_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xpm_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xpm_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xpm_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xmuu VARIABLES ############################################

set(xorg_xorg_xmuu_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xmuu_LIB_DIRS_RELEASE )
set(xorg_xorg_xmuu_BIN_DIRS_RELEASE )
set(xorg_xorg_xmuu_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xmuu_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xmuu_RES_DIRS_RELEASE )
set(xorg_xorg_xmuu_DEFINITIONS_RELEASE )
set(xorg_xorg_xmuu_OBJECTS_RELEASE )
set(xorg_xorg_xmuu_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xmuu_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xmuu_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xmuu_LIBS_RELEASE )
set(xorg_xorg_xmuu_SYSTEM_LIBS_RELEASE Xmuu)
set(xorg_xorg_xmuu_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xmuu_FRAMEWORKS_RELEASE )
set(xorg_xorg_xmuu_DEPENDENCIES_RELEASE )
set(xorg_xorg_xmuu_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xmuu_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xmuu_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xmuu_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xmuu_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xmuu_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xmuu_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xmuu_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xmuu_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xmuu_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xmu VARIABLES ############################################

set(xorg_xorg_xmu_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xmu_LIB_DIRS_RELEASE )
set(xorg_xorg_xmu_BIN_DIRS_RELEASE )
set(xorg_xorg_xmu_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xmu_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xmu_RES_DIRS_RELEASE )
set(xorg_xorg_xmu_DEFINITIONS_RELEASE )
set(xorg_xorg_xmu_OBJECTS_RELEASE )
set(xorg_xorg_xmu_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xmu_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xmu_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xmu_LIBS_RELEASE )
set(xorg_xorg_xmu_SYSTEM_LIBS_RELEASE Xmu Xt X11)
set(xorg_xorg_xmu_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xmu_FRAMEWORKS_RELEASE )
set(xorg_xorg_xmu_DEPENDENCIES_RELEASE )
set(xorg_xorg_xmu_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xmu_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xmu_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xmu_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xmu_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xmu_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xmu_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xmu_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xmu_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xmu_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xkbfile VARIABLES ############################################

set(xorg_xorg_xkbfile_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xkbfile_LIB_DIRS_RELEASE )
set(xorg_xorg_xkbfile_BIN_DIRS_RELEASE )
set(xorg_xorg_xkbfile_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xkbfile_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xkbfile_RES_DIRS_RELEASE )
set(xorg_xorg_xkbfile_DEFINITIONS_RELEASE )
set(xorg_xorg_xkbfile_OBJECTS_RELEASE )
set(xorg_xorg_xkbfile_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xkbfile_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xkbfile_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xkbfile_LIBS_RELEASE )
set(xorg_xorg_xkbfile_SYSTEM_LIBS_RELEASE xkbfile)
set(xorg_xorg_xkbfile_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xkbfile_FRAMEWORKS_RELEASE )
set(xorg_xorg_xkbfile_DEPENDENCIES_RELEASE )
set(xorg_xorg_xkbfile_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xkbfile_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xkbfile_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xkbfile_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xkbfile_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xkbfile_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xkbfile_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xkbfile_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xkbfile_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xkbfile_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xinerama VARIABLES ############################################

set(xorg_xorg_xinerama_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xinerama_LIB_DIRS_RELEASE )
set(xorg_xorg_xinerama_BIN_DIRS_RELEASE )
set(xorg_xorg_xinerama_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xinerama_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xinerama_RES_DIRS_RELEASE )
set(xorg_xorg_xinerama_DEFINITIONS_RELEASE )
set(xorg_xorg_xinerama_OBJECTS_RELEASE )
set(xorg_xorg_xinerama_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xinerama_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xinerama_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xinerama_LIBS_RELEASE )
set(xorg_xorg_xinerama_SYSTEM_LIBS_RELEASE Xinerama)
set(xorg_xorg_xinerama_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xinerama_FRAMEWORKS_RELEASE )
set(xorg_xorg_xinerama_DEPENDENCIES_RELEASE )
set(xorg_xorg_xinerama_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xinerama_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xinerama_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xinerama_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xinerama_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xinerama_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xinerama_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xinerama_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xinerama_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xinerama_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xi VARIABLES ############################################

set(xorg_xorg_xi_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xi_LIB_DIRS_RELEASE )
set(xorg_xorg_xi_BIN_DIRS_RELEASE )
set(xorg_xorg_xi_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xi_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xi_RES_DIRS_RELEASE )
set(xorg_xorg_xi_DEFINITIONS_RELEASE )
set(xorg_xorg_xi_OBJECTS_RELEASE )
set(xorg_xorg_xi_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xi_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xi_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xi_LIBS_RELEASE )
set(xorg_xorg_xi_SYSTEM_LIBS_RELEASE Xi)
set(xorg_xorg_xi_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xi_FRAMEWORKS_RELEASE )
set(xorg_xorg_xi_DEPENDENCIES_RELEASE )
set(xorg_xorg_xi_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xi_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xi_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xi_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xi_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xi_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xi_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xi_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xi_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xi_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xfixes VARIABLES ############################################

set(xorg_xorg_xfixes_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xfixes_LIB_DIRS_RELEASE )
set(xorg_xorg_xfixes_BIN_DIRS_RELEASE )
set(xorg_xorg_xfixes_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xfixes_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xfixes_RES_DIRS_RELEASE )
set(xorg_xorg_xfixes_DEFINITIONS_RELEASE )
set(xorg_xorg_xfixes_OBJECTS_RELEASE )
set(xorg_xorg_xfixes_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xfixes_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xfixes_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xfixes_LIBS_RELEASE )
set(xorg_xorg_xfixes_SYSTEM_LIBS_RELEASE Xfixes)
set(xorg_xorg_xfixes_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xfixes_FRAMEWORKS_RELEASE )
set(xorg_xorg_xfixes_DEPENDENCIES_RELEASE )
set(xorg_xorg_xfixes_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xfixes_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xfixes_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xfixes_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xfixes_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xfixes_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xfixes_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xfixes_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xfixes_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xfixes_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xext VARIABLES ############################################

set(xorg_xorg_xext_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xext_LIB_DIRS_RELEASE )
set(xorg_xorg_xext_BIN_DIRS_RELEASE )
set(xorg_xorg_xext_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xext_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xext_RES_DIRS_RELEASE )
set(xorg_xorg_xext_DEFINITIONS_RELEASE )
set(xorg_xorg_xext_OBJECTS_RELEASE )
set(xorg_xorg_xext_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xext_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xext_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xext_LIBS_RELEASE )
set(xorg_xorg_xext_SYSTEM_LIBS_RELEASE Xext)
set(xorg_xorg_xext_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xext_FRAMEWORKS_RELEASE )
set(xorg_xorg_xext_DEPENDENCIES_RELEASE )
set(xorg_xorg_xext_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xext_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xext_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xext_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xext_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xext_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xext_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xext_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xext_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xext_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xdmcp VARIABLES ############################################

set(xorg_xorg_xdmcp_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xdmcp_LIB_DIRS_RELEASE )
set(xorg_xorg_xdmcp_BIN_DIRS_RELEASE )
set(xorg_xorg_xdmcp_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xdmcp_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xdmcp_RES_DIRS_RELEASE )
set(xorg_xorg_xdmcp_DEFINITIONS_RELEASE )
set(xorg_xorg_xdmcp_OBJECTS_RELEASE )
set(xorg_xorg_xdmcp_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xdmcp_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xdmcp_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xdmcp_LIBS_RELEASE )
set(xorg_xorg_xdmcp_SYSTEM_LIBS_RELEASE Xdmcp)
set(xorg_xorg_xdmcp_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xdmcp_FRAMEWORKS_RELEASE )
set(xorg_xorg_xdmcp_DEPENDENCIES_RELEASE )
set(xorg_xorg_xdmcp_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xdmcp_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xdmcp_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xdmcp_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xdmcp_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xdmcp_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xdmcp_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xdmcp_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xdmcp_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xdmcp_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xdamage VARIABLES ############################################

set(xorg_xorg_xdamage_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xdamage_LIB_DIRS_RELEASE )
set(xorg_xorg_xdamage_BIN_DIRS_RELEASE )
set(xorg_xorg_xdamage_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xdamage_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xdamage_RES_DIRS_RELEASE )
set(xorg_xorg_xdamage_DEFINITIONS_RELEASE )
set(xorg_xorg_xdamage_OBJECTS_RELEASE )
set(xorg_xorg_xdamage_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xdamage_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xdamage_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xdamage_LIBS_RELEASE )
set(xorg_xorg_xdamage_SYSTEM_LIBS_RELEASE Xdamage Xfixes)
set(xorg_xorg_xdamage_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xdamage_FRAMEWORKS_RELEASE )
set(xorg_xorg_xdamage_DEPENDENCIES_RELEASE )
set(xorg_xorg_xdamage_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xdamage_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xdamage_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xdamage_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xdamage_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xdamage_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xdamage_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xdamage_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xdamage_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xdamage_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xcursor VARIABLES ############################################

set(xorg_xorg_xcursor_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xcursor_LIB_DIRS_RELEASE )
set(xorg_xorg_xcursor_BIN_DIRS_RELEASE )
set(xorg_xorg_xcursor_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xcursor_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xcursor_RES_DIRS_RELEASE )
set(xorg_xorg_xcursor_DEFINITIONS_RELEASE )
set(xorg_xorg_xcursor_OBJECTS_RELEASE )
set(xorg_xorg_xcursor_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xcursor_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xcursor_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xcursor_LIBS_RELEASE )
set(xorg_xorg_xcursor_SYSTEM_LIBS_RELEASE Xcursor)
set(xorg_xorg_xcursor_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xcursor_FRAMEWORKS_RELEASE )
set(xorg_xorg_xcursor_DEPENDENCIES_RELEASE )
set(xorg_xorg_xcursor_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcursor_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcursor_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xcursor_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xcursor_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xcursor_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xcursor_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xcursor_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xcursor_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xcursor_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xcomposite VARIABLES ############################################

set(xorg_xorg_xcomposite_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xcomposite_LIB_DIRS_RELEASE )
set(xorg_xorg_xcomposite_BIN_DIRS_RELEASE )
set(xorg_xorg_xcomposite_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xcomposite_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xcomposite_RES_DIRS_RELEASE )
set(xorg_xorg_xcomposite_DEFINITIONS_RELEASE )
set(xorg_xorg_xcomposite_OBJECTS_RELEASE )
set(xorg_xorg_xcomposite_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xcomposite_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xcomposite_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xcomposite_LIBS_RELEASE )
set(xorg_xorg_xcomposite_SYSTEM_LIBS_RELEASE Xcomposite)
set(xorg_xorg_xcomposite_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xcomposite_FRAMEWORKS_RELEASE )
set(xorg_xorg_xcomposite_DEPENDENCIES_RELEASE )
set(xorg_xorg_xcomposite_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcomposite_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xcomposite_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xcomposite_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xcomposite_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xcomposite_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xcomposite_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xcomposite_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xcomposite_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xcomposite_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xaw7 VARIABLES ############################################

set(xorg_xorg_xaw7_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xaw7_LIB_DIRS_RELEASE )
set(xorg_xorg_xaw7_BIN_DIRS_RELEASE )
set(xorg_xorg_xaw7_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xaw7_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xaw7_RES_DIRS_RELEASE )
set(xorg_xorg_xaw7_DEFINITIONS_RELEASE )
set(xorg_xorg_xaw7_OBJECTS_RELEASE )
set(xorg_xorg_xaw7_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xaw7_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xaw7_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xaw7_LIBS_RELEASE )
set(xorg_xorg_xaw7_SYSTEM_LIBS_RELEASE Xaw7 Xt X11)
set(xorg_xorg_xaw7_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xaw7_FRAMEWORKS_RELEASE )
set(xorg_xorg_xaw7_DEPENDENCIES_RELEASE )
set(xorg_xorg_xaw7_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xaw7_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xaw7_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xaw7_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xaw7_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xaw7_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xaw7_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xaw7_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xaw7_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xaw7_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::xau VARIABLES ############################################

set(xorg_xorg_xau_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_xau_LIB_DIRS_RELEASE )
set(xorg_xorg_xau_BIN_DIRS_RELEASE )
set(xorg_xorg_xau_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_xau_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_xau_RES_DIRS_RELEASE )
set(xorg_xorg_xau_DEFINITIONS_RELEASE )
set(xorg_xorg_xau_OBJECTS_RELEASE )
set(xorg_xorg_xau_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_xau_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_xau_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_xau_LIBS_RELEASE )
set(xorg_xorg_xau_SYSTEM_LIBS_RELEASE Xau)
set(xorg_xorg_xau_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_xau_FRAMEWORKS_RELEASE )
set(xorg_xorg_xau_DEPENDENCIES_RELEASE )
set(xorg_xorg_xau_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_xau_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_xau_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_xau_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_xau_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_xau_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_xau_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_xau_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_xau_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_xau_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::ice VARIABLES ############################################

set(xorg_xorg_ice_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_ice_LIB_DIRS_RELEASE )
set(xorg_xorg_ice_BIN_DIRS_RELEASE )
set(xorg_xorg_ice_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_ice_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_ice_RES_DIRS_RELEASE )
set(xorg_xorg_ice_DEFINITIONS_RELEASE )
set(xorg_xorg_ice_OBJECTS_RELEASE )
set(xorg_xorg_ice_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_ice_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_ice_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_ice_LIBS_RELEASE )
set(xorg_xorg_ice_SYSTEM_LIBS_RELEASE ICE)
set(xorg_xorg_ice_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_ice_FRAMEWORKS_RELEASE )
set(xorg_xorg_ice_DEPENDENCIES_RELEASE )
set(xorg_xorg_ice_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_ice_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_ice_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_ice_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_ice_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_ice_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_ice_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_ice_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_ice_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_ice_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::fontenc VARIABLES ############################################

set(xorg_xorg_fontenc_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_fontenc_LIB_DIRS_RELEASE )
set(xorg_xorg_fontenc_BIN_DIRS_RELEASE )
set(xorg_xorg_fontenc_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_fontenc_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_fontenc_RES_DIRS_RELEASE )
set(xorg_xorg_fontenc_DEFINITIONS_RELEASE )
set(xorg_xorg_fontenc_OBJECTS_RELEASE )
set(xorg_xorg_fontenc_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_fontenc_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_fontenc_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_fontenc_LIBS_RELEASE )
set(xorg_xorg_fontenc_SYSTEM_LIBS_RELEASE fontenc)
set(xorg_xorg_fontenc_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_fontenc_FRAMEWORKS_RELEASE )
set(xorg_xorg_fontenc_DEPENDENCIES_RELEASE )
set(xorg_xorg_fontenc_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_fontenc_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_fontenc_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_fontenc_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_fontenc_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_fontenc_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_fontenc_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_fontenc_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_fontenc_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_fontenc_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::x11-xcb VARIABLES ############################################

set(xorg_xorg_x11-xcb_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_x11-xcb_LIB_DIRS_RELEASE )
set(xorg_xorg_x11-xcb_BIN_DIRS_RELEASE )
set(xorg_xorg_x11-xcb_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_x11-xcb_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_x11-xcb_RES_DIRS_RELEASE )
set(xorg_xorg_x11-xcb_DEFINITIONS_RELEASE )
set(xorg_xorg_x11-xcb_OBJECTS_RELEASE )
set(xorg_xorg_x11-xcb_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_x11-xcb_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_x11-xcb_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_x11-xcb_LIBS_RELEASE )
set(xorg_xorg_x11-xcb_SYSTEM_LIBS_RELEASE X11-xcb X11 xcb)
set(xorg_xorg_x11-xcb_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_x11-xcb_FRAMEWORKS_RELEASE )
set(xorg_xorg_x11-xcb_DEPENDENCIES_RELEASE )
set(xorg_xorg_x11-xcb_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_x11-xcb_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_x11-xcb_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_x11-xcb_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_x11-xcb_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_x11-xcb_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_x11-xcb_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_x11-xcb_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_x11-xcb_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_x11-xcb_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT xorg::x11 VARIABLES ############################################

set(xorg_xorg_x11_INCLUDE_DIRS_RELEASE )
set(xorg_xorg_x11_LIB_DIRS_RELEASE )
set(xorg_xorg_x11_BIN_DIRS_RELEASE )
set(xorg_xorg_x11_LIBRARY_TYPE_RELEASE SHARED)
set(xorg_xorg_x11_IS_HOST_WINDOWS_RELEASE 0)
set(xorg_xorg_x11_RES_DIRS_RELEASE )
set(xorg_xorg_x11_DEFINITIONS_RELEASE )
set(xorg_xorg_x11_OBJECTS_RELEASE )
set(xorg_xorg_x11_COMPILE_DEFINITIONS_RELEASE )
set(xorg_xorg_x11_COMPILE_OPTIONS_C_RELEASE "")
set(xorg_xorg_x11_COMPILE_OPTIONS_CXX_RELEASE "")
set(xorg_xorg_x11_LIBS_RELEASE )
set(xorg_xorg_x11_SYSTEM_LIBS_RELEASE X11)
set(xorg_xorg_x11_FRAMEWORK_DIRS_RELEASE )
set(xorg_xorg_x11_FRAMEWORKS_RELEASE )
set(xorg_xorg_x11_DEPENDENCIES_RELEASE )
set(xorg_xorg_x11_SHARED_LINK_FLAGS_RELEASE )
set(xorg_xorg_x11_EXE_LINK_FLAGS_RELEASE )
set(xorg_xorg_x11_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(xorg_xorg_x11_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg_xorg_x11_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg_xorg_x11_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg_xorg_x11_EXE_LINK_FLAGS_RELEASE}>
)
set(xorg_xorg_x11_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg_xorg_x11_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg_xorg_x11_COMPILE_OPTIONS_C_RELEASE}>")