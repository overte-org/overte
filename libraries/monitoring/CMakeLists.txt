set(TARGET_NAME monitoring)
setup_hifi_library()
link_hifi_libraries(shared networking)

add_crashpad()
target_breakpad()


if (UNIX AND NOT APPLE)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)
  target_link_libraries(Threads::Threads)
endif()

if (WIN32)
  add_compile_definitions(_USE_MATH_DEFINES)
endif()
