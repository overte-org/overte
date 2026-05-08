# 
#  Created by Bradley Austin Davis on 2016/02/16
#
#  Distributed under the Apache License, Version 2.0.
#  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
# 
macro(TARGET_VULKAN)
    if(APPLE)
        find_package(MoltenVK REQUIRED)
        target_include_directories(${TARGET_NAME} PRIVATE ${MoltenVK_INCLUDE_DIR})
        target_link_libraries(${TARGET_NAME} ${MoltenVK_LIBRARY})
    else()
        find_package(Vulkan REQUIRED)
        target_include_directories(${TARGET_NAME} PRIVATE ${VULKAN_INCLUDE_DIR})
        target_link_libraries(${TARGET_NAME} ${VULKAN_LIBRARY})
    endif()
    find_package(VulkanMemoryAllocator REQUIRED)
    find_package(Qt5 COMPONENTS X11Extras REQUIRED)
    target_link_libraries(${TARGET_NAME} GPUOpen::VulkanMemoryAllocator)
    target_link_libraries(${TARGET_NAME} ${Qt5X11Extras_LIBRARIES})
endmacro()
