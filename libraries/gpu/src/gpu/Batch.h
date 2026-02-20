//
//  Batch.h
//  interface/src/gpu
//
//  Created by Sam Gateau on 10/14/2014.
//  Copyright 2014 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_Batch_h
#define hifi_gpu_Batch_h

#include <vector>
#include <mutex>
#include <functional>
#include <glm/gtc/type_ptr.hpp>

#include <shared/NsightHelpers.h>

#include "Framebuffer.h"
#include "Pipeline.h"
#include "Query.h"
#include "Stream.h"
#include "Texture.h"
#include "Transform.h"
#include "ShaderConstants.h"

class QDebug;
#define BATCH_PREALLOCATE_MIN 128
namespace gpu {

// The named batch data provides a mechanism for accumulating data into buffers over the course
// of many independent calls.  For instance, two objects in the scene might both want to render
// a simple box, but are otherwise unaware of each other.  The common code that they call to render
// the box can create buffers to store the rendering parameters for each box and register a function
// that will be called with the accumulated buffer data when the batch commands are finally
// executed against the backend


class Batch {
public:
    typedef Stream::Slot Slot;

    enum {
        // This is tied to RenderMirrorTask::MAX_MIRROR_DEPTH and RenderMirrorTask::MAX_MIRRORS_PER_LEVEL
        // We have 1 view at mirror depth 0, 3 more at mirror depth 1, 9 more at mirror depth 2, and 27 more at mirror depth 3
        // For each view, we have one slot for the background and one for the primary view, and that's all repeated for the secondary camera
        // So this is 2 slots/view/camera * 2 cameras * (1 + 3 + 9 + 27) views
        MAX_TRANSFORM_SAVE_SLOT_COUNT = 160
    };

    /**
     * A structure passed to shaders. Contains index of an object in transform objects buffer for this draw call.
     */
    class DrawCallInfo {
    public:
        using Index = uint16_t;

        /**
         * @brief Create DrawCallInfo object with no user data.
         * @param idx Index of the transform object for the given draw call.
         */
        DrawCallInfo(Index idx) : index(idx) {}

        /**
         * @brief Create DrawCallInfo object with user data.
         * It's used when drawing meshes. First bit of user data enables blendshapes and secodn on enables skinning.
         * @param idx Index of the transform object for the given draw call.
         * @param user User data. Currently used only for meshes.
         */
        DrawCallInfo(Index idx, Index user) : index(idx), user(user) {}

        Index index { 0 };
        uint16_t user { 0 };

    };
    // Make sure DrawCallInfo has no extra padding
    static_assert(sizeof(DrawCallInfo) == 4, "DrawCallInfo size is incorrect.");

    using DrawCallInfoBuffer = std::vector<DrawCallInfo>;

    /**
     * Used for instancing, currently for basic shapes with simple single color shading.
     */
    struct NamedBatchData {
        using BufferPointers = std::vector<BufferPointer>;
        using Function = std::function<void(gpu::Batch&, NamedBatchData&)>;

        /**
         * Contains buffers needed for a given instanced draw call.
         * Currently, it can contain color data on INSTANCE_COLOR_BUFFER index, and fade data on INSTANCE_FADE1_BUFFER...INSTANCE_FADE7_BUFFER.
         * These also have per-instance stride.
         */
        BufferPointers buffers;

        /**
         * Function that adds batch commands necessary to draw instanced objects from this NamedBatchData.
         * It's called after all the objects are collected.
         */
        Function function;

        /**
         * The whole named batch is drawn as a single drawInstanced call.
         * drawCallInfos buffer is bound with per instance stride, providing indexes of transformations for each drawn object.
         */
        DrawCallInfoBuffer drawCallInfos;

        size_t count() const { return drawCallInfos.size(); }

        /**
         * @brief Adds commands for drawInstanced call for this set of instanced objects.
         * @param batch Batch to which commands will be added to.
         */
        void process(Batch& batch) {
            if (function) {
                function(batch, *this);
            }
        }
    };

    using NamedBatchDataMap = std::map<std::string, NamedBatchData>;

    // Contains DrawCallInfo structures for non-instanced draw calls.
    DrawCallInfoBuffer _drawCallInfos;

    // Used for reserving proper size for the _drawCallInfos based on previous frames.
    static size_t _drawCallInfosMax;

    /** `_currentNamedCall` is set during batch creation with `startNamedCall` and `stopNamedCall` when adding commands
     * for drawing instanced shapes. After the draw call command is stored `_currentNamedCall` is set to `nullptr` again.
     * When the rendering backend iterates through commands, the corresponding `do_startNamedCall` and `do_stopNamedCall`
     * set ``_currentNamedCall` in the same way.
     * During batch generation it's used for getting the correct draw call info buffer.
     * On the renderer backend side it's used for setting up transforms.
     */
    mutable std::string _currentNamedCall;

    /**
     * @brief Returns current DrawCallInfoBuffer.
     * DrawCallInfoBuffer stores index of the transform and 16-bit user data field for each draw call.
     * When a named draw call is progress (used for instances), getDrawCallInfoBuffer returns reference to DrawCallInfoBuffer
     * belonging to that particular named call. Otherwise it returns reference to the main DrawCallInfoBuffer.
     * @return Constant reference to DrawCallInfoBuffer.
     */
    const DrawCallInfoBuffer& getDrawCallInfoBuffer() const;

    /**
     * @brief Returns current DrawCallInfoBuffer.
     * DrawCallInfoBuffer stores index of the transform and 16-bit user data field for each draw call.
     * When a named draw call is progress (used for instances), getDrawCallInfoBuffer returns reference to DrawCallInfoBuffer
     * belonging to that particular named call. Otherwise, it returns reference to the main DrawCallInfoBuffer.
     * @return Reference to DrawCallInfoBuffer.
     */
    DrawCallInfoBuffer& getDrawCallInfoBuffer();

    /**
     * @brief Adds a new TransformObject to batch if needed and stores DrawCallInfo with its index for the draw call.
     * It's called from inside the functions that add a draw call command to the batch.
     * Returns immediately without doing anything in case of named calls (used for instancing shapes).
     */
    void captureDrawCallInfo();

    /**
     * @brief Adds a new TransformObject to batch if needed and stores DrawCallInfo with its index for the draw call.
     * Used when adding commands for drawing instances of shapes.
     * @param name Name of a particular instance group.
     */
    void captureNamedDrawCallInfo(std::string name);

