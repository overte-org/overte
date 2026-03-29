//
//  Backend.h
//  interface/src/gpu
//
//  Created by Olivier Prat on 05/18/2018.
//  Copyright 2018 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_gpu_Backend_h
#define hifi_gpu_Backend_h

#include <GLMHelpers.h>

#include "Forward.h"
#include "Batch.h"
#include "Buffer.h"
#include "Framebuffer.h"

class QImage;

namespace gpu {
class Context;

struct ContextStats {
public:
    int _ISNumFormatChanges = 0;
    int _ISNumInputBufferChanges = 0;
    int _ISNumIndexBufferChanges = 0;

    int _RSNumResourceBufferBounded = 0;
    int _RSNumTextureBounded = 0;
    int _RSAmountTextureMemoryBounded = 0;

    int _DSNumAPIDrawcalls = 0;
    int _DSNumDrawcalls = 0;
    int _DSNumTriangles = 0;

    int _PSNumSetPipelines = 0;

    ContextStats() {}
    ContextStats(const ContextStats& stats) = default;

    void evalDelta(const ContextStats& begin, const ContextStats& end);
};

class Backend {
public:
    virtual ~Backend() {}

    virtual void shutdown() {}
    virtual const std::string& getVersion() const = 0;

    void setStereoState(const StereoState& stereo);

    /**
     * @brief Renders given frame.
     *
     * Is called by Context::executeFrame, which is called by the display plugin on the Present Thread.
     * @param frame Frame to be rendered
     */

    virtual void executeFrame(const FramePointer& frame) = 0;

    /**
     * @brief Renders a single batch.
     * Renders additional batches that were not included in the Frame object.
     * It's used by Android build to draw virtual on-screen d-pad buttons, and also for drawing cursor, both in desktop and VR mode.
     * It's called by display plugins.
     * @param batch
     */
    virtual void render(const Batch& batch) = 0;

    /**
     * @brief
     * DOCTODO: what does it do?
     */
    virtual void syncCache() = 0;

    /**
     * @brief DOCTODO check if it's called anywhere else
     * It's only called on Android builds by GraphicsEngine::initializeGPU.
     * @param program
     */
    virtual void syncProgram(const gpu::ShaderPointer& program) = 0;

    /**
     * @brief Perform per-frame cleanup.
     * Currently called only by OpenGL backend.
     */
    virtual void recycle() const = 0;

    /**
     * @brief Downloads given framebuffer to CPU memory.
     * Used for taking screenshots.
     * @param srcFramebuffer Framebuffer which is to be downloaded.
     * @param region Area which needs to be stored, in pixels.
     * @param destImage
     */
    virtual void downloadFramebuffer(const FramebufferPointer& srcFramebuffer, const Vec4i& region, QImage& destImage) = 0;

    /**
     * @brief Updates view correction matrix.
     * View correction matrix is used to move camera from the position it was originally in when Frame object was generated.
     * In desktop mode it's always an identity matrix.
     * In VR it reduces the apparent latency by using eyes transform from just before the frame begins execution.
     * It's also called by setContextMirrorViewCorrection batch command.
     * TODO: investigate if it can be used in desktop mode for reducing apparent latency.
     * @param correction New view correction matrix.
     * @param primary True if it's for the primary view, false if it's for a mirror.
     */
    virtual void updatePresentFrame(const Mat4& correction = Mat4(), bool primary = true) = 0;

    /**
     * @brief Checks if texture format is supported.
     * It's used by NetworkTexture.
     * @param format Texture format to check for.
     * @return `true` is texture format is supported, `false` if not.
     */
    virtual bool supportedTextureFormat(const gpu::Element& format) const = 0;

        // Shared header between C++ and GLSL
#include "TransformCamera_shared.slh"

    /**
     * Stores camera information and is copied to the shader as a uniform buffer.
     */
    class TransformCamera : public _TransformCamera {
    public:
        /**
         * @brief Calculates helper matrices that are derived from ones passed to this object.
         * Is called only from inside getMonoCamera and getStereoCamera.
         * @param view Current view transform.
         * @param previousView Transform from previous frame, used for velocity buffer and TAA.
         * @param previousProjection Projection matrix from the previous frame.
         * @return Pointer to this TransformCamera object
         * DOCTODO: can be made private?
         */
        const Backend::TransformCamera& recomputeDerived(const Transform& view, const Transform& previousView, const Mat4& previousProjection) const;

