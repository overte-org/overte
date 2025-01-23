########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

list(APPEND nss_COMPONENT_NAMES nss::util nss::freebl nss::tools nss::libnss nss::softokn nss::ssl nss::smime nss::nss_pc)
list(REMOVE_DUPLICATES nss_COMPONENT_NAMES)
if(DEFINED nss_FIND_DEPENDENCY_NAMES)
  list(APPEND nss_FIND_DEPENDENCY_NAMES nspr)
  list(REMOVE_DUPLICATES nss_FIND_DEPENDENCY_NAMES)
else()
  set(nss_FIND_DEPENDENCY_NAMES nspr)
endif()
set(nspr_FIND_MODE "NO_MODULE")

########### VARIABLES #######################################################################
#############################################################################################
set(nss_PACKAGE_FOLDER_RELEASE "/home/juliangro/.conan2/p/b/nssb1d8c7def7e78/p")
set(nss_BUILD_MODULES_PATHS_RELEASE )


set(nss_INCLUDE_DIRS_RELEASE "${nss_PACKAGE_FOLDER_RELEASE}/include"
			"${nss_PACKAGE_FOLDER_RELEASE}/include/nss")
set(nss_RES_DIRS_RELEASE )
set(nss_DEFINITIONS_RELEASE )
set(nss_SHARED_LINK_FLAGS_RELEASE )
set(nss_EXE_LINK_FLAGS_RELEASE )
set(nss_OBJECTS_RELEASE )
set(nss_COMPILE_DEFINITIONS_RELEASE )
set(nss_COMPILE_OPTIONS_C_RELEASE )
set(nss_COMPILE_OPTIONS_CXX_RELEASE )
set(nss_LIB_DIRS_RELEASE "${nss_PACKAGE_FOLDER_RELEASE}/lib")
set(nss_BIN_DIRS_RELEASE "${nss_PACKAGE_FOLDER_RELEASE}/bin")
set(nss_LIBRARY_TYPE_RELEASE SHARED)
set(nss_IS_HOST_WINDOWS_RELEASE 0)
set(nss_LIBS_RELEASE smime3 ssl3 softokn3 nss3 freebl3 nssutil3)
set(nss_SYSTEM_LIBS_RELEASE )
set(nss_FRAMEWORK_DIRS_RELEASE )
set(nss_FRAMEWORKS_RELEASE )
set(nss_BUILD_DIRS_RELEASE )
set(nss_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(nss_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${nss_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${nss_COMPILE_OPTIONS_C_RELEASE}>")
set(nss_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${nss_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${nss_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${nss_EXE_LINK_FLAGS_RELEASE}>")


set(nss_COMPONENTS_RELEASE nss::util nss::freebl nss::tools nss::libnss nss::softokn nss::ssl nss::smime nss::nss_pc)
########### COMPONENT nss::nss_pc VARIABLES ############################################

set(nss_nss_nss_pc_INCLUDE_DIRS_RELEASE "${nss_PACKAGE_FOLDER_RELEASE}/include")
set(nss_nss_nss_pc_LIB_DIRS_RELEASE "${nss_PACKAGE_FOLDER_RELEASE}/lib")
set(nss_nss_nss_pc_BIN_DIRS_RELEASE "${nss_PACKAGE_FOLDER_RELEASE}/bin")
set(nss_nss_nss_pc_LIBRARY_TYPE_RELEASE SHARED)
set(nss_nss_nss_pc_IS_HOST_WINDOWS_RELEASE 0)
set(nss_nss_nss_pc_RES_DIRS_RELEASE )
set(nss_nss_nss_pc_DEFINITIONS_RELEASE )
set(nss_nss_nss_pc_OBJECTS_RELEASE )
set(nss_nss_nss_pc_COMPILE_DEFINITIONS_RELEASE )
set(nss_nss_nss_pc_COMPILE_OPTIONS_C_RELEASE "")
set(nss_nss_nss_pc_COMPILE_OPTIONS_CXX_RELEASE "")
set(nss_nss_nss_pc_LIBS_RELEASE )
set(nss_nss_nss_pc_SYSTEM_LIBS_RELEASE )
set(nss_nss_nss_pc_FRAMEWORK_DIRS_RELEASE )
set(nss_nss_nss_pc_FRAMEWORKS_RELEASE )
set(nss_nss_nss_pc_DEPENDENCIES_RELEASE nss::libnss nss::ssl nss::smime)
set(nss_nss_nss_pc_SHARED_LINK_FLAGS_RELEASE )
set(nss_nss_nss_pc_EXE_LINK_FLAGS_RELEASE )
set(nss_nss_nss_pc_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(nss_nss_nss_pc_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${nss_nss_nss_pc_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${nss_nss_nss_pc_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${nss_nss_nss_pc_EXE_LINK_FLAGS_RELEASE}>
)
set(nss_nss_nss_pc_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${nss_nss_nss_pc_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${nss_nss_nss_pc_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT nss::smime VARIABLES ############################################

set(nss_nss_smime_INCLUDE_DIRS_RELEASE "${nss_PACKAGE_FOLDER_RELEASE}/include")
set(nss_nss_smime_LIB_DIRS_RELEASE "${nss_PACKAGE_FOLDER_RELEASE}/lib")
set(nss_nss_smime_BIN_DIRS_RELEASE "${nss_PACKAGE_FOLDER_RELEASE}/bin")
set(nss_nss_smime_LIBRARY_TYPE_RELEASE SHARED)
set(nss_nss_smime_IS_HOST_WINDOWS_RELEASE 0)
set(nss_nss_smime_RES_DIRS_RELEASE )
set(nss_nss_smime_DEFINITIONS_RELEASE )
set(nss_nss_smime_OBJECTS_RELEASE )
set(nss_nss_smime_COMPILE_DEFINITIONS_RELEASE )
set(nss_nss_smime_COMPILE_OPTIONS_C_RELEASE "")
set(nss_nss_smime_COMPILE_OPTIONS_CXX_RELEASE "")
set(nss_nss_smime_LIBS_RELEASE smime3)
set(nss_nss_smime_SYSTEM_LIBS_RELEASE )
set(nss_nss_smime_FRAMEWORK_DIRS_RELEASE )
set(nss_nss_smime_FRAMEWORKS_RELEASE )
set(nss_nss_smime_DEPENDENCIES_RELEASE nss::libnss nss::util nspr::nspr)
set(nss_nss_smime_SHARED_LINK_FLAGS_RELEASE )
set(nss_nss_smime_EXE_LINK_FLAGS_RELEASE )
set(nss_nss_smime_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(nss_nss_smime_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${nss_nss_smime_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${nss_nss_smime_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${nss_nss_smime_EXE_LINK_FLAGS_RELEASE}>
)
set(nss_nss_smime_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${nss_nss_smime_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${nss_nss_smime_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT nss::ssl VARIABLES ############################################

set(nss_nss_ssl_INCLUDE_DIRS_RELEASE "${nss_PACKAGE_FOLDER_RELEASE}/include")
set(nss_nss_ssl_LIB_DIRS_RELEASE "${nss_PACKAGE_FOLDER_RELEASE}/lib")
set(nss_nss_ssl_BIN_DIRS_RELEASE "${nss_PACKAGE_FOLDER_RELEASE}/bin")
set(nss_nss_ssl_LIBRARY_TYPE_RELEASE SHARED)
set(nss_nss_ssl_IS_HOST_WINDOWS_RELEASE 0)
set(nss_nss_ssl_RES_DIRS_RELEASE )
set(nss_nss_ssl_DEFINITIONS_RELEASE )
set(nss_nss_ssl_OBJECTS_RELEASE )
set(nss_nss_ssl_COMPILE_DEFINITIONS_RELEASE )
set(nss_nss_ssl_COMPILE_OPTIONS_C_RELEASE "")
set(nss_nss_ssl_COMPILE_OPTIONS_CXX_RELEASE "")
set(nss_nss_ssl_LIBS_RELEASE ssl3)
set(nss_nss_ssl_SYSTEM_LIBS_RELEASE )
set(nss_nss_ssl_FRAMEWORK_DIRS_RELEASE )
set(nss_nss_ssl_FRAMEWORKS_RELEASE )
set(nss_nss_ssl_DEPENDENCIES_RELEASE nss::libnss nss::util nspr::nspr)
set(nss_nss_ssl_SHARED_LINK_FLAGS_RELEASE )
set(nss_nss_ssl_EXE_LINK_FLAGS_RELEASE )
set(nss_nss_ssl_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(nss_nss_ssl_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${nss_nss_ssl_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${nss_nss_ssl_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${nss_nss_ssl_EXE_LINK_FLAGS_RELEASE}>
)
set(nss_nss_ssl_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${nss_nss_ssl_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${nss_nss_ssl_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT nss::softokn VARIABLES ############################################

set(nss_nss_softokn_INCLUDE_DIRS_RELEASE "${nss_PACKAGE_FOLDER_RELEASE}/include")
set(nss_nss_softokn_LIB_DIRS_RELEASE "${nss_PACKAGE_FOLDER_RELEASE}/lib")
set(nss_nss_softokn_BIN_DIRS_RELEASE "${nss_PACKAGE_FOLDER_RELEASE}/bin")
set(nss_nss_softokn_LIBRARY_TYPE_RELEASE SHARED)
set(nss_nss_softokn_IS_HOST_WINDOWS_RELEASE 0)
set(nss_nss_softokn_RES_DIRS_RELEASE )
set(nss_nss_softokn_DEFINITIONS_RELEASE )
set(nss_nss_softokn_OBJECTS_RELEASE )
set(nss_nss_softokn_COMPILE_DEFINITIONS_RELEASE )
set(nss_nss_softokn_COMPILE_OPTIONS_C_RELEASE "")
set(nss_nss_softokn_COMPILE_OPTIONS_CXX_RELEASE "")
set(nss_nss_softokn_LIBS_RELEASE softokn3)
set(nss_nss_softokn_SYSTEM_LIBS_RELEASE )
set(nss_nss_softokn_FRAMEWORK_DIRS_RELEASE )
set(nss_nss_softokn_FRAMEWORKS_RELEASE )
set(nss_nss_softokn_DEPENDENCIES_RELEASE nss::libnss nss::freebl)
set(nss_nss_softokn_SHARED_LINK_FLAGS_RELEASE )
set(nss_nss_softokn_EXE_LINK_FLAGS_RELEASE )
set(nss_nss_softokn_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(nss_nss_softokn_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${nss_nss_softokn_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${nss_nss_softokn_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${nss_nss_softokn_EXE_LINK_FLAGS_RELEASE}>
)
set(nss_nss_softokn_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${nss_nss_softokn_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${nss_nss_softokn_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT nss::libnss VARIABLES ############################################

set(nss_nss_libnss_INCLUDE_DIRS_RELEASE "${nss_PACKAGE_FOLDER_RELEASE}/include"
			"${nss_PACKAGE_FOLDER_RELEASE}/include/nss")
set(nss_nss_libnss_LIB_DIRS_RELEASE "${nss_PACKAGE_FOLDER_RELEASE}/lib")
set(nss_nss_libnss_BIN_DIRS_RELEASE "${nss_PACKAGE_FOLDER_RELEASE}/bin")
set(nss_nss_libnss_LIBRARY_TYPE_RELEASE SHARED)
set(nss_nss_libnss_IS_HOST_WINDOWS_RELEASE 0)
set(nss_nss_libnss_RES_DIRS_RELEASE )
set(nss_nss_libnss_DEFINITIONS_RELEASE )
set(nss_nss_libnss_OBJECTS_RELEASE )
set(nss_nss_libnss_COMPILE_DEFINITIONS_RELEASE )
set(nss_nss_libnss_COMPILE_OPTIONS_C_RELEASE "")
set(nss_nss_libnss_COMPILE_OPTIONS_CXX_RELEASE "")
set(nss_nss_libnss_LIBS_RELEASE nss3)
set(nss_nss_libnss_SYSTEM_LIBS_RELEASE )
set(nss_nss_libnss_FRAMEWORK_DIRS_RELEASE )
set(nss_nss_libnss_FRAMEWORKS_RELEASE )
set(nss_nss_libnss_DEPENDENCIES_RELEASE nss::util nspr::nspr)
set(nss_nss_libnss_SHARED_LINK_FLAGS_RELEASE )
set(nss_nss_libnss_EXE_LINK_FLAGS_RELEASE )
set(nss_nss_libnss_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(nss_nss_libnss_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${nss_nss_libnss_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${nss_nss_libnss_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${nss_nss_libnss_EXE_LINK_FLAGS_RELEASE}>
)
set(nss_nss_libnss_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${nss_nss_libnss_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${nss_nss_libnss_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT nss::tools VARIABLES ############################################

set(nss_nss_tools_INCLUDE_DIRS_RELEASE "${nss_PACKAGE_FOLDER_RELEASE}/include")
set(nss_nss_tools_LIB_DIRS_RELEASE "${nss_PACKAGE_FOLDER_RELEASE}/lib")
set(nss_nss_tools_BIN_DIRS_RELEASE "${nss_PACKAGE_FOLDER_RELEASE}/bin")
set(nss_nss_tools_LIBRARY_TYPE_RELEASE SHARED)
set(nss_nss_tools_IS_HOST_WINDOWS_RELEASE 0)
set(nss_nss_tools_RES_DIRS_RELEASE )
set(nss_nss_tools_DEFINITIONS_RELEASE )
set(nss_nss_tools_OBJECTS_RELEASE )
set(nss_nss_tools_COMPILE_DEFINITIONS_RELEASE )
set(nss_nss_tools_COMPILE_OPTIONS_C_RELEASE "")
set(nss_nss_tools_COMPILE_OPTIONS_CXX_RELEASE "")
set(nss_nss_tools_LIBS_RELEASE )
set(nss_nss_tools_SYSTEM_LIBS_RELEASE )
set(nss_nss_tools_FRAMEWORK_DIRS_RELEASE )
set(nss_nss_tools_FRAMEWORKS_RELEASE )
set(nss_nss_tools_DEPENDENCIES_RELEASE nspr::nspr)
set(nss_nss_tools_SHARED_LINK_FLAGS_RELEASE )
set(nss_nss_tools_EXE_LINK_FLAGS_RELEASE )
set(nss_nss_tools_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(nss_nss_tools_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${nss_nss_tools_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${nss_nss_tools_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${nss_nss_tools_EXE_LINK_FLAGS_RELEASE}>
)
set(nss_nss_tools_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${nss_nss_tools_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${nss_nss_tools_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT nss::freebl VARIABLES ############################################

set(nss_nss_freebl_INCLUDE_DIRS_RELEASE "${nss_PACKAGE_FOLDER_RELEASE}/include"
			"${nss_PACKAGE_FOLDER_RELEASE}/include/nss")
set(nss_nss_freebl_LIB_DIRS_RELEASE "${nss_PACKAGE_FOLDER_RELEASE}/lib")
set(nss_nss_freebl_BIN_DIRS_RELEASE "${nss_PACKAGE_FOLDER_RELEASE}/bin")
set(nss_nss_freebl_LIBRARY_TYPE_RELEASE SHARED)
set(nss_nss_freebl_IS_HOST_WINDOWS_RELEASE 0)
set(nss_nss_freebl_RES_DIRS_RELEASE )
set(nss_nss_freebl_DEFINITIONS_RELEASE )
set(nss_nss_freebl_OBJECTS_RELEASE )
set(nss_nss_freebl_COMPILE_DEFINITIONS_RELEASE )
set(nss_nss_freebl_COMPILE_OPTIONS_C_RELEASE "")
set(nss_nss_freebl_COMPILE_OPTIONS_CXX_RELEASE "")
set(nss_nss_freebl_LIBS_RELEASE freebl3)
set(nss_nss_freebl_SYSTEM_LIBS_RELEASE )
set(nss_nss_freebl_FRAMEWORK_DIRS_RELEASE )
set(nss_nss_freebl_FRAMEWORKS_RELEASE )
set(nss_nss_freebl_DEPENDENCIES_RELEASE )
set(nss_nss_freebl_SHARED_LINK_FLAGS_RELEASE )
set(nss_nss_freebl_EXE_LINK_FLAGS_RELEASE )
set(nss_nss_freebl_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(nss_nss_freebl_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${nss_nss_freebl_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${nss_nss_freebl_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${nss_nss_freebl_EXE_LINK_FLAGS_RELEASE}>
)
set(nss_nss_freebl_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${nss_nss_freebl_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${nss_nss_freebl_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT nss::util VARIABLES ############################################

set(nss_nss_util_INCLUDE_DIRS_RELEASE "${nss_PACKAGE_FOLDER_RELEASE}/include"
			"${nss_PACKAGE_FOLDER_RELEASE}/include/nss")
set(nss_nss_util_LIB_DIRS_RELEASE "${nss_PACKAGE_FOLDER_RELEASE}/lib")
set(nss_nss_util_BIN_DIRS_RELEASE "${nss_PACKAGE_FOLDER_RELEASE}/bin")
set(nss_nss_util_LIBRARY_TYPE_RELEASE SHARED)
set(nss_nss_util_IS_HOST_WINDOWS_RELEASE 0)
set(nss_nss_util_RES_DIRS_RELEASE )
set(nss_nss_util_DEFINITIONS_RELEASE )
set(nss_nss_util_OBJECTS_RELEASE )
set(nss_nss_util_COMPILE_DEFINITIONS_RELEASE )
set(nss_nss_util_COMPILE_OPTIONS_C_RELEASE "")
set(nss_nss_util_COMPILE_OPTIONS_CXX_RELEASE "")
set(nss_nss_util_LIBS_RELEASE nssutil3)
set(nss_nss_util_SYSTEM_LIBS_RELEASE )
set(nss_nss_util_FRAMEWORK_DIRS_RELEASE )
set(nss_nss_util_FRAMEWORKS_RELEASE )
set(nss_nss_util_DEPENDENCIES_RELEASE nspr::nspr)
set(nss_nss_util_SHARED_LINK_FLAGS_RELEASE )
set(nss_nss_util_EXE_LINK_FLAGS_RELEASE )
set(nss_nss_util_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(nss_nss_util_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${nss_nss_util_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${nss_nss_util_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${nss_nss_util_EXE_LINK_FLAGS_RELEASE}>
)
set(nss_nss_util_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${nss_nss_util_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${nss_nss_util_COMPILE_OPTIONS_C_RELEASE}>")