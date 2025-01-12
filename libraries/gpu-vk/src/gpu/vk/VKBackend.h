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
#include <glad/glad.h>

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

namespace gpu { namespace vk {

class VKAttachmentTexture;

static const int MAX_NUM_UNIFORM_BUFFERS = 14; // There's also camera buffer at slot 15

static const int32_t MIN_REQUIRED_TEXTURE_IMAGE_UNITS = 16;
static const int32_t MIN_REQUIRED_COMBINED_UNIFORM_BLOCKS = 70;
static const int32_t MIN_REQUIRED_COMBINED_TEXTURE_IMAGE_UNITS = 48;
static const int32_t MIN_REQUIRED_UNIFORM_BUFFER_BINDINGS = 36;
static const int32_t MIN_REQUIRED_UNIFORM_LOCATIONS = 1024;

static const int MAX_NUM_RESOURCE_BUFFERS = 16;
static const int MAX_NUM_RESOURCE_TEXTURES = 16;

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
    class FrameData;
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

    struct UniformStageState;

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

        mutable std::map<std::string, VkDeviceSize> _drawCallInfoOffsets;

        //uint32_t _objectBufferTexture{ 0 };
        size_t _cameraUboSize{ 0 };
        bool _viewIsCamera{ false };
        bool _skybox{ false };
        Transform _view;
        CameraCorrection _correction;
        bool _viewCorrectionEnabled{ true };
        // This is set by frame player to override camera correction setting
        bool _viewCorrectionEnabledForFramePlayer{ false };

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
        void update(size_t commandIndex, const StereoState& stereo, VKBackend::UniformStageState &uniform, FrameData &currentFrame) const;
        void bindCurrentCamera(int stereoSide, VKBackend::UniformStageState &uniform, FrameData &currentFrame) const;
    } _transform;

protected:
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
        std::array<VkBuffer, MAX_NUM_INPUT_BUFFERS> _bufferVBOs;

        BufferReference _indexBuffer;
        Offset _indexBufferOffset { 0 };
        Type _indexBufferType { UINT32 };

        BufferReference _indirectBuffer;
        Offset _indirectBufferOffset { 0 };
        Offset _indirectBufferStride { 0 };

        uint32_t _defaultVAO { 0 };

