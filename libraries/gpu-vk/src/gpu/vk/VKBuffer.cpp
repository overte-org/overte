//
//  Created by Bradley Austin Davis on 2016/08/07
//  Adapted for Vulkan in 2022-2025 by dr Karol Suprynowicz.
//  Copyright 2013-2018 High Fidelity, Inc.
//  Copyright 2023-2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//
#include "VKBuffer.h"
#include "VKBackend.h"

using namespace gpu::vk;

VKBuffer* VKBuffer::sync(VKBackend& backend, const gpu::Buffer& buffer, bool transfer) {
    if (buffer.getSysmem().getSize() != 0) {
        if (buffer._getUpdateCount == 0) {
            qWarning() << "Unsynced buffer";
        }
        if (buffer._getUpdateCount < buffer._applyUpdateCount) {
            qWarning() << "Unsynced buffer " << buffer._getUpdateCount << " " << buffer._applyUpdateCount;
        }
    }

    VKBuffer* object = gpu::Backend::getGPUObject<VKBuffer>(buffer);
    // Has the storage size changed? VKTODO: is something missing here?

    bool newBuffer = false;
    if (!object || object->stagingAllocation.size < buffer._renderSysmem.getSize() /*object->_stamp != buffer._renderSysmem.getStamp()*/) {
        if (object) {
            backend._currentFrame->_bufferResizeCounter++;
        } else {
            backend._currentFrame->_bufferCreationCounter++;
        }
        // VKTODO: Should previous gpuobject be replaced?
        object = new VKBuffer(backend, buffer);
        newBuffer = true;
        backend._buffers.insert(object);
    }
    // VKTODO: delete the old buffer after rendering the frame
    if (0 != (buffer._renderPages._flags & PageManager::DIRTY) || newBuffer) {
        object->transferToStaging(backend);
        if (transfer) {
            if (backend._inRenderTransferPass) {
                // All barriers will be added in one command at the end of transfer pass.
                object->transferWithDelayedBarrier(backend, backend._currentCommandBuffer);
            } else {
                object->transferWithBarrier(backend, backend._currentCommandBuffer);
            }
        }
    }
    Q_ASSERT(object->allocation.size == buffer._renderSysmem.getSize());

    return object;
}

VkBuffer VKBuffer::getBuffer(VKBackend& backend, const gpu::Buffer& buffer) {
    VKBuffer* bo = sync(backend, buffer);
    if (bo) {
        return bo->buffer;
    } else {
        return nullptr;
    }
}

void VKBuffer::transferToStaging(VKBackend &backend) {
    Size offset;
    Size blockSize;
    Size currentPage { 0 };
    auto data = _gpuObject._renderSysmem.readData();
    //auto dataSize = _gpuObject._renderSysmem.getSize();
    while (_gpuObject._renderPages.getNextTransferBlock(offset, blockSize, currentPage)) {
        if (offset + blockSize > _localData.size()) {
            Q_ASSERT(false);
        }
        memcpy(_localData.data()+offset, data+offset, blockSize);
    }
    stagingAllocation.map();
    stagingAllocation.copy(stagingAllocation.size, _localData.data());
    stagingAllocation.flush(VK_WHOLE_SIZE);
    stagingAllocation.unmap();
    _gpuObject._renderPages._flags &= ~PageManager::DIRTY;
}

void VKBuffer::transferOnly(VKBackend &backend) {
    // Only for development and testing, should never be called otherwise.
    Q_ASSERT(false);
    auto device = backend.getContext().device;
    VkCommandBuffer copyCmd = device->createCommandBuffer(device->graphicsCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    VkBufferCopy copyRegion = {};

    copyRegion.size = _localData.size();
    vkCmdCopyBuffer(
        copyCmd,
        stagingBuffer,
        buffer,
        1,
        &copyRegion);

    // VKTODO: is a memory barrier needed here?
    // VKTODO: all transfers should be done as a part of the transfer pass.

    device->flushCommandBuffer(copyCmd, backend.getContext().graphicsQueue, device->graphicsCommandPool, true);
}

void VKBuffer::transferWithBarrier(VKBackend &backend, VkCommandBuffer commandBuffer) {
    VkBufferCopy copyRegion = {};

    copyRegion.size = _localData.size();
    vkCmdCopyBuffer(
        commandBuffer,
        stagingBuffer,
        buffer,
        1,
        &copyRegion);

    VkBufferMemoryBarrier bufferMemoryBarrier{
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = buffer,
        .offset = 0,
        .size = VK_WHOLE_SIZE
    };

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
        VK_FLAGS_NONE,
        0, nullptr,
        1, &bufferMemoryBarrier,
        0, nullptr);

    backend._currentFrame->_bufferTransferCounterRenderPass++;
    incrementTransferCount(backend, copyRegion.size);
    // VKTODO: is a memory barrier needed here or just buffer barrier is ok?
    // VKTODO: all transfers should be done as a part of the transfer pass.
}

void VKBuffer::transferWithDelayedBarrier(VKBackend &backend, VkCommandBuffer commandBuffer) {
    VkBufferCopy copyRegion = {};

    copyRegion.size = _localData.size() == 0 ? 256 : _localData.size();
    vkCmdCopyBuffer(
        commandBuffer,
        stagingBuffer,
        buffer,
        1,
        &copyRegion);

    VkBufferMemoryBarrier bufferMemoryBarrier{
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = buffer,
        .offset = 0,
        .size = VK_WHOLE_SIZE
    };

    backend._currentFrame->addBufferBarrier(bufferMemoryBarrier);

    backend._currentFrame->_bufferTransferCounterTransferPass++;
    incrementTransferCount(backend, copyRegion.size);
    // VKTODO: is a memory barrier needed here or just buffer barrier is ok?
}

