//
//  AmbientOcclusionEffect.cpp
//  libraries/render-utils/src/
//
//  Created by Niraj Venkat on 7/15/15.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "AmbientOcclusionEffect.h"

#include <algorithm> //min max and more

#include <glm/gtc/random.hpp>

#include <PathUtils.h>
#include <SharedUtil.h>
#include <gpu/Context.h>
#include <shaders/Shaders.h>
#include <render/ShapePipeline.h>
#include <MathUtils.h>

#include "RenderUtilsLogging.h"

#include "render-utils/ShaderConstants.h"
#include "DeferredLightingEffect.h"
#include "TextureCache.h"
#include "FramebufferCache.h"
#include "DependencyManager.h"
#include "ViewFrustum.h"

gpu::PipelinePointer AmbientOcclusionEffect::_occlusionPipeline;
gpu::PipelinePointer AmbientOcclusionEffect::_bilateralBlurPipeline;
gpu::PipelinePointer AmbientOcclusionEffect::_mipCreationPipeline;
gpu::PipelinePointer AmbientOcclusionEffect::_gatherPipeline;
gpu::PipelinePointer AmbientOcclusionEffect::_buildNormalsPipeline;

#define MAX_SSAO_SAMPLES 64.0f
#define MAX_HBAO_SAMPLES 6.0f

AmbientOcclusionFramebuffer::AmbientOcclusionFramebuffer() {
}

bool AmbientOcclusionFramebuffer::update(const gpu::TexturePointer& linearDepthBuffer, int resolutionLevel, int depthResolutionLevel, bool isStereo) {
    // If the depth buffer or size changed, we need to delete our FBOs
    bool reset = false;
    if (_linearDepthTexture != linearDepthBuffer) {
        _linearDepthTexture = linearDepthBuffer;
        reset = true;
    }
    if (_resolutionLevel != resolutionLevel || isStereo != _isStereo || _depthResolutionLevel != depthResolutionLevel) {
        _resolutionLevel = resolutionLevel;
        _depthResolutionLevel = depthResolutionLevel;
        _isStereo = isStereo;
        reset = true;
    }
    if (_linearDepthTexture) {
        auto newFrameSize = glm::ivec2(_linearDepthTexture->getDimensions());
        if (_frameSize != newFrameSize) {
            _frameSize = newFrameSize;
            reset = true;
        }
    }
    
    if (reset) {
        clear();
    }

    return reset;
}

void AmbientOcclusionFramebuffer::clear() {
    _occlusionFramebuffer.reset();
    _occlusionTexture.reset();
    _occlusionBlurredFramebuffer.reset();
    _occlusionBlurredTexture.reset();
    _normalFramebuffer.reset();
    _normalTexture.reset();
}

gpu::TexturePointer AmbientOcclusionFramebuffer::getLinearDepthTexture() {
    return _linearDepthTexture;
}

void AmbientOcclusionFramebuffer::allocate() {
#if SSAO_BILATERAL_BLUR_USE_NORMAL    
    auto occlusionformat = gpu::Element{ gpu::VEC4, gpu::HALF, gpu::RGBA };
#else
    auto occlusionformat = gpu::Element{ gpu::VEC3, gpu::NUINT8, gpu::RGB };
#endif

    //  Full frame
    {
        auto width = _frameSize.x;
        auto height = _frameSize.y;
        auto sampler = gpu::Sampler(gpu::Sampler::FILTER_MIN_MAG_LINEAR, gpu::Sampler::WRAP_CLAMP);

        _occlusionTexture = gpu::Texture::createRenderBuffer(occlusionformat, width, height, gpu::Texture::SINGLE_MIP, sampler);
        _occlusionFramebuffer = gpu::FramebufferPointer(gpu::Framebuffer::create("occlusion"));
        _occlusionFramebuffer->setRenderBuffer(0, _occlusionTexture);

        _occlusionBlurredTexture = gpu::Texture::createRenderBuffer(occlusionformat, width, height, gpu::Texture::SINGLE_MIP, sampler);
        _occlusionBlurredFramebuffer = gpu::FramebufferPointer(gpu::Framebuffer::create("occlusionBlurred"));
        _occlusionBlurredFramebuffer->setRenderBuffer(0, _occlusionBlurredTexture);
    }

    // Lower res frame
    {
        auto sideSize = _frameSize;
        if (_isStereo) {
            sideSize.x >>= 1;
        }
        sideSize >>= _resolutionLevel;
        if (_isStereo) {
            sideSize.x <<= 1;
        }
        auto width = sideSize.x;
        auto height = sideSize.y;
        auto format = gpu::Element{ gpu::VEC4, gpu::NINT2_10_10_10, gpu::RGBA };
        _normalTexture = gpu::Texture::createRenderBuffer(format, width, height, gpu::Texture::SINGLE_MIP, 
                                                          gpu::Sampler(gpu::Sampler::FILTER_MIN_MAG_POINT, gpu::Sampler::WRAP_CLAMP));
        _normalFramebuffer = gpu::FramebufferPointer(gpu::Framebuffer::create("ssaoNormals"));
        _normalFramebuffer->setRenderBuffer(0, _normalTexture);
    }

#if SSAO_USE_QUAD_SPLIT
    {
        auto splitSize = _frameSize;
        if (_isStereo) {
            splitSize.x >>= 1;
        }
        splitSize = divideRoundUp(_frameSize >> _resolutionLevel, SSAO_SPLIT_COUNT);
        if (_isStereo) {
            splitSize.x <<= 1;
        }
        auto width = splitSize.x;
        auto height = splitSize.y;

        _occlusionSplitTexture = gpu::Texture::createRenderBufferArray(occlusionformat, width, height, SSAO_SPLIT_COUNT*SSAO_SPLIT_COUNT, gpu::Texture::SINGLE_MIP,
                                                                       gpu::Sampler(gpu::Sampler::FILTER_MIN_MAG_LINEAR, gpu::Sampler::WRAP_CLAMP));
        for (int i = 0; i < SSAO_SPLIT_COUNT*SSAO_SPLIT_COUNT; i++) {
            _occlusionSplitFramebuffers[i] = gpu::FramebufferPointer(gpu::Framebuffer::create("occlusion"));
            _occlusionSplitFramebuffers[i]->setRenderBuffer(0, _occlusionSplitTexture, i);
        }
    }
#endif
}

