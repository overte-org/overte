
//
//  RenderDeferredTask.cpp
//  render-utils/src/
//
//  Created by Sam Gateau on 5/29/15.
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "RenderDeferredTask.h"

#include <QtCore/qglobal.h>

#include <DependencyManager.h>

#include <PerfStat.h>
#include <PathUtils.h>
#include <ViewFrustum.h>
#include <gpu/Context.h>
#include <graphics/ShaderConstants.h>

#include <render/CullTask.h>
#include <render/FilterTask.h>
#include <render/SortTask.h>
#include <render/DrawTask.h>
#include <render/DrawStatus.h>
#include <render/DrawSceneOctree.h>
#include <render/BlurTask.h>

#include "RenderHifi.h"
#include "render-utils/ShaderConstants.h"
#include "RenderCommonTask.h"
#include "LightingModel.h"
#include "StencilMaskPass.h"
#include "DebugDeferredBuffer.h"
#include "DeferredFramebuffer.h"
#include "DeferredLightingEffect.h"
#include "SurfaceGeometryPass.h"
#include "VelocityBufferPass.h"
#include "FramebufferCache.h"
#include "TextureCache.h"
#include "ZoneRenderer.h"
#include "FadeEffect.h"
#include "BloomStage.h"
#include "RenderUtilsLogging.h"
#include "RenderHUDLayerTask.h"

#include "AmbientOcclusionEffect.h"
#include "AntialiasingEffect.h"
#include "ToneMapAndResampleTask.h"
#include "SubsurfaceScattering.h"
#include "DrawHaze.h"
#include "BloomEffect.h"
#include "HighlightEffect.h"

#include <sstream>

using namespace render;
extern void initDeferredPipelines(render::ShapePlumber& plumber, const render::ShapePipeline::BatchSetter& batchSetter, const render::ShapePipeline::ItemSetter& itemSetter);

namespace ru {
    using render_utils::slot::texture::Texture;
    using render_utils::slot::buffer::Buffer;
}

namespace gr {
    using graphics::slot::texture::Texture;
    using graphics::slot::buffer::Buffer;
}


class RenderDeferredTaskDebug {
public:
    using ExtraBuffers = render::VaryingSet6<LinearDepthFramebufferPointer, SurfaceGeometryFramebufferPointer, AmbientOcclusionFramebufferPointer, gpu::BufferView, SubsurfaceScatteringResourcePointer, VelocityFramebufferPointer>;
    using Input = render::VaryingSet9<RenderFetchCullSortTask::Output, RenderShadowTask::Output,
        AssembleLightingStageTask::Output, LightClusteringPass::Output, 
        PrepareDeferred::Outputs, ExtraBuffers, GenerateDeferredFrameTransform::Output,
        JitterSample::Output, LightingModel>;

    using JobModel = render::Task::ModelI<RenderDeferredTaskDebug, Input>;

    RenderDeferredTaskDebug();

    void build(JobModel& task, const render::Varying& inputs, render::Varying& outputs);
private:
};


RenderDeferredTask::RenderDeferredTask()
{
}

void RenderDeferredTask::configure(const Config& config) {
    // Propagate resolution scale to sub jobs who need it
    auto preparePrimaryBufferConfig = config.getConfig<PreparePrimaryFramebuffer>("PreparePrimaryBufferDeferred");
    assert(preparePrimaryBufferConfig);
    preparePrimaryBufferConfig->setResolutionScale(config.resolutionScale);
}

