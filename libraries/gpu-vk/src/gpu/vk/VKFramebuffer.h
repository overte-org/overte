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

namespace gpu { namespace vk {

class VKFramebuffer : public vk::VKObject<Framebuffer> {
public:
    //vks::Framebuffer vksFrameBuffer;
    VkRenderPass vkRenderPass {VK_NULL_HANDLE};
    VkFramebuffer vkFramebuffer {VK_NULL_HANDLE};

    static VKFramebuffer* sync(vk::VKBackend& backend, const Framebuffer& framebuffer) {
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
    static uint32_t getId(vk::VKBackend& backend, const Framebuffer& framebuffer) {
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
    // Contains attachment number?
    //std::vector<uint32_t> _colorBuffers;

    // From VKS
    struct FramebufferAttachment
    {
        VkImage image;
        VkDeviceMemory memory;
        VkImageView view;
        VkFormat format;
        VkImageSubresourceRange subresourceRange;
        VkAttachmentDescription description;

        /**
        * @brief Returns true if the attachment has a depth component
        */
        bool hasDepth()
        {
            std::vector<VkFormat> formats =
                {
                    VK_FORMAT_D16_UNORM,
                    VK_FORMAT_X8_D24_UNORM_PACK32,
                    VK_FORMAT_D32_SFLOAT,
                    VK_FORMAT_D16_UNORM_S8_UINT,
                    VK_FORMAT_D24_UNORM_S8_UINT,
                    VK_FORMAT_D32_SFLOAT_S8_UINT,
                };
            return std::find(formats.begin(), formats.end(), format) != std::end(formats);
        }

        /**
        * @brief Returns true if the attachment has a stencil component
        */
        bool hasStencil()
        {
            std::vector<VkFormat> formats =
                {
                    VK_FORMAT_S8_UINT,
                    VK_FORMAT_D16_UNORM_S8_UINT,
                    VK_FORMAT_D24_UNORM_S8_UINT,
                    VK_FORMAT_D32_SFLOAT_S8_UINT,
                };
            return std::find(formats.begin(), formats.end(), format) != std::end(formats);
        }

        /**
        * @brief Returns true if the attachment is a depth and/or stencil attachment
        */
        bool isDepthStencil()
        {
            return(hasDepth() || hasStencil());
        }

    };

    std::vector<FramebufferAttachment> attachments;

protected:
    enum FramebufferStatus { VK_FRAMEBUFFER_COMPLETE } _status;
    virtual void update();
    bool checkStatus(FramebufferStatus target) const;
    VkResult createFramebuffer();
    struct VKAttachmentCreateInfo
    {
        uint32_t width, height;
        uint32_t layerCount;
        VkFormat format;
        VkImageUsageFlags usage;
        VkSampleCountFlagBits imageSampleCount = VK_SAMPLE_COUNT_1_BIT;
    };
    uint32_t addAttachment(VKAttachmentCreateInfo createinfo, VkImage image);

    // VKTODO: We need a check on backend.lock(), or to pass backend reference instead
    VKFramebuffer(const std::weak_ptr<vk::VKBackend>& backend, const Framebuffer& framebuffer) : VKObject(*backend.lock(), framebuffer) {}
        //vksFrameBuffer(backend.lock()->getContext().device.get()){}
    // VKTODO: Do we need virtual destructor here?
    ~VKFramebuffer();

};

} }


#endif
