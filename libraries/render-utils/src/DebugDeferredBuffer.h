//
//  DebugDeferredBuffer.h
//  libraries/render-utils/src
//
//  Created by Clement on 12/3/15.
//  Copyright 2015 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_DebugDeferredBuffer_h
#define hifi_DebugDeferredBuffer_h

#include <QFileInfo>

#include <render/DrawTask.h>
#include "DeferredFrameTransform.h"
#include "DeferredFramebuffer.h"
#include "SurfaceGeometryPass.h"
#include "AmbientOcclusionEffect.h"

#include "LightStage.h"

class DebugDeferredBufferConfig : public render::Job::Config {
    Q_OBJECT
    Q_PROPERTY(int mode MEMBER mode WRITE setMode)
    Q_PROPERTY(glm::vec4 size MEMBER size NOTIFY dirty)
public:
    DebugDeferredBufferConfig() : render::Job::Config(false) {}

    void setMode(int newMode);

    int mode{ 0 };
    glm::vec4 size{ 0.0f, -1.0f, 1.0f, 1.0f };
signals:
    void dirty();
};

class DebugDeferredBuffer {
public:
    using Inputs = render::VaryingSet7<DeferredFramebufferPointer,
                                       LinearDepthFramebufferPointer,
                                       SurfaceGeometryFramebufferPointer,
                                       AmbientOcclusionFramebufferPointer,
                                       DeferredFrameTransformPointer,
                                       LightStage::ShadowFramePointer,
                                       gpu::TexturePointer>;
    using Config = DebugDeferredBufferConfig;
    using JobModel = render::Job::ModelI<DebugDeferredBuffer, Inputs, Config>;

    DebugDeferredBuffer(uint transformSlot);
    ~DebugDeferredBuffer();

    void configure(const Config& config);
    void run(const render::RenderContextPointer& renderContext, const Inputs& inputs);

protected:
    friend class DebugDeferredBufferConfig;

    enum Mode : uint8_t
    {
        // Use Mode suffix to avoid collisions
        Off = 0,
        DepthMode,
        AlbedoMode,
        NormalMode,
        RoughnessMode,
        MetallicMode,
        EmissiveMode,
        UnlitMode,
        OcclusionMode,
        LightmapMode,
        ScatteringMode,
        LightingMode,
        ShadowCascade0Mode,
        ShadowCascade1Mode,
        ShadowCascade2Mode,
        ShadowCascade3Mode,
        ShadowCascadeIndicesMode,
        LinearDepthMode,
        HalfLinearDepthMode,
        HalfNormalMode,
        CurvatureMode,
        NormalCurvatureMode,
        DiffusedCurvatureMode,
        DiffusedNormalCurvatureMode,
        CurvatureOcclusionMode,
        ScatteringDebugMode,
        AmbientOcclusionMode,
        AmbientOcclusionBlurredMode,
        AmbientOcclusionNormalMode,
        VelocityMode,
        AntialiasingIntensityMode,
        CustomMode,  // Needs to stay last

        NumModes,
    };

private:
    Mode _mode{ Off };
    glm::vec4 _size;
    uint _transformSlot;

#include "debug_deferred_buffer_shared.slh"

    using ParametersBuffer = gpu::StructBuffer<DebugParameters>;

    struct CustomPipeline {
        gpu::PipelinePointer pipeline;
        QFileInfo info;
    };
    using StandardPipelines = std::array<gpu::PipelinePointer, NumModes>;
    using CustomPipelines = std::unordered_map<std::string, CustomPipeline>;

    static bool pipelineNeedsUpdate(Mode mode, const std::string& customFile = std::string());
    static gpu::PipelinePointer& getPipeline(Mode mode, const std::string& customFile = std::string());
    static std::string getShaderSourceCode(Mode mode, const std::string& customFile = std::string());

    ParametersBuffer _parameters;
    static StandardPipelines _pipelines;
    static CustomPipelines _customPipelines;
    int _geometryId{ 0 };
};

#endif  // hifi_DebugDeferredBuffer_h