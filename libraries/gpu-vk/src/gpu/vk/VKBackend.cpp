//
//  Created by Bradley Austin Davis on 2016/08/07
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "VKBackend.h"

#include <mutex>
#include <queue>
#include <list>
#include <functional>
#include <iomanip>
#include <glm/gtc/type_ptr.hpp>

#include <QtCore/QProcessEnvironment>

// For hash_combine
#include <RegisteredMetaTypes.h>

#include <vk/Helpers.h>
#include <vk/Version.h>
#include <vk/Pipelines.h>

#include "VKForward.h"
#include "VKShared.h"

using namespace gpu;
using namespace gpu::vulkan;

static VKBackend* INSTANCE{ nullptr };
static const char* VK_BACKEND_PROPERTY_NAME = "com.highfidelity.vk.backend";
static bool enableDebugMarkers = false;

namespace std {
template <>
struct hash<gpu::Element> {
    size_t operator()(const gpu::Element& a) const { return std::hash<uint16>()(a.getRaw()); }
};
}  // namespace std

template <typename Container>  // we can make this generic for any container [1]
struct container_hash {
    std::size_t operator()(const Container& c) const {
        size_t seed = 0;
        for (const auto& e : c) {
            std::hash_combine(seed, e);
        }
        return seed;
    }
};

BackendPointer VKBackend::createBackend() {
    // FIXME provide a mechanism to override the backend for testing
    // Where the gpuContext is initialized and where the TRUE Backend is created and assigned
    std::shared_ptr<VKBackend> result = std::make_shared<VKBackend>();
    INSTANCE = result.get();
    void* voidInstance = &(*result);
    qApp->setProperty(VK_BACKEND_PROPERTY_NAME, QVariant::fromValue(voidInstance));
    return result;
}

VKBackend& getBackend() {
    if (!INSTANCE) {
        INSTANCE = static_cast<VKBackend*>(qApp->property(VK_BACKEND_PROPERTY_NAME).value<void*>());
    }
    return *INSTANCE;
}

void VKBackend::init() {
}

VKBackend::VKBackend() {
    if (!_context.instance) {
        _context.createInstance();
        _context.createDevice();
    }

    {
        VkPipelineCacheCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        std::vector<uint8_t> pipelineCacheData;
        if (vks::util::loadPipelineCacheData(pipelineCacheData) && !pipelineCacheData.empty()) {
            createInfo.pInitialData = pipelineCacheData.data();
            createInfo.initialDataSize = pipelineCacheData.size();
        }

        vkCreatePipelineCache(_context.device->logicalDevice, &createInfo, nullptr, &_pipelineCache);
    }

    // Get the graphics queue
    qCDebug(gpu_vk_logging) << "VK Version:     " << vks::Version(_context.device->properties.apiVersion).toString().c_str();
    qCDebug(gpu_vk_logging) << "VK Driver:      " << vks::Version(_context.device->properties.driverVersion).toString().c_str();
    qCDebug(gpu_vk_logging) << "VK Vendor ID:   " << _context.device->properties.vendorID;
    qCDebug(gpu_vk_logging) << "VK Device ID:   " << _context.device->properties.deviceID;
    qCDebug(gpu_vk_logging) << "VK Device Name: " << _context.device->properties.deviceName;
    qCDebug(gpu_vk_logging) << "VK Device Type: " << _context.device->properties.deviceType;
}

VKBackend::~VKBackend() {
    // FIXME queue up all the trash calls
    VK_CHECK_RESULT(vkQueueWaitIdle(_graphicsQueue));
    VK_CHECK_RESULT(vkQueueWaitIdle(_transferQueue));
    VK_CHECK_RESULT(vkDeviceWaitIdle(_context.device->logicalDevice));

    {
        size_t pipelineCacheDataSize;
        VK_CHECK_RESULT(vkGetPipelineCacheData(_context.device->logicalDevice, _pipelineCache, &pipelineCacheDataSize, nullptr));
        std::vector<uint8_t> pipelineCacheData;
        pipelineCacheData.resize(pipelineCacheDataSize);
        VK_CHECK_RESULT(vkGetPipelineCacheData(_context.device->logicalDevice, _pipelineCache, &pipelineCacheDataSize, nullptr));
        if (!pipelineCacheData.empty()) {
            vks::util::savePipelineCacheData(pipelineCacheData);
        }
    }

    _context.destroyContext();
}

const std::string& VKBackend::getVersion() const {
    static const std::string VERSION{ "VK1.1" };
    return VERSION;
}

bool VKBackend::isTextureManagementSparseEnabled() const {
    return _context.device->features.sparseResidencyImage2D == VK_TRUE;
}

bool VKBackend::supportedTextureFormat(const gpu::Element& format) const {
    switch (format.getSemantic()) {
        case COMPRESSED_BC1_SRGB:
        case COMPRESSED_BC1_SRGBA:
        case COMPRESSED_BC3_SRGBA:
        case COMPRESSED_BC4_RED:
        case COMPRESSED_BC5_XY:
        case COMPRESSED_BC6_RGB:
        case COMPRESSED_BC7_SRGBA:
            return _context.device->features.textureCompressionBC == VK_TRUE;

        case COMPRESSED_ETC2_RGB:
        case COMPRESSED_ETC2_SRGB:
        case COMPRESSED_ETC2_RGB_PUNCHTHROUGH_ALPHA:
        case COMPRESSED_ETC2_SRGB_PUNCHTHROUGH_ALPHA:
        case COMPRESSED_ETC2_RGBA:
        case COMPRESSED_ETC2_SRGBA:
        case COMPRESSED_EAC_RED:
        case COMPRESSED_EAC_RED_SIGNED:
        case COMPRESSED_EAC_XY:
        case COMPRESSED_EAC_XY_SIGNED:
            return _context.device->features.textureCompressionETC2 == VK_TRUE;

            //case COMPRESSED_ASTC_RGBA_10x10:
            //case COMPRESSED_ASTC_RGBA_10x5:
            //case COMPRESSED_ASTC_RGBA_10x6:
            //case COMPRESSED_ASTC_RGBA_10x8:
            //case COMPRESSED_ASTC_RGBA_12x10:
            //case COMPRESSED_ASTC_RGBA_12x12:
            //case COMPRESSED_ASTC_RGBA_4x4:
            //case COMPRESSED_ASTC_RGBA_5x4:
            //case COMPRESSED_ASTC_RGBA_5x5:
            //case COMPRESSED_ASTC_RGBA_6x5:
            //case COMPRESSED_ASTC_RGBA_6x6:
            //case COMPRESSED_ASTC_RGBA_8x5:
            //case COMPRESSED_ASTC_RGBA_8x6:
            //case COMPRESSED_ASTC_RGBA_8x8:
            //case COMPRESSED_ASTC_SRGB8_ALPHA8_10x10:
            //case COMPRESSED_ASTC_SRGB8_ALPHA8_10x5:
            //case COMPRESSED_ASTC_SRGB8_ALPHA8_10x6:
            //case COMPRESSED_ASTC_SRGB8_ALPHA8_10x8:
            //case COMPRESSED_ASTC_SRGB8_ALPHA8_12x10:
            //case COMPRESSED_ASTC_SRGB8_ALPHA8_12x12:
            //case COMPRESSED_ASTC_SRGB8_ALPHA8_4x4:
            //case COMPRESSED_ASTC_SRGB8_ALPHA8_5x4:
            //case COMPRESSED_ASTC_SRGB8_ALPHA8_5x5:
            //case COMPRESSED_ASTC_SRGB8_ALPHA8_6x5:
            //case COMPRESSED_ASTC_SRGB8_ALPHA8_6x6:
            //case COMPRESSED_ASTC_SRGB8_ALPHA8_8x5:
            //case COMPRESSED_ASTC_SRGB8_ALPHA8_8x6:
            //case COMPRESSED_ASTC_SRGB8_ALPHA8_8x8:
            // return _context.deviceFeatures.textureCompressionASTC_LDR == VK_TRUE;

        default:
            break;
    }
    return true;
}