#if SSAO_USE_QUAD_SPLIT
gpu::FramebufferPointer AmbientOcclusionFramebuffer::getOcclusionSplitFramebuffer(int index) {
    assert(index < SSAO_SPLIT_COUNT*SSAO_SPLIT_COUNT);
    if (!_occlusionSplitFramebuffers[index]) {
        allocate();
    }
    return _occlusionSplitFramebuffers[index];
}

gpu::TexturePointer AmbientOcclusionFramebuffer::getOcclusionSplitTexture() {
    if (!_occlusionSplitTexture) {
        allocate();
    }
    return _occlusionSplitTexture;
}
#endif

gpu::FramebufferPointer AmbientOcclusionFramebuffer::getOcclusionFramebuffer() {
    if (!_occlusionFramebuffer) {
        allocate();
    }
    return _occlusionFramebuffer;
}

gpu::TexturePointer AmbientOcclusionFramebuffer::getOcclusionTexture() {
    if (!_occlusionTexture) {
        allocate();
    }
    return _occlusionTexture;
}

gpu::FramebufferPointer AmbientOcclusionFramebuffer::getOcclusionBlurredFramebuffer() {
    if (!_occlusionBlurredFramebuffer) {
        allocate();
    }
    return _occlusionBlurredFramebuffer;
}

gpu::TexturePointer AmbientOcclusionFramebuffer::getOcclusionBlurredTexture() {
    if (!_occlusionBlurredTexture) {
        allocate();
    }
    return _occlusionBlurredTexture;
}

gpu::FramebufferPointer AmbientOcclusionFramebuffer::getNormalFramebuffer() {
    if (!_normalFramebuffer) {
        allocate();
    }
    return _normalFramebuffer;
}

gpu::TexturePointer AmbientOcclusionFramebuffer::getNormalTexture() {
    if (!_normalTexture) {
        allocate();
    }
    return _normalTexture;
}

AmbientOcclusionEffectConfig::AmbientOcclusionEffectConfig() :
    render::GPUJobConfig::Persistent(QStringList() << "Render" << "Engine" << "Ambient Occlusion") {}

void AmbientOcclusionEffectConfig::setSSAORadius(float newRadius) {
    ssaoRadius = std::max(0.01f, newRadius); emit dirty();
}

void AmbientOcclusionEffectConfig::setSSAOObscuranceLevel(float level) {
    ssaoObscuranceLevel = std::max(0.01f, level);
    emit dirty();
}

void AmbientOcclusionEffectConfig::setSSAOFalloffAngle(float bias) {
    ssaoFalloffAngle = std::max(0.0f, std::min(bias, 1.0f));
    emit dirty();
}

void AmbientOcclusionEffectConfig::setSSAONumSpiralTurns(float turns) {
    ssaoNumSpiralTurns = std::max(0.0f, (float)turns);
    emit dirty();
}

void AmbientOcclusionEffectConfig::setSSAONumSamples(int samples) {
    ssaoNumSamples = std::max(1.0f, (float)samples);
    emit dirty();
}

void AmbientOcclusionEffectConfig::setHBAORadius(float newRadius) {
    hbaoRadius = std::max(0.01f, newRadius); emit dirty();
}

void AmbientOcclusionEffectConfig::setHBAOObscuranceLevel(float level) {
    hbaoObscuranceLevel = std::max(0.01f, level);
    emit dirty();
}

void AmbientOcclusionEffectConfig::setHBAOFalloffAngle(float bias) {
    hbaoFalloffAngle = std::max(0.0f, std::min(bias, 1.0f));
    emit dirty();
}

void AmbientOcclusionEffectConfig::setHBAONumSamples(int samples) {
    hbaoNumSamples = std::max(1.0f, (float)samples);
    emit dirty();
}

void AmbientOcclusionEffectConfig::setEdgeSharpness(float sharpness) { 
    edgeSharpness = std::max(0.0f, (float)sharpness); 
    emit dirty(); 
}

void AmbientOcclusionEffectConfig::setResolutionLevel(int level) { 
    resolutionLevel = std::max(0, std::min(level, MAX_RESOLUTION_LEVEL)); 
    emit dirty(); 
}

