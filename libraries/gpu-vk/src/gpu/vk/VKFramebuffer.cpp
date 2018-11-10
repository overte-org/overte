//
//  Created by Bradley Austin Davis on 2016/08/07
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#if 0
#include "VKFramebuffer.h"
#include "VKBackend.h"

using namespace gpu;
using namespace gpu::gl;

VKFramebuffer::~VKFramebuffer() { 
    if (_id) { 
        auto backend = _backend.lock();
        if (backend) {
            backend->releaseFramebuffer(_id);
        }
    } 
}

bool VKFramebuffer::checkStatus(VKenum target) const {
    bool result = false;
    switch (_status) {
    case VK_FRAMEBUFFER_COMPLETE:
        // Success !
        result = true;
        break;
    case VK_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        qCDebug(gpu_vk_logging) << "VKFramebuffer::syncGPUObject : Framebuffer not valid, VK_FRAMEBUFFER_INCOMPLETE_ATTACHMENT.";
        break;
    case VK_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        qCDebug(gpu_vk_logging) << "VKFramebuffer::syncGPUObject : Framebuffer not valid, VK_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT.";
        break;
    case VK_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        qCDebug(gpu_vk_logging) << "VKFramebuffer::syncGPUObject : Framebuffer not valid, VK_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER.";
        break;
    case VK_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        qCDebug(gpu_vk_logging) << "VKFramebuffer::syncGPUObject : Framebuffer not valid, VK_FRAMEBUFFER_INCOMPLETE_READ_BUFFER.";
        break;
    case VK_FRAMEBUFFER_UNSUPPORTED:
        qCDebug(gpu_vk_logging) << "VKFramebuffer::syncGPUObject : Framebuffer not valid, VK_FRAMEBUFFER_UNSUPPORTED.";
        break;
    }
    return result;
}
#endif