        void reset();
    } _input;

    void resetInputStage();

    struct UniformStageState {
        struct BufferState {
            // Only one of buffer or vksBuffer may be not NULL
            BufferReference buffer{};
            vks::Buffer *vksBuffer{};
            uint32_t offset{ 0 }; // VKTODO: is it correct type
            uint32_t size{ 0 }; // VKTODO: is it correct type

            BufferState& operator=(const BufferState& other) = delete;
            void reset() {
                gpu::reset(buffer);
                gpu::reset(vksBuffer);
                offset = 0;
                size = 0;
            }

            /*bool compare(const BufferPointer& buffer, uint32_t offset, uint32_t size) {
                const auto& self = *this;
                return (self.offset == offset && self.size == size && gpu::compare(self.buffer, buffer));
            }*/
        };

        // MAX_NUM_UNIFORM_BUFFERS-1 is the max uniform index BATCHES are allowed to set, but
        // MIN_REQUIRED_UNIFORM_BUFFER_BINDINGS is used here because the backend sets some
        // internal UBOs for things like camera correction
        std::array<BufferState, MIN_REQUIRED_UNIFORM_BUFFER_BINDINGS> _buffers;
    } _uniform;

    void updateVkDescriptorWriteSetsUniform(VkDescriptorSet target);
    void releaseUniformBuffer(uint32_t slot);
    void resetUniformStage();

    // VKTODO
    struct ResourceStageState {
        struct TextureState {
            TextureReference texture{};
            TextureState& operator=(const TextureState& other) = delete;
            void reset() {
                gpu::reset(texture);
            }
        };
        struct BufferState {
            BufferReference buffer{};
            vks::Buffer *vksBuffer{};
            BufferState& operator=(const BufferState& other) = delete;
            void reset() {
                gpu::reset(buffer);
                gpu::reset(vksBuffer);
            }
        };
        std::array<BufferState, MAX_NUM_RESOURCE_BUFFERS> _buffers{};
        std::array<TextureState, MAX_NUM_RESOURCE_TEXTURES> _textures{};
        //int findEmptyTextureSlot() const;
    } _resource;

    void updateVkDescriptorWriteSetsTexture(VkDescriptorSet target);
    void bindResourceTexture(uint32_t slot, const TexturePointer& texture);
    void releaseResourceTexture(uint32_t slot);
    void resetTextureStage();

    void updateVkDescriptorWriteSetsStorage(VkDescriptorSet target);
    void releaseResourceBuffer(uint32_t slot);
    void resetResourceStage();

    // VKTODO
    /*struct OutputStageState {
        FramebufferReference _framebuffer{};
        int _drawFBO{ 0 };
    } _output;*/

    // VKTODO
    struct QueryStageState {
        uint32_t _rangeQueryDepth{ 0 };
    } _queryStage;

    void resetQueryStage();

    VkRenderPass _currentVkRenderPass{ VK_NULL_HANDLE };
    gpu::FramebufferReference _currentFramebuffer{ nullptr }; // Framebuffer used in currently happening render pass
    VkFramebuffer _currentVkFramebuffer{ VK_NULL_HANDLE }; // Framebuffer used in currently happening render pass
    bool _hasFramebufferChanged {false}; // Set to true when batch calls setFramebuffer command. Used to end render pass and update input image layouts.
    // Checks if renderpass change is needed and changes it if required
    void updateRenderPass();
    void updateAttachmentLayoutsAfterRenderPass();
    void resetRenderPass();

    // Contains objects that are created per frame and need to be deleted after the frame is rendered
    class FrameData {
    public:
        std::vector<VkDescriptorSet> uniformDescriptorSets;
        std::vector<VkDescriptorSet> textureDescriptorSets;
        std::vector<VkDescriptorSet> storageDescriptorSets;
        VkDescriptorPool _descriptorPool;
        std::vector<std::shared_ptr<vks::Buffer>> _buffers;
        std::vector<VkRenderPass> _renderPasses;

        std::shared_ptr<vks::Buffer> _objectBuffer;
        std::shared_ptr<vks::Buffer> _cameraBuffer;
        std::shared_ptr<vks::Buffer> _drawCallInfoBuffer;

        std::shared_ptr<vks::Buffer> _glUniformBuffer; // Contains data from glUniform... calls
        std::vector<uint8_t> _glUniformData;
        std::unordered_map<int, size_t> _glUniformOffsetMap;
        size_t _glUniformBufferPosition {0}; // Position where data from next glUniform... call is placed

        BufferView _cameraCorrectionBuffer { gpu::BufferView(std::make_shared<gpu::Buffer>(gpu::Buffer::UniformBuffer, sizeof(CameraCorrection), nullptr )) };
        BufferView _cameraCorrectionBufferIdentity { gpu::BufferView(std::make_shared<gpu::Buffer>(gpu::Buffer::UniformBuffer, sizeof(CameraCorrection), nullptr )) };

        void addGlUniform(size_t size, const void *data, size_t commandIndex);

        FrameData(VKBackend *backend);
        FrameData() = delete;
        ~FrameData();
        // Executed after the frame was rendered so that it can be reused
        void cleanup(); // VKTODO
    private:
        // Creates descriptor pool for current frame
        void createDescriptorPool();
        VKBackend *_backend;
    };

