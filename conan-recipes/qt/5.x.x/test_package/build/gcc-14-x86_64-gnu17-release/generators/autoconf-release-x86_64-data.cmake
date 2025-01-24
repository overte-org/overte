########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(autoconf_COMPONENT_NAMES "")
if(DEFINED autoconf_FIND_DEPENDENCY_NAMES)
  list(APPEND autoconf_FIND_DEPENDENCY_NAMES m4)
  list(REMOVE_DUPLICATES autoconf_FIND_DEPENDENCY_NAMES)
else()
  set(autoconf_FIND_DEPENDENCY_NAMES m4)
endif()
set(m4_FIND_MODE "NO_MODULE")

########### VARIABLES #######################################################################
#############################################################################################
set(autoconf_PACKAGE_FOLDER_RELEASE "/home/juliangro/.conan2/p/autocf2af015330354/p")
set(autoconf_BUILD_MODULES_PATHS_RELEASE )


set(autoconf_INCLUDE_DIRS_RELEASE )
set(autoconf_RES_DIRS_RELEASE "${autoconf_PACKAGE_FOLDER_RELEASE}/res")
set(autoconf_DEFINITIONS_RELEASE )
set(autoconf_SHARED_LINK_FLAGS_RELEASE )
set(autoconf_EXE_LINK_FLAGS_RELEASE )
set(autoconf_OBJECTS_RELEASE )
set(autoconf_COMPILE_DEFINITIONS_RELEASE )
set(autoconf_COMPILE_OPTIONS_C_RELEASE )
set(autoconf_COMPILE_OPTIONS_CXX_RELEASE )
set(autoconf_LIB_DIRS_RELEASE )
set(autoconf_BIN_DIRS_RELEASE "${autoconf_PACKAGE_FOLDER_RELEASE}/bin")
set(autoconf_LIBRARY_TYPE_RELEASE UNKNOWN)
set(autoconf_IS_HOST_WINDOWS_RELEASE 0)
set(autoconf_LIBS_RELEASE )
set(autoconf_SYSTEM_LIBS_RELEASE )
set(autoconf_FRAMEWORK_DIRS_RELEASE )
set(autoconf_FRAMEWORKS_RELEASE )
set(autoconf_BUILD_DIRS_RELEASE )
set(autoconf_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(autoconf_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${autoconf_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${autoconf_COMPILE_OPTIONS_C_RELEASE}>")
set(autoconf_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${autoconf_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${autoconf_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${autoconf_EXE_LINK_FLAGS_RELEASE}>")


set(autoconf_COMPONENTS_RELEASE )