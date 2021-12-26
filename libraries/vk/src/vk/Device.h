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

/*        template <typename T>
        void destroy(T& t, OptionalAllocationCallbacks allocator = nullptr) const {
#if __cplusplus > 201703L
            static_assert(false);
#else
            assert(false);
#endif
        }*/


        void destroy(vk::Fence& object, OptionalAllocationCallbacks allocator) const {
            destroyFence(object, allocator);
            object = vk::Fence();
        }

        void destroy(vk::Semaphore& object, OptionalAllocationCallbacks allocator) const {
            destroySemaphore(object, allocator);
            object = vk::Semaphore();
        }

        void destroy(vk::Event& object, OptionalAllocationCallbacks allocator) const {
            destroyEvent(object, allocator);
            object = vk::Event();
        }

        void destroy(vk::QueryPool& object, OptionalAllocationCallbacks allocator) const {
            destroyQueryPool(object, allocator);
            object = vk::QueryPool();
        }

        void destroy(vk::Buffer& object, OptionalAllocationCallbacks allocator) const {
            destroyBuffer(object, allocator);
            object = vk::Buffer();
        }

        void destroy(vk::BufferView& object, OptionalAllocationCallbacks allocator) const {
            destroyBufferView(object, allocator);
            object = vk::BufferView();
        }

        void destroy(vk::Image& object, OptionalAllocationCallbacks allocator) const {
            destroyImage(object, allocator);
            object = vk::Image();
        }
        
        void destroy(vk::ImageView& object, OptionalAllocationCallbacks allocator) const {
            destroyImageView(object, allocator);
            object = vk::ImageView();
        }

        void destroy(vk::ShaderModule& object, OptionalAllocationCallbacks allocator) const {
            destroyShaderModule(object, allocator);
            object = vk::ShaderModule();
        }

        void destroy(vk::PipelineCache& object, OptionalAllocationCallbacks allocator) const {
            destroyPipelineCache(object, allocator);
            object = vk::PipelineCache();
        }

        void destroy(vk::Pipeline& object, OptionalAllocationCallbacks allocator) const {
            destroyPipeline(object, allocator);
            object = vk::Pipeline();
        }

        void destroy(vk::PipelineLayout& object, OptionalAllocationCallbacks allocator) const {
            destroyPipelineLayout(object, allocator);
            object = vk::PipelineLayout();
        }

        void destroy(vk::Sampler& object, OptionalAllocationCallbacks allocator) const {
            destroySampler(object, allocator);
            object = vk::Sampler();
        }

        void destroy(vk::DescriptorSetLayout& object, OptionalAllocationCallbacks allocator) const {
            destroyDescriptorSetLayout(object, allocator);
            object = vk::DescriptorSetLayout();
        }

        void destroy(vk::DescriptorPool& object, OptionalAllocationCallbacks allocator) const {
            destroyDescriptorPool(object, allocator);
            object = vk::DescriptorPool();
        }

        void destroy(vk::Framebuffer& object, OptionalAllocationCallbacks allocator) const {
            destroyFramebuffer(object, allocator);
            object = vk::Framebuffer();
        }

        void destroy(vk::RenderPass& object, OptionalAllocationCallbacks allocator) const {
            destroyRenderPass(object, allocator);
            object = vk::RenderPass();
        }
        
        void destroy(vk::CommandPool& object, OptionalAllocationCallbacks allocator) const {
            destroyCommandPool(object, allocator);
            object = vk::CommandPool();
        }

        void destroy(vk::SwapchainKHR& object, OptionalAllocationCallbacks allocator) const {
            destroySwapchainKHR(object, allocator);
            object = vk::SwapchainKHR();
        }

        void destroy(vk::IndirectCommandsLayoutNV& object, OptionalAllocationCallbacks allocator) const {
            destroyIndirectCommandsLayoutNV(object, allocator);
            object = vk::IndirectCommandsLayoutNV();
        }

        //void destroy(vk::ObjectTableNVX& object, OptionalAllocationCallbacks allocator) const {
            //destroyObjectTableNVX(object, allocator);
            //object = vk::ObjectTableNVX();
        //}

        void destroy(vk::DescriptorUpdateTemplateKHR& object, OptionalAllocationCallbacks allocator) const {
            destroyDescriptorUpdateTemplateKHR(object, allocator);
            object = vk::DescriptorUpdateTemplateKHR();
        }

        void destroy(vk::SamplerYcbcrConversionKHR& object, OptionalAllocationCallbacks allocator) const {
            destroySamplerYcbcrConversionKHR(object, allocator);
            object = vk::SamplerYcbcrConversionKHR();
        }

        void destroy(vk::ValidationCacheEXT& object, OptionalAllocationCallbacks allocator) const {
            destroyValidationCacheEXT(object, allocator);
            object = vk::ValidationCacheEXT();
        }
    };
}
