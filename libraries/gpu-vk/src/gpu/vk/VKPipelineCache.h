#pragma once

// For hash_combine
#include <RegisteredMetaTypes.h>

#include <gpu/Forward.h>
#include <gpu/Context.h>

#include <vk/Config.h>
#include <vk/Context.h>
#include <vk/VulkanDebug.h>
#include <vulkan/vulkan_core.h>

#include "VKForward.h"

//define OVERTE_VK_PIPELINE_DEBUG

namespace std {
template <>
struct hash<gpu::Element> {
    size_t operator()(const gpu::Element& a) const { return std::hash<uint32_t>()(a.getRaw()); }
};

template <>
struct hash<::VkImageLayout> {
    size_t operator()(const VkImageLayout& a) const { return std::hash<uint32_t>()((uint32_t)a); }
};



}  // namespace std
namespace gpu { namespace vk {

inline size_t hashRenderPassPair(const std::pair<VkFormat,VkImageLayout>& a) {
    size_t seed = 0;
    std::hash_combine(seed, a.first);
    std::hash_combine(seed, a.second);
    return seed;
}

template <typename Container>
struct RenderPassHash {
    std::size_t operator()(const Container& c) const {
        size_t seed = 0;
        for (const auto& e : c) {
            std::hash_combine(seed, hashRenderPassPair(e));
        }
        return seed;
    }
};

struct Cache {
    struct PipelineLayout {
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;
        VkDescriptorSetLayout uniformLayout;
        VkDescriptorSetLayout textureLayout;
        VkDescriptorSetLayout storageLayout;
#ifdef OVERTE_VK_PIPELINE_DEBUG
        std::string vertexShader;
        std::string fragmentShader;
#endif
    };
    std::unordered_map<uint32_t, VkShaderModule> moduleMap;
    std::unordered_map<std::string, PipelineLayout> pipelineMap;

    struct Pipeline {
        using RenderpassKey = std::vector<std::pair<VkFormat, VkImageLayout>>;
        using BindingMap = std::unordered_map<uint32_t, VkShaderStageFlags>;
        using LocationMap = shader::Reflection::LocationMap;

        gpu::PipelineReference pipeline{ GPU_REFERENCE_INIT_VALUE };
        gpu::FormatReference format{ GPU_REFERENCE_INIT_VALUE };
        gpu::FramebufferReference framebuffer{ GPU_REFERENCE_INIT_VALUE };
        gpu::Primitive primitiveTopology;


        // VKTODO: maybe these should be moved from here to cache object?
        std::unordered_map<RenderpassKey, VkRenderPass, RenderPassHash<std::vector<std::pair<VkFormat, VkImageLayout>>>> _renderPassMap; // VKTODO: make sure render passes are retrieved from here

        // These get set when stride gets set by setInputBuffer
        std::array<Offset, MAX_NUM_INPUT_BUFFERS> _bufferStrides{ 0 };
        std::bitset<MAX_NUM_INPUT_BUFFERS> _bufferStrideSet;

        void clearStrides() { _bufferStrideSet.reset(); }

        void setPipeline(const gpu::PipelinePointer& pipeline);

        void setVertexFormat(const gpu::Stream::FormatPointer& format);

        void setFramebuffer(const gpu::FramebufferPointer& framebuffer);

        static void updateBindingMap(BindingMap& bindingMap,
                                     const LocationMap& locationMap,
                                     VkShaderStageFlagBits shaderStage);

        static void setBindingMap(BindingMap& bindingMap, const LocationMap& vertexMap, const LocationMap& fragmentMap);

        static BindingMap getBindingMap(const LocationMap& vertexMap, const LocationMap& fragmentMap);

        // VKTODO: This needs to be used instead of getPipeline
        // Returns structure containing pipeline layout and descriptor set layouts
        PipelineLayout getPipelineAndDescriptorLayout(const vks::Context& context);

        RenderpassKey getRenderPassKey(gpu::Framebuffer* framebuffer) const;

        VkRenderPass getRenderPass(const vks::Context& context);
        static std::string getRenderpassKeyString(const RenderpassKey& renderpassKey);
        std::string getStridesKey() const;
        std::string getKey() const;
    } pipelineState;

    static VkStencilOpState getStencilOp(const gpu::State::StencilTest& stencil);

    VkShaderModule getShaderModule(const vks::Context& context, const shader::Source& source);

    PipelineLayout getPipeline(const vks::Context& context);
};

} }