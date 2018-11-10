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
#include <vk/Buffer.h>

namespace gpu { namespace vulkan {

class VKBuffer : public VKObject<gpu::Buffer>, public vks::Buffer {
public:
    static VKBuffer* sync(VKBackend& backend, const gpu::Buffer& buffer);
    static ::vk::Buffer getBuffer(VKBackend& backend, const gpu::Buffer& buffer);

    ~VKBuffer();
protected:
    VKBuffer(VKBackend& backend, const gpu::Buffer& buffer);
    const Stamp _stamp{ 0 };
};

} }

#endif
