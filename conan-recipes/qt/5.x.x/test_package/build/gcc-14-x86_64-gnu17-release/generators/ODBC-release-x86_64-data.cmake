########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

list(APPEND odbc_COMPONENT_NAMES odbc::_odbc odbc::odbcinst odbc::odbccr)
list(REMOVE_DUPLICATES odbc_COMPONENT_NAMES)
if(DEFINED odbc_FIND_DEPENDENCY_NAMES)
  list(APPEND odbc_FIND_DEPENDENCY_NAMES libtool)
  list(REMOVE_DUPLICATES odbc_FIND_DEPENDENCY_NAMES)
else()
  set(odbc_FIND_DEPENDENCY_NAMES libtool)
endif()
set(libtool_FIND_MODE "NO_MODULE")

########### VARIABLES #######################################################################
#############################################################################################
set(odbc_PACKAGE_FOLDER_RELEASE "/home/juliangro/.conan2/p/b/odbcb745ba7f94832/p")
set(odbc_BUILD_MODULES_PATHS_RELEASE )


set(odbc_INCLUDE_DIRS_RELEASE )
set(odbc_RES_DIRS_RELEASE )
set(odbc_DEFINITIONS_RELEASE )
set(odbc_SHARED_LINK_FLAGS_RELEASE )
set(odbc_EXE_LINK_FLAGS_RELEASE )
set(odbc_OBJECTS_RELEASE )
set(odbc_COMPILE_DEFINITIONS_RELEASE )
set(odbc_COMPILE_OPTIONS_C_RELEASE )
set(odbc_COMPILE_OPTIONS_CXX_RELEASE )
set(odbc_LIB_DIRS_RELEASE "${odbc_PACKAGE_FOLDER_RELEASE}/lib")
set(odbc_BIN_DIRS_RELEASE )
set(odbc_LIBRARY_TYPE_RELEASE STATIC)
set(odbc_IS_HOST_WINDOWS_RELEASE 0)
set(odbc_LIBS_RELEASE )
set(odbc_SYSTEM_LIBS_RELEASE pthread)
set(odbc_FRAMEWORK_DIRS_RELEASE )
set(odbc_FRAMEWORKS_RELEASE )
set(odbc_BUILD_DIRS_RELEASE )
set(odbc_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(odbc_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${odbc_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${odbc_COMPILE_OPTIONS_C_RELEASE}>")
set(odbc_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${odbc_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${odbc_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${odbc_EXE_LINK_FLAGS_RELEASE}>")


set(odbc_COMPONENTS_RELEASE odbc::_odbc odbc::odbcinst odbc::odbccr)
########### COMPONENT odbc::odbccr VARIABLES ############################################

set(odbc_odbc_odbccr_INCLUDE_DIRS_RELEASE )
set(odbc_odbc_odbccr_LIB_DIRS_RELEASE "${odbc_PACKAGE_FOLDER_RELEASE}/lib")
set(odbc_odbc_odbccr_BIN_DIRS_RELEASE )
set(odbc_odbc_odbccr_LIBRARY_TYPE_RELEASE STATIC)
set(odbc_odbc_odbccr_IS_HOST_WINDOWS_RELEASE 0)
set(odbc_odbc_odbccr_RES_DIRS_RELEASE )
set(odbc_odbc_odbccr_DEFINITIONS_RELEASE )
set(odbc_odbc_odbccr_OBJECTS_RELEASE )
set(odbc_odbc_odbccr_COMPILE_DEFINITIONS_RELEASE )
set(odbc_odbc_odbccr_COMPILE_OPTIONS_C_RELEASE "")
set(odbc_odbc_odbccr_COMPILE_OPTIONS_CXX_RELEASE "")
set(odbc_odbc_odbccr_LIBS_RELEASE )
set(odbc_odbc_odbccr_SYSTEM_LIBS_RELEASE )
set(odbc_odbc_odbccr_FRAMEWORK_DIRS_RELEASE )
set(odbc_odbc_odbccr_FRAMEWORKS_RELEASE )
set(odbc_odbc_odbccr_DEPENDENCIES_RELEASE )
set(odbc_odbc_odbccr_SHARED_LINK_FLAGS_RELEASE )
set(odbc_odbc_odbccr_EXE_LINK_FLAGS_RELEASE )
set(odbc_odbc_odbccr_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(odbc_odbc_odbccr_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${odbc_odbc_odbccr_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${odbc_odbc_odbccr_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${odbc_odbc_odbccr_EXE_LINK_FLAGS_RELEASE}>
)
set(odbc_odbc_odbccr_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${odbc_odbc_odbccr_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${odbc_odbc_odbccr_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT odbc::odbcinst VARIABLES ############################################

set(odbc_odbc_odbcinst_INCLUDE_DIRS_RELEASE )
set(odbc_odbc_odbcinst_LIB_DIRS_RELEASE "${odbc_PACKAGE_FOLDER_RELEASE}/lib")
set(odbc_odbc_odbcinst_BIN_DIRS_RELEASE )
set(odbc_odbc_odbcinst_LIBRARY_TYPE_RELEASE STATIC)
set(odbc_odbc_odbcinst_IS_HOST_WINDOWS_RELEASE 0)
set(odbc_odbc_odbcinst_RES_DIRS_RELEASE )
set(odbc_odbc_odbcinst_DEFINITIONS_RELEASE )
set(odbc_odbc_odbcinst_OBJECTS_RELEASE )
set(odbc_odbc_odbcinst_COMPILE_DEFINITIONS_RELEASE )
set(odbc_odbc_odbcinst_COMPILE_OPTIONS_C_RELEASE "")
set(odbc_odbc_odbcinst_COMPILE_OPTIONS_CXX_RELEASE "")
set(odbc_odbc_odbcinst_LIBS_RELEASE )
set(odbc_odbc_odbcinst_SYSTEM_LIBS_RELEASE pthread)
set(odbc_odbc_odbcinst_FRAMEWORK_DIRS_RELEASE )
set(odbc_odbc_odbcinst_FRAMEWORKS_RELEASE )
set(odbc_odbc_odbcinst_DEPENDENCIES_RELEASE libtool::libtool)
set(odbc_odbc_odbcinst_SHARED_LINK_FLAGS_RELEASE )
set(odbc_odbc_odbcinst_EXE_LINK_FLAGS_RELEASE )
set(odbc_odbc_odbcinst_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(odbc_odbc_odbcinst_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${odbc_odbc_odbcinst_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${odbc_odbc_odbcinst_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${odbc_odbc_odbcinst_EXE_LINK_FLAGS_RELEASE}>
)
set(odbc_odbc_odbcinst_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${odbc_odbc_odbcinst_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${odbc_odbc_odbcinst_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT odbc::_odbc VARIABLES ############################################

set(odbc_odbc__odbc_INCLUDE_DIRS_RELEASE )
set(odbc_odbc__odbc_LIB_DIRS_RELEASE "${odbc_PACKAGE_FOLDER_RELEASE}/lib")
set(odbc_odbc__odbc_BIN_DIRS_RELEASE )
set(odbc_odbc__odbc_LIBRARY_TYPE_RELEASE STATIC)
set(odbc_odbc__odbc_IS_HOST_WINDOWS_RELEASE 0)
set(odbc_odbc__odbc_RES_DIRS_RELEASE )
set(odbc_odbc__odbc_DEFINITIONS_RELEASE )
set(odbc_odbc__odbc_OBJECTS_RELEASE )
set(odbc_odbc__odbc_COMPILE_DEFINITIONS_RELEASE )
set(odbc_odbc__odbc_COMPILE_OPTIONS_C_RELEASE "")
set(odbc_odbc__odbc_COMPILE_OPTIONS_CXX_RELEASE "")
set(odbc_odbc__odbc_LIBS_RELEASE )
set(odbc_odbc__odbc_SYSTEM_LIBS_RELEASE pthread)
set(odbc_odbc__odbc_FRAMEWORK_DIRS_RELEASE )
set(odbc_odbc__odbc_FRAMEWORKS_RELEASE )
set(odbc_odbc__odbc_DEPENDENCIES_RELEASE libtool::libtool)
set(odbc_odbc__odbc_SHARED_LINK_FLAGS_RELEASE )
set(odbc_odbc__odbc_EXE_LINK_FLAGS_RELEASE )
set(odbc_odbc__odbc_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(odbc_odbc__odbc_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${odbc_odbc__odbc_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${odbc_odbc__odbc_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${odbc_odbc__odbc_EXE_LINK_FLAGS_RELEASE}>
)
set(odbc_odbc__odbc_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${odbc_odbc__odbc_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${odbc_odbc__odbc_COMPILE_OPTIONS_C_RELEASE}>")