        // Jitter should be divided by framebuffer size
        /**
         *
         * @param isSkybox During skybox render pass translation is disabled.
         * @param view Current view transform.
         * @param previousView View transform from previous frame, used for TAA.
         * @param previousProjection Projection from previous frame, used for TAA.
         * @param normalizedJitter For TAA. Jitter should be divided by framebuffer size.
         * @return
         */

        TransformCamera getMonoCamera(bool isSkybox, const Transform& view, Transform previousView, Mat4 previousProjection, Vec2 normalizedJitter) const;

        /**
         *
         * @param eye 0 - left eye, 1 - right eye.
         * @param stereo Structure containing information about stereo rendering setup for the current frame.
         * @param prevStereo Structure containing information about stereo rendering setup for the previous frame. Used for TAA.
         * @param view Current view transform.
         * @param previousView View transform from previous frame, used for TAA.
         * @param normalizedJitter For TAA. Jitter should be divided by framebuffer size.
         * @return
         */
        TransformCamera getEyeCamera(int eye, const StereoState& stereo, const StereoState& prevStereo, const Transform& view, const Transform& previousView,
            Vec2 normalizedJitter) const;
    };

    /**
     * @brief Sets backend-specific object associated with a given renderer object such as texture or buffer.
     * @tparam T Backend-specific object type.
     * @tparam U Renderer object type.
     * @param object Renderer object.
     * @param gpuObject Backend-specific object.
     */
    template <typename T, typename U>
    static void setGPUObject(const U& object, T* gpuObject) {
        object.gpuObject.setGPUObject(gpuObject);
    }

    /**
     * @brief Returns backend-specific object associated with a given renderer object such as texture or buffer.
     * @tparam T Backend-specific object type.
     * @tparam U Renderer object type.
     * @param object Renderer object.
     * @return Backend-specific object, or nullptr if not available.
     */
    template <typename T, typename U>
    static T* getGPUObject(const U& object) {
        return reinterpret_cast<T*>(object.gpuObject.getGPUObject());
    }

    /**
     * @brief Resets render statistics such as draw call count.
     * Used by Context::resetStats()
     */
    void resetStats() const { _stats = ContextStats(); }

    /**
     * @brief Retrieves current render statistics such as draw call count.
     * @param stats Object to which current statistics will be written.
     */
    void getStats(ContextStats& stats) const { stats = _stats; }

    /**
     * @brief DOCTODO: what are sparse textures?
     * @return
     */
    virtual bool isTextureManagementSparseEnabled() const = 0;

    // These should only be accessed by Backend implementation to report the buffer and texture allocations,
    // they are NOT public objects
    static ContextMetricSize freeGPUMemSize;

    static ContextMetricCount bufferCount;
    static ContextMetricSize bufferGPUMemSize;

    static ContextMetricCount textureResidentCount;
    static ContextMetricCount textureFramebufferCount;
    static ContextMetricCount textureResourceCount;
    static ContextMetricCount textureExternalCount;

    static ContextMetricSize textureResidentGPUMemSize;
    static ContextMetricSize textureFramebufferGPUMemSize;
    static ContextMetricSize textureResourceGPUMemSize;
    static ContextMetricSize textureExternalGPUMemSize;

    static ContextMetricCount texturePendingGPUTransferCount;
    static ContextMetricSize texturePendingGPUTransferMemSize;
    static ContextMetricSize textureResourcePopulatedGPUMemSize;
    static ContextMetricSize textureResourceIdealGPUMemSize;

protected:
    virtual bool isStereo() const {
        return _stereo.isStereo();
    }

    void getStereoProjections(mat4* eyeProjections) const {
        for (int i = 0; i < 2; ++i) {
            eyeProjections[i] = _stereo._eyeProjections[i];
        }
    }

    void getStereoViews(mat4* eyeViews) const {
        for (int i = 0; i < 2; ++i) {
            eyeViews[i] = _stereo._eyeViews[i];
        }
    }

    friend class Context;
    mutable ContextStats _stats;
    StereoState _stereo;
    StereoState _prevStereo;
};

}

#endif
