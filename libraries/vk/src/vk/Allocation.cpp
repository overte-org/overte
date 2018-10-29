#include "Allocation.h"

#include <mutex>

using namespace vks;

#if VULKAN_USE_VMA


VmaAllocator& Allocation::getAllocator() {
    static VmaAllocator allocator;
    return allocator;
}

void Allocation::initAllocator(const vk::PhysicalDevice& physicalDevice, const vk::Device& device) {
    static std::once_flag once;
    std::call_once(once, [&] {
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.physicalDevice = physicalDevice;
        allocatorInfo.device = device;

        VmaAllocator& allocator = getAllocator();
        vmaCreateAllocator(&allocatorInfo, &allocator);
    });
}



void Allocation::flush(vk::DeviceSize size, vk::DeviceSize offset) {
    vmaFlushAllocation(getAllocator(), allocation, offset, size);
}

void Allocation::invalidate(vk::DeviceSize size, vk::DeviceSize offset) {
    vmaInvalidateAllocation(getAllocator(), allocation, offset, size);
}

void* Allocation::rawmap(size_t offset, vk::DeviceSize size) {
    if (offset != 0 || size != VK_WHOLE_SIZE) {
        throw std::runtime_error("Unsupported");
    }

    if (!mapped) {
        vmaMapMemory(getAllocator(), allocation, &mapped);
    }

    return mapped;
}

void Allocation::unmap() {
    if (mapped) {
        vmaUnmapMemory(getAllocator(), allocation);
        mapped = nullptr;
    }
}

void Allocation::destroy() {
    unmap();
}

#else

void Alloction::initAllocator(const vk::PhysicalDevice&, const vk::Device&) {
}

void Alloction::flush(vk::DeviceSize size, vk::DeviceSize offset = 0) {
    return device.flushMappedMemoryRanges(vk::MappedMemoryRange{ memory, offset, size });
}

void Alloction::invalidate(vk::DeviceSize size, vk::DeviceSize offset = 0) {
    return device.invalidateMappedMemoryRanges(vk::MappedMemoryRange{ memory, offset, size });
}

void* Alloction::rawmap(size_t offset = 0, VkDeviceSize size) {
    mapped = device.mapMemory(memory, offset, size, vk::MemoryMapFlags());
    return (T*)mapped;
}

void Alloction::unmap() {
    device.unmapMemory(memory);
}

void Alloction::destroy();
{
    if (mapped) {
        unmap();
    }

    if (memory) {
        device.freeMemory(memory);
        memory = nullptr;
    }
}

#endif
