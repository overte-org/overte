//
//  Context.h
//  interface/src/gpu
//
//  Created by Sam Gateau on 10/27/2014.
//  Copyright 2014 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_Context_h
#define hifi_gpu_Context_h

#include <assert.h>
#include <mutex>
#include <queue>

#include "Texture.h"
#include "Pipeline.h"
#include "Frame.h"
#include "PointerStorage.h"
#include "Backend.h"

namespace gpu {
/**
  * DOCTODO
  */
class Context {
public:
    using Size = Resource::Size;
    typedef BackendPointer (*CreateBackend)();

    /**
     * @brief Creates graphics-API specific Backend object.
     *
     * This one call must happen before any context is created or used (Shader::MakeProgram) in order
     * to set up the Backend and any singleton data needed.
     * @tparam T Type of the backend to create, specific to given graphics API.
     */
    template <class T>
    static void init() {
        std::call_once(_initialized, [] {
            _createBackendCallback = T::createBackend;
            T::init();
        });
    }

    Context();
    ~Context();

    /**
     * @bried Shuts down and deletes graphics backend.
     */
    void shutdown();

    /**
     * @return String describing which graphics API-specific backend is currently in use.
     */
    const std::string& getBackendVersion() const;

    /**
     * @brief Creates and sets up new Frame object to which batches of renderer commands will be written.
     *
     * @param renderView The view matrix used for rendering the frame, only applicable for HMDs.
     * @param renderPose HMD head pose.
     */
    void beginFrame(const glm::mat4& renderView = glm::mat4(), const glm::mat4& renderPose = glm::mat4());

    /**
     * @brief Appends a batch at the end of the current frame.
     * @param batch Batch to append.
     */
    void appendFrameBatch(const BatchPointer& batch);

    /**
     * @brief Finalizes the currently recorded frame.
     *
     * After calling this function batches cannot be added to the frame anymore.
     * @return Finished frame. Later frame gets submitted to the display plugin to be rendered on Present thread.
     */
    FramePointer endFrame();

    /**
     * @brief Acquires batch from a batch pool and sets its name.
     *
     * If there are no free batches, it creates new one.
     * @param name Name to set fot the batch.
     * @return Shared pointer to the batch.
     */
    static BatchPointer acquireBatch(const char* name = nullptr);

    /**
     * @brief Clears the batch and adds it batch to the pool.
     *
     * @param batch Batch to be released.
     */
    static void releaseBatch(Batch* batch);

    /**
     * Handle any pending operations to clean up (recycle / deallocate) resources no longer in use.
     * MUST only be called on the Present thread, which does rendering.
     */
    void recycle() const;

    /**
     * Execute a batch immediately, rather than as part of a frame.
     * MUST only be called on the Present thread, which does rendering.
     *
     * @param batch
     */
    void executeBatch(Batch& batch) const;

    /**
     * Execute a batch immediately, rather than as part of a frame.
     * MUST only be called on the Present thread, which does rendering.
     *
     * @param name Name of the batch.
     * @param lambda Function that adds commands to batch that will be executed.
     */
    void executeBatch(const char* name, std::function<void(Batch&)> lambda) const;

    /**
     * Executes a frame, applying any updates contained in the frame batches to the rendering
     * thread shadow copies.  Either executeFrame or consumeFrameUpdates MUST be called on every frame
     * generated, IN THE ORDER they were generated.
     *
     * @param frame Frame to be executed.
     */
    void executeFrame(const FramePointer& frame) const;

    /**
     * Consuming a frame applies any updates queued from the recording thread and applies them to the
     * shadow copy used by the rendering thread.
     *
     * EVERY frame generated MUST be consumed, regardless of whether the frame is actually executed,
     * or the buffer shadow copies can become unsynced from the recording thread copies.
     *
     * Consuming a frame is idempotent, as the frame encapsulates the updates and clears them out as
     * it applies them, so calling it more than once on a given frame will have no effect after the
     * first time
     *
     * This is automatically called by executeFrame, so you only need to call it if you
     * have frames you aren't going to otherwise execute, for instance when a display plugin is
     * being disabled, or in the null display plugin where no rendering actually occurs
     * MUST only be called on the Present thread, which does rendering.
     *
     * @param frame Frame for which updates are to be consumed.
     */
    void consumeFrameUpdates(const FramePointer& frame) const;

    /**
     * @return Pointer to the Backend object of type specific to currently used graphics API.
     */
    const BackendPointer& getBackend() const { return _backend; }

    /**
     * @brief Enables stereo for the context.
     *
     * Called in `render_performFrame`, where frame rendering commands are recorded.
     * @param enable True if stereo rendering should be enabled.
     */
    void enableStereo(bool enable = true);