void RenderDeferredTask::build(JobModel& task, const render::Varying& input, render::Varying& output, render::CullFunctor cullFunctor, size_t depth) {
    static auto fadeEffect = DependencyManager::get<FadeEffect>();
    // Prepare the ShapePipelines
    static ShapePlumberPointer shapePlumber = std::make_shared<ShapePlumber>();
    static std::once_flag once;
    std::call_once(once, [] {
        initDeferredPipelines(*shapePlumber, fadeEffect->getBatchSetter(), fadeEffect->getItemUniformSetter());
    });

    const auto& inputs = input.get<Input>();
    
    // Separate the fetched items
    const auto& fetchedItems = inputs.get0();

        const auto& items = fetchedItems.get0();

            // Extract opaques / transparents / lights / metas / layered / background
            const auto& opaques = items[RenderFetchCullSortTask::OPAQUE_SHAPE];
            const auto& transparents = items[RenderFetchCullSortTask::TRANSPARENT_SHAPE];
            const auto& mirrors = items[RenderFetchCullSortTask::MIRROR];
            const auto& inFrontOpaque = items[RenderFetchCullSortTask::LAYER_FRONT_OPAQUE_SHAPE];
            const auto& inFrontTransparent = items[RenderFetchCullSortTask::LAYER_FRONT_TRANSPARENT_SHAPE];
            const auto& hudOpaque = items[RenderFetchCullSortTask::LAYER_HUD_OPAQUE_SHAPE];
            const auto& hudTransparent = items[RenderFetchCullSortTask::LAYER_HUD_TRANSPARENT_SHAPE];

    // Lighting model comes next, the big configuration of the view
    const auto& lightingModel = inputs[1];

    // Extract the Lighting Stages Current frame ( and zones)
    const auto& lightingStageInputs = inputs.get2();
        // Fetch the current frame stacks from all the stages
        const auto currentStageFrames = lightingStageInputs.get0();
            const auto lightFrame = currentStageFrames[0];
            const auto backgroundFrame = currentStageFrames[1];
            const auto& hazeFrame = currentStageFrames[2];
            const auto& bloomFrame = currentStageFrames[3];

    // Shadow Task Outputs
    const auto& shadowTaskOutputs = inputs.get3();

        // Shadow Stage Frame
        const auto shadowFrame = shadowTaskOutputs[1];

    fadeEffect->build(task, opaques);

    const auto jitter = task.addJob<JitterSample>("JitterCam");

    // GPU jobs: Start preparing the primary, deferred and lighting buffer
    const auto scaledPrimaryFramebuffer = task.addJob<PreparePrimaryFramebuffer>("PreparePrimaryBufferDeferred");

    // Prepare deferred, generate the shared Deferred Frame Transform. Only valid with the scaled frame buffer
    const auto deferredFrameTransform = task.addJob<GenerateDeferredFrameTransform>("DeferredFrameTransform", jitter);

    const auto prepareDeferredInputs = PrepareDeferred::Inputs(scaledPrimaryFramebuffer, lightingModel).asVarying();
    const auto prepareDeferredOutputs = task.addJob<PrepareDeferred>("PrepareDeferred", prepareDeferredInputs);
    const auto deferredFramebuffer = prepareDeferredOutputs.getN<PrepareDeferred::Outputs>(0);
    const auto lightingFramebuffer = prepareDeferredOutputs.getN<PrepareDeferred::Outputs>(1);

    // draw a stencil mask in hidden regions of the framebuffer.
    task.addJob<PrepareStencil>("PrepareStencil", scaledPrimaryFramebuffer);

    // Render opaque objects in DeferredBuffer
    const auto opaqueInputs = DrawStateSortDeferred::Inputs(opaques, lightingModel, jitter).asVarying();
    task.addJob<DrawStateSortDeferred>("DrawOpaqueDeferred", opaqueInputs, shapePlumber);

    if (depth < RenderMirrorTask::MAX_MIRROR_DEPTH) {
        const auto mirrorInputs = RenderMirrorTask::Inputs(mirrors, scaledPrimaryFramebuffer, jitter).asVarying();
        for (size_t i = 0; i < RenderMirrorTask::MAX_MIRRORS_PER_LEVEL; i++) {
            task.addJob<RenderMirrorTask>("RenderMirrorTask" + std::to_string(i) + "Depth" + std::to_string(depth), mirrorInputs, i, cullFunctor, depth);
        }
    }

    // Opaque all rendered

    // Linear Depth Pass
    const auto linearDepthPassInputs = LinearDepthPass::Inputs(deferredFrameTransform, deferredFramebuffer).asVarying();
    const auto linearDepthPassOutputs = task.addJob<LinearDepthPass>("LinearDepth", linearDepthPassInputs);
    const auto linearDepthTarget = linearDepthPassOutputs.getN<LinearDepthPass::Outputs>(0);
    
    // Curvature pass
    const auto surfaceGeometryPassInputs = SurfaceGeometryPass::Inputs(deferredFrameTransform, deferredFramebuffer, linearDepthTarget).asVarying();
    const auto surfaceGeometryPassOutputs = task.addJob<SurfaceGeometryPass>("SurfaceGeometry", surfaceGeometryPassInputs);
    const auto surfaceGeometryFramebuffer = surfaceGeometryPassOutputs.getN<SurfaceGeometryPass::Outputs>(0);
    const auto curvatureFramebuffer = surfaceGeometryPassOutputs.getN<SurfaceGeometryPass::Outputs>(1);
    const auto midCurvatureNormalFramebuffer = surfaceGeometryPassOutputs.getN<SurfaceGeometryPass::Outputs>(2);
    const auto lowCurvatureNormalFramebuffer = surfaceGeometryPassOutputs.getN<SurfaceGeometryPass::Outputs>(3);

    // Simply update the scattering resource
    const auto scatteringResource = task.addJob<SubsurfaceScattering>("Scattering");

    // AO job
    const auto ambientOcclusionInputs = AmbientOcclusionEffect::Input(lightingModel, deferredFrameTransform, deferredFramebuffer, linearDepthTarget).asVarying();
    const auto ambientOcclusionOutputs = task.addJob<AmbientOcclusionEffect>("AmbientOcclusion", ambientOcclusionInputs);
    const auto ambientOcclusionFramebuffer = ambientOcclusionOutputs.getN<AmbientOcclusionEffect::Output>(0);
    const auto ambientOcclusionUniforms = ambientOcclusionOutputs.getN<AmbientOcclusionEffect::Output>(1);

    // Velocity
    const auto velocityBufferInputs = VelocityBufferPass::Inputs(deferredFrameTransform, deferredFramebuffer).asVarying();
    const auto velocityBufferOutputs = task.addJob<VelocityBufferPass>("VelocityBuffer", velocityBufferInputs);
    const auto velocityBuffer = velocityBufferOutputs.getN<VelocityBufferPass::Outputs>(0);

    // Light Clustering
    // Create the cluster grid of lights, cpu job for now
    const auto lightClusteringPassInputs = LightClusteringPass::Input(deferredFrameTransform, lightingModel, lightFrame, linearDepthTarget).asVarying();
    const auto lightClusters = task.addJob<LightClusteringPass>("LightClustering", lightClusteringPassInputs);

    // DeferredBuffer is complete, now let's shade it into the LightingBuffer
    const auto extraDeferredBuffer = RenderDeferred::ExtraDeferredBuffer(surfaceGeometryFramebuffer, ambientOcclusionFramebuffer, scatteringResource).asVarying();
    const auto deferredLightingInputs = RenderDeferred::Inputs(deferredFrameTransform, deferredFramebuffer, extraDeferredBuffer, lightingModel, lightClusters, lightFrame, shadowFrame, hazeFrame).asVarying();
    task.addJob<RenderDeferred>("RenderDeferred", deferredLightingInputs);

    // Similar to light stage, background stage has been filled by several potential render items and resolved for the frame in this job
    const auto backgroundInputs = DrawBackgroundStage::Inputs(lightingModel, backgroundFrame, hazeFrame).asVarying();
    task.addJob<DrawBackgroundStage>("DrawBackgroundDeferred", backgroundInputs);

    const auto drawHazeInputs = render::Varying(DrawHaze::Inputs(hazeFrame, lightingFramebuffer, linearDepthTarget, deferredFrameTransform, lightingModel, lightFrame));
    task.addJob<DrawHaze>("DrawHazeDeferred", drawHazeInputs);

    // Render transparent objects forward in LightingBuffer
    const auto transparentsInputs = RenderTransparentDeferred::Inputs(transparents, hazeFrame, lightFrame, lightingModel, lightClusters, shadowFrame, jitter).asVarying();
    task.addJob<RenderTransparentDeferred>("DrawTransparentDeferred", transparentsInputs, shapePlumber);

    // Highlight 
    const auto outlineInputs = DrawHighlightTask::Inputs(items, deferredFramebuffer, lightingFramebuffer, deferredFrameTransform, jitter).asVarying();
    task.addJob<DrawHighlightTask>("DrawHighlight", outlineInputs);

    // Layered Over (in front)
    const auto inFrontOpaquesInputs = DrawLayered3D::Inputs(inFrontOpaque, lightingModel, hazeFrame, jitter).asVarying();
    const auto inFrontTransparentsInputs = DrawLayered3D::Inputs(inFrontTransparent, lightingModel, hazeFrame, jitter).asVarying();
    task.addJob<DrawLayered3D>("DrawInFrontOpaque", inFrontOpaquesInputs, true);
    task.addJob<DrawLayered3D>("DrawInFrontTransparent", inFrontTransparentsInputs, false);

    // AA job before bloom to limit flickering
    const auto antialiasingInputs = Antialiasing::Inputs(deferredFrameTransform, lightingFramebuffer, linearDepthTarget, velocityBuffer).asVarying();
    task.addJob<Antialiasing>("Antialiasing", antialiasingInputs);

    // Add bloom
    const auto bloomInputs = BloomEffect::Inputs(deferredFrameTransform, lightingFramebuffer, bloomFrame, lightingModel).asVarying();
    task.addJob<BloomEffect>("Bloom", bloomInputs);

    const auto destFramebuffer = static_cast<gpu::FramebufferPointer>(nullptr);

    // Lighting Buffer ready for tone mapping
    const auto toneMappingInputs = ToneMapAndResample::Input(lightingFramebuffer, destFramebuffer).asVarying();
    const auto toneMappedBuffer = task.addJob<ToneMapAndResample>("ToneMapping", toneMappingInputs);

    // Debugging task is happening in the "over" layer after tone mapping and just before HUD
    { // Debug the bounds of the rendered items, still look at the zbuffer
        const auto extraDebugBuffers = RenderDeferredTaskDebug::ExtraBuffers(linearDepthTarget, surfaceGeometryFramebuffer, ambientOcclusionFramebuffer, ambientOcclusionUniforms, scatteringResource, velocityBuffer);
        const auto debugInputs = RenderDeferredTaskDebug::Input(fetchedItems, shadowTaskOutputs, lightingStageInputs, lightClusters, prepareDeferredOutputs, extraDebugBuffers,
            deferredFrameTransform, jitter, lightingModel).asVarying();
        task.addJob<RenderDeferredTaskDebug>("DebugRenderDeferredTask", debugInputs);
    }

    // HUD Layer
    const auto renderHUDLayerInputs = RenderHUDLayerTask::Input(toneMappedBuffer, lightingModel, hudOpaque, hudTransparent, hazeFrame).asVarying();
    task.addJob<RenderHUDLayerTask>("RenderHUDLayer", renderHUDLayerInputs);
}

