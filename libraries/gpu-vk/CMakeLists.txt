set(TARGET_NAME gpu-vk)
setup_hifi_library()
link_hifi_libraries(shared shaders vk gl gpu)
GroupSources("src")

target_vulkan()