void AmbientOcclusionEffectConfig::setBlurRadius(int radius) { 
    blurRadius = std::max(0, std::min(MAX_BLUR_RADIUS, radius)); 
    emit dirty(); 
}

AmbientOcclusionEffect::AOParameters::AOParameters() {
    _resolutionInfo = glm::vec4(0.0f);
    _radiusInfo = glm::vec4(0.0f);
    _ditheringInfo = glm::vec4(0.0f);
    _sampleInfo = glm::vec4(0.0f);
    _falloffInfo = glm::vec4(0.0f);
}

AmbientOcclusionEffect::BlurParameters::BlurParameters() {
    _blurInfo = { 1.0f, 2.0f, 0.0f, 3.0f };
}

void AmbientOcclusionEffect::configure(const Config& config) {
    _debug = config.debug;
    _debugAmbientOcclusion->setTechnique(config.horizonBased ? AmbientOcclusionTechnique::HBAO : AmbientOcclusionTechnique::SSAO);
    _debugAmbientOcclusion->setJitter(config.jitterEnabled);
    _debugAmbientOcclusion->setResolutionLevel(config.resolutionLevel);
    _debugAmbientOcclusion->setEdgeSharpness(config.edgeSharpness);
    _debugAmbientOcclusion->setBlurRadius(config.blurRadius);
    _debugAmbientOcclusion->setAORadius(config.horizonBased ? config.hbaoRadius : config.ssaoRadius);
    _debugAmbientOcclusion->setAOObscuranceLevel(config.horizonBased ? config.hbaoObscuranceLevel : config.ssaoObscuranceLevel);
    _debugAmbientOcclusion->setAOFalloffAngle(config.horizonBased ? config.hbaoFalloffAngle : config.ssaoFalloffAngle);
    _debugAmbientOcclusion->setAOSamplingAmount(config.horizonBased ? (config.hbaoNumSamples / MAX_HBAO_SAMPLES) :
        (config.ssaoNumSamples / MAX_SSAO_SAMPLES));
    _debugAmbientOcclusion->setSSAONumSpiralTurns(config.ssaoNumSpiralTurns);

    _perspectiveScale = config.perspectiveScale;
    _ditheringEnabled = config.ditheringEnabled;
    _borderingEnabled = config.borderingEnabled;
    _fetchMipsEnabled = config.fetchMipsEnabled;
}