void VKBuffer::incrementTransferCount(VKBackend& backend, size_t transferSize) {
    backend._currentFrame->_bufferTransferredBytes += transferSize;

    if(_gpuObject.getUsage() & gpu::Buffer::UniformBuffer) {
        backend._currentFrame->_uniformBufferTransferCounter++;
    }
    if(_gpuObject.getUsage() & gpu::Buffer::VertexBuffer) {
        backend._currentFrame->_vertexBufferTransferCounter++;
    }
    if(_gpuObject.getUsage() & gpu::Buffer::IndexBuffer) {
        backend._currentFrame->_indexBufferTransferCounter++;
    }
    if(_gpuObject.getUsage() & gpu::Buffer::ResourceBuffer) {
        backend._currentFrame->_resourceBufferTransferCounter++;
    }
}

void VKBuffer::incrementCount(VKBackend& backend) {
    static int uniformCountTransfer = 0;
    static int uniformCountRender = 0;
    static int vertexCountTransfer = 0;
    static int vertexCountRender = 0;
    static int indexCountTransfer = 0;
    static int indexCountRender = 0;
    static int resourceCountTransfer = 0;
    static int resourceCountRender = 0;

    if(_gpuObject.getUsage() & gpu::Buffer::UniformBuffer) {
        if (backend._inRenderTransferPass) {
            uniformCountTransfer++;
        } else {
            uniformCountRender++;
        }
    }
    if(_gpuObject.getUsage() & gpu::Buffer::VertexBuffer) {
        if (backend._inRenderTransferPass) {
            vertexCountTransfer++;
        } else {
            vertexCountRender++;
        }
    }
    if(_gpuObject.getUsage() & gpu::Buffer::IndexBuffer) {
        if (backend._inRenderTransferPass) {
            indexCountTransfer++;
        } else {
            indexCountRender++;
        }
    }
    if(_gpuObject.getUsage() & gpu::Buffer::ResourceBuffer) {
        if (backend._inRenderTransferPass) {
            resourceCountTransfer++;
        } else {
            resourceCountRender++;
        }
    }
}

VKBuffer::VKBuffer(VKBackend& backend, const gpu::Buffer& gpuBuffer) : VKObject(backend, gpuBuffer),
    _stamp(gpuBuffer._renderSysmem.getStamp()) {

    Backend::setGPUObject(gpuBuffer, this);
    // Flags match VkBufferUsageFlagBits - this was in original Vulkan branch
    VkBufferUsageFlags usageFlags{ (VkBufferUsageFlags)gpuBuffer.getUsage() };
    auto data = _gpuObject._renderSysmem.readData();
    auto dataSize = _gpuObject._renderSysmem.getSize();
    _localData.resize(dataSize);
    memcpy(_localData.data(), data, dataSize);

    this->stagingAllocation.size = gpuBuffer._renderSysmem.getSize();
    this->allocation.size = this->stagingAllocation.size;

    VkBufferCreateInfo transferVkBufferCI{};
    transferVkBufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    transferVkBufferCI.pNext = nullptr;
    transferVkBufferCI.flags = 0;
    transferVkBufferCI.size = stagingAllocation.size == 0 ? 256 : stagingAllocation.size;
    transferVkBufferCI.usage = VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT;
    transferVkBufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    transferVkBufferCI.queueFamilyIndexCount = 0;
    transferVkBufferCI.pQueueFamilyIndices = nullptr;

    VmaAllocationCreateInfo transferAllocationCI = {};
    transferAllocationCI.usage = VMA_MEMORY_USAGE_CPU_TO_GPU; // VKTODO: sometime in the future we should free staging buffers for large buffers that are not updated.
    transferAllocationCI.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    VK_CHECK_RESULT(vmaCreateBuffer(stagingAllocation.getAllocator(), &transferVkBufferCI, &transferAllocationCI,
                                    &stagingBuffer, &stagingAllocation.allocation, nullptr));

    VkBufferCreateInfo vkBufferCI{};
    vkBufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCI.pNext = nullptr;
    vkBufferCI.flags = 0;
    vkBufferCI.size = allocation.size == 0 ? 256 : allocation.size; // TODO: why does renderer create zero-size buffers sometimes?
    vkBufferCI.usage = usageFlags | VK_BUFFER_USAGE_2_TRANSFER_DST_BIT;
    vkBufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkBufferCI.queueFamilyIndexCount = 0;
    vkBufferCI.pQueueFamilyIndices = nullptr;

    VmaAllocationCreateInfo allocationCI = {};
    allocationCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocationCI.flags = 0;
    VK_CHECK_RESULT(vmaCreateBuffer(allocation.getAllocator(), &vkBufferCI, &allocationCI,
                                    &buffer, &allocation.allocation, nullptr));

    incrementCount(backend);

    Backend::bufferCount.increment();
    Backend::bufferGPUMemSize.update(0, stagingAllocation.size);
}

VKBuffer::~VKBuffer() {
    Backend::bufferGPUMemSize.update(allocation.size, 0);
    Backend::bufferCount.decrement();
    auto backend = _backend.lock();
    if (backend) {
        auto &recycler = backend->getContext().recycler;
        recycler.trashVkBuffer(stagingBuffer);
        recycler.trashVmaAllocation(stagingAllocation.allocation);
        recycler.trashVkBuffer(buffer);
        recycler.trashVmaAllocation(allocation.allocation);
        recycler.bufferDeleted(this);
    }
}
