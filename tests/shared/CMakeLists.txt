
# Declare dependencies
macro (setup_testcase_dependencies)
  setup_memory_debugger()
  setup_thread_debugger()


  # link in the shared libraries
  link_hifi_libraries(shared test-utils)

  package_libraries_for_deployment()
endmacro ()

setup_hifi_testcase()