struct Cache {
    std::unordered_map<uint32_t, VkShaderModule> moduleMap;

    struct Pipeline {
        using RenderpassKey = std::vector<VkFormat>;
        using BindingMap = std::unordered_map<uint32_t, VkShaderStageFlags>;
        using LocationMap = shader::Reflection::LocationMap;

        gpu::PipelineReference pipeline{ GPU_REFERENCE_INIT_VALUE };
        gpu::FormatReference format{ GPU_REFERENCE_INIT_VALUE };
        gpu::FramebufferReference framebuffer{ GPU_REFERENCE_INIT_VALUE };

        std::unordered_map<gpu::PipelineReference, VkPipelineLayout> _layoutMap;
        std::unordered_map<RenderpassKey, VkRenderPass, container_hash<RenderpassKey>> _renderPassMap;

        template <typename T>
        static std::string hex(T t) {
            std::stringstream sStream;
            sStream << std::setw(sizeof(T)) << std::setfill('0') << std::hex << t;
            return sStream.str();
        }

        void setPipeline(const gpu::PipelinePointer& pipeline) {
            if (!gpu::compare(this->pipeline, pipeline)) {
                gpu::assign(this->pipeline, pipeline);
            }
        }

        void setVertexFormat(const gpu::Stream::FormatPointer& format) {
            if (!gpu::compare(this->format, format)) {
                gpu::assign(this->format, format);
            }
        }

        void setFramebuffer(const gpu::FramebufferPointer& framebuffer) {
            if (!gpu::compare(this->framebuffer, framebuffer)) {
                gpu::assign(this->framebuffer, framebuffer);
            }
        }

        static void updateBindingMap(BindingMap& bindingMap,
                                     const LocationMap& locationMap,
                                     VkShaderStageFlagBits shaderStage) {
            for (const auto& entry : locationMap) {
                bindingMap[entry.second] |= shaderStage;
            }
        }

        static void setBindingMap(BindingMap& bindingMap, const LocationMap& vertexMap, const LocationMap& fragmentMap) {
            bindingMap.clear();
            updateBindingMap(bindingMap, vertexMap, VK_SHADER_STAGE_VERTEX_BIT);
            updateBindingMap(bindingMap, fragmentMap, VK_SHADER_STAGE_FRAGMENT_BIT);
        }

        static BindingMap getBindingMap(const LocationMap& vertexMap, const LocationMap& fragmentMap) {
            BindingMap result;
            setBindingMap(result, vertexMap, fragmentMap);
            return result;
        }

        VkPipelineLayout getPipelineLayout(const vks::Context& context) {
            auto itr = _layoutMap.find(pipeline);
            if (_layoutMap.end() == itr) {
                auto pipeline = gpu::acquire(this->pipeline);
                auto program = pipeline->getProgram();
                const auto& vertexReflection = program->getShaders()[0]->getReflection();
                const auto& fragmentRefelection = program->getShaders()[1]->getReflection();

                std::vector<VkDescriptorSetLayoutBinding> uniLayout;
#define SEP_DESC 1
#if SEP_DESC
                std::vector<VkDescriptorSetLayoutBinding> texLayout;
                std::vector<VkDescriptorSetLayoutBinding> stoLayout;
#else
                auto& texLayout = uniLayout;
                auto& stoLayout = uniLayout;
#endif

                for (const auto& entry : getBindingMap(vertexReflection.uniformBuffers, fragmentRefelection.uniformBuffers)) {
                    VkDescriptorSetLayoutBinding binding = vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,entry.second,entry.first,1);
                    uniLayout.push_back(binding);
                }
                for (const auto& entry : getBindingMap(vertexReflection.textures, fragmentRefelection.textures)) {
                    VkDescriptorSetLayoutBinding binding = vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,entry.second,entry.first,1);
                    texLayout.push_back(binding);
                }
                for (const auto& entry : getBindingMap(vertexReflection.resourceBuffers, fragmentRefelection.resourceBuffers)) {
                    VkDescriptorSetLayoutBinding binding = vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,entry.second,entry.first,1);
                    stoLayout.push_back(binding);
                }

                // Create the descriptor set layouts
                std::vector<VkDescriptorSetLayout> layouts;
                if (!uniLayout.empty()) {
                    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI = vks::initializers::descriptorSetLayoutCreateInfo(uniLayout.data(), uniLayout.size());
                    VkDescriptorSetLayout descriptorSetLayout;
                    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(context.device->logicalDevice, &descriptorSetLayoutCI, nullptr, &descriptorSetLayout));
                    layouts.push_back(descriptorSetLayout);
                }
#if SEP_DESC
                if (!texLayout.empty()) {
                    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI = vks::initializers::descriptorSetLayoutCreateInfo(texLayout.data(), texLayout.size());
                    VkDescriptorSetLayout descriptorSetLayout;
                    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(context.device->logicalDevice, &descriptorSetLayoutCI, nullptr, &descriptorSetLayout));
                    layouts.push_back(descriptorSetLayout);
                }
                if (!stoLayout.empty()) {
                    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI = vks::initializers::descriptorSetLayoutCreateInfo(stoLayout.data(), stoLayout.size());
                    VkDescriptorSetLayout descriptorSetLayout;
                    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(context.device->logicalDevice, &descriptorSetLayoutCI, nullptr, &descriptorSetLayout));
                    layouts.push_back(descriptorSetLayout);
                }
