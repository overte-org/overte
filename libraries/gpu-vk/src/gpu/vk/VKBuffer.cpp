//
//  Created by Bradley Austin Davis on 2016/08/07
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "VKBuffer.h"
#include "VKBackend.h"

using namespace gpu::vulkan;

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
    if (!object || object->_stamp != buffer._renderSysmem.getStamp()) {
        object = new VKBuffer(backend, buffer);
    }

    return object;
}

vk::Buffer VKBuffer::getBuffer(VKBackend& backend, const gpu::Buffer& buffer) {
    VKBuffer* bo = sync(backend, buffer);
    if (bo) {
        return bo->buffer;
    } else {
        return nullptr;
    }
}

VKBuffer::VKBuffer(VKBackend& backend, const gpu::Buffer& buffer) : VKObject(backend, buffer) {
    vk::BufferUsageFlags usageFlags{ (VkBufferUsageFlags)buffer.getUsage() };
    (vks::Buffer&)(*this) =
        backend._context.createBuffer(usageFlags, buffer.getSize(), vk::MemoryPropertyFlagBits::eDeviceLocal);

    Backend::bufferCount.increment();
    Backend::bufferGPUMemSize.update(0, size);
}

VKBuffer::~VKBuffer() {
    Backend::bufferGPUMemSize.update(size, 0);
    Backend::bufferCount.decrement();
    auto backend = _backend.lock();
    if (backend) {
        backend->trash(*this);
    }
}

