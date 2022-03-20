//
//  SubsurfaceScattering.cpp
//  libraries/render-utils/src/
//
//  Created by Sam Gateau 6/3/2016.
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "SubsurfaceScattering.h"

#include <gpu/Context.h>
#include <shaders/Shaders.h>
#include <render/ShapePipeline.h>
#include <graphics/ShaderConstants.h>

#include "render-utils/ShaderConstants.h"

#include "FramebufferCache.h"

#include "DeferredLightingEffect.h"


namespace ru {
    using render_utils::slot::texture::Texture;
    using render_utils::slot::buffer::Buffer;
}

namespace gr {
    using graphics::slot::texture::Texture;
    using graphics::slot::buffer::Buffer;
}


SubsurfaceScatteringResource::SubsurfaceScatteringResource() {
    Parameters parameters;
    _parametersBuffer = gpu::BufferView(std::make_shared<gpu::Buffer>(sizeof(Parameters), (const gpu::Byte*) &parameters));

}

void SubsurfaceScatteringResource::setBentNormalFactors(const glm::vec4& rgbsBentFactors) {
    if (rgbsBentFactors != getBentNormalFactors()) {
        _parametersBuffer.edit<Parameters>().normalBentInfo = rgbsBentFactors;
    }
}

glm::vec4 SubsurfaceScatteringResource::getBentNormalFactors() const {
    return _parametersBuffer.get<Parameters>().normalBentInfo;
}

void SubsurfaceScatteringResource::setCurvatureFactors(const glm::vec2& sbCurvatureFactors) {
    if (sbCurvatureFactors != getCurvatureFactors()) {
        _parametersBuffer.edit<Parameters>().curvatureInfo = sbCurvatureFactors;
    }
}

glm::vec2 SubsurfaceScatteringResource::getCurvatureFactors() const {
    return _parametersBuffer.get<Parameters>().curvatureInfo;
}


void SubsurfaceScatteringResource::setLevel(float level) {
    if (level != getLevel()) {
        _parametersBuffer.edit<Parameters>().level = level;
    }
}
float SubsurfaceScatteringResource::getLevel() const {
    return _parametersBuffer.get<Parameters>().level;
}

void SubsurfaceScatteringResource::setShowBRDF(bool show) {
    if (show != isShowBRDF()) {
        _parametersBuffer.edit<Parameters>().showBRDF = show;
    }
}
bool SubsurfaceScatteringResource::isShowBRDF() const {
    return (bool)_parametersBuffer.get<Parameters>().showBRDF;
}

void SubsurfaceScatteringResource::setShowCurvature(bool show) {
    if (show != isShowCurvature()) {
        _parametersBuffer.edit<Parameters>().showCurvature = show;
    }
}
bool SubsurfaceScatteringResource::isShowCurvature() const {
    return (bool)_parametersBuffer.get<Parameters>().showCurvature;
}

void SubsurfaceScatteringResource::setShowDiffusedNormal(bool show) {
    if (show != isShowDiffusedNormal()) {
        _parametersBuffer.edit<Parameters>().showDiffusedNormal = show;
    }
}
bool SubsurfaceScatteringResource::isShowDiffusedNormal() const {
    return (bool)_parametersBuffer.get<Parameters>().showDiffusedNormal;
}

void SubsurfaceScatteringResource::generateScatteringTable(RenderArgs* args) {
    if (!_scatteringProfile) {
        _scatteringProfile = generateScatteringProfile(args);
    }
    if (!_scatteringTable) {
        _scatteringTable = generatePreIntegratedScattering(_scatteringProfile, args);
    }
    if (!_scatteringSpecular) {
        _scatteringSpecular = generateScatteringSpecularBeckmann(args);
    }
}

SubsurfaceScattering::SubsurfaceScattering() {
    _scatteringResource = std::make_shared<SubsurfaceScatteringResource>();
}

void SubsurfaceScattering::configure(const Config& config) {

    glm::vec4 bentInfo(config.bentRed, config.bentGreen, config.bentBlue, config.bentScale);
    _scatteringResource->setBentNormalFactors(bentInfo);

    glm::vec2 curvatureInfo(config.curvatureOffset, config.curvatureScale);
    _scatteringResource->setCurvatureFactors(curvatureInfo);

    _scatteringResource->setLevel((float)config.enableScattering);
    _scatteringResource->setShowBRDF(config.showScatteringBRDF);
    _scatteringResource->setShowCurvature(config.showCurvature);
    _scatteringResource->setShowDiffusedNormal(config.showDiffusedNormal);
}