    /**
     * @param name Name of the batch. Used by the renderer backend for labeling draw calls, name will show in the captured frame in Renderdoc.
     */
    Batch(const std::string& name = "");
    // Disallow copy construction and assignment of batches
    Batch(const Batch& batch) = delete;
    Batch& operator=(const Batch& batch) = delete;
    ~Batch();

    /**
     * @param name New name for this batch
     */
    void setName(const std::string& name);

    /**
     * @return Name of this batch.
     */
    const std::string& getName() const { return _name; }

    /**
     * @brief Clears all the data from this batch.
     * After batches are rendered, they are not deleted, but they are cleared and reused instead.
     * This allows storing maximum numbers of commands and various objects from previous frames for this kind of batch
     * and reserving aproppriate amount of memory for them to avoid performance loss due to reallocation.
     */
    void clear();

    /**
     * @brief Disables or enables stereo for the whole batch.
     * Batches may need to override the context level stereo settings
     * if they're performing framebuffer copy operations, like the
     * deferred lighting resolution mechanism.
     * @param enable
     */
    void enableStereo(bool enable = true);

    /**
     * @brief Check if stereo is enabled for this batch.
     * Used by the rendering backend to check if batch overrides stereo setting from the context.
     * @return Returns `true` if stereo is enabled, `false` if not.
     */
    bool isStereoEnabled() const;

    /**
     * @brief Enables skybox rendering mode.
     * Stereo batches will pre-translate the view matrix, but this isn't
     * appropriate for skybox or other things intended to be drawn at
     * infinite distance, so provide a mechanism to render in stereo
     * without the pre-translation of the view.
     * Used by skybox and splash frame.
     * @param enable Should skybox mode be enabled.
     */
    void enableSkybox(bool enable = true);

    /**
     * @return `true` if skybox rendering mode is enabled.
     */
    bool isSkyboxEnabled() const;

    /**
     * @brief Sets Drawcall Uniform value.
     * One 16bit word uniform value is available during the drawcall.
     * Its value must be set before each drawcall, because it is reset to the reset value between each drawcall.
     * Drawcall uniform is used to tell shader if skinning and/or blendshapes are enabled.
     * @param uniform Value to ser Drawcall Unform to.
     */
    void setDrawcallUniform(uint16 uniform);


    /**
     * @brief Sets the default value of the Drawcall Uniform.
     *
     * The reset value is 0 by default and can be changed as a batch state with this call.
     * Drawcall uniform is used to tell shader if skinning and/or blendshapes are enabled.
     *
     * @param resetUniform Default Drawcall Uniform value.
     */
    void setDrawcallUniformReset(uint16 resetUniform);

    // Drawcalls

    /**
     * @brief Adds a command to the batch to perform a draw call using a mesh without index buffer.
     * @param primitiveType Type of primitives to draw (for example TRIANGLES or TRIANGLE_STRIP).
     * @param numVertices Number of vertices to draw.
     * @param startVertex Position of the first vertex in the vertex buffer.
     */
    void draw(Primitive primitiveType, uint32 numVertices, uint32 startVertex = 0);

    /**
     * @brief Adds a command to the batch to perform a draw call using a mesh with index buffer.
     * @param primitiveType Type of primitives to draw (for example TRIANGLES or TRIANGLE_STRIP).
     * @param numIndices Number of indices to draw.
     * @param startIndex Position of the first index to draw in the index buffer.
     */
    void drawIndexed(Primitive primitiveType, uint32 numIndices, uint32 startIndex = 0);

    /**
     * @brief Adds a command to the batch to perform an instanced draw call using mesh without index buffer.
     * @param numInstances Number of the instances to draw.
     * @param primitiveType Type of primitives to draw (for example TRIANGLES or TRIANGLE_STRIP).
     * @param numVertices Number of vertices for each instance.
     * @param startVertex Position of the first vertex in vertex buffer.
     * @param startInstance Index of the first instance.
     */
    void drawInstanced(uint32 numInstances, Primitive primitiveType, uint32 numVertices, uint32 startVertex = 0, uint32 startInstance = 0);

    /**
     * @brief Adds a command to the batch to perform an instanced draw call using mesh with index buffer.
     * @param numInstances Number of the instances to draw.
     * @param primitiveType Type of primitives to draw (for example TRIANGLES or TRIANGLE_STRIP).
     * @param numIndices Number of indices for each instance.
     * @param startIndex Position of the first index in the index buffer.
     * @param startInstance Index of the first instance.
     */
    void drawIndexedInstanced(uint32 numInstances, Primitive primitiveType, uint32 numIndices, uint32 startIndex = 0, uint32 startInstance = 0);

    /**
     * @brief DOCTODO
     * Currently not used.
     * @param numCommands
     * @param primitiveType Type of primitives to draw (for example TRIANGLES or TRIANGLE_STRIP).
     */
    void multiDrawIndirect(uint32 numCommands, Primitive primitiveType);

    /**
     * @brief DOCTODO
     * Currently not used.
     * @param numCommands
     * @param primitiveType Type of primitives to draw (for example TRIANGLES or TRIANGLE_STRIP).
     */
    void multiDrawIndexedIndirect(uint32 numCommands, Primitive primitiveType);

    /**
     * @brief Sets function used to generate batch commands to draw objects of a given named call.
     * Named calls are used for object instancing, currently for basic shapes with simple shading.
     * During batch generation all objects that can be drawn together are gathered and batch command
     * to draw them using instanced draw call is generated before the batch is run.
     * @param instanceName Describes objects that can be instanced together in an  exhaustive way.
     * For shapes instance name contains drawing mode, shape name and hash of shape pipeline pointer.
     * @param function Function that will be used to generate batch commands for this group of instanced objects.
     */
    void setupNamedCalls(const std::string& instanceName, NamedBatchData::Function function);

    /**
     * @brief Get a reference to a buffer for a given instancing group.
     * If the buffer does not exist yet, it gets created.
     * The buffers have per-instance stride, which means there will be separate set of data provided to each instance.
     * @param instanceName Name of the group of instanced objects.
     * @param index Index of the requested buffer.
     * @return Constant reference to a shared pointer to a requested buffer.
     */
    const BufferPointer& getNamedBuffer(const std::string& instanceName, uint8_t index = 0);

    // DOCTODO: what to do with these comments?
    // Input Stage
    // InputFormat
    // InputBuffers
    // IndexBuffer

    /**
     * @brief Adds a command to set input format to the batch.
     *
     * @param format Input format that the following draw calls will use.
     */
    void setInputFormat(const Stream::FormatPointer& format);