#endif
                VkPipelineLayoutCreateInfo pipelineLayoutCI = vks::initializers::pipelineLayoutCreateInfo(layouts.data(), (uint32_t)layouts.size());
                VkPipelineLayout pipelineLayout;
                vkCreatePipelineLayout(context.device->logicalDevice, &pipelineLayoutCI, nullptr, &pipelineLayout);
                return _layoutMap[this->pipeline] = pipelineLayout;
                //return _layoutMap[this->pipeline] = nullptr;
            }
            return itr->second;
        }

        RenderpassKey getRenderPassKey(gpu::Framebuffer* framebuffer) const {
            RenderpassKey result;
            if (!framebuffer) {
                result.push_back(VK_FORMAT_R8G8B8A8_SRGB);
            } else {
                for (const auto& attachment : framebuffer->getRenderBuffers()) {
                    if (attachment.isValid()) {
                        result.push_back(evalTexelFormatInternal(attachment._element));
                    }
                }
                if (framebuffer->hasDepthStencil()) {
                    result.push_back(evalTexelFormatInternal(framebuffer->getDepthStencilBufferFormat()));
                }
            }
            return result;
        }

        VkRenderPass getRenderPass(const vks::Context& context) {
            const auto framebuffer = gpu::acquire(this->framebuffer);

            RenderpassKey key = getRenderPassKey(framebuffer);
            auto itr = _renderPassMap.find(key);
            if (itr == _renderPassMap.end()) {
                std::vector<VkAttachmentDescription> attachments;
                attachments.reserve(key.size());
                std::vector<VkAttachmentReference> colorAttachmentReferences;
                VkAttachmentReference depthReference{};
                for (const auto& format : key) {
                    VkAttachmentDescription attachment{};
                    attachment.format = format;
                    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
                    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
                    if (isDepthStencilFormat(format)) {
                        attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                        depthReference.attachment = (uint32_t)(attachments.size());
                        depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                    } else {
                        attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                        VkAttachmentReference reference;
                        reference.attachment = (uint32_t)(attachments.size());
                        reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                        colorAttachmentReferences.push_back(reference);
                    }
                    attachments.push_back(attachment);
                }

                std::vector<VkSubpassDescription> subpasses;
                {
                    VkSubpassDescription subpass{};
                    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                    if (depthReference.layout != VK_IMAGE_LAYOUT_UNDEFINED) {
                        subpass.pDepthStencilAttachment = &depthReference;
                    }
                    subpass.colorAttachmentCount = (uint32_t)colorAttachmentReferences.size();
                    subpass.pColorAttachments = colorAttachmentReferences.data();
                    subpasses.push_back(subpass);
                }

                VkRenderPassCreateInfo renderPassInfo{};
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                renderPassInfo.attachmentCount = (uint32_t)attachments.size();
                renderPassInfo.pAttachments = attachments.data();
                renderPassInfo.subpassCount = (uint32_t)subpasses.size();
                renderPassInfo.pSubpasses = subpasses.data();
                VkRenderPass renderPass;
                VK_CHECK_RESULT(vkCreateRenderPass(context.device->logicalDevice, &renderPassInfo, nullptr, &renderPass));
                _renderPassMap[key] = renderPass;
                return renderPass;
            }
            return itr->second;
        }
        static std::string getRenderpassKeyString(const RenderpassKey& renderpassKey) {
            std::string result;

            for (const auto& e : renderpassKey) {
                result += hex((uint32_t)e);
            }
            return result;
        }
        std::string getKey() const {
            const auto framebuffer = gpu::acquire(this->framebuffer);
            RenderpassKey renderpassKey = getRenderPassKey(framebuffer);
            const gpu::Pipeline& pipeline = *gpu::acquire(this->pipeline);
            const gpu::State& state = *pipeline.getState();
            const auto& vertexShader = pipeline.getProgram()->getShaders()[0]->getSource();
            const auto& fragmentShader = pipeline.getProgram()->getShaders()[1]->getSource();
            std::string key;
            // FIXME account for customized shaders (preferably by forcing shaders to have a new unique ID at runtime when they're using replacement strings)
            key = hex(shader::makeProgramId(vertexShader.id, fragmentShader.id));
            key += "_" + getRenderpassKeyString(renderpassKey);
            key += "_" + state.getKey();
            key += "_" + format->getKey();
            return key;
        }
    } pipelineState;

    static VkStencilOpState getStencilOp(const gpu::State::StencilTest& stencil) {
        VkStencilOpState result;
        result.compareOp = (VkCompareOp)stencil.getFunction();
        result.passOp = (VkStencilOp)stencil.getPassOp();
        result.failOp = (VkStencilOp)stencil.getFailOp();
        result.depthFailOp = (VkStencilOp)stencil.getDepthFailOp();
        result.reference = stencil.getReference();
        result.compareMask = stencil.getReadMask();
        result.writeMask = 0xFF;
        return result;
    }

    VkShaderModule getShaderModule(const vks::Context& context, const shader::Source& source) {
        auto itr = moduleMap.find(source.id);
        if (moduleMap.end() == itr) {
            const auto& dialectSource = source.dialectSources.find(shader::Dialect::glsl450)->second;
            const auto& variantSource = dialectSource.variantSources.find(shader::Variant::Mono)->second;
            const auto& spirv = variantSource.spirv;
            VkShaderModule result;
            VkShaderModuleCreateInfo shaderModuleCreateInfo{};
            shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            shaderModuleCreateInfo.codeSize = spirv.size();
            shaderModuleCreateInfo.pCode = (const uint32_t*)spirv.data();
            VK_CHECK_RESULT(vkCreateShaderModule(context.device->logicalDevice, &shaderModuleCreateInfo, nullptr, &result));
            moduleMap[source.id] = result;
            return result;
        }
        return itr->second;
    }

    VkPipeline getPipeline(const vks::Context& context) {
        auto renderpass = pipelineState.getRenderPass(context);
        auto pipelineLayout = pipelineState.getPipelineLayout(context);
        const gpu::Pipeline& pipeline = *gpu::acquire(pipelineState.pipeline);
        const gpu::State& state = *pipeline.getState();

        const auto& vertexShader = pipeline.getProgram()->getShaders()[0]->getSource();
        const auto& fragmentShader = pipeline.getProgram()->getShaders()[1]->getSource();
        // FIXME

        const gpu::State::Data& stateData = state.getValues();
        vks::pipelines::GraphicsPipelineBuilder builder{ context.device->logicalDevice, pipelineLayout, renderpass };

        // Input assembly
        {
            auto& ia = builder.inputAssemblyState;
            // ia.primitiveRestartEnable = ???
            // ia.topology = vk::PrimitiveTopology::eTriangleList; ???
        }

        qDebug() << vertexShader.name.c_str() << " " << fragmentShader.name.c_str();

        // Shader modules
        {
            builder.shaderStages.resize(2,{});
            {
                auto& shaderStage = builder.shaderStages[0];
                shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
                shaderStage.pName = "main";
                shaderStage.module = getShaderModule(context, vertexShader);
            }
            {
                auto& shaderStage = builder.shaderStages[1];
                shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                shaderStage.pName = "main";
                shaderStage.module = getShaderModule(context, fragmentShader);
            }
        }

        // Rasterization state
        {
            auto& ra = builder.rasterizationState;
            ra.cullMode = (VkCullModeFlagBits)stateData.cullMode;
            //Bool32 ra.depthBiasEnable;
            //float ra.depthBiasConstantFactor;
            //float ra.depthBiasClamp;
            //float ra.depthBiasSlopeFactor;
            ra.depthClampEnable = stateData.flags.depthClampEnable ? VK_TRUE : VK_FALSE;
            ra.frontFace = stateData.flags.frontFaceClockwise ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
            // ra.lineWidth
            ra.polygonMode = (VkPolygonMode)(2 - stateData.fillMode);
        }

        // Color blending
        {
            auto& cb = builder.colorBlendState;
            auto& ass = cb.blendAttachmentStates;
            auto& rbs = pipelineState.framebuffer->getRenderBuffers();
            uint32_t rbCount = 0;
            for (const auto& rb : rbs) {
                if (rb.isValid()) {
                    ++rbCount;
                }
            }

            for (uint32_t i = 1; i < rbCount; ++i) {
                ass.push_back(ass.back());
            }
        }

        // Depth/Stencil
        {
            auto& ds = builder.depthStencilState;
            ds.depthTestEnable = stateData.depthTest.isEnabled() ? VK_TRUE : VK_FALSE;
            ds.depthWriteEnable = stateData.depthTest.getWriteMask() != 0 ? VK_TRUE : VK_FALSE;
            ds.depthCompareOp = (VkCompareOp)stateData.depthTest.getFunction();
            ds.front = getStencilOp(stateData.stencilTestFront);
            ds.back = getStencilOp(stateData.stencilTestBack);
        }

        // Vertex input
        {
            const auto& vertexReflection = pipeline.getProgram()->getShaders()[0]->getReflection();

            const gpu::Stream::Format& format = *gpu::acquire(pipelineState.format);
            auto& vis = builder.vertexInputState;
            auto& bd = vis.bindingDescriptions;
            auto channelCount = format.getNumChannels();
            for (const auto& entry : format.getChannels()) {
                const auto& slot = entry.first;
                const auto& channel = entry.second;
                bd.push_back({ slot, (uint32_t)channel._stride, (VkVertexInputRate)(channel._frequency) });
            }

            bool colorFound = false;
            auto& ad = vis.attributeDescriptions;
            for (const auto& entry : format.getAttributes()) {
                const auto& slot = entry.first;
                const auto& attribute = entry.second;
                if (slot == Stream::COLOR) {
                    colorFound = true;
                }

                ad.push_back(
                    { slot, attribute._channel, evalTexelFormatInternal(attribute._element), (uint32_t)attribute._offset });
            }

            if (!colorFound && vertexShader.reflection.validInput(Stream::COLOR)) {
                ad.push_back({ Stream::COLOR, 0, VK_FORMAT_R8G8B8A8_UNORM, 0 });
            }

            // Explicitly add the draw call info slot if required
            if (vertexReflection.validInput(gpu::slot::attr::DrawCallInfo)) {
                ad.push_back(
                    { gpu::slot::attr::DrawCallInfo, gpu::slot::attr::DrawCallInfo, VK_FORMAT_R32G32_SINT, (uint32_t)0 });
                bd.push_back({ gpu::slot::attr::DrawCallInfo, (uint32_t)sizeof(uint16_t) * 2, VK_VERTEX_INPUT_RATE_VERTEX });
            }
        }

        auto result = builder.create();
        builder.shaderStages.clear();
        return result;
    }
};

