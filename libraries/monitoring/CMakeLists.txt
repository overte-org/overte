set(TARGET_NAME monitoring)
setup_hifi_library()
link_hifi_libraries(shared networking)

add_crashpad()
target_breakpad()

if (WIN32)
  add_compile_definitions(_USE_MATH_DEFINES)
endif()
