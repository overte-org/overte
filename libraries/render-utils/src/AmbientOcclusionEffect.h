//
//  AmbientOcclusionEffect.h
//  libraries/render-utils/src/
//
//  Created by Niraj Venkat on 7/15/15.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_AmbientOcclusionEffect_h
#define hifi_AmbientOcclusionEffect_h

#include <string>
#include <DependencyManager.h>

#include "render/DrawTask.h"

#include "LightingModel.h"
#include "DeferredFrameTransform.h"
#include "DeferredFramebuffer.h"
#include "SurfaceGeometryPass.h"
#include "AmbientOcclusionStage.h"

#include "ssao_shared.h"

class AmbientOcclusionFramebuffer {
public:
    AmbientOcclusionFramebuffer();

    gpu::FramebufferPointer getOcclusionFramebuffer();
    gpu::TexturePointer getOcclusionTexture();
    
    gpu::FramebufferPointer getOcclusionBlurredFramebuffer();
    gpu::TexturePointer getOcclusionBlurredTexture();

    gpu::FramebufferPointer getNormalFramebuffer();
    gpu::TexturePointer getNormalTexture();

#if SSAO_USE_QUAD_SPLIT
    gpu::FramebufferPointer getOcclusionSplitFramebuffer(int index);
    gpu::TexturePointer getOcclusionSplitTexture();
#endif

    // Update the source framebuffer size which will drive the allocation of all the other resources.
    bool update(const gpu::TexturePointer& linearDepthBuffer, int resolutionLevel, int depthResolutionLevel, bool isStereo);
    gpu::TexturePointer getLinearDepthTexture();
    const glm::ivec2& getSourceFrameSize() const { return _frameSize; }
    bool isStereo() const { return _isStereo; }

protected:

    void clear();
    void allocate();
    
    gpu::TexturePointer _linearDepthTexture;
    
    gpu::FramebufferPointer _occlusionFramebuffer;
    gpu::TexturePointer _occlusionTexture;

    gpu::FramebufferPointer _occlusionBlurredFramebuffer;
    gpu::TexturePointer _occlusionBlurredTexture;

    gpu::FramebufferPointer _normalFramebuffer;
    gpu::TexturePointer _normalTexture;

#if SSAO_USE_QUAD_SPLIT
    gpu::FramebufferPointer _occlusionSplitFramebuffers[SSAO_SPLIT_COUNT*SSAO_SPLIT_COUNT];
    gpu::TexturePointer _occlusionSplitTexture;
#endif

    glm::ivec2 _frameSize;
    int _resolutionLevel { 0 };
    int _depthResolutionLevel { 0 };
    bool _isStereo { false };
};

using AmbientOcclusionFramebufferPointer = std::shared_ptr<AmbientOcclusionFramebuffer>;

class AmbientOcclusionEffectConfig : public render::GPUJobConfig::Persistent {
    Q_OBJECT
    Q_PROPERTY(bool debug MEMBER debug NOTIFY dirty)
    Q_PROPERTY(bool horizonBased MEMBER horizonBased NOTIFY dirty)
    Q_PROPERTY(bool ditheringEnabled MEMBER ditheringEnabled NOTIFY dirty)
    Q_PROPERTY(bool borderingEnabled MEMBER borderingEnabled NOTIFY dirty)
    Q_PROPERTY(bool fetchMipsEnabled MEMBER fetchMipsEnabled NOTIFY dirty)
    Q_PROPERTY(bool jitterEnabled MEMBER jitterEnabled NOTIFY dirty)

    Q_PROPERTY(int resolutionLevel MEMBER resolutionLevel WRITE setResolutionLevel)
    Q_PROPERTY(float edgeSharpness MEMBER edgeSharpness WRITE setEdgeSharpness)
    Q_PROPERTY(int blurRadius MEMBER blurRadius WRITE setBlurRadius)

    // SSAO
    Q_PROPERTY(float ssaoRadius MEMBER ssaoRadius WRITE setSSAORadius)
    Q_PROPERTY(float ssaoObscuranceLevel MEMBER ssaoObscuranceLevel WRITE setSSAOObscuranceLevel)
    Q_PROPERTY(float ssaoFalloffAngle MEMBER ssaoFalloffAngle WRITE setSSAOFalloffAngle)
    Q_PROPERTY(float ssaoNumSpiralTurns MEMBER ssaoNumSpiralTurns WRITE setSSAONumSpiralTurns)
    Q_PROPERTY(int ssaoNumSamples MEMBER ssaoNumSamples WRITE setSSAONumSamples)