Cache _cache;

void VKBackend::executeFrame(const FramePointer& frame) {
    using namespace vks::debugutils;
    {
        PROFILE_RANGE(gpu_vk_detail, "Preprocess");
        const auto& commandBuffer = _currentCommandBuffer;
        for (const auto& batchPtr : frame->batches) {
            const auto& batch = *batchPtr;
            cmdBeginLabel(commandBuffer, "batch:" + batch.getName(), glm::vec4{ 1, 1, 0, 1 });
            const auto& commands = batch.getCommands();
            const auto& offsets = batch.getCommandOffsets();
            const auto numCommands = commands.size();
            const auto* command = commands.data();
            const auto* offset = offsets.data();
            bool renderpassActive = false;
            for (auto commandIndex = 0; commandIndex < numCommands; ++commandIndex, ++command, ++offset) {
                const auto& paramOffset = *offset;
                // How to resolve renderpass
                switch (*command) {
                    case Batch::COMMAND_draw:
                    case Batch::COMMAND_drawIndexed:
                    case Batch::COMMAND_drawInstanced:
                    case Batch::COMMAND_drawIndexedInstanced:
                    case Batch::COMMAND_multiDrawIndirect:
                    case Batch::COMMAND_multiDrawIndexedIndirect:
                        // resolve layout
                        // resolve pipeline
                        // resolve descriptor set(s)
                        _cache.getPipeline(_context);
                        break;

                    case Batch::COMMAND_setPipeline: {
                        const auto& pipeline = batch._pipelines.get(batch._params[paramOffset]._uint);
                        _cache.pipelineState.setPipeline(pipeline);
                    } break;

                    case Batch::COMMAND_setInputFormat: {
                        const auto& format = batch._streamFormats.get(batch._params[paramOffset]._uint);
                        _cache.pipelineState.setVertexFormat(format);
                    } break;

                    case Batch::COMMAND_setFramebuffer: {
                        if (renderpassActive) {
                            cmdEndLabel(commandBuffer);
                        }
                        const auto& framebuffer = batch._framebuffers.get(batch._params[paramOffset]._uint);
                        cmdBeginLabel(commandBuffer, "framebuffer:" + framebuffer->getName(), vec4{ 1, 0, 1, 1 });
                        renderpassActive = true;
                        _cache.pipelineState.setFramebuffer(framebuffer);
                    } break;

                    case Batch::COMMAND_setInputBuffer:
                    case Batch::COMMAND_setIndexBuffer:
                    case Batch::COMMAND_setIndirectBuffer:

                    case Batch::COMMAND_setModelTransform:
                    case Batch::COMMAND_setViewTransform:
                    case Batch::COMMAND_setProjectionTransform:
                    case Batch::COMMAND_setProjectionJitter:
                    case Batch::COMMAND_setViewportTransform:
                    case Batch::COMMAND_setDepthRangeTransform:

                    case Batch::COMMAND_setStateBlendFactor:
                    case Batch::COMMAND_setStateScissorRect:

                    case Batch::COMMAND_setUniformBuffer:
                    case Batch::COMMAND_setResourceBuffer:
                    case Batch::COMMAND_setResourceTexture:
                    case Batch::COMMAND_setResourceTextureTable:
                    case Batch::COMMAND_setResourceFramebufferSwapChainTexture:
                        break;

                    case Batch::COMMAND_setFramebufferSwapChain:
                    case Batch::COMMAND_clearFramebuffer:
                    case Batch::COMMAND_blit:
                    case Batch::COMMAND_generateTextureMips:

                    case Batch::COMMAND_advance:

                    case Batch::COMMAND_beginQuery:
                    case Batch::COMMAND_endQuery:
                    case Batch::COMMAND_getQuery:

                    case Batch::COMMAND_resetStages:

                    case Batch::COMMAND_disableContextViewCorrection:
                    case Batch::COMMAND_restoreContextViewCorrection:

                    case Batch::COMMAND_disableContextStereo:
                    case Batch::COMMAND_restoreContextStereo:

                    case Batch::COMMAND_runLambda:

                    case Batch::COMMAND_startNamedCall:
                    case Batch::COMMAND_stopNamedCall:

                    case Batch::COMMAND_pushProfileRange:
                    case Batch::COMMAND_popProfileRange:
                        break;
                    // These weren't handled here old Vulkan branch
                    case Batch::COMMAND_generateTextureMipsWithPipeline:
                    case Batch::COMMAND_glUniform1f:
                    case Batch::COMMAND_glUniform2f:
                    case Batch::COMMAND_glUniform3f:
                    case Batch::COMMAND_glUniform4f:
                        Q_ASSERT(false);
                    case Batch::NUM_COMMANDS:
                        Q_ASSERT(false);
                }
                // loop through commands
                // did the framebuffer setup change?  (new renderpass)
                // did the descriptor setup change?
                // did the pipeline / vertex format change?
                // derive pipeline layout
                // generate pipeline
                // find unique descriptor targets
                // do we need to transfer data to the GPU?
            }

            if (renderpassActive) {
                cmdEndLabel(commandBuffer);
                renderpassActive = false;
            }
            cmdEndLabel(commandBuffer);
        }
    }

    // loop through commands

    //

    //{
    //    PROFILE_RANGE(gpu_vk_detail, _stereo._enable ? "Render Stereo" : "Render");
    //    renderPassDraw(batch);
    //}

    // Restore the saved stereo state for the next batch
    // _stereo._enable = savedStereo;
}