    /**
     * @return `true` if stereo rendering was enabled with `enableStereo`.
     */
    bool isStereo();

    /**
     * @brief Sets projection matrices for each eye.
     *
     * Used in `render_performFrame` before recording the frame rendering commands,
     * @param eyeProjections
     */
    void setStereoProjections(const mat4 eyeProjections[2]);

    /**
     * @brief Sets projection matrices for each eye.
     *
     * DOCTODO: What coordinate system does this use?
     * Used in `render_performFrame` before recording the frame rendering commands,
     * @param eyeViews
     */
    void setStereoViews(const mat4 eyeViews[2]);

    /**
     * @brief Writes current stereo projection matrices to a given `mat4` array.
     * @param eyeProjections Pointer to the array of matrices. Array needs to have 2 matrices.
     */
    void getStereoProjections(mat4* eyeProjections) const;

    /**
     * @brief Writes current stereo view matrices to a given `mat4` array.
     * @param eyeViews Pointer to the array of matrices. Array needs to have 2 matrices.
     */
    void getStereoViews(mat4* eyeViews) const;

    /**
     * @brief Writes a region of a given framebuffer to a QImage.
     *
     * Downloading the Framebuffer is a synchronous action that is not efficient.
     * It s here for convenience to easily capture a snapshot
     * @param srcFramebuffer Framebuffer from which to capture the contents.
     * @param region Area to capture. First two coordinates are position of the corner in pixels, and second two are width and height.
     * @param destImage QImage object. Needs to be a correct size and format.
     */
    void downloadFramebuffer(const FramebufferPointer& srcFramebuffer, const Vec4i& region, QImage& destImage);

    // Reporting stats of the context
    /**
     * @brief Resets render statistics such as draw call count.
     */
    void resetStats() const;

    /**
    * @brief Retrieves current render statistics such as draw call count.
     * @param stats Object to which current statistics will be written.
     */
    void getStats(ContextStats& stats) const;

    /**
    * @brief Retrieves most recent frame render statistics such as draw call count.
     *
     * Same as above but updated at every end of a frame
     * @param stats Object to which current statistics will be written.
     */
    void getFrameStats(ContextStats& stats) const;

    /**
     * @brief Creates a custom pipeline for generating mipmaps.
     *
     * Used for SSAO.
     * @param pixelShader Pixel shader to use for mipmap generation.
     * @return Shared pointer to the pipeline.
     */
    static PipelinePointer createMipGenerationPipeline(const ShaderPointer& pixelShader);

    /**
     * @return GPU frame time in milliseconds.
     */
    double getFrameTimerGPUAverage() const;

    /**
     * @return Batch frame time in milliseconds.
     */
    double getFrameTimerBatchAverage() const;

    /**
     * @return Free GPU memory in bytes.
     */
    static Size getFreeGPUMemSize();

    /**
     * @return Used GPU memory in bytes.
     */
    static Size getUsedGPUMemSize();

    /**
     * @return Number of buffers on the GPU.
     */
    static uint32_t getBufferGPUCount();

    /**
     * @return Size of GPU memory used by buffers in bytes.
     */
    static Size getBufferGPUMemSize();

    /**
     * @return Number of textures on the GPU.
     */
    static uint32_t getTextureGPUCount();

    /**
     * @return Number of textures on the GPU which usage is `TextureUsageType::STRICT_RESOURCE`.
     */
    static uint32_t getTextureResidentGPUCount();

    /**
     * @return Number of framebuffers on the GPU.
     */
    static uint32_t getTextureFramebufferGPUCount();

    /**
     * @return Number of resource textures on the GPU. These are regular textures used for things such as models or skyboxes.
     */
    static uint32_t getTextureResourceGPUCount();

    /**
     * @return Number of external textures, used for things such as QML and webentities.
     */
    static uint32_t getTextureExternalGPUCount();

    /**
     * @return Amount of GPU memory used by textures, in bytes.
     */
    static Size getTextureGPUMemSize();

    /**
     * @return Amount of GPU memory in bytes used by textures on the GPU which usage is `TextureUsageType::STRICT_RESOURCE`.
     */
    static Size getTextureResidentGPUMemSize();

    /**
     * @return Amount of GPU memory used by framebuffers, in bytes.
     */
    static Size getTextureFramebufferGPUMemSize();

    /**
     * @return Amount of GPU memory used by resource textures on the GPU. These are regular textures used for things such as models or skyboxes.
     */
    static Size getTextureResourceGPUMemSize();

    /**
     * @return Amount of GPU memory used by external textures, used for things such as QML and webentities.
     */
    static Size getTextureExternalGPUMemSize();

