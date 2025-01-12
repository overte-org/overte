//
//  Created by Bradley Austin Davis on 2016/08/07
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "VKFramebuffer.h"
#include "VKBackend.h"
#include "VKTexture.h"
#include "VKShared.h"

void gpu::vk::VKFramebuffer::update() {
    auto backend = _backend.lock();
    VkDevice device = backend->getContext().device->logicalDevice;
    // VKTODO: this is wrong, most of framebuffer code will need to be rewritten
    if (vkFramebuffer != VK_NULL_HANDLE) {
        vkDestroyFramebuffer(device, vkFramebuffer, nullptr);
    }
    // VKTODO: free all attachments too
    VKTexture* vkTexture = nullptr;
    TexturePointer surface;
    bool lastTextureWasNull = false;
    if (_gpuObject.getColorStamps() != _colorStamps) {
        if (_gpuObject.hasColor()) {
            // VKTODO: Do these need to be deleted?
            attachments.clear();
            /*static const GLenum colorAttachments[] = {
                GL_COLOR_ATTACHMENT0,
                GL_COLOR_ATTACHMENT1,
                GL_COLOR_ATTACHMENT2,
                GL_COLOR_ATTACHMENT3,
                GL_COLOR_ATTACHMENT4,
                GL_COLOR_ATTACHMENT5,
                GL_COLOR_ATTACHMENT6,
                GL_COLOR_ATTACHMENT7,
                GL_COLOR_ATTACHMENT8,
                GL_COLOR_ATTACHMENT9,
                GL_COLOR_ATTACHMENT10,
                GL_COLOR_ATTACHMENT11,
                GL_COLOR_ATTACHMENT12,
                GL_COLOR_ATTACHMENT13,
                GL_COLOR_ATTACHMENT14,
                GL_COLOR_ATTACHMENT15 };*/

            //int unit = 0;
            for (auto& b : _gpuObject.getRenderBuffers()) {
                surface = b._texture;
                if (surface) {
                    Q_ASSERT(TextureUsageType::RENDERBUFFER == surface->getUsageType());
                    vkTexture = backend->syncGPUObject(*surface.get());
                } else {
                    vkTexture = nullptr;
                }

                if (vkTexture) {
                    if (lastTextureWasNull) {
                        Q_ASSERT(false);
                    }
                    if (vkTexture->_target == VK_IMAGE_VIEW_TYPE_2D) {
                        VKAttachmentCreateInfo attachmentCI {};
                        attachmentCI.width = vkTexture->_gpuObject.getWidth();
                        attachmentCI.height = vkTexture->_gpuObject.getHeight();
                        attachmentCI.layerCount = 1;
                        attachmentCI.format = gpu::vk::evalTexelFormatInternal(vkTexture->_gpuObject.getTexelFormat());
                        attachmentCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
                        attachmentCI.imageSampleCount = VK_SAMPLE_COUNT_1_BIT;
                        addAttachment(attachmentCI, vkTexture->_vkImage);
                        //glNamedFramebufferTexture(_id, colorAttachments[unit], gltexture->_texture, 0);
                        // VKTODO: how to do this?
                    /*} else if (vkTexture->_target == GL_TEXTURE_2D_MULTISAMPLE) {
                        glNamedFramebufferTexture(_id, colorAttachments[unit], gltexture->_texture, 0);*/
                    } else {
                        // VKTODO: what is subresource?
                        Q_ASSERT(false);
                        //glNamedFramebufferTextureLayer(_id, colorAttachments[unit], gltexture->_texture, 0, b._subresource);
                    }
                    //_colorBuffers.push_back(unit);
                } else {
                    lastTextureWasNull = true;
                    // VKTODO: what to do here?
                    //glNamedFramebufferTexture(_id, colorAttachments[unit], 0, 0);
                }
                //unit++;
            }
        }
        _colorStamps = _gpuObject.getColorStamps();
    }

    /*GLenum attachement = GL_DEPTH_STENCIL_ATTACHMENT;
    if (!_gpuObject.hasStencil()) {
        attachement = GL_DEPTH_ATTACHMENT;
    } else if (!_gpuObject.hasDepth()) {
        attachement = GL_STENCIL_ATTACHMENT;
    }*/

    if (_gpuObject.getDepthStamp() != _depthStamp) {
        auto surface = _gpuObject.getDepthStencilBuffer();
        auto backend = _backend.lock();
        if (_gpuObject.hasDepthStencil() && surface) {
            Q_ASSERT(TextureUsageType::RENDERBUFFER == surface->getUsageType());
            vkTexture = backend->syncGPUObject(*surface.get());
        }

        if (vkTexture) {
            if (vkTexture->_target == VK_IMAGE_VIEW_TYPE_2D) {
                VKAttachmentCreateInfo attachmentCI {};
                attachmentCI.width = vkTexture->_gpuObject.getWidth();
                attachmentCI.height = vkTexture->_gpuObject.getHeight();
                attachmentCI.layerCount = 1;
                attachmentCI.format = gpu::vk::evalTexelFormatInternal(vkTexture->_gpuObject.getTexelFormat());
                attachmentCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                attachmentCI.imageSampleCount = VK_SAMPLE_COUNT_1_BIT;
                addAttachment(attachmentCI, vkTexture->_vkImage);
                //glNamedFramebufferTexture(_id, attachement, gltexture->_texture, 0);
                // VKTODO
            /*}
            else if (vkTexture->_target == GL_TEXTURE_2D_MULTISAMPLE) {
                glNamedFramebufferTexture(_id, attachement, gltexture->_texture, 0);*/
            } else {
                Q_ASSERT(false);
                // VKTODO
                //glNamedFramebufferTextureLayer(_id, attachement, gltexture->_texture, 0,
                //                               _gpuObject.getDepthStencilBufferSubresource());
            }
        } else {
            Q_ASSERT(false);
            // VKTODO
            //glNamedFramebufferTexture(_id, attachement, 0, 0);
        }
        _depthStamp = _gpuObject.getDepthStamp();
    }

    // Last but not least, define where we draw
    if (!attachments.empty()) {
        VK_CHECK_RESULT(createFramebuffer());
        //glNamedFramebufferDrawBuffers(_id, (GLsizei)_colorBuffers.size(), _colorBuffers.data());
    } else {
        Q_ASSERT(false);
        // VKTODO
        //glNamedFramebufferDrawBuffer(_id, GL_NONE);
    }

    // Now check for completness
    //_status = glCheckNamedFramebufferStatus(_id, GL_DRAW_FRAMEBUFFER);

    // restore the current framebuffer
    //checkStatus();
}