void VKBackend::setDrawCommandBuffer(VkCommandBuffer commandBuffer) {
    _currentCommandBuffer = commandBuffer;
}

void VKBackend::trash(const VKBuffer& buffer) {
}
#if 0
void VKBackend::do_resetStages(const Batch& batch, size_t paramOffset) {
}

void VKBackend::do_runLambda(const Batch& batch, size_t paramOffset) {
    std::function<void()> f = batch._lambdas.get(batch._params[paramOffset]._uint);
    f();
}

void VKBackend::do_startNamedCall(const Batch& batch, size_t paramOffset) {
    batch._currentNamedCall = batch._names.get(batch._params[paramOffset]._uint);
}

void VKBackend::do_stopNamedCall(const Batch& batch, size_t paramOffset) {
    batch._currentNamedCall.clear();
}

void VKBackend::do_pushProfileRange(const Batch& batch, size_t paramOffset) {
    const auto& name = batch._profileRanges.get(batch._params[paramOffset]._uint);
    ::vks::debug::marker::beginRegion(vk::CommandBuffer{}, name, glm::vec4{ 1.0 });
}

void VKBackend::do_popProfileRange(const Batch& batch, size_t paramOffset) {
    ::vks::debug::marker::endRegion(vk::CommandBuffer{});
}

void VKBackend::setCameraCorrection(const Mat4& correction) {
}

void VKBackend::renderPassTransfer(Batch& batch) {
    const size_t numCommands = batch.getCommands().size();
    const Batch::Commands::value_type* command = batch.getCommands().data();
    const Batch::CommandOffsets::value_type* offset = batch.getCommandOffsets().data();

    _inRenderTransferPass = true;
    { // Sync all the buffers
        PROFILE_RANGE("syncGPUBuffer");

        for (auto& cached : batch._buffers._items) {
            if (cached._data) {
                syncGPUObject(*cached._data);
            }
        }
    }

    { // Sync all the buffers
        PROFILE_RANGE("syncCPUTransform");
        _transform._cameras.clear();
        _transform._cameraOffsets.clear();

        for (_commandIndex = 0; _commandIndex < numCommands; ++_commandIndex) {
            switch (*command) {
            case Batch::COMMAND_draw:
            case Batch::COMMAND_drawIndexed:
            case Batch::COMMAND_drawInstanced:
            case Batch::COMMAND_drawIndexedInstanced:
            case Batch::COMMAND_multiDrawIndirect:
            case Batch::COMMAND_multiDrawIndexedIndirect:
                _transform.preUpdate(_commandIndex, _stereo);
                break;

            case Batch::COMMAND_setViewportTransform:
            case Batch::COMMAND_setViewTransform:
            case Batch::COMMAND_setProjectionTransform: {
                CommandCall call = _commandCalls[(*command)];
                (this->*(call))(batch, *offset);
                break;
            }

            default:
                break;
            }
            command++;
            offset++;
        }
    }

    { // Sync the transform buffers
        PROFILE_RANGE("syncGPUTransform");
        transferTransformState(batch);
    }

    _inRenderTransferPass = false;
}

void VKBackend::renderPassDraw(const Batch& batch) {
    _currentDraw = -1;
    _transform._camerasItr = _transform._cameraOffsets.begin();
    const size_t numCommands = batch.getCommands().size();
    const Batch::Commands::value_type* command = batch.getCommands().data();
    const Batch::CommandOffsets::value_type* offset = batch.getCommandOffsets().data();
    for (_commandIndex = 0; _commandIndex < numCommands; ++_commandIndex) {
        switch (*command) {
            // Ignore these commands on this pass, taken care of in the transfer pass
            // Note we allow COMMAND_setViewportTransform to occur in both passes
            // as it both updates the transform object (and thus the uniforms in the 
            // UBO) as well as executes the actual viewport call
        case Batch::COMMAND_setModelTransform:
        case Batch::COMMAND_setViewTransform:
        case Batch::COMMAND_setProjectionTransform:
            break;

        case Batch::COMMAND_draw:
        case Batch::COMMAND_drawIndexed:
        case Batch::COMMAND_drawInstanced:
        case Batch::COMMAND_drawIndexedInstanced:
        case Batch::COMMAND_multiDrawIndirect:
        case Batch::COMMAND_multiDrawIndexedIndirect: {
            // updates for draw calls
            ++_currentDraw;
            updateInput();
            updateTransform(batch);
            updatePipeline();

            CommandCall call = _commandCalls[(*command)];
            (this->*(call))(batch, *offset);
            break;
        }
        default: {
            CommandCall call = _commandCalls[(*command)];
            (this->*(call))(batch, *offset);
            break;
        }
        }

        command++;
        offset++;
    }
}

