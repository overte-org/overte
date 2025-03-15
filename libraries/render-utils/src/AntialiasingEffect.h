//
//  AntialiasingEffect.h
//  libraries/render-utils/src/
//
//  Created by Raffi Bedikian on 8/30/15
//  Copyright 2015 High Fidelity, Inc.
//  Copyright 2022-2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef hifi_AntialiasingEffect_h
#define hifi_AntialiasingEffect_h

#include <DependencyManager.h>

#include "render/DrawTask.h"
#include "DeferredFrameTransform.h"
#include "DeferredFramebuffer.h"
#include "SurfaceGeometryPass.h"

class AntialiasingSetupConfig : public render::Job::Config {
    Q_OBJECT
    Q_PROPERTY(float scale MEMBER scale NOTIFY dirty)
    Q_PROPERTY(bool freeze MEMBER freeze NOTIFY dirty)
    Q_PROPERTY(bool stop MEMBER stop NOTIFY dirty)
    Q_PROPERTY(int index READ getIndex NOTIFY dirty)
    Q_PROPERTY(State state READ getState WRITE setState NOTIFY dirty)
    Q_PROPERTY(Mode mode READ getAAMode WRITE setAAMode NOTIFY dirty)

public:
    AntialiasingSetupConfig() : render::Job::Config(true) {}

    /*@jsdoc
     *Antialiasing modes. <table>
     *   <thead>
     *     <tr><th>Value</th><th>Name</th><th>Description</th>
     *   </thead>
     *   <tbody>
     *     <tr><td><code>0</code></td><td>NONE</td><td>Antialiasing is disabled.</td></tr>
     *     <tr><td><code>1</code></td><td>TAA</td><td>Temporal Antialiasing.</td></tr>
     *     <tr><td><code>2</code></td><td>FXAA</td><td>FXAA.</td></tr>
     *     <tr><td><code>3</code></td><td>MODE_COUNT</td><td>Indicates number of antialiasing modes</td></tr>
     *   </tbody>
     * </table>
     * @typedef {number} AntialiasingMode
     */
    enum class Mode {
        NONE = 0,
        TAA,
        FXAA,
        MODE_COUNT
    };
    Q_ENUM(Mode) // Stored as signed int.

    /*@jsdoc
     *TAA Antialiasing state. <table>
     *   <thead>
     *     <tr><th>Value</th><th>Name</th><th>Description</th>
     *   </thead>
     *   <tbody>
     *     <tr><td><code>0</code></td><td>NONE</td><td>TAA is disabled.</td></tr>
     *     <tr><td><code>1</code></td><td>PAUSE</td><td>TAA jitter is paused.</td></tr>
     *     <tr><td><code>2</code></td><td>PLAY</td><td>TAA jitter is playing.</td></tr>
     *     <tr><td><code>3</code></td><td>STATE_COUNT</td><td>Indicates number of antialiasing states</td></tr>
     *   </tbody>
     * </table>
     * @typedef {number} AntialiasingState
     */
    enum class State
    {
        NONE = 0,
        PAUSE,
        PLAY,
        STATE_COUNT
    };
    Q_ENUM(State)

    float scale { 0.75f };
    bool stop { false };
    bool freeze { false };
    Mode mode { Mode::TAA };

public slots:
    int prev();
    int next();
    State none();
    State pause();
    State play();

    int getIndex() const { return _index; }
    void setIndex(int current);

    State getState() const { return _state; }
    void setState(State state);

    Mode getAAMode() const { return mode; }
    void setAAMode(Mode mode);

signals:
    void dirty();

private:
    State _state { State::PLAY };
    int _index { 0 };

};

class AntialiasingSetup {
public:

    using Config = AntialiasingSetupConfig;
    using Output = AntialiasingSetupConfig::Mode;
    using JobModel = render::Job::ModelO<AntialiasingSetup, Output, Config>;

    AntialiasingSetup();

    void configure(const Config& config);
    void run(const render::RenderContextPointer& renderContext, Output& output);

private:

    std::vector<glm::vec2> _sampleSequence;
    float _scale { 1.0f };
    int _freezedSampleIndex { 0 };
    bool _isStopped { false };
    bool _isFrozen { false };
    AntialiasingSetupConfig::Mode _mode{ AntialiasingSetupConfig::Mode::TAA };
};


class AntialiasingConfig : public render::Job::Config {
    Q_OBJECT
    Q_PROPERTY(float blend MEMBER blend NOTIFY dirty)
    Q_PROPERTY(float sharpen MEMBER sharpen NOTIFY dirty)
    Q_PROPERTY(float covarianceGamma MEMBER covarianceGamma NOTIFY dirty)

    Q_PROPERTY(bool constrainColor MEMBER constrainColor NOTIFY dirty)
    Q_PROPERTY(bool feedbackColor MEMBER feedbackColor NOTIFY dirty)
    Q_PROPERTY(bool bicubicHistoryFetch MEMBER bicubicHistoryFetch NOTIFY dirty)

    Q_PROPERTY(bool debug MEMBER debug NOTIFY dirty)
    Q_PROPERTY(float debugX MEMBER debugX NOTIFY dirty)
    Q_PROPERTY(bool fxaaOnOff READ debugFXAA WRITE setDebugFXAA NOTIFY dirty)
    Q_PROPERTY(float debugShowVelocityThreshold MEMBER debugShowVelocityThreshold NOTIFY dirty)
    Q_PROPERTY(bool showCursorPixel MEMBER showCursorPixel NOTIFY dirty)
    Q_PROPERTY(glm::vec2 debugCursorTexcoord MEMBER debugCursorTexcoord NOTIFY dirty)
    Q_PROPERTY(float debugOrbZoom MEMBER debugOrbZoom NOTIFY dirty)

