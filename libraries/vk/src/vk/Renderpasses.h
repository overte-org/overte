#pragma once

#include "Config.h"

namespace vks { namespace renderpasses {

    struct AttachmentDescription : public vk::AttachmentDescription {
        AttachmentDescription& withFlags(vk::AttachmentDescriptionFlags flags) {
            this->flags = flags; return *this;
        }
        AttachmentDescription& withFormat(vk::Format format) {
            this->format = format; return *this;
        }
        AttachmentDescription& withInitialLayout(vk::ImageLayout layout) {
            this->initialLayout = layout; return *this;
        }
        AttachmentDescription& withFinalLayout(vk::ImageLayout layout) {
            this->finalLayout = layout; return *this;
        }
        AttachmentDescription& withSampleCount(vk::SampleCountFlagBits samples) {
            this->samples = samples; return *this;
        }
        AttachmentDescription& withLoadOp(vk::AttachmentLoadOp loadOp) {
            this->loadOp = loadOp; return *this;
        }
        AttachmentDescription& withStoreOp(vk::AttachmentStoreOp storeOp) {
            this->storeOp = storeOp; return *this;
        }
        AttachmentDescription& withLStenciloadOp(vk::AttachmentLoadOp loadOp) {
            this->stencilLoadOp = loadOp; return *this;
        }
        AttachmentDescription& withStencilStoreOp(vk::AttachmentStoreOp storeOp) {
            this->stencilStoreOp = storeOp; return *this;
        }
    };

    struct SubpassDescription : public vk::SubpassDescription {
        std::vector<vk::AttachmentReference> colorAttachments;
        std::vector<vk::AttachmentReference> inputAttachments;
        std::vector<vk::AttachmentReference> resolveAttachments;
        vk::AttachmentReference depthStencilAttachment;
        std::vector<uint32_t> preserveAttachments;

        void update() {
            this->colorAttachmentCount = (uint32_t)colorAttachments.size();
            this->pColorAttachments = colorAttachments.data();
            this->inputAttachmentCount = (uint32_t)inputAttachments.size();
            this->pInputAttachments = inputAttachments.data();
            this->pResolveAttachments = resolveAttachments.data();
            this->pDepthStencilAttachment = &depthStencilAttachment;
            this->preserveAttachmentCount = (uint32_t)preserveAttachments.size();
            this->pPreserveAttachments = preserveAttachments.data();
        }
    };

    struct RenderPassBuilder {
        std::vector<vk::AttachmentDescription> attachments;
        std::vector<vk::SubpassDependency> subpassDependencies;
        std::vector<SubpassDescription> subpasses;

        size_t addAttachment(const vk::AttachmentDescription& attachment) {
            attachments.push_back(attachment);
            return attachments.size() - 1;
        }

        vk::RenderPass build(const vk::Device& device) {
            for (auto& subpass : subpasses) {
                subpass.update();
            }

            vk::RenderPassCreateInfo renderPassInfo;
            renderPassInfo.attachmentCount = (uint32_t)attachments.size();
            renderPassInfo.pAttachments = attachments.data();
            renderPassInfo.subpassCount = (uint32_t)subpasses.size();
            renderPassInfo.pSubpasses = subpasses.data();
            renderPassInfo.dependencyCount = (uint32_t)subpassDependencies.size();
            renderPassInfo.pDependencies = subpassDependencies.data();
            return device.createRenderPass(renderPassInfo);
        }
    };

#if 0
subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
attachments.resize(colorFormats.size());
colorAttachmentReferences.resize(attachments.size());
// Color attachment
for (uint32_t i = 0; i < attachments.size(); ++i) {
    attachments[i].format = colorFormats[i];
    attachments[i].loadOp = vk::AttachmentLoadOp::eClear;
    attachments[i].storeOp = colorFinalLayout == vk::ImageLayout::eColorAttachmentOptimal ? vk::AttachmentStoreOp::eDontCare : vk::AttachmentStoreOp::eStore;
    attachments[i].initialLayout = vk::ImageLayout::eUndefined;
    attachments[i].finalLayout = colorFinalLayout;

    vk::AttachmentReference& attachmentReference = colorAttachmentReferences[i];
    attachmentReference.attachment = i;
    attachmentReference.layout = vk::ImageLayout::eColorAttachmentOptimal;

    subpass.colorAttachmentCount = (uint32_t)colorAttachmentReferences.size();
    subpass.pColorAttachments = colorAttachmentReferences.data();
}

// Do we have a depth format?
vk::AttachmentReference depthAttachmentReference;
if (depthFormat != vk::Format::eUndefined) {
    vk::AttachmentDescription depthAttachment;
    depthAttachment.format = depthFormat;
    depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    // We might be using the depth attacment for something, so preserve it if it's final layout is not undefined
    depthAttachment.storeOp =
        depthFinalLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal ? vk::AttachmentStoreOp::eDontCare : vk::AttachmentStoreOp::eStore;
    depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
    depthAttachment.finalLayout = depthFinalLayout;
    attachments.push_back(depthAttachment);
    depthAttachmentReference.attachment = (uint32_t)attachments.size() - 1;
    depthAttachmentReference.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    subpass.pDepthStencilAttachment = &depthAttachmentReference;
}

{
    if ((colorFinalLayout != vk::ImageLayout::eColorAttachmentOptimal) && (colorFinalLayout != vk::ImageLayout::eUndefined)) {
        // Implicit transition
        vk::SubpassDependency dependency;
        dependency.srcSubpass = 0;
        dependency.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;

        dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstAccessMask = vks::util::accessFlagsForLayout(colorFinalLayout);
        dependency.dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
        subpassDependencies.push_back(dependency);
    }

    if ((depthFinalLayout != vk::ImageLayout::eColorAttachmentOptimal) && (depthFinalLayout != vk::ImageLayout::eUndefined)) {
        // Implicit transition
        vk::SubpassDependency dependency;
        dependency.srcSubpass = 0;
        dependency.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        dependency.srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;

        dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstAccessMask = vks::util::accessFlagsForLayout(depthFinalLayout);
        dependency.dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
        subpassDependencies.push_back(dependency);
    }
#endif

}}  // namespace vks::renderpasses