// From VKS
VkResult gpu::vk::VKFramebuffer::createFramebuffer()
{
    std::vector<VkAttachmentDescription> attachmentDescriptions;
    for (auto& attachment : attachments)
    {
        attachmentDescriptions.push_back(attachment.description);
    };

    // Collect attachment references
    std::vector<VkAttachmentReference> colorReferences;
    VkAttachmentReference depthReference = {};
    bool hasDepth = false;
    bool hasColor = false;

    uint32_t attachmentIndex = 0;

    for (auto& attachment : attachments)
    {
        if (attachment.isDepthStencil())
        {
            // Only one depth attachment allowed
            assert(!hasDepth);
            depthReference.attachment = attachmentIndex;
            depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            hasDepth = true;
        }
        else
        {
            colorReferences.push_back({ attachmentIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
            hasColor = true;
        }
        attachmentIndex++;
    };

    // Default render pass setup uses only one subpass
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    if (hasColor)
    {
        subpass.pColorAttachments = colorReferences.data();
        subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
    }
    if (hasDepth)
    {
        subpass.pDepthStencilAttachment = &depthReference;
    }

    // Use subpass dependencies for attachment layout transitions
    std::array<VkSubpassDependency, 2> dependencies;

    // VKTODO: what are these for?
    /*dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;*/

    // Create render pass
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pAttachments = attachmentDescriptions.data();
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    // VKTODO
    //renderPassInfo.dependencyCount = 2;
    //renderPassInfo.pDependencies = dependencies.data();
    renderPassInfo.dependencyCount = 0;
    VK_CHECK_RESULT(vkCreateRenderPass(_backend.lock()->_context.device->logicalDevice, &renderPassInfo, nullptr, &vkRenderPass));

    std::vector<VkImageView> attachmentViews;
    for (auto attachment : attachments)
    {
        attachmentViews.push_back(attachment.view);
    }

    // Find. max number of layers across attachments
    uint32_t maxLayers = 0;
    for (auto attachment : attachments)
    {
        if (attachment.subresourceRange.layerCount > maxLayers)
        {
            maxLayers = attachment.subresourceRange.layerCount;
        }
    }

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = vkRenderPass;
    framebufferInfo.pAttachments = attachmentViews.data();
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
    framebufferInfo.width = _gpuObject.getWidth();
    framebufferInfo.height = _gpuObject.getHeight();
    framebufferInfo.layers = maxLayers;
    VK_CHECK_RESULT(vkCreateFramebuffer(_backend.lock()->_context.device->logicalDevice, &framebufferInfo, nullptr, &vkFramebuffer));

    return VK_SUCCESS;
}

//bool gpu::vk::VKFramebuffer::checkStatus(gpu::vk::VKFramebuffer::FramebufferStatus target) const {
    // VKTODO
    /*switch (_status) {
        case GL_FRAMEBUFFER_COMPLETE:
            // Success !
            return true;

        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            qCWarning(gpugllogging) << "GLFramebuffer::syncGPUObject : Framebuffer not valid, GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT.";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            qCWarning(gpugllogging) << "GLFramebuffer::syncGPUObject : Framebuffer not valid, GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT.";
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            qCWarning(gpugllogging) << "GLFramebuffer::syncGPUObject : Framebuffer not valid, GL_FRAMEBUFFER_UNSUPPORTED.";
            break;
#if !defined(USE_GLES)
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            qCWarning(gpugllogging) << "GLFramebuffer::syncGPUObject : Framebuffer not valid, GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER.";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            qCWarning(gpugllogging) << "GLFramebuffer::syncGPUObject : Framebuffer not valid, GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER.";
            break;
#endif
        default:
            break;
    }
    return false;
*/
//}

gpu::vk::VKFramebuffer::~VKFramebuffer() {
    auto backend = _backend.lock();
    auto &recycler = backend->getContext().recycler; // VKTODO: these sometimes get destroyed after backend was destroyed?
    recycler.framebufferDeleted(this);
    //VKTODO
    /*if (_id) {
        auto backend = _backend.lock();
        if (backend) {
            backend->releaseFramebuffer(_id);
        }
    }*/
}

// From VKS
uint32_t gpu::vk::VKFramebuffer::addAttachment(VKAttachmentCreateInfo createinfo, VkImage image)
{
    FramebufferAttachment attachment {};

    attachment.format = createinfo.format;

    VkImageAspectFlags aspectMask = VK_FLAGS_NONE;

    // Select aspect mask and layout depending on usage

    // Color attachment
    if (createinfo.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
    {
        Q_ASSERT(attachment.format != VK_FORMAT_D24_UNORM_S8_UINT);
        aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    // Depth (and/or stencil) attachment
    if (createinfo.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        if (attachment.hasDepth())
        {
            aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        }
        if (attachment.hasStencil())
        {
            aspectMask = aspectMask | VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }

    assert(aspectMask > 0);

    VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
    VkMemoryRequirements memReqs;

    attachment.image = image;

    attachment.subresourceRange = {};
    attachment.subresourceRange.aspectMask = aspectMask;
    attachment.subresourceRange.levelCount = 1;
    attachment.subresourceRange.layerCount = createinfo.layerCount;

    VkImageViewCreateInfo imageView = vks::initializers::imageViewCreateInfo();
    imageView.viewType = (createinfo.layerCount == 1) ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    imageView.format = createinfo.format;
    imageView.subresourceRange = attachment.subresourceRange;
    imageView.subresourceRange.aspectMask = (attachment.hasDepth()) ? VK_IMAGE_ASPECT_DEPTH_BIT : aspectMask;
    imageView.image = attachment.image;
    VK_CHECK_RESULT(vkCreateImageView(_backend.lock()->_context.device->logicalDevice, &imageView, nullptr, &attachment.view));

    // Fill attachment description
    attachment.description = {};
    attachment.description.samples = createinfo.imageSampleCount;
    attachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.description.storeOp = (createinfo.usage & VK_IMAGE_USAGE_SAMPLED_BIT) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.description.format = createinfo.format;
    attachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // Final layout
    // If not, final layout depends on attachment type
    if (attachment.hasDepth() || attachment.hasStencil())
    {
        //attachment.description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        attachment.description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // VKTODO: this is tricky, because it depends on what the image will be used for
    }
    else
    {
        //attachment.description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        attachment.description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // VKTODO: this is tricky, because it depends on what the image will be used for
    }

    attachments.push_back(attachment);

    return static_cast<uint32_t>(attachments.size() - 1);
}

// VKTODO: get rid of _backend.lock()

#if 0

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