    /**
     * @brief Adds a command to set input buffer for the following draw calls.
     *
     * @param channel Input slot. For possible values check `gpu::Stream::InputSlot`.
     * @param buffer Pointer to the buffer.
     * @param offset Offset in bytes from the start of the buffer object.
     * @param stride Stride (increment between consecutive entries) in bytes.
     */
    void setInputBuffer(Slot channel, const BufferPointer& buffer, Offset offset, Offset stride);

    /**
     * @brief Adds a command to set input buffer for the following draw calls.
     *
     * @param channel Input slot. For possible values check `gpu::Stream::InputSlot`.
     * @param buffer `BufferView` which points to a buffer and contains information about offset, stride and element type.
     */
    void setInputBuffer(Slot channel, const BufferView& buffer); // not a command, just a shortcut from a BufferView

    /**
     * @brief Adds commands to set several input buffers for the following draw calls.
     *
     * @param startChannel BufferStream will be applied starting from this slot.
     * @param stream Reference to a `BufferStream` object, containing a set of buffers with corresponding offsets and strides.
     */
    void setInputStream(Slot startChannel, const BufferStream& stream); // not a command, just unroll into a loop of setInputBuffer

    /**
     * @brief Adds a command to set index buffer for the following draw calls.
     *
     * @param type Element type for the index buffer.
     * @param buffer Buffer that will be set as index buffer.
     * @param offset Offset at which index data starts.
     */
    void setIndexBuffer(Type type, const BufferPointer& buffer, Offset offset);

    /**
     * @brief Adds a command to set index buffer for the following draw calls.
     *
     * @param buffer `BufferView` object containing  stride, offset and shared pointer to the buffer.
     */
    void setIndexBuffer(const BufferView& buffer); // not a command, just a shortcut from a BufferView

    /**
     * @brief Adds a command to set the indirect buffer.
     *
     * Indirect buffer is used by the multiDrawXXXIndirect calls.
     * The indirect buffer contains the command descriptions to execute multiple drawcalls in a single call.
     * Currently not used.
     *
     * @param buffer Shared pointer of the indirect buffer to set.
     * @param offset Offset at which indirect buffer data starts.
     * @param stride Stride for indirect data.
     */
    void setIndirectBuffer(const BufferPointer& buffer, Offset offset = 0, Offset stride = 0);

    // multi command description for multiDrawIndexedIndirect
    class DrawIndirectCommand {
    public:
        uint  _count { 0 };
        uint  _instanceCount { 0 };
        uint  _firstIndex { 0 };
        uint  _baseInstance { 0 };
    };

    // multi command description for multiDrawIndexedIndirect
    class DrawIndexedIndirectCommand {
    public:
        uint  _count { 0 };
        uint  _instanceCount { 0 };
        uint  _firstIndex { 0 };
        uint  _baseVertex { 0 };
        uint  _baseInstance { 0 };
    };

    // Transform Stage
    // Vertex position is transformed by ModelTransform from object space to world space
    // Then by the inverse of the ViewTransform from world space to eye space
    // finally projected into the clip space by the projection transform
    // WARNING: ViewTransform transform from eye space to world space, its inverse is composed
    // with the ModelTransform to create the equivalent of the gl ModelViewMatrix
    /**
     * @brief Adds a command to set model transform for the following draw calls.
     * @param model Model transform.
     */
    void setModelTransform(const Transform& model);

    /**
     * @brief Adds a command to set model transform for the following draw calls.
     * @param model Current model transform.
     * @param previousModel Model transform in the previous frame, used for velocity buffer.
     */
    void setModelTransform(const Transform& model, const Transform& previousModel);

    /**
     * @brief Adds a command to set current transform to identity transform.
     */
    void resetViewTransform() { setViewTransform(Transform(), false); }

    /**
     * @brief Adds a command to set the view transform.
     *
     * When `camera` is set to `false` view correction is not applied, even if it's enabled.
     *
     * @param view View transform to set.
     * @param camera Set to `false` for rendering shadows.
     */
    void setViewTransform(const Transform& view, bool camera = true);

    /**
     * @brief Adds a command to set projection transform.
     *
     * Projection transform is set to identity matrix for screen space effects such as bloom.
     *
     * @param proj Projection matrix.
     */
    void setProjectionTransform(const Mat4& proj);

    /**
     * @brief Adds a command to enable od disable projection jitter.
     *
     * Jitter is used for TAA.
     *
     * @param isProjectionEnabled
     */
    void setProjectionJitterEnabled(bool isProjectionEnabled);

    /**
     * @brief Adds a command to set the jitter sequence used for TAA.
     *
     * @param sequence Jitter sequence array with values scaled in pixels.
     * @param count Number of steps in jitter sequence.
     */
    void setProjectionJitterSequence(const Vec2* sequence, size_t count);

    /**
     * @brief Adds a command to set the jitter scale.
     *
     * @param scale
     */
    void setProjectionJitterScale(float scale);

    /**
     * @brief Adds a command to store previous value of projection enabled state and change the current state to a new value.
     *
     * Very simple 1 level stack management of jitter.
     *
     * @param isProjectionEnabled New value to set.
     */
    void pushProjectionJitterEnabled(bool isProjectionEnabled);

    /**
     * @brief Adds a command to restore previous value of projection being enabled or disabled.
     *
     * Very simple 1 level stack management of jitter.
     */
    void popProjectionJitterEnabled();

    /**
     * @brief Adds a command to set the viewport in pixels.
     *
     * Viewport is xy = low left corner in framebuffer, zw = width height of the viewport, expressed in pixels.
     *
     * @param viewport Viewport position and size.
     */
    void setViewportTransform(const Vec4i& viewport);

    /**
     * @brief Sets depth range.
     *
     * Currently not used.
     * On OpenGL backend it uses `glDepthRangef`.
     *
     * @param nearDepth Near depth.
     * @param farDepth Far Depth.
     */
    void setDepthRangeTransform(float nearDepth, float farDepth);

    /**
     * @brief Adds a command to save the current view and projection transform into one of the storage slots.
     *
     * Transform is saved together with the view correction and transform from the previous frame.
     * Look into comment next to `gpu::Batch::MAX_TRANSFORM_SAVE_SLOT_COUNT` to see how number of storage slots is calculated.
     *
     * @param saveSlot Slot to save the transform to. Must be lower than `gpu::Batch::MAX_TRANSFORM_SAVE_SLOT_COUNT`.
     */

    void saveViewProjectionTransform(uint saveSlot);