void SubsurfaceScattering::run(const render::RenderContextPointer& renderContext, Outputs& outputs) {
    assert(renderContext->args);
    assert(renderContext->args->hasViewFrustum());
    
    if (!_scatteringResource->getScatteringTable()) {
        _scatteringResource->generateScatteringTable(renderContext->args);
    }

    outputs = _scatteringResource;
}

#ifdef GENERATE_SCATTERING_RESOURCE_ON_CPU

// Reference: http://www.altdevblogaday.com/2011/12/31/skin-shading-in-unity3d/
#include <cstdio>
#include <cmath>
#include <algorithm>

#define _PI 3.14159265358979523846

using namespace std;

double gaussian(float v, float r) {
    double g = (1.0 / sqrt(2.0 * _PI * v)) * exp(-(r*r) / (2.0 * v));
    return g;
}

vec3 scatter(double r) {
    // Values from GPU Gems 3 "Advanced Skin Rendering".
    // Originally taken from real life samples.
    static const double profile[][4] = {
        { 0.0064, 0.233, 0.455, 0.649 },
        { 0.0484, 0.100, 0.336, 0.344 },
        { 0.1870, 0.118, 0.198, 0.000 },
        { 0.5670, 0.113, 0.007, 0.007 },
        { 1.9900, 0.358, 0.004, 0.000 },
        { 7.4100, 0.078, 0.000, 0.000 }
    };
    static const int profileNum = 6;
    vec3 ret(0.0);
    for (int i = 0; i < profileNum; i++) {
        double g = gaussian(profile[i][0] * 1.414f, r);
        ret.x += g * profile[i][1];
        ret.y += g * profile[i][2];
        ret.z += g * profile[i][3];
    }

    return ret;
}

vec3 integrate(double cosTheta, double skinRadius) {
    // Angle from lighting direction.
    double theta = acos(cosTheta);
    vec3 totalWeights(0.0);
    vec3 totalLight(0.0);
    vec3 skinColour(1.0);

    double a = -(_PI);

    double inc = 0.005;

    while (a <= (_PI)) {
        double sampleAngle = theta + a;
        double diffuse = cos(sampleAngle);
        if (diffuse < 0.0) diffuse = 0.0;
        if (diffuse > 1.0) diffuse = 1.0;

        // Distance.
        double sampleDist = abs(2.0 * skinRadius * sin(a * 0.5));

        // Profile Weight.
        vec3 weights = scatter(sampleDist);

        totalWeights += weights;
        totalLight.x += diffuse * weights.x * (skinColour.x * skinColour.x);
        totalLight.y += diffuse * weights.y * (skinColour.y * skinColour.y);
        totalLight.z += diffuse * weights.z * (skinColour.z * skinColour.z);
        a += inc;
    }

    vec3 result;
    result.x = totalLight.x / totalWeights.x;
    result.y = totalLight.y / totalWeights.y;
    result.z = totalLight.z / totalWeights.z;

    return result;
}

void diffuseScatter(gpu::TexturePointer& lut) {
    int width = lut->getWidth();
    int height = lut->getHeight();
    
    const int COMPONENT_COUNT = 4;
    std::vector<unsigned char> bytes(COMPONENT_COUNT * height * width);
    
    int index = 0;
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            // Lookup by: x: NDotL y: 1 / r
            float y = 2.0 * 1.0 / ((j + 1.0) / (double)height);
            float x = ((i / (double)width) * 2.0) - 1.0;
            vec3 val = integrate(x, y);

            // Convert to linear
           // val.x = sqrt(val.x);
           // val.y = sqrt(val.y);
           // val.z = sqrt(val.z);

            // Convert to 24-bit image.
            unsigned char valI[3];
            if (val.x > 1.0) val.x = 1.0;
            if (val.y > 1.0) val.y = 1.0;
            if (val.z > 1.0) val.z = 1.0;
            valI[0] = (unsigned char)(val.x * 256.0);
            valI[1] = (unsigned char)(val.y * 256.0);
            valI[2] = (unsigned char)(val.z * 256.0);
            
            bytes[COMPONENT_COUNT * index] = valI[0];
            bytes[COMPONENT_COUNT * index + 1] = valI[1];
            bytes[COMPONENT_COUNT * index + 2] = valI[2];
            bytes[COMPONENT_COUNT * index + 3] = 255.0;
            
            index++;
        }
    }

    lut->assignStoredMip(0, gpu::Element::COLOR_RGBA_32, bytes.size(), bytes.data());
}


