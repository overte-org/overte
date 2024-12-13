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

};