    /**
     * @brief Adds a command to restore transform from the particular save slot.
     *
     * @param saveSlot Slot to restore the transform from. Must be lower than `gpu::Batch::MAX_TRANSFORM_SAVE_SLOT_COUNT`.
     */
    void setSavedViewProjectionTransform(uint saveSlot);

    /**
     * @brief Adds a command to write the saved view and projection transform from a given slot to a buffer.
     *
     * Used for saving view and projection transform to deferred frame transform buffer.
     *
     * @param saveSlot Slot to restore the transform from. Must be lower than `gpu::Batch::MAX_TRANSFORM_SAVE_SLOT_COUNT`.
     * @param buffer Buffer to which transform should be saved to.
     * @param offset Offset Byte offset from which to start writing.
     */
    void copySavedViewProjectionTransformToBuffer(uint saveSlot, const BufferPointer& buffer, Offset offset);

    // Pipeline Stage
    /**
     * @brief Adds a command to set a pipeline for the following draw calls.
     *
     * @param pipeline Pipeline to set.
     */
    void setPipeline(const PipelinePointer& pipeline);

    /**
     * @brief Adds a command to set blend color.
     *
     * Currently not used.
     * Uses `glBlendColor` internally on the OpenGL backend. Not implemented on the Vulkan backend.
     *
     * @param factor Color (RGBA) to set.
     */
    void setStateBlendFactor(const Vec4& factor);

    /**
     * @brief Adds a command to set the scissor rectangle.
     *
     * The rectangle coordinates are xy for the low left corner of the rectangle and zw for the width and height
     * of the rectangle, expressed in pixels.
     *
     * @param rect Rectangle coordinates.
     */
    void setStateScissorRect(const Vec4i& rect);

    /**
     * @brief Adds a command to set a uniform buffer on a given slot.
     *
     * A `nullptr` can be passed to unset the buffer.
     *
     * @param slot Slot index.
     * @param buffer Buffer that contains uniform data.
     * @param offset Offset from which data start in the buffer.
     * @param size Size ofg the uniform data.
     */
    void setUniformBuffer(uint32 slot, const BufferPointer& buffer, Offset offset, Offset size);

    /**
     * @brief Adds a command to set a uniform buffer on a given slot.
     *
     * @param slot Slot index.
     * @param view `BufferView` object containing size, offset and a shared pointer to the buffer.
     */
    void setUniformBuffer(uint32 slot, const BufferView& view); // not a command, just a shortcut from a BufferView

    /**
     * @brief Adds a command to set a resource buffer on a given slot.
     *
     * Used for providing data such as mesh blendshapes or polyline geometry to the shader.
     * A `nullptr` can be passed to unset the buffer.
     *
     * @param slot Slot index.
     * @param buffer Resource buffer.
     */
    void setResourceBuffer(uint32 slot, const BufferPointer& buffer);

    /**
     * @brief Adds a command to set a texture on a given slot.
     *
     * A `nullptr` can be passed to unset the buffer.
     *
     * @param slot Slot index.
     * @param texture Texture object.
     */
    void setResourceTexture(uint32 slot, const TexturePointer& texture);

    /**
     * @brief Adds a command to set a texture on a given slot using a `TextureView`.
     *
     * @param slot Slot index.
     * @param view `TextureView` object containing subresource index and shared pointer to the texture.
     */
    void setResourceTexture(uint32 slot, const TextureView& view); // not a command, just a shortcut from a TextureView

    /**
     * @brief Adds a command to set several textures at once.
     *
     * @param table Shared pointer to a `TextureTable` object.
     * @param slot First slot to set. It works as an offset for indices in `textureTable`.
     */
    void setResourceTextureTable(const TextureTablePointer& table, uint32 slot = 0);

    /**
     * @brief Adds a command to set a texture with a given index from the framebuffer swap chain.
     *
     * Used for TAA.
     *
     * @param slot Slot index.
     * @param framebuffer Shared pointer to the `FramebufferSwapChain` object.
     * @param swapChainIndex Index of the texture in the swap chain. After the swap chain advances it changes to which framebuffer texture it points.
     * @param renderBufferSlot Index of the render buffer inside framebuffer object that will be used as a texture.
     */
    void setResourceFramebufferSwapChainTexture(uint32 slot, const FramebufferSwapChainPointer& framebuffer, unsigned int swapChainIndex, unsigned int renderBufferSlot = 0U); // not a command, just a shortcut from a TextureView

    // Ouput Stage
    /**
     * @brief Adds a command to set a framebuffer to which following commands will render to.
     *
     * @param framebuffer Shared pointer to the framebuffer object. Can be passed a `nullptr` to unset the framebuffer.
     */
    void setFramebuffer(const FramebufferPointer& framebuffer);

    /**
     * @brief Adds a command to set current framebuffer from a swap chain.
     *
     * Used for TAA.
     *
     * @param framebuffer Shared pointer to a `FramebufferSwapChain` object.
     * @param swapChainIndex Index of the framebuffer in the `FramebufferSwapChain` object. After the swap chain advances it changes to which framebuffer texture it points.
     */
    void setFramebufferSwapChain(const FramebufferSwapChainPointer& framebuffer, unsigned int swapChainIndex);

    /**
     * @brief Adds a command to advance the swap chain.
     *
     * When swap chain advances, framebuffer with index 1 moves to 0, 2 moves to 1 and so on.
     *
     * @param swapChain Shared pointer to the swap chain object.
     */
    void advance(const SwapChainPointer& swapChain);

    /**
     * @brief Adds a command to clear framebuffer layers.
     *
     * Targets can be any of the render buffers contained in the currently bound Framebuffer.
     * Optionally the scissor test can be enabled locally for this command and to restrict the clearing command to the pixels contained in the scissor rectangle.
     *
     * @param targets A bitmask composed of `Framebuffer::BufferMask` enums indicating which targets to clear.
     * @param color Color value to clear color targets with.
     * @param depth Depth value to clear depth targets with.
     * @param stencil Stencil value to clear stencil targets with.
     * @param enableScissor If enabled, only pixels inside scissor rectangle will be cleared.
     */
    void clearFramebuffer(Framebuffer::Masks targets, const Vec4& color, float depth, int stencil, bool enableScissor = false);

