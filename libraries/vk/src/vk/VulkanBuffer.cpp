/*
* Vulkan buffer class
*
* Encapsulates a Vulkan buffer
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include <memory>
#include "VulkanBuffer.h"

namespace vks
{    
    /** 
    * Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
    * 
    * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete buffer range.
    * @param offset (Optional) Byte offset from beginning
    * 
    * @return VkResult of the buffer mapping call
    */
    /*VkResult Buffer::map(VkDeviceSize size, VkDeviceSize offset)
    {
#if VULKAN_USE_VMA
            return vmaMapMemory(getAllocator(),allocation, &mapped);
#else
        return vkMapMemory(device, memory, offset, size, 0, &mapped);
#endif
    }*/

    /**
    * Unmap a mapped memory range
    *
    * @note Does not return a result as vkUnmapMemory can't fail
    */
    /*void Buffer::unmap()
    {
        if (mapped)
        {
#if VULKAN_USE_VMA
                    vmaUnmapMemory(getAllocator(), allocation);
#else
                    vkUnmapMemory(device, memory);
#endif
            mapped = nullptr;
        }
    }*/

    /** 
    * Attach the allocated memory block to the buffer
    * 
    * @param offset (Optional) Byte offset (from the beginning) for the memory region to bind
    * 
    * @return VkResult of the bindBufferMemory call
    */
    VkResult Buffer::bind(VkDeviceSize offset)
    {
#if VULKAN_USE_VMA
        return vmaBindBufferMemory(getAllocator(), allocation, buffer);
#else
        return vkBindBufferMemory(device, buffer, memory, offset);
#endif
    }

    /**
    * Setup the default descriptor for this buffer
    *
    * @param size (Optional) Size of the memory range of the descriptor
    * @param offset (Optional) Byte offset from beginning
    *
    */
    void Buffer::setupDescriptor(VkDeviceSize size, VkDeviceSize offset)
    {
        descriptor.offset = offset;
        descriptor.buffer = buffer;
        descriptor.range = size;
    }

    // VKTODO: maybe this should be a static function returning buffer
    std::shared_ptr<Buffer> Buffer::createUniform(VkDeviceSize bufferSize) {
        std::shared_ptr<Buffer> newBuffer = std::make_shared<Buffer>();
        newBuffer->size = bufferSize;
        VkBufferCreateInfo bufferCI = {  };
        bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCI.size = newBuffer->size;
        bufferCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        VmaAllocationCreateInfo allocationCI{};
        allocationCI.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        allocationCI.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        vmaCreateBuffer(vks::Allocation::getAllocator(), &bufferCI, &allocationCI, &newBuffer->buffer, &newBuffer->allocation, nullptr);
        return newBuffer;
    }

    std::shared_ptr<Buffer> Buffer::createStorage(VkDeviceSize bufferSize) {
        std::shared_ptr<Buffer> newBuffer = std::make_shared<Buffer>();
        newBuffer->size = bufferSize;
        VkBufferCreateInfo bufferCI = {  };
        bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCI.size = newBuffer->size;
        bufferCI.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VmaAllocationCreateInfo allocationCI{};
        allocationCI.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        allocationCI.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        vmaCreateBuffer(vks::Allocation::getAllocator(), &bufferCI, &allocationCI, &newBuffer->buffer, &newBuffer->allocation, nullptr);
        return newBuffer;
    }

    std::shared_ptr<Buffer> Buffer::createVertex(VkDeviceSize bufferSize) {
        //VKTODO: This needs to be on GPU-only memory in the future
        std::shared_ptr<Buffer> newBuffer = std::make_shared<Buffer>();
        newBuffer->size = bufferSize;
        VkBufferCreateInfo bufferCI = {  };
        bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCI.size = newBuffer->size;
        bufferCI.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        VmaAllocationCreateInfo allocationCI{};
        allocationCI.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        allocationCI.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        vmaCreateBuffer(vks::Allocation::getAllocator(), &bufferCI, &allocationCI, &newBuffer->buffer, &newBuffer->allocation, nullptr);
        return newBuffer;
    }

    /**
    * Copies the specified data to the mapped buffer
    * 
    * @param data Pointer to the data to copy
    * @param size Size of the data to copy in machine units
    *
    */
    void Buffer::copyTo(void* data, VkDeviceSize size)
    {
        assert(mapped);
        memcpy(mapped, data, size);
    }

    /** 
    * Flush a memory range of the buffer to make it visible to the device
    *
    * @note Only required for non-coherent memory
    *
    * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the complete buffer range.
    * @param offset (Optional) Byte offset from beginning
    *
    * @return VkResult of the flush call
    */
    /*VkResult Buffer::flush(VkDeviceSize size, VkDeviceSize offset)
    {
#if VULKAN_USE_VMA
        vmaFlushAllocation(getAllocator(), allocation, offset, size);
        return VK_SUCCESS;
#else
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkFlushMappedMemoryRanges(device, 1, &mappedRange);
#endif
    }*/

    /**
    * Invalidate a memory range of the buffer to make it visible to the host
    *
    * @note Only required for non-coherent memory
    *
    * @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate the complete buffer range.
    * @param offset (Optional) Byte offset from beginning
    *
    * @return VkResult of the invalidate call
    */
    /*VkResult Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset)
    {
#if VULKAN_USE_VMA
        vmaInvalidateAllocation(getAllocator(), allocation, offset, size);
        return VK_SUCCESS;
#else
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkInvalidateMappedMemoryRanges(device, 1, &mappedRange);
#endif
    }*/

    /** 
    * Release all Vulkan resources held by this buffer
    */
    void Buffer::destroy()
    {
        if (buffer)
        {
            vkDestroyBuffer(device, buffer, nullptr);
        }
        Allocation::destroy();
#if VULKAN_USE_VMA
        if (allocation) {
            vmaFreeMemory(getAllocator(), allocation);
        }
#else
        if (memory)
        {
            vkFreeMemory(device, memory, nullptr);
        }
#endif
    }
};