    /**
     * TODO: Currently not used.
     * @return
     */
    static uint32_t getTexturePendingGPUTransferCount();

    /**
     * @return Size in bytes of the textures that are currently transferred.
     */
    static Size getTexturePendingGPUTransferMemSize();

    /**
     * @return Size in bytes of currently populated mips variable allocation textures.
     */
    static Size getTextureResourcePopulatedGPUMemSize();

    /**
     * @return Total size of textures that are in use currently, including mips that are not currently on GPU.
     */
    static Size getTextureResourceIdealGPUMemSize();

    /**
     * Structure containing a vector of shader pointers that are to be synchronized and a callback that will be called when synchronization is finished.
     * On OpenGL backend synchronization means loading and compiling the shader.
     */
    struct ProgramsToSync {
        /**
         * @param programs Vector of shaders to be loaded and compiled.
         * @param callback Function to call when work is done.
         * @param rate Limits how many shaders can be processed in a single frame.
         */
        ProgramsToSync(const std::vector<gpu::ShaderPointer>& programs, std::function<void()> callback, size_t rate) :
            programs(programs), callback(callback), rate(rate) {}

        std::vector<gpu::ShaderPointer> programs;
        std::function<void()> callback;
        size_t rate;
    };

    /**
     * @brief Schedule shaders for loading.
     *
     * @param programIDs IDs representing shaders to be synchronized. Can be found in `ShaderEnums.h` file, which is generated at build time.
     * @param callback Function to call when work is done.
     * @param rate Limits how many shaders can be processed in a single frame.
     */
    void pushProgramsToSync(const std::vector<uint32_t>& programIDs, std::function<void()> callback, size_t rate = 0);

    /**
     * @brief Schedule shaders for loading.
     *
     * @param programs Pointers to shaders to be loaded and compiled.
     * @param callback Function to call when work is done.
     * @param rate Limits how many shaders can be processed in a single frame.
     */
    void pushProgramsToSync(const std::vector<gpu::ShaderPointer>& programs, std::function<void()> callback, size_t rate = 0);

    /**
     * @brief Loads and compiles shaders.
     *
     * Called once per frame.
     */
    void processProgramsToSync();

protected:
    /**
     * Copy constructor, not used.
     *
     * @param context Not used.
     */
    Context(const Context& context);

    /**
     * Instance of Backend class specific to currently selected graphics APT, for example Vulkan or OpenGL.
     */
    std::shared_ptr<Backend> _backend;

    /**
     *
     */
    bool _frameActive{ false };

    /**
     * Frame currently being recorded.
     * A new Frame object is placed here by `beginFrame` and the variable is reset by `endFrame`.
     */
    FramePointer _currentFrame;

    /**
     * Time it takes for the frame to execute.
     */
    RangeTimerPointer _frameRangeTimer;

    /**
     * Stereo state is configured before frame recording starts.
     */
    StereoState _stereo;

    /**
     * Mutex protecting `_programsToSyncQueue`, must be locked every time it's accessed.
     */
    std::mutex _programsToSyncMutex;

    /**
     * Queue for loading and compiling shaders. It helps avoid stutter later during the game.
     */
    std::queue<ProgramsToSync> _programsToSyncQueue;

    /**
     * Shaders that were already loaded/compiled.
     */
    gpu::Shaders _syncedPrograms;

    /**
     * Next shader that will be loaded/compiled.
     */
    size_t _nextProgramToSyncIndex { 0 };

    /**
     * Sampled at the end of every frame, the stats of all the counters.
     */
    mutable ContextStats _frameStats;

    /**
     * API-specific call that creates rendering backend.
     */
    static CreateBackend _createBackendCallback;

    /**
     * Ensures that initialization inside Context::init() is called only once.
     */
    static std::once_flag _initialized;

    /**
     * @brief Deletes all batches in the batch pool.
     *
     * Called from the Context destructor.
     * TODO: Should probably move this functionality to Batch. (this comment was here since High Fidelity days, I'm not
     *  sure if it still makes sense)
     */
    static void clearBatches();

    /**
     * Mutex for accessing batch pool.
     */
    static std::mutex _batchPoolMutex;

    /**
     * Pool of batches that can be acquired by renderer and reused for recording.
     */
    static std::list<Batch*> _batchPool;

    friend class Shader;
    friend class Backend;
};
typedef std::shared_ptr<Context> ContextPointer;

/**
 * Record commands as a batch and append the batch to the current frame.
 *
 * @param name Name of the batch.
 * @param context gpu::Context instance.
 * @param f Function that contains function calls that will record batch commands.
 */
void doInBatch(const char* name, const std::shared_ptr<gpu::Context>& context, const std::function<void(Batch& batch)>& f);

};  // namespace gpu

#endif
