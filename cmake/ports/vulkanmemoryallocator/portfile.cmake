set(SOURCE_VERSION 3.3.0)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
    REF 009ecd192c1289c7529bff248a16cfe896254816
    SHA512 3a47a4bc81562b96f598f357b803d2219fa167e6eb87779837efa6e85fc6eaff8e1cfbdea0935117a7b3c630cc99944b94e91823eafa8a94d80b2c459f33c317
    HEAD_REF master
)

file(INSTALL ${SOURCE_PATH}/include/vk_mem_alloc.h DESTINATION ${CURRENT_PACKAGES_DIR}/include/vma)
file(INSTALL ${SOURCE_PATH}/LICENSE.txt DESTINATION ${CURRENT_PACKAGES_DIR}/share/vulkanmemoryallocator RENAME copyright)
