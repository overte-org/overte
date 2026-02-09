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
     * Used when addign commands for drawing instances of shapes.
     * @param name
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
     * This allows storing maximum nubrers of commands and various objects from previous frames for this kind of batch
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
     * Its value must be set before each drawcall.
     * Drawcall uniform is used to tell shader if skinning and/or blendshapes are enabled.
     * @param uniform Value to ser Drawcall Unform to.
     */
    void setDrawcallUniform(uint16 uniform);
    // It is reset to the reset value between each drawcalls
    // The reset value is 0 by default and can be changed as a batch state with this call
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
     * @param offset
     * @param stride
     */
    void setInputBuffer(Slot channel, const BufferPointer& buffer, Offset offset, Offset stride);

    /**
     *
     * @param channel
     * @param buffer
     */
    void setInputBuffer(Slot channel, const BufferView& buffer); // not a command, just a shortcut from a BufferView

    /**
     *
     * @param startChannel
     * @param stream
     */
    void setInputStream(Slot startChannel, const BufferStream& stream); // not a command, just unroll into a loop of setInputBuffer

    /**
     *
     * @param type
     * @param buffer
     * @param offset
     */
    void setIndexBuffer(Type type, const BufferPointer& buffer, Offset offset);

    /**
     *
     * @param buffer
     */
    void setIndexBuffer(const BufferView& buffer); // not a command, just a shortcut from a BufferView

    // Indirect buffer is used by the multiDrawXXXIndirect calls
    // The indirect buffer contains the command descriptions to execute multiple drawcalls in a single call
    /**
     *
     * @param buffer
     * @param offset
     * @param stride
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
     *
     * @param model
     */
    void setModelTransform(const Transform& model);

    /**
     *
     * @param model
     * @param previousModel
     */
    void setModelTransform(const Transform& model, const Transform& previousModel);

    /**
     *
     */
    void resetViewTransform() { setViewTransform(Transform(), false); }

    /**
     *
     * @param view
     * @param camera
     */
    void setViewTransform(const Transform& view, bool camera = true);

    /**
     *
     * @param proj
     */
    void setProjectionTransform(const Mat4& proj);

    /**
     *
     * @param isProjectionEnabled
     */
    void setProjectionJitterEnabled(bool isProjectionEnabled);

    /**
     *
     * @param sequence
     * @param count
     */
    void setProjectionJitterSequence(const Vec2* sequence, size_t count);

    /**
     *
     * @param scale
     */
    void setProjectionJitterScale(float scale);

    // Very simple 1 level stack management of jitter.
    /**
     *
     * @param isProjectionEnabled
     */
    void pushProjectionJitterEnabled(bool isProjectionEnabled);

    /**
     *
     */
    void popProjectionJitterEnabled();

    // Viewport is xy = low left corner in framebuffer, zw = width height of the viewport, expressed in pixels
    /**
     *
     * @param viewport
     */
    void setViewportTransform(const Vec4i& viewport);

    /**
     *
     * @param nearDepth
     * @param farDepth
     */
    void setDepthRangeTransform(float nearDepth, float farDepth);

    /**
     *
     * @param saveSlot
     */

    void saveViewProjectionTransform(uint saveSlot);

    /**
     *
     * @param saveSlot
     */
    void setSavedViewProjectionTransform(uint saveSlot);

    /**
     *
     * @param saveSlot
     * @param buffer
     * @param offset
     */
    void copySavedViewProjectionTransformToBuffer(uint saveSlot, const BufferPointer& buffer, Offset offset);

    // Pipeline Stage
    /**
     *
     * @param pipeline
     */
    void setPipeline(const PipelinePointer& pipeline);

    /**
     *
     * @param factor
     */
    void setStateBlendFactor(const Vec4& factor);

    // Set the Scissor rect
    // the rect coordinates are xy for the low left corner of the rect and zw for the width and height of the rect, expressed in pixels
    /**
     *
     * @param rect
     */
    void setStateScissorRect(const Vec4i& rect);

    /**
     *
     * @param slot
     * @param buffer
     * @param offset
     * @param size
     */
    void setUniformBuffer(uint32 slot, const BufferPointer& buffer, Offset offset, Offset size);

    /**
     *
     * @param slot
     * @param view
     */
    void setUniformBuffer(uint32 slot, const BufferView& view); // not a command, just a shortcut from a BufferView

    /**
     *
     * @param slot
     * @param buffer
     */
    void setResourceBuffer(uint32 slot, const BufferPointer& buffer);

    /**
     *
     * @param slot
     * @param texture
     */
    void setResourceTexture(uint32 slot, const TexturePointer& texture);

    /**
     *
     * @param slot
     * @param view
     */
    void setResourceTexture(uint32 slot, const TextureView& view); // not a command, just a shortcut from a TextureView

    /**
     *
     * @param table
     * @param slot
     */
    void setResourceTextureTable(const TextureTablePointer& table, uint32 slot = 0);

    /**
     *
     * @param slot
     * @param framebuffer
     * @param swapChainIndex
     * @param renderBufferSlot
     */
    void setResourceFramebufferSwapChainTexture(uint32 slot, const FramebufferSwapChainPointer& framebuffer, unsigned int swapChainIndex, unsigned int renderBufferSlot = 0U); // not a command, just a shortcut from a TextureView

    // Ouput Stage
    /**
     *
     * @param framebuffer
     */
    void setFramebuffer(const FramebufferPointer& framebuffer);

    /**
     *
     * @param framebuffer
     * @param swapChainIndex
     */
    void setFramebufferSwapChain(const FramebufferSwapChainPointer& framebuffer, unsigned int swapChainIndex);

    /**
     *
     * @param swapChain
     */
    void advance(const SwapChainPointer& swapChain);

    // Clear framebuffer layers
    // Targets can be any of the render buffers contained in the currently bound Framebuffer
    // Optionally the scissor test can be enabled locally for this command and to restrict the clearing command to the pixels contained in the scissor rectangle
    /**
     *
     * @param targets
     * @param color
     * @param depth
     * @param stencil
     * @param enableScissor
     */
    void clearFramebuffer(Framebuffer::Masks targets, const Vec4& color, float depth, int stencil, bool enableScissor = false);

    /**
     *
     * @param targets
     * @param color
     * @param enableScissor
     */
    void clearColorFramebuffer(Framebuffer::Masks targets, const Vec4& color, bool enableScissor = false); // not a command, just a shortcut for clearFramebuffer, mask out targets to make sure it touches only color targets

    /**
     *
     * @param depth
     * @param enableScissor
     */
    void clearDepthFramebuffer(float depth, bool enableScissor = false); // not a command, just a shortcut for clearFramebuffer, it touches only depth target

    /**
     *
     * @param stencil
     * @param enableScissor
     */
    void clearStencilFramebuffer(int stencil, bool enableScissor = false); // not a command, just a shortcut for clearFramebuffer, it touches only stencil target

    /**
     *
     * @param depth
     * @param stencil
     * @param enableScissor
     */
    void clearDepthStencilFramebuffer(float depth, int stencil, bool enableScissor = false); // not a command, just a shortcut for clearFramebuffer, it touches depth and stencil target

    // Blit src framebuffer to destination
    // the srcRect and dstRect are the rect region in source and destination framebuffers expressed in pixel space
    // with xy and zw the bounding corners of the rect region.
    /**
     *
     * @param src
     * @param srcRect
     * @param dst
     * @param dstRect
     */
    void blit(const FramebufferPointer& src, const Vec4i& srcRect, const FramebufferPointer& dst, const Vec4i& dstRect);

    // Generate the mips for a texture
    /**
     *
     * @param texture
     */
    void generateTextureMips(const TexturePointer& texture);

    // Generate the mips for a texture using the current pipeline
    /**
     *
     * @param destTexture
     * @param numMips
     */
    void generateTextureMipsWithPipeline(const TexturePointer& destTexture, int numMips = -1);

    // Query Section

    /**
     *
     * @param query
     */
    void beginQuery(const QueryPointer& query);

    /**
     *
     * @param query
     */
    void endQuery(const QueryPointer& query);

    /**
     *
     * @param query
     */
    void getQuery(const QueryPointer& query);

    // Reset the stage caches and states
    /**
     *
     */
    void resetStages();

    /**
     *
     */
    void disableContextViewCorrection();

    /**
     *
     */
    void restoreContextViewCorrection();

    /**
     *
     * @param shouldMirror
     */
    void setContextMirrorViewCorrection(bool shouldMirror);

    /**
     *
     */
    void disableContextStereo();

    /**
     *
     */
    void restoreContextStereo();

    // Debugging
    /**
     *
     * @param name
     */
    void pushProfileRange(const char* name);

    /**
     *
     */
    void popProfileRange();

    // TODO: As long as we have gl calls explicitly issued from interface
    // code, we need to be able to record and batch these calls. THe long
    // term strategy is to get rid of any GL calls in favor of the HIFI GPU API
    // For now, instead of calling the raw gl Call, use the equivalent call on the batch so the call is being recorded
    // The implementation of these functions is in GLBackend.cpp

    /**
     *
     * @param location
     * @param v0
     */
    void _glUniform1i(int location, int v0);

    /**
     *
     * @param location
     * @param v0
     */
    void _glUniform1f(int location, float v0);

    /**
     *
     * @param location
     * @param v0
     * @param v1
     */
    void _glUniform2f(int location, float v0, float v1);

    /**
     *
     * @param location
     * @param v0
     * @param v1
     * @param v2
     */
    void _glUniform3f(int location, float v0, float v1, float v2);

    /**
     *
     * @param location
     * @param v0
     * @param v1
     * @param v2
     * @param v3
     */
    void _glUniform4f(int location, float v0, float v1, float v2, float v3);

    /**
     *
     * @param location
     * @param count
     * @param value
     */
    void _glUniform3fv(int location, int count, const float* value);

    /**
     *
     * @param location
     * @param count
     * @param value
     */
    void _glUniform4fv(int location, int count, const float* value);

    /**
     *
     * @param location
     * @param count
     * @param value
     */
    void _glUniform4iv(int location, int count, const int* value);

    /**
     *
     * @param location
     * @param count
     * @param transpose
     * @param value
     */
    void _glUniformMatrix3fv(int location, int count, unsigned char transpose, const float* value);

    /**
     *
     * @param location
     * @param count
     * @param transpose
     * @param value
     */
    void _glUniformMatrix4fv(int location, int count, unsigned char transpose, const float* value);

    /**
     *
     * @param location
     * @param v0
     */
    void _glUniform(int location, int v0) {
        _glUniform1i(location, v0);
    }

    /**
     *
     * @param location
     * @param v0
     */
    void _glUniform(int location, float v0) {
        _glUniform1f(location, v0);
    }

    /**
     *
     * @param location
     * @param v
     */
    void _glUniform(int location, const glm::vec2& v) {
        _glUniform2f(location, v.x, v.y);
    }

    /**
     *
     * @param location
     * @param v
     */
    void _glUniform(int location, const glm::vec3& v) {
        _glUniform3f(location, v.x, v.y, v.z);
    }

    /**
     *
     * @param location
     * @param v
     */
    void _glUniform(int location, const glm::vec4& v) {
        _glUniform4f(location, v.x, v.y, v.z, v.w);
    }

    /**
     *
     * @param location
     * @param v
     */
    void _glUniform(int location, const glm::mat3& v) {
        _glUniformMatrix3fv(location, 1, false, glm::value_ptr(v));
    }

    /**
     *
     * @param location
     * @param v
     */
    void _glUniform(int location, const glm::mat4& v) {
        _glUniformMatrix4fv(location, 1, false, glm::value_ptr(v));
    }

    // Maybe useful but shoudln't be public. Please convince me otherwise
    // Well porting to gles i need it...
    /**
     *
     * @param f
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
     *
     * @return
     */
    const Commands& getCommands() const { return _commands; }

    /**
     *
     * @return
     */