private:
    void draw(VkPrimitiveTopology mode, uint32 numVertices, uint32 startVertex);
    void renderPassTransfer(const Batch& batch);
    void renderPassDraw(const Batch& batch);
    void transferGlUniforms();
    void transferTransformState(const Batch& batch);
    void updateInput();
    void updateTransform(const Batch& batch);
    void updatePipeline();

    vk::VKFramebuffer* syncGPUObject(const Framebuffer& framebuffer);
    VKBuffer* syncGPUObject(const Buffer& buffer);
    VKTexture* syncGPUObject(const Texture& texture);
    VKQuery* syncGPUObject(const Query& query);

    void blitToFramebuffer(VKAttachmentTexture &input, const Vec4i& srcViewport, VKAttachmentTexture &output, const Vec4i& dstViewport);

public:
    VKBackend();
    ~VKBackend();
    void shutdown() override;
    vks::Context& getContext() { return _context; }
    void syncProgram(const gpu::ShaderPointer& program) override {}
    void syncCache() override {}
    void recycle() const override {}
    void setCameraCorrection(const Mat4& correction, const Mat4& prevRenderView, bool primary, bool reset = false) override;
    uint32_t getTextureID(const TexturePointer&) override { return 0; }
    void executeFrame(const FramePointer& frame) final;
    bool isTextureManagementSparseEnabled() const override;
    bool supportedTextureFormat(const gpu::Element& format) const override;
    const std::string& getVersion() const override;
    void downloadFramebuffer(const FramebufferPointer& srcFramebuffer, const Vec4i& region, QImage& destImage) final;
    void setDrawCommandBuffer(VkCommandBuffer commandBuffer);
    size_t getNumInputBuffers() const { return _input._invalidBuffers.size(); }
    VkDescriptorImageInfo getDefaultTextureDescriptorInfo() ;
    // Used by GPU frame player to move camera around
    void enableContextViewCorrectionForFramePlayer() { _transform._viewCorrectionEnabledForFramePlayer = true; };
    void setIsFramePlayer(bool isFramePlayer) { _isFramePlayer = isFramePlayer; };

    static gpu::Primitive getPrimitiveTopologyFromCommand(Batch::Command command, const Batch& batch, size_t paramOffset);

    int getRealUniformLocation(int location);

    virtual void store_glUniform1f(const Batch& batch, size_t paramOffset) final;
    virtual void store_glUniform2f(const Batch& batch, size_t paramOffset) final;
    virtual void store_glUniform3f(const Batch& batch, size_t paramOffset) final;
    virtual void store_glUniform4f(const Batch& batch, size_t paramOffset) final;
    virtual void store_glUniform3fv(const Batch& batch, size_t paramOffset) final;
    virtual void store_glUniform4fv(const Batch& batch, size_t paramOffset) final;
    virtual void store_glUniform4iv(const Batch& batch, size_t paramOffset) final;
    virtual void store_glUniformMatrix3fv(const Batch& batch, size_t paramOffset) final;
    virtual void store_glUniformMatrix4fv(const Batch& batch, size_t paramOffset) final;

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
    virtual void do_generateTextureMipsWithPipeline(const Batch& batch, size_t paramOffset) final;

    virtual void do_glUniform1f(const Batch& batch, size_t paramOffset) final;
    virtual void do_glUniform2f(const Batch& batch, size_t paramOffset) final;
    virtual void do_glUniform3f(const Batch& batch, size_t paramOffset) final;
    virtual void do_glUniform4f(const Batch& batch, size_t paramOffset) final;
    virtual void do_glUniform3fv(const Batch& batch, size_t paramOffset) final;
    virtual void do_glUniform4fv(const Batch& batch, size_t paramOffset) final;
    virtual void do_glUniform4iv(const Batch& batch, size_t paramOffset) final;
    virtual void do_glUniformMatrix3fv(const Batch& batch, size_t paramOffset) final;
    virtual void do_glUniformMatrix4fv(const Batch& batch, size_t paramOffset) final;

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
    virtual void do_setResourceTextureTable(const Batch& batch, size_t paramOffset) final;
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
    virtual void do_setContextMirrorViewCorrection(const Batch& batch, size_t paramOffset) final;

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
    // Initializes parts of the backend that can't be initialized in the constuctor.
    void initBeforeFirstFrame();

    void initTransform();
    void initDefaultTexture();

    // Gets a frame data object from the pool and sets _currentFrame to point to it.
    // Needs to be called before frame command buffers creation starts
    void acquireFrameData();
    // Called after frame command buffers are generated.
    // Pointer needs to be kept until rendering finished.
    void releaseFrameData() { _currentFrame.reset(); };
