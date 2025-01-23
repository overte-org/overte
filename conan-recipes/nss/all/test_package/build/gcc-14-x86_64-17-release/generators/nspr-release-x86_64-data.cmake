########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(nspr_COMPONENT_NAMES "")
if(DEFINED nspr_FIND_DEPENDENCY_NAMES)
  list(APPEND nspr_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES nspr_FIND_DEPENDENCY_NAMES)
else()
  set(nspr_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(nspr_PACKAGE_FOLDER_RELEASE "/home/juliangro/.conan2/p/b/nsprc5d1b95cc6e17/p")
set(nspr_BUILD_MODULES_PATHS_RELEASE )


set(nspr_INCLUDE_DIRS_RELEASE "${nspr_PACKAGE_FOLDER_RELEASE}/include"
			"${nspr_PACKAGE_FOLDER_RELEASE}/include/nspr")
set(nspr_RES_DIRS_RELEASE "${nspr_PACKAGE_FOLDER_RELEASE}/res")
set(nspr_DEFINITIONS_RELEASE )
set(nspr_SHARED_LINK_FLAGS_RELEASE )
set(nspr_EXE_LINK_FLAGS_RELEASE )
set(nspr_OBJECTS_RELEASE )
set(nspr_COMPILE_DEFINITIONS_RELEASE )
set(nspr_COMPILE_OPTIONS_C_RELEASE )
set(nspr_COMPILE_OPTIONS_CXX_RELEASE )
set(nspr_LIB_DIRS_RELEASE "${nspr_PACKAGE_FOLDER_RELEASE}/lib")
set(nspr_BIN_DIRS_RELEASE "${nspr_PACKAGE_FOLDER_RELEASE}/bin")
set(nspr_LIBRARY_TYPE_RELEASE SHARED)
set(nspr_IS_HOST_WINDOWS_RELEASE 0)
set(nspr_LIBS_RELEASE plds4 plc4 nspr4)
set(nspr_SYSTEM_LIBS_RELEASE dl pthread rt)
set(nspr_FRAMEWORK_DIRS_RELEASE )
set(nspr_FRAMEWORKS_RELEASE )
set(nspr_BUILD_DIRS_RELEASE )
set(nspr_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(nspr_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${nspr_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${nspr_COMPILE_OPTIONS_C_RELEASE}>")
set(nspr_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${nspr_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${nspr_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${nspr_EXE_LINK_FLAGS_RELEASE}>")


set(nspr_COMPONENTS_RELEASE )