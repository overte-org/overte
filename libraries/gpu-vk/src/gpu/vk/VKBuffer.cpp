//
//  Created by Bradley Austin Davis on 2016/08/07
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "VKBuffer.h"
#include "VKBackend.h"

using namespace gpu::vk;

VKBuffer* VKBuffer::sync(VKBackend& backend, const gpu::Buffer& buffer) {
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

    if (!object || object->_stamp != buffer._renderSysmem.getStamp()) {
        // VKTODO: Should previous gpuobject be replaced?
        object = new VKBuffer(backend, buffer);
    }
    // VKTODO: delete the old buffer after rendering the frame
    if (0 != (buffer._renderPages._flags & PageManager::DIRTY)) {
        object->transfer();
    }
    Q_ASSERT(object->size == buffer._renderSysmem.getSize());

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
void VKBuffer::transfer() {
    Size offset;
    Size blockSize;
    Size currentPage { 0 };
    auto data = _gpuObject._renderSysmem.readData();
    auto dataSize = _gpuObject._renderSysmem.getSize();
    while (_gpuObject._renderPages.getNextTransferBlock(offset, blockSize, currentPage)) {
        if (offset + blockSize > _localData.size()) {
            Q_ASSERT(false);
        }
        memcpy(_localData.data()+offset, data+offset, blockSize);
    }
    map();
    copy(size, _localData.data());
    flush(VK_WHOLE_SIZE);
    unmap();
    _gpuObject._renderPages._flags &= ~PageManager::DIRTY;
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

    this->size = gpuBuffer._renderSysmem.getSize();
    VkBufferCreateInfo vkBufferCI{};
    vkBufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCI.pNext = nullptr;
    vkBufferCI.flags = 0;
    vkBufferCI.size = size == 0 ? 256 : size;
    vkBufferCI.usage = usageFlags;
    vkBufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkBufferCI.queueFamilyIndexCount = 0;
    vkBufferCI.pQueueFamilyIndices = nullptr;

    VmaAllocationCreateInfo allocationCI = {};
    allocationCI.usage = VMA_MEMORY_USAGE_CPU_TO_GPU; // VKTODO: different kind will be required later for device-local memory
    allocationCI.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    auto pCreateInfo = &vkBufferCI;
    VK_CHECK_RESULT(vmaCreateBuffer(Allocation::getAllocator(), pCreateInfo, &allocationCI,
                                    &buffer, &allocation, nullptr));

    map();
    copy(size, _localData.data());
    flush(VK_WHOLE_SIZE);
    unmap();

    Backend::bufferCount.increment();
    Backend::bufferGPUMemSize.update(0, size);
}

VKBuffer::~VKBuffer() {
    Backend::bufferGPUMemSize.update(size, 0);
    Backend::bufferCount.decrement();
    auto backend = _backend.lock();
    if (backend) {
        auto &recycler = backend->getContext().recycler;
        recycler.trashVkBuffer(buffer);
        recycler.trashVmaAllocation(allocation);
        recycler.bufferDeleted(this);
    }
}
