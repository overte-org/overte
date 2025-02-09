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
#include "VKPipelineCache.h"
// VKTODO: how to include this?
#include "../../../../render-utils/src/render-utils/ShaderConstants.h"
#include "shared/FileUtils.h"

#define FORCE_STRICT_TEXTURE 1

using namespace gpu;
using namespace gpu::vk;

// VKTODO: this is ugly solution
Cache _cache;

size_t VKBackend::UNIFORM_BUFFER_OFFSET_ALIGNMENT{ 4 };

static VKBackend* INSTANCE{ nullptr };
static const char* VK_BACKEND_PROPERTY_NAME = "com.highfidelity.vk.backend";
//static bool enableDebugMarkers = false;

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
#ifdef QT_NO_DEBUG
        _context.setValidationEnabled(false); // VKTODO: find nicer way of toggling this
#else
        _context.setValidationEnabled(true); // VKTODO: find nicer way of toggling this
#endif
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
}

VKBackend::~VKBackend() {
    Q_ASSERT(isBackendShutdownComplete);
    // FIXME queue up all the trash calls
    _context.destroyContext();
}

void VKBackend::shutdown() {
    VK_CHECK_RESULT(vkQueueWaitIdle(_context.graphicsQueue));
    VK_CHECK_RESULT(vkQueueWaitIdle(_context.transferQueue) );
    VK_CHECK_RESULT(vkDeviceWaitIdle(_context.device->logicalDevice));

    _context.shutdownWindows();

    // Release frames so their destructors can do a cleanup
    _currentFrame.reset();
    _framesToReuse.resize(0);
    _framePool.resize(0);

    for (auto &module : _cache.moduleMap) {
        _context.recycler.trashVkShaderModule(module.second);
    }
    _cache.moduleMap.clear();

    for (auto &renderPass : _cache.pipelineState._renderPassMap) {
        _context.recycler.trashVkRenderPass(renderPass.second);
    }
    _cache.pipelineState._renderPassMap.clear();

    for (auto &pipeline : _cache.pipelineMap) {
        _context.recycler.trashVkDescriptorSetLayout(pipeline.second.uniformLayout);
        _context.recycler.trashVkDescriptorSetLayout(pipeline.second.storageLayout);
        _context.recycler.trashVkDescriptorSetLayout(pipeline.second.textureLayout);
        _context.recycler.trashVkPipelineLayout(pipeline.second.pipelineLayout);
        _context.recycler.trashVkPipeline(pipeline.second.pipeline);
    }
    _cache.pipelineMap.clear();

    beforeShutdownCleanup();

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
    vkDestroyPipelineCache(_context.device->logicalDevice, _pipelineCache, nullptr);
    vmaDestroyAllocator(vks::Allocation::getAllocator());

    isBackendShutdownComplete = true;
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

void VKBackend::executeFrame(const FramePointer& frame) {
    using namespace vks::debugutils;

    // Initialize parts that cannot be initialized in default constructor.
    if (!_isInitialized) {
        initBeforeFirstFrame();
        _isInitialized = true;
    }

    // Create descriptor pool
    // VKTODO: delete descriptor pool after it's not needed
    //_frameData._descriptorPool
    // VKTODO: make sure that previous frame finished rendering
    perFrameCleanup();
    acquireFrameData();

    int batch_count = 0;
    {
        const auto& commandBuffer = _currentCommandBuffer;
        for (const auto& batchPtr : frame->batches) {
            const auto& batch = *batchPtr;
            /*if (batch.getName() == "SubsurfaceScattering::diffuseProfileGPU") {
                continue;
            }*/
            /*if (batch.getName() == "SurfaceGeometryPass::run") {
                printf("SurfaceGeometryPass");
            }*/
            /*if (batch.getName() == "BlurGaussian::run") {
                continue;
            }*/
            cmdBeginLabel(commandBuffer, "batch:" + batch.getName(), glm::vec4{ 1, 1, 0, 1 });
            const auto& commands = batch.getCommands();
            const auto& offsets = batch.getCommandOffsets();
            const auto numCommands = commands.size();
            const auto* command = commands.data();
            const auto* offset = offsets.data();
            bool renderpassActive = false;
            for (auto commandIndex = 0; commandIndex < (int)numCommands; ++commandIndex, ++command, ++offset) {
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
                            // VKTODO
                            //_cache.getPipeline(_context);
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
                        default:
                            break;
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

            if (batch.getName() == "Resample::run") {
                _outputTexture = syncGPUObject(*_cache.pipelineState.framebuffer);
            }
            if (batch.getName() == "CompositeHUD") {
                _outputTexture = syncGPUObject(*_cache.pipelineState.framebuffer);
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
    // VKTODO: provide nicer way of doing this
    _frameCounter++;
    if (_frameCounter % 4000 == 0) {
        dumpVmaMemoryStats();
    }
    // loop through commands

    //


    // Restore the saved stereo state for the next batch
    // _stereo._enable = savedStereo;
}

void VKBackend::setDrawCommandBuffer(VkCommandBuffer commandBuffer) {
    _currentCommandBuffer = commandBuffer;
}

VkDescriptorImageInfo VKBackend::getDefaultTextureDescriptorInfo() {
    return _defaultTextureVk->getDescriptorImageInfo();
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
        if (_viewIsCamera && ((_viewCorrectionEnabled || _viewCorrectionEnabledForFramePlayer) && _correction.correction != glm::mat4())) {
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
        buffer.buffer = currentFrame._cameraBuffer.get();
        buffer.size = sizeof(CameraBufferElement);
        buffer.offset = _currentCameraOffset + eye * _cameraUboSize;
        //glBindBufferRange(GL_UNIFORM_BUFFER, slot::buffer::Buffer::CameraTransform, _cameraBuffer, _currentCameraOffset + eye * _cameraUboSize, sizeof(CameraBufferElement));
    }
}

void VKBackend::store_glUniform1f(const Batch& batch, size_t paramOffset) {
    _currentFrame->addGlUniform(sizeof(float), reinterpret_cast<const void*>(&batch._params[paramOffset + 0]._float), _commandIndex);
}

void VKBackend::store_glUniform2f(const Batch& batch, size_t paramOffset) {
    std::array<float, 2> array {batch._params[paramOffset + 1]._float,
                                batch._params[paramOffset + 0]._float};
    _currentFrame->addGlUniform(sizeof(float) * 2, reinterpret_cast<const void*>(array.data()), _commandIndex);
}

void VKBackend::store_glUniform3f(const Batch& batch, size_t paramOffset) {
    std::array<float, 3> array {batch._params[paramOffset + 2]._float,
                                batch._params[paramOffset + 1]._float,
                                batch._params[paramOffset + 0]._float};
    _currentFrame->addGlUniform(sizeof(float) * 3, reinterpret_cast<const void*>(array.data()), _commandIndex);

}

void VKBackend::store_glUniform4f(const Batch& batch, size_t paramOffset) {
    std::array<float, 4> array {batch._params[paramOffset + 3]._float,
                                batch._params[paramOffset + 2]._float,
                                batch._params[paramOffset + 1]._float,
                                batch._params[paramOffset + 0]._float};
    _currentFrame->addGlUniform(sizeof(float) * 4, reinterpret_cast<const void*>(array.data()), _commandIndex);
}

void VKBackend::store_glUniform3fv(const Batch& batch, size_t paramOffset) {
    _currentFrame->addGlUniform(sizeof(float) * 3 * batch._params[paramOffset + 1]._uint,
                                reinterpret_cast<const void*>(batch.readData(batch._params[paramOffset + 0]._uint)), _commandIndex);
}

void VKBackend::store_glUniform4fv(const Batch& batch, size_t paramOffset) {
    _currentFrame->addGlUniform(sizeof(float) * 4 * batch._params[paramOffset + 1]._uint,
                                reinterpret_cast<const void*>(batch.readData(batch._params[paramOffset + 0]._uint)), _commandIndex);
}

void VKBackend::store_glUniform4iv(const Batch& batch, size_t paramOffset) {
    _currentFrame->addGlUniform(sizeof(int) * 4 * batch._params[paramOffset + 1]._uint,
                                reinterpret_cast<const void*>(batch.readData(batch._params[paramOffset + 0]._uint)), _commandIndex);
}

void VKBackend::store_glUniformMatrix3fv(const Batch& batch, size_t paramOffset) {
    _currentFrame->addGlUniform(sizeof(float) * 3 * 3 * batch._params[paramOffset + 2]._uint,
                                reinterpret_cast<const void*>(batch.readData(batch._params[paramOffset + 0]._uint)), _commandIndex);
    // VKTODO: transpose if batch._params[paramOffset + 1]._uint != 0
    /*glUniformMatrix3fv(
        location,
        batch._params[paramOffset + 2]._uint,
        batch._params[paramOffset + 1]._uint,
        (const GLfloat*)batch.readData(batch._params[paramOffset + 0]._uint));*/
}

void VKBackend::store_glUniformMatrix4fv(const Batch& batch, size_t paramOffset) {
    _currentFrame->addGlUniform(sizeof(float) * 4 * 4 * batch._params[paramOffset + 2]._uint,
                                reinterpret_cast<const void*>(batch.readData(batch._params[paramOffset + 0]._uint)), _commandIndex);
    // VKTODO: transpose if batch._params[paramOffset + 1]._uint != 0
    /*glUniformMatrix4fv(
        location,
        batch._params[paramOffset + 2]._uint,
        batch._params[paramOffset + 1]._uint,
        (const GLfloat*)batch.readData(batch._params[paramOffset + 0]._uint));*/
}

void VKBackend::do_resetStages(const Batch& batch, size_t paramOffset) {
    //VKTODO: make sure all stages are reset
    //VKTODO: should inout stage be reset here?
    resetUniformStage();
    resetTextureStage();
    resetResourceStage();
    resetQueryStage();
    resetInputStage();
    /*resetPipelineStage();
    resetTransformStage();
    resetOutputStage();*/ //VKTODO
}

void VKBackend::do_disableContextViewCorrection(const Batch& batch, size_t paramOffset) {
    _transform._viewCorrectionEnabled = false;
}

void VKBackend::do_restoreContextViewCorrection(const Batch& batch, size_t paramOffset) {
    _transform._viewCorrectionEnabled = true;
}

void VKBackend::do_setContextMirrorViewCorrection(const Batch& batch, size_t paramOffset) {
    //VKTODO
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

static const int INVALID_UNIFORM_INDEX = -1;

int VKBackend::getRealUniformLocation(int location) {
    return location;
    /*auto variant = isStereo() ? shader::Variant::Stereo : shader::Variant::Mono;
    auto index = static_cast<uint32_t>(variant);

    auto& shader = _cache.pipelineState.pipeline->getProgram()->_shaderObjects[index];
    auto itr = shader.uniformRemap.find(location);
    if (itr == shader.uniformRemap.end()) {
        // This shouldn't happen, because we use reflection to determine all the possible
        // uniforms.  If someone is requesting a uniform that isn't in the remapping structure
        // that's a bug from the calling code, because it means that location wasn't in the
        // reflection
        qWarning() << "Unexpected location requested for shader: #" << location;
        return INVALID_UNIFORM_INDEX;
    }
    return itr->second;*/ //VKTOODO: is this needed?
}

void VKBackend::do_glUniform1f(const Batch& batch, size_t paramOffset) {
    /*if (_pipeline._program == 0) {
        // We should call updatePipeline() to bind the program but we are not doing that
        // because these uniform setters are deprecated and we don;t want to create side effect
        return;
    }
    updatePipeline();*/

    int location = getRealUniformLocation(batch._params[paramOffset + 1]._int);
    Q_ASSERT(location != INVALID_UNIFORM_INDEX);
    //_uniform._buffers[location].vksBuffer = nullptr;
    _uniform._buffers[location].buffer = _currentFrame->_glUniformBuffer.get();
    _uniform._buffers[location].offset = _currentFrame->_glUniformOffsetMap[(int)_commandIndex];
    _uniform._buffers[location].size = sizeof(float);
}

void VKBackend::do_glUniform2f(const Batch& batch, size_t paramOffset) {
    qDebug() << "VKTODO: do_glUniform2f not implemented";
    /*if (_pipeline._program == 0) {
        // We should call updatePipeline() to bind the program but we are not doing that
        // because these uniform setters are deprecated and we don;t want to create side effect
        return;
    }
    updatePipeline();*/

     int location = getRealUniformLocation(batch._params[paramOffset + 2]._int);
     Q_ASSERT(location != INVALID_UNIFORM_INDEX);
    //_uniform._buffers[location].vksBuffer = nullptr;
    _uniform._buffers[location].buffer = _currentFrame->_glUniformBuffer.get();
    _uniform._buffers[location].offset = _currentFrame->_glUniformOffsetMap[(int)_commandIndex];
    _uniform._buffers[location].size = sizeof(float) * 2;
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

void VKBackend::do_glUniform3fv(const Batch& batch, size_t paramOffset) {
    qDebug() << "VKTODO: do_glUniform3fv not implemented";

    /*if (_pipeline._program == 0) {
        // We should call updatePipeline() to bind the program but we are not doing that
        // because these uniform setters are deprecated and we don;t want to create side effect
        return;
    }
    updatePipeline();
    GLint location = getRealUniformLocation(batch._params[paramOffset + 2]._int);
    glUniform3fv(
        location,
        batch._params[paramOffset + 1]._uint,
        (const GLfloat*)batch.readData(batch._params[paramOffset + 0]._uint));

    (void)CHECK_GL_ERROR();*/
}

void VKBackend::do_glUniform4fv(const Batch& batch, size_t paramOffset) {
    qDebug() << "VKTODO: do_glUniform4fv not implemented";
    /*if (_pipeline._program == 0) {
        // We should call updatePipeline() to bind the program but we are not doing that
        // because these uniform setters are deprecated and we don;t want to create side effect
        return;
    }
    updatePipeline();

    GLint location = getRealUniformLocation(batch._params[paramOffset + 2]._int);
    GLsizei count = batch._params[paramOffset + 1]._uint;
    const GLfloat* value = (const GLfloat*)batch.readData(batch._params[paramOffset + 0]._uint);
    glUniform4fv(location, count, value);

    (void)CHECK_GL_ERROR();*/
}

void VKBackend::do_glUniform4iv(const Batch& batch, size_t paramOffset) {
    qDebug() << "VKTODO: do_glUniform4iv not implemented";
    /*if (_pipeline._program == 0) {
        // We should call updatePipeline() to bind the program but we are not doing that
        // because these uniform setters are deprecated and we don;t want to create side effect
        return;
    }
    updatePipeline();
    GLint location = getRealUniformLocation(batch._params[paramOffset + 2]._int);
    glUniform4iv(
        location,
        batch._params[paramOffset + 1]._uint,
        (const GLint*)batch.readData(batch._params[paramOffset + 0]._uint));

    (void)CHECK_GL_ERROR();*/
}

void VKBackend::do_glUniformMatrix3fv(const Batch& batch, size_t paramOffset) {
    qDebug() << "VKTODO: do_glUniformMatrix3fv not implemented";
    /*if (_pipeline._program == 0) {
        // We should call updatePipeline() to bind the program but we are not doing that
        // because these uniform setters are deprecated and we don;t want to create side effect
        return;
    }
    updatePipeline();

    GLint location = getRealUniformLocation(batch._params[paramOffset + 3]._int);
    glUniformMatrix3fv(
        location,
        batch._params[paramOffset + 2]._uint,
        batch._params[paramOffset + 1]._uint,
        (const GLfloat*)batch.readData(batch._params[paramOffset + 0]._uint));
    (void)CHECK_GL_ERROR();*/
}

void VKBackend::do_glUniformMatrix4fv(const Batch& batch, size_t paramOffset) {
    qDebug() << "VKTODO: do_glUniformMatrix4fv not implemented";
    /*if (_pipeline._program == 0) {
        // We should call updatePipeline() to bind the program but we are not doing that
        // because these uniform setters are deprecated and we don;t want to create side effect
        return;
    }
    updatePipeline();

    GLint location = getRealUniformLocation(batch._params[paramOffset + 3]._int);
    glUniformMatrix4fv(
        location,
        batch._params[paramOffset + 2]._uint,
        batch._params[paramOffset + 1]._uint,
        (const GLfloat*)batch.readData(batch._params[paramOffset + 0]._uint));
    (void)CHECK_GL_ERROR();*/
}

void VKBackend::do_pushProfileRange(const Batch& batch, size_t paramOffset) {
    const auto& name = batch._profileRanges.get(batch._params[paramOffset]._uint);
    ::vks::debugutils::cmdBeginLabel(_currentCommandBuffer, name, glm::vec4{ 1.0 });
}

void VKBackend::do_popProfileRange(const Batch& batch, size_t paramOffset) {
    ::vks::debugutils::cmdEndLabel(_currentCommandBuffer);
}

void VKBackend::setCameraCorrection(const Mat4& correction, const Mat4& prevRenderView, bool primary, bool reset) {
    // VKTODO: `primary` property is not used yet
    auto invCorrection = glm::inverse(correction);
    auto invPrevView = glm::inverse(prevRenderView);
    _transform._correction.prevView = (reset ? Mat4() : prevRenderView);
    _transform._correction.prevViewInverse = (reset ? Mat4() : invPrevView);
    _transform._correction.correction = correction;
    _transform._correction.correctionInverse = invCorrection;
    std::vector<uint8_t> bufferData;
    bufferData.resize(_transform._cameraUboSize * _transform._cameras.size());
    for (size_t i = 0; i < _transform._cameras.size(); ++i) {
        memcpy(bufferData.data() + (_transform._cameraUboSize * i), &_transform._cameras[i], sizeof(TransformStageState::CameraBufferElement));
    }
    if (_currentFrame && _currentFrame->_cameraBuffer) {
        _currentFrame->_cameraBuffer = std::make_shared<gpu::Buffer>(gpu::Buffer::UniformBuffer, bufferData.size(), bufferData.data());
        _currentFrame->_cameraBuffer->flush();
        _currentFrame->_buffers.push_back(_currentFrame->_cameraBuffer);
        syncGPUObject(*_currentFrame->_cameraBuffer);
    }
}

void VKBackend::resetInputStage() {
    _input.reset();
}

void VKBackend::InputStageState::reset() {
    _invalidFormat = true;
    _lastUpdateStereoState = false;
    _format = GPU_REFERENCE_INIT_VALUE;
    _formatKey = "";
    _attributeActivation.reset();
    _invalidBuffers.reset();
    _attribBindingBuffers.reset();
    for (int i = 0; i < MAX_NUM_INPUT_BUFFERS; i++) {
        _buffers[i] = nullptr;
        _bufferOffsets[i] = 0;
        _bufferStrides[i] = 0;
        _bufferVBOs[i] = VK_NULL_HANDLE;
    }
    _indexBuffer = nullptr;
    _indexBufferOffset = 0;
    _indexBufferType = UINT32;
    _indirectBuffer = nullptr;
    _indirectBufferOffset = 0;
    _indirectBufferStride = 0;
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
        if ((_uniform._buffers[i].buffer)
            && (vertexReflection.validUniformBuffer(i) || fragmentReflection.validUniformBuffer(i))) {
            // VKTODO: move vulkan buffer creation to the transfer parts and aggregate several buffers together maybe?
            VkDescriptorBufferInfo bufferInfo{};
            if (_uniform._buffers[i].buffer) {
                VKBuffer * buffer = syncGPUObject(*_uniform._buffers[i].buffer);
                Q_ASSERT(buffer);
                bufferInfo.buffer = buffer->buffer;
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
            VKTexture* texture = syncGPUObject(*_resource._textures[i].texture);
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            if (texture) {
                imageInfo = texture->getDescriptorImageInfo();
            }
            if (imageInfo.imageLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
                qDebug() << "Writing descriptor " << i << " with texture: " << QString::fromStdString(_resource._textures[i].texture->source());
                if (texture) {
                    qDebug() << "Warning: Texture " + QString::fromStdString(_resource._textures[i].texture->source()) + " being bound at input slot "
                                    << i << " is in VK_IMAGE_LAYOUT_UNDEFINED layout.";
                } else if (_resource._textures[i].texture) {
                    qDebug() << "Cannot sync texture during descriptor " << i
                             << " write: " << QString::fromStdString(_resource._textures[i].texture->source());
                } else {
                    qDebug() << "Texture is null during descriptor " << i
                             << " write.";
                }
                // VKTODO: is there a more reliable way of telling which one we need?
                if (i == render_utils::slot::texture::Skybox) {
                    imageInfo = _defaultSkyboxTextureImageInfo;
                } else {
                    imageInfo = _defaultTextureImageInfo;
                }
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
                descriptorWriteSet.pImageInfo = &_defaultTextureImageInfo;
                if (i == render_utils::slot::texture::Skybox) {
                    descriptorWriteSet.pImageInfo = &_defaultSkyboxTextureImageInfo;
                } else {
                    descriptorWriteSet.pImageInfo = &_defaultTextureImageInfo;
                }
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
        if ((_resource._buffers[i].buffer)
            && (vertexReflection.validResourceBuffer(i) || fragmentReflection.validResourceBuffer(i))) {
            // VKTODO: move vulkan buffer creation to the transfer parts and aggregate several buffers together maybe?
            VkDescriptorBufferInfo bufferInfo{};
            if (_resource._buffers[i].buffer) {
                VKBuffer* buffer = syncGPUObject(*_resource._buffers[i].buffer);
                bufferInfo.buffer = buffer->buffer;
                bufferInfo.range = _resource._buffers[i].buffer->_renderSysmem.getSize();
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
    // If framebuffer has changed, it means that render pass changed. _currentRenderPass is set to nullptr in such case.
    if (_hasFramebufferChanged) {
        Q_ASSERT(_currentVkRenderPass == nullptr);
        // Current renderpass has ended and vkCmdEndRenderPass was already called so we cat set proper layouts here
        // and avoid generating render pass twice.
        if (_currentFramebuffer) {
            updateAttachmentLayoutsAfterRenderPass();
        }
        transitionAttachmentImageLayouts(*_cache.pipelineState.framebuffer);
    }
    // Retrieve from cache or create render pass.
    auto renderPass = _cache.pipelineState.getRenderPass(_context);
    auto framebuffer = syncGPUObject(*_cache.pipelineState.framebuffer);

    // Current render pass is already up to date.
    // VKTODO: check if framebuffer has changed and if so update render pass too.
    if (_currentVkRenderPass == renderPass && _currentVkFramebuffer == framebuffer->vkFramebuffer) {
        return;
    }

    // Current render pass needs to be finished before starting new one
    if (_currentVkRenderPass) {
        vkCmdEndRenderPass(_currentCommandBuffer);
        updateAttachmentLayoutsAfterRenderPass();
        transitionAttachmentImageLayouts(*_cache.pipelineState.framebuffer);
    }
    // Input image layouts shouldn't affect render pass and always need to be done between
    transitionInputImageLayouts();

    // Render pass needs to be retrieved twice, since `updateAttachmentLayoutsAfterRenderPass` and `transitionAttachmentImageLayouts`
    // can be called only once we know that renderpass ended and may change attachment image layouts and thus change render pass again.
    renderPass = _cache.pipelineState.getRenderPass(_context);

    _currentVkRenderPass = renderPass;
    _currentFramebuffer = _cache.pipelineState.framebuffer;
    _currentVkFramebuffer = framebuffer->vkFramebuffer;
    _hasFramebufferChanged = false;

    auto renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
    renderPassBeginInfo.renderPass = renderPass;
    Q_ASSERT(_cache.pipelineState.framebuffer);
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
    VkOffset2D offset{_transform._viewport.x, _transform._viewport.y};
    VkExtent2D extent{static_cast<uint32_t>(_transform._viewport.z), static_cast<uint32_t>(_transform._viewport.w)};
    if (offset.x > framebuffer->_gpuObject.getWidth() || offset.y > framebuffer->_gpuObject.getHeight()) {
        offset.x = std::min(offset.x, (int32_t)framebuffer->_gpuObject.getWidth());
        offset.y = std::min(offset.y, (int32_t)framebuffer->_gpuObject.getHeight());
        qDebug() << "Vulkan framebuffer offset too high";
    }
    if (extent.width + offset.x > framebuffer->_gpuObject.getWidth() || extent.height + offset.y > framebuffer->_gpuObject.getHeight()) {
        extent.width = std::min(extent.width + offset.x, (uint32_t)framebuffer->_gpuObject.getWidth()) - offset.x;
        extent.height = std::min(extent.height + offset.y, (uint32_t)framebuffer->_gpuObject.getHeight()) - offset.y;
        qDebug() << "Vulkan framebuffer extent too large";
    }
    renderPassBeginInfo.renderArea = VkRect2D{offset, extent};
    vkCmdBeginRenderPass(_currentCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VKBackend::updateAttachmentLayoutsAfterRenderPass() {
    Q_ASSERT(_currentFramebuffer);
    auto &renderBuffers = _currentFramebuffer->getRenderBuffers();
    for (auto buffer : renderBuffers) {
        if (!buffer) {
            continue;
        }
        Q_ASSERT(buffer._texture);
        auto gpuObject = syncGPUObject(*buffer._texture);
        Q_ASSERT(gpuObject);
        auto attachmentTexture = dynamic_cast<VKAttachmentTexture*>(gpuObject);
        Q_ASSERT(attachmentTexture);
        if (buffer._texture->isDepthStencilRenderTarget()) {
            if (attachmentTexture->getVkImageLayout() == VK_IMAGE_LAYOUT_UNDEFINED) {
                attachmentTexture->_vkImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            }
        } else {
            if (attachmentTexture->getVkImageLayout() == VK_IMAGE_LAYOUT_UNDEFINED) {
                attachmentTexture->_vkImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }
        }
    }

    auto depthStencilBuffer = _currentFramebuffer->getDepthStencilBuffer();
    if (depthStencilBuffer) {
        auto gpuObject = syncGPUObject(*depthStencilBuffer);
        Q_ASSERT(gpuObject);
        auto attachmentTexture = dynamic_cast<VKAttachmentTexture*>(gpuObject);
        Q_ASSERT(attachmentTexture);
        if (attachmentTexture->getVkImageLayout() == VK_IMAGE_LAYOUT_UNDEFINED) {
            attachmentTexture->_vkImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }
    }
}

void VKBackend::resetRenderPass() {
    if (_currentVkRenderPass) {
        updateAttachmentLayoutsAfterRenderPass();
        _currentVkRenderPass = VK_NULL_HANDLE;
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
                case Batch::COMMAND_glUniform1f:
                    store_glUniform1f(batch, *offset);
                    break;
                case Batch::COMMAND_glUniform2f:
                    store_glUniform2f(batch, *offset);
                    break;
                case Batch::COMMAND_glUniform3f:
                    store_glUniform3f(batch, *offset);
                    break;
                case Batch::COMMAND_glUniform4f:
                    store_glUniform4f(batch, *offset);
                    break;
                case Batch::COMMAND_glUniform3fv:
                    store_glUniform3fv(batch, *offset);
                    break;
                case Batch::COMMAND_glUniform4fv:
                    store_glUniform4fv(batch, *offset);
                    break;
                case Batch::COMMAND_glUniform4iv:
                    store_glUniform4iv(batch, *offset);
                    break;
                case Batch::COMMAND_glUniformMatrix3fv:
                    store_glUniform3fv(batch, *offset);
                    break;
                case Batch::COMMAND_glUniformMatrix4fv:
                    store_glUniform4fv(batch, *offset);
                    break;
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

                case Batch::COMMAND_setFramebuffer:
                    break;

                case Batch::COMMAND_setResourceTexture:
                    break;

                case Batch::COMMAND_setResourceFramebufferSwapChainTexture:
                    break;

                default:
                    break;
                }
            command++;
            offset++;
        }
    }

    { // Sync the transform buffers
        PROFILE_RANGE(gpu_vk_detail, "syncGPUTransform");
        transferGlUniforms();
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

            /*if (_cache.pipelineState.pipeline->getProgram()->getShaders()[0]->getSource().name == "simple_procedural.vert") {
                printf("simple_procedural.vert");
                break;
            }*/ // VKTODO: currently crashes on procedural shaders. I tried this as a workaround,but it didn't work.
            _cache.pipelineState.primitiveTopology = getPrimitiveTopologyFromCommand(*command, batch, *offset);
            updateInput();
            updateTransform(batch);
            updatePipeline();
            updateRenderPass();
            // VKTODO: this is inefficient
            auto layout = _cache.getPipeline(_context);
            vkCmdBindPipeline(_currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout.pipeline);
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

            // VKTODO: remove when webentities work
            auto pipeline = gpu::acquire(_cache.pipelineState.pipeline);
            auto program = pipeline->getProgram();
            const auto& vertexReflection = program->getShaders()[0]->getReflection();
            const auto& fragmentReflection = program->getShaders()[1]->getReflection();
            /*if (fragmentReflection.textures.count("webTexture")) {
                printf("webTexture");
            }*/
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
    if (_currentVkRenderPass) {
        updateAttachmentLayoutsAfterRenderPass();
    }
    _currentVkRenderPass = VK_NULL_HANDLE;
    _currentFramebuffer = nullptr;
    // VKTODO: which other stages should be reset here?
    resetInputStage();
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
    auto object = vk::VKFramebuffer::sync(*this, framebuffer);
    if (!_framebuffers.count(object)) {
        _framebuffers.insert(object);
    }
    return object;
}

VKBuffer* VKBackend::syncGPUObject(const Buffer& buffer) {
    auto object = vk::VKBuffer::sync(*this, buffer);
    if (!_buffers.count(object)) {
        _buffers.insert(object);
    }
    return object;
}

VKTexture* VKBackend::syncGPUObject(const Texture& texture) {
    // VKTODO
    /*if (!texture) {
        return nullptr;
    }*/
    VKTexture* object = Backend::getGPUObject<VKTexture>(texture);

    if (TextureUsageType::EXTERNAL == texture.getUsageType()) {
        if (_isFramePlayer) {
            return nullptr; // Frame player does not support external textures
        }
        Q_ASSERT(GLAD_GL_EXT_memory_object);
        Q_ASSERT(GLAD_GL_EXT_semaphore);
        Texture::ExternalUpdates updates = texture.getUpdates();
        if (!updates.empty()) {
            Texture::ExternalRecycler recycler = texture.getExternalRecycler();
            Q_ASSERT(recycler);
            // Discard any superfluous updates
            while (updates.size() > 1) {
                const auto& update = updates.front();
                // Superfluous updates will never have been read, but we want to ensure the previous
                // writes to them are complete before they're written again, so return them with the
                // same fences they arrived with.  This can happen on any thread because no GL context
                // work is involved
                recycler(update.first, update.second);
                updates.pop_front();
            } // VKTODO: should that be thread-safe?

            // The last texture remaining is the one we'll use to create the GLTexture
            const auto& update = updates.front();
            // Check for a fence, and if it exists, inject a wait into the command stream, then destroy the fence
            if (update.second) {
                GLsync fence = static_cast<GLsync>(update.second);
                glWaitSync(fence, 0, GL_TIMEOUT_IGNORED); // VKTODO: Maybe take earlier texture instead of waiting to avoid stall?
                glDeleteSync(fence);
            }

            if (!object) {
                object = new VKExternalTexture(shared_from_this(), texture);
            }
            auto externalTexture = dynamic_cast<VKExternalTexture*>(object);
            Q_ASSERT(externalTexture);
            externalTexture->setSource(update.first);
            externalTexture->transferGL(*this); // VKTODO: add texture resizing if needed

            // Create the new texture object (replaces any previous texture object)

            return object;
            //new GLExternalTexture(shared_from_this(), texture, update.first);
        }

        // Return the texture object (if any) associated with the texture, without extensive logic
        // (external textures are
        //return Backend::getGPUObject<GLTexture>(texture);*/

        //return Parent::syncGPUObject(texturePointer);
    }

    if (!texture.isDefined()) {
        // NO texture definition yet so let's avoid thinking
        return nullptr;
    }

    // VKTODO: check object->_storageStamp to see if texture is outdated
    if (!object) {
        switch (texture.getUsageType()) {
            case TextureUsageType::RENDERBUFFER:
                object = new VKAttachmentTexture(shared_from_this(), texture);
                break;

            case TextureUsageType::EXTERNAL:
                if (_isFramePlayer) {
                    return nullptr; // Frame player does not support external textures
                }
                object = new VKExternalTexture(shared_from_this(), texture);
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

                {
                    auto storedFormat = texture.getStoredMipFormat();
                    auto texelFormat = texture.getTexelFormat();
                    if (storedFormat.getDimension() != texelFormat.getDimension()) {
                        qDebug() << "Element dimension mismatch, stored: " << storedFormat.getDimension() << " texel: " << texelFormat.getDimension();
                        return nullptr;
                    }
                    if (storedFormat.getType() != texelFormat.getType()) {
                        qDebug() << " mismatch, stored: " << storedFormat.getType() << " texel: " << texelFormat.getType();
                        return nullptr;
                    }
                    auto storedVkFormat = evalTexelFormatInternal(texture.getStoredMipFormat());
                    auto texelVkFormat = evalTexelFormatInternal(texture.getTexelFormat());
                    if (storedVkFormat != texelVkFormat) {
                        if (!(storedVkFormat == VK_FORMAT_R8G8B8_UNORM && texelVkFormat == VK_FORMAT_R8G8B8A8_UNORM) // Adding alpha channel needed
                            && !(storedVkFormat == VK_FORMAT_R8G8B8_UNORM && texelVkFormat == VK_FORMAT_R8G8B8A8_SRGB) // Adding alpha channel needed and maybe SRGB conversion?
                            && !(storedVkFormat == VK_FORMAT_B8G8R8A8_SRGB && texelVkFormat == VK_FORMAT_R8G8B8A8_SRGB) // Swapping channels needed
                            && !(storedVkFormat == VK_FORMAT_B8G8R8A8_UNORM && texelVkFormat == VK_FORMAT_R8G8B8A8_UNORM)) { // Swapping channels needed
                            qDebug() << "Format mismatch, stored: " << storedVkFormat << " texel: " << texelVkFormat;
                            return nullptr;
                        }
                    }
                }

                //VKTODO: for some reason some textures have only level 1 mipmap stored, while only level 0 is needed
                // I'm not sure why yet
                // For now I'll skip these
                {
                    auto numMips = texture.getNumMips();
                    bool areMipsAvailable = false;
                    for (uint16_t sourceMip = 0; sourceMip < numMips; ++sourceMip) {
                        if (texture.isStoredMipFaceAvailable(sourceMip)) {
                            areMipsAvailable = true;
                        }
                    }
                    if (!areMipsAvailable) {
                        qWarning() << "VKTODO: Texture has no mip levels available";
                        return nullptr;
                    }
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

    if (!_textures.count(object)) {
        _textures.insert(object);
    }
    return object;
}

VKQuery* VKBackend::syncGPUObject(const Query& query) {
    auto object = vk::VKQuery::sync(*this, query);
    if (!_queries.count(object)) {
        _queries.insert(object);
    }
    return object;
}

void VKBackend::blitToFramebuffer(VKAttachmentTexture &input, const Vec4i& srcViewport, VKAttachmentTexture &output, const Vec4i& dstViewport) {
    // is vks::tools::insertImageMemoryBarrier needed?
    VkImageBlit imageBlit{};
    // Do we ever want to blit multiple layers/mips?
    imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBlit.srcSubresource.layerCount = 1;
    imageBlit.srcSubresource.mipLevel = 0;
    imageBlit.srcOffsets[0].x = srcViewport.x;
    imageBlit.srcOffsets[0].y = srcViewport.y;
    imageBlit.srcOffsets[1].x = srcViewport.z;
    imageBlit.srcOffsets[1].y = srcViewport.w;
    imageBlit.srcOffsets[1].z = 1;

    imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBlit.dstSubresource.layerCount = 1;
    imageBlit.dstSubresource.mipLevel = 0;
    imageBlit.dstOffsets[0].x = srcViewport.x;
    imageBlit.dstOffsets[0].y = srcViewport.y;
    imageBlit.dstOffsets[1].x = srcViewport.z;
    imageBlit.dstOffsets[1].y = srcViewport.w;
    imageBlit.dstOffsets[1].z = 1;

    VkImageSubresourceRange mipSubRange = {};
    mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    mipSubRange.baseMipLevel = 0;
    mipSubRange.levelCount = 1;
    mipSubRange.layerCount = 1;

    vks::tools::insertImageMemoryBarrier(
        _currentCommandBuffer,
        input._vkImage,
        VK_ACCESS_TRANSFER_READ_BIT,
        0,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // VKTODO:
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        mipSubRange);

    vks::tools::insertImageMemoryBarrier(
        _currentCommandBuffer,
        output._vkImage,
        0,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // VKTODO
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        mipSubRange);

    vkCmdBlitImage(
        _currentCommandBuffer,
        input._vkImage,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        output._vkImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &imageBlit,
        VK_FILTER_LINEAR);

    vks::tools::insertImageMemoryBarrier(
        _currentCommandBuffer,
        output._vkImage,
        0,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // VKTODO
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        mipSubRange);

    output._vkImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // VKTODO
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


                    (void)CHECK_GL_ERROR();
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
        (void)CHECK_GL_ERROR();

        _input._invalidFormat = false;
        _stats._ISNumFormatChanges++;
    }*/

    if (_input._invalidBuffers.any()) {

        // Profile the count of buffers to update and use it to shortcut the for loop
        int numInvalids = (int) _input._invalidBuffers.count();
        _stats._ISNumInputBufferChanges += numInvalids;

        for (size_t buffer_index = 0; buffer_index < _input._buffers.size(); buffer_index++) {
            if (_input._invalidBuffers.test(buffer_index)) {
                _cache.pipelineState._bufferStrides[buffer_index] = _input._bufferStrides[buffer_index];
                _cache.pipelineState._bufferStrideSet.set(buffer_index, true);
                auto backendBuffer = syncGPUObject(*_input._buffers[buffer_index]);
                VkBuffer vkBuffer = VK_NULL_HANDLE;
                if (backendBuffer) {
                    vkBuffer = backendBuffer->buffer;
                }
                Q_ASSERT(vkBuffer != VK_NULL_HANDLE);

                //auto vkBuffer = VKBuffer::getBuffer(*this, *_input._buffers[buffer]);
                VkDeviceSize vkOffset = _input._bufferOffsets[buffer_index];
                vkCmdBindVertexBuffers(_currentCommandBuffer, buffer_index, 1, &vkBuffer, &vkOffset);
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

void VKBackend::FrameData::addGlUniform(size_t size, const void* data, size_t commandIndex) {
    _glUniformData.resize(_glUniformBufferPosition + size);
    memcpy(_glUniformData.data()+_glUniformBufferPosition, data, size);
    _glUniformBufferPosition += size;
}

VKBackend::FrameData::FrameData(VKBackend *backend) : _backend(backend) {
    createDescriptorPool();
    _cameraCorrectionBuffer.edit<CameraCorrection>() = CameraCorrection();
    _cameraCorrectionBuffer._buffer->flush();
    _cameraCorrectionBufferIdentity.edit<CameraCorrection>() = CameraCorrection();
    _cameraCorrectionBufferIdentity._buffer->flush();
}

VKBackend::FrameData::~FrameData() {
    cleanup();
    vkDestroyDescriptorPool(_backend->_context.device->logicalDevice, _descriptorPool, nullptr);
    if (_objectBuffer) {
        _objectBuffer.reset();
    }
    if (_cameraBuffer) {
        _cameraBuffer.reset();
    }
    if (_drawCallInfoBuffer) {
        _drawCallInfoBuffer.reset();
    }
    if (_glUniformBuffer) {
        _glUniformBuffer.reset();
    }
}

void VKBackend::FrameData::cleanup() {
    for (auto renderPass : _renderPasses) {
        vkDestroyRenderPass(_backend->_context.device->logicalDevice, renderPass, nullptr);
    }
    _renderPasses.resize(0);
    _glUniformBufferPosition = 0;
    _glUniformOffsetMap.clear();
    _glUniformData.clear();

    size_t vectorCapacity = _buffers.capacity();
    _buffers.clear();
    _buffers.reserve(vectorCapacity);

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

    _defaultTexture = gpu::Texture::create2D(gpu::Element{ gpu::VEC4, gpu::NUINT8, gpu::RGBA }, width, height, 1U,
                                                gpu::Sampler(gpu::Sampler::FILTER_MIN_MAG_LINEAR, gpu::Sampler::WRAP_CLAMP));
    _defaultTexture->setSource("defaultVulkanTexture");
    _defaultTexture->setStoredMipFormat(_defaultTexture->getTexelFormat());
    _defaultTexture->assignStoredMip(0, width * height * sizeof(uint8_t) * 4, (const gpu::Byte*)buffer.data());
    _defaultTextureVk = syncGPUObject(*_defaultTexture);
    _defaultTextureImageInfo = _defaultTextureVk->getDescriptorImageInfo();

    // Skyboxes cannot use single layer texture on Vulkan so a separate one needs to be created
    Q_ASSERT(width == height);
    _defaultSkyboxTexture = gpu::Texture::createCube(gpu::Element{ gpu::VEC4, gpu::NUINT8, gpu::RGBA }, width, 1U,
                                                        gpu::Sampler(gpu::Sampler::FILTER_MIN_MAG_LINEAR, gpu::Sampler::WRAP_CLAMP));
    _defaultSkyboxTexture->setSource("defaultVulkanSkyboxTexture");
    _defaultSkyboxTexture->setStoredMipFormat(_defaultSkyboxTexture->getTexelFormat());
    for (int i = 0; i < 6; i++) {
        _defaultSkyboxTexture->assignStoredMipFace(0, i, width * height * sizeof(uint8_t) * 4, (const gpu::Byte*)buffer.data());
    }
    _defaultSkyboxTextureVk = syncGPUObject(*_defaultSkyboxTexture);
    _defaultSkyboxTextureImageInfo = _defaultSkyboxTextureVk->getDescriptorImageInfo();
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

void VKBackend::transitionInputImageLayouts() {
    for (auto texture : _resource._textures) {
        if (!texture.texture) {
            continue;
        }
        auto gpuObject = Backend::getGPUObject<VKTexture>(*texture.texture);
        if (!gpuObject) {
            continue;
        }
        auto attachmentTexture = dynamic_cast<VKAttachmentTexture*>(gpuObject);
        if (!attachmentTexture) {
            continue;
        }
        if (attachmentTexture->_vkImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
            VkImageSubresourceRange mipSubRange = {};
            mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            mipSubRange.baseMipLevel = 0;
            mipSubRange.levelCount = 1;
            mipSubRange.layerCount = 1;
            vks::tools::insertImageMemoryBarrier(_currentCommandBuffer, attachmentTexture->_vkImage,
                                                 VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                                                 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,  // VKTODO
                                                 mipSubRange);
            attachmentTexture->_vkImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        } else if (attachmentTexture->_vkImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            // Sometimes texture is used both as an input and as a depth stencil framebuffer attachment, we need to check for this
            bool isAttachment = false;
            auto &framebuffer = _cache.pipelineState.framebuffer;
            if (framebuffer) {
                auto depthStencilBuffer = _cache.pipelineState.framebuffer->getDepthStencilBuffer();
                if (depthStencilBuffer) {
                    auto depthStencilGpuObject = Backend::getGPUObject<VKTexture>(*depthStencilBuffer);
                    if (depthStencilGpuObject) {
                        if (depthStencilGpuObject->_vkImage == attachmentTexture->_vkImage) {
                            isAttachment = true;
                        }
                    }
                }
            }
            if (isAttachment) {
                VkImageSubresourceRange mipSubRange = {};
                mipSubRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
                mipSubRange.baseMipLevel = 0;
                mipSubRange.levelCount = 1;
                mipSubRange.layerCount = 1;
                vks::tools::insertImageMemoryBarrier(_currentCommandBuffer, attachmentTexture->_vkImage,
                                                     VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                                     VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                                                     VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                     VK_IMAGE_LAYOUT_GENERAL, // VKTODO: is here a better alyout for this use case?
                                                     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,  // VKTODO
                                                     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,  // VKTODO
                                                     mipSubRange);
                attachmentTexture->_vkImageLayout = VK_IMAGE_LAYOUT_GENERAL;
            } else {
                VkImageSubresourceRange mipSubRange = {};
                mipSubRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
                mipSubRange.baseMipLevel = 0;
                mipSubRange.levelCount = 1;
                mipSubRange.layerCount = 1;
                vks::tools::insertImageMemoryBarrier(_currentCommandBuffer, attachmentTexture->_vkImage,
                                                     VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                                     VK_ACCESS_SHADER_READ_BIT,
                                                     VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,  // VKTODO
                                                     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,  // VKTODO
                                                     mipSubRange);
                attachmentTexture->_vkImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }
        };

    }
}

void VKBackend::transitionAttachmentImageLayouts(gpu::Framebuffer &framebuffer) {
    auto &renderBuffers = framebuffer.getRenderBuffers();
    for (auto buffer : renderBuffers) {
        if (!buffer) {
            continue;
        }
        auto gpuObject = Backend::getGPUObject<VKTexture>(*buffer._texture);
        if (!gpuObject) {
            continue;
        }
        auto attachmentTexture = dynamic_cast<VKAttachmentTexture*>(gpuObject);
        if (!attachmentTexture) {
            continue;
        }

        if (attachmentTexture->_vkImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            if (attachmentTexture->_gpuObject.isDepthStencilRenderTarget()) {
                // VKTODO: Check if the same depth render target is used as one of the inputs, if so then don't update it here
                VkImageSubresourceRange mipSubRange = {};
                mipSubRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
                mipSubRange.baseMipLevel = 0;
                mipSubRange.levelCount = 1;
                mipSubRange.layerCount = 1;
                vks::tools::insertImageMemoryBarrier(_currentCommandBuffer, attachmentTexture->_vkImage,
                                                     VK_ACCESS_SHADER_READ_BIT,
                                                     VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                     VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,  // VKTODO: should be
                                                     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,        // VKTODO
                                                     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, mipSubRange); // VKTODO: what stage mask for depth stencil?
                attachmentTexture->_vkImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            } else {
                VkImageSubresourceRange mipSubRange = {};
                mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                mipSubRange.baseMipLevel = 0;
                mipSubRange.levelCount = 1;
                mipSubRange.layerCount = 1;
                vks::tools::insertImageMemoryBarrier(_currentCommandBuffer, attachmentTexture->_vkImage,
                                                     VK_ACCESS_SHADER_READ_BIT,
                                                     VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,  // VKTODO: should be
                                                     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,        // VKTODO
                                                     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, mipSubRange);
                attachmentTexture->_vkImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }
        }
    }
    auto &depthStencilBuffer = framebuffer.getDepthStencilBuffer();
    if (!depthStencilBuffer) {
        return;
    }
    auto gpuObject = Backend::getGPUObject<VKTexture>(*depthStencilBuffer);
    if (!gpuObject) {
        return;
    }
    auto attachmentTexture = dynamic_cast<VKAttachmentTexture*>(gpuObject);
    if (!attachmentTexture) {
        return;
    }
    if (attachmentTexture->_vkImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        Q_ASSERT(attachmentTexture->_gpuObject.isDepthStencilRenderTarget());
        VkImageSubresourceRange mipSubRange = {};
        mipSubRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        mipSubRange.baseMipLevel = 0;
        mipSubRange.levelCount = 1;
        mipSubRange.layerCount = 1;
        vks::tools::insertImageMemoryBarrier(_currentCommandBuffer, attachmentTexture->_vkImage, VK_ACCESS_SHADER_READ_BIT,
                                             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                             VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,  // VKTODO: should be
                                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,                // VKTODO
                                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                             mipSubRange);  // VKTODO: what stage mask for depth stencil?
        attachmentTexture->_vkImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }
}

void VKBackend::perFrameCleanup() {
    auto &recycler = _context.recycler;
    std::lock_guard<std::recursive_mutex> lockGuard(recycler.recyclerMutex);
    // Remove pointers to objects that were deleted during the frame.
    for (auto framebuffer : recycler.deletedFramebuffers) {
        _framebuffers.erase(framebuffer);
    }
    size_t capacityBeforeClear = recycler.deletedFramebuffers.capacity(); // C++ standard doesn't guarantee keeping capacity on clear and that could impact performance
    recycler.deletedFramebuffers.clear();
    recycler.deletedFramebuffers.reserve(capacityBeforeClear);

    for (auto buffer : recycler.deletedBuffers) {
        _buffers.erase(buffer);
    }
    capacityBeforeClear = recycler.deletedBuffers.capacity();
    recycler.deletedBuffers.clear();
    recycler.deletedBuffers.reserve(capacityBeforeClear);

    for (auto texture : recycler.deletedTextures) {
        _textures.erase(texture);
    }
    capacityBeforeClear = recycler.deletedTextures.capacity();
    recycler.deletedTextures.clear();
    recycler.deletedTextures.reserve(capacityBeforeClear);

    for (auto query : recycler.deletedQueries) {
        _queries.erase(query);
    }
    capacityBeforeClear = recycler.deletedQueries.capacity();
    recycler.deletedQueries.clear();
    recycler.deletedQueries.reserve(capacityBeforeClear);

    auto device = _context.device->logicalDevice;

    for (auto framebuffer : recycler.vkFramebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    capacityBeforeClear = recycler.vkFramebuffers.capacity();
    recycler.vkFramebuffers.clear();
    recycler.vkFramebuffers.reserve(capacityBeforeClear);

    for (auto sampler : recycler.vkSamplers) {
        vkDestroySampler(device, sampler, nullptr);
    }
    capacityBeforeClear = recycler.vkSamplers.capacity();
    recycler.vkSamplers.clear();
    recycler.vkSamplers.reserve(capacityBeforeClear);

    for (auto imageView : recycler.vkImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }
    capacityBeforeClear = recycler.vkImageViews.capacity();
    recycler.vkImageViews.clear();
    recycler.vkImageViews.reserve(capacityBeforeClear);

    for (auto image : recycler.vkImages) {
        vkDestroyImage(device, image, nullptr);
    }
    capacityBeforeClear = recycler.vkImages.capacity();
    recycler.vkImages.clear();
    recycler.vkImages.reserve(capacityBeforeClear);

    for (auto buffer : recycler.vkBuffers) {
        vkDestroyBuffer(device, buffer, nullptr);
    }
    capacityBeforeClear = recycler.vkBuffers.capacity();
    recycler.vkBuffers.clear();
    recycler.vkBuffers.reserve(capacityBeforeClear);

    for (auto renderPass: recycler.vkRenderPasses) {
        vkDestroyRenderPass(device, renderPass, nullptr);
    }
    capacityBeforeClear = recycler.vkRenderPasses.capacity();
    recycler.vkRenderPasses.clear();
    recycler.vkRenderPasses.reserve(capacityBeforeClear);

    for (auto descriptorSetLayout: recycler.vkDescriptorSetLayouts) {
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    }
    capacityBeforeClear = recycler.vkDescriptorSetLayouts.capacity();
    recycler.vkDescriptorSetLayouts.clear();
    recycler.vkDescriptorSetLayouts.reserve(capacityBeforeClear);

    for (auto pipelineLayout: recycler.vkPipelineLayouts) {
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    }
    capacityBeforeClear = recycler.vkPipelineLayouts.capacity();
    recycler.vkPipelineLayouts.clear();
    recycler.vkPipelineLayouts.reserve(capacityBeforeClear);

    for (auto pipeline: recycler.vkPipelines) {
        vkDestroyPipeline(device, pipeline, nullptr);
    }
    capacityBeforeClear = recycler.vkPipelines.capacity();
    recycler.vkPipelines.clear();
    recycler.vkPipelines.reserve(capacityBeforeClear);

    for (auto shaderModule: recycler.vkShaderModules) {
        vkDestroyShaderModule(device, shaderModule, nullptr);
    }
    capacityBeforeClear = recycler.vkShaderModules.capacity();
    recycler.vkShaderModules.clear();
    recycler.vkShaderModules.reserve(capacityBeforeClear);

    for (auto swapchain : recycler.vkSwapchainsKHR) {
        vkDestroySwapchainKHR(device, swapchain, nullptr);
    }
    capacityBeforeClear = recycler.vkSwapchainsKHR.capacity();
    recycler.vkSwapchainsKHR.clear();
    recycler.vkSwapchainsKHR.reserve(capacityBeforeClear);

    for (auto semaphore: recycler.vkSemaphores) {
        vkDestroySemaphore(device, semaphore, nullptr);
    }
    capacityBeforeClear = recycler.vkSemaphores.capacity();
    recycler.vkSemaphores.clear();
    recycler.vkSemaphores.reserve(capacityBeforeClear);

    for (auto surface: recycler.vkSurfacesKHR) {
        vkDestroySurfaceKHR(_context.instance, surface, nullptr);
    }
    capacityBeforeClear = recycler.vkSurfacesKHR.capacity();
    recycler.vkSurfacesKHR.clear();
    recycler.vkSurfacesKHR.reserve(capacityBeforeClear);

    for (auto memory: recycler.vkDeviceMemories) {
        vkFreeMemory(device, memory, nullptr);
    }
    capacityBeforeClear = recycler.vkDeviceMemories.capacity();
    recycler.vkDeviceMemories.clear();
    recycler.vkDeviceMemories.reserve(capacityBeforeClear);

    for (auto allocation : recycler.vmaAllocations) {
        vmaFreeMemory(vks::Allocation::getAllocator(), allocation);
    }
    capacityBeforeClear = recycler.vmaAllocations.capacity();
    recycler.vmaAllocations.clear();
    recycler.vmaAllocations.reserve(capacityBeforeClear);

    {
        Lock lock(_externalTexturesMutex);
        if (!_externalTexturesTrash.empty()) {
            std::vector<GLsync> fences;
            fences.resize(_externalTexturesTrash.size());
            for (size_t i = 0; i < _externalTexturesTrash.size(); ++i) {
                fences[i] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
            }
            // External texture fences will be read in another thread/context, so we need a flush
            glFlush();
            size_t index = 0;
            for (auto pair : _externalTexturesTrash) {
                auto fence = fences[index++];
                pair.second(pair.first, fence);
            }
            _externalTexturesTrash.clear();
        }
    }
}

void VKBackend::beforeShutdownCleanup() {
    auto &recycler = _context.recycler;
    // Lock prevents destroying objects while hashes
    std::lock_guard<std::recursive_mutex> lockGuard(recycler.recyclerMutex);
    // Remove pointers to objects that were deleted during the frame.
    // This prevents access-after-delete.
    perFrameCleanup();

    // Delete remaining backend objects.
    for (auto framebuffer : _framebuffers) {
        framebuffer->_gpuObject.gpuObject.setGPUObject(nullptr);
    }
    _framebuffers.clear();

    for (auto buffer : _buffers) {
        buffer->_gpuObject.gpuObject.setGPUObject(nullptr);
    }
    _buffers.clear();

    for (auto texture : _textures) {
        texture->_gpuObject.gpuObject.setGPUObject(nullptr);
    }
    _textures.clear();

    for (auto query : _queries) {
        query->_gpuObject.gpuObject.setGPUObject(nullptr);
    }
    _queries.clear();

    // Deleted objects got added to recycler, so they need to be cleaned since sets are already empty.
    recycler.deletedFramebuffers.clear();
    recycler.deletedBuffers.clear();
    recycler.deletedTextures.clear();
    recycler.deletedQueries.clear();

    // One more cleanup to destroy Vulkan objects released by backend objects.
    perFrameCleanup();
}

void VKBackend::dumpVmaMemoryStats() {
    char *stats;
    vmaBuildStatsString(vks::Allocation::getAllocator(), &stats, VK_TRUE);
    std::ofstream statsFile;
    static QString pathTemplate = "/tmp/vma_{DATE}_{TIME}.json";
    QString fullPath = FileUtils::computeDocumentPath(FileUtils::replaceDateTimeTokens(pathTemplate));
    statsFile.open (fullPath.toStdString());
    statsFile << stats;
    statsFile.close();
    vmaFreeStatsString(vks::Allocation::getAllocator(), stats);
}

void VKBackend::releaseExternalTexture(GLuint id, const Texture::ExternalRecycler& recycler) {
    Lock lock(_externalTexturesMutex);
    _externalTexturesTrash.emplace_back(id, recycler);
}

void VKBackend::initBeforeFirstFrame() {
    initTransform();
    initDefaultTexture();
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

    //auto& drawCallInfoBuffer = batch.getDrawCallInfoBuffer();
    if (batch._currentNamedCall.empty()) {
        //auto& drawCallInfo = drawCallInfoBuffer[_currentDraw];
        if (_transform._enabledDrawcallInfoBuffer) {
            //glDisableVertexAttribArray(gpu::Stream::DRAW_CALL_INFO); // Make sure attrib array is disabled
            _transform._enabledDrawcallInfoBuffer = false;
        }
        // VKTODO
        // Since Vulkan has no glVertexAttrib equivalent we need to pass a buffer pointer here
        //glVertexAttribI2i(gpu::Stream::DRAW_CALL_INFO, drawCallInfo.index, drawCallInfo.unused);
        // Draw call info for unnamed calls starts at the beginning of the buffer, with offset dependent on _currentDraw
        VkDeviceSize vkOffset = _currentDraw * sizeof(gpu::Batch::DrawCallInfo);
        Q_ASSERT(_currentFrame->_drawCallInfoBuffer);
        auto gpuBuffer = syncGPUObject(*_currentFrame->_drawCallInfoBuffer);
        vkCmdBindVertexBuffers(_currentCommandBuffer, gpu::Stream::DRAW_CALL_INFO, 1, &gpuBuffer->buffer, &vkOffset);
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
        auto gpuBuffer = syncGPUObject(*_currentFrame->_drawCallInfoBuffer);
        vkCmdBindVertexBuffers(_currentCommandBuffer, gpu::Stream::DRAW_CALL_INFO, 1, &gpuBuffer->buffer, &vkOffset);
        //glBindVertexBuffer(gpu::Stream::DRAW_CALL_INFO, _transform._drawCallInfoBuffer, (GLintptr)_transform._drawCallInfoOffsets[batch._currentNamedCall], 2 * sizeof(GLushort));
    }

    // VKTODO: camera correction
    auto* cameraCorrectionObject = syncGPUObject(*_currentFrame->_cameraCorrectionBuffer._buffer);
    Q_ASSERT(cameraCorrectionObject);
    _uniform._buffers[gpu::slot::buffer::CameraCorrection].buffer = _currentFrame->_cameraCorrectionBuffer._buffer.get();
    _uniform._buffers[gpu::slot::buffer::CameraCorrection].offset = _currentFrame->_cameraCorrectionBuffer._offset;
    _uniform._buffers[gpu::slot::buffer::CameraCorrection].size = _currentFrame->_cameraCorrectionBuffer._size;
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

void VKBackend::transferGlUniforms() {
    auto size = _currentFrame->_glUniformData.size();
    if (size) {
        _currentFrame->_glUniformBuffer = std::make_shared<gpu::Buffer>(gpu::Buffer::UniformBuffer, size,
                                                                        _currentFrame->_glUniformData.data());
        _currentFrame->_glUniformBuffer->flush();
        _currentFrame->_buffers.push_back(_currentFrame->_glUniformBuffer);
    }
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
        _currentFrame->_cameraBuffer = std::make_shared<gpu::Buffer>(gpu::Buffer::UniformBuffer, bufferData.size(), bufferData.data());//vks::Buffer::createUniform(bufferData.size());
        _currentFrame->_cameraBuffer->flush();
        _currentFrame->_buffers.push_back(_currentFrame->_cameraBuffer);
        syncGPUObject(*_currentFrame->_cameraBuffer);
    }else{
        _currentFrame->_cameraBuffer.reset();
    }

    if (!batch._objects.empty()) {
        _currentFrame->_objectBuffer = std::make_shared<gpu::Buffer>(gpu::Buffer::ResourceBuffer,
                                                                     batch._objects.size() * sizeof(Batch::TransformObject),
                                                                     reinterpret_cast<const uint8_t*>(batch._objects.data()));//vks::Buffer::createStorage(batch._objects.size() * sizeof(Batch::TransformObject));
        _currentFrame->_objectBuffer->flush();
        syncGPUObject(*_currentFrame->_objectBuffer);
        _currentFrame->_buffers.push_back(_currentFrame->_objectBuffer);
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
        _currentFrame->_drawCallInfoBuffer = std::make_shared<gpu::Buffer>(gpu::Buffer::VertexBuffer,
                                                                           bufferData.size(),
                                                                           bufferData.data());
        _currentFrame->_drawCallInfoBuffer->flush();
        _currentFrame->_buffers.push_back(_currentFrame->_drawCallInfoBuffer);
    }else{
        _currentFrame->_drawCallInfoBuffer.reset();
    }

    // VKTODO
    if (_currentFrame->_objectBuffer) {
        _resource._buffers[slot::storage::ObjectTransforms].buffer = _currentFrame->_objectBuffer.get();
    }
    //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot::storage::ObjectTransforms, _transform._objectBuffer);

    // Make sure the current Camera offset is unknown before render Draw
    _transform._currentCameraOffset = INVALID_OFFSET;
}


void VKBackend::downloadFramebuffer(const FramebufferPointer& srcFramebuffer, const Vec4i& region, QImage& destImage) {
    // VKTODO
}

gpu::Primitive VKBackend::getPrimitiveTopologyFromCommand(Batch::Command command, const gpu::Batch& batch, size_t paramOffset) {
    Primitive primitiveType{};
    switch (command) {
        case Batch::COMMAND_draw:
            primitiveType = (Primitive)batch._params[paramOffset + 2]._uint;
            break;
        case Batch::COMMAND_drawIndexed:
            primitiveType = (Primitive)batch._params[paramOffset + 2]._uint;
            break;
        case Batch::COMMAND_drawInstanced:
            primitiveType = (Primitive)batch._params[paramOffset + 3]._uint;
            break;
        case Batch::COMMAND_drawIndexedInstanced:
            primitiveType = (Primitive)batch._params[paramOffset + 3]._uint;
            break;
        case Batch::COMMAND_multiDrawIndirect:
            primitiveType = (Primitive)batch._params[paramOffset + 1]._uint;
            break;
        case Batch::COMMAND_multiDrawIndexedIndirect:
            primitiveType = (Primitive)batch._params[paramOffset + 1]._uint;
            break;
        default:
            Q_ASSERT(false);
    }
    return primitiveType;
}

void VKBackend::do_draw(const Batch& batch, size_t paramOffset) {
    auto primitiveType = (Primitive)batch._params[paramOffset + 2]._uint;
    VkPrimitiveTopology mode = PRIMITIVE_TO_VK[primitiveType];
    uint32 numVertices = batch._params[paramOffset + 1]._uint;
    uint32 startVertex = batch._params[paramOffset + 0]._uint;

    draw(mode, numVertices, startVertex);
}

void VKBackend::do_drawIndexed(const Batch& batch, size_t paramOffset) {
    //Primitive primitiveType = (Primitive)batch._params[paramOffset + 2]._uint; // Do not remove, it's here for readability
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
        // VKTODO: what should vertexOffset be?
        // VKTODO: mode needs to be in the pipeline
        vkCmdDrawIndexed(_currentCommandBuffer, numIndices, 1, startIndex, 0, 0);
        //glDrawElements(mode, numIndices, glType, indexBufferByteOffset);
        _stats._DSNumTriangles += numIndices / 3;
        _stats._DSNumDrawcalls++;
    }
    _stats._DSNumAPIDrawcalls++;
}

void VKBackend::do_drawInstanced(const Batch& batch, size_t paramOffset) {
    int numInstances = batch._params[paramOffset + 4]._uint;
    //Primitive primitiveType = (Primitive)batch._params[paramOffset + 3]._uint; // Do not remove, it's here for readability
    uint32 numVertices = batch._params[paramOffset + 2]._uint;
    uint32 startVertex = batch._params[paramOffset + 1]._uint;


    if (isStereo()) {
        int trueNumInstances = 2 * numInstances;

#ifdef GPU_STEREO_DRAWCALL_INSTANCED
        vkCmdDraw(_currentCommandBuffer, numVertices, numInstances, startVertex, 0);
        //glDrawArraysInstanced(mode, startVertex, numVertices, trueNumInstances);
#else
        setupStereoSide(0);
        glDrawArraysInstanced(mode, startVertex, numVertices, numInstances);
        setupStereoSide(1);
        glDrawArraysInstanced(mode, startVertex, numVertices, numInstances);
#endif

        _stats._DSNumTriangles += (trueNumInstances * numVertices) / 3;
        // VKTODO: should that count as 2 draw calls for whole set of instances?
        _stats._DSNumDrawcalls += trueNumInstances;
    } else {
        vkCmdDraw(_currentCommandBuffer, numVertices, numInstances, startVertex, 0);
        //glDrawArraysInstanced(mode, startVertex, numVertices, numInstances);
        _stats._DSNumTriangles += (numInstances * numVertices) / 3;
        // VKTODO: should that count as 1 draw call for whole set of instances?
        _stats._DSNumDrawcalls += numInstances;
    }
    _stats._DSNumAPIDrawcalls++;
}

void VKBackend::do_drawIndexedInstanced(const Batch& batch, size_t paramOffset) {
    int numInstances = batch._params[paramOffset + 4]._uint;
    //Primitive primitiveType = (Primitive)batch._params[paramOffset + 3]._uint; // Do not remove, it's here for readability
    uint32 numIndices = batch._params[paramOffset + 2]._uint;
    uint32 startIndex = batch._params[paramOffset + 1]._uint;
    //uint32 startInstance = batch._params[paramOffset + 0]._uint; // VKTODO: what to do with this
    //GLenum glType = gl::ELEMENT_TYPE_TO_GL[_input._indexBufferType];
    //auto typeByteSize = TYPE_SIZE[_input._indexBufferType];
    //void* indexBufferByteOffset = reinterpret_cast<void*>(startIndex * typeByteSize + _input._indexBufferOffset);

    if (isStereo()) {
        int trueNumInstances = 2 * numInstances;

#ifdef GPU_STEREO_DRAWCALL_INSTANCED
        vkCmdDrawIndexed(_currentCommandBuffer, numIndices, trueNumInstances, startIndex, 0, 0); // VKTODO: where to get vertexOffset?
        //glDrawElementsInstancedBaseVertexBaseInstance(mode, numIndices, glType, indexBufferByteOffset, trueNumInstances, 0, startInstance);
#else
        setupStereoSide(0);
        glDrawElementsInstancedBaseVertexBaseInstance(mode, numIndices, glType, indexBufferByteOffset, numInstances, 0, startInstance);
        setupStereoSide(1);
        glDrawElementsInstancedBaseVertexBaseInstance(mode, numIndices, glType, indexBufferByteOffset, numInstances, 0, startInstance);
#endif
        _stats._DSNumTriangles += (trueNumInstances * numIndices) / 3;
        _stats._DSNumDrawcalls += trueNumInstances;
    } else {
        vkCmdDrawIndexed(_currentCommandBuffer, numIndices, numInstances, startIndex, 0, 0); // VKTODO: where to get vertexOffset?
        //glDrawElementsInstancedBaseVertexBaseInstance(mode, numIndices, glType, indexBufferByteOffset, numInstances, 0, startInstance);
        _stats._DSNumTriangles += (numInstances * numIndices) / 3;
        _stats._DSNumDrawcalls += numInstances;
    }

    _stats._DSNumAPIDrawcalls++;
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

    resetRenderPass();
    _hasFramebufferChanged = true;
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
        resetRenderPass();
        _hasFramebufferChanged = true;
    }
}

void VKBackend::do_clearFramebuffer(const Batch& batch, size_t paramOffset) {
    // VKTODO: This could possibly be optimized by clearing on next render pass.
    // VKTODO: use vkCmdClearColorImage(), vkCmdClearDepthStencilImage() or vkCmdClearAttachments() instead

    if (_stereo._enable && !_cache.pipelineState.pipeline->getState()->isScissorEnable()) {
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
    //int useScissor = batch._params[paramOffset + 0]._int; // VKTODO: what to do with this?

    auto framebuffer = _cache.pipelineState.framebuffer;
    auto gpuFramebuffer = syncGPUObject(*framebuffer);
    auto &renderBuffers = framebuffer->getRenderBuffers();

    Cache::Pipeline::RenderpassKey key = _cache.pipelineState.getRenderPassKey(framebuffer);
    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkClearValue> clearValues;
    attachments.reserve(key.size());
    std::vector<VkAttachmentReference> colorAttachmentReferences;
    VkAttachmentReference depthReference{};
    for (size_t i = 0; i < key.size(); i++) {
        const auto& formatAndLayout = key[i];
        /*if (surface) {
            Q_ASSERT(TextureUsageType::RENDERBUFFER == surface->getUsageType());
            vkTexture = backend->syncGPUObject(*surface.get());*/
        auto texture = renderBuffers[i]._texture;
        if (!texture) {
            // Last texture of the key can be depth stencil attachment
            Q_ASSERT(i + 1 == key.size());
            texture = framebuffer->getDepthStencilBuffer();
        }
        Q_ASSERT(texture);
        VKAttachmentTexture *attachmentTexture = nullptr;
        //if (texture) {
        auto gpuTexture = syncGPUObject(*texture);
        Q_ASSERT(gpuTexture);
        attachmentTexture = dynamic_cast<VKAttachmentTexture*>(gpuTexture);
        Q_ASSERT(attachmentTexture);
        //}
        VkAttachmentDescription attachment{};
        attachment.format = formatAndLayout.first;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        if (masks & Framebuffer::BUFFER_STENCIL) {
            attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        } else {
            attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        }
        //attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        if (texture->isDepthStencilRenderTarget()) {
            clearValues.push_back(VkClearValue{.depthStencil =  VkClearDepthStencilValue{ depth, (uint32_t)stencil }});
            if (masks & Framebuffer::BUFFER_DEPTH) {
                attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                if (!formatHasStencil(attachment.format)) {
                    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                }
            } else {
                attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            }
            // Texture state needs to be updated

            if (((masks & Framebuffer::BUFFER_DEPTH) && (masks & Framebuffer::BUFFER_STENCIL))
                || ((masks & Framebuffer::BUFFER_DEPTH) && !formatHasStencil(attachment.format))) {
                attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                //Q_ASSERT(attachmentTexture->_vkImageLayout != VK_IMAGE_LAYOUT_GENERAL); //
                attachmentTexture->_vkImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            } else {
                if (attachmentTexture->_vkImageLayout == VK_IMAGE_LAYOUT_GENERAL) {
                    attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
                    attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
                    depthReference.layout = VK_IMAGE_LAYOUT_GENERAL;
                } else {
                    attachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                    attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                    attachmentTexture->_vkImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                }
            }
            depthReference.attachment = (uint32_t)(attachments.size());
        } else {
            clearValues.push_back(VkClearValue{.color =  VkClearColorValue{.float32 =  { color.x, color.y, color.z, color.w }}});
            if (masks & Framebuffer::BUFFER_COLORS) {
                attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            } else {
                attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            }
            attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            // Texture state needs to be updated
            if (attachmentTexture) {
                Q_ASSERT(attachmentTexture->_vkImageLayout != VK_IMAGE_LAYOUT_GENERAL);
                attachmentTexture->_vkImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                //Q_ASSERT(attachmentTexture->_gpuObject.isColorRenderTarget()); // isColorRenderTarget is broken
            }
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
        // VKTODO Is that correct?
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

    /*static const std::array<VkClearValue, 2> clearValuesx{
        VkClearValue{.color =  VkClearColorValue{.float32 =  { color.x, color.y, color.z, color.w }}},
        VkClearValue{.depthStencil =  VkClearDepthStencilValue{ depth, (uint32_t)stencil }},
    };*/


    VkRenderPass renderPass;
    VK_CHECK_RESULT(vkCreateRenderPass(_context.device->logicalDevice, &renderPassInfo, nullptr, &renderPass));

    auto rect = VkRect2D{VkOffset2D{0, 0},
                         VkExtent2D{framebuffer->getWidth(), framebuffer->getHeight()}};
    VkRenderPassBeginInfo beginInfo = vks::initializers::renderPassBeginInfo();
    beginInfo.renderPass = renderPass;
    beginInfo.framebuffer = gpuFramebuffer->vkFramebuffer;
    beginInfo.renderArea = rect;
    beginInfo.clearValueCount = (uint32_t)clearValues.size();
    beginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(_currentCommandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(_currentCommandBuffer);
    _currentFrame->_renderPasses.push_back(renderPass); // To be deleted after frame rendering is complete

    // VKTODO: some sort of barrier is needed?
    for (auto &renderBuffer : renderBuffers) {
        auto texture = renderBuffer._texture;
        if (!texture) {
            continue;
        }
        auto gpuTexture = syncGPUObject(*texture);
        Q_ASSERT(gpuTexture);
        auto *attachmentTexture = dynamic_cast<VKAttachmentTexture*>(gpuTexture);
        Q_ASSERT(attachmentTexture);
        if (masks & Framebuffer::BUFFER_COLORS) {
            if (attachmentTexture->_gpuObject.isDepthStencilRenderTarget()) {
                // VKTODO: Check if the same depth render target is used as one of the inputs, if so then don't update it here
                VkImageSubresourceRange mipSubRange = {};
                mipSubRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
                mipSubRange.baseMipLevel = 0;
                mipSubRange.levelCount = 1;
                mipSubRange.layerCount = 1;
                vks::tools::insertImageMemoryBarrier(_currentCommandBuffer, attachmentTexture->_vkImage,
                                                     VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                                     VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                                     VK_IMAGE_LAYOUT_UNDEFINED,
                                                     VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,  // VKTODO: should be
                                                     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,        // VKTODO
                                                     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, mipSubRange); // VKTODO: what stage mask for depth stencil?
                Q_ASSERT(attachmentTexture->_vkImageLayout != VK_IMAGE_LAYOUT_GENERAL);
                attachmentTexture->_vkImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            } else {
                VkImageSubresourceRange mipSubRange = {};
                mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                mipSubRange.baseMipLevel = 0;
                mipSubRange.levelCount = 1;
                mipSubRange.layerCount = 1;
                vks::tools::insertImageMemoryBarrier(_currentCommandBuffer, attachmentTexture->_vkImage,
                                                     VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                                     VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,  // VKTODO: should be
                                                     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,        // VKTODO
                                                     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, mipSubRange);
                attachmentTexture->_vkImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }
        }
    }

    if (masks & Framebuffer::BUFFER_STENCIL || masks & Framebuffer::BUFFER_DEPTH) {
        auto texture = framebuffer->getDepthStencilBuffer();
        Q_ASSERT(texture);
        auto gpuTexture = syncGPUObject(*texture);
        Q_ASSERT(gpuTexture);
        auto *attachmentTexture = dynamic_cast<VKAttachmentTexture*>(gpuTexture);
        Q_ASSERT(attachmentTexture);

        if ( attachmentTexture->_vkImage == (VkImage)(0x260000000026UL)) {
            printf("0x260000000026UL");
        }

        VkImageSubresourceRange mipSubRange = {};
        mipSubRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (formatHasStencil(evalTexelFormatInternal(texture->getTexelFormat()))) {
            mipSubRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        mipSubRange.baseMipLevel = 0;
        mipSubRange.levelCount = 1;
        mipSubRange.layerCount = 1;
        VkImageLayout layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        if (attachmentTexture->_vkImageLayout == VK_IMAGE_LAYOUT_GENERAL) {
            layout = VK_IMAGE_LAYOUT_GENERAL;
        }
        vks::tools::insertImageMemoryBarrier(_currentCommandBuffer, attachmentTexture->_vkImage,
                                             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                             layout,
                                             layout,  // VKTODO: should be
                                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,        // VKTODO
                                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, mipSubRange); // VKTODO: what stage mask for depth stencil?

        attachmentTexture->_vkImageLayout = layout;
    }

    /*VKuint glmask = 0;
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
    }*/

    // Apply scissor if needed and if not already on
    // VKTODO: how to do this here?
    /*bool doEnableScissor = (useScissor && (!_pipeline._stateCache.scissorEnable));
    if (doEnableScissor) {
        glEnable(VK_SCISSOR_TEST);
    }*/

    // Clear!
    //glClear(glmask);

    // VKTODO: how to do this here?
    // Restore scissor if needed
    /*if (doEnableScissor) {
        glDisable(VK_SCISSOR_TEST);
    }*/

    // Restore write mask meaning turn back off
    /*if (restoreDepthMask) {
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
    auto srcframebuffer = batch._framebuffers.get(batch._params[paramOffset]._uint);
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
    auto dstFbo = syncGPUObject(*dstframebuffer);
    auto srcFbo = syncGPUObject(*srcframebuffer);

    // Sometimes framebuffer with multiple attachments is blitted into one with a single attachment.
    auto &srcRenderBuffers = srcFbo->_gpuObject.getRenderBuffers();
    auto srcDepthStencilBuffer = srcFbo->_gpuObject.getDepthStencilBuffer();
    auto &dstRenderBuffers = dstFbo->_gpuObject.getRenderBuffers();
    auto dstDepthStencilBuffer = dstFbo->_gpuObject.getDepthStencilBuffer();

    for (size_t i = 0; i < srcRenderBuffers.size(); i++) {
        if (srcRenderBuffers[i]._texture && dstRenderBuffers[i]._texture) {
            auto source = syncGPUObject(*srcRenderBuffers[i]._texture);
            auto destination = syncGPUObject(*dstRenderBuffers[i]._texture);
            if (source && destination) {
                auto srcAttachmentTexture = dynamic_cast<VKAttachmentTexture*>(source);
                auto dstAttachmentTexture = dynamic_cast<VKAttachmentTexture*>(destination);
                if (srcAttachmentTexture && dstAttachmentTexture) {
                    blitToFramebuffer(*srcAttachmentTexture, srcvp, *dstAttachmentTexture, dstvp);
                }
            }
        }
    }
    if (srcDepthStencilBuffer && dstDepthStencilBuffer) {
        auto source = syncGPUObject(*srcDepthStencilBuffer);
        auto destination = syncGPUObject(*dstDepthStencilBuffer);
        if (source && destination) {
            auto srcAttachmentTexture = dynamic_cast<VKAttachmentTexture*>(source);
            auto dstAttachmentTexture = dynamic_cast<VKAttachmentTexture*>(destination);
            if (srcAttachmentTexture && dstAttachmentTexture) {
                blitToFramebuffer(*srcAttachmentTexture, srcvp, *dstAttachmentTexture, dstvp);
            }
        }
    }
}

void VKBackend::do_setInputFormat(const Batch& batch, size_t paramOffset) {
    const auto& format = batch._streamFormats.get(batch._params[paramOffset]._uint);
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
                //vkBuffer = VKBuffer::getBuffer(*this, *buffer);
                auto backendBuffer = syncGPUObject(*buffer);
                if (backendBuffer) {
                    vkBuffer = backendBuffer->buffer;
                }
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
    gpu::Offset newOffset = batch._params[paramOffset + 0]._uint;

    BufferPointer indexBuffer = batch._buffers.get(batch._params[paramOffset + 1]._uint);
    if (indexBuffer.get() != _input._indexBuffer || newOffset != _input._indexBufferOffset) {
        _input._indexBuffer = indexBuffer.get();
        _input._indexBufferOffset = batch._params[paramOffset + 0]._uint;
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
            auto backendBuffer = syncGPUObject(*indexBuffer);
            VkBuffer vkBuffer = VK_NULL_HANDLE;
            if (backendBuffer) {
                vkBuffer = backendBuffer->buffer;
            }
            Q_ASSERT(vkBuffer != VK_NULL_HANDLE);
            vkCmdBindIndexBuffer(_currentCommandBuffer, vkBuffer, _input._indexBufferOffset, indexType);
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

void VKBackend::do_generateTextureMipsWithPipeline(const Batch& batch, size_t paramOffset) {
    // VKTODO
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

    /*if (!_inRenderTransferPass && !isStereo()) {
        ivec4& vp = _transform._viewport;
    }*/

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
    uint32_t slot = batch._params[paramOffset + 1]._uint;
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
    //setResourceTexture(slot, resourceTexture);

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

    _resource._textures[slot].texture = resourceTexture.get();
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
    (&::gpu::vk::VKBackend::do_generateTextureMipsWithPipeline),

    (&::gpu::vk::VKBackend::do_advance),

    (&::gpu::vk::VKBackend::do_beginQuery),
    (&::gpu::vk::VKBackend::do_endQuery),
    (&::gpu::vk::VKBackend::do_getQuery),

    (&::gpu::vk::VKBackend::do_resetStages),

    (&::gpu::vk::VKBackend::do_disableContextViewCorrection),
    (&::gpu::vk::VKBackend::do_restoreContextViewCorrection),
    (&::gpu::vk::VKBackend::do_setContextMirrorViewCorrection),

    (&::gpu::vk::VKBackend::do_disableContextStereo),
    (&::gpu::vk::VKBackend::do_restoreContextStereo),

    (&::gpu::vk::VKBackend::do_runLambda),

    (&::gpu::vk::VKBackend::do_startNamedCall),
    (&::gpu::vk::VKBackend::do_stopNamedCall),

    (&::gpu::vk::VKBackend::do_glUniform1f), // Seems to be deprecated?
    (&::gpu::vk::VKBackend::do_glUniform2f),
    (&::gpu::vk::VKBackend::do_glUniform3f),
    (&::gpu::vk::VKBackend::do_glUniform4f),
    (&::gpu::vk::VKBackend::do_glUniform3fv),
    (&::gpu::vk::VKBackend::do_glUniform4fv),
    (&::gpu::vk::VKBackend::do_glUniform4iv),
    (&::gpu::vk::VKBackend::do_glUniformMatrix3fv),
    (&::gpu::vk::VKBackend::do_glUniformMatrix4fv),

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

// VKTODO: all syncGPUObject calls should be done before the frame drawing begins