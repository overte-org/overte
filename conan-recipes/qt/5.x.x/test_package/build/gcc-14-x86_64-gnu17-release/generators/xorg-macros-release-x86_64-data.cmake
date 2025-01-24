########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(xorg-macros_COMPONENT_NAMES "")
if(DEFINED xorg-macros_FIND_DEPENDENCY_NAMES)
  list(APPEND xorg-macros_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES xorg-macros_FIND_DEPENDENCY_NAMES)
else()
  set(xorg-macros_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(xorg-macros_PACKAGE_FOLDER_RELEASE "/home/juliangro/.conan2/p/xorg-a7814e7f591fa/p")
set(xorg-macros_BUILD_MODULES_PATHS_RELEASE )


set(xorg-macros_INCLUDE_DIRS_RELEASE )
set(xorg-macros_RES_DIRS_RELEASE )
set(xorg-macros_DEFINITIONS_RELEASE )
set(xorg-macros_SHARED_LINK_FLAGS_RELEASE )
set(xorg-macros_EXE_LINK_FLAGS_RELEASE )
set(xorg-macros_OBJECTS_RELEASE )
set(xorg-macros_COMPILE_DEFINITIONS_RELEASE )
set(xorg-macros_COMPILE_OPTIONS_C_RELEASE )
set(xorg-macros_COMPILE_OPTIONS_CXX_RELEASE )
set(xorg-macros_LIB_DIRS_RELEASE )
set(xorg-macros_BIN_DIRS_RELEASE )
set(xorg-macros_LIBRARY_TYPE_RELEASE UNKNOWN)
set(xorg-macros_IS_HOST_WINDOWS_RELEASE 0)
set(xorg-macros_LIBS_RELEASE )
set(xorg-macros_SYSTEM_LIBS_RELEASE )
set(xorg-macros_FRAMEWORK_DIRS_RELEASE )
set(xorg-macros_FRAMEWORKS_RELEASE )
set(xorg-macros_BUILD_DIRS_RELEASE )
set(xorg-macros_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(xorg-macros_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${xorg-macros_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${xorg-macros_COMPILE_OPTIONS_C_RELEASE}>")
set(xorg-macros_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${xorg-macros_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${xorg-macros_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${xorg-macros_EXE_LINK_FLAGS_RELEASE}>")


set(xorg-macros_COMPONENTS_RELEASE )