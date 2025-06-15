set(TARGET_NAME ice-server)

# setup the project and link required Qt modules
setup_hifi_project(Network)

# link the shared hifi libraries
link_hifi_libraries(embedded-webserver networking shared)
package_libraries_for_deployment()

# find OpenSSL
find_package(OpenSSL REQUIRED)

setup_memory_debugger()
setup_thread_debugger()

# append OpenSSL to our list of libraries to link
target_link_libraries(${TARGET_NAME} OpenSSL::SSL OpenSSL::Crypto)
