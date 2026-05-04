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

class VKBuffer : public VKObject<gpu::Buffer> {
public:
    static VKBuffer* sync(VKBackend& backend, const gpu::Buffer& buffer, bool transfer = true);
    static VkBuffer getBuffer(VKBackend& backend, const gpu::Buffer& buffer);

    /// Transfers buffer to GPU using separate command buffer.
    /// This is intended for testing only.
    void transferOnly(VKBackend &backend);

    /// Transfers CPU-side buffer to GPU-accessible staging area.
    void transferToStaging(VKBackend &backend);

    /// Adds command to transfer data from the staging buffer to GPU-only buffer.
    /// Adds a barrier immediately after transfer command.
    void transferWithBarrier(VkCommandBuffer commandBuffer);

    /// Adds command to transfer data from the staging buffer to GPU-only buffer.
    /// Adds a barrier that will be waited on at the end of transfer pass.
    /// Can be used only during transfer pass.
    void transferWithDelayedBarrier(VKBackend &backend, VkCommandBuffer commandBuffer);

    ~VKBuffer() override;

    VkBuffer buffer{ VK_NULL_HANDLE };
    vks::Allocation allocation;
    VkBuffer stagingBuffer{ VK_NULL_HANDLE };
    vks::Allocation stagingAllocation;

protected:
    VKBuffer(VKBackend& backend, const gpu::Buffer& buffer);
    const Stamp _stamp{ 0 };

    /// Used for debugging.
    /// Uploading buffers during render pass causes significant performance loss.
    void incrementCount(VKBackend& backend);

    // Local copy of buffer data. Updates are copied into it before transfer.
    std::vector<uint8_t> _localData;
};

} }

#endif