void VKBackend::setupStereoSide(int side) {
    ivec4 vp = _transform._viewport;
    vp.z /= 2;
    glViewport(vp.x + side * vp.z, vp.y, vp.z, vp.w);

    _transform.bindCurrentCamera(side);
}


void VKBackend::do_setFramebuffer(const Batch& batch, size_t paramOffset) {
    auto framebuffer = batch._framebuffers.get(batch._params[paramOffset]._uint);
    if (_output._framebuffer != framebuffer) {
        auto newFBO = getFramebufferID(framebuffer);
        if (_output._drawFBO != newFBO) {
            _output._drawFBO = newFBO;
            glBindFramebuffer(VK_DRAW_FRAMEBUFFER, newFBO);
        }
        _output._framebuffer = framebuffer;
    }
}

void VKBackend::do_clearFramebuffer(const Batch& batch, size_t paramOffset) {
    if (_stereo._enable && !_pipeline._stateCache.scissorEnable) {
        qWarning("Clear without scissor in stereo mode");
    }

    uint32 masks = batch._params[paramOffset + 7]._uint;
    Vec4 color;
    color.x = batch._params[paramOffset + 6]._float;
    color.y = batch._params[paramOffset + 5]._float;
    color.z = batch._params[paramOffset + 4]._float;
    color.w = batch._params[paramOffset + 3]._float;
    float depth = batch._params[paramOffset + 2]._float;
    int stencil = batch._params[paramOffset + 1]._int;
    int useScissor = batch._params[paramOffset + 0]._int;

    VKuint glmask = 0;
    if (masks & Framebuffer::BUFFER_STENCIL) {
        glClearStencil(stencil);
        glmask |= VK_STENCIL_BUFFER_BIT;
        // TODO: we will probably need to also check the write mask of stencil like we do
        // for depth buffer, but as would say a famous Fez owner "We'll cross that bridge when we come to it"
    }

    bool restoreDepthMask = false;
    if (masks & Framebuffer::BUFFER_DEPTH) {
        glClearDepth(depth);
        glmask |= VK_DEPTH_BUFFER_BIT;

        bool cacheDepthMask = _pipeline._stateCache.depthTest.getWriteMask();
        if (!cacheDepthMask) {
            restoreDepthMask = true;
            glDepthMask(VK_TRUE);
        }
    }

    std::vector<VKenum> drawBuffers;
    if (masks & Framebuffer::BUFFER_COLORS) {
        if (_output._framebuffer) {
            for (unsigned int i = 0; i < Framebuffer::MAX_NUM_RENDER_BUFFERS; i++) {
                if (masks & (1 << i)) {
                    drawBuffers.push_back(VK_COLOR_ATTACHMENT0 + i);
                }
            }

            if (!drawBuffers.empty()) {
                glDrawBuffers((VKsizei)drawBuffers.size(), drawBuffers.data());
                glClearColor(color.x, color.y, color.z, color.w);
                glmask |= VK_COLOR_BUFFER_BIT;

                (void)CHECK_VK_ERROR();
            }
        } else {
            glClearColor(color.x, color.y, color.z, color.w);
            glmask |= VK_COLOR_BUFFER_BIT;
        }

        // Force the color mask cache to WRITE_ALL if not the case
        do_setStateColorWriteMask(State::ColorMask::WRITE_ALL);
    }

    // Apply scissor if needed and if not already on
    bool doEnableScissor = (useScissor && (!_pipeline._stateCache.scissorEnable));
    if (doEnableScissor) {
        glEnable(VK_SCISSOR_TEST);
    }

    // Clear!
    glClear(glmask);

    // Restore scissor if needed
    if (doEnableScissor) {
        glDisable(VK_SCISSOR_TEST);
    }

    // Restore write mask meaning turn back off
    if (restoreDepthMask) {
        glDepthMask(VK_FALSE);
    }

    // Restore the color draw buffers only if a frmaebuffer is bound
    if (_output._framebuffer && !drawBuffers.empty()) {
        auto glFramebuffer = syncGPUObject(*_output._framebuffer);
        if (glFramebuffer) {
            glDrawBuffers((VKsizei)glFramebuffer->_colorBuffers.size(), glFramebuffer->_colorBuffers.data());
        }
    }

    (void)CHECK_VK_ERROR();
}

void VKBackend::downloadFramebuffer(const FramebufferPointer& srcFramebuffer, const Vec4i& region, QImage& destImage) {
}

void VKBackend::do_setInputFormat(const Batch& batch, size_t paramOffset) {
    Stream::FormatPointer format = batch._streamFormats.get(batch._params[paramOffset]._uint);

    if (format != _input._format) {
        _input._format = format;
        _input._invalidFormat = true;
    }
}

void VKBackend::do_setInputBuffer(const Batch& batch, size_t paramOffset) {
    Offset stride = batch._params[paramOffset + 0]._uint;
    Offset offset = batch._params[paramOffset + 1]._uint;
    BufferPointer buffer = batch._buffers.get(batch._params[paramOffset + 2]._uint);
    uint32 channel = batch._params[paramOffset + 3]._uint;

    if (channel < getNumInputBuffers()) {
        bool isModified = false;
        if (_input._buffers[channel] != buffer) {
            _input._buffers[channel] = buffer;

            VKuint vbo = 0;
            if (buffer) {
                vbo = getBufferID((*buffer));
            }
            _input._bufferVBOs[channel] = vbo;

            isModified = true;
        }

        if (_input._bufferOffsets[channel] != offset) {
            _input._bufferOffsets[channel] = offset;
            isModified = true;
        }

        if (_input._bufferStrides[channel] != stride) {
            _input._bufferStrides[channel] = stride;
            isModified = true;
        }

        if (isModified) {
            _input._invalidBuffers.set(channel);
        }
    }
}

void VKBackend::do_setIndexBuffer(const Batch& batch, size_t paramOffset) {
    _input._indexBufferType = (Type)batch._params[paramOffset + 2]._uint;
    _input._indexBufferOffset = batch._params[paramOffset + 0]._uint;

    BufferPointer indexBuffer = batch._buffers.get(batch._params[paramOffset + 1]._uint);
    if (indexBuffer != _input._indexBuffer) {
        _input._indexBuffer = indexBuffer;
        if (indexBuffer) {
            glBindBuffer(VK_ELEMENT_ARRAY_BUFFER, getBufferID(*indexBuffer));
        } else {
            // FIXME do we really need this?  Is there ever a draw call where we care that the element buffer is null?
            glBindBuffer(VK_ELEMENT_ARRAY_BUFFER, 0);
        }
    }
    (void)CHECK_VK_ERROR();
}

