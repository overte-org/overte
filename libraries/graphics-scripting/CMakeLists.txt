set(TARGET_NAME graphics-scripting)
setup_hifi_library()
link_hifi_libraries(shared networking graphics model-serializers image material-networking model-networking script-engine)
include_hifi_library_headers(gpu)

if (WIN32)
  add_compile_definitions(_USE_MATH_DEFINES)
endif()