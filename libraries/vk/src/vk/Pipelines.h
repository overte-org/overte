#pragma once

#include "Context.h"

namespace vks {
    namespace pipelines {
        struct PipelineRasterizationStateCreateInfo : public VkPipelineRasterizationStateCreateInfo {
            using Parent = VkPipelineRasterizationStateCreateInfo;
            PipelineRasterizationStateCreateInfo() : Parent {} {
                sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                lineWidth = 1.0f;
                cullMode = VK_CULL_MODE_BACK_BIT;
            }
        };


        struct PipelineInputAssemblyStateCreateInfo : public VkPipelineInputAssemblyStateCreateInfo {
            PipelineInputAssemblyStateCreateInfo() :
                VkPipelineInputAssemblyStateCreateInfo {} {
                sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            }
        };

        struct PipelineColorBlendAttachmentState : public VkPipelineColorBlendAttachmentState {
            PipelineColorBlendAttachmentState() :
                VkPipelineColorBlendAttachmentState {} {
                colorWriteMask = vks::util::fullColorWriteMask();
            }
        };

        struct PipelineColorBlendStateCreateInfo : public VkPipelineColorBlendStateCreateInfo {
            // Default to a single color attachment state with no blending
            std::vector<PipelineColorBlendAttachmentState> blendAttachmentStates{ PipelineColorBlendAttachmentState() };
            
            PipelineColorBlendStateCreateInfo() :
                VkPipelineColorBlendStateCreateInfo{} {
                sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            }

            void update() {
                this->attachmentCount = (uint32_t)blendAttachmentStates.size();
                this->pAttachments = blendAttachmentStates.data();
            }
        };

        struct PipelineDynamicStateCreateInfo : public VkPipelineDynamicStateCreateInfo {
            std::vector<VkDynamicState> dynamicStateEnables;

            PipelineDynamicStateCreateInfo() :
                VkPipelineDynamicStateCreateInfo{} {
                sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
            }

            void update() {
                this->dynamicStateCount = (uint32_t)dynamicStateEnables.size();
                this->pDynamicStates = dynamicStateEnables.data();
            }
        };

        struct PipelineVertexInputStateCreateInfo : public VkPipelineVertexInputStateCreateInfo {
            std::vector<VkVertexInputBindingDescription> bindingDescriptions;
            std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

            PipelineVertexInputStateCreateInfo() :
                VkPipelineVertexInputStateCreateInfo{} {
                sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            }
            void update() {
                vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();
                vertexBindingDescriptionCount = (uint32_t)bindingDescriptions.size();
                pVertexBindingDescriptions = bindingDescriptions.data();
                pVertexAttributeDescriptions = attributeDescriptions.data();
            }
        };

        struct PipelineViewportStateCreateInfo : public VkPipelineViewportStateCreateInfo {
            std::vector<VkViewport> viewports;
            std::vector<VkRect2D> scissors;

            PipelineViewportStateCreateInfo() :
                VkPipelineViewportStateCreateInfo{} {
                sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            }

            void update() {
                if (viewports.empty()) {
                    viewportCount = 1;
                    pViewports = nullptr;
                } else {
                    viewportCount = (uint32_t)viewports.size();
                    pViewports = viewports.data();
                }

                if (scissors.empty()) {
                    scissorCount = 1;
                    pScissors = 0;
                } else {
                    scissorCount = (uint32_t)scissors.size();
                    pScissors = scissors.data();
                }
            }
        };

        struct PipelineDepthStencilStateCreateInfo : public VkPipelineDepthStencilStateCreateInfo {
            PipelineDepthStencilStateCreateInfo(bool depthEnable = true):
                VkPipelineDepthStencilStateCreateInfo{} {
                sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
                if (depthEnable) {
                    //depthTestEnable = VK_TRUE; //VKTODO
                    depthTestEnable = VK_FALSE;
                    depthWriteEnable = VK_TRUE;
                    //depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL; //VKTODO
                    depthCompareOp = VK_COMPARE_OP_ALWAYS;
                }
            }
        };
        struct GraphicsPipelineBuilder {
        private:
            void init() {
                pipelineCreateInfo.pRasterizationState = &rasterizationState;
                pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
                pipelineCreateInfo.pColorBlendState = &colorBlendState;
                pipelineCreateInfo.pMultisampleState = &multisampleState;
                pipelineCreateInfo.pViewportState = &viewportState;
                pipelineCreateInfo.pDepthStencilState = &depthStencilState;
                pipelineCreateInfo.pDynamicState = &dynamicState;
                pipelineCreateInfo.pVertexInputState = &vertexInputState;
            }
        public:
            GraphicsPipelineBuilder(const VkDevice& device, const VkPipelineLayout layout, const VkRenderPass& renderPass) :
                device(device) {
                pipelineCreateInfo.layout = layout;
                pipelineCreateInfo.renderPass = renderPass;
                multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
                init();
            }

            GraphicsPipelineBuilder(const GraphicsPipelineBuilder& other) : GraphicsPipelineBuilder(other.device, other.layout, other.renderPass) {}

            GraphicsPipelineBuilder& operator=(const GraphicsPipelineBuilder& other) = delete;

            ~GraphicsPipelineBuilder() {
                destroyShaderModules();
            }

            const VkDevice& device;
            VkPipelineCache pipelineCache{ VK_NULL_HANDLE }; // TODO: Add pipeline cache here
            // TODO: is this initialized properly
            VkRenderPass& renderPass { pipelineCreateInfo.renderPass };
            VkPipelineLayout& layout { pipelineCreateInfo.layout };
            // TODO: these need to be initialized
            PipelineInputAssemblyStateCreateInfo inputAssemblyState;
            PipelineRasterizationStateCreateInfo rasterizationState;
            VkPipelineMultisampleStateCreateInfo multisampleState {};
            PipelineDepthStencilStateCreateInfo depthStencilState;
            PipelineViewportStateCreateInfo viewportState;
            PipelineDynamicStateCreateInfo dynamicState;
            PipelineColorBlendStateCreateInfo colorBlendState;
            PipelineVertexInputStateCreateInfo vertexInputState;
            std::vector<VkPipelineShaderStageCreateInfo> shaderStages {};

            VkGraphicsPipelineCreateInfo pipelineCreateInfo{};

            void update() {
                pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
                pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
                pipelineCreateInfo.pStages = shaderStages.data();
                dynamicState.update();
                colorBlendState.update();
                vertexInputState.update();
                viewportState.update();
            }

            void destroyShaderModules() {
                for (const auto shaderStage : shaderStages) {
                    vkDestroyShaderModule(device, shaderStage.module, nullptr);
                }
                shaderStages.clear();
            }

            VkPipeline create(const VkPipelineCache& cache) {
                update();
                VkPipeline vkPipeline;
                VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, cache, 1, &pipelineCreateInfo, nullptr, &vkPipeline));
                return vkPipeline;
            }

            VkPipeline create() {
                return create(pipelineCache);
            }
        };
    }
} // namespace vks::pipelines