RenderDeferredTaskDebug::RenderDeferredTaskDebug() {
}

void RenderDeferredTaskDebug::build(JobModel& task, const render::Varying& input, render::Varying& outputs) {

    const auto& inputs = input.get<Input>();

    // RenderFetchCullSortTask out
    const auto& fetchCullSortTaskOut = inputs.get0();
        const auto& items = fetchCullSortTaskOut.get0();

            // Extract opaques / transparents / lights / metas / layered / background
            const auto& opaques = items[RenderFetchCullSortTask::OPAQUE_SHAPE];
            const auto& transparents = items[RenderFetchCullSortTask::TRANSPARENT_SHAPE];
            const auto& lights = items[RenderFetchCullSortTask::LIGHT];
            const auto& metas = items[RenderFetchCullSortTask::META];
            const auto& inFrontOpaque = items[RenderFetchCullSortTask::LAYER_FRONT_OPAQUE_SHAPE];
            const auto& inFrontTransparent = items[RenderFetchCullSortTask::LAYER_FRONT_TRANSPARENT_SHAPE];
            const auto& hudOpaque = items[RenderFetchCullSortTask::LAYER_HUD_OPAQUE_SHAPE];
            const auto& hudTransparent = items[RenderFetchCullSortTask::LAYER_HUD_TRANSPARENT_SHAPE];

        const auto& spatialSelection = fetchCullSortTaskOut[1];

    // RenderShadowTask out
    const auto& shadowOut = inputs.get1();

    const auto& renderShadowTaskOut = shadowOut[0];
    const auto& shadowFrame = shadowOut[1];

    // Extract the Lighting Stages Current frame ( and zones)
    const auto lightingStageInputs = inputs.get2();
        // Fetch the current frame stacks from all the stages
        const auto stageCurrentFrames = lightingStageInputs.get0();
            const auto lightFrame = stageCurrentFrames[0];
            const auto backgroundFrame = stageCurrentFrames[1];
            const auto hazeFrame = stageCurrentFrames[2];
            const auto bloomFrame = stageCurrentFrames[3];

        // Zones
        const auto& zones = lightingStageInputs[1];

    // Light CLuster
    const auto& lightClusters = inputs[3];

    // PrepareDeferred out
    const auto& prepareDeferredOutputs = inputs.get4();
        const auto& deferredFramebuffer = prepareDeferredOutputs[0];

    // extraDeferredBuffer 
    const auto& extraDeferredBuffer = inputs.get5();
        const auto& linearDepthTarget = extraDeferredBuffer[0];
        const auto& surfaceGeometryFramebuffer = extraDeferredBuffer[1];
        const auto& ambientOcclusionFramebuffer = extraDeferredBuffer[2];
        const auto& ambientOcclusionUniforms = extraDeferredBuffer[3];
        const auto& scatteringResource = extraDeferredBuffer[4];
        const auto& velocityBuffer = extraDeferredBuffer[5];

    // GenerateDeferredFrameTransform out
    const auto& deferredFrameTransform = inputs[6];

    // Jitter out
    const auto& jitter = inputs[7];

    // Lighting Model out
    const auto& lightingModel = inputs[8];



    // Light Cluster Grid Debuging job
    {
        const auto debugLightClustersInputs = DebugLightClusters::Inputs(deferredFrameTransform, lightingModel, linearDepthTarget, lightClusters).asVarying();
        task.addJob<DebugLightClusters>("DebugLightClusters", debugLightClustersInputs);
    }


    { // Debug the bounds of the rendered items, still look at the zbuffer
        task.addJob<DrawBounds>("DrawMetaBounds", metas);
        task.addJob<DrawBounds>("DrawOpaqueBounds", opaques);
        task.addJob<DrawBounds>("DrawTransparentBounds", transparents);

        task.addJob<DrawBounds>("DrawLightBounds", lights);
        task.addJob<DrawBounds>("DrawZones", zones);
        const auto frustums = task.addJob<ExtractFrustums>("ExtractFrustums", shadowFrame);
        const auto viewFrustum = frustums.getN<ExtractFrustums::Outputs>(ExtractFrustums::VIEW_FRUSTUM);
        task.addJob<DrawFrustum>("DrawViewFrustum", viewFrustum, glm::vec3(0.0f, 1.0f, 0.0f));
        for (auto i = 0; i < ExtractFrustums::SHADOW_CASCADE_FRUSTUM_COUNT; i++) {
            const auto shadowFrustum = frustums.getN<ExtractFrustums::Outputs>(ExtractFrustums::SHADOW_CASCADE0_FRUSTUM + i);
            float tint = 1.0f - i / float(ExtractFrustums::SHADOW_CASCADE_FRUSTUM_COUNT - 1);
            char jobName[64];
            sprintf(jobName, "DrawShadowFrustum%d", i);
            task.addJob<DrawFrustum>(jobName, shadowFrustum, glm::vec3(0.0f, tint, 1.0f));
            if (!renderShadowTaskOut.isNull()) {
                const auto& shadowCascadeSceneBBoxes = renderShadowTaskOut.get<RenderShadowTask::CascadeBoxes>();
                const auto shadowBBox = shadowCascadeSceneBBoxes[ExtractFrustums::SHADOW_CASCADE0_FRUSTUM + i];
                sprintf(jobName, "DrawShadowBBox%d", i);
                task.addJob<DrawAABox>(jobName, shadowBBox, glm::vec3(1.0f, tint, 0.0f));
            }
        }
    }

    { // Debug Selection... 
      // TODO: It s busted
        // Select items that need to be outlined and show them
        const auto selectionBaseName = "contextOverlayHighlightList";

        const auto selectMetaInput = SelectItems::Inputs(metas, Varying(), std::string()).asVarying();
        const auto selectedMetas = task.addJob<SelectItems>("MetaSelection", selectMetaInput, selectionBaseName);
        const auto selectMetaAndOpaqueInput = SelectItems::Inputs(opaques, selectedMetas, std::string()).asVarying();
        const auto selectedMetasAndOpaques = task.addJob<SelectItems>("OpaqueSelection", selectMetaAndOpaqueInput, selectionBaseName);
        const auto selectItemInput = SelectItems::Inputs(transparents, selectedMetasAndOpaques, std::string()).asVarying();
        const auto selectedItems = task.addJob<SelectItems>("TransparentSelection", selectItemInput, selectionBaseName);

        // Render.getConfig("RenderMainView.DrawSelectionBounds").enabled = true
        task.addJob<DrawBounds>("DrawSelectionBounds", selectedItems);
    }

    { // Debug the bounds of the layered objects, still look at the zbuffer
        task.addJob<DrawBounds>("DrawInFrontOpaqueBounds", inFrontOpaque);
        task.addJob<DrawBounds>("DrawInFrontTransparentBounds", inFrontTransparent);
    }

    { // Debug the bounds of the layered objects, still look at the zbuffer
        task.addJob<DrawBounds>("DrawHUDOpaqueBounds", hudOpaque);
        task.addJob<DrawBounds>("DrawHUDTransparentBounds", hudTransparent);
    }

    // Debugging stages
    {

        // Debugging Deferred buffer job
        const auto debugFramebuffers = DebugDeferredBuffer::Inputs(deferredFramebuffer, linearDepthTarget, surfaceGeometryFramebuffer, ambientOcclusionFramebuffer, velocityBuffer, deferredFrameTransform, shadowFrame).asVarying();
        task.addJob<DebugDeferredBuffer>("DebugDeferredBuffer", debugFramebuffers);

        const auto debugSubsurfaceScatteringInputs = DebugSubsurfaceScattering::Inputs(deferredFrameTransform, deferredFramebuffer, lightingModel,
            surfaceGeometryFramebuffer, ambientOcclusionFramebuffer, scatteringResource).asVarying();
        task.addJob<DebugSubsurfaceScattering>("DebugScattering", debugSubsurfaceScatteringInputs);

        const auto debugAmbientOcclusionInputs = DebugAmbientOcclusion::Inputs(deferredFrameTransform, deferredFramebuffer, linearDepthTarget, ambientOcclusionUniforms).asVarying();
        task.addJob<DebugAmbientOcclusion>("DebugAmbientOcclusion", debugAmbientOcclusionInputs);

        // Scene Octree Debugging job
        {
            task.addJob<DrawSceneOctree>("DrawSceneOctree", spatialSelection);
            task.addJob<DrawItemSelection>("DrawItemSelection", spatialSelection);
        }

        // Status icon rendering job
        {
            // Grab a texture map representing the different status icons and assign that to the drawStatusJob
            static gpu::TexturePointer statusIconMap;
            static std::once_flag once;
            std::call_once(once, [] {
                auto iconMapPath = PathUtils::resourcesPath() + "icons/statusIconAtlas.svg";
                statusIconMap = DependencyManager::get<TextureCache>()->getImageTexture(iconMapPath, image::TextureUsage::STRICT_TEXTURE);
            });
            const auto drawStatusInputs = DrawStatus::Input(opaques, jitter).asVarying();
            task.addJob<DrawStatus>("DrawStatus", drawStatusInputs, DrawStatus(statusIconMap));
        }

        const auto debugZoneInputs = DebugZoneLighting::Inputs(deferredFrameTransform, lightFrame, backgroundFrame).asVarying();
        task.addJob<DebugZoneLighting>("DrawZoneStack", debugZoneInputs);
    }
}

