//
//  Created by Bradley Austin Davis on 2016/08/07
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_vk_VKBuffer_h
#define hifi_gpu_vk_VKBuffer_h

#include "VKForward.h"
#include "VKShared.h"
#include <vk/VulkanBuffer.h>

namespace gpu { namespace vk {

class VKBuffer : public VKObject<gpu::Buffer>, public vks::Buffer {
public:
    static VKBuffer* sync(VKBackend& backend, const gpu::Buffer& buffer);
    static VkBuffer getBuffer(VKBackend& backend, const gpu::Buffer& buffer);
    void transfer();

    ~VKBuffer() override;
protected:
    VKBuffer(VKBackend& backend, const gpu::Buffer& buffer);
    const Stamp _stamp{ 0 };
    // Local copy of buffer data. Updates are copied into it before transfer.
    std::vector<uint8_t> _localData;
};

} }

#endif