########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(libtool_COMPONENT_NAMES "")
if(DEFINED libtool_FIND_DEPENDENCY_NAMES)
  list(APPEND libtool_FIND_DEPENDENCY_NAMES automake)
  list(REMOVE_DUPLICATES libtool_FIND_DEPENDENCY_NAMES)
else()
  set(libtool_FIND_DEPENDENCY_NAMES automake)
endif()
set(automake_FIND_MODE "NO_MODULE")

########### VARIABLES #######################################################################
#############################################################################################
set(libtool_PACKAGE_FOLDER_RELEASE "/home/juliangro/.conan2/p/b/libto201aaf23b4bde/p")
set(libtool_BUILD_MODULES_PATHS_RELEASE )


set(libtool_INCLUDE_DIRS_RELEASE )
set(libtool_RES_DIRS_RELEASE )
set(libtool_DEFINITIONS_RELEASE )
set(libtool_SHARED_LINK_FLAGS_RELEASE )
set(libtool_EXE_LINK_FLAGS_RELEASE )
set(libtool_OBJECTS_RELEASE )
set(libtool_COMPILE_DEFINITIONS_RELEASE )
set(libtool_COMPILE_OPTIONS_C_RELEASE )
set(libtool_COMPILE_OPTIONS_CXX_RELEASE )
set(libtool_LIB_DIRS_RELEASE "${libtool_PACKAGE_FOLDER_RELEASE}/lib")
set(libtool_BIN_DIRS_RELEASE )
set(libtool_LIBRARY_TYPE_RELEASE STATIC)
set(libtool_IS_HOST_WINDOWS_RELEASE 0)
set(libtool_LIBS_RELEASE )
set(libtool_SYSTEM_LIBS_RELEASE dl)
set(libtool_FRAMEWORK_DIRS_RELEASE )
set(libtool_FRAMEWORKS_RELEASE )
set(libtool_BUILD_DIRS_RELEASE )
set(libtool_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(libtool_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${libtool_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${libtool_COMPILE_OPTIONS_C_RELEASE}>")
set(libtool_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${libtool_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${libtool_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${libtool_EXE_LINK_FLAGS_RELEASE}>")


set(libtool_COMPONENTS_RELEASE )