gpu::FramebufferPointer PreparePrimaryFramebuffer::createFramebuffer(const char* name, const glm::uvec2& frameSize) {
    gpu::FramebufferPointer framebuffer = gpu::FramebufferPointer(gpu::Framebuffer::create(name));
    auto colorFormat = gpu::Element::COLOR_SRGBA_32;

    auto defaultSampler = gpu::Sampler(gpu::Sampler::FILTER_MIN_MAG_LINEAR);
    auto primaryColorTexture = gpu::Texture::createRenderBuffer(colorFormat, frameSize.x, frameSize.y, gpu::Texture::SINGLE_MIP, defaultSampler);

    framebuffer->setRenderBuffer(0, primaryColorTexture);

    auto depthFormat = gpu::Element(gpu::SCALAR, gpu::UINT32, gpu::DEPTH_STENCIL); // Depth24_Stencil8 texel format
    auto primaryDepthTexture = gpu::Texture::createRenderBuffer(depthFormat, frameSize.x, frameSize.y, gpu::Texture::SINGLE_MIP, defaultSampler);

    framebuffer->setDepthStencilBuffer(primaryDepthTexture, depthFormat);

    return framebuffer;
}

void PreparePrimaryFramebuffer::configure(const Config& config) {
    _resolutionScale = config.getResolutionScale();
}