    /**
     * @brief Adds a command to clear framebuffer layers. Clears only color targets.
     *
     * Not a command, just a shortcut for clearFramebuffer, masks out targets to make sure it touches only color targets.
     *
     * @param targets A bitmask composed of `Framebuffer::BufferMask` enums indicating which targets to clear.
     * @param color Color value to clear color targets with.
     * @param enableScissor If enabled, only pixels inside scissor rectangle will be cleared.
     */
    void clearColorFramebuffer(Framebuffer::Masks targets, const Vec4& color, bool enableScissor = false);

    /**
     * @brief Adds a command to clear framebuffer layers. Clears only depth targets.
     *
     * Not a command, just a shortcut for clearFramebuffer, masks out targets to make sure it touches only depth targets.
     *
     * @param depth Depth value to clear depth targets with.
     * @param enableScissor If enabled, only pixels inside scissor rectangle will be cleared.
     */
    void clearDepthFramebuffer(float depth, bool enableScissor = false);

    /**
     * @brief Adds a command to clear framebuffer layers. Clears only stencil targets.
     *
     * Not a command, just a shortcut for clearFramebuffer, masks out targets to make sure it touches only stencil targets.
     *
     * @param stencil Stencil value to clear stencil targets with.
     * @param enableScissor If enabled, only pixels inside scissor rectangle will be cleared.
     */
    void clearStencilFramebuffer(int stencil, bool enableScissor = false);

    /**
     * @brief Adds a command to clear framebuffer layers. Clears only depth and stencil targets.
     *
     * Not a command, just a shortcut for clearFramebuffer, masks out targets to make sure it touches only stencil targets.
     *
     * @param depth Depth value to clear depth targets with.
     * @param stencil Stencil value to clear stencil targets with.
     * @param enableScissor If enabled, only pixels inside scissor rectangle will be cleared.
     */
    void clearDepthStencilFramebuffer(float depth, int stencil, bool enableScissor = false); // not a command, just a shortcut for clearFramebuffer, it touches depth and stencil target

    /**
     * @brief Adds a command to blit one framebuffer to another.
     *
     * The srcRect and dstRect are the rect region in source and destination framebuffers expressed in pixel space,
     * with xy and zw the bounding corners of the rect region.
     *
     * @param src Source framebuffer.
     * @param srcRect Coordinates of the rectangle to blit in the source framebuffer.
     * @param dst Destination framebuffer.
     * @param dstRect Coordinates of the rectangle to blit in the destination framebuffer.
     */
    void blit(const FramebufferPointer& src, const Vec4i& srcRect, const FramebufferPointer& dst, const Vec4i& dstRect);

    /**
     * @brief Adds a command to generates mipmaps.
     *
     * Used by the ambient occlusion effect when SSAO technique is enabled.
     *
     * @param texture Texture for which mipmaps should be generated.
     */
    void generateTextureMips(const TexturePointer& texture);

    /**
     * @brief Adds a command to generates mipmaps using a custom pipeline.
     *
     * Generate the mips for a texture using the current pipeline.
     * Used by the ambient occlusion effect when HBAO technique is enabled.
     *
     * @param destTexture Destination texture.
     * @param numMips Number of mipmaps to generate. Use -1 to generate all mipmap levels.
     */
    void generateTextureMipsWithPipeline(const TexturePointer& destTexture, int numMips = -1);

    // Query Section

    /**
     * @brief Adds a command to start a timestamp query.
     *
     * Queries are used for performance monitoring.
     *
     * @param query Shared pointer to the query object.
     */
    void beginQuery(const QueryPointer& query);

    /**
     * @brief Adds a command to finish a timestamp query.
     *
     * @param query Shared pointer to the query object.
     */
    void endQuery(const QueryPointer& query);

    /**
     * @brief Adds a command to get the result of a timestamp query.
     *
     * @param query Shared pointer to the query object.
     */
    void getQuery(const QueryPointer& query);

    /**
     * @brief Adds a command to reset the renderer backend stage caches and states.
     *
     */
    void resetStages();

    /**
     * @brief Adds a command to disable view correction.
     *
     * Used for secondary camera.
     */
    void disableContextViewCorrection();

    /**
     * @brief Adds a command to restore view correction.
     *
     * Used on the end of secondary camera rendering.
     */
    void restoreContextViewCorrection();

    /**
     * @brief Adds a command to set up view correction for a mirror.
     *
     * View correction is used in VR mode to update eye position just before rendering.
     * Camera positions for mirrors also need to be updated. For the first reflection camera correction needs to be mirrored.
     * Since the renderer supports multiple reflections, for the even reflections view correction gets mirrored twice
     * so it cancels out. `shouldMirror` is true for odd reflections and false for even reflections.
     *
     * @param shouldMirror True if view correction is mirrored for this mirror view.
     */
    void setContextMirrorViewCorrection(bool shouldMirror);

    /**
     * @brief Adds a command to disable stereo rendering.
     *
     * Used for secondary camera.
     */
    void disableContextStereo();

    /**
     * @brief Adds a command to restore previous state of the stereo rendering setting.
     *
     * Used after rendering secondary camera view.
     */
    void restoreContextStereo();

    // Debugging
    /**
     * @brief Adds a command to start a profile range with a given label.
     *
     * Used for labelling steps of the rendering process.
     * The labels show up in RenderDoc and other GPU profiling/debugging tools.
     *
     * @param name Label text.
     */
    void pushProfileRange(const char* name);

    /**
     * @brief Adds a command to end previously started profile range.
     */
    void popProfileRange();

    // TODO: As long as we have gl calls explicitly issued from interface
    // code, we need to be able to record and batch these calls. THe long
    // term strategy is to get rid of any GL calls in favor of the HIFI GPU API
    // For now, instead of calling the raw gl Call, use the equivalent call on the batch so the call is being recorded
    // The implementation of these functions is in GLBackend.cpp
    // VKTODO: do we still need to remove these? They are implemented on Vulkan in a very efficient way.

    /**
     * @brief Add a command to pass a uniform variable with a single integer to the shader in following drawcalls.
     *
     * @param location Slot at which uniform variable will be available.
     * @param v0 Integer value.
     */
    void _glUniform1i(int location, int v0);

    /**
     * @brief Add a command to pass a uniform variable with a single floating point number to the shader in following drawcalls.
     *
     * @param location Slot at which uniform variable will be available.
     * @param v0 Floating point value.
     */
    void _glUniform1f(int location, float v0);

    /**
     * @brief Add a command to pass a uniform variable with 2-dimensional floating point vector to the shader in following drawcalls.
     *
     * @param location Slot at which uniform variable will be available.
     * @param v0 First floating point value.
     * @param v1 Second floating point value.
     */
    void _glUniform2f(int location, float v0, float v1);

