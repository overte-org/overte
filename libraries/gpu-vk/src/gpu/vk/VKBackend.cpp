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

#include <gpu/TextureTable.h>
#include <vk/Helpers.h>
#include <vk/Version.h>
#include <vk/Pipelines.h>
#include "VKFramebuffer.h"
#include "VKBuffer.h"
#include "VKQuery.h"

#include "VKForward.h"
#include "VKShared.h"
#include "VKTexture.h"

#define FORCE_STRICT_TEXTURE 1

using namespace gpu;
using namespace gpu::vk;

size_t VKBackend::UNIFORM_BUFFER_OFFSET_ALIGNMENT{ 4 };

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

    // Add frame data to frames pool
    for (int i = 0; i < 3; i++) {
        _framePool.push_back(std::make_shared<FrameData>(this));
        _framesToReuse.push_back(_framePool.back());
    }
    initTransform();
    initDefaultTexture();
}

VKBackend::~VKBackend() {
    // FIXME queue up all the trash calls
    // VKTODO: move to context
    VK_CHECK_RESULT(vkQueueWaitIdle(_context.graphicsQueue));
    VK_CHECK_RESULT(vkQueueWaitIdle(_context.transferQueue) );
    VK_CHECK_RESULT(vkDeviceWaitIdle(_context.device->logicalDevice));

    // Release frames so their destructors can do a cleanup
    _currentFrame.reset();
    _framesToReuse.resize(0);
    _framePool.resize(0);

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

        struct PipelineLayout {
            VkPipelineLayout pipelineLayout;
            VkDescriptorSetLayout uniformLayout;
            VkDescriptorSetLayout textureLayout;
            VkDescriptorSetLayout storageLayout;
        };

        std::unordered_map<gpu::PipelineReference, PipelineLayout> _layoutMap;
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

        // VKTODO: This needs to be used instead of getPipeline
        // Returns structure containing pipeline layout and descriptor set layouts
        PipelineLayout getPipelineAndDescriptorLayout(const vks::Context& context) {
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
                PipelineLayout layout {};

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
                    layout.uniformLayout = descriptorSetLayout;
                }
#if SEP_DESC
                if (!texLayout.empty()) {
                    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI = vks::initializers::descriptorSetLayoutCreateInfo(texLayout.data(), texLayout.size());
                    VkDescriptorSetLayout descriptorSetLayout;
                    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(context.device->logicalDevice, &descriptorSetLayoutCI, nullptr, &descriptorSetLayout));
                    layouts.push_back(descriptorSetLayout);
                    layout.textureLayout = descriptorSetLayout;
                } else {
                    printf("empty");
                }
                if (!stoLayout.empty()) {
                    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI = vks::initializers::descriptorSetLayoutCreateInfo(stoLayout.data(), stoLayout.size());
                    VkDescriptorSetLayout descriptorSetLayout;
                    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(context.device->logicalDevice, &descriptorSetLayoutCI, nullptr, &descriptorSetLayout));
                    layouts.push_back(descriptorSetLayout);
                    layout.storageLayout = descriptorSetLayout;
                }
#endif
                VkPipelineLayoutCreateInfo pipelineLayoutCI = vks::initializers::pipelineLayoutCreateInfo(layouts.data(), (uint32_t)layouts.size());
                VkPipelineLayout pipelineLayout;
                VK_CHECK_RESULT(vkCreatePipelineLayout(context.device->logicalDevice, &pipelineLayoutCI, nullptr, &pipelineLayout));

                layout.pipelineLayout = pipelineLayout;

                return _layoutMap[this->pipeline] = layout;
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
                        // VKTODO: why _element often has different format than texture's pixel format, and seemingly wrong one?
                        //result.push_back(evalTexelFormatInternal(attachment._element));
                        result.push_back(evalTexelFormatInternal(attachment._texture->getTexelFormat()));
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
            } else {
                printf("found");
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
        //VKTODO: pipelines are not cached here currently
        auto renderpass = pipelineState.getRenderPass(context);
        auto pipelineLayout = pipelineState.getPipelineAndDescriptorLayout(context);
        const gpu::Pipeline& pipeline = *gpu::acquire(pipelineState.pipeline);
        const gpu::State& state = *pipeline.getState();

        const auto& vertexShader = pipeline.getProgram()->getShaders()[0]->getSource();
        const auto& fragmentShader = pipeline.getProgram()->getShaders()[1]->getSource();
        // FIXME

        const gpu::State::Data& stateData = state.getValues();
        vks::pipelines::GraphicsPipelineBuilder builder{ context.device->logicalDevice, pipelineLayout.pipelineLayout, renderpass };

        // Input assembly
        {
            auto& ia = builder.inputAssemblyState;
            // VKTODO: this looks unfinished
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
            //ra.depthClampEnable = VK_TRUE;
            ra.depthClampEnable = stateData.flags.depthClampEnable ? VK_TRUE : VK_FALSE; // VKTODO
            ra.frontFace = stateData.flags.frontFaceClockwise ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
            // ra.lineWidth
            ra.polygonMode = (VkPolygonMode)(2 - stateData.fillMode);
        }

        // Color blending
        {
            auto& cb = builder.colorBlendState;
            auto& ass = cb.blendAttachmentStates;
            Q_ASSERT(pipelineState.framebuffer);
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
            //ds.depthTestEnable = VK_FALSE;
            ds.depthTestEnable = stateData.depthTest.isEnabled() ? VK_TRUE : VK_FALSE; //VKTODO
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
                VkVertexInputBindingDescription bindingDescription {};
                bindingDescription.binding = slot;
                bindingDescription.stride = (uint32_t)channel._stride;
                qDebug() << "binding " << bindingDescription.binding << "stride" << bindingDescription.stride;
                bindingDescription.inputRate = (VkVertexInputRate)(channel._frequency);
                bd.push_back(bindingDescription);
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
                    { gpu::slot::attr::DrawCallInfo, gpu::slot::attr::DrawCallInfo, VK_FORMAT_R16G16_SINT, (uint32_t)0 });
                //bd.push_back({ gpu::slot::attr::DrawCallInfo, (uint32_t)sizeof(uint16_t) * 2, VK_VERTEX_INPUT_RATE_VERTEX });
                bd.push_back({ gpu::slot::attr::DrawCallInfo, 0, VK_VERTEX_INPUT_RATE_VERTEX });
            }
        }

        auto result = builder.create();
        builder.shaderStages.clear();
        return result;
    }
};

// VKTODO: this is ugly solution
Cache _cache;

