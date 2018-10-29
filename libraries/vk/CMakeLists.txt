set(TARGET_NAME vk)
setup_hifi_library(Gui)
GroupSources(src)
link_hifi_libraries(shared shaders gl)

target_vulkan()