void diffuseProfile(gpu::TexturePointer& profile) {
    int width = profile->getWidth();
    int height = profile->getHeight();
    
    const int COMPONENT_COUNT = 4;
    std::vector<unsigned char> bytes(COMPONENT_COUNT * height * width);

    int index = 0;
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            float y = (double)(i + 1.0) / (double)width;
            vec3 val = scatter(y * 2.0f);

            // Convert to 24-bit image.
            unsigned char valI[3];
            if (val.x > 1.0) val.x = 1.0;
            if (val.y > 1.0) val.y = 1.0;
            if (val.z > 1.0) val.z = 1.0;
            valI[0] = (unsigned char)(val.x * 255.0);
            valI[1] = (unsigned char)(val.y * 255.0);
            valI[2] = (unsigned char)(val.z * 255.0);

            bytes[COMPONENT_COUNT * index] = valI[0];
            bytes[COMPONENT_COUNT * index + 1] = valI[1];
            bytes[COMPONENT_COUNT * index + 2] = valI[2];
            bytes[COMPONENT_COUNT * index + 3] = 255.0;

            index++;
        }
    }

    
    profile->assignStoredMip(0, gpu::Element::COLOR_RGBA_32, bytes.size(), bytes.data());
}

#endif

void diffuseProfileGPU(gpu::TexturePointer& profileMap, RenderArgs* args) {
    int width = profileMap->getWidth();
    int height = profileMap->getHeight();

    gpu::PipelinePointer makePipeline;
    {
        gpu::ShaderPointer program = gpu::Shader::createProgram(shader::render_utils::program::subsurfaceScattering_makeProfile);

        gpu::StatePointer state = std::make_shared<gpu::State>();

        makePipeline = gpu::Pipeline::create(program, state);
    }

    auto makeFramebuffer = gpu::FramebufferPointer(gpu::Framebuffer::create("diffuseProfile"));
    makeFramebuffer->setRenderBuffer(0, profileMap);

    gpu::doInBatch("SubsurfaceScattering::diffuseProfileGPU", args->_context, [=](gpu::Batch& batch) {
        batch.enableStereo(false);

        batch.setViewportTransform(glm::ivec4(0, 0, width, height));

        batch.setFramebuffer(makeFramebuffer);
        batch.setPipeline(makePipeline);
        batch.draw(gpu::TRIANGLE_STRIP, 4);
        batch.setResourceTexture(0, nullptr);
        batch.setPipeline(nullptr);
        batch.setFramebuffer(nullptr);
    });
}


void diffuseScatterGPU(const gpu::TexturePointer& profileMap, gpu::TexturePointer& lut, RenderArgs* args) {
    int width = lut->getWidth();
    int height = lut->getHeight();

    gpu::ShaderPointer program = gpu::Shader::createProgram(shader::render_utils::program::subsurfaceScattering_makeLUT);

    gpu::StatePointer state = std::make_shared<gpu::State>();

    gpu::PipelinePointer makePipeline = gpu::Pipeline::create(program, state);
    
    auto makeFramebuffer = gpu::FramebufferPointer(gpu::Framebuffer::create("diffuseScatter"));
    makeFramebuffer->setRenderBuffer(0, lut);

    gpu::doInBatch("SubsurfaceScattering::diffuseScatterGPU", args->_context, [=](gpu::Batch& batch) {
        batch.enableStereo(false);
        batch.setViewportTransform(glm::ivec4(0, 0, width, height));
        batch.setFramebuffer(makeFramebuffer);
        batch.setPipeline(makePipeline);
        batch.setResourceTexture(0, profileMap);
        batch.draw(gpu::TRIANGLE_STRIP, 4);
        batch.setResourceTexture(0, nullptr);
        batch.setPipeline(nullptr);
        batch.setFramebuffer(nullptr);

    });
}

void computeSpecularBeckmannGPU(gpu::TexturePointer& beckmannMap, RenderArgs* args) {
    int width = beckmannMap->getWidth();
    int height = beckmannMap->getHeight();

    gpu::PipelinePointer makePipeline;
    {
        gpu::ShaderPointer program = gpu::Shader::createProgram(shader::render_utils::program::subsurfaceScattering_makeSpecularBeckmann);

        gpu::StatePointer state = std::make_shared<gpu::State>();

        makePipeline = gpu::Pipeline::create(program, state);
    }

    auto makeFramebuffer = gpu::FramebufferPointer(gpu::Framebuffer::create("computeSpecularBeckmann"));
    makeFramebuffer->setRenderBuffer(0, beckmannMap);

    gpu::doInBatch("SubsurfaceScattering::computeSpecularBeckmannGPU", args->_context, [=](gpu::Batch& batch) {
        batch.enableStereo(false);

        batch.setViewportTransform(glm::ivec4(0, 0, width, height));

        batch.setFramebuffer(makeFramebuffer);
        batch.setPipeline(makePipeline);
        batch.draw(gpu::TRIANGLE_STRIP, 4);
        batch.setResourceTexture(0, nullptr);
        batch.setPipeline(nullptr);
        batch.setFramebuffer(nullptr);
    });
}