public:
    // Called after frame finishes rendering. Cleans up and puts frame data object back to the pool.
    void recycleFrame();
    void waitForGPU();

    void releaseExternalTexture(GLuint id, const Texture::ExternalRecycler& recycler);

    // VKTODO: quick hack
    VKFramebuffer *_outputTexture{ nullptr };
protected:
    void transitionInputImageLayouts(); // This can be called only form `updateRenderPass`
    void transitionAttachmentImageLayouts(gpu::Framebuffer &framebuffer); // This can be called only form `updateRenderPass`

    // These are filled by syncGPUObject() calls, and are needed to track backend objects so that they can be destroyed before
    // destroying backend.
    // Access to these objects happens only from the backend thread. Destructors don't access them directly, but through a recycler.
    std::unordered_set<VKFramebuffer*> _framebuffers;
    std::unordered_set<VKBuffer*> _buffers;
    std::unordered_set<VKTexture*> _textures;
    std::unordered_set<VKQuery*> _queries;
    void perFrameCleanup();
    // Called by the destructor
    void beforeShutdownCleanup();
    void dumpVmaMemoryStats();

    std::mutex _externalTexturesMutex;
    std::list<std::pair<GLuint, Texture::ExternalRecycler>> _externalTexturesTrash;

    // Logical device, application's view of the physical device (GPU)
    // VkPipeline cache object
    VkPipelineCache _pipelineCache;

    vks::Context& _context{ vks::Context::get() };
    //VkQueue _graphicsQueue; //TODO: initialize from device
    //VkQueue _transferQueue; //TODO: initialize from device
    std::shared_ptr<gpu::Texture> _defaultTexture;
    VKTexture* _defaultTextureVk{ nullptr };
    VkDescriptorImageInfo _defaultTextureImageInfo{};
    std::shared_ptr<gpu::Texture> _defaultSkyboxTexture;
    VKTexture* _defaultSkyboxTextureVk{ nullptr };
    VkDescriptorImageInfo _defaultSkyboxTextureImageInfo{};
    friend class VKBuffer;
    friend class VKFramebuffer;
    VkCommandBuffer _currentCommandBuffer;
    size_t _commandIndex{ 0 };
    int _currentDraw{ -1 };
    bool _inRenderTransferPass{ false };
    // VKTODO: maybe move to _transform?
    Vec4i _currentScissorRect{ 0 };
    // This allows for one frame to be renderer while commands are generated for next one already
    std::vector<std::shared_ptr<FrameData>> _framePool;
    std::deque<std::shared_ptr<FrameData>> _framesToReuse;
    // Frame for which commands are currently generated
    std::shared_ptr<FrameData> _currentFrame;
    // Frame for which command buffer is already generated and it's currently being rendered.
    std::shared_ptr<FrameData> _currentlyRenderedFrame;
    size_t _frameCounter{ 0 };

    // Safety check to ensure that shutdown was completed before destruction.
    std::atomic<bool> isBackendShutdownComplete{ false };

    typedef void (VKBackend::*CommandCall)(const Batch&, size_t);
    static std::array<VKBackend::CommandCall, Batch::NUM_COMMANDS> _commandCalls;
    static const size_t INVALID_OFFSET = (size_t)-1;
    static size_t UNIFORM_BUFFER_OFFSET_ALIGNMENT;
    bool _isFramePlayer {false};
    bool _isInitialized {false};
};

}}  // namespace gpu::vulkan

#endif
