# Copyright 2017-2019, High Fidelity, Inc.
# Copyright 2021-2023 Overte e.V.
# SPDX-License-Identifier: Apache-2.0

set(TARGET_NAME oven)

setup_hifi_project(Widgets Gui Concurrent)

link_hifi_libraries(shared shaders image gpu ktx model-serializers hfm baking graphics networking procedural material-networking model-baker task)
include_hifi_library_headers(script-engine)

setup_memory_debugger()
setup_thread_debugger()
add_crashpad()
target_breakpad()


if (WIN32)
  package_libraries_for_deployment()
elseif (UNIX AND NOT APPLE)
  find_package(Threads REQUIRED)
  if(THREADS_HAVE_PTHREAD_ARG)
    target_compile_options(oven PUBLIC "-pthread")
  endif()
elseif (APPLE)
  # Fix up the rpath so macdeployqt works
  set_target_properties(${TARGET_NAME} PROPERTIES INSTALL_RPATH "@executable_path/../Frameworks")
endif()

install_beside_console()
install_beside_client()