void AmbientOcclusionEffect::updateParameters(const graphics::AmbientOcclusionPointer ambientOcclusion) {
    bool shouldUpdateBlurs = false;
    bool shouldUpdateTechnique = false;

    if (!_framebuffer) {
        _framebuffer = std::make_shared<AmbientOcclusionFramebuffer>();
        shouldUpdateBlurs = true;
    }

    // Update bilateral blur
    if (ambientOcclusion->getBlurRadius() != _hblurParametersBuffer->getBlurRadius() || _blurEdgeSharpness != ambientOcclusion->getEdgeSharpness()) {
        const float BLUR_EDGE_DISTANCE_SCALE = float(10000 * SSAO_DEPTH_KEY_SCALE);
        const float BLUR_EDGE_NORMAL_SCALE = 2.0f;

        auto& hblur = _hblurParametersBuffer.edit()._blurInfo;
        auto& vblur = _vblurParametersBuffer.edit()._blurInfo;
        float blurRadialSigma = float(ambientOcclusion->getBlurRadius()) * 0.5f;
        float blurRadialScale = 1.0f / (2.0f*blurRadialSigma*blurRadialSigma);
        glm::vec3 blurScales = -glm::vec3(blurRadialScale, glm::vec2(BLUR_EDGE_DISTANCE_SCALE, BLUR_EDGE_NORMAL_SCALE) * ambientOcclusion->getEdgeSharpness());

        _blurEdgeSharpness = ambientOcclusion->getEdgeSharpness();

        hblur.x = blurScales.x;
        hblur.y = blurScales.y;
        hblur.z = blurScales.z;
        hblur.w = (float)ambientOcclusion->getBlurRadius();

        vblur.x = blurScales.x;
        vblur.y = blurScales.y;
        vblur.z = blurScales.z;
        vblur.w = (float)ambientOcclusion->getBlurRadius();
    }

    if (_perspectiveScale != _aoParametersBuffer->getPerspectiveScale()) {
        _aoParametersBuffer.edit()._resolutionInfo.z = _perspectiveScale;
    }

    if (_ditheringEnabled != _aoParametersBuffer->isDitheringEnabled()) {
        auto& current = _aoParametersBuffer.edit()._ditheringInfo;
        current.x = (float)_ditheringEnabled;
    }

    if (_borderingEnabled != _aoParametersBuffer->isBorderingEnabled()) {
        auto& current = _aoParametersBuffer.edit()._ditheringInfo;
        current.w = (float)_borderingEnabled;
    }

    if (_fetchMipsEnabled != _aoParametersBuffer->isFetchMipsEnabled()) {
        auto& current = _aoParametersBuffer.edit()._sampleInfo;
        current.w = (float)_fetchMipsEnabled;
    }

    bool horizonBased = ambientOcclusion->getTechnique() == AmbientOcclusionTechnique::HBAO;
    if (_aoParametersBuffer->isHorizonBased() != horizonBased) {
        auto& current = _aoParametersBuffer.edit()._resolutionInfo;
        current.y = horizonBased & 1;
        shouldUpdateTechnique = true;
    }

    if (ambientOcclusion->getResolutionLevel() != _aoParametersBuffer->getResolutionLevel()) {
        auto& current = _aoParametersBuffer.edit()._resolutionInfo;
        current.x = (float)ambientOcclusion->getResolutionLevel();
        shouldUpdateBlurs = true;
    }

    if (horizonBased) {
        // Configure for HBAO
        const auto& radius = ambientOcclusion->getAORadius();
        if (shouldUpdateTechnique || radius != _aoParametersBuffer->getRadius()) {
            auto& current = _aoParametersBuffer.edit()._radiusInfo;
            current.x = radius;
            current.y = radius * radius;
            current.z = 1.0f / current.y;
        }

        if (shouldUpdateTechnique || ambientOcclusion->getAOObscuranceLevel() != _aoParametersBuffer->getObscuranceLevel()) {
            auto& current = _aoParametersBuffer.edit()._radiusInfo;
            current.w = ambientOcclusion->getAOObscuranceLevel();
        }

        const float falloffAngle = std::min(1.0f - EPSILON, ambientOcclusion->getAOFalloffAngle());
        if (shouldUpdateTechnique || falloffAngle != _aoParametersBuffer->getFalloffAngle()) {
            auto& current = _aoParametersBuffer.edit()._falloffInfo;
            current.x = falloffAngle;
            current.y = 1.0f / (1.0f - current.x);
            // Compute sin from cos
            current.z = sqrtf(1.0f - current.x * current.x);
            current.w = 1.0f / current.z;
        }

        const int numSamples = std::max(1, (int)(ambientOcclusion->getAOSamplingAmount() * MAX_HBAO_SAMPLES));
        if (shouldUpdateTechnique || numSamples != _aoParametersBuffer->getNumSamples()) {
            auto& current = _aoParametersBuffer.edit()._sampleInfo;
            current.x = numSamples;
            current.y = 1.0f / numSamples;
            updateRandomSamples();
            updateJitterSamples();
        }
    } else {
        // Configure for SSAO
        const double RADIUS_POWER = 6.0;
        const auto& radius = ambientOcclusion->getAORadius();
        if (shouldUpdateTechnique || radius != _aoParametersBuffer->getRadius()) {
            auto& current = _aoParametersBuffer.edit()._radiusInfo;
            current.x = radius;
            current.y = radius * radius;
            current.z = (float)(10.0 / pow((double)radius, RADIUS_POWER));
        }

        if (shouldUpdateTechnique || ambientOcclusion->getAOObscuranceLevel() != _aoParametersBuffer->getObscuranceLevel()) {
            auto& current = _aoParametersBuffer.edit()._radiusInfo;
            current.w = ambientOcclusion->getAOObscuranceLevel();
        }

        if (shouldUpdateTechnique || ambientOcclusion->getAOFalloffAngle() != _aoParametersBuffer->getFalloffAngle()) {
            auto& current = _aoParametersBuffer.edit()._falloffInfo;
            current.x = ambientOcclusion->getAOFalloffAngle();
        }

        if (shouldUpdateTechnique || ambientOcclusion->getSSAONumSpiralTurns() != _aoParametersBuffer->getNumSpiralTurns()) {
            auto& current = _aoParametersBuffer.edit()._sampleInfo;
            current.z = ambientOcclusion->getSSAONumSpiralTurns();
        }

        const int numSamples = std::max(1, (int)(ambientOcclusion->getAOSamplingAmount() * MAX_SSAO_SAMPLES));
        if (shouldUpdateTechnique || numSamples != _aoParametersBuffer->getNumSamples()) {
            auto& current = _aoParametersBuffer.edit()._sampleInfo;
            current.x = numSamples;
            current.y = 1.0f / numSamples;
            updateRandomSamples();
            updateJitterSamples();
        }
    }

    if (shouldUpdateBlurs) {
        updateBlurParameters();
    }
}

void AmbientOcclusionEffect::updateRandomSamples() {
    // Regenerate offsets
    if (_aoParametersBuffer->isHorizonBased()) {
        const int B = 3;
        const float invB = 1.0f / (float)B;
        float sampleScale = float(2.0 * M_PI / double(_aoParametersBuffer->getNumSamples()));

        for (size_t i = 0; i < _randomSamples.size(); i++) {
            auto index = i + 1; // Indices start at 1, not 0
            float f = 1.0f;
            float r = 0.0f;

            while (index > 0) {
                f = f * invB;
                r = r + f * (float)(index % B);
                index = index / B;
            }
            _randomSamples[i] = r * sampleScale;
        }
    } else {
        for (size_t i = 0; i < _randomSamples.size(); i++) {
            _randomSamples[i] = randFloat() * float(2.0 * M_PI);
        }
    }
}
void AmbientOcclusionEffect::updateBlurParameters() {
    const auto resolutionLevel = _aoParametersBuffer->getResolutionLevel();
    auto& vblur = _vblurParametersBuffer.edit();
    auto& hblur = _hblurParametersBuffer.edit();
    auto frameSize = _framebuffer->getSourceFrameSize();
    if (_framebuffer->isStereo()) {
        frameSize.x >>= 1;
    }
    const auto occlusionSize = frameSize >> resolutionLevel;

    if (occlusionSize.x == 0 || occlusionSize.y == 0) {
        return;
    }

    // Occlusion UV limit
    hblur._blurAxis.z = occlusionSize.x / float(frameSize.x);
    hblur._blurAxis.w = occlusionSize.y / float(frameSize.y);

    vblur._blurAxis.z = 1.0f;
    vblur._blurAxis.w = occlusionSize.y / float(frameSize.y);

    // Occlusion axis
    hblur._blurAxis.x = hblur._blurAxis.z / occlusionSize.x;
    hblur._blurAxis.y = 0.0f;

    vblur._blurAxis.x = 0.0f;
    vblur._blurAxis.y = vblur._blurAxis.w / occlusionSize.y;
}