    /**
     * @brief Add a command to pass a uniform variable with 3-dimensional floating point vector to the shader in following drawcalls.
     *
     * @param location Slot at which uniform variable will be available.
     * @param v0 First floating point value.
     * @param v1 Second floating point value.
     * @param v2 Third floating point value.
     */
    void _glUniform3f(int location, float v0, float v1, float v2);

    /**
     * @brief Add a command to pass a uniform variable with 4-dimensional floating point vector to the shader in following drawcalls.
     *
     * @param location Slot at which uniform variable will be available.
     * @param v0 First floating point value.
     * @param v1 Second floating point value.
     * @param v2 Third floating point value.
     * @param v3 Fourth floating point value.
     */
    void _glUniform4f(int location, float v0, float v1, float v2, float v3);

    /**
     * @brief Add a command to pass a uniform variable with an array of 3-dimensional floating point vectors to the shader in following drawcalls.
     *
     * @param location Slot at which uniform variable will be available.
     * @param count Number of 3-dimensional vectors in the array.
     * @param value Array of floating point numbers with 3*count length.
     */
    void _glUniform3fv(int location, int count, const float* value);

    /**
     * @brief Add a command to pass a uniform variable with an array of 4-dimensional floating point vectors to the shader in following drawcalls.
     *
     * @param location Slot at which uniform variable will be available.
     * @param count Number of 4-dimensional vectors in the array.
     * @param value Array of floating point numbers with 4*count length.
     */
    void _glUniform4fv(int location, int count, const float* value);

    /**
     * @brief Add a command to pass a uniform variable with an array of 4-dimensional integer vectors to the shader in following drawcalls.
     *
     * @param location Slot at which uniform variable will be available.
     * @param count Number of 4-dimensional vectors in the array.
     * @param value Array of integers with 4*count length.
     */
    void _glUniform4iv(int location, int count, const int* value);

    /**
     * @brief Add a command to pass a uniform variable with an array of 3x3 floating point matrices to the shader in following drawcalls.
     *
     * @param location Slot at which uniform variable will be available.
     * @param count Number of 4-dimensional vectors in the array.
     * @param transpose Should the matrices be transposed.
     * @param value Array of floating point numbers with 9*count length.
     */
    void _glUniformMatrix3fv(int location, int count, unsigned char transpose, const float* value);

    /**
     * @brief Add a command to pass a uniform variable with an array of 4x4 floating point matrices to the shader in following drawcalls.
     *
     * @param location Slot at which uniform variable will be available.
     * @param count Number of 4-dimensional matrices in the array.
     * @param transpose Should the matrices be transposed.
     * @param value Array of floating point numbers with 16*count length.
     */
    void _glUniformMatrix4fv(int location, int count, unsigned char transpose, const float* value);

    /**
     * @brief Add a command to pass a uniform variable with a single integer to the shader in following drawcalls.
     *
     * @param location Slot at which uniform variable will be available.
     * @param v0 Integer value.
     */
    void _glUniform(int location, int v0) {
        _glUniform1i(location, v0);
    }

    /**
     * @brief Add a command to pass a uniform variable with a single floating point number to the shader in following drawcalls.
     *
     * @param location Slot at which uniform variable will be available.
     * @param v0 Floating point value.
     */
    void _glUniform(int location, float v0) {
        _glUniform1f(location, v0);
    }

    /**
     * @brief Add a command to pass a uniform variable with 2-dimensional floating point vector to the shader in following drawcalls.
     *
     * @param location Slot at which uniform variable will be available.
     * @param v 2-dimensional floating point vector.
     */
    void _glUniform(int location, const glm::vec2& v) {
        _glUniform2f(location, v.x, v.y);
    }

    /**
     * @brief Add a command to pass a uniform variable with 3-dimensional floating point vector to the shader in following drawcalls.
     *
     * @param location Slot at which uniform variable will be available.
     * @param v 3-dimensional floating point vector.
     */
    void _glUniform(int location, const glm::vec3& v) {
        _glUniform3f(location, v.x, v.y, v.z);
    }

    /**
     * @brief Add a command to pass a uniform variable with an array of 4-dimensional floating point vectors to the shader in following drawcalls.
     *
     * @param location Slot at which uniform variable will be available.
     * @param v 4-dimensional floating point vector.
     */
    void _glUniform(int location, const glm::vec4& v) {
        _glUniform4f(location, v.x, v.y, v.z, v.w);
    }

    /**
     * @brief Add a command to pass a uniform variable with an array of 3x3 floating point matrices to the shader in following drawcalls.
     *
     * @param location Slot at which uniform variable will be available.
     * @param v 3x3 floating point matrix.
     */
    void _glUniform(int location, const glm::mat3& v) {
        _glUniformMatrix3fv(location, 1, false, glm::value_ptr(v));
    }

    /**
     * @brief Add a command to pass a uniform variable with an array of 4-dimensional floating point vectors to the shader in following drawcalls.
     *
     * @param location Slot at which uniform variable will be available.
     * @param v 4x4 floating point matrix.
     */
    void _glUniform(int location, const glm::mat4& v) {
        _glUniformMatrix4fv(location, 1, false, glm::value_ptr(v));
    }

    // Maybe useful but shouldn't be public. Please convince me otherwise
    // Well porting to GLES i need it...
    /**
     * @brief Runs a lambda function during batch execution.
     *
     * Currently not used.
     *
     * @param f Function to run.
     */
    void runLambda(std::function<void()> f);

    enum Command {
        COMMAND_draw = 0,
        COMMAND_drawIndexed,
        COMMAND_drawInstanced,
        COMMAND_drawIndexedInstanced,
        COMMAND_multiDrawIndirect,
        COMMAND_multiDrawIndexedIndirect,

        COMMAND_setInputFormat,
        COMMAND_setInputBuffer,
        COMMAND_setIndexBuffer,
        COMMAND_setIndirectBuffer,

        COMMAND_setModelTransform,
        COMMAND_setViewTransform,
        COMMAND_setProjectionTransform,
        COMMAND_setProjectionJitterEnabled,
        COMMAND_setProjectionJitterSequence,
        COMMAND_setProjectionJitterScale,
        COMMAND_setViewportTransform,
        COMMAND_setDepthRangeTransform,

        COMMAND_saveViewProjectionTransform,
        COMMAND_setSavedViewProjectionTransform,
        COMMAND_copySavedViewProjectionTransformToBuffer,

        COMMAND_setPipeline,
        COMMAND_setStateBlendFactor,
        COMMAND_setStateScissorRect,

