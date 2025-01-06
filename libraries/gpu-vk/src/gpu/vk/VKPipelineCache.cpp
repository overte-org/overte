#include "VKPipelineCache.h"

#include <iomanip>

#include "VKShared.h"
#include <vk/Pipelines.h>
#include "VKTexture.h"

using namespace gpu;
using namespace gpu::vk;

template <typename T>
static std::string hex(T t) {
    std::stringstream sStream;
    sStream << std::setw(sizeof(T)) << std::setfill('0') << std::hex << t;
    return sStream.str();
}

void Cache::Pipeline::setPipeline(const gpu::PipelinePointer& pipeline) {
    if (!gpu::compare(this->pipeline, pipeline)) {
        gpu::assign(this->pipeline, pipeline);
    }
    clearStrides();
}

void Cache::Pipeline::setVertexFormat(const gpu::Stream::FormatPointer& format) {
    if (!gpu::compare(this->format, format)) {
        gpu::assign(this->format, format);
    }
    clearStrides();
}

void Cache::Pipeline::setFramebuffer(const gpu::FramebufferPointer& framebuffer) {
    if (!gpu::compare(this->framebuffer, framebuffer)) {
        gpu::assign(this->framebuffer, framebuffer);
    }
}

void Cache::Pipeline::updateBindingMap(BindingMap& bindingMap,
                             const LocationMap& locationMap,
                             VkShaderStageFlagBits shaderStage) {
    for (const auto& entry : locationMap) {
        bindingMap[entry.second] |= shaderStage;
    }
}

void Cache::Pipeline::setBindingMap(BindingMap& bindingMap, const LocationMap& vertexMap, const LocationMap& fragmentMap) {
    bindingMap.clear();
    updateBindingMap(bindingMap, vertexMap, VK_SHADER_STAGE_VERTEX_BIT);
    updateBindingMap(bindingMap, fragmentMap, VK_SHADER_STAGE_FRAGMENT_BIT);
}

Cache::Pipeline::BindingMap Cache::Pipeline::getBindingMap(const LocationMap& vertexMap, const LocationMap& fragmentMap) {
    BindingMap result;
    setBindingMap(result, vertexMap, fragmentMap);
    return result;
}

// VKTODO: This needs to be used instead of getPipeline
// Returns structure containing pipeline layout and descriptor set layouts
Cache::Pipeline::PipelineLayout Cache::Pipeline::getPipelineAndDescriptorLayout(const vks::Context& context) {
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
        PipelineLayout layout{};

        for (const auto& entry : getBindingMap(vertexReflection.uniformBuffers, fragmentRefelection.uniformBuffers)) {
            VkDescriptorSetLayoutBinding binding =
                vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, entry.second,
                                                              entry.first, 1);
            uniLayout.push_back(binding);
        }
        if (fragmentRefelection.textures.count("webTexture")){
            printf("webTexture");
        }
        for (const auto& entry : getBindingMap(vertexReflection.textures, fragmentRefelection.textures)) {
            VkDescriptorSetLayoutBinding binding =
                vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, entry.second,
                                                              entry.first, 1);
            texLayout.push_back(binding);
        }
        for (const auto& entry : getBindingMap(vertexReflection.resourceBuffers, fragmentRefelection.resourceBuffers)) {
            VkDescriptorSetLayoutBinding binding =
                vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, entry.second,
                                                              entry.first, 1);
            stoLayout.push_back(binding);
        }

        // Create the descriptor set layouts
        std::vector<VkDescriptorSetLayout> layouts;
        if (!uniLayout.empty() || !texLayout.empty() || !stoLayout.empty()) {
            VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI =
                vks::initializers::descriptorSetLayoutCreateInfo(uniLayout.data(), uniLayout.size());
            VkDescriptorSetLayout descriptorSetLayout;
            VK_CHECK_RESULT(vkCreateDescriptorSetLayout(context.device->logicalDevice, &descriptorSetLayoutCI, nullptr,
                                                        &descriptorSetLayout));
            layouts.push_back(descriptorSetLayout);
            layout.uniformLayout = descriptorSetLayout;
        }
#if SEP_DESC
        // Descriptor set needs to be created even if it's empty if later descriptor sets are not empty.
        if (!texLayout.empty() || !stoLayout.empty()) {
            VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI =
                vks::initializers::descriptorSetLayoutCreateInfo(texLayout.data(), texLayout.size());
            VkDescriptorSetLayout descriptorSetLayout;
            VK_CHECK_RESULT(vkCreateDescriptorSetLayout(context.device->logicalDevice, &descriptorSetLayoutCI, nullptr,
                                                        &descriptorSetLayout));
            layouts.push_back(descriptorSetLayout);
            layout.textureLayout = descriptorSetLayout;
        } else {
            printf("empty");
        }
        if (!stoLayout.empty()) {
            VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI =
                vks::initializers::descriptorSetLayoutCreateInfo(stoLayout.data(), stoLayout.size());
            VkDescriptorSetLayout descriptorSetLayout;
            VK_CHECK_RESULT(vkCreateDescriptorSetLayout(context.device->logicalDevice, &descriptorSetLayoutCI, nullptr,
                                                        &descriptorSetLayout));
            layouts.push_back(descriptorSetLayout);
            layout.storageLayout = descriptorSetLayout;
        }