void AmbientOcclusionEffect::updateFramebufferSizes() {
    auto& params = _aoParametersBuffer.edit();
    const int stereoDivide = _framebuffer->isStereo() & 1;
    auto sourceFrameSideSize = _framebuffer->getSourceFrameSize();
    sourceFrameSideSize.x >>= stereoDivide;

    const int resolutionLevel = _aoParametersBuffer.get().getResolutionLevel();
    const int depthResolutionLevel = getDepthResolutionLevel();
    const auto occlusionFrameSize = sourceFrameSideSize >> resolutionLevel;
    auto normalTextureSize = _framebuffer->getNormalTexture()->getDimensions();

    normalTextureSize.x >>= stereoDivide;

    params._sideSizes[0].x = normalTextureSize.x;
    params._sideSizes[0].y = normalTextureSize.y;
    params._sideSizes[0].z = resolutionLevel;
    params._sideSizes[0].w = depthResolutionLevel;

    params._sideSizes[1].x = params._sideSizes[0].x;
    params._sideSizes[1].y = params._sideSizes[0].y;
    auto occlusionSplitSize = divideRoundUp(occlusionFrameSize, SSAO_SPLIT_COUNT);
    params._sideSizes[1].z = occlusionSplitSize.x;
    params._sideSizes[1].w = occlusionSplitSize.y;
}

const gpu::PipelinePointer& AmbientOcclusionEffect::getOcclusionPipeline() {
    if (!_occlusionPipeline) {
        gpu::ShaderPointer program = gpu::Shader::createProgram(shader::render_utils::program::ssao_makeOcclusion);
        gpu::StatePointer state = std::make_shared<gpu::State>();

        state->setColorWriteMask(true, true, true, true);

        // Good to go add the brand new pipeline
        _occlusionPipeline = gpu::Pipeline::create(program, state);
    }
    return _occlusionPipeline;
}

const gpu::PipelinePointer& AmbientOcclusionEffect::getBilateralBlurPipeline() {
    if (!_bilateralBlurPipeline) {
        gpu::ShaderPointer program = gpu::Shader::createProgram(shader::render_utils::program::ssao_bilateralBlur);
        gpu::StatePointer state = std::make_shared<gpu::State>();

        state->setColorWriteMask(true, true, true, false);
        
        // Good to go add the brand new pipeline
        _bilateralBlurPipeline = gpu::Pipeline::create(program, state);
    }
    return _bilateralBlurPipeline;
}

const gpu::PipelinePointer& AmbientOcclusionEffect::getMipCreationPipeline() {
	if (!_mipCreationPipeline) {
		_mipCreationPipeline = gpu::Context::createMipGenerationPipeline(gpu::Shader::createPixel(shader::render_utils::fragment::ssao_mip_depth));
	}
	return _mipCreationPipeline;
}

const gpu::PipelinePointer& AmbientOcclusionEffect::getGatherPipeline() {
    if (!_gatherPipeline) {
        gpu::ShaderPointer program = gpu::Shader::createProgram(shader::render_utils::program::ssao_gather);
        gpu::StatePointer state = std::make_shared<gpu::State>();

        state->setColorWriteMask(true, true, true, true);

        // Good to go add the brand new pipeline
        _gatherPipeline = gpu::Pipeline::create(program, state);
    }
    return _gatherPipeline;
}

const gpu::PipelinePointer& AmbientOcclusionEffect::getBuildNormalsPipeline() {
    if (!_buildNormalsPipeline) {
        gpu::ShaderPointer program = gpu::Shader::createProgram(shader::render_utils::program::ssao_buildNormals);
        gpu::StatePointer state = std::make_shared<gpu::State>();

        state->setColorWriteMask(true, true, true, true);

        // Good to go add the brand new pipeline
        _buildNormalsPipeline = gpu::Pipeline::create(program, state);
    }
    return _buildNormalsPipeline;
}

int AmbientOcclusionEffect::getDepthResolutionLevel() const {
    return std::min(1, _aoParametersBuffer->getResolutionLevel());
}

void AmbientOcclusionEffect::updateJitterSamples() {
    if (_aoParametersBuffer->isHorizonBased()) {
        for (int splitId = 0; splitId < SSAO_SPLIT_COUNT*SSAO_SPLIT_COUNT; splitId++) {
            auto& sample = _aoFrameParametersBuffer[splitId].edit();
            sample._angleInfo.x = _randomSamples[splitId + SSAO_RANDOM_SAMPLE_COUNT * _frameId];
        }
    } else {
        auto& sample = _aoFrameParametersBuffer[0].edit();
        sample._angleInfo.x = _randomSamples[_frameId];
    }
}