void PreparePrimaryFramebuffer::run(const RenderContextPointer& renderContext, Output& primaryFramebuffer) {
    glm::uvec2 frameSize(renderContext->args->_viewport.z, renderContext->args->_viewport.w);
    glm::uvec2 scaledFrameSize(glm::vec2(frameSize) * _resolutionScale);

    // Resizing framebuffers instead of re-building them seems to cause issues with threaded 
    // rendering
    if (!_primaryFramebuffer || _primaryFramebuffer->getSize() != scaledFrameSize) {
        _primaryFramebuffer = createFramebuffer("deferredPrimary", scaledFrameSize);
    }

    primaryFramebuffer = _primaryFramebuffer;

    // Set viewport for the rest of the scaled passes
    renderContext->args->_viewport.z = scaledFrameSize.x;
    renderContext->args->_viewport.w = scaledFrameSize.y;
}


void RenderTransparentDeferred::run(const RenderContextPointer& renderContext, const Inputs& inputs) {
    assert(renderContext->args);
    assert(renderContext->args->hasViewFrustum());

    auto config = std::static_pointer_cast<Config>(renderContext->jobConfig);

    const auto& inItems = inputs.get0();
    const auto& hazeFrame = inputs.get1();
    const auto& lightFrame = inputs.get2();
    const auto& lightingModel = inputs.get3();
    const auto& lightClusters = inputs.get4();
    // Not needed yet: const auto& shadowFrame = inputs.get5();
    const auto jitter = inputs.get6();
    auto deferredLightingEffect = DependencyManager::get<DeferredLightingEffect>();

    RenderArgs* args = renderContext->args;

    gpu::doInBatch("RenderTransparentDeferred::run", args->_context, [&](gpu::Batch& batch) {
        args->_batch = &batch;
        
        // Setup camera, projection and viewport for all items
        batch.setViewportTransform(args->_viewport);
        batch.setStateScissorRect(args->_viewport);

        glm::mat4 projMat;
        Transform viewMat;
        args->getViewFrustum().evalProjectionMatrix(projMat);
        args->getViewFrustum().evalViewTransform(viewMat);

        batch.setProjectionTransform(projMat);
        batch.setProjectionJitter(jitter.x, jitter.y);
        batch.setViewTransform(viewMat);

        // Setup lighting model for all items;
        batch.setUniformBuffer(ru::Buffer::LightModel, lightingModel->getParametersBuffer());
        batch.setResourceTexture(ru::Texture::AmbientFresnel, lightingModel->getAmbientFresnelLUT());

        // Set the light
        deferredLightingEffect->setupKeyLightBatch(args, batch, *lightFrame);
        deferredLightingEffect->setupLocalLightsBatch(batch, lightClusters);

        // Setup haze if current zone has haze
        const auto& hazeStage = args->_scene->getStage<HazeStage>();
        if (hazeStage && hazeFrame->_hazes.size() > 0) {
            const auto& hazePointer = hazeStage->getHaze(hazeFrame->_hazes.front());
            if (hazePointer) {
                batch.setUniformBuffer(graphics::slot::buffer::Buffer::HazeParams, hazePointer->getHazeParametersBuffer());
            }
        }

        // From the lighting model define a global shapKey ORED with individiual keys
        ShapeKey::Builder keyBuilder;
        if (lightingModel->isWireframeEnabled()) {
            keyBuilder.withWireframe();
        }

        ShapeKey globalKey = keyBuilder.build();
        args->_globalShapeKey = globalKey._flags.to_ulong();

        renderShapes(renderContext, _shapePlumber, inItems, _maxDrawn, globalKey);

        args->_batch = nullptr;
        args->_globalShapeKey = 0;

        deferredLightingEffect->unsetLocalLightsBatch(batch);
        deferredLightingEffect->unsetKeyLightBatch(batch);
    });

    config->setNumDrawn((int)inItems.size());
}