#endif
        VkPipelineLayoutCreateInfo pipelineLayoutCI =
            vks::initializers::pipelineLayoutCreateInfo(layouts.data(), (uint32_t)layouts.size());
        VkPipelineLayout pipelineLayout;
        VK_CHECK_RESULT(
            vkCreatePipelineLayout(context.device->logicalDevice, &pipelineLayoutCI, nullptr, &pipelineLayout));

        layout.pipelineLayout = pipelineLayout;

        return _layoutMap[this->pipeline] = layout;
        //return _layoutMap[this->pipeline] = nullptr;
    }
    return itr->second;
}

Cache::Pipeline::RenderpassKey Cache::Pipeline::getRenderPassKey(gpu::Framebuffer* framebuffer) const {
    RenderpassKey result;
    if (!framebuffer) {
        result.emplace_back(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED); // VKTODO: this is definitely wrong, why is it that way?
        Q_ASSERT(false);
    } else {
        for (const auto& attachment : framebuffer->getRenderBuffers()) {
            if (attachment.isValid()) {
                // VKTODO: why _element often has different format than texture's pixel format, and seemingly wrong one?
                //result.push_back(evalTexelFormatInternal(attachment._element));
                VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
                auto *gpuTexture = Backend::getGPUObject<VKTexture>(*attachment._texture);
                if (gpuTexture) {
                    auto attachmentTexture = dynamic_cast<VKAttachmentTexture*>(gpuTexture);
                    if (attachmentTexture) {
                        layout = attachmentTexture->getVkImageLayout();
                    }
                }
                result.emplace_back(evalTexelFormatInternal(attachment._texture->getTexelFormat()), layout);
            }
        }
        if (framebuffer->hasDepthStencil()) {
            VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
            if (framebuffer->getDepthStencilBuffer()) {
                auto* gpuTexture = Backend::getGPUObject<VKTexture>(*framebuffer->getDepthStencilBuffer());
                if (gpuTexture) {
                    auto attachmentTexture = dynamic_cast<VKAttachmentTexture*>(gpuTexture);
                    if (attachmentTexture) {
                        layout = attachmentTexture->getVkImageLayout();
                    }
                }
            }
            result.emplace_back(evalTexelFormatInternal(framebuffer->getDepthStencilBufferFormat()), layout);
        }
    }
    return result;
}

