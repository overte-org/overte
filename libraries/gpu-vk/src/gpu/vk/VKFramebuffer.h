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

namespace gpu { namespace vk {

class VKFramebuffer : public VKObject<Framebuffer> {
public:
    template <typename VKFramebufferType>
    static VKFramebufferType* sync(VKBackend& backend, const Framebuffer& framebuffer) {
        VKFramebufferType* object = Backend::getGPUObject<VKFramebufferType>(framebuffer);

        bool needsUpate { false };
        if (!object ||
            framebuffer.getDepthStamp() != object->_depthStamp ||
            framebuffer.getColorStamps() != object->_colorStamps) {
            needsUpate = true;
        }

        // If GPU object already created and in sync
        if (!needsUpate) {
            return object;
        } else if (framebuffer.isEmpty()) {
            // NO framebuffer definition yet so let's avoid thinking
            return nullptr;
        }

        // need to have a gpu object?
        if (!object) {
            // All is green, assign the gpuobject to the Framebuffer
            object = new VKFramebufferType(backend.shared_from_this(), framebuffer);
            Backend::setGPUObject(framebuffer, object);
            (void)CHECK_VK_ERROR();
        }

        object->update();
        return object;
    }

    template <typename VKFramebufferType>
    static VKuint getId(VKBackend& backend, const Framebuffer& framebuffer) {
        VKFramebufferType* fbo = sync<VKFramebufferType>(backend, framebuffer);
        if (fbo) {
            return fbo->_id;
        } else {
            return 0;
        }
    }

    const VKuint& _fbo { _id };
    std::vector<VKenum> _colorBuffers;
    Stamp _depthStamp { 0 };
    std::vector<Stamp> _colorStamps;

protected:
    VKenum _status { VK_FRAMEBUFFER_COMPLETE };
    virtual void update() = 0;
    bool checkStatus(VKenum target) const;

    VKFramebuffer(const std::weak_ptr<VKBackend>& backend, const Framebuffer& framebuffer, VKuint id) : VKObject(backend, framebuffer, id) {}
    ~VKFramebuffer();

};

} }


#endif
