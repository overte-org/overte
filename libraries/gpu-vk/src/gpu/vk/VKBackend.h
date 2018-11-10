//
//  Created by Bradley Austin Davis on 2016/08/07
//  Copyright 2013-2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_vk_VKBackend_h
#define hifi_gpu_vk_VKBackend_h

#include <assert.h>
#include <functional>
#include <memory>
#include <bitset>
#include <queue>
#include <utility>
#include <list>
#include <array>

#include <gpu/Forward.h>
#include <gpu/Context.h>

#include <vk/Config.h>
#include <vk/Context.h>
#include <vk/Debug.h>

#include "VKForward.h"

namespace gpu { namespace vulkan {

class VKBackend : public Backend, public std::enable_shared_from_this<VKBackend> {
    // Context Backend static interface required
    friend class gpu::Context;
    static void init();
    static BackendPointer createBackend();

public:
    VKBackend();
    ~VKBackend();
    void syncCache() override {}
    void recycle() const override {}
    void setCameraCorrection(const Mat4& correction, const Mat4& prevRenderView, bool reset = false) override {}
    uint32_t getTextureID(const TexturePointer&) override { return 0; }
    void executeFrame(const FramePointer& frame) final override;
    bool isTextureManagementSparseEnabled() const override;
    bool supportedTextureFormat(const gpu::Element& format) const override;
    const std::string& getVersion() const override;
    void downloadFramebuffer(const FramebufferPointer& srcFramebuffer, const Vec4i& region, QImage& destImage) final override {}

    void trash(const VKBuffer& buffer);

#if 0
    // Draw Stage
    virtual void do_draw(const Batch& batch, size_t paramOffset) final;
    virtual void do_drawIndexed(const Batch& batch, size_t paramOffset) final;
    virtual void do_drawInstanced(const Batch& batch, size_t paramOffset) final;
    virtual void do_drawIndexedInstanced(const Batch& batch, size_t paramOffset) final;
    virtual void do_multiDrawIndirect(const Batch& batch, size_t paramOffset) final;
    virtual void do_multiDrawIndexedIndirect(const Batch& batch, size_t paramOffset) final;

    // Input Stage
    virtual void do_setInputFormat(const Batch& batch, size_t paramOffset) final;
    virtual void do_setInputBuffer(const Batch& batch, size_t paramOffset) final;
    virtual void do_setIndexBuffer(const Batch& batch, size_t paramOffset) final;
    virtual void do_setIndirectBuffer(const Batch& batch, size_t paramOffset) final;
    virtual void do_generateTextureMips(const Batch& batch, size_t paramOffset) final;

    virtual void do_glUniform1f(const Batch& batch, size_t paramOffset) final;
    virtual void do_glUniform2f(const Batch& batch, size_t paramOffset) final;
    virtual void do_glUniform3f(const Batch& batch, size_t paramOffset) final;
    virtual void do_glUniform4f(const Batch& batch, size_t paramOffset) final;
    virtual void do_glColor4f(const Batch& batch, size_t paramOffset) final;

    // Transform Stage
    virtual void do_setModelTransform(const Batch& batch, size_t paramOffset) final;
    virtual void do_setViewTransform(const Batch& batch, size_t paramOffset) final;
    virtual void do_setProjectionTransform(const Batch& batch, size_t paramOffset) final;
    virtual void do_setProjectionJitter(const Batch& batch, size_t paramOffset) final;
    virtual void do_setViewportTransform(const Batch& batch, size_t paramOffset) final;
    virtual void do_setDepthRangeTransform(const Batch& batch, size_t paramOffset) final;

    // Uniform Stage
    virtual void do_setUniformBuffer(const Batch& batch, size_t paramOffset) final;

    // Resource Stage
    virtual void do_setResourceBuffer(const Batch& batch, size_t paramOffset) final;
    virtual void do_setResourceTexture(const Batch& batch, size_t paramOffset) final;
    virtual void do_setResourceTextureTable(const Batch& batch, size_t paramOffset);
    virtual void do_setResourceFramebufferSwapChainTexture(const Batch& batch, size_t paramOffset) final;

    // Pipeline Stage
    virtual void do_setPipeline(const Batch& batch, size_t paramOffset) final;

    // Output stage
    virtual void do_setFramebuffer(const Batch& batch, size_t paramOffset) final;
    virtual void do_setFramebufferSwapChain(const Batch& batch, size_t paramOffset) final;
    virtual void do_clearFramebuffer(const Batch& batch, size_t paramOffset) final;
    virtual void do_blit(const Batch& batch, size_t paramOffset) final;
    virtual void do_advance(const Batch& batch, size_t paramOffset) final;
    virtual void do_setStateBlendFactor(const Batch& batch, size_t paramOffset) final;
    virtual void do_setStateScissorRect(const Batch& batch, size_t paramOffset) final;

    // Query section
    virtual void do_beginQuery(const Batch& batch, size_t paramOffset) final;
    virtual void do_endQuery(const Batch& batch, size_t paramOffset) final;
    virtual void do_getQuery(const Batch& batch, size_t paramOffset) final;

    // Reset stages
    virtual void do_resetStages(const Batch& batch, size_t paramOffset) final;
    virtual void do_disableContextViewCorrection(const Batch& batch, size_t paramOffset) final;
    virtual void do_restoreContextViewCorrection(const Batch& batch, size_t paramOffset) final;
    virtual void do_disableContextStereo(const Batch& batch, size_t paramOffset) final;
    virtual void do_restoreContextStereo(const Batch& batch, size_t paramOffset) final;

    // Other
    virtual void do_runLambda(const Batch& batch, size_t paramOffset) final;
    virtual void do_startNamedCall(const Batch& batch, size_t paramOffset) final;
    virtual void do_stopNamedCall(const Batch& batch, size_t paramOffset) final;

    // Performance profiling markers
    virtual void do_pushProfileRange(const Batch& batch, size_t paramOffset) final;
    virtual void do_popProfileRange(const Batch& batch, size_t paramOffset) final;
#endif
protected:
    // Logical device, application's view of the physical device (GPU)
    // vk::Pipeline cache object
    vk::PipelineCache _pipelineCache;

    vks::Context& _context{ vks::Context::get() };
    const vk::PhysicalDeviceProperties& _deviceProperties { _context.deviceProperties };
    const vk::PhysicalDeviceFeatures& _deviceFeatures { _context.deviceFeatures };
    const vk::PhysicalDeviceMemoryProperties& _memoryProperties { _context.deviceMemoryProperties };
    const vk::Device& _device{ _context.device };
    const vk::Queue _graphicsQueue;
    const vk::Queue _transferQueue;
    friend class VKBuffer;
};

}}  // namespace gpu::vulkan

#endif
