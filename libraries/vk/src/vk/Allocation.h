#pragma once

#include "Config.h"
#include "Device.h"


namespace vks {

    // A wrapper class for an allocation, either an Image or Buffer.  Not intended to be used used directly
    // but only as a base class providing common functionality for the classes below.
    //
    // Provides easy to use mechanisms for mapping, unmapping and copying host data to the device memory
    struct Allocation {
        static void initAllocator(const vk::PhysicalDevice&, const vk::Device&);

        void* rawmap(size_t offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);
        void unmap();

        template <typename T = void>
        inline T* map(size_t offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) {
            return (T*)rawmap(offset, size);
        }

        inline void copy(size_t size, const void* data, VkDeviceSize offset = 0) const {
            memcpy((uint8_t*)mapped + offset, data, size);
        }

        template<typename T>
        inline void copy(const T& data, VkDeviceSize offset = 0) const {
            copy(sizeof(T), &data, offset);
        }

        template<typename T>
        inline void copy(const std::vector<T>& data, VkDeviceSize offset = 0) const {
            copy(sizeof(T) * data.size(), data.data(), offset);
        }

        void flush(vk::DeviceSize size, vk::DeviceSize offset = 0);
        void invalidate(vk::DeviceSize size, vk::DeviceSize offset = 0);
        virtual void destroy();


        vks::Device device;
        vk::DeviceSize size{ 0 };
        vk::DeviceSize alignment{ 0 };
        vk::DeviceSize allocSize{ 0 };

#if VULKAN_USE_VMA
        static VmaAllocator& getAllocator();

        VmaAllocation allocation;
        /** @brief Memory propertys flags to be filled by external source at buffer creation (to query at some later point) */
        vk::MemoryPropertyFlags memoryPropertyFlags;
#else
        vk::DeviceMemory memory;
#endif
        void* mapped{ nullptr };

    };
}