    // HBAO
    Q_PROPERTY(float hbaoRadius MEMBER hbaoRadius WRITE setHBAORadius)
    Q_PROPERTY(float hbaoObscuranceLevel MEMBER hbaoObscuranceLevel WRITE setHBAOObscuranceLevel)
    Q_PROPERTY(float hbaoFalloffAngle MEMBER hbaoFalloffAngle WRITE setHBAOFalloffAngle)
    Q_PROPERTY(int hbaoNumSamples MEMBER hbaoNumSamples WRITE setHBAONumSamples)

public:
    AmbientOcclusionEffectConfig();

    const int MAX_RESOLUTION_LEVEL = 4;
    const int MAX_BLUR_RADIUS = 15;

    void setResolutionLevel(int level);
    void setEdgeSharpness(float sharpness);
    void setBlurRadius(int radius);

    void setSSAORadius(float newRadius);
    void setSSAOObscuranceLevel(float level);
    void setSSAOFalloffAngle(float bias);
    void setSSAONumSamples(int samples);
    void setSSAONumSpiralTurns(float turns);

    void setHBAORadius(float newRadius);
    void setHBAOObscuranceLevel(float level);
    void setHBAOFalloffAngle(float bias);
    void setHBAONumSamples(int samples);

    bool debug { false };

    bool jitterEnabled { false }; // Add small jittering to AO samples at each frame
    bool horizonBased { false }; // Use horizon based AO
    int resolutionLevel { 2 };
    float edgeSharpness { 1.0f };
    int blurRadius { 4 }; // 0 means no blurring

    float ssaoRadius { 1.0f };
    float ssaoObscuranceLevel { 0.4f }; // intensify or dim down the obscurance effect
    float ssaoFalloffAngle { 0.15f };
    int ssaoNumSamples { 32 };
    float ssaoNumSpiralTurns { 7.0f }; // defining an angle span to distribute the samples ray directions

    float hbaoRadius { 0.7f };
    float hbaoObscuranceLevel { 0.75f }; // intensify or dim down the obscurance effect
    float hbaoFalloffAngle { 0.3f };
    int hbaoNumSamples { 1 };

    float perspectiveScale { 1.0f };
    bool ditheringEnabled { true }; // randomize the distribution of taps per pixel, should always be true
    bool borderingEnabled { true }; // avoid evaluating information from non existing pixels out of the frame, should always be true
    bool fetchMipsEnabled { true }; // fetch taps in sub mips to otpimize cache, should always be true

signals:
    void dirty();
};

#define SSAO_RANDOM_SAMPLE_COUNT 16

class AmbientOcclusionEffect {
public:
    using Input = render::VaryingSet5<LightingModelPointer, DeferredFrameTransformPointer, DeferredFramebufferPointer,
        LinearDepthFramebufferPointer, AmbientOcclusionStage::FramePointer>;
    using Output = render::VaryingSet2<AmbientOcclusionFramebufferPointer, gpu::BufferView>;
    using Config = AmbientOcclusionEffectConfig;
    using JobModel = render::Job::ModelIO<AmbientOcclusionEffect, Input, Output, Config>;

    AmbientOcclusionEffect() {}

    void configure(const Config& config);
    void run(const render::RenderContextPointer& renderContext, const Input& input, Output& output);

    // Class describing the uniform buffer with all the parameters common to the AO shaders
    class AOParameters : public AmbientOcclusionParams {
    public:
        AOParameters();

        int getResolutionLevel() const { return _resolutionInfo.x; }
        float getRadius() const { return _radiusInfo.x; }
        float getPerspectiveScale() const { return _resolutionInfo.z; }
        float getObscuranceLevel() const { return _radiusInfo.w; }
        float getFalloffAngle() const { return (float)_falloffInfo.x; }
        
        float getNumSpiralTurns() const { return _sampleInfo.z; }
        int getNumSamples() const { return (int)_sampleInfo.x; }
        bool isFetchMipsEnabled() const { return _sampleInfo.w; }

        bool isDitheringEnabled() const { return _ditheringInfo.x != 0.0f; }
        bool isBorderingEnabled() const { return _ditheringInfo.w != 0.0f; }
        bool isHorizonBased() const { return _resolutionInfo.y != 0.0f; }
    };
    using AOParametersBuffer = gpu::StructBuffer<AOParameters>;

private:

    // Class describing the uniform buffer with all the parameters common to the bilateral blur shaders
    class BlurParameters : public AmbientOcclusionBlurParams {
    public:
        BlurParameters();

        float getEdgeSharpness() const { return (float)_blurInfo.x; }
        int getBlurRadius() const { return (int)_blurInfo.w; }
    };
    using BlurParametersBuffer = gpu::StructBuffer<BlurParameters>;
    using FrameParametersBuffer = gpu::StructBuffer< AmbientOcclusionFrameParams>;

    void updateParameters(const graphics::AmbientOcclusionPointer ambientOcclusion);
    void updateBlurParameters();
    void updateFramebufferSizes();
    void updateRandomSamples();
    void updateJitterSamples();

    int getDepthResolutionLevel() const;

    AOParametersBuffer _aoParametersBuffer;
    FrameParametersBuffer _aoFrameParametersBuffer[SSAO_SPLIT_COUNT*SSAO_SPLIT_COUNT];
    BlurParametersBuffer _vblurParametersBuffer;
    BlurParametersBuffer _hblurParametersBuffer;
    float _blurEdgeSharpness { 0.0f };

    static const gpu::PipelinePointer& getOcclusionPipeline();
    static const gpu::PipelinePointer& getBilateralBlurPipeline();
    static const gpu::PipelinePointer& getMipCreationPipeline();
    static const gpu::PipelinePointer& getGatherPipeline();
    static const gpu::PipelinePointer& getBuildNormalsPipeline();

    static gpu::PipelinePointer _occlusionPipeline;
    static gpu::PipelinePointer _bilateralBlurPipeline;
    static gpu::PipelinePointer _mipCreationPipeline;
    static gpu::PipelinePointer _gatherPipeline;
    static gpu::PipelinePointer _buildNormalsPipeline;

    AmbientOcclusionFramebufferPointer _framebuffer;
    std::array<float, SSAO_RANDOM_SAMPLE_COUNT * SSAO_SPLIT_COUNT*SSAO_SPLIT_COUNT> _randomSamples;
    int _frameId { 0 };
    bool _debug { false };
    float _perspectiveScale { 1.0f };
    bool _ditheringEnabled { true };
    bool _borderingEnabled { true };
    bool _fetchMipsEnabled { true };
    graphics::AmbientOcclusionPointer _debugAmbientOcclusion { std::make_shared<graphics::AmbientOcclusion>() };

    gpu::RangeTimerPointer _gpuTimer;

    friend class DebugAmbientOcclusion;
};


class DebugAmbientOcclusionConfig : public render::Job::Config {
    Q_OBJECT

    Q_PROPERTY(bool showCursorPixel MEMBER showCursorPixel NOTIFY dirty)
    Q_PROPERTY(glm::vec<2,float,glm::packed_highp> debugCursorTexcoord MEMBER debugCursorTexcoord NOTIFY dirty)
public:
    DebugAmbientOcclusionConfig() : render::Job::Config(false) {}

    bool showCursorPixel { false };
    glm::vec2 debugCursorTexcoord { 0.5f, 0.5f };

signals:
    void dirty();
};


class DebugAmbientOcclusion {
public:
    using Inputs = render::VaryingSet4<DeferredFrameTransformPointer, DeferredFramebufferPointer, LinearDepthFramebufferPointer, AmbientOcclusionEffect::AOParametersBuffer>;
    using Config = DebugAmbientOcclusionConfig;
    using JobModel = render::Job::ModelI<DebugAmbientOcclusion, Inputs, Config>;

    DebugAmbientOcclusion() {}

    void configure(const Config& config);
    void run(const render::RenderContextPointer& renderContext, const Inputs& inputs);
    
private:
       
    // Class describing the uniform buffer with all the parameters common to the debug AO shaders
    class Parameters {
    public:
        // Pixel info
        glm::vec4 pixelInfo { 0.0f, 0.0f, 0.0f, 0.0f };
        
        Parameters() {}
    };
    gpu::StructBuffer<Parameters> _parametersBuffer;

    static gpu::PipelinePointer& getDebugPipeline();

    static gpu::PipelinePointer _debugPipeline;

    bool _showCursorPixel { false };
};

#endif // hifi_AmbientOcclusionEffect_h