const CommandOffsets& getCommandOffsets() const { return _commandOffsets; }

    /**
     *
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
     *
     * @return
     */
    const Params& getParams() const { return _params; }

    // The template cache mechanism for the gpu::Object passed to the gpu::Batch
    // this allow us to have one cache container for each different types and eventually
    // be smarter how we manage them
    /**
     *
     * @tparam T
     */
    template <typename T>
    class Cache {
    public:
        typedef T Data;
        Data _data;
        Cache(const Data& data) : _data(data) {}
        static size_t _max;

        /**
         *
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
             *
             * @return
             */
            size_t size() const { return _items.size(); }

            /**
             *
             * @param data
             * @return
             */
            size_t cache(const Data& data) {
                size_t offset = _items.size();
                _items.emplace_back(data);
                return offset;
            }

            /**
             *
             * @param offset
             * @return
             */
            const Data& get(uint32 offset) const {
                assert((offset < _items.size()));
                return (_items.data() + offset)->_data;
            }

            /**
             *
             */
            void clear() {
                _items.clear();
            }
        };
    };

    using CommandHandler = std::function<void(Command, const Param*)>;

    /**
     *
     * @param handler
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
     *
     * @param size
     * @param data
     * @return
     */
    size_t cacheData(size_t size, const void* data);

    /**
     *
     * @param offset
     * @return
     */
    Byte* editData(size_t offset) {
        if (offset >= _data.size()) {
            return 0;
        }
        return (_data.data() + offset);
    }

    /**
     *
     * @param offset
     * @return
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
     *
     */
    bool _invalidModel { true };

    /**
     *
     */
    Transform _currentModel;

    /**
     *
     */
    Transform _previousModel;

    /**
     *
     */
    mutable bool _mustUpdatePreviousModels;

    /**
     *
     */
    mutable TransformObjects _objects;
    static size_t _objectsMax; //

    /**
     *
     */
    Stream::FormatPointer _currentStreamFormat;

    /**
     *
     */
    PipelinePointer _currentPipeline;

    /**
     *
     */
    BufferCaches _buffers;

    /**
     *
     */
    TextureCaches _textures;

    /**
     *
     */
    TextureTableCaches _textureTables;

    /**
     *
     */
    SamplerCaches _samplers;
    /**
     *
     */
    StreamFormatCaches _streamFormats;

    /**
     *
     */
    TransformCaches _transforms;

    /**
     *
     */
    PipelineCaches _pipelines;

    /**
     *
     */
    FramebufferCaches _framebuffers;

    /**
     *
     */
    SwapChainCaches _swapChains;

    /**
     *
     */
    QueryCaches _queries;

    /**
     *
     */
    LambdaCache _lambdas;

    /**
     *
     */
    StringCaches _profileRanges;

    /**
     *
     */
    StringCaches _names;


    /**
     *
     */
    NamedBatchDataMap _namedData;


    /**
     *
     */
    bool _isJitterOnProjectionEnabled { false };


    /**
     *
     */
    uint16_t _drawcallUniform { 0 };

    /**
     *
     */
    uint16_t _drawcallUniformReset { 0 };


    /**
     *
     */
    bool _enableStereo { true };

    /**
     *
     */
    bool _enableSkybox { false };