        COMMAND_setUniformBuffer,
        COMMAND_setResourceBuffer,
        COMMAND_setResourceTexture,
        COMMAND_setResourceTextureTable,
        COMMAND_setResourceFramebufferSwapChainTexture,

        COMMAND_setFramebuffer,
        COMMAND_setFramebufferSwapChain,
        COMMAND_clearFramebuffer,
        COMMAND_blit,
        COMMAND_generateTextureMips,
        COMMAND_generateTextureMipsWithPipeline,

        COMMAND_advance,

        COMMAND_beginQuery,
        COMMAND_endQuery,
        COMMAND_getQuery,

        COMMAND_resetStages,

        COMMAND_disableContextViewCorrection,
        COMMAND_restoreContextViewCorrection,
        COMMAND_setContextMirrorViewCorrection,

        COMMAND_disableContextStereo,
        COMMAND_restoreContextStereo,

        COMMAND_runLambda,

        COMMAND_startNamedCall,
        COMMAND_stopNamedCall,

        // TODO: As long as we have gl calls explicitely issued from interface
        // code, we need to be able to record and batch these calls. THe long
        // term strategy is to get rid of any GL calls in favor of the HIFI GPU API
        // Since these are implemented on Vulkan in a very efficient way, maybe we can keep them instead?
        COMMAND_glUniform1i,
        COMMAND_glUniform1f,
        COMMAND_glUniform2f,
        COMMAND_glUniform3f,
        COMMAND_glUniform4f,
        COMMAND_glUniform3fv,
        COMMAND_glUniform4fv,
        COMMAND_glUniform4iv,
        COMMAND_glUniformMatrix3fv,
        COMMAND_glUniformMatrix4fv,

        COMMAND_pushProfileRange,
        COMMAND_popProfileRange,

        NUM_COMMANDS,
    };
    typedef std::vector<Command> Commands;
    typedef std::vector<size_t> CommandOffsets;

    /**
     * @brief Get the command vector.
     *
     * @return Reference to the vector of renderer commands.
     */
    const Commands& getCommands() const { return _commands; }

    /**
     * @brief Get the vector of offsets for renderer commands parameters.
     *
     * Parameters are stored in a `Params` vector. Offset is the index of the first parameter for a given command.
     *
     * @return Vector of offsets for renderer commands parameters.
     */
    const CommandOffsets& getCommandOffsets() const { return _commandOffsets; }

    /**
     * @brief A variant-like class representing single renderer command parameter.
     */
    class Param {
    public:
        union {
#if (QT_POINTER_SIZE == 8)
            size_t _size;
#endif
            int32 _int;
            uint32 _uint;
            float _float;
            char _chars[sizeof(size_t)];
        };
#if (QT_POINTER_SIZE == 8)
        Param(size_t val) : _size(val) {}
#endif
        Param(int32 val) : _int(val) {}
        Param(uint32 val) : _uint(val) {}
        Param(float val) : _float(val) {}
    };
    typedef std::vector<Param> Params;

    /**
     * @brief Gets the renderer commands parameters vector.
     *
     * When commands are added to the batch, their parameters are stored in this vector.
     *
     * @return Vector containing parameters for the renderer commands for this batch.
     */
    const Params& getParams() const { return _params; }

    /**
     * @brief The template cache mechanism for the gpu::Object passed to the gpu::Batch.
     *
     * This allows us to have one cache container for each different types and eventually be smarter how we manage them.
     *
     * @tparam T Type of the object inheriting from gpu::Object to store.
     */
    template <typename T>
    class Cache {
    public:
        typedef T Data;
        Data _data;
        Cache(const Data& data) : _data(data) {}
        static size_t _max;

        /**
         * @brief Templated container for objects inheriting from gpu::Object.
         */
        class Vector {
        public:
            std::vector< Cache<T> > _items;

            Vector() {
                _items.reserve(_max);
            }

            ~Vector() {
                _max = std::max(_items.size(), _max);
            }

            /**
             * @brief Get the current size of the container.
             * @return Current size of the container.
             */
            size_t size() const { return _items.size(); }

            /**
             * @brief Adds a new object to the container.
             *
             * @param data Object to add.
             * @return Index of the added object in the container.
             */
            size_t cache(const Data& data) {
                size_t offset = _items.size();
                _items.emplace_back(data);
                return offset;
            }

            /**
             * @brief Retrieve object with given index.
             *
             * @param offset Index of the object to be retrieved.
             * @return Reference to the object with a given index.
             */
            const Data& get(uint32 offset) const {
                assert((offset < _items.size()));
                return (_items.data() + offset)->_data;
            }

            /**
             * @brief Clear contents of this container.
             */
            void clear() {
                _items.clear();
            }
        };
    };

    using CommandHandler = std::function<void(Command, const Param*)>;

    /**
     * @brief Iterates though each command in the batch and runs a provided function on it.
     *
     * Used by frame writer.
     *
     * @param handler Function to run on every command.
     */
    void forEachCommand(const CommandHandler& handler) const {
        size_t count = _commands.size();
        for (size_t i = 0; i < count; ++i) {
            const auto command = _commands[i];
            const auto offset = _commandOffsets[i];
            const Param* params = _params.data() + offset;
            handler(command, params);
        }
    }

    typedef Cache<BufferPointer>::Vector BufferCaches;
    typedef Cache<TexturePointer>::Vector TextureCaches;
    typedef Cache<TextureTablePointer>::Vector TextureTableCaches;
    typedef Cache<Sampler>::Vector SamplerCaches;
    typedef Cache<Stream::FormatPointer>::Vector StreamFormatCaches;
    typedef Cache<Transform>::Vector TransformCaches;
    typedef Cache<PipelinePointer>::Vector PipelineCaches;
    typedef Cache<FramebufferPointer>::Vector FramebufferCaches;
    typedef Cache<SwapChainPointer>::Vector SwapChainCaches;
    typedef Cache<QueryPointer>::Vector QueryCaches;
    typedef Cache<std::string>::Vector StringCaches;
    typedef Cache<std::function<void()>>::Vector LambdaCache;

    // Cache Data in a byte array if too big to fit in Param
    // FOr example Mat4s are going there
    typedef unsigned char Byte;
    typedef std::vector<Byte> Bytes;

    /**
     * @brief Store data for a parameter that doesn't fit inside `Param` object.
     *
     * Parameters such as matrices or vectors are stored here, and then `Param` object stores index at which given parameter starts in the cache.
     *
     * @param size Size of a given parameter.
     * @param data Pointer to the parameter data.
     * @return Byte offset in the cache buffer at which stored parameter starts.
     */
    size_t cacheData(size_t size, const void* data);