VkRenderPass Cache::Pipeline::getRenderPass(const vks::Context& context) {
    const auto framebuffer = gpu::acquire(this->framebuffer);

    RenderpassKey key = getRenderPassKey(framebuffer);
    auto itr = _renderPassMap.find(key);
    if (itr == _renderPassMap.end()) {
        auto &renderBuffers = framebuffer->getRenderBuffers();
        std::vector<VkAttachmentDescription> attachments;
        attachments.reserve(key.size());
        std::vector<VkAttachmentReference> colorAttachmentReferences;
        VkAttachmentReference depthReference{};
        for (size_t i = 0; i < key.size(); i++) {
            Q_ASSERT(i < renderBuffers.size());
            std::shared_ptr<gpu::Texture> texture = renderBuffers[i]._texture;
            if (!texture) {
                // Last texture of the key can be depth stencil attachment
                Q_ASSERT(i + 1 == key.size());
                texture = framebuffer->getDepthStencilBuffer();
            } else {
                texture = renderBuffers[i]._texture;
            }
            VKAttachmentTexture *attachmentTexture = nullptr;
            if (texture) {
                auto gpuObject = Backend::getGPUObject<VKTexture>(*texture);
                if (gpuObject) {
                    attachmentTexture = dynamic_cast<VKAttachmentTexture*>(gpuObject);
                }
            }
            VkAttachmentDescription attachment{};
            attachment.format = key[i].first;
            // Framebuffers are always cleared with a separate command in the renderer/
            if (!attachmentTexture || attachmentTexture->getVkImageLayout() == VK_IMAGE_LAYOUT_UNDEFINED) {
                attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            } else {
                attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
                attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            }
            attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
            //attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachment.samples = VK_SAMPLE_COUNT_1_BIT;
            if (texture->isDepthStencilRenderTarget()) {
                if (!attachmentTexture || attachmentTexture->getVkImageLayout() == VK_IMAGE_LAYOUT_UNDEFINED) {
                    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                } else {
                    if (attachmentTexture->getVkImageLayout() == VK_IMAGE_LAYOUT_GENERAL) {
                        attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
                        attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
                        depthReference.layout = VK_IMAGE_LAYOUT_GENERAL;
                    } else {
                        Q_ASSERT(attachmentTexture->getVkImageLayout() == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
                        attachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                        attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                        depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                    }
                }
                depthReference.attachment = (uint32_t)(attachments.size());
            } else {
                if (!attachmentTexture || attachmentTexture->getVkImageLayout() == VK_IMAGE_LAYOUT_UNDEFINED) {
                    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                } else {
                    Q_ASSERT(attachmentTexture->getVkImageLayout() == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
                    attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                }
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
            //if (depthReference.layout != VK_IMAGE_LAYOUT_UNDEFINED) {
            if (framebuffer->getDepthStencilBuffer()) {
                Q_ASSERT(depthReference.layout != VK_IMAGE_LAYOUT_UNDEFINED);
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

// VKTODO:
/*std::string Cache::Pipeline::getRenderpassKeyString(const RenderpassKey& renderpassKey) {
    std::string result;

    for (const auto& e : renderpassKey) {
        result += hex((uint32_t)e);
    }
    return result;
}*/

std::string Cache::Pipeline::getStridesKey() const {
    std::string key;
    for (int i = 0; i < MAX_NUM_INPUT_BUFFERS; i++) {
        if (_bufferStrideSet[i]) {
            key += "_" + hex(i) + "s" + hex(_bufferStrides[i]);
        }
    }
    return key;
}

// VKTODO:
/*std::string Cache::Pipeline::getKey() const {
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
    key += "_" + hex(primitiveTopology);
    key += "_" + getStridesKey();
    return key;
}*/

VkStencilOpState Cache::getStencilOp(const gpu::State::StencilTest& stencil) {
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

VkShaderModule Cache::getShaderModule(const vks::Context& context, const shader::Source& source) {
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

VkPipeline Cache::getPipeline(const vks::Context& context) {
    //VKTODO: pipelines are not cached here currently
    auto renderpass = pipelineState.getRenderPass(context);
    auto pipelineLayout = pipelineState.getPipelineAndDescriptorLayout(context);
    const gpu::Pipeline& pipeline = *gpu::acquire(pipelineState.pipeline);
    const gpu::State& state = *pipeline.getState();

    const auto& vertexShader = pipeline.getProgram()->getShaders()[0]->getSource();
    const auto& fragmentShader = pipeline.getProgram()->getShaders()[1]->getSource();
    // FIXME

    const gpu::State::Data& stateData = state.getValues();
    vks::pipelines::GraphicsPipelineBuilder builder{ context.device->logicalDevice, pipelineLayout.pipelineLayout,
                                                     renderpass };

    // Input assembly
    {
        auto& inputAssembly = builder.inputAssemblyState;
        inputAssembly.topology = PRIMITIVE_TO_VK[pipelineState.primitiveTopology];
        // VKTODO: this looks unfinished
        // ia.primitiveRestartEnable = ???
        // ia.topology = vk::PrimitiveTopology::eTriangleList; ???
    }

    // Shader modules
    {
        builder.shaderStages.resize(2, {});
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
        auto& rasterizationState = builder.rasterizationState;
        rasterizationState.cullMode = (VkCullModeFlagBits)stateData.cullMode;
        //Bool32 ra.depthBiasEnable;
        //float ra.depthBiasConstantFactor;
        //float ra.depthBiasClamp;
        //float ra.depthBiasSlopeFactor;
        //ra.depthClampEnable = VK_TRUE;
        rasterizationState.depthClampEnable = stateData.flags.depthClampEnable ? VK_TRUE : VK_FALSE;  // VKTODO
        rasterizationState.frontFace =
            stateData.flags.frontFaceClockwise ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
        // ra.lineWidth
        rasterizationState.polygonMode = (VkPolygonMode)(2 - stateData.fillMode);
    }

    // Color blending
    {
        auto& colorBlendState = builder.colorBlendState;
        auto& attachmentStates = colorBlendState.blendAttachmentStates;
        Q_ASSERT(pipelineState.framebuffer);
        auto& blendFunction = stateData.blendFunction;
        auto& attachmentState = colorBlendState.blendAttachmentStates[0];
        attachmentState.blendEnable = stateData.blendFunction.isEnabled();
        attachmentState.srcColorBlendFactor = BLEND_ARGS_TO_VK[blendFunction.getSourceColor()];
        attachmentState.dstColorBlendFactor = BLEND_ARGS_TO_VK[blendFunction.getDestinationColor()];
        attachmentState.colorBlendOp = BLEND_OPS_TO_VK[blendFunction.getOperationColor()];
        attachmentState.srcAlphaBlendFactor = BLEND_ARGS_TO_VK[blendFunction.getSourceAlpha()];
        attachmentState.dstAlphaBlendFactor = BLEND_ARGS_TO_VK[blendFunction.getDestinationAlpha()];
        attachmentState.alphaBlendOp = BLEND_OPS_TO_VK[blendFunction.getOperationAlpha()];
        attachmentState.colorWriteMask = colorMaskToVk(stateData.colorWriteMask);
        auto& rbs = pipelineState.framebuffer->getRenderBuffers();
        uint32_t rbCount = 0;
        for (const auto& rb : rbs) {
            if (rb.isValid()) {
                ++rbCount;
            }
        }

        for (uint32_t i = 1; i < rbCount; ++i) {
            attachmentStates.push_back(attachmentStates.back());
        }
    }

    // Depth/Stencil
    {
        auto& ds = builder.depthStencilState;
        //ds.depthTestEnable = VK_FALSE;
        ds.depthTestEnable = stateData.depthTest.isEnabled() ? VK_TRUE : VK_FALSE;  //VKTODO
        ds.depthWriteEnable = stateData.depthTest.getWriteMask() != 0 ? VK_TRUE : VK_FALSE;
        ds.depthCompareOp = (VkCompareOp)stateData.depthTest.getFunction();
        ds.stencilTestEnable = stateData.stencilActivation.enabled;
        ds.front = getStencilOp(stateData.stencilTestFront);
        ds.front.writeMask = stateData.stencilActivation.frontWriteMask;
        ds.back = getStencilOp(stateData.stencilTestBack);
        ds.back.writeMask = stateData.stencilActivation.backWriteMask;
    }

    // Vertex input
    if (pipelineState.format) {
        const auto& vertexReflection = pipeline.getProgram()->getShaders()[0]->getReflection();

        const gpu::Stream::Format& format = *gpu::acquire(pipelineState.format);
        auto& bindingDescriptions = builder.vertexInputState.bindingDescriptions;
        auto channelCount = format.getNumChannels();
        for (const auto& entry : format.getChannels()) {
            const auto& slot = entry.first;
            const auto& channel = entry.second;
            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = slot;
            if (pipelineState._bufferStrideSet[slot]) {
                bindingDescription.stride = pipelineState._bufferStrides[slot];
            } else {
                bindingDescription.stride = (uint32_t)channel._stride;
            }
            bindingDescription.inputRate = (VkVertexInputRate)(channel._frequency);
            bindingDescriptions.push_back(bindingDescription);
        }

        std::array<bool, 16> isAttributeSlotOccupied{};

        bool colorFound = false;
        auto& attributeDescriptions = builder.vertexInputState.attributeDescriptions;
        for (const auto& entry : format.getAttributes()) {
            const auto& slot = entry.first;
            const auto& attribute = entry.second;
            if (slot == Stream::COLOR) {
                colorFound = true;
            }
            isAttributeSlotOccupied[slot] = true;

            attributeDescriptions.push_back(
                { slot, attribute._channel, evalTexelFormatInternal(attribute._element), (uint32_t)attribute._offset });
        }

        if (!colorFound && vertexReflection.validInput(Stream::COLOR)) {
            attributeDescriptions.push_back({ Stream::COLOR, 0, VK_FORMAT_R8G8B8A8_UNORM, 0 });
        }

        if (!isAttributeSlotOccupied[Stream::TEXCOORD0] && vertexReflection.validInput(Stream::TEXCOORD0)) {
            attributeDescriptions.push_back({ Stream::TEXCOORD0, 0, VK_FORMAT_R8G8B8A8_UNORM, 0 });
        }

        if (!isAttributeSlotOccupied[Stream::TEXCOORD1] && vertexReflection.validInput(Stream::TEXCOORD1)) {
            attributeDescriptions.push_back({ Stream::TEXCOORD1, 0, VK_FORMAT_R8G8B8A8_UNORM, 0 });
        }

        // Explicitly add the draw call info slot if required
        if (vertexReflection.validInput(gpu::slot::attr::DrawCallInfo)) {
            attributeDescriptions.push_back(
                { gpu::slot::attr::DrawCallInfo, gpu::slot::attr::DrawCallInfo, VK_FORMAT_R16G16_SINT, (uint32_t)0 });
            //bd.push_back({ gpu::slot::attr::DrawCallInfo, (uint32_t)sizeof(uint16_t) * 2, VK_VERTEX_INPUT_RATE_VERTEX });
            bindingDescriptions.push_back({ gpu::slot::attr::DrawCallInfo, 0, VK_VERTEX_INPUT_RATE_VERTEX });
        }
    }

    auto program = pipeline.getProgram();
    const auto& vertexReflection = program->getShaders()[0]->getReflection();
    const auto& fragmentRefelection = program->getShaders()[1]->getReflection();
    auto result = builder.create();
    builder.shaderStages.clear();
    return result;
}
