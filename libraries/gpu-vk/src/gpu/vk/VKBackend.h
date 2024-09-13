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
#include <vk/VulkanDebug.h>
#include <vulkan/vulkan_core.h>

#include "VKForward.h"
#include "../../../../vk/src/vk/Context.h"

//#define GPU_STEREO_TECHNIQUE_DOUBLED_SMARTER
#define GPU_STEREO_TECHNIQUE_INSTANCED

// Let these be configured by the one define picked above
#ifdef GPU_STEREO_TECHNIQUE_DOUBLED_SIMPLE
#define GPU_STEREO_DRAWCALL_DOUBLED
#endif

#ifdef GPU_STEREO_TECHNIQUE_DOUBLED_SMARTER
#define GPU_STEREO_DRAWCALL_DOUBLED
#define GPU_STEREO_CAMERA_BUFFER
#endif

#ifdef GPU_STEREO_TECHNIQUE_INSTANCED
#define GPU_STEREO_DRAWCALL_INSTANCED
#define GPU_STEREO_CAMERA_BUFFER
#endif

namespace gpu { namespace vulkan {

class VKInputFormat : public GPUObject {
public:
    static VKInputFormat* sync(const Stream::Format& inputFormat);

    VKInputFormat();
    ~VKInputFormat();

    std::string key;
};

class VKBackend : public Backend, public std::enable_shared_from_this<VKBackend> {
    // Context Backend static interface required
    friend class gpu::Context;
    static void init();
    static BackendPointer createBackend();

protected:
    // Allows for correction of the camera pose to account for changes
    // between the time when a was recorded and the time(s) when it is
    // executed
    // Prev is the previous correction used at previous frame
    struct CameraCorrection {
        mat4 correction;
        mat4 correctionInverse;
        mat4 prevView;
        mat4 prevViewInverse;
    };

    struct TransformStageState {
#ifdef GPU_STEREO_CAMERA_BUFFER
        struct Cameras {
            TransformCamera _cams[2];

            Cameras(){};
            Cameras(const TransformCamera& cam) { _cams[0] = cam; };
            Cameras(const TransformCamera& camL, const TransformCamera& camR) {
                _cams[0] = camL;
                _cams[1] = camR;
            };
        };

        using CameraBufferElement = Cameras;
#else
        using CameraBufferElement = TransformCamera;
#endif
        using TransformCameras = std::vector<CameraBufferElement>;

        TransformCamera _camera;
        TransformCameras _cameras;

        mutable std::map<std::string, void*> _drawCallInfoOffsets;

        uint32_t _objectBuffer{ 0 };
        uint32_t _cameraBuffer{ 0 };
        uint32_t _drawCallInfoBuffer{ 0 };
        uint32_t _objectBufferTexture{ 0 };
        size_t _cameraUboSize{ 0 };
        bool _viewIsCamera{ false };
        bool _skybox{ false };
        Transform _view;
        CameraCorrection _correction;
        bool _viewCorrectionEnabled{ true };

        Mat4 _projection;
        Vec4i _viewport{ 0, 0, 1, 1 };
        Vec2 _depthRange{ 0.0f, 1.0f };
        Vec2 _projectionJitter{ 0.0f, 0.0f };
        bool _invalidView{ false };
        bool _invalidProj{ false };
        bool _invalidViewport{ false };

        bool _enabledDrawcallInfoBuffer{ false };

        using Pair = std::pair<size_t, size_t>;
        using List = std::list<Pair>;
        List _cameraOffsets;
        mutable List::const_iterator _camerasItr;
        mutable size_t _currentCameraOffset{ INVALID_OFFSET };

        void preUpdate(size_t commandIndex, const StereoState& stereo, Vec2u framebufferSize);
        void update(size_t commandIndex, const StereoState& stereo) const;
        void bindCurrentCamera(int stereoSide) const;
    } _transform;

    static const int MAX_NUM_ATTRIBUTES = Stream::NUM_INPUT_SLOTS;
    // The drawcall Info attribute  channel is reserved and is the upper bound for the number of availables Input buffers
    static const int MAX_NUM_INPUT_BUFFERS = Stream::DRAW_CALL_INFO;