    /**
     * @brief Get a pointer to the parameter cache data at given offset.
     *
     * Data at the returned address can be edited.
     *
     * @param offset Offset at which data to be edited is stored.
     * @return Pointer to the requested data in the parameter cache or 0 if the offset out of bounds of the cache.
     */
    Byte* editData(size_t offset) {
        if (offset >= _data.size()) {
            return 0;
        }
        return (_data.data() + offset);
    }

    /**
     * @brief Get a constant pointer to the parameter cache data at given offset.
     *
     * @param offset Offset at which data to be read is stored.
     * @return Constant pointer to the requested data in the parameter cache or 0 if the offset out of bounds of the cache.
     */
    const Byte* readData(size_t offset) const {
        if (offset >= _data.size()) {
            return 0;
        }
        return (_data.data() + offset);
    }

    Commands _commands; //
    static size_t _commandsMax; //

    CommandOffsets _commandOffsets; //
    static size_t _commandOffsetsMax; //

    Params _params; //
    static size_t _paramsMax; //

    Bytes _data; //
    static size_t _dataMax; //

#include "TransformObject_shared.slh"

    using TransformObjects = std::vector<TransformObject>;

    /**
     * `true` means that current model transform is not stored yet. It gets stored by `captureDrawCallInfo` on next drawcall,
     * and then it gets reused by following drawcalls if it didn't change.
     */
    bool _invalidModel { true };

    /**
     * @brief Current model transform.
     */
    Transform _currentModel;

    /**
     * @brief Model transform from the previos frame.
     *
     * Used to calculate velocity buffer for TAA.
     */
    Transform _previousModel;

    /**
     * @brief Overwrite previous model transforms with the current ones.
     *
     * Batch can be executed multiple times when there's no new batch ready.
     * In such case using transforms from the previous batch would mean incorrect results in velocity buffers.
     */
    mutable bool _mustUpdatePreviousModels;

    /**
     * @brief Model transforms are stored here.
     */
    mutable TransformObjects _objects;
    static size_t _objectsMax; // Needed for reserving vector size and avoiding reallocation.

    Stream::FormatPointer _currentStreamFormat; // Only used for currently disabled `validateDrawState`.
    PipelinePointer _currentPipeline; // Only used for currently disabled `validateDrawState`.

    // Shared pointers to objects needed for the batch are stored here.

    BufferCaches _buffers;
    TextureCaches _textures;
    TextureTableCaches _textureTables;
    SamplerCaches _samplers;
    StreamFormatCaches _streamFormats;
    TransformCaches _transforms;
    PipelineCaches _pipelines;
    FramebufferCaches _framebuffers;
    SwapChainCaches _swapChains;
    QueryCaches _queries;
    LambdaCache _lambdas;
    StringCaches _profileRanges;

    /**
     * @brief Stores names of the named calls (used for instancing).
     */
    StringCaches _names;

    /**
     * @brief `NamedBatchData` objects mapped by name strings. Used for instancing.
     */
    NamedBatchDataMap _namedData;

    /**
     * @brief Stores the current state of TAA jitter setting.
     */
    bool _isJitterOnProjectionEnabled { false };


    /**
     * @brief Current value of drawcall uniform variable, that is passed to a shader as a part of draw call info.
     *
     * Currently used for enabling/disabling skinning and blendshapes.
     */
    uint16_t _drawcallUniform { 0 };

    /**
     * @brief Default value of the Drawcall Uniform.
     *
     * Drawcall Uniform is reset to this value after each drawcall.
     * Can be set with `setDrawcallUniformReset` function.
     */
    uint16_t _drawcallUniformReset { 0 };


    /**
     * `true` if stereo rendering enabled for this batch.
     */
    bool _enableStereo { true };

    /**
     * `true` if this batch renders in skybox mode. Affects how transforms are handled by the backend.
     */
    bool _enableSkybox { false };

protected:
    std::string _name; // Name of this batch.

    friend class Context;
    friend class Frame;

    /**
     * @brief Finalize the frame after all the draw call commands have been collected.
     *
     * Apply all the named calls to the end of the batch (for instancing) and prepare updates for the render shadow copies of the buffers.
     *
     * @param updates
     */
    void finishFrame(BufferUpdates& updates);

    /**
     * @brief Directly copy from the main data to the render thread shadow copy.
     *
     *  Calls `flush` on buffers used for this batch.
     *
     * MUST only be called on the render thread.
     * MUST only be called on batches created on the render thread.
     */
    void flush();

    /**
     * @brief Performs various checks before draw call is issued.
     *
     * Checks if current inputs and shader inputs match.
     * Currently commented out.
     */
    void validateDrawState() const;

    /**
     * @brief Adds a command to start a named call.
     *
     * Used for instancing. When this command is run on backend side, it sets the name of the current call.
     *
     * @param name Name of the named drawcall (instancing group)
     */
    void startNamedCall(const std::string& name);

    /**
     * @brief Adds a command to end a named call.
     *
     * Used for instancing. When this command is run on backend side, it clears the name of the current call.
     */
    void stopNamedCall();

    /**
     * @brief Internal implementation of captureDrawCallInfo.
     *
     * For details see `captureDrawCallInfo`
     */
    void captureDrawCallInfoImpl();
};

/**
 * GPU object cashes in Batch structures keep track of how much memory needs to be preallocated.
 * Since it's a static member, it's defined here.
 */
template <typename T>
size_t Batch::Cache<T>::_max = BATCH_PREALLOCATE_MIN;

}  // namespace gpu

#if defined(NSIGHT_FOUND)

/**
 * Adds `pushProfileRange` command when it's created and `popProfileRange` when it's destroyed.
 * Used by rendering debugging tools.
 */
class ProfileRangeBatch {
public:
    /**
     * @brief Adds `pushProfileRange` command.
     * @param batch Batch to which it will be applied.
     * @param name Name of the profile range.
     */
    ProfileRangeBatch(gpu::Batch& batch, const char *name);

    /**
     * @brief Adds `popProfileRange` command.
     */
    ~ProfileRangeBatch();

private:
    gpu::Batch& _batch;
};

#define PROFILE_RANGE_BATCH(batch, name) ProfileRangeBatch profileRangeThis(batch, name);

#else

#define PROFILE_RANGE_BATCH(batch, name)

#endif

#endif
