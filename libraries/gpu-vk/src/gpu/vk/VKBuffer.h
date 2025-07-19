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
#ifndef hifi_gpu_vk_VKBuffer_h
#define hifi_gpu_vk_VKBuffer_h

#include "VKForward.h"
#include "VKShared.h"

namespace gpu { namespace vk {

class VKBuffer : public VKObject<gpu::Buffer>, public vks::Allocation {
public:
    static VKBuffer* sync(VKBackend& backend, const gpu::Buffer& buffer);
    static VkBuffer getBuffer(VKBackend& backend, const gpu::Buffer& buffer);
    void transfer();

    ~VKBuffer() override;

    VkBuffer buffer{ VK_NULL_HANDLE };
protected:
    VKBuffer(VKBackend& backend, const gpu::Buffer& buffer);
    const Stamp _stamp{ 0 };
    // Local copy of buffer data. Updates are copied into it before transfer.
    std::vector<uint8_t> _localData;
};

} }

#endif