void VKBackend::executeFrame(const FramePointer& frame) {
    using namespace vks::debugutils;
    // Create descriptor pool
    // VKTODO: delete descriptor pool after it's not needed
    //_frameData._descriptorPool
    acquireFrameData();

    int batch_count = 0;
    {
        const auto& commandBuffer = _currentCommandBuffer;
        for (const auto& batchPtr : frame->batches) {
            /*if (batch_count == 6) {//12
                return;
            }*/
            const auto& batch = *batchPtr;
            if (batch.getName() == "CompositeHUD") {
                continue; // VKTODO: crashes frame player currently
            }
            if (batch.getName() == "Resample::run") {
                continue; // VKTODO: no framebuffer commands support yet
            }
            cmdBeginLabel(commandBuffer, "batch:" + batch.getName(), glm::vec4{ 1, 1, 0, 1 });
            const auto& commands = batch.getCommands();
            const auto& offsets = batch.getCommandOffsets();
            const auto numCommands = commands.size();
            const auto* command = commands.data();
            const auto* offset = offsets.data();
            bool renderpassActive = false;
            for (auto commandIndex = 0; commandIndex < numCommands; ++commandIndex, ++command, ++offset) {
                const auto& paramOffset = *offset;
                {
                    PROFILE_RANGE(gpu_vk_detail, "Preprocess");
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
                            if (framebuffer) {
                                cmdBeginLabel(commandBuffer, "framebuffer:" + framebuffer->getName(), vec4{ 1, 0, 1, 1 });
                            } else {
                                cmdBeginLabel(commandBuffer, "framebuffer: NULL", vec4{ 1, 0, 1, 1 });
                            }
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

            {
                PROFILE_RANGE(gpu_vk_detail, "Transfer");
                renderPassTransfer(batch);
            }

            {
                PROFILE_RANGE(gpu_vk_detail, _stereo._enable ? "Render Stereo" : "Render");
                renderPassDraw(batch);
            }

            if (renderpassActive) {
                cmdEndLabel(commandBuffer);
                renderpassActive = false;
            }
            cmdEndLabel(commandBuffer);
            batch_count++;
        }
    }

    VK_CHECK_RESULT(vkQueueWaitIdle(_context.graphicsQueue));
    VK_CHECK_RESULT(vkQueueWaitIdle(_context.transferQueue) );
    VK_CHECK_RESULT(vkDeviceWaitIdle(_context.device->logicalDevice));

    //auto frameToRecycle = _currentFrame;
    // Move pointer to current frame to property that will store it while it's being rendered and before it's recycled.
    _currentlyRenderedFrame = _currentFrame;
    releaseFrameData();

    // loop through commands

    //


    // Restore the saved stereo state for the next batch
    // _stereo._enable = savedStereo;
}

void VKBackend::setDrawCommandBuffer(VkCommandBuffer commandBuffer) {
    _currentCommandBuffer = commandBuffer;
}

void VKBackend::trash(VKBuffer& buffer) {
    // VKTODO: thread safety for this and similar calls
    buffer.destroy();
}

void VKBackend::TransformStageState::preUpdate(size_t commandIndex, const StereoState& stereo, Vec2u framebufferSize) {
    // Check all the dirty flags and update the state accordingly
    if (_invalidViewport) {
        _camera._viewport = glm::vec4(_viewport);
    }

    if (_invalidProj) {
        _camera._projection = _projection;
    }

    if (_invalidView) {
        // Apply the correction
        if (_viewIsCamera && (_viewCorrectionEnabled && _correction.correction != glm::mat4())) {
            // FIXME should I switch to using the camera correction buffer in Transform.slf and leave this out?
            Transform result;
            _view.mult(result, _view, _correction.correctionInverse);
            if (_skybox) {
                result.setTranslation(vec3());
            }
            _view = result;
        }
        // This is when the _view matrix gets assigned
        _view.getInverseMatrix(_camera._view);
    }

    if (_invalidView || _invalidProj || _invalidViewport) {
        size_t offset = _cameraUboSize * _cameras.size();
        Vec2 finalJitter = _projectionJitter / Vec2(framebufferSize);
        _cameraOffsets.push_back(TransformStageState::Pair(commandIndex, offset));

        if (stereo.isStereo()) {
#ifdef GPU_STEREO_CAMERA_BUFFER
            _cameras.push_back(CameraBufferElement(_camera.getEyeCamera(0, stereo, _view, finalJitter), _camera.getEyeCamera(1, stereo, _view, finalJitter)));
#else
            _cameras.push_back((_camera.getEyeCamera(0, stereo, _view, finalJitter)));
            _cameras.push_back((_camera.getEyeCamera(1, stereo, _view, finalJitter)));
#endif
        } else {
#ifdef GPU_STEREO_CAMERA_BUFFER
            _cameras.push_back(CameraBufferElement(_camera.getMonoCamera(_view, finalJitter)));
#else
            _cameras.push_back((_camera.getMonoCamera(_view, finalJitter)));
#endif
        }
    }

    // Flags are clean
    _invalidView = _invalidProj = _invalidViewport = false;
}

void VKBackend::TransformStageState::update(size_t commandIndex, const StereoState& stereo, VKBackend::UniformStageState &uniform, FrameData &currentFrame) const {
    size_t offset = INVALID_OFFSET;
    while ((_camerasItr != _cameraOffsets.end()) && (commandIndex >= (*_camerasItr).first)) {
        offset = (*_camerasItr).second;
        _currentCameraOffset = offset;
        ++_camerasItr;
    }

    if (offset != INVALID_OFFSET) {
#ifdef GPU_STEREO_CAMERA_BUFFER
        bindCurrentCamera(0, uniform, currentFrame);
#else
        if (!stereo.isStereo()) {
            bindCurrentCamera(0);
        }
#endif
    }
}

void VKBackend::TransformStageState::bindCurrentCamera(int eye, VKBackend::UniformStageState &uniform, FrameData &currentFrame) const {
    if (_currentCameraOffset != INVALID_OFFSET) {
        static_assert(slot::buffer::Buffer::CameraTransform >= MAX_NUM_UNIFORM_BUFFERS, "TransformCamera may overlap pipeline uniform buffer slots. Invalidate uniform buffer slot cache for safety (call _uniform._buffers[TRANSFORM_CAMERA_SLOT].reset()).");
        // VKTODO: add convenience function for this?
        auto &buffer = uniform._buffers[slot::buffer::Buffer::CameraTransform];
        Q_ASSERT(currentFrame._cameraBuffer);
        buffer.vksBuffer = currentFrame._cameraBuffer.get();
        buffer.size = sizeof(CameraBufferElement);
        buffer.offset = _currentCameraOffset + eye * _cameraUboSize;
        //glBindBufferRange(GL_UNIFORM_BUFFER, slot::buffer::Buffer::CameraTransform, _cameraBuffer, _currentCameraOffset + eye * _cameraUboSize, sizeof(CameraBufferElement));
    }
}

void VKBackend::do_resetStages(const Batch& batch, size_t paramOffset) {
    //VKTODO: make sure all stages are reset
    //VKTODO: should inout stage be reset here?
    resetUniformStage();
    resetTextureStage();
    resetResourceStage();
    resetQueryStage();
    //resetInputStage();
    //resetPipelineStage();
    //resetTransformStage();
    //resetOutputStage();
}

void VKBackend::do_disableContextViewCorrection(const Batch& batch, size_t paramOffset) {
    _transform._viewCorrectionEnabled = false;
}

void VKBackend::do_restoreContextViewCorrection(const Batch& batch, size_t paramOffset) {
    _transform._viewCorrectionEnabled = true;
}

void VKBackend::do_disableContextStereo(const Batch& batch, size_t paramOffset) {

}

void VKBackend::do_restoreContextStereo(const Batch& batch, size_t paramOffset) {

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

void VKBackend::do_glUniform1f(const Batch& batch, size_t paramOffset) {
    qDebug() << "VKTODO: do_glUniform1f not implemented";
    /*if (_pipeline._program == 0) {
        // We should call updatePipeline() to bind the program but we are not doing that
        // because these uniform setters are deprecated and we don;t want to create side effect
        return;
    }
    updatePipeline();

    GLint location = getRealUniformLocation(batch._params[paramOffset + 1]._int);
    glUniform1f(
        location,
        batch._params[paramOffset + 0]._float);
    (void)CHECK_GL_ERROR();*/
}

void VKBackend::do_glUniform2f(const Batch& batch, size_t paramOffset) {
    qDebug() << "VKTODO: do_glUniform2f not implemented";
    /*if (_pipeline._program == 0) {
        // We should call updatePipeline() to bind the program but we are not doing that
        // because these uniform setters are deprecated and we don;t want to create side effect
        return;
    }
    updatePipeline();
    GLint location = getRealUniformLocation(batch._params[paramOffset + 2]._int);
    glUniform2f(
        location,
        batch._params[paramOffset + 1]._float,
        batch._params[paramOffset + 0]._float);
    (void)CHECK_GL_ERROR();*/
}

void VKBackend::do_glUniform3f(const Batch& batch, size_t paramOffset) {
    qDebug() << "VKTODO: do_glUniform3f not implemented";
    /*if (_pipeline._program == 0) {
        // We should call updatePipeline() to bind the program but we are not doing that
        // because these uniform setters are deprecated and we don;t want to create side effect
        return;
    }
    updatePipeline();
    GLint location = getRealUniformLocation(batch._params[paramOffset + 3]._int);
    glUniform3f(
        location,
        batch._params[paramOffset + 2]._float,
        batch._params[paramOffset + 1]._float,
        batch._params[paramOffset + 0]._float);
    (void)CHECK_GL_ERROR();*/
}

void VKBackend::do_glUniform4f(const Batch& batch, size_t paramOffset) {
    qDebug() << "VKTODO: do_glUniform4f not implemented";
    /*if (_pipeline._program == 0) {
        // We should call updatePipeline() to bind the program but we are not doing that
        // because these uniform setters are deprecated and we don;t want to create side effect
        return;
    }
    updatePipeline();
    GLint location = getRealUniformLocation(batch._params[paramOffset + 4]._int);
    glUniform4f(
        location,
        batch._params[paramOffset + 3]._float,
        batch._params[paramOffset + 2]._float,
        batch._params[paramOffset + 1]._float,
        batch._params[paramOffset + 0]._float);
    (void)CHECK_GL_ERROR();*/
}

void VKBackend::do_pushProfileRange(const Batch& batch, size_t paramOffset) {
    const auto& name = batch._profileRanges.get(batch._params[paramOffset]._uint);
    ::vks::debugutils::cmdBeginLabel(_currentCommandBuffer, name, glm::vec4{ 1.0 });
}

void VKBackend::do_popProfileRange(const Batch& batch, size_t paramOffset) {
    ::vks::debugutils::cmdEndLabel(_currentCommandBuffer);
}

void VKBackend::setCameraCorrection(const Mat4& correction, const Mat4& prevRenderView, bool reset) {
    // VKTODO
    /*auto invCorrection = glm::inverse(correction);
    auto invPrevView = glm::inverse(prevRenderView);
    _transform._correction.prevView = (reset ? Mat4() : prevRenderView);
    _transform._correction.prevViewInverse = (reset ? Mat4() : invPrevView);
    _transform._correction.correction = correction;
    _transform._correction.correctionInverse = invCorrection;
    _pipeline._cameraCorrectionBuffer._buffer->setSubData(0, _transform._correction);
    _pipeline._cameraCorrectionBuffer._buffer->flush();*/
}

void VKBackend::updateVkDescriptorWriteSetsUniform(VkDescriptorSet target) {
    // VKTODO: can be used for "verification mode" later
    // VKTODO: it looks like renderer tends to bind buffers that should not be bound at given point? Or maybe I'm missing reset somewhere
    auto pipeline = gpu::acquire(_cache.pipelineState.pipeline);
    auto program = pipeline->getProgram();
    const auto& vertexReflection = program->getShaders()[0]->getReflection();
    const auto& fragmentReflection = program->getShaders()[1]->getReflection();

    auto bindingMap = Cache::Pipeline::getBindingMap(vertexReflection.uniformBuffers, fragmentReflection.uniformBuffers);

    std::vector<VkWriteDescriptorSet> sets;
    std::vector<VkDescriptorBufferInfo> bufferInfos;
    sets.reserve(_uniform._buffers.size());
    bufferInfos.reserve(_uniform._buffers.size()); // This is to avoid vector reallocation and changing pointer adresses
    for (size_t i = 0; i < _uniform._buffers.size(); i++) {
        if ((_uniform._buffers[i].buffer || _uniform._buffers[i].vksBuffer)
            && (vertexReflection.validUniformBuffer(i) || fragmentReflection.validUniformBuffer(i))) {

            // These cannot be set at the same time
            Q_ASSERT(!(_uniform._buffers[i].buffer && _uniform._buffers[i].vksBuffer));
            // VKTODO: move vulkan buffer creation to the transfer parts and aggregate several buffers together maybe?
            VkDescriptorBufferInfo bufferInfo{};
            if (_uniform._buffers[i].buffer) {
                Q_ASSERT(i != slot::buffer::Buffer::CameraTransform);  // Camera buffer slot cannot be occupied by anything else
                VKBuffer * buffer = syncGPUObject(*_uniform._buffers[i].buffer);
                bufferInfo.buffer = buffer->buffer;
            } else if (_uniform._buffers[i].vksBuffer) {
                bufferInfo.buffer = _uniform._buffers[i].vksBuffer->buffer;
            }
            bufferInfo.offset = _uniform._buffers[i].offset;
            bufferInfo.range = _uniform._buffers[i].size;
            bufferInfos.push_back(bufferInfo);
            VkWriteDescriptorSet descriptorWriteSet{};
            descriptorWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWriteSet.dstSet = target;
            descriptorWriteSet.dstBinding = i;
            descriptorWriteSet.dstArrayElement = 0;
            descriptorWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWriteSet.descriptorCount = 1;
            descriptorWriteSet.pBufferInfo = &bufferInfos.back();
            sets.push_back(descriptorWriteSet);
        }
    }
    vkUpdateDescriptorSets(_context.device->logicalDevice, sets.size(), sets.data(), 0, nullptr);
}

void VKBackend::updateVkDescriptorWriteSetsTexture(VkDescriptorSet target) {
    // VKTODO: renderer leaves unbound texture slots, and that's not allowed on Vulkan
    // VKTODO: can be used for "verification mode" later
    // VKTODO: it looks like renderer tends to bind buffers that should not be bound at given point? Or maybe I'm missing reset somewhere
    auto pipeline = gpu::acquire(_cache.pipelineState.pipeline);
    auto program = pipeline->getProgram();
    const auto& vertexReflection = program->getShaders()[0]->getReflection();
    const auto& fragmentReflection = program->getShaders()[1]->getReflection();

    auto bindingMap = Cache::Pipeline::getBindingMap(vertexReflection.textures, fragmentReflection.textures);

    std::vector<VkWriteDescriptorSet> sets;
    std::vector<VkDescriptorImageInfo> imageInfos;
    sets.reserve(_uniform._buffers.size());
    imageInfos.reserve(_uniform._buffers.size()); // This is to avoid vector reallocation and changing pointer adresses
    for (size_t i = 0; i < _resource._textures.size(); i++) {
        if (_resource._textures[i].texture && (vertexReflection.validTexture(i) || fragmentReflection.validTexture(i))) {
            // VKTODO: move vulkan texture creation to the transfer parts
            // VKTODO: this doesn't work yet
            VKTexture* texture = syncGPUObject(*_resource._textures[i].texture);
            VkDescriptorImageInfo imageInfo{};
            if (texture) {
                qDebug() << "Writing descriptor " << i << " with texture: " << _resource._textures[i].texture->source();
                imageInfo = texture->getDescriptorImageInfo();
            } else {
                if (_resource._textures[i].texture) {
                    qDebug() << "Cannot sync texture during descriptor " << i
                             << " write: " << _resource._textures[i].texture->source();
                } else {
                    qDebug() << "Texture is null during descriptor " << i
                             << " write: " << _resource._textures[i].texture->source();
                }
                imageInfo = _defaultTexture.descriptor;
            }
            //imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            //imageInfo.imageView = texture->;
            //imageInfo.sampler = _defaultTexture.sampler;
            imageInfos.push_back(imageInfo);

            VkWriteDescriptorSet descriptorWriteSet{};
            descriptorWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWriteSet.dstSet = target;
            descriptorWriteSet.dstBinding = i;
            descriptorWriteSet.dstArrayElement = 0;
            descriptorWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWriteSet.descriptorCount = 1;
            descriptorWriteSet.pImageInfo = &imageInfos.back();
            sets.push_back(descriptorWriteSet);
        } else {
            auto binding = bindingMap.find(i);
            if (binding != bindingMap.end()) {
                // VKTODO: fill unbound but needed slots with default texture
                VkWriteDescriptorSet descriptorWriteSet{};
                descriptorWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWriteSet.dstSet = target;
                descriptorWriteSet.dstBinding = i;
                descriptorWriteSet.dstArrayElement = 0;
                descriptorWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWriteSet.descriptorCount = 1;
                descriptorWriteSet.pImageInfo = &_defaultTexture.descriptor;
                sets.push_back(descriptorWriteSet);
            }
        }
    }
    vkUpdateDescriptorSets(_context.device->logicalDevice, sets.size(), sets.data(), 0, nullptr);
}

void VKBackend::updateVkDescriptorWriteSetsStorage(VkDescriptorSet target) {
    // VKTODO: can be used for "verification mode" later
    // VKTODO: it looks like renderer tends to bind buffers that should not be bound at given point? Or maybe I'm missing reset somewhere
    auto pipeline = gpu::acquire(_cache.pipelineState.pipeline);
    auto program = pipeline->getProgram();
    const auto& vertexReflection = program->getShaders()[0]->getReflection();
    const auto& fragmentReflection = program->getShaders()[1]->getReflection();

    auto bindingMap = Cache::Pipeline::getBindingMap(vertexReflection.resourceBuffers, fragmentReflection.resourceBuffers);

    std::vector<VkWriteDescriptorSet> sets;
    std::vector<VkDescriptorBufferInfo> bufferInfos;
    sets.reserve(_uniform._buffers.size());
    bufferInfos.reserve(_uniform._buffers.size()); // This is to avoid vector reallocation and changing pointer adresses
    for (size_t i = 0; i < _resource._buffers.size(); i++) {
        if ((_resource._buffers[i].buffer || _resource._buffers[i].vksBuffer)
            && (vertexReflection.validUniformBuffer(i) || fragmentReflection.validUniformBuffer(i))) {

            Q_ASSERT(!(_resource._buffers[i].buffer && _resource._buffers[i].vksBuffer));
            // VKTODO: move vulkan buffer creation to the transfer parts and aggregate several buffers together maybe?
            VkDescriptorBufferInfo bufferInfo{};
            if (_resource._buffers[i].buffer) {
                VKBuffer* buffer = syncGPUObject(*_resource._buffers[i].buffer);
                bufferInfo.buffer = buffer->buffer;
                bufferInfo.range = _resource._buffers[i].buffer->getSize();
            } else if (_resource._buffers[i].vksBuffer)
            {
                bufferInfo.buffer = _resource._buffers[i].vksBuffer->buffer;
                bufferInfo.range = _resource._buffers[i].vksBuffer->size;
            }
            bufferInfo.offset = 0;
            bufferInfos.push_back(bufferInfo);

            VkWriteDescriptorSet descriptorWriteSet{};
            descriptorWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWriteSet.dstSet = target;
            descriptorWriteSet.dstBinding = i;
            descriptorWriteSet.dstArrayElement = 0;
            descriptorWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            descriptorWriteSet.descriptorCount = 1;
            descriptorWriteSet.pBufferInfo = &bufferInfos.back();
            sets.push_back(descriptorWriteSet);
        }
    }
    vkUpdateDescriptorSets(_context.device->logicalDevice, sets.size(), sets.data(), 0, nullptr);
}


void VKBackend::releaseUniformBuffer(uint32_t slot) {
    auto& bufferState = _uniform._buffers[slot];
    if (valid(bufferState.buffer)) {
        // VKTODO
        //glBindBufferBase(GL_UNIFORM_BUFFER, slot, 0);  // RELEASE
        //(void)CHECK_GL_ERROR();
    }
    bufferState.reset();
}

void VKBackend::resetUniformStage() {
    for (auto &buffer: _uniform._buffers) {
        buffer.reset();
    }
}

void VKBackend::bindResourceTexture(uint32_t slot, const gpu::TexturePointer& texture) {
    qDebug() << "bindResourceTexture: " << slot;
    if (!texture) {
        releaseResourceTexture(slot);
        return;
    }
    // check cache before thinking
    if (_resource._textures[slot].texture == texture.get()) {
        return;
    }

    // One more True texture bound
    _stats._RSNumTextureBounded++;

    // Always make sure the VKObject is in sync
    // VKTODO
    //VKTexture* object = syncGPUObject(resourceTexture);
    //if (object) {
    //uint32_t to = object->_texture;
    //uint32_t target = object->_target;
    //glActiveTexture(VK_TEXTURE0 + slot);
    //glBindTexture(target, to);

    _resource._textures[slot].texture = texture.get();

    //_stats._RSAmountTextureMemoryBounded += object->size();

    //} else {
    //    releaseResourceTexture(slot);
    //    return;
    //}
}

void VKBackend::releaseResourceTexture(uint32_t slot) {
    auto& textureState = _resource._textures[slot];
    if (valid(textureState.texture)) {
        // VKTODO
        //glActiveTexture(GL_TEXTURE0 + slot);
        //glBindTexture(textureState._target, 0);  // RELEASE
        //(void)CHECK_GL_ERROR();
        reset(textureState.texture);
    }
}

void VKBackend::resetTextureStage() {
    for (auto &texture : _resource._textures) {
        texture.reset();
    }
}

void VKBackend::releaseResourceBuffer(uint32_t slot) {
    auto& bufferReference = _resource._buffers[slot].buffer;
    auto buffer = acquire(bufferReference);
    if (buffer) {
        // VKTODO
        //glActiveTexture(GL_TEXTURE0 + GLESBackend::RESOURCE_BUFFER_SLOT0_TEX_UNIT + slot);
        //glBindTexture(GL_TEXTURE_BUFFER, 0);
        reset(bufferReference);
    }
    if (_resource._buffers[slot].vksBuffer) {
        reset(_resource._buffers[slot].vksBuffer);
    }
}

void VKBackend::resetResourceStage() {
    for (auto &buffer : _resource._buffers) {
        buffer.reset();
    }
}

void VKBackend::resetQueryStage() {
    _queryStage._rangeQueryDepth = 0;
}

void VKBackend::updateRenderPass() {
    auto renderPass = _cache.pipelineState.getRenderPass(_context);

    // Current render pass is already up to date
    // VKTODO: check if framebuffer has changed and if so update render pass too
    if (_currentRenderPass == renderPass) {
        return;
    }

    // Current render pass needs to be finished before starting new one
    if (_currentRenderPass) {
        vkCmdEndRenderPass(_currentCommandBuffer);
    }

    _currentRenderPass = renderPass;

    auto renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
    renderPassBeginInfo.renderPass = renderPass;
    Q_ASSERT(_cache.pipelineState.framebuffer);
    auto framebuffer = VKFramebuffer::sync(*this, *_cache.pipelineState.framebuffer);
    Q_ASSERT(framebuffer);
    renderPassBeginInfo.framebuffer = framebuffer->vkFramebuffer;
    renderPassBeginInfo.clearValueCount = framebuffer->attachments.size();
    std::vector<VkClearValue> clearValues;
    clearValues.resize(framebuffer->attachments.size());
    for (size_t i = 0; i < framebuffer->attachments.size(); i++) {
        if (framebuffer->attachments[i].isDepthStencil()) {
            clearValues[i].depthStencil = { 1.0f, 0 };
        } else {
            clearValues[i].color = { { 0.2f, 0.5f, 0.1f, 1.0f } };
        }
    }
    renderPassBeginInfo.pClearValues = clearValues.data();
    renderPassBeginInfo.renderArea = VkRect2D{VkOffset2D {_transform._viewport.x, _transform._viewport.y}, VkExtent2D {(uint32_t)_transform._viewport.z, (uint32_t)_transform._viewport.w}};
    vkCmdBeginRenderPass(_currentCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VKBackend::resetRenderPass() {
    if (_currentRenderPass) {
        _currentRenderPass = VK_NULL_HANDLE;
        vkCmdEndRenderPass(_currentCommandBuffer);
    }
}

void VKBackend::renderPassTransfer(const Batch& batch) {
    const size_t numCommands = batch.getCommands().size();
    const Batch::Commands::value_type* command = batch.getCommands().data();
    const Batch::CommandOffsets::value_type* offset = batch.getCommandOffsets().data();

    _inRenderTransferPass = true;
    { // Sync all the buffers
        PROFILE_RANGE(gpu_vk_detail, "syncGPUBuffer");
        // VKTODO: this is filling entire GPU VRAM for some reason
        /*for (auto& cached : batch._buffers._items) {
            if (cached._data) {
                syncGPUObject(*cached._data);
            }
        }*/
    }

    { // Sync all the buffers
        PROFILE_RANGE(gpu_vk_detail, "syncCPUTransform");
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
                // VKTODO: pass current framebuffer size
                _transform.preUpdate(_commandIndex, _stereo, Vec2u(640, 480));
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
        PROFILE_RANGE(gpu_vk_detail, "syncGPUTransform");
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
            if (_cache.pipelineState.framebuffer->getRenderBuffers()[0]._texture->getTexelFormat().getSemantic() == gpu::R11G11B10) {
                printf("Test");
            }
            updateRenderPass();
            // VKTODO: this is inefficient
            vkCmdBindPipeline(_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _cache.getPipeline(_context));
            // VKTODO: this will create too many set viewport commands, but should work
            VkViewport viewport;
            viewport.x = (float)_transform._viewport.x;
            viewport.y = (float)_transform._viewport.y;
            viewport.width = (float)_transform._viewport.z;
            viewport.height = (float)_transform._viewport.w;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(_currentCommandBuffer, 0, 1, &viewport);
            // VKTODO: this will create too many set scissor commands, but should work
            VkRect2D scissor;
            scissor.offset.x = _currentScissorRect.x;
            scissor.offset.y = _currentScissorRect.y;
            scissor.extent.width = _currentScissorRect.z;
            scissor.extent.height = _currentScissorRect.w;
            vkCmdSetScissor(_currentCommandBuffer, 0, 1, &scissor);
            auto layout = _cache.pipelineState.getPipelineAndDescriptorLayout(_context);
            // VKTODO: Descriptor sets and associated buffers should be set up during pre-pass
            // VKTODO: move this to a function
            if (layout.uniformLayout) {
                // TODO: allocate 3 at once?
                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = _currentFrame->_descriptorPool;
                allocInfo.descriptorSetCount = 1;
                allocInfo.pSetLayouts = &layout.uniformLayout;
                VkDescriptorSet descriptorSet;
                VK_CHECK_RESULT(vkAllocateDescriptorSets(_context.device->logicalDevice, &allocInfo, &descriptorSet));

                updateVkDescriptorWriteSetsUniform(descriptorSet);
                vkCmdBindDescriptorSets(_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout.pipelineLayout, 0, 1,
                                        &descriptorSet, 0, nullptr);
            }
            if (layout.textureLayout) {
                // TODO: allocate 3 at once?
                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = _currentFrame->_descriptorPool;
                allocInfo.descriptorSetCount = 1;
                allocInfo.pSetLayouts = &layout.textureLayout;
                VkDescriptorSet descriptorSet;
                VK_CHECK_RESULT(vkAllocateDescriptorSets(_context.device->logicalDevice, &allocInfo, &descriptorSet));

                updateVkDescriptorWriteSetsTexture(descriptorSet);
                vkCmdBindDescriptorSets(_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout.pipelineLayout, 1, 1,
                                        &descriptorSet, 0, nullptr);
            }
            if (layout.storageLayout) {
                // TODO: allocate 3 at once?
                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = _currentFrame->_descriptorPool;
                allocInfo.descriptorSetCount = 1;
                allocInfo.pSetLayouts = &layout.storageLayout;
                VkDescriptorSet descriptorSet;
                VK_CHECK_RESULT(vkAllocateDescriptorSets(_context.device->logicalDevice, &allocInfo, &descriptorSet));

                updateVkDescriptorWriteSetsStorage(descriptorSet);
                vkCmdBindDescriptorSets(_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout.pipelineLayout, 2, 1,
                                        &descriptorSet, 0, nullptr);
            }
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
    resetRenderPass();
}

void VKBackend::draw(VkPrimitiveTopology mode, uint32 numVertices, uint32 startVertex) {
    // VKTODO: no stereo for now
    if (isStereo()) {
#ifdef GPU_STEREO_DRAWCALL_INSTANCED
        // VKTODO: how to set mode, in VkPipelineInputAssemblyStateCreateInfo, part of VkGraphicsPipelineCreateInfo?
        // That would require pipelines generated for all cases
        vkCmdDraw(_currentCommandBuffer, numVertices, 2, startVertex, 0);
#else
        setupStereoSide(0);
        glDrawArrays(mode, startVertex, numVertices);
        setupStereoSide(1);
        glDrawArrays(mode, startVertex, numVertices);
#endif

        _stats._DSNumTriangles += 2 * numVertices / 3;
        _stats._DSNumDrawcalls += 2;

    } else {
        // VKTODO: how to set mode, in VkPipelineInputAssemblyStateCreateInfo, part of VkGraphicsPipelineCreateInfo?
        // That would require pipelines generated for all cases
        vkCmdDraw(_currentCommandBuffer, numVertices, 1, startVertex, 0);
        _stats._DSNumTriangles += numVertices / 3;
        _stats._DSNumDrawcalls++;
    }
    _stats._DSNumAPIDrawcalls++;
}

#ifdef GPU_STEREO_DRAWCALL_DOUBLED
void VKBackend::setupStereoSide(int side) {
    ivec4 vp = _transform._viewport;
    vp.z /= 2;
    VkViewport viewport;
    viewport.x = (float)(vp.x + side * vp.z);
    viewport.y = (float)(vp.y);
    viewport.width = (float)(vp.z);
    viewport.height = vp.w;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    //VKTODO: there can be multiple viewports
    vkCmdSetViewport(_currentCommandBuffer, 0, 1, &viewport);

    _transform.bindCurrentCamera(side);
}
#endif

vk::VKFramebuffer* VKBackend::syncGPUObject(const Framebuffer& framebuffer) {
    // VKTODO
    return vk::VKFramebuffer::sync(*this, framebuffer);
}

VKBuffer* VKBackend::syncGPUObject(const Buffer& buffer) {
    // VKTODO
    return vk::VKBuffer::sync(*this, buffer);
}

VKTexture* VKBackend::syncGPUObject(const Texture& texture) {
    // VKTODO
    /*if (!texture) {
        return nullptr;
    }*/

    if (TextureUsageType::EXTERNAL == texture.getUsageType()) {
        // VKTODO:
        return nullptr;
        //return Parent::syncGPUObject(texturePointer);
    }

    if (!texture.isDefined()) {
        // NO texture definition yet so let's avoid thinking
        return nullptr;
    }

    VKTexture* object = Backend::getGPUObject<VKTexture>(texture);
    // VKTODO: check object->_storageStamp to see if texture is outdated
    if (!object) {
        switch (texture.getUsageType()) {
            case TextureUsageType::RENDERBUFFER:
                object = new VKAttachmentTexture(shared_from_this(), texture);
                break;

#if FORCE_STRICT_TEXTURE
            case TextureUsageType::RESOURCE:
#endif
            case TextureUsageType::STRICT_RESOURCE:

                // Stored size can sometimes be reported as 0 for valid textures.
                if (texture.getStoredSize() == 0 && texture.getStoredMipFormat() == gpu::Element()){
                    qDebug(gpu_vk_logging) << "No data on texture";
                    texture.getStoredMipFormat();
                    texture.getStoredSize();
                    return nullptr;
                }

                if (evalTexelFormatInternal(texture.getStoredMipFormat()) != evalTexelFormatInternal(texture.getTexelFormat())) {
                    qDebug() << "Format mismatch, stored: " << evalTexelFormatInternal(texture.getStoredMipFormat()) << " texel: " << evalTexelFormatInternal(texture.getTexelFormat());
                    return nullptr;
                }

                // VKTODO: What is strict resource?
                qWarning() << "TextureUsageType::STRICT_RESOURCE";
                qCDebug(gpu_vk_logging) << "Strict texture " << texture.source().c_str();
                object = new VKStrictResourceTexture(shared_from_this(), texture);
                break;

#if !FORCE_STRICT_TEXTURE
            case TextureUsageType::RESOURCE: {
                // VKTODO
                /*auto& transferEngine  = _textureManagement._transferEngine;
                if (transferEngine->allowCreate()) {
#if ENABLE_SPARSE_TEXTURE
                    if (isTextureManagementSparseEnabled() && GL45Texture::isSparseEligible(texture)) {
                        object = new GL45SparseResourceTexture(shared_from_this(), texture);
                    } else {
                        object = new GL45ResourceTexture(shared_from_this(), texture);
                    }
#else
                    object = new GL45ResourceTexture(shared_from_this(), texture);
#endif
                    transferEngine->addMemoryManagedTexture(texturePointer);
                } else {
                    auto fallback = texturePointer->getFallbackTexture();
                    if (fallback) {
                        object = static_cast<GL45Texture*>(syncGPUObject(fallback));
                    }
                }*/
                break;
            }
#endif
            default:
                Q_UNREACHABLE();
        }
    } else {

        if (texture.getUsageType() == TextureUsageType::RESOURCE) {
            // VKTODO
            /*auto varTex = static_cast<GL45VariableAllocationTexture*> (object);

            if (varTex->_minAllocatedMip > 0) {
                auto minAvailableMip = texture.minAvailableMipLevel();
                if (minAvailableMip < varTex->_minAllocatedMip) {
                    varTex->_minAllocatedMip = minAvailableMip;
                }
            }*/
        }
    }

    return object;
}

VKQuery* VKBackend::syncGPUObject(const Query& query) {
    // VKTODO
    return vk::VKQuery::sync(*this, query);
}

void VKBackend::updateInput() {
    // VKTODO
    bool isStereoNow = isStereo();
    // track stereo state change potentially happening without changing the input format
    // this is a rare case requesting to invalid the format
#ifdef GPU_STEREO_DRAWCALL_INSTANCED
    _input._invalidFormat |= (isStereoNow != _input._lastUpdateStereoState);
#endif
    _input._lastUpdateStereoState = isStereoNow;

    /*if (_input._invalidFormat) {
        InputStageState::ActivationCache newActivation;

        // Assign the vertex format required
        auto format = acquire(_input._format);
        if (format) {
            _input._attribBindingBuffers.reset();

            const auto& attributes = format->getAttributes();
            const auto& inputChannels = format->getChannels();
            for (auto& channelIt : inputChannels) {
                auto bufferChannelNum = (channelIt).first;
                const Stream::Format::ChannelMap::value_type::second_type& channel = (channelIt).second;
                _input._attribBindingBuffers.set(bufferChannelNum);

                GLuint frequency = 0;
                for (unsigned int i = 0; i < channel._slots.size(); i++) {
                    const Stream::Attribute& attrib = attributes.at(channel._slots[i]);

                    GLuint slot = attrib._slot;
                    GLuint count = attrib._element.getLocationScalarCount();
                    uint8_t locationCount = attrib._element.getLocationCount();
                    GLenum type = gl::ELEMENT_TYPE_TO_GL[attrib._element.getType()];

                    GLuint offset = (GLuint)attrib._offset;
                    GLboolean isNormalized = attrib._element.isNormalized();

                    GLenum perLocationSize = attrib._element.getLocationSize();

                    for (GLuint locNum = 0; locNum < locationCount; ++locNum) {
                        GLuint attriNum = (GLuint)(slot + locNum);
                        newActivation.set(attriNum);
                        if (!_input._attributeActivation[attriNum]) {
                            _input._attributeActivation.set(attriNum);
                            glEnableVertexAttribArray(attriNum);
                        }
                        if (attrib._element.isInteger()) {
                            glVertexAttribIFormat(attriNum, count, type, offset + locNum * perLocationSize);
                        } else {
                            glVertexAttribFormat(attriNum, count, type, isNormalized, offset + locNum * perLocationSize);
                        }
                        glVertexAttribBinding(attriNum, attrib._channel);
                    }

                    if (i == 0) {
                        frequency = attrib._frequency;
                    } else {
                        assert(frequency == attrib._frequency);
                    }

                }
#ifdef GPU_STEREO_DRAWCALL_INSTANCED
                glVertexBindingDivisor(bufferChannelNum, frequency * (isStereoNow ? 2 : 1));
#else
                glVertexBindingDivisor(bufferChannelNum, frequency);
#endif
            }
        }

        // Manage Activation what was and what is expected now
        // This should only disable VertexAttribs since the one needed by the vertex format (if it exists) have been enabled above
        for (GLuint i = 0; i < (GLuint)newActivation.size(); i++) {
            bool newState = newActivation[i];
            if (newState != _input._attributeActivation[i]) {
                if (newState) {
                    glEnableVertexAttribArray(i);
                } else {
                    glDisableVertexAttribArray(i);
                }
                _input._attributeActivation.flip(i);
            }
        }

        _input._invalidFormat = false;
        _stats._ISNumFormatChanges++;
    }*/

    if (_input._invalidBuffers.any()) {
        auto vbo = _input._bufferVBOs.data();
        auto offset = _input._bufferOffsets.data();
        auto stride = _input._bufferStrides.data();

        // Profile the count of buffers to update and use it to short cut the for loop
        int numInvalids = (int) _input._invalidBuffers.count();
        _stats._ISNumInputBufferChanges += numInvalids;

        for (size_t buffer = 0; buffer < _input._buffers.size(); buffer++, vbo++, offset++, stride++) {
            if (_input._invalidBuffers.test(buffer)) {
                auto vkBuffer = VKBuffer::getBuffer(*this, *_input._buffers[buffer]);
                qDebug() << "Vertex buffer size: " << _input._buffers[buffer]->getSize();
                qDebug() << "Vertex buffer usage: " << _input._buffers[buffer]->getUsage();
                VkDeviceSize vkOffset = _input._bufferOffsets[buffer];
                vkCmdBindVertexBuffers(_currentCommandBuffer, buffer, 1, &vkBuffer, &vkOffset);
                //glBindVertexBuffer(buffer, (*vbo), (*offset), (GLsizei)(*stride));
                numInvalids--;
                if (numInvalids <= 0) {
                    break;
                }
            }
        }

        _input._invalidBuffers.reset();
    }
}

void VKBackend::FrameData::createDescriptorPool() {
    std::vector<VkDescriptorPoolSize> poolSizes = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 50000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 50000 }
    };

    VkDescriptorPoolCreateInfo descriptorPoolCI = {};
    descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCI.flags = 0;
    descriptorPoolCI.maxSets = 10000;
    descriptorPoolCI.poolSizeCount = (uint32_t)poolSizes.size();
    descriptorPoolCI.pPoolSizes = poolSizes.data();

    VK_CHECK_RESULT(vkCreateDescriptorPool(_backend->_context.device->logicalDevice, &descriptorPoolCI, nullptr, &_descriptorPool));
}

VKBackend::FrameData::FrameData(VKBackend *backend) : _backend(backend) {
    createDescriptorPool();
}

VKBackend::FrameData::~FrameData() {
    cleanup();
    vkDestroyDescriptorPool(_backend->_context.device->logicalDevice, _descriptorPool, nullptr);
    if (_objectBuffer) {
        _objectBuffer->destroy();
        _objectBuffer.reset();
    }
    if (_cameraBuffer) {
        _cameraBuffer->destroy();
        _cameraBuffer.reset();
    }
    if (_drawCallInfoBuffer) {
        _drawCallInfoBuffer->destroy();
        _drawCallInfoBuffer.reset();
    }
}

void VKBackend::FrameData::cleanup() {
    for (auto renderPass : _renderPasses) {
        vkDestroyRenderPass(_backend->_context.device->logicalDevice, renderPass, nullptr);
    }
    _renderPasses.resize(0);

    uniformDescriptorSets.resize(0);
    textureDescriptorSets.resize(0);
    storageDescriptorSets.resize(0);
    // Should descriptor pool be cleared every frame?
    vkResetDescriptorPool(_backend->_context.device->logicalDevice, _descriptorPool, 0);
}

void VKBackend::initDefaultTexture() {
    int width = 1;
    int height = 1;
    std::vector<uint8_t> buffer;
    buffer.resize(width * height * 4);
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            buffer[x + y * width] = 0;
            buffer[x + y * width + 1] = 0;
            buffer[x + y * width + 2] = 0;
            buffer[x + y * width + 3] = 255;
        }
    }
    _defaultTexture.fromBuffer(buffer.data(), buffer.size(), VK_FORMAT_R8G8B8A8_SRGB, width, height, _context.device.get(),  _context.transferQueue);
}

void VKBackend::acquireFrameData() {
    Q_ASSERT(!_framesToReuse.empty());
    _currentFrame = _framesToReuse.front();
    _framesToReuse.pop_front();
}

void VKBackend::recycleFrame() {
    if (_currentlyRenderedFrame) {
        _currentlyRenderedFrame->cleanup();
        _framesToReuse.push_back(_currentlyRenderedFrame);
        _currentlyRenderedFrame.reset();
    }
}

void VKBackend::waitForGPU() {
    VK_CHECK_RESULT(vkQueueWaitIdle(_context.graphicsQueue));
    VK_CHECK_RESULT(vkQueueWaitIdle(_context.transferQueue));
    VK_CHECK_RESULT(vkDeviceWaitIdle(_context.device->logicalDevice));
}


void VKBackend::initTransform() {

#ifdef GPU_SSBO_TRANSFORM_OBJECT
//#else
////    glCreateTextures(GL_TEXTURE_BUFFER, 1, &_transform._objectBufferTexture);
#endif
    size_t cameraSize = sizeof(TransformStageState::CameraBufferElement);
    while (_transform._cameraUboSize < cameraSize) {
        _transform._cameraUboSize += UNIFORM_BUFFER_OFFSET_ALIGNMENT;
    }
}

void VKBackend::updateTransform(const gpu::Batch& batch) {
    // VKTODO
    _transform.update(_commandIndex, _stereo, _uniform, *_currentFrame);

    auto& drawCallInfoBuffer = batch.getDrawCallInfoBuffer();
    if (batch._currentNamedCall.empty()) {
        //auto& drawCallInfo = drawCallInfoBuffer[_currentDraw];
        if (_transform._enabledDrawcallInfoBuffer) {
            //glDisableVertexAttribArray(gpu::Stream::DRAW_CALL_INFO); // Make sure attrib array is disabled
            _transform._enabledDrawcallInfoBuffer = false;
        }
        // VKTODO
        // Since Vulkan has no glVertexAttrib equivalent we need to pass a buffer pointer here
        //glVertexAttribI2i(gpu::Stream::DRAW_CALL_INFO, drawCallInfo.index, drawCallInfo.unused);
        qDebug() << "drawCallInfo.unused: " << drawCallInfoBuffer[_currentDraw].unused;
        // Draw call info for unnamed calls starts at the beginning of the buffer, with offset dependent on _currentDraw
        VkDeviceSize vkOffset = _currentDraw * sizeof(gpu::Batch::DrawCallInfo);
        Q_ASSERT(_currentFrame->_drawCallInfoBuffer);
        vkCmdBindVertexBuffers(_currentCommandBuffer, gpu::Stream::DRAW_CALL_INFO, 1, &_currentFrame->_drawCallInfoBuffer->buffer, &vkOffset);
    } else {
        if (!_transform._enabledDrawcallInfoBuffer) {
            //glEnableVertexAttribArray(gpu::Stream::DRAW_CALL_INFO); // Make sure attrib array is enabled
            //glVertexAttribIFormat(gpu::Stream::DRAW_CALL_INFO, 2, GL_UNSIGNED_SHORT, 0);
            //glVertexAttribBinding(gpu::Stream::DRAW_CALL_INFO, gpu::Stream::DRAW_CALL_INFO);
#ifdef GPU_STEREO_DRAWCALL_INSTANCED
            //glVertexBindingDivisor(gpu::Stream::DRAW_CALL_INFO, (isStereo() ? 2 : 1));
#else
            //glVertexBindingDivisor(gpu::Stream::DRAW_CALL_INFO, 1);
#endif
            _transform._enabledDrawcallInfoBuffer = true;
        }
        // NOTE: A stride of zero in BindVertexBuffer signifies that all elements are sourced from the same location,
        //       so we must provide a stride.
        //       This is in contrast to VertexAttrib*Pointer, where a zero signifies tightly-packed elements.
        // VKTODO: _drawCallInfoBuffer is empty currently
        VkDeviceSize vkOffset = _transform._drawCallInfoOffsets[batch._currentNamedCall];
        Q_ASSERT(_currentFrame->_drawCallInfoBuffer);
        vkCmdBindVertexBuffers(_currentCommandBuffer, gpu::Stream::DRAW_CALL_INFO, 1, &_currentFrame->_drawCallInfoBuffer->buffer, &vkOffset);
        //glBindVertexBuffer(gpu::Stream::DRAW_CALL_INFO, _transform._drawCallInfoBuffer, (GLintptr)_transform._drawCallInfoOffsets[batch._currentNamedCall], 2 * sizeof(GLushort));
    }
}

void VKBackend::updatePipeline() {
    // VKTODO
    /*if (_pipeline._invalidProgram) {
        // doing it here is aproblem for calls to glUniform.... so will do it on assing...
        glUseProgram(_pipeline._program);
        (void)CHECK_GL_ERROR();
        _pipeline._invalidProgram = false;
    }

    if (_pipeline._invalidState) {
        if (_pipeline._state) {
            // first reset to default what should be
            // the fields which were not to default and are default now
            resetPipelineState(_pipeline._state->_signature);

            // Update the signature cache with what's going to be touched
            _pipeline._stateSignatureCache |= _pipeline._state->_signature;

            // And perform
            for (const auto& command : _pipeline._state->_commands) {
                command->run(this);
            }
        } else {
            // No state ? anyway just reset everything
            resetPipelineState(0);
        }
        _pipeline._invalidState = false;
    }*/
}

void VKBackend::transferTransformState(const Batch& batch) {
    // VKTODO
    // FIXME not thread safe
    static std::vector<uint8_t> bufferData;
    if (!_transform._cameras.empty()) {
        bufferData.resize(_transform._cameraUboSize * _transform._cameras.size());
        for (size_t i = 0; i < _transform._cameras.size(); ++i) {
            memcpy(bufferData.data() + (_transform._cameraUboSize * i), &_transform._cameras[i], sizeof(TransformStageState::CameraBufferElement));
        }
        _currentFrame->_cameraBuffer = vks::Buffer::createUniform(bufferData.size());
        _currentFrame->_cameraBuffer->map();
        _currentFrame->_cameraBuffer->copy(bufferData.size(), bufferData.data());
        _currentFrame->_cameraBuffer->flush(VK_WHOLE_SIZE);
        _currentFrame->_cameraBuffer->unmap();
    }else{
        _currentFrame->_cameraBuffer.reset();
    }

    if (!batch._objects.empty()) {
        _currentFrame->_objectBuffer = vks::Buffer::createStorage(batch._objects.size() * sizeof(Batch::TransformObject));
        _currentFrame->_objectBuffer->map();
        _currentFrame->_objectBuffer->copy(batch._objects.size() * sizeof(Batch::TransformObject), batch._objects.data());
        _currentFrame->_objectBuffer->flush(VK_WHOLE_SIZE);
        _currentFrame->_objectBuffer->unmap();
        //glNamedBufferData(_transform._objectBuffer, batch._objects.size() * sizeof(Batch::TransformObject), batch._objects.data(), GL_STREAM_DRAW);
    }else{
        _currentFrame->_objectBuffer.reset();
    }

    if (!batch._namedData.empty() || !batch._drawCallInfos.empty()) {
        bufferData.clear();
        bufferData.reserve(batch._drawCallInfos.size() * sizeof(Batch::DrawCallInfo));
        // VKTODO
        {
            auto currentSize = bufferData.size();
            auto bytesToCopy = batch._drawCallInfos.size() * sizeof(Batch::DrawCallInfo);
            bufferData.resize(currentSize + bytesToCopy);
            memcpy(bufferData.data() + currentSize, batch._drawCallInfos.data(), bytesToCopy);
        }
        for (auto& data : batch._namedData) {
            auto currentSize = bufferData.size();
            auto bytesToCopy = data.second.drawCallInfos.size() * sizeof(Batch::DrawCallInfo);
            bufferData.resize(currentSize + bytesToCopy);
            memcpy(bufferData.data() + currentSize, data.second.drawCallInfos.data(), bytesToCopy);
            _transform._drawCallInfoOffsets[data.first] = currentSize;
        }
        //_transform._drawCallInfoBuffer = std::make_shared<vks::Buffer>();
        //_frameData._buffers.push_back(_transform._drawCallInfoBuffer);
        _currentFrame->_drawCallInfoBuffer = vks::Buffer::createVertex(bufferData.size());
        _currentFrame->_drawCallInfoBuffer->map();
        _currentFrame->_drawCallInfoBuffer->copy(bufferData.size(), bufferData.data());
        _currentFrame->_drawCallInfoBuffer->flush(VK_WHOLE_SIZE);
        _currentFrame->_drawCallInfoBuffer->unmap();
        //glNamedBufferData(_transform._drawCallInfoBuffer, bufferData.size(), bufferData.data(), GL_STREAM_DRAW);
    }else{
        _currentFrame->_drawCallInfoBuffer.reset();
    }

    // VKTODO
    if (_currentFrame->_objectBuffer) {
        _resource._buffers[slot::storage::ObjectTransforms].vksBuffer = _currentFrame->_objectBuffer.get();
    }
    //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot::storage::ObjectTransforms, _transform._objectBuffer);

    // Make sure the current Camera offset is unknown before render Draw
    _transform._currentCameraOffset = INVALID_OFFSET;
}


void VKBackend::downloadFramebuffer(const FramebufferPointer& srcFramebuffer, const Vec4i& region, QImage& destImage) {
    // VKTODO
}

void VKBackend::do_draw(const Batch& batch, size_t paramOffset) {
    auto primitiveType = (Primitive)batch._params[paramOffset + 2]._uint;
    VkPrimitiveTopology mode = PRIMITIVE_TO_VK[primitiveType];
    uint32 numVertices = batch._params[paramOffset + 1]._uint;
    uint32 startVertex = batch._params[paramOffset + 0]._uint;

    draw(mode, numVertices, startVertex);
}

void VKBackend::do_drawIndexed(const Batch& batch, size_t paramOffset) {
    Primitive primitiveType = (Primitive)batch._params[paramOffset + 2]._uint;
    VkPrimitiveTopology mode = PRIMITIVE_TO_VK[primitiveType];
    uint32 numIndices = batch._params[paramOffset + 1]._uint;
    uint32 startIndex = batch._params[paramOffset + 0]._uint;

    // VKTODO: index element type is set on index buffer binding with vkCmdBindIndexBuffer.
    //GLenum glType = gl::ELEMENT_TYPE_TO_VK_INDEX_TYPE[_input._indexBufferType];

    //auto typeByteSize = TYPE_SIZE[_input._indexBufferType];
    //GLvoid* indexBufferByteOffset = reinterpret_cast<GLvoid*>(startIndex * typeByteSize + _input._indexBufferOffset);

    if (isStereo()) {
#ifdef GPU_STEREO_DRAWCALL_INSTANCED
        // VKTODO: what should vertexOffest be?
        // VKTODO: mode needs to be in the pipeline
        vkCmdDrawIndexed(_currentCommandBuffer, numIndices, 2, startIndex, 0, 0);
#else
        setupStereoSide(0);
        glDrawElements(mode, numIndices, glType, indexBufferByteOffset);
        setupStereoSide(1);
        glDrawElements(mode, numIndices, glType, indexBufferByteOffset);
#endif
        _stats._DSNumTriangles += 2 * numIndices / 3;
        _stats._DSNumDrawcalls += 2;
    } else {
        // VKTODO: what should vertexOffest be?
        // VKTODO: mode needs to be in the pipeline
        vkCmdDrawIndexed(_currentCommandBuffer, numIndices, 1, startIndex, 0, 0);
        //glDrawElements(mode, numIndices, glType, indexBufferByteOffset);
        _stats._DSNumTriangles += numIndices / 3;
        _stats._DSNumDrawcalls++;
    }
    _stats._DSNumAPIDrawcalls++;
}

void VKBackend::do_drawInstanced(const Batch& batch, size_t paramOffset) {
    // VKTODO: can be done later
    /*GLint numInstances = batch._params[paramOffset + 4]._uint;
    Primitive primitiveType = (Primitive)batch._params[paramOffset + 3]._uint;
    GLenum mode = gl::PRIMITIVE_TO_GL[primitiveType];
    uint32 numVertices = batch._params[paramOffset + 2]._uint;
    uint32 startVertex = batch._params[paramOffset + 1]._uint;


    if (isStereo()) {
        GLint trueNumInstances = 2 * numInstances;

#ifdef GPU_STEREO_DRAWCALL_INSTANCED
        glDrawArraysInstanced(mode, startVertex, numVertices, trueNumInstances);
#else
        setupStereoSide(0);
        glDrawArraysInstanced(mode, startVertex, numVertices, numInstances);
        setupStereoSide(1);
        glDrawArraysInstanced(mode, startVertex, numVertices, numInstances);
#endif

        _stats._DSNumTriangles += (trueNumInstances * numVertices) / 3;
        _stats._DSNumDrawcalls += trueNumInstances;
    } else {
        glDrawArraysInstanced(mode, startVertex, numVertices, numInstances);
        _stats._DSNumTriangles += (numInstances * numVertices) / 3;
        _stats._DSNumDrawcalls += numInstances;
    }
    _stats._DSNumAPIDrawcalls++;

    (void) CHECK_GL_ERROR();*/
}

void VKBackend::do_drawIndexedInstanced(const Batch& batch, size_t paramOffset) {
    // VKTODO: can be done later
    /*GLint numInstances = batch._params[paramOffset + 4]._uint;
    GLenum mode = gl::PRIMITIVE_TO_GL[(Primitive)batch._params[paramOffset + 3]._uint];
    uint32 numIndices = batch._params[paramOffset + 2]._uint;
    uint32 startIndex = batch._params[paramOffset + 1]._uint;
    uint32 startInstance = batch._params[paramOffset + 0]._uint;
    GLenum glType = gl::ELEMENT_TYPE_TO_GL[_input._indexBufferType];
    auto typeByteSize = TYPE_SIZE[_input._indexBufferType];
    GLvoid* indexBufferByteOffset = reinterpret_cast<GLvoid*>(startIndex * typeByteSize + _input._indexBufferOffset);

    if (isStereo()) {
        GLint trueNumInstances = 2 * numInstances;

#ifdef GPU_STEREO_DRAWCALL_INSTANCED
        glDrawElementsInstancedBaseVertexBaseInstance(mode, numIndices, glType, indexBufferByteOffset, trueNumInstances, 0, startInstance);
#else
        setupStereoSide(0);
        glDrawElementsInstancedBaseVertexBaseInstance(mode, numIndices, glType, indexBufferByteOffset, numInstances, 0, startInstance);
        setupStereoSide(1);
        glDrawElementsInstancedBaseVertexBaseInstance(mode, numIndices, glType, indexBufferByteOffset, numInstances, 0, startInstance);
#endif
        _stats._DSNumTriangles += (trueNumInstances * numIndices) / 3;
        _stats._DSNumDrawcalls += trueNumInstances;
    } else {
        glDrawElementsInstancedBaseVertexBaseInstance(mode, numIndices, glType, indexBufferByteOffset, numInstances, 0, startInstance);
        _stats._DSNumTriangles += (numInstances * numIndices) / 3;
        _stats._DSNumDrawcalls += numInstances;
    }

    _stats._DSNumAPIDrawcalls++;

    (void)CHECK_GL_ERROR();*/
}

void VKBackend::do_multiDrawIndirect(const Batch& batch, size_t paramOffset) {
    // VKTODO: can be done later
    /*uint commandCount = batch._params[paramOffset + 0]._uint;
    GLenum mode = gl::PRIMITIVE_TO_GL[(Primitive)batch._params[paramOffset + 1]._uint];
    glMultiDrawArraysIndirect(mode, reinterpret_cast<GLvoid*>(_input._indirectBufferOffset), commandCount, (GLsizei)_input._indirectBufferStride);
    _stats._DSNumDrawcalls += commandCount;
    _stats._DSNumAPIDrawcalls++;
    (void)CHECK_GL_ERROR();*/
}

void VKBackend::do_multiDrawIndexedIndirect(const Batch& batch, size_t paramOffset) {
    // VKTODO: can be done later
    /*uint commandCount = batch._params[paramOffset + 0]._uint;
    GLenum mode = gl::PRIMITIVE_TO_GL[(Primitive)batch._params[paramOffset + 1]._uint];
    GLenum indexType = gl::ELEMENT_TYPE_TO_GL[_input._indexBufferType];
    glMultiDrawElementsIndirect(mode, indexType, reinterpret_cast<GLvoid*>(_input._indirectBufferOffset), commandCount, (GLsizei)_input._indirectBufferStride);
    _stats._DSNumDrawcalls += commandCount;
    _stats._DSNumAPIDrawcalls++;
    (void)CHECK_GL_ERROR();*/
}

void VKBackend::do_setFramebuffer(const Batch& batch, size_t paramOffset) {
    auto framebuffer = batch._framebuffers.get(batch._params[paramOffset]._uint);
    _cache.pipelineState.setFramebuffer(framebuffer);
    // VKTODO?
    /*auto framebuffer = batch._framebuffers.get(batch._params[paramOffset]._uint);
    if (_output._framebuffer != framebuffer) {
        auto newFBO = getFramebufferID(framebuffer);
        if (_output._drawFBO != newFBO) {
            _output._drawFBO = newFBO;
            glBindFramebuffer(VK_DRAW_FRAMEBUFFER, newFBO);
        }
        _output._framebuffer = framebuffer;
    }*/
}

void VKBackend::do_setFramebufferSwapChain(const Batch& batch, size_t paramOffset) {
    // VKTODO?
    auto swapChain = std::static_pointer_cast<FramebufferSwapChain>(batch._swapChains.get(batch._params[paramOffset]._uint));
    if (swapChain) {
        auto index = batch._params[paramOffset + 1]._uint;
        const auto& framebuffer = swapChain->get(index);
        _cache.pipelineState.setFramebuffer(framebuffer);
    }
}

void VKBackend::do_clearFramebuffer(const Batch& batch, size_t paramOffset) {
    // VKTODO: Check if this is needed on Vulkan or just clearing values in render pass info is enough
    /*if (_stereo._enable && !_pipeline._stateCache.scissorEnable) {
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
    }*/
}

void VKBackend::do_blit(const Batch& batch, size_t paramOffset) {
    // VKTODO
    /*auto srcframebuffer = batch._framebuffers.get(batch._params[paramOffset]._uint);
    Vec4i srcvp;
    for (auto i = 0; i < 4; ++i) {
        srcvp[i] = batch._params[paramOffset + 1 + i]._int;
    }

    auto dstframebuffer = batch._framebuffers.get(batch._params[paramOffset + 5]._uint);
    Vec4i dstvp;
    for (auto i = 0; i < 4; ++i) {
        dstvp[i] = batch._params[paramOffset + 6 + i]._int;
    }

    // Assign dest framebuffer if not bound already
    auto destFbo = getFramebufferID(dstframebuffer);
    auto srcFbo = getFramebufferID(srcframebuffer);
    glBlitNamedFramebuffer(srcFbo, destFbo,
                           srcvp.x, srcvp.y, srcvp.z, srcvp.w,
                           dstvp.x, dstvp.y, dstvp.z, dstvp.w,
                           GL_COLOR_BUFFER_BIT, GL_LINEAR);
    (void) CHECK_GL_ERROR();*/
}

void VKBackend::do_setInputFormat(const Batch& batch, size_t paramOffset) {
    const auto& format = batch._streamFormats.get(batch._params[paramOffset]._uint);
    // VKTODO: what about index format?
    _cache.pipelineState.setVertexFormat(format);
    // VKTODO: is _input needed anywhere?
    if (!compare(_input._format, format)) {
        if (format) {
            assign(_input._format, format);
            auto inputFormat = VKInputFormat::sync((*format));
            assert(inputFormat);
            if (_input._formatKey != inputFormat->key) {
                _input._formatKey = inputFormat->key;
                _input._invalidFormat = true;
            }
        } else {
            reset(_input._format);
            _input._formatKey.clear();
            _input._invalidFormat = true;
        }
    }
}

void VKBackend::do_setInputBuffer(const Batch& batch, size_t paramOffset) {
    // VKTODO
    Offset stride = batch._params[paramOffset + 0]._uint;
    Offset offset = batch._params[paramOffset + 1]._uint;
    BufferPointer buffer = batch._buffers.get(batch._params[paramOffset + 2]._uint);
    uint32 channel = batch._params[paramOffset + 3]._uint;

    if (channel < getNumInputBuffers()) {
        bool isModified = false;
        if (_input._buffers[channel] != buffer.get()) {
            // VKTODO: _input._buffers should be a smart pointer probably to avoid access after delete
            _input._buffers[channel] = buffer.get();

            VkBuffer vkBuffer = VK_NULL_HANDLE;
            if (buffer) {
                vkBuffer = VKBuffer::getBuffer(*this, *buffer);
            }
            _input._bufferVBOs[channel] = vkBuffer;

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
    // VKTODO
    _input._indexBufferType = (Type)batch._params[paramOffset + 2]._uint;
    _input._indexBufferOffset = batch._params[paramOffset + 0]._uint;

    BufferPointer indexBuffer = batch._buffers.get(batch._params[paramOffset + 1]._uint);
    if (indexBuffer.get() != _input._indexBuffer) {
        _input._indexBuffer = indexBuffer.get();
        if (indexBuffer) {
            //auto vulkanBuffer = getGPUObject<VKBuffer>(*indexBuffer);
            // VKTODO: which index type?
            VkIndexType indexType = VK_INDEX_TYPE_NONE_KHR;
            if (_input._indexBufferType == gpu::UINT32) {
                indexType = VK_INDEX_TYPE_UINT32;
            } else if (_input._indexBufferType == gpu::UINT16) {
                indexType = VK_INDEX_TYPE_UINT16;
            } else {
                Q_ASSERT(false);
            }
            vkCmdBindIndexBuffer(_currentCommandBuffer, VKBuffer::getBuffer(*this, *indexBuffer), _input._indexBufferOffset, VK_INDEX_TYPE_UINT32);
            //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, getBufferID(*indexBuffer));
        } else {
            // FIXME do we really need this?  Is there ever a draw call where we care that the element buffer is null?
            Q_ASSERT(false);
            //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
    }
}

void VKBackend::do_setIndirectBuffer(const Batch& batch, size_t paramOffset) {
    // VKTODO
    /*_input._indirectBufferOffset = batch._params[paramOffset + 1]._uint;
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

    (void)CHECK_VK_ERROR();*/
}

void VKBackend::do_generateTextureMips(const Batch& batch, size_t paramOffset) {
    // VKTODO
    /*TexturePointer resourceTexture = batch._textures.get(batch._params[paramOffset + 0]._uint);
    if (!resourceTexture) {
        return;
    }

    // DO not transfer the texture, this call is expected for rendering texture
    VKTexture* object = syncGPUObject(resourceTexture, false);
    if (!object) {
        return;
    }

    object->generateMips();*/
}

void VKBackend::do_advance(const Batch& batch, size_t paramOffset) {
    auto ringbuffer = batch._swapChains.get(batch._params[paramOffset]._uint);
    if (ringbuffer) {
        ringbuffer->advance();
    }
}

void VKBackend::do_beginQuery(const Batch& batch, size_t paramOffset) {
    // VKTODO
    /*auto query = batch._queries.get(batch._params[paramOffset]._uint);
    VKQuery* glquery = syncGPUObject(*query);
    if (glquery) {
        if (timeElapsed) {
            glBeginQuery(VK_TIME_ELAPSED, glquery->_endqo);
        } else {
            glQueryCounter(glquery->_beginqo, VK_TIMESTAMP);
        }
        (void)CHECK_VK_ERROR();
    }*/
}

void VKBackend::do_endQuery(const Batch& batch, size_t paramOffset) {
    // VKTODO
    /*auto query = batch._queries.get(batch._params[paramOffset]._uint);
    VKQuery* glquery = syncGPUObject(*query);
    if (glquery) {
        if (timeElapsed) {
            glEndQuery(VK_TIME_ELAPSED);
        } else {
            glQueryCounter(glquery->_endqo, VK_TIMESTAMP);
        }
        (void)CHECK_VK_ERROR();
    }*/
}

void VKBackend::do_getQuery(const Batch& batch, size_t paramOffset) {
    //VKTODO
    /*auto query = batch._queries.get(batch._params[paramOffset]._uint);
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
    }*/
}

// Transform Stage
void VKBackend::do_setModelTransform(const Batch& batch, size_t paramOffset) {
    // VKTODO: Why it's empty?
}

void VKBackend::do_setViewTransform(const Batch& batch, size_t paramOffset) {
    _transform._view = batch._transforms.get(batch._params[paramOffset]._uint);
    _transform._viewIsCamera = batch._params[paramOffset + 1]._uint != 0;
    _transform._invalidView = true;
}

void VKBackend::do_setProjectionTransform(const Batch& batch, size_t paramOffset) {
    memcpy(glm::value_ptr(_transform._projection), batch.readData(batch._params[paramOffset]._uint), sizeof(Mat4));
    _transform._projection = glm::scale(_transform._projection, glm::vec3(1.0f, -1.0f, 1.0f));
    _transform._invalidProj = true;
}

void VKBackend::do_setProjectionJitter(const Batch& batch, size_t paramOffset) {
    _transform._projectionJitter.x = batch._params[paramOffset]._float;
    _transform._projectionJitter.y = batch._params[paramOffset+1]._float;
    _transform._invalidProj = true;
}

void VKBackend::do_setViewportTransform(const Batch& batch, size_t paramOffset) {
    memcpy(glm::value_ptr(_transform._viewport), batch.readData(batch._params[paramOffset]._uint), sizeof(Vec4i));
    //memcpy(&_transform._viewport, batch.editData(batch._params[paramOffset]._uint), sizeof(Vec4i));

    if (!_inRenderTransferPass && !isStereo()) {
        ivec4& vp = _transform._viewport;
    }

    // The Viewport is tagged invalid because the CameraTransformUBO is not up to date and will need update on next drawcall
    _transform._invalidViewport = true;
}

void VKBackend::do_setDepthRangeTransform(const Batch& batch, size_t paramOffset) {
    //VKTODO
    /*Vec2 depthRange(batch._params[paramOffset + 1]._float, batch._params[paramOffset + 0]._float);

    if ((depthRange.x != _transform._depthRange.x) || (depthRange.y != _transform._depthRange.y)) {
        _transform._depthRange = depthRange;

        glDepthRangef(depthRange.x, depthRange.y);
    }*/
}

void VKBackend::do_setStateScissorRect(const Batch& batch, size_t paramOffset) {
    //VKTODO: this is ugly but should work
    Vec4i rect;
    memcpy(glm::value_ptr(rect), batch.readData(batch._params[paramOffset]._uint), sizeof(Vec4i));

    if (_stereo._enable) {
        rect.z /= 2;
        if (_stereo._pass) {
            rect.x += rect.z;
        }
    }
    _currentScissorRect = rect;
    //glScissor(rect.x, rect.y, rect.z, rect.w);
}

void VKBackend::do_setPipeline(const Batch& batch, size_t paramOffset) {
    //VKTODO
    PipelinePointer pipeline = batch._pipelines.get(batch._params[paramOffset + 0]._uint);

    /*auto currentPipeline = _cache.pipelineState;
    if (currentPipeline.pipeline == pipeline.get()) {
        return;
    }*/

    _cache.pipelineState.setPipeline(pipeline);

    // A true new Pipeline
    _stats._PSNumSetPipelines++;

    // null pipeline == reset
    /*if (!pipeline) {

        currentPipeline._pipeline.reset();

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
    }*/
}

void VKBackend::do_setStateBlendFactor(const Batch& batch, size_t paramOffset) {
    //VKTODO
    /*Vec4 factor(batch._params[paramOffset + 0]._float,
                batch._params[paramOffset + 1]._float,
                batch._params[paramOffset + 2]._float,
                batch._params[paramOffset + 3]._float);

    glBlendColor(factor.x, factor.y, factor.z, factor.w);
    (void)CHECK_GL_ERROR();*/
}

void VKBackend::do_setUniformBuffer(const Batch& batch, size_t paramOffset) {
    //VKTODO
    uint32_t slot = batch._params[paramOffset + 3]._uint;
    BufferPointer uniformBuffer = batch._buffers.get(batch._params[paramOffset + 2]._uint);
    uint32_t rangeStart = batch._params[paramOffset + 1]._uint;
    uint32_t rangeSize = batch._params[paramOffset + 0]._uint;

    // Create descriptor

    if (!uniformBuffer) {
        releaseUniformBuffer(slot);
        return;
    }

    // check cache before thinking
    if (_uniform._buffers[slot].buffer == uniformBuffer.get()) {
        return;
    }

    // Sync BufferObject
    auto* object = syncGPUObject(*uniformBuffer);
    if (object) {
        //glBindBufferRange(VK_UNIFORM_BUFFER, slot, object->_buffer, rangeStart, rangeSize);

        _uniform._buffers[slot].buffer = uniformBuffer.get();
        _uniform._buffers[slot].offset = rangeStart;
        _uniform._buffers[slot].size = rangeSize;
    } else {
        releaseResourceTexture(slot);
        return;
    }
}

void VKBackend::do_setResourceBuffer(const Batch& batch, size_t paramOffset) {
    //VKTODO:
    uint32_t slot = batch._params[paramOffset + 1]._uint;
    if (slot >= (uint32_t)MAX_NUM_RESOURCE_BUFFERS) {
        qCDebug(gpu_vk_logging) << "GLBackend::do_setResourceBuffer: Trying to set a resource Buffer at slot #" << slot
                              << " which doesn't exist. MaxNumResourceBuffers = " << MAX_NUM_RESOURCE_BUFFERS;
        return;
    }

    const auto& resourceBuffer = batch._buffers.get(batch._params[paramOffset + 0]._uint);

    if (!resourceBuffer) {
        releaseResourceBuffer(slot);
        return;
    }
    // check cache before thinking
    if (compare(_resource._buffers[slot].buffer, resourceBuffer)) {
        return;
    }

    // One more True Buffer bound
    _stats._RSNumResourceBufferBounded++;

    // If successful then cache it
    auto* object = syncGPUObject(*resourceBuffer);
    if (object) {
        assign(_resource._buffers[slot].buffer, resourceBuffer);
    } else {  // else clear slot and cache
        releaseResourceBuffer(slot);
        return;
    }
}

void VKBackend::do_setResourceTexture(const Batch& batch, size_t paramOffset) {
    // VKTODO:
    uint32_t slot = batch._params[paramOffset + 1]._uint;
    TexturePointer resourceTexture = batch._textures.get(batch._params[paramOffset + 0]._uint);

    if (slot == 2) {
        printf("break");
    }

    if (!resourceTexture) {
        releaseResourceTexture(slot);
        return;
    }
    // check cache before thinking
    if (_resource._textures[slot].texture == resourceTexture.get()) {
        return;
    }

    // One more True texture bound
    _stats._RSNumTextureBounded++;

    // Always make sure the VKObject is in sync
    // VKTODO
    //VKTexture* object = syncGPUObject(resourceTexture);
    //if (object) {
        //uint32_t to = object->_texture;
        //uint32_t target = object->_target;
        //glActiveTexture(VK_TEXTURE0 + slot);
        //glBindTexture(target, to);

        _resource._textures[slot].texture = resourceTexture.get();

        //_stats._RSAmountTextureMemoryBounded += object->size();

    //} else {
    //    releaseResourceTexture(slot);
    //    return;
    //}
}

void VKBackend::do_setResourceTextureTable(const gpu::Batch& batch, size_t paramOffset) {
    const auto& textureTablePointer = batch._textureTables.get(batch._params[paramOffset]._uint);
    if (!textureTablePointer) {
        return;
    }

    const auto& textureTable = *textureTablePointer;
    const auto& textures = textureTable.getTextures();
    for (uint32_t slot = 0; slot < textures.size(); ++slot) {
        bindResourceTexture(slot, textures[slot]);
    }
}

void VKBackend::do_setResourceFramebufferSwapChainTexture(const Batch& batch, size_t paramOffset) {
    /*uint32_t slot = batch._params[paramOffset + 1]._uint;
    if (slot >= MAX_NUM_RESOURCE_TEXTURES) {
        qCDebug(gpu_vk_logging)
            << "GLBackend::do_setResourceFramebufferSwapChainTexture: Trying to set a resource Texture at slot #" << slot
            << " which doesn't exist. MaxNumResourceTextures = " << MAX_NUM_RESOURCE_TEXTURES;
        return;
    }

    auto swapChain =
        std::static_pointer_cast<FramebufferSwapChain>(batch._swapChains.get(batch._params[paramOffset + 0]._uint));

    if (!swapChain) {
        releaseResourceTexture(slot);
        return;
    }
    auto index = batch._params[paramOffset + 2]._uint;
    auto renderBufferSlot = batch._params[paramOffset + 3]._uint;
    const auto& resourceFramebuffer = swapChain->get(index);
    const auto& resourceTexture = resourceFramebuffer->getRenderBuffer(renderBufferSlot);
    setResourceTexture(slot, resourceTexture);*/
}

std::array<VKBackend::CommandCall, Batch::NUM_COMMANDS> VKBackend::_commandCalls{ {
    (&::gpu::vk::VKBackend::do_draw),
    (&::gpu::vk::VKBackend::do_drawIndexed),
    (&::gpu::vk::VKBackend::do_drawInstanced),
    (&::gpu::vk::VKBackend::do_drawIndexedInstanced),
    (&::gpu::vk::VKBackend::do_multiDrawIndirect),
    (&::gpu::vk::VKBackend::do_multiDrawIndexedIndirect),

    (&::gpu::vk::VKBackend::do_setInputFormat),
    (&::gpu::vk::VKBackend::do_setInputBuffer),
    (&::gpu::vk::VKBackend::do_setIndexBuffer),
    (&::gpu::vk::VKBackend::do_setIndirectBuffer),

    (&::gpu::vk::VKBackend::do_setModelTransform),
    (&::gpu::vk::VKBackend::do_setViewTransform),
    (&::gpu::vk::VKBackend::do_setProjectionTransform),
    (&::gpu::vk::VKBackend::do_setProjectionJitter),
    (&::gpu::vk::VKBackend::do_setViewportTransform),
    (&::gpu::vk::VKBackend::do_setDepthRangeTransform),

    (&::gpu::vk::VKBackend::do_setPipeline),
    (&::gpu::vk::VKBackend::do_setStateBlendFactor),
    (&::gpu::vk::VKBackend::do_setStateScissorRect),

    (&::gpu::vk::VKBackend::do_setUniformBuffer),
    (&::gpu::vk::VKBackend::do_setResourceBuffer),
    (&::gpu::vk::VKBackend::do_setResourceTexture),
    (&::gpu::vk::VKBackend::do_setResourceTextureTable), // This is not needed, it's only for bindless textures, which are not enabled in OpenGL version
    (&::gpu::vk::VKBackend::do_setResourceFramebufferSwapChainTexture),

    (&::gpu::vk::VKBackend::do_setFramebuffer),
    (&::gpu::vk::VKBackend::do_setFramebufferSwapChain),
    (&::gpu::vk::VKBackend::do_clearFramebuffer),
    (&::gpu::vk::VKBackend::do_blit),
    (&::gpu::vk::VKBackend::do_generateTextureMips),

    (&::gpu::vk::VKBackend::do_advance),

    (&::gpu::vk::VKBackend::do_beginQuery),
    (&::gpu::vk::VKBackend::do_endQuery),
    (&::gpu::vk::VKBackend::do_getQuery),

    (&::gpu::vk::VKBackend::do_resetStages),

    (&::gpu::vk::VKBackend::do_disableContextViewCorrection),
    (&::gpu::vk::VKBackend::do_restoreContextViewCorrection),
    (&::gpu::vk::VKBackend::do_disableContextStereo),
    (&::gpu::vk::VKBackend::do_restoreContextStereo),

    (&::gpu::vk::VKBackend::do_runLambda),

    (&::gpu::vk::VKBackend::do_startNamedCall),
    (&::gpu::vk::VKBackend::do_stopNamedCall),

    (&::gpu::vk::VKBackend::do_glUniform1f), // Seems to be deprecated?
    (&::gpu::vk::VKBackend::do_glUniform2f),
    (&::gpu::vk::VKBackend::do_glUniform3f),
    (&::gpu::vk::VKBackend::do_glUniform4f),

    (&::gpu::vk::VKBackend::do_pushProfileRange),
    (&::gpu::vk::VKBackend::do_popProfileRange),
} };

VKInputFormat::VKInputFormat() {
}

VKInputFormat::~VKInputFormat() {

}

VKInputFormat* VKInputFormat::sync(const Stream::Format& inputFormat) {
    VKInputFormat* object = Backend::getGPUObject<VKInputFormat>(inputFormat);

    if (!object) {
        object = new VKInputFormat();
        object->key = inputFormat.getKey();
        Backend::setGPUObject(inputFormat, object);
    }

    return object;
}
