########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(opengl_COMPONENT_NAMES "")
if(DEFINED opengl_FIND_DEPENDENCY_NAMES)
  list(APPEND opengl_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES opengl_FIND_DEPENDENCY_NAMES)
else()
  set(opengl_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(opengl_PACKAGE_FOLDER_RELEASE "/home/juliangro/.conan2/p/openg6b1f04d72d11e/p")
set(opengl_BUILD_MODULES_PATHS_RELEASE )


set(opengl_INCLUDE_DIRS_RELEASE )
set(opengl_RES_DIRS_RELEASE )
set(opengl_DEFINITIONS_RELEASE )
set(opengl_SHARED_LINK_FLAGS_RELEASE )
set(opengl_EXE_LINK_FLAGS_RELEASE )
set(opengl_OBJECTS_RELEASE )
set(opengl_COMPILE_DEFINITIONS_RELEASE )
set(opengl_COMPILE_OPTIONS_C_RELEASE )
set(opengl_COMPILE_OPTIONS_CXX_RELEASE )
set(opengl_LIB_DIRS_RELEASE )
set(opengl_BIN_DIRS_RELEASE )
set(opengl_LIBRARY_TYPE_RELEASE SHARED)
set(opengl_IS_HOST_WINDOWS_RELEASE 0)
set(opengl_LIBS_RELEASE )
set(opengl_SYSTEM_LIBS_RELEASE GL)
set(opengl_FRAMEWORK_DIRS_RELEASE )
set(opengl_FRAMEWORKS_RELEASE )
set(opengl_BUILD_DIRS_RELEASE )
set(opengl_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(opengl_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${opengl_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${opengl_COMPILE_OPTIONS_C_RELEASE}>")
set(opengl_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${opengl_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${opengl_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${opengl_EXE_LINK_FLAGS_RELEASE}>")


set(opengl_COMPONENTS_RELEASE )