void DrawStateSortDeferred::run(const RenderContextPointer& renderContext, const Inputs& inputs) {
    assert(renderContext->args);
    assert(renderContext->args->hasViewFrustum());

    auto config = std::static_pointer_cast<Config>(renderContext->jobConfig);

    const auto& inItems = inputs.get0();
    const auto& lightingModel = inputs.get1();
    const auto jitter = inputs.get2();

    RenderArgs* args = renderContext->args;

    gpu::doInBatch("DrawStateSortDeferred::run", args->_context, [&](gpu::Batch& batch) {
        args->_batch = &batch;

        // Setup camera, projection and viewport for all items
        batch.setViewportTransform(args->_viewport);
        batch.setStateScissorRect(args->_viewport);

        glm::mat4 projMat;
        Transform viewMat;
        args->getViewFrustum().evalProjectionMatrix(projMat);
        args->getViewFrustum().evalViewTransform(viewMat);

        batch.setProjectionTransform(projMat);
        batch.setProjectionJitter(jitter.x, jitter.y);
        batch.setViewTransform(viewMat);

        // Setup lighting model for all items;
        batch.setUniformBuffer(ru::Buffer::LightModel, lightingModel->getParametersBuffer());
        batch.setResourceTexture(ru::Texture::AmbientFresnel, lightingModel->getAmbientFresnelLUT());

        // From the lighting model define a global shapeKey ORED with individiual keys
        ShapeKey::Builder keyBuilder;
        if (lightingModel->isWireframeEnabled()) {
            keyBuilder.withWireframe();
        }

        ShapeKey globalKey = keyBuilder.build();
        args->_globalShapeKey = globalKey._flags.to_ulong();

        if (_stateSort) {
            renderStateSortShapes(renderContext, _shapePlumber, inItems, _maxDrawn, globalKey);
        } else {
            renderShapes(renderContext, _shapePlumber, inItems, _maxDrawn, globalKey);
        }
        args->_batch = nullptr;
        args->_globalShapeKey = 0;
    });

    config->setNumDrawn((int)inItems.size());
}