void VKBackend::do_setIndirectBuffer(const Batch& batch, size_t paramOffset) {
    _input._indirectBufferOffset = batch._params[paramOffset + 1]._uint;
    _input._indirectBufferStride = batch._params[paramOffset + 2]._uint;

    BufferPointer buffer = batch._buffers.get(batch._params[paramOffset]._uint);
    if (buffer != _input._indirectBuffer) {
        _input._indirectBuffer = buffer;
        if (buffer) {
            glBindBuffer(VK_DRAW_INDIRECT_BUFFER, getBufferID(*buffer));
        } else {
            // FIXME do we really need this?  Is there ever a draw call where we care that the element buffer is null?
            glBindBuffer(VK_DRAW_INDIRECT_BUFFER, 0);
        }
    }

    (void)CHECK_VK_ERROR();
}

void VKBackend::do_generateTextureMips(const Batch& batch, size_t paramOffset) {
    TexturePointer resourceTexture = batch._textures.get(batch._params[paramOffset + 0]._uint);
    if (!resourceTexture) {
        return;
    }

    // DO not transfer the texture, this call is expected for rendering texture
    VKTexture* object = syncGPUObject(resourceTexture, false);
    if (!object) {
        return;
    }

    object->generateMips();
}

void VKBackend::do_beginQuery(const Batch& batch, size_t paramOffset) {
    auto query = batch._queries.get(batch._params[paramOffset]._uint);
    VKQuery* glquery = syncGPUObject(*query);
    if (glquery) {
        if (timeElapsed) {
            glBeginQuery(VK_TIME_ELAPSED, glquery->_endqo);
        } else {
            glQueryCounter(glquery->_beginqo, VK_TIMESTAMP);
        }
        (void)CHECK_VK_ERROR();
    }
}

void VKBackend::do_endQuery(const Batch& batch, size_t paramOffset) {
    auto query = batch._queries.get(batch._params[paramOffset]._uint);
    VKQuery* glquery = syncGPUObject(*query);
    if (glquery) {
        if (timeElapsed) {
            glEndQuery(VK_TIME_ELAPSED);
        } else {
            glQueryCounter(glquery->_endqo, VK_TIMESTAMP);
        }
        (void)CHECK_VK_ERROR();
    }
}

void VKBackend::do_getQuery(const Batch& batch, size_t paramOffset) {
    auto query = batch._queries.get(batch._params[paramOffset]._uint);
    VKQuery* glquery = syncGPUObject(*query);
    if (glquery) {
        glGetQueryObjectui64v(glquery->_endqo, VK_QUERY_RESULT_AVAILABLE, &glquery->_result);
        if (glquery->_result == VK_TRUE) {
            if (timeElapsed) {
                glGetQueryObjectui64v(glquery->_endqo, VK_QUERY_RESULT, &glquery->_result);
            } else {
                VKuint64 start, end;
                glGetQueryObjectui64v(glquery->_beginqo, VK_QUERY_RESULT, &start);
                glGetQueryObjectui64v(glquery->_endqo, VK_QUERY_RESULT, &end);
                glquery->_result = end - start;
            }
            query->triggerReturnHandler(glquery->_result);
        }
        (void)CHECK_VK_ERROR();
    }
}

// Transform Stage
void VKBackend::do_setModelTransform(const Batch& batch, size_t paramOffset) {
}

void VKBackend::do_setViewTransform(const Batch& batch, size_t paramOffset) {
    _transform._view = batch._transforms.get(batch._params[paramOffset]._uint);
    _transform._viewIsCamera = batch._params[paramOffset + 1]._uint != 0;
    _transform._invalidView = true;
}

void VKBackend::do_setProjectionTransform(const Batch& batch, size_t paramOffset) {
    memcpy(&_transform._projection, batch.editData(batch._params[paramOffset]._uint), sizeof(Mat4));
    _transform._invalidProj = true;
}

void VKBackend::do_setViewportTransform(const Batch& batch, size_t paramOffset) {
    memcpy(&_transform._viewport, batch.editData(batch._params[paramOffset]._uint), sizeof(Vec4i));

    if (!_inRenderTransferPass && !isStereo()) {
        ivec4& vp = _transform._viewport;
        glViewport(vp.x, vp.y, vp.z, vp.w);
    }

    // The Viewport is tagged invalid because the CameraTransformUBO is not up to date and will need update on next drawcall
    _transform._invalidViewport = true;
}

void VKBackend::do_setDepthRangeTransform(const Batch& batch, size_t paramOffset) {
    Vec2 depthRange(batch._params[paramOffset + 1]._float, batch._params[paramOffset + 0]._float);

    if ((depthRange.x != _transform._depthRange.x) || (depthRange.y != _transform._depthRange.y)) {
        _transform._depthRange = depthRange;

        glDepthRangef(depthRange.x, depthRange.y);
    }
}

void VKBackend::do_setStateScissorRect(const Batch& batch, size_t paramOffset) {
    Vec4i rect;
    memcpy(&rect, batch.editData(batch._params[paramOffset]._uint), sizeof(Vec4i));

    if (_stereo._enable) {
        rect.z /= 2;
        if (_stereo._pass) {
            rect.x += rect.z;
        }
    }
    glScissor(rect.x, rect.y, rect.z, rect.w);
    (void)CHECK_VK_ERROR();
}

void VKBackend::do_setPipeline(const Batch& batch, size_t paramOffset) {
    PipelinePointer pipeline = batch._pipelines.get(batch._params[paramOffset + 0]._uint);

    if (_pipeline._pipeline == pipeline) {
        return;
    }

    // A true new Pipeline
    _stats._PSNumSetPipelines++;

    // null pipeline == reset
    if (!pipeline) {
        _pipeline._pipeline.reset();

        _pipeline._program = 0;
        _pipeline._cameraCorrectionLocation = -1;
        _pipeline._programShader = nullptr;
        _pipeline._invalidProgram = true;

        _pipeline._state = nullptr;
        _pipeline._invalidState = true;
    } else {
        auto pipelineObject = VKPipeline::sync(*this, *pipeline);
        if (!pipelineObject) {
            return;
        }

        // check the program cache
        // pick the program version
        VKuint glprogram = pipelineObject->_program->getProgram();

        if (_pipeline._program != glprogram) {
            _pipeline._program = glprogram;
            _pipeline._programShader = pipelineObject->_program;
            _pipeline._invalidProgram = true;
            _pipeline._cameraCorrectionLocation = pipelineObject->_cameraCorrection;
        }

        // Now for the state
        if (_pipeline._state != pipelineObject->_state) {
            _pipeline._state = pipelineObject->_state;
            _pipeline._invalidState = true;
        }

        // Remember the new pipeline
        _pipeline._pipeline = pipeline;
    }

    // THis should be done on Pipeline::update...
    if (_pipeline._invalidProgram) {
        glUseProgram(_pipeline._program);
        if (_pipeline._cameraCorrectionLocation != -1) {
            auto cameraCorrectionBuffer = syncGPUObject(*_pipeline._cameraCorrectionBuffer._buffer);
            glBindBufferRange(VK_UNIFORM_BUFFER, _pipeline._cameraCorrectionLocation, cameraCorrectionBuffer->_id, 0,
                              sizeof(CameraCorrection));
        }
        (void)CHECK_VK_ERROR();
        _pipeline._invalidProgram = false;
    }
}

