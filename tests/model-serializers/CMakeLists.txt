
# Declare dependencies
macro (setup_testcase_dependencies)
  # link in the shared libraries
  link_hifi_libraries(shared test-utils model-serializers networking model-networking hfm graphics gpu image)

  package_libraries_for_deployment()
endmacro ()

setup_hifi_testcase(Script Network)
