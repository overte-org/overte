########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(automake_COMPONENT_NAMES "")
if(DEFINED automake_FIND_DEPENDENCY_NAMES)
  list(APPEND automake_FIND_DEPENDENCY_NAMES autoconf)
  list(REMOVE_DUPLICATES automake_FIND_DEPENDENCY_NAMES)
else()
  set(automake_FIND_DEPENDENCY_NAMES autoconf)
endif()
set(autoconf_FIND_MODE "NO_MODULE")

########### VARIABLES #######################################################################
#############################################################################################
set(automake_PACKAGE_FOLDER_RELEASE "/home/juliangro/.conan2/p/autom480a421c82a75/p")
set(automake_BUILD_MODULES_PATHS_RELEASE )


set(automake_INCLUDE_DIRS_RELEASE )
set(automake_RES_DIRS_RELEASE "${automake_PACKAGE_FOLDER_RELEASE}/res")
set(automake_DEFINITIONS_RELEASE )
set(automake_SHARED_LINK_FLAGS_RELEASE )
set(automake_EXE_LINK_FLAGS_RELEASE )
set(automake_OBJECTS_RELEASE )
set(automake_COMPILE_DEFINITIONS_RELEASE )
set(automake_COMPILE_OPTIONS_C_RELEASE )
set(automake_COMPILE_OPTIONS_CXX_RELEASE )
set(automake_LIB_DIRS_RELEASE )
set(automake_BIN_DIRS_RELEASE "${automake_PACKAGE_FOLDER_RELEASE}/bin")
set(automake_LIBRARY_TYPE_RELEASE UNKNOWN)
set(automake_IS_HOST_WINDOWS_RELEASE 0)
set(automake_LIBS_RELEASE )
set(automake_SYSTEM_LIBS_RELEASE )
set(automake_FRAMEWORK_DIRS_RELEASE )
set(automake_FRAMEWORKS_RELEASE )
set(automake_BUILD_DIRS_RELEASE )
set(automake_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(automake_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${automake_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${automake_COMPILE_OPTIONS_C_RELEASE}>")
set(automake_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${automake_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${automake_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${automake_EXE_LINK_FLAGS_RELEASE}>")


set(automake_COMPONENTS_RELEASE )