protected:
    std::string _name; //

    friend class Context;
    friend class Frame;

    // Apply all the named calls to the end of the batch
    // and prepare updates for the render shadow copies of the buffers
    /**
     *
     * @param updates
     */
    void finishFrame(BufferUpdates& updates);

    // Directly copy from the main data to the render thread shadow copy
    // MUST only be called on the render thread
    // MUST only be called on batches created on the render thread
    /**
     *
     */
    void flush();

    /**
     *
     */
    void validateDrawState() const;

    /**
     *
     * @param name
     */
    void startNamedCall(const std::string& name);

    /**
     *
     */
    void stopNamedCall();

    /**
     *
     */
    void captureDrawCallInfoImpl();
};

/**
 *
 */
template <typename T>
size_t Batch::Cache<T>::_max = BATCH_PREALLOCATE_MIN;

}  // namespace gpu

#if defined(NSIGHT_FOUND)

/**
 *
 */
class ProfileRangeBatch {
public:
    /**
     *
     * @param batch
     * @param name
     */
    ProfileRangeBatch(gpu::Batch& batch, const char *name);
    ~ProfileRangeBatch();

private:
    gpu::Batch& _batch;
};

#define PROFILE_RANGE_BATCH(batch, name) ProfileRangeBatch profileRangeThis(batch, name);

#else

#define PROFILE_RANGE_BATCH(batch, name)

#endif

#endif