    Q_PROPERTY(bool showClosestFragment MEMBER showClosestFragment NOTIFY dirty)

public:
    AntialiasingConfig() : render::Job::Config(true) {}

    void setDebugFXAA(bool debug) { debugFXAAX = (debug ? 0.0f : 1.0f); emit dirty();}
    bool debugFXAA() const { return (debugFXAAX == 0.0f ? true : false); }

    float blend { 0.2f };
    float sharpen { 0.05f };

    bool constrainColor { true };
    float covarianceGamma { 1.15f };
    bool feedbackColor { false };
    bool bicubicHistoryFetch { true };

    float debugX { 0.0f };
    float debugFXAAX { 1.0f };
    float debugShowVelocityThreshold { 1.0f };
    glm::vec2 debugCursorTexcoord { 0.5f, 0.5f };
    float debugOrbZoom { 2.0f };

    bool debug { false };
    bool showCursorPixel { false };
    bool showClosestFragment { false };

signals:
    void dirty();
};

#define SET_BIT(bitfield, bitIndex, value) bitfield = ((bitfield) & ~(1 << (bitIndex))) | ((value) << (bitIndex))
#define GET_BIT(bitfield, bitIndex) ((bitfield) & (1 << (bitIndex)))

struct TAAParams {
    float nope { 0.0f };
    float blend { 0.15f };
    float covarianceGamma { 0.9f };
    float debugShowVelocityThreshold { 1.0f };

    glm::ivec4 flags { 0 };
    glm::vec4 pixelInfo { 0.5f, 0.5f, 2.0f, 0.0f };
    glm::vec4 regionInfo { 0.0f, 0.0f, 1.0f, 0.0f };

    void setConstrainColor(bool enabled) { SET_BIT(flags.y, 1, enabled); }
    bool isConstrainColor() const { return (bool)GET_BIT(flags.y, 1); }

    void setFeedbackColor(bool enabled) { SET_BIT(flags.y, 4, enabled); }
    bool isFeedbackColor() const { return (bool)GET_BIT(flags.y, 4); }

    void setBicubicHistoryFetch(bool enabled) { SET_BIT(flags.y, 0, enabled); }
    bool isBicubicHistoryFetch() const { return (bool)GET_BIT(flags.y, 0); }

    void setSharpenedOutput(bool enabled) { SET_BIT(flags.y, 2, enabled); }
    bool isSharpenedOutput() const { return (bool)GET_BIT(flags.y, 2); }

    void setDebug(bool enabled) { SET_BIT(flags.x, 0, enabled); }
    bool isDebug() const { return (bool) GET_BIT(flags.x, 0); }

    void setShowDebugCursor(bool enabled) { SET_BIT(flags.x, 1, enabled); }
    bool showDebugCursor() const { return (bool)GET_BIT(flags.x, 1); }

    void setDebugCursor(glm::vec2 debugCursor) { pixelInfo.x = debugCursor.x; pixelInfo.y = debugCursor.y; }
    glm::vec2 getDebugCursor() const { return glm::vec2(pixelInfo.x, pixelInfo.y); }
    
    void setDebugOrbZoom(float orbZoom) { pixelInfo.z = orbZoom; }
    float getDebugOrbZoom() const { return pixelInfo.z; }

    void setShowClosestFragment(bool enabled) { SET_BIT(flags.x, 3, enabled); }

    bool isFXAAEnabled() const { return regionInfo.z == 0.0f; }
};
using TAAParamsBuffer = gpu::StructBuffer<TAAParams>;

class Antialiasing {
public:
    using Inputs = render::VaryingSet4<DeferredFrameTransformPointer, DeferredFramebufferPointer, LinearDepthFramebufferPointer, AntialiasingSetupConfig::Mode>;
    using Outputs = gpu::TexturePointer;
    using Config = AntialiasingConfig;
    using JobModel = render::Job::ModelIO<Antialiasing, Inputs, Outputs, Config>;

    Antialiasing(bool isSharpenEnabled = true);
    ~Antialiasing();
    void configure(const Config& config);
    void run(const render::RenderContextPointer& renderContext, const Inputs& inputs, Outputs& outputs);

    static const gpu::PipelinePointer& getAntialiasingPipeline();
    static const gpu::PipelinePointer& getIntensityPipeline();
    static const gpu::PipelinePointer& getBlendPipeline();
    static const gpu::PipelinePointer& getDebugBlendPipeline();

private:
    struct AntialiasingBuffer {
        gpu::FramebufferSwapChainPointer _swapChain;
        gpu::TexturePointer _textures[2];

        void clear() {
            _swapChain.reset();
            _textures[0].reset();
            _textures[1].reset();
        }
    };
    AntialiasingBuffer _antialiasingBuffers;
    gpu::FramebufferPointer _intensityFramebuffer;
    gpu::TexturePointer _intensityTexture;
    gpu::BufferPointer _blendParamsBuffer;

    static gpu::PipelinePointer _antialiasingPipeline;
    static gpu::PipelinePointer _intensityPipeline;
    static gpu::PipelinePointer _blendPipeline;
    static gpu::PipelinePointer _debugBlendPipeline;

    TAAParamsBuffer _params;
    float _sharpen { 0.15f };
    bool _isSharpenEnabled { true };
    float _debugFXAAX { 0.0f };
};

#endif // hifi_AntialiasingEffect_h
