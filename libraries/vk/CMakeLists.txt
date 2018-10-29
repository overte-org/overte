set(TARGET_NAME vk)
setup_hifi_library(Gui)
GroupSources(src)
link_hifi_libraries(shared gl)

target_vulkan()
