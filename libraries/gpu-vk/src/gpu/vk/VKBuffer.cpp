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
    // Has the storage size changed?
    if (buffer.isDirty()) {
        printf("dirty\n");
    }
    if (!object || object->_stamp != buffer._renderSysmem.getStamp()) {
        object = new VKBuffer(backend, buffer);
    }
    // VKTODO: delete the old buffer after rendering the frame
    if (0 != (buffer._renderPages._flags & PageManager::DIRTY)) {
        object->transfer();
    }

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
    Size size;
    Size currentPage { 0 };
    auto data = _gpuObject._renderSysmem.readData();
    auto dataSize = _gpuObject._renderSysmem.getSize();
    while (_gpuObject._renderPages.getNextTransferBlock(offset, size, currentPage)) {
        if (offset + size > _localData.size()) {
            size = _localData.size() - offset;
            Q_ASSERT(false);
        }
        memcpy(_localData.data()+offset, data+offset, size);
    }
    map();
    copy(_localData.size(), _localData.data());
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
    // VKTODO: VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT is wrong here but is not used on VMA yet.
    (vks::Buffer&)(*this) =
        //backend._context.createBuffer(usageFlags, gpuBuffer.getSize(), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        backend._context.createBuffer(usageFlags, dataSize, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    map();
    copy(dataSize, _localData.size());
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
        backend->recycler.trashVkBuffer(buffer);
        backend->recycler.trashVmaAllocation(allocation);
    }
}

