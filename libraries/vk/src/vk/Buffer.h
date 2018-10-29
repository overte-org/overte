#pragma once

#include "Allocation.h"

namespace vks
{    
    /**
    * @brief Encapsulates access to a Vulkan buffer backed up by device memory
    * @note To be filled by an external source like the VulkanDevice
    */
    struct Buffer : public Allocation
    {
    private:
        using Parent = Allocation;

    public:
        vk::Buffer buffer;
        /** @brief Usage flags to be filled by external source at buffer creation (to query at some later point) */
        vk::BufferUsageFlags usageFlags;
        vk::DescriptorBufferInfo descriptor;

        operator bool() const {
            return buffer.operator bool();
        }

        /**
        * Setup the default descriptor for this buffer
        *
        * @param size (Optional) Size of the memory range of the descriptor
        * @param offset (Optional) Byte offset from beginning
        *
        */
        void setupDescriptor(vk::DeviceSize size, vk::DeviceSize offset = 0) {
            descriptor.offset = offset;
            descriptor.buffer = buffer;
            descriptor.range = size;
        }

        /** 
        * Release all Vulkan resources held by this buffer
        */
        void destroy() {
            if (buffer) {
                device.destroy(buffer);
                buffer = vk::Buffer{};
            }
            Parent::destroy();
        }

    };
}