void AmbientOcclusionEffect::run(const render::RenderContextPointer& renderContext, const Input& input, Output& output) {
    assert(renderContext->args);
    assert(renderContext->args->hasViewFrustum());

    RenderArgs* args = renderContext->args;

    const auto& lightingModel = input.get0();
    const auto ambientOcclusionFrame = input.get4();

    const auto& ambientOcclusionStage = renderContext->_scene->getStage<AmbientOcclusionStage>();
    graphics::AmbientOcclusionPointer ambientOcclusion;
    if (_debug) {
        ambientOcclusion = _debugAmbientOcclusion;
    } else if (ambientOcclusionStage && ambientOcclusionFrame->_ambientOcclusions.size()) {
        ambientOcclusion = ambientOcclusionStage->getAmbientOcclusion(ambientOcclusionFrame->_ambientOcclusions.front());
    }

    if (!ambientOcclusion || !lightingModel->isAmbientOcclusionEnabled()) {
        output.edit0().reset();
        return;
    }

    updateParameters(ambientOcclusion);

    const auto& frameTransform = input.get1();
    const auto& linearDepthFramebuffer = input.get3();
    
    const int resolutionLevel = _aoParametersBuffer->getResolutionLevel();
    const auto depthResolutionLevel = getDepthResolutionLevel();
    const auto isHorizonBased = _aoParametersBuffer->isHorizonBased();

    auto fullResDepthTexture = linearDepthFramebuffer->getLinearDepthTexture();
    auto occlusionDepthTexture = fullResDepthTexture;
    auto sourceViewport = args->_viewport;
    auto occlusionViewport = sourceViewport >> resolutionLevel;
    auto firstBlurViewport = sourceViewport;
    firstBlurViewport.w = occlusionViewport.w;

    if (!_gpuTimer) {
        _gpuTimer = std::make_shared < gpu::RangeTimer>(__FUNCTION__);
    }

    if (!_framebuffer) {
        _framebuffer = std::make_shared<AmbientOcclusionFramebuffer>();
    }

    if (depthResolutionLevel > 0) {
        occlusionDepthTexture = linearDepthFramebuffer->getHalfLinearDepthTexture();
    }

    if (_framebuffer->update(fullResDepthTexture, resolutionLevel, depthResolutionLevel, args->isStereo())) {
        updateBlurParameters();
        updateFramebufferSizes();
    }
    
    auto occlusionFBO = _framebuffer->getOcclusionFramebuffer();
    auto occlusionBlurredFBO = _framebuffer->getOcclusionBlurredFramebuffer();
    
    output.edit0() = _framebuffer;
    output.edit1() = _aoParametersBuffer;

    auto occlusionPipeline = getOcclusionPipeline();
    auto bilateralBlurPipeline = getBilateralBlurPipeline();
    auto mipCreationPipeline = getMipCreationPipeline();
#if SSAO_USE_QUAD_SPLIT
    auto gatherPipeline = getGatherPipeline();
    auto buildNormalsPipeline = getBuildNormalsPipeline();
    auto occlusionNormalFramebuffer = _framebuffer->getNormalFramebuffer();
    auto occlusionNormalTexture = _framebuffer->getNormalTexture();
    auto normalViewport = glm::ivec4{ 0, 0, occlusionNormalFramebuffer->getWidth(), occlusionNormalFramebuffer->getHeight() };
    auto splitSize = glm::ivec2(_framebuffer->getOcclusionSplitTexture()->getDimensions());
    auto splitViewport = glm::ivec4{ 0, 0, splitSize.x, splitSize.y };
#endif

    // Update sample rotation
    if (ambientOcclusion->getJitter()) {
        updateJitterSamples();
        _frameId = (_frameId + 1) % (SSAO_RANDOM_SAMPLE_COUNT);
    }

    gpu::doInBatch("AmbientOcclusionEffect::run", args->_context, [=](gpu::Batch& batch) {
		PROFILE_RANGE_BATCH(batch, "SSAO");
		batch.enableStereo(false);

        _gpuTimer->begin(batch);

        batch.resetViewTransform();

        batch.setProjectionTransform(glm::mat4());
        batch.setModelTransform(Transform());

        // We need this with the mips levels
        batch.pushProfileRange("Depth Mips");
        if (isHorizonBased) {
            batch.setPipeline(mipCreationPipeline);
            batch.generateTextureMipsWithPipeline(occlusionDepthTexture);
        } else {
            batch.generateTextureMips(occlusionDepthTexture);
        }
        batch.popProfileRange();

#if SSAO_USE_QUAD_SPLIT
        batch.pushProfileRange("Normal Gen.");
        // Build face normals pass
        batch.setModelTransform(Transform());
        batch.setViewportTransform(normalViewport);
        batch.setPipeline(buildNormalsPipeline);
        batch.setResourceTexture(render_utils::slot::texture::SsaoDepth, fullResDepthTexture);
        batch.setResourceTexture(render_utils::slot::texture::SsaoNormal, nullptr);
        batch.setUniformBuffer(render_utils::slot::buffer::DeferredFrameTransform, frameTransform->getFrameTransformBuffer());
        batch.setUniformBuffer(render_utils::slot::buffer::SsaoParams, _aoParametersBuffer);
        batch.setFramebuffer(occlusionNormalFramebuffer);
        batch.draw(gpu::TRIANGLE_STRIP, 4);
        batch.popProfileRange();
#endif

        // Occlusion pass
        batch.pushProfileRange("Occlusion");

        batch.setUniformBuffer(render_utils::slot::buffer::DeferredFrameTransform, frameTransform->getFrameTransformBuffer());
        batch.setUniformBuffer(render_utils::slot::buffer::SsaoParams, _aoParametersBuffer);
        batch.setPipeline(occlusionPipeline);
        batch.setResourceTexture(render_utils::slot::texture::SsaoDepth, occlusionDepthTexture);

        if (_aoParametersBuffer->isHorizonBased()) {
#if SSAO_USE_QUAD_SPLIT
            batch.setResourceTexture(render_utils::slot::texture::SsaoNormal, occlusionNormalTexture);
            {
                const auto uvScale = glm::vec3(
                    (splitSize.x * SSAO_SPLIT_COUNT) / float(occlusionViewport.z),
                        (splitSize.y * SSAO_SPLIT_COUNT) / float(occlusionViewport.w),
                    1.0f);
                const auto postPixelOffset = glm::vec2(0.5f) / glm::vec2(occlusionViewport.z, occlusionViewport.w);
                const auto prePixelOffset = glm::vec2(0.5f * uvScale.x, 0.5f * uvScale.y) / glm::vec2(splitSize);
                Transform model;

                batch.setViewportTransform(splitViewport);

                model.setScale(uvScale);
                for (int y = 0; y < SSAO_SPLIT_COUNT; y++) {
                    for (int x = 0; x < SSAO_SPLIT_COUNT; x++) {
                        const int splitIndex = x + y * SSAO_SPLIT_COUNT;
                        const auto uvTranslate = glm::vec3(
                            postPixelOffset.x * (2 * x + 1) - prePixelOffset.x,
                            postPixelOffset.y * (2 * y + 1) - prePixelOffset.y,
                            0.0f
                        );
                        model.setTranslation(uvTranslate);
                        batch.setModelTransform(model);
                        batch.setFramebuffer(_framebuffer->getOcclusionSplitFramebuffer(splitIndex));
                        batch.setUniformBuffer(render_utils::slot::buffer::SsaoFrameParams, _aoFrameParametersBuffer[splitIndex]);
                        batch.draw(gpu::TRIANGLE_STRIP, 4);
                    }
                }
            }
#else
            batch.setViewportTransform(occlusionViewport);
            batch.setModelTransform(Transform());
            batch.setFramebuffer(occlusionFBO);
            batch.setUniformBuffer(render_utils::slot::buffer::SsaoFrameParams, _aoFrameParametersBuffer[0]);
            batch.draw(gpu::TRIANGLE_STRIP, 4);
#endif
        } else {
#if SSAO_USE_QUAD_SPLIT
            batch.setResourceTexture(render_utils::slot::texture::SsaoNormal, occlusionNormalTexture);
#endif
            batch.setViewportTransform(occlusionViewport);
            batch.setModelTransform(Transform());
            batch.setFramebuffer(occlusionFBO);
            batch.setUniformBuffer(render_utils::slot::buffer::SsaoFrameParams, _aoFrameParametersBuffer[0]);
            batch.draw(gpu::TRIANGLE_STRIP, 4);
        }

        batch.popProfileRange();

#if SSAO_USE_QUAD_SPLIT
        if (_aoParametersBuffer->isHorizonBased()) {
            // Gather back the four separate renders into one interleaved one
            batch.pushProfileRange("Gather");
            batch.setViewportTransform(occlusionViewport);
            batch.setModelTransform(Transform());
            batch.setFramebuffer(occlusionFBO);
            batch.setPipeline(gatherPipeline);
            batch.setResourceTexture(render_utils::slot::texture::SsaoOcclusion, _framebuffer->getOcclusionSplitTexture());
            batch.draw(gpu::TRIANGLE_STRIP, 4);
            batch.popProfileRange();
        }
#endif

        {
            PROFILE_RANGE_BATCH(batch, "Bilateral Blur");
            // Blur 1st pass
            batch.pushProfileRange("Horiz.");
            {
                const auto uvScale = glm::vec3(
                    occlusionViewport.z / float(sourceViewport.z),
                    occlusionViewport.w / float(sourceViewport.w),
                    1.0f);
                Transform model;
                model.setScale(uvScale);
                batch.setModelTransform(model);
            }
            batch.setPipeline(bilateralBlurPipeline);
            // Use full resolution depth for bilateral upscaling and blur
            batch.setResourceTexture(render_utils::slot::texture::SsaoDepth, fullResDepthTexture);
#if SSAO_USE_QUAD_SPLIT
            batch.setResourceTexture(render_utils::slot::texture::SsaoNormal, occlusionNormalTexture);
#else
            batch.setResourceTexture(render_utils::slot::texture::SsaoNormal, nullptr);
#endif
            batch.setViewportTransform(firstBlurViewport);
            batch.setFramebuffer(occlusionBlurredFBO);
            batch.setUniformBuffer(render_utils::slot::buffer::SsaoBlurParams, _hblurParametersBuffer);
            batch.setResourceTexture(render_utils::slot::texture::SsaoOcclusion, occlusionFBO->getRenderBuffer(0));
            batch.draw(gpu::TRIANGLE_STRIP, 4);
            batch.popProfileRange();

            // Blur 2nd pass
            batch.pushProfileRange("Vert.");
            {
                const auto uvScale = glm::vec3(
                    1.0f,
                    occlusionViewport.w / float(sourceViewport.w),
                    1.0f);

                Transform model;
                model.setScale(uvScale);
                batch.setModelTransform(model);
            }
            batch.setViewportTransform(sourceViewport);
            batch.setFramebuffer(occlusionFBO);
            batch.setUniformBuffer(render_utils::slot::buffer::SsaoBlurParams, _vblurParametersBuffer);
            batch.setResourceTexture(render_utils::slot::texture::SsaoOcclusion, occlusionBlurredFBO->getRenderBuffer(0));
            batch.draw(gpu::TRIANGLE_STRIP, 4);
            batch.popProfileRange();
        }

        batch.setResourceTexture(render_utils::slot::texture::SsaoDepth, nullptr);
        batch.setResourceTexture(render_utils::slot::texture::SsaoNormal, nullptr);
        batch.setResourceTexture(render_utils::slot::texture::SsaoOcclusion, nullptr);

        _gpuTimer->end(batch);
    });

    // Update the timer
    auto config = std::static_pointer_cast<Config>(renderContext->jobConfig);
    config->setGPUBatchRunTime(_gpuTimer->getGPUAverage(), _gpuTimer->getBatchAverage());
}