    struct InputStageState {
        bool _invalidFormat { true };
        bool _lastUpdateStereoState { false };
        FormatReference _format { GPU_REFERENCE_INIT_VALUE };
        std::string _formatKey;

        typedef std::bitset<MAX_NUM_ATTRIBUTES> ActivationCache;
        ActivationCache _attributeActivation { 0 };

        typedef std::bitset<MAX_NUM_INPUT_BUFFERS> BuffersState;

        BuffersState _invalidBuffers { 0 };
        BuffersState _attribBindingBuffers { 0 };

        std::array<BufferReference, MAX_NUM_INPUT_BUFFERS> _buffers;
        std::array<Offset, MAX_NUM_INPUT_BUFFERS> _bufferOffsets;
        std::array<Offset, MAX_NUM_INPUT_BUFFERS> _bufferStrides;
        std::array<uint32_t, MAX_NUM_INPUT_BUFFERS> _bufferVBOs;

        BufferReference _indexBuffer;
        Offset _indexBufferOffset { 0 };
        Type _indexBufferType { UINT32 };

        BufferReference _indirectBuffer;
        Offset _indirectBufferOffset { 0 };
        Offset _indirectBufferStride { 0 };

        uint32_t _defaultVAO { 0 };
    } _input;

    void draw(VkPrimitiveTopology mode, uint32 numVertices, uint32 startVertex);
    void renderPassTransfer(const Batch& batch);
    void renderPassDraw(const Batch& batch);
    void transferTransformState(const Batch& batch) const;
    void updateInput();
    void updateTransform(const Batch& batch);
    void updatePipeline();

    vulkan::VKFramebuffer* syncGPUObject(const Framebuffer& framebuffer);
    VKBuffer* syncGPUObject(const Buffer& buffer);
    VKTexture* syncGPUObject(const TexturePointer& texture);
    VKQuery* syncGPUObject(const Query& query);

public:
    VKBackend();
    ~VKBackend();
    vks::Context& getContext() { return _context; }
    void syncProgram(const gpu::ShaderPointer& program) override {}
    void syncCache() override {}
    void recycle() const override {}
    void setCameraCorrection(const Mat4& correction, const Mat4& prevRenderView, bool reset = false) override;
    uint32_t getTextureID(const TexturePointer&) override { return 0; }
    void executeFrame(const FramePointer& frame) final;
    bool isTextureManagementSparseEnabled() const override;
    bool supportedTextureFormat(const gpu::Element& format) const override;
    const std::string& getVersion() const override;
    void downloadFramebuffer(const FramebufferPointer& srcFramebuffer, const Vec4i& region, QImage& destImage) final;
    void setDrawCommandBuffer(VkCommandBuffer commandBuffer);

    void trash(const VKBuffer& buffer);

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
    virtual void do_setResourceTextureTable(const Batch& batch, size_t paramOffset) {}; // VKTODO: not needed currently, to be implemented in the future
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

protected:
    // Logical device, application's view of the physical device (GPU)
    // VkPipeline cache object
    VkPipelineCache _pipelineCache;

    vks::Context& _context{ vks::Context::get() };
    VkQueue _graphicsQueue; //TODO: initialize from device
    VkQueue _transferQueue; //TODO: initialize from device
    friend class VKBuffer;
    friend class VKFramebuffer;
    VkCommandBuffer _currentCommandBuffer;
    size_t _commandIndex{ 0 };
    int _currentDraw{ -1 };
    bool _inRenderTransferPass{ false };
    // VKTODO: maybe move to _transform?
    Vec4i _currentScissorRect{ 0 };

    typedef void (VKBackend::*CommandCall)(const Batch&, size_t);
    static std::array<VKBackend::CommandCall, Batch::NUM_COMMANDS> _commandCalls;
    static const size_t INVALID_OFFSET = (size_t)-1;
};

}}  // namespace gpu::vulkan

#endif
