########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(libxshmfence_COMPONENT_NAMES "")
if(DEFINED libxshmfence_FIND_DEPENDENCY_NAMES)
  list(APPEND libxshmfence_FIND_DEPENDENCY_NAMES xorg-proto)
  list(REMOVE_DUPLICATES libxshmfence_FIND_DEPENDENCY_NAMES)
else()
  set(libxshmfence_FIND_DEPENDENCY_NAMES xorg-proto)
endif()
set(xorg-proto_FIND_MODE "NO_MODULE")

########### VARIABLES #######################################################################
#############################################################################################
set(libxshmfence_PACKAGE_FOLDER_RELEASE "/home/juliangro/.conan2/p/b/libxse5635a2079549/p")
set(libxshmfence_BUILD_MODULES_PATHS_RELEASE )


set(libxshmfence_INCLUDE_DIRS_RELEASE )
set(libxshmfence_RES_DIRS_RELEASE )
set(libxshmfence_DEFINITIONS_RELEASE )
set(libxshmfence_SHARED_LINK_FLAGS_RELEASE )
set(libxshmfence_EXE_LINK_FLAGS_RELEASE )
set(libxshmfence_OBJECTS_RELEASE )
set(libxshmfence_COMPILE_DEFINITIONS_RELEASE )
set(libxshmfence_COMPILE_OPTIONS_C_RELEASE )
set(libxshmfence_COMPILE_OPTIONS_CXX_RELEASE )
set(libxshmfence_LIB_DIRS_RELEASE "${libxshmfence_PACKAGE_FOLDER_RELEASE}/lib")
set(libxshmfence_BIN_DIRS_RELEASE )
set(libxshmfence_LIBRARY_TYPE_RELEASE STATIC)
set(libxshmfence_IS_HOST_WINDOWS_RELEASE 0)
set(libxshmfence_LIBS_RELEASE )
set(libxshmfence_SYSTEM_LIBS_RELEASE )
set(libxshmfence_FRAMEWORK_DIRS_RELEASE )
set(libxshmfence_FRAMEWORKS_RELEASE )
set(libxshmfence_BUILD_DIRS_RELEASE )
set(libxshmfence_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(libxshmfence_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${libxshmfence_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${libxshmfence_COMPILE_OPTIONS_C_RELEASE}>")
set(libxshmfence_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${libxshmfence_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${libxshmfence_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${libxshmfence_EXE_LINK_FLAGS_RELEASE}>")


set(libxshmfence_COMPONENTS_RELEASE )