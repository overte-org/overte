#pragma once

#include "Config.h"

namespace vks {

    struct Device : public vk::Device {
        using OptionalAllocationCallbacks = vk::Optional<const vk::AllocationCallbacks>;

        using vk::Device::destroy;

        Device& operator=(const vk::Device& device) {
            (vk::Device&)(*this) = device;
            return *this;
        }

        template <typename T>
        void destroy(T& t, OptionalAllocationCallbacks allocator = nullptr) const {
#if __cplusplus > 201703L
            static_assert(false);
#else
            assert(false);
#endif
        }


        template <>
        void destroy<vk::Fence>(vk::Fence& object, OptionalAllocationCallbacks allocator) const {
            destroyFence(object, allocator);
            object = vk::Fence();
        }

        template <>
        void destroy<vk::Semaphore>(vk::Semaphore& object, OptionalAllocationCallbacks allocator) const {
            destroySemaphore(object, allocator);
            object = vk::Semaphore();
        }

        template <>
        void destroy<vk::Event>(vk::Event& object, OptionalAllocationCallbacks allocator) const {
            destroyEvent(object, allocator);
            object = vk::Event();
        }

        template <>
        void destroy<vk::QueryPool>(vk::QueryPool& object, OptionalAllocationCallbacks allocator) const {
            destroyQueryPool(object, allocator);
            object = vk::QueryPool();
        }

        template <>
        void destroy<vk::Buffer>(vk::Buffer& object, OptionalAllocationCallbacks allocator) const {
            destroyBuffer(object, allocator);
            object = vk::Buffer();
        }

        template <>
        void destroy<vk::BufferView>(vk::BufferView& object, OptionalAllocationCallbacks allocator) const {
            destroyBufferView(object, allocator);
            object = vk::BufferView();
        }

        template <>
        void destroy<vk::Image>(vk::Image& object, OptionalAllocationCallbacks allocator) const {
            destroyImage(object, allocator);
            object = vk::Image();
        }
        
        template <>
        void destroy<vk::ImageView>(vk::ImageView& object, OptionalAllocationCallbacks allocator) const {
            destroyImageView(object, allocator);
            object = vk::ImageView();
        }

        template <>
        void destroy<vk::ShaderModule>(vk::ShaderModule& object, OptionalAllocationCallbacks allocator) const {
            destroyShaderModule(object, allocator);
            object = vk::ShaderModule();
        }

        template <>
        void destroy<vk::PipelineCache>(vk::PipelineCache& object, OptionalAllocationCallbacks allocator) const {
            destroyPipelineCache(object, allocator);
            object = vk::PipelineCache();
        }

        template <>
        void destroy<vk::Pipeline>(vk::Pipeline& object, OptionalAllocationCallbacks allocator) const {
            destroyPipeline(object, allocator);
            object = vk::Pipeline();
        }

        template <>
        void destroy<vk::PipelineLayout>(vk::PipelineLayout& object, OptionalAllocationCallbacks allocator) const {
            destroyPipelineLayout(object, allocator);
            object = vk::PipelineLayout();
        }

        template <>
        void destroy<vk::Sampler>(vk::Sampler& object, OptionalAllocationCallbacks allocator) const {
            destroySampler(object, allocator);
            object = vk::Sampler();
        }

        template <>
        void destroy<vk::DescriptorSetLayout>(vk::DescriptorSetLayout& object, OptionalAllocationCallbacks allocator) const {
            destroyDescriptorSetLayout(object, allocator);
            object = vk::DescriptorSetLayout();
        }

        template <>
        void destroy<vk::DescriptorPool>(vk::DescriptorPool& object, OptionalAllocationCallbacks allocator) const {
            destroyDescriptorPool(object, allocator);
            object = vk::DescriptorPool();
        }

        template <>
        void destroy<vk::Framebuffer>(vk::Framebuffer& object, OptionalAllocationCallbacks allocator) const {
            destroyFramebuffer(object, allocator);
            object = vk::Framebuffer();
        }

        template <>
        void destroy<vk::RenderPass>(vk::RenderPass& object, OptionalAllocationCallbacks allocator) const {
            destroyRenderPass(object, allocator);
            object = vk::RenderPass();
        }
        
        template <>
        void destroy<vk::CommandPool>(vk::CommandPool& object, OptionalAllocationCallbacks allocator) const {
            destroyCommandPool(object, allocator);
            object = vk::CommandPool();
        }

        template <>
        void destroy<vk::SwapchainKHR>(vk::SwapchainKHR& object, OptionalAllocationCallbacks allocator) const {
            destroySwapchainKHR(object, allocator);
            object = vk::SwapchainKHR();
        }

        template <>
        void destroy<vk::IndirectCommandsLayoutNVX>(vk::IndirectCommandsLayoutNVX& object, OptionalAllocationCallbacks allocator) const {
            destroyIndirectCommandsLayoutNVX(object, allocator);
            object = vk::IndirectCommandsLayoutNVX();
        }

        template <>
        void destroy<vk::ObjectTableNVX>(vk::ObjectTableNVX& object, OptionalAllocationCallbacks allocator) const {
            destroyObjectTableNVX(object, allocator);
            object = vk::ObjectTableNVX();
        }

        template <>
        void destroy<vk::DescriptorUpdateTemplateKHR>(vk::DescriptorUpdateTemplateKHR& object, OptionalAllocationCallbacks allocator) const {
            destroyDescriptorUpdateTemplateKHR(object, allocator);
            object = vk::DescriptorUpdateTemplateKHR();
        }

        template <>
        void destroy<vk::SamplerYcbcrConversionKHR>(vk::SamplerYcbcrConversionKHR& object, OptionalAllocationCallbacks allocator) const {
            destroySamplerYcbcrConversionKHR(object, allocator);
            object = vk::SamplerYcbcrConversionKHR();
        }

        template <>
        void destroy<vk::ValidationCacheEXT>(vk::ValidationCacheEXT& object, OptionalAllocationCallbacks allocator) const {
            destroyValidationCacheEXT(object, allocator);
            object = vk::ValidationCacheEXT();
        }
    };
}