gpu::PipelinePointer DebugAmbientOcclusion::_debugPipeline;

void DebugAmbientOcclusion::configure(const Config& config) {
    _showCursorPixel = config.showCursorPixel;

    auto cursorPos = glm::vec2(_parametersBuffer->pixelInfo);
    if (cursorPos != config.debugCursorTexcoord) {
        _parametersBuffer.edit().pixelInfo = glm::vec4(config.debugCursorTexcoord, 0.0f, 0.0f);
    }
}

gpu::PipelinePointer& DebugAmbientOcclusion::getDebugPipeline() {
    if (!_debugPipeline) {
        gpu::ShaderPointer program = gpu::Shader::createProgram(shader::render_utils::program::ssao_debugOcclusion);
        gpu::StatePointer state = std::make_shared<gpu::State>();

        state->setColorWriteMask(true, true, true, false);
        state->setBlendFunction(true, gpu::State::SRC_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::INV_SRC_ALPHA);
        // Good to go add the brand new pipeline
        _debugPipeline = gpu::Pipeline::create(program, state);
    }
    return _debugPipeline;
}

void DebugAmbientOcclusion::run(const render::RenderContextPointer& renderContext, const Inputs& inputs) {
    assert(renderContext->args);
    assert(renderContext->args->hasViewFrustum());

    if (!_showCursorPixel) {
        return;
    }

    RenderArgs* args = renderContext->args;

    const auto& frameTransform = inputs.get0();
    const auto& linearDepthFramebuffer = inputs.get2();
    const auto& ambientOcclusionUniforms = inputs.get3();
    
    // Skip if AO is not started yet
    if (!ambientOcclusionUniforms._buffer) {
        return;
    }

    auto fullResDepthTexture = linearDepthFramebuffer->getLinearDepthTexture();
    auto sourceViewport = args->_viewport;
    auto occlusionViewport = sourceViewport;

    auto resolutionLevel = ambientOcclusionUniforms->getResolutionLevel();
    
    if (resolutionLevel > 0) {
        fullResDepthTexture = linearDepthFramebuffer->getHalfLinearDepthTexture();
        occlusionViewport = occlusionViewport >> ambientOcclusionUniforms->getResolutionLevel();
    }

    auto framebufferSize = glm::ivec2(fullResDepthTexture->getDimensions());
    
    float sMin = occlusionViewport.x / (float)framebufferSize.x;
    float sWidth = occlusionViewport.z / (float)framebufferSize.x;
    float tMin = occlusionViewport.y / (float)framebufferSize.y;
    float tHeight = occlusionViewport.w / (float)framebufferSize.y;
    
    auto debugPipeline = getDebugPipeline();
    
    gpu::doInBatch("DebugAmbientOcclusion::run", args->_context, [=](gpu::Batch& batch) {
        batch.enableStereo(false);

        batch.setViewportTransform(sourceViewport);
        batch.setProjectionTransform(glm::mat4());
        batch.setViewTransform(Transform());

        Transform model;
        model.setTranslation(glm::vec3(sMin, tMin, 0.0f));
        model.setScale(glm::vec3(sWidth, tHeight, 1.0f));
        batch.setModelTransform(model);

        batch.setUniformBuffer(render_utils::slot::buffer::DeferredFrameTransform, frameTransform->getFrameTransformBuffer());
        batch.setUniformBuffer(render_utils::slot::buffer::SsaoParams, ambientOcclusionUniforms);
        batch.setUniformBuffer(render_utils::slot::buffer::SsaoDebugParams, _parametersBuffer);
        
        batch.setPipeline(debugPipeline);
        batch.setResourceTexture(render_utils::slot::texture::SsaoDepth, fullResDepthTexture);
        batch.draw(gpu::TRIANGLE_STRIP, 4);

        batch.setResourceTexture(render_utils::slot::texture::SsaoDepth, nullptr);
    });
}
