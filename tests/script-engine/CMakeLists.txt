
# Declare dependencies
macro (setup_testcase_dependencies)
  # link in the shared libraries
  link_hifi_libraries(shared test-utils script-engine networking)

  package_libraries_for_deployment()
endmacro ()

setup_hifi_testcase(Network)