void VKBackend::do_setUniformBuffer(const Batch& batch, size_t paramOffset) {
    VKuint slot = batch._params[paramOffset + 3]._uint;
    BufferPointer uniformBuffer = batch._buffers.get(batch._params[paramOffset + 2]._uint);
    VKintptr rangeStart = batch._params[paramOffset + 1]._uint;
    VKsizeiptr rangeSize = batch._params[paramOffset + 0]._uint;

    if (!uniformBuffer) {
        releaseUniformBuffer(slot);
        return;
    }

    // check cache before thinking
    if (_uniform._buffers[slot] == uniformBuffer) {
        return;
    }

    // Sync BufferObject
    auto* object = syncGPUObject(*uniformBuffer);
    if (object) {
        glBindBufferRange(VK_UNIFORM_BUFFER, slot, object->_buffer, rangeStart, rangeSize);

        _uniform._buffers[slot] = uniformBuffer;
        (void)CHECK_VK_ERROR();
    } else {
        releaseResourceTexture(slot);
        return;
    }
}

void VKBackend::do_setResourceTexture(const Batch& batch, size_t paramOffset) {
    VKuint slot = batch._params[paramOffset + 1]._uint;
    TexturePointer resourceTexture = batch._textures.get(batch._params[paramOffset + 0]._uint);

    if (!resourceTexture) {
        releaseResourceTexture(slot);
        return;
    }
    // check cache before thinking
    if (_resource._textures[slot] == resourceTexture) {
        return;
    }

    // One more True texture bound
    _stats._RSNumTextureBounded++;

    // Always make sure the VKObject is in sync
    VKTexture* object = syncGPUObject(resourceTexture);
    if (object) {
        VKuint to = object->_texture;
        VKuint target = object->_target;
        glActiveTexture(VK_TEXTURE0 + slot);
        glBindTexture(target, to);

        (void)CHECK_VK_ERROR();

        _resource._textures[slot] = resourceTexture;

        _stats._RSAmountTextureMemoryBounded += object->size();

    } else {
        releaseResourceTexture(slot);
        return;
    }
}

std::array<VKBackend::CommandCall, Batch::NUM_COMMANDS> VKBackend::_commandCalls{ {
    (&::gpu::vulkan::VKBackend::do_draw),
    (&::gpu::vulkan::VKBackend::do_drawIndexed),
    (&::gpu::vulkan::VKBackend::do_drawInstanced),
    (&::gpu::vulkan::VKBackend::do_drawIndexedInstanced),
    (&::gpu::vulkan::VKBackend::do_multiDrawIndirect),
    (&::gpu::vulkan::VKBackend::do_multiDrawIndexedIndirect),

    (&::gpu::vulkan::VKBackend::do_setInputFormat),
    (&::gpu::vulkan::VKBackend::do_setInputBuffer),
    (&::gpu::vulkan::VKBackend::do_setIndexBuffer),
    (&::gpu::vulkan::VKBackend::do_setIndirectBuffer),

    (&::gpu::vulkan::VKBackend::do_setModelTransform),
    (&::gpu::vulkan::VKBackend::do_setViewTransform),
    (&::gpu::vulkan::VKBackend::do_setProjectionTransform),
    (&::gpu::vulkan::VKBackend::do_setProjectionJitter),
    (&::gpu::vulkan::VKBackend::do_setViewportTransform),
    (&::gpu::vulkan::VKBackend::do_setDepthRangeTransform),

    (&::gpu::vulkan::VKBackend::do_setPipeline),
    (&::gpu::vulkan::VKBackend::do_setStateBlendFactor),
    (&::gpu::vulkan::VKBackend::do_setStateScissorRect),

    (&::gpu::vulkan::VKBackend::do_setUniformBuffer),
    (&::gpu::vulkan::VKBackend::do_setResourceBuffer),
    (&::gpu::vulkan::VKBackend::do_setResourceTexture),
    (&::gpu::vulkan::VKBackend::do_setResourceTextureTable),
    (&::gpu::vulkan::VKBackend::do_setResourceFramebufferSwapChainTexture),

    (&::gpu::vulkan::VKBackend::do_setFramebuffer),
    (&::gpu::vulkan::VKBackend::do_setFramebufferSwapChain),
    (&::gpu::vulkan::VKBackend::do_clearFramebuffer),
    (&::gpu::vulkan::VKBackend::do_blit),
    (&::gpu::vulkan::VKBackend::do_generateTextureMips),

    (&::gpu::vulkan::VKBackend::do_advance),

    (&::gpu::vulkan::VKBackend::do_beginQuery),
    (&::gpu::vulkan::VKBackend::do_endQuery),
    (&::gpu::vulkan::VKBackend::do_getQuery),

    (&::gpu::vulkan::VKBackend::do_resetStages),

    (&::gpu::vulkan::VKBackend::do_disableContextViewCorrection),
    (&::gpu::vulkan::VKBackend::do_restoreContextViewCorrection),
    (&::gpu::vulkan::VKBackend::do_disableContextStereo),
    (&::gpu::vulkan::VKBackend::do_restoreContextStereo),

    (&::gpu::vulkan::VKBackend::do_runLambda),

    (&::gpu::vulkan::VKBackend::do_startNamedCall),
    (&::gpu::vulkan::VKBackend::do_stopNamedCall),

    (&::gpu::vulkan::VKBackend::do_glUniform1f),
    (&::gpu::vulkan::VKBackend::do_glUniform2f),
    (&::gpu::vulkan::VKBackend::do_glUniform3f),
    (&::gpu::vulkan::VKBackend::do_glUniform4f),
    (&::gpu::vulkan::VKBackend::do_glColor4f),

    (&::gpu::vulkan::VKBackend::do_pushProfileRange),
    (&::gpu::vulkan::VKBackend::do_popProfileRange),
} };
#endif
