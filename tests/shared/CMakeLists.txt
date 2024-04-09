
# Declare dependencies
macro (setup_testcase_dependencies)
  setup_memory_debugger()
  setup_thread_debugger()


  # link in the shared libraries
  link_hifi_libraries(shared test-utils)

  package_libraries_for_deployment()


  if("${TARGET_NAME}" STREQUAL "shared-PathUtilsTests")
    # Create a fake FHS structure. The FHS auto-detection detects the layout based on the location of the binary, so to test
    # that we just need to make this structure and copy the test into it.

    add_custom_command(TARGET shared-PathUtilsTests POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/fhs/usr/bin )
    add_custom_command(TARGET shared-PathUtilsTests POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/fhs/etc/overte )
    add_custom_command(TARGET shared-PathUtilsTests POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/fhs/var/run/overte )
    add_custom_command(TARGET shared-PathUtilsTests POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/fhs/usr/lib/overte )
    add_custom_command(TARGET shared-PathUtilsTests POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/fhs/usr/share/overte )
    add_custom_command(TARGET shared-PathUtilsTests POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/shared-PathUtilsTests ${CMAKE_CURRENT_BINARY_DIR}/fhs/usr/bin/ )

  endif()
endmacro ()

setup_hifi_testcase()
