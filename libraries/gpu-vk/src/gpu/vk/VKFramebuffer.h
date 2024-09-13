//
//  Created by Bradley Austin Davis on 2016/08/07
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_vk_VKFramebuffer_h
#define hifi_gpu_vk_VKFramebuffer_h

#include "VKShared.h"
#include "VKBackend.h"
#include <vk/VulkanFrameBuffer.hpp>

namespace gpu { namespace vulkan {

class VKFramebuffer : public vulkan::VKObject<Framebuffer> {
public:
    //vks::Framebuffer vksFrameBuffer;
    VkFramebuffer vkFramebuffer{VK_NULL_HANDLE};

    static VKFramebuffer* sync(vulkan::VKBackend& backend, const Framebuffer& framebuffer) {
        VKFramebuffer* object = Backend::getGPUObject<VKFramebuffer>(framebuffer);

        bool needsUpdate{ false };
        if (!object ||
            framebuffer.getDepthStamp() != object->_depthStamp ||
            framebuffer.getColorStamps() != object->_colorStamps) {
            needsUpdate = true;
        }

        // If GPU object already created and in sync
        if (!needsUpdate) {
            return object;
        } else if (framebuffer.isEmpty()) {
            // NO framebuffer definition yet so let's avoid thinking
            return nullptr;
        }

        // need to have a gpu object?
        if (!object) {
            // All is green, assign the gpuobject to the Framebuffer
            object = new VKFramebuffer(backend.shared_from_this(), framebuffer);
            Backend::setGPUObject(framebuffer, object);
        }

        object->update();
        return object;
    }

    // VKTODO: what type should it return?
    template <typename VKFramebufferType>
    static uint32_t getId(vulkan::VKBackend& backend, const Framebuffer& framebuffer) {
        VKFramebufferType* fbo = sync(backend, framebuffer);
        if (fbo) {
            return fbo->_id;
        } else {
            return 0;
        }
    }

    // VKTODO: probably a Vulkan handle instead of this
    //const VKuint& _fbo { _id };
    //std::vector<VKenum> _colorBuffers;
    Stamp _depthStamp { 0 };
    std::vector<Stamp> _colorStamps;

protected:
    enum FramebufferStatus { VK_FRAMEBUFFER_COMPLETE } _status;
    virtual void update();
    bool checkStatus(FramebufferStatus target) const;

    // VKTODO: We need a check on backend.lock(), or to pass backend reference instead
    VKFramebuffer(const std::weak_ptr<vulkan::VKBackend>& backend, const Framebuffer& framebuffer) : VKObject(*backend.lock(), framebuffer) {}
        //vksFrameBuffer(backend.lock()->getContext().device.get()){}
    // VKTODO: Do we need virtual destructor here?
    ~VKFramebuffer();

};

} }


#endif
