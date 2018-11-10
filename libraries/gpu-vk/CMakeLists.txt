set(TARGET_NAME gpu-vk)
setup_hifi_library()
link_hifi_libraries(shared shaders vk gpu)
GroupSources("src")

target_vulkan()