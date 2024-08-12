set(TARGET_NAME vk)
setup_hifi_library(Gui)
GroupSources(src)
link_hifi_libraries(shared shaders gl)
find_package(Qt5Gui)
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

target_vulkan()