gpu::TexturePointer SubsurfaceScatteringResource::generateScatteringProfile(RenderArgs* args) {
    const int PROFILE_RESOLUTION = 512;
    //  const auto pixelFormat = gpu::Element::COLOR_SRGBA_32;
    const auto pixelFormat = gpu::Element::COLOR_R11G11B10;
    auto profileMap = gpu::Texture::createRenderBuffer(pixelFormat, PROFILE_RESOLUTION, 1, gpu::Texture::SINGLE_MIP, gpu::Sampler(gpu::Sampler::FILTER_MIN_MAG_MIP_LINEAR, gpu::Sampler::WRAP_CLAMP));
    profileMap->setSource("Generated Scattering Profile");
    diffuseProfileGPU(profileMap, args);
    return profileMap;
}

gpu::TexturePointer SubsurfaceScatteringResource::generatePreIntegratedScattering(const gpu::TexturePointer& profile, RenderArgs* args) {

    const int TABLE_RESOLUTION = 512;
  //  const auto pixelFormat = gpu::Element::COLOR_SRGBA_32;
    const auto pixelFormat = gpu::Element::COLOR_R11G11B10;
    auto scatteringLUT = gpu::Texture::createRenderBuffer(pixelFormat, TABLE_RESOLUTION, TABLE_RESOLUTION, gpu::Texture::SINGLE_MIP, gpu::Sampler(gpu::Sampler::FILTER_MIN_MAG_MIP_LINEAR, gpu::Sampler::WRAP_CLAMP));
    //diffuseScatter(scatteringLUT);
    scatteringLUT->setSource("Generated pre-integrated scattering");
    diffuseScatterGPU(profile, scatteringLUT, args);
    return scatteringLUT;
}

gpu::TexturePointer SubsurfaceScatteringResource::generateScatteringSpecularBeckmann(RenderArgs* args) {
    const int SPECULAR_RESOLUTION = 256;
    auto beckmannMap = gpu::Texture::createRenderBuffer(gpu::Element::COLOR_RGBA_32, SPECULAR_RESOLUTION, SPECULAR_RESOLUTION, gpu::Texture::SINGLE_MIP, gpu::Sampler(gpu::Sampler::FILTER_MIN_MAG_MIP_LINEAR, gpu::Sampler::WRAP_CLAMP));
    beckmannMap->setSource("Generated beckmannMap");
    computeSpecularBeckmannGPU(beckmannMap, args);
    return beckmannMap;
}

DebugSubsurfaceScattering::DebugSubsurfaceScattering() {
}

void DebugSubsurfaceScattering::configure(const Config& config) {

    _showProfile = config.showProfile;
    _showLUT = config.showLUT;
    _showSpecularTable = config.showSpecularTable;
    _showCursorPixel = config.showCursorPixel;
    _debugCursorTexcoord = config.debugCursorTexcoord;
    if (!_debugParams) {
        _debugParams = std::make_shared<gpu::Buffer>(sizeof(glm::vec4), nullptr);
    }
    _debugParams->setSubData(0, _debugCursorTexcoord);
}



gpu::PipelinePointer DebugSubsurfaceScattering::getScatteringPipeline() {
    if (!_scatteringPipeline) {
        gpu::ShaderPointer program = gpu::Shader::createProgram(shader::render_utils::program::subsurfaceScattering_drawScattering);
        gpu::StatePointer state = std::make_shared<gpu::State>();

        _scatteringPipeline = gpu::Pipeline::create(program, state);
    }

    return _scatteringPipeline;
}

gpu::PipelinePointer _showLUTPipeline;

gpu::PipelinePointer DebugSubsurfaceScattering::getShowLUTPipeline() {
    if (!_showLUTPipeline) {
        gpu::ShaderPointer program = gpu::Shader::createProgram(shader::gpu::program::drawUnitQuatTextureOpaque);
        gpu::StatePointer state = std::make_shared<gpu::State>();
        _showLUTPipeline = gpu::Pipeline::create(program, state);
    }

    return _showLUTPipeline;
}