void SetSeparateDeferredDepthBuffer::run(const render::RenderContextPointer& renderContext, const Inputs& inputs) {
    assert(renderContext->args);

    const auto deferredFramebuffer = inputs->getDeferredFramebuffer();
    const auto frameSize = deferredFramebuffer->getSize();
    const auto renderbufferCount = deferredFramebuffer->getNumRenderBuffers();

    if (!_framebuffer || _framebuffer->getSize() != frameSize || _framebuffer->getNumRenderBuffers() != renderbufferCount) {
        auto depthFormat = deferredFramebuffer->getDepthStencilBufferFormat();
        auto depthStencilTexture = gpu::TexturePointer(gpu::Texture::createRenderBuffer(depthFormat, frameSize.x, frameSize.y));
        _framebuffer = gpu::FramebufferPointer(gpu::Framebuffer::create("deferredFramebufferSeparateDepth"));
        _framebuffer->setDepthStencilBuffer(depthStencilTexture, depthFormat);
        for (decltype(deferredFramebuffer->getNumRenderBuffers()) i = 0; i < renderbufferCount; i++) {
            _framebuffer->setRenderBuffer(i, deferredFramebuffer->getRenderBuffer(i));
        }
    }

    RenderArgs* args = renderContext->args;
    gpu::doInBatch("SetSeparateDeferredDepthBuffer::run", args->_context, [this](gpu::Batch& batch) {
        batch.setFramebuffer(_framebuffer);
    });
}