void DebugSubsurfaceScattering::run(const render::RenderContextPointer& renderContext, const Inputs& inputs) {
    assert(renderContext->args);
    assert(renderContext->args->hasViewFrustum());

    RenderArgs* args = renderContext->args;


    auto& frameTransform = inputs.get0();
    auto& deferredFramebuffer = inputs.get1();

    auto& surfaceGeometryFramebuffer = inputs.get3();
    auto curvatureFramebuffer = surfaceGeometryFramebuffer->getCurvatureFramebuffer();
    auto linearDepthTexture = surfaceGeometryFramebuffer->getLinearDepthTexture();

    auto& diffusedFramebuffer = inputs.get4();
    auto& scatteringResource = inputs.get5();

    if (!scatteringResource) {
        return;
    }
    auto scatteringProfile = scatteringResource->getScatteringProfile();
    auto scatteringTable = scatteringResource->getScatteringTable();
    auto scatteringSpecular = scatteringResource->getScatteringSpecular();



    auto lightStage = renderContext->_scene->getStage<LightStage>();
    assert(lightStage);
   // const auto light = DependencyManager::get<DeferredLightingEffect>()->getLightStage()->getLight(0);
    const auto light = lightStage->getLight(0);
    if (!_debugParams) {
        _debugParams = std::make_shared<gpu::Buffer>(sizeof(glm::vec4), nullptr);
        _debugParams->setSubData(0, _debugCursorTexcoord);
    }
    
    gpu::doInBatch("DebugSubsurfaceScattering::run", args->_context, [=](gpu::Batch& batch) {
        batch.enableStereo(false);


        auto viewportSize = std::min(args->_viewport.z, args->_viewport.w) >> 1;
        auto offsetViewport = viewportSize * 0.1;

        if (_showProfile) {
            batch.setViewportTransform(glm::ivec4(0, 0, viewportSize, offsetViewport));
            batch.setPipeline(getShowLUTPipeline());
            batch.setResourceTexture(0, scatteringProfile);
            batch.draw(gpu::TRIANGLE_STRIP, 4);
        }

        if (_showLUT) {
            batch.setViewportTransform(glm::ivec4(0, offsetViewport * 1.5, viewportSize, viewportSize));
            batch.setPipeline(getShowLUTPipeline());
            batch.setResourceTexture(0, scatteringTable);
            batch.draw(gpu::TRIANGLE_STRIP, 4);

            if (_showCursorPixel) {

                auto debugScatteringPipeline = getScatteringPipeline();
                batch.setPipeline(debugScatteringPipeline);

                Transform model;
                model.setTranslation(glm::vec3(0.0, offsetViewport * 1.5 / args->_viewport.w, 0.0));
                model.setScale(glm::vec3(viewportSize / (float)args->_viewport.z, viewportSize / (float)args->_viewport.w, 1.0));
                batch.setModelTransform(model);

                batch.setUniformBuffer(ru::Buffer::DeferredFrameTransform, frameTransform->getFrameTransformBuffer());
                batch.setUniformBuffer(ru::Buffer::SsscParams, scatteringResource->getParametersBuffer());
                if (light) {
                    batch.setUniformBuffer(gr::Buffer::Light, light->getLightSchemaBuffer());
                }
                batch.setResourceTexture(ru::Texture::SsscLut, scatteringTable);
                batch.setResourceTexture(ru::Texture::DeferredCurvature, curvatureFramebuffer->getRenderBuffer(0));
                batch.setResourceTexture(ru::Texture::DeferredDiffusedCurvature, diffusedFramebuffer->getRenderBuffer(0));
                batch.setResourceTexture(ru::Texture::DeferredNormal, deferredFramebuffer->getDeferredNormalTexture());
                batch.setResourceTexture(ru::Texture::DeferredColor, deferredFramebuffer->getDeferredColorTexture());
               	batch.setResourceTexture(ru::Texture::DeferredDepth, linearDepthTexture);
                batch.setUniformBuffer(1, _debugParams);
                batch.draw(gpu::TRIANGLE_STRIP, 4);
            }
        }

        if (_showSpecularTable) {
            batch.setViewportTransform(glm::ivec4(viewportSize + offsetViewport * 0.5, 0, viewportSize * 0.5, viewportSize * 0.5));
            batch.setPipeline(getShowLUTPipeline());
            batch.setResourceTexture(0, scatteringSpecular);
            batch.draw(gpu::TRIANGLE_STRIP, 4);
        }

        batch.setViewportTransform(args->_viewport);

    });
}
