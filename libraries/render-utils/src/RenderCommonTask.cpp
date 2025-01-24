//
//  RenderCommonTask.cpp
//
//  Created by Bradley Austin Davis on 2018/01/09
//  Copyright 2013-2018 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "RenderCommonTask.h"

#include <gpu/Context.h>
#include <graphics/ShaderConstants.h>

#include "render-utils/ShaderConstants.h"
#include "DeferredLightingEffect.h"
#include "FadeEffect.h"
#include "RenderUtilsLogging.h"
#include "RenderViewTask.h"
#include "StencilMaskPass.h"

namespace ru {
    using render_utils::slot::texture::Texture;
    using render_utils::slot::buffer::Buffer;
}

namespace gr {
    using graphics::slot::texture::Texture;
    using graphics::slot::buffer::Buffer;
}

using RenderArgsPointer = std::shared_ptr<RenderArgs>;

using namespace render;
extern void initMirrorPipelines(ShapePlumber& plumber, gpu::StatePointer state, const render::ShapePipeline::BatchSetter& batchSetter, const render::ShapePipeline::ItemSetter& itemSetter, bool forward);

void BeginGPURangeTimer::run(const render::RenderContextPointer& renderContext, gpu::RangeTimerPointer& timer) {
    timer = _gpuTimer;
    gpu::doInBatch("BeginGPURangeTimer", renderContext->args->_context, [&](gpu::Batch& batch) {
        _gpuTimer->begin(batch);
        batch.pushProfileRange(timer->name().c_str());
    });
}

void EndGPURangeTimer::run(const render::RenderContextPointer& renderContext, const gpu::RangeTimerPointer& timer) {
    gpu::doInBatch("EndGPURangeTimer", renderContext->args->_context, [&](gpu::Batch& batch) {
        batch.popProfileRange();
        timer->end(batch);
    });
    
    auto config = std::static_pointer_cast<Config>(renderContext->jobConfig);
    config->setGPUBatchRunTime(timer->getGPUAverage(), timer->getBatchAverage());
}

DrawLayered3D::DrawLayered3D(const render::ShapePlumberPointer& shapePlumber, bool opaque, bool jitter, uint transformSlot) :
    _shapePlumber(shapePlumber),
    _transformSlot(transformSlot),
    _opaquePass(opaque),
    _isJitterEnabled(jitter) {
}

void DrawLayered3D::run(const RenderContextPointer& renderContext, const Inputs& inputs) {
    assert(renderContext->args);
    assert(renderContext->args->hasViewFrustum());

    auto config = std::static_pointer_cast<Config>(renderContext->jobConfig);

    const auto& inItems = inputs.get0();
    const auto& frameTransform = inputs.get1();
    const auto& lightingModel = inputs.get2();
    const auto& hazeFrame = inputs.get3();
    
    config->setNumDrawn((int)inItems.size());
    emit config->numDrawnChanged();

    RenderArgs* args = renderContext->args;

    graphics::HazePointer haze;
    const auto& hazeStage = renderContext->args->_scene->getStage<HazeStage>();
    if (hazeStage && hazeFrame->_elements.size() > 0) {
        // We use _hazes.back() here because the last haze object will always have haze disabled.
        haze = hazeStage->getElement(hazeFrame->_elements.back());
    }

    // Clear the framebuffer without stereo
    // Needs to be distinct from the other batch because using the clear call 
    // while stereo is enabled triggers a warning
    if (_opaquePass) {
        gpu::doInBatch("DrawLayered3D::run::clear", args->_context, [&](gpu::Batch& batch) {
            batch.enableStereo(false);
            batch.clearDepthFramebuffer(true, false);
        });
    }

    if (!inItems.empty()) {
        auto deferredLightingEffect = DependencyManager::get<DeferredLightingEffect>();

        // Render the items
        gpu::doInBatch("DrawLayered3D::main", args->_context, [&](gpu::Batch& batch) {
            PROFILE_RANGE_BATCH(batch, "DrawLayered3D::main");
            args->_batch = &batch;
            batch.setViewportTransform(args->_viewport);
            batch.setStateScissorRect(args->_viewport);

            batch.setProjectionJitterEnabled(_isJitterEnabled);
            batch.setSavedViewProjectionTransform(_transformSlot);

            // Setup lighting model for all items;
            batch.setUniformBuffer(ru::Buffer::LightModel, lightingModel->getParametersBuffer());
            batch.setResourceTexture(ru::Texture::AmbientFresnel, lightingModel->getAmbientFresnelLUT());
            batch.setUniformBuffer(ru::Buffer::DeferredFrameTransform, frameTransform->getFrameTransformBuffer());

            if (haze) {
                batch.setUniformBuffer(graphics::slot::buffer::Buffer::HazeParams, haze->getHazeParametersBuffer());
            }

            // Set the light
            deferredLightingEffect->setupKeyLightBatch(args, batch);

            auto renderMethod = args->_renderMethod;
            args->_renderMethod = Args::RenderMethod::FORWARD;
            if (_opaquePass) {
                renderStateSortShapes(renderContext, _shapePlumber, inItems, _maxDrawn);
            } else {
                renderShapes(renderContext, _shapePlumber, inItems, _maxDrawn);
            }

            deferredLightingEffect->unsetLocalLightsBatch(batch);

            args->_renderMethod = renderMethod;
            args->_batch = nullptr;
        });
    }
}

void Blit::run(const RenderContextPointer& renderContext, const gpu::FramebufferPointer& srcFramebuffer) {
    assert(renderContext->args);
    assert(renderContext->args->_context);

    RenderArgs* renderArgs = renderContext->args;
    auto blitFbo = renderArgs->_blitFramebuffer;

    if (!blitFbo) {
        qCWarning(renderutils) << "Blit::run - no blit frame buffer.";
        return;
    }

    // Determine size from viewport
    int width = renderArgs->_viewport.z;
    int height = renderArgs->_viewport.w;

    // Blit primary to blit FBO
    auto primaryFbo = srcFramebuffer;

    gpu::doInBatch("Blit", renderArgs->_context, [&](gpu::Batch& batch) {
        batch.setFramebuffer(blitFbo);

        gpu::Vec4i rect;
        rect.z = width;
        rect.w = height;

        batch.blit(primaryFbo, rect, blitFbo, rect);
    });
}

NewFramebuffer::NewFramebuffer(gpu::Element pixelFormat) {
    _pixelFormat = pixelFormat;
}

void NewFramebuffer::run(const render::RenderContextPointer& renderContext, Output& output) {
    RenderArgs* args = renderContext->args;
    glm::uvec2 frameSize(args->_viewport.z, args->_viewport.w);
    output.reset();

    if (_outputFramebuffer && _outputFramebuffer->getSize() != frameSize) {
        _outputFramebuffer.reset();
    }

    if (!_outputFramebuffer) {
        _outputFramebuffer = gpu::FramebufferPointer(gpu::Framebuffer::create("newFramebuffer.out"));
        auto colorFormat = _pixelFormat;
        auto defaultSampler = gpu::Sampler(gpu::Sampler::FILTER_MIN_MAG_LINEAR);
        auto colorTexture = gpu::Texture::createRenderBuffer(colorFormat, frameSize.x, frameSize.y, gpu::Texture::SINGLE_MIP, defaultSampler);
        _outputFramebuffer->setRenderBuffer(0, colorTexture);
    }

    output = _outputFramebuffer;
}

void NewOrDefaultFramebuffer::run(const render::RenderContextPointer& renderContext, const Input& input, Output& output) {
    RenderArgs* args = renderContext->args;
    // auto frameSize = input;
    glm::uvec2 frameSize(args->_viewport.z, args->_viewport.w);
    output.reset();

    // First if the default Framebuffer is the correct size then use it
    auto destBlitFbo = args->_blitFramebuffer;
    if (destBlitFbo && destBlitFbo->getSize() == frameSize) {
        output = destBlitFbo;
        return;
    }

    // Else use the lodal Framebuffer
    if (_outputFramebuffer && _outputFramebuffer->getSize() != frameSize) {
        _outputFramebuffer.reset();
    }

    if (!_outputFramebuffer) {
        _outputFramebuffer = gpu::FramebufferPointer(gpu::Framebuffer::create("newOrDefaultFramebuffer.out"));
        auto colorFormat = gpu::Element::COLOR_SRGBA_32;
        auto defaultSampler = gpu::Sampler(gpu::Sampler::FILTER_MIN_MAG_LINEAR);
        auto colorTexture = gpu::Texture::createRenderBuffer(colorFormat, frameSize.x, frameSize.y, gpu::Texture::SINGLE_MIP, defaultSampler);
        _outputFramebuffer->setRenderBuffer(0, colorTexture);
    }

    output = _outputFramebuffer;
}

void ResolveFramebuffer::run(const render::RenderContextPointer& renderContext, const Inputs& inputs, Outputs& outputs) {
    RenderArgs* args = renderContext->args;
    auto srcFbo = inputs.get0();
    auto destFbo = inputs.get1();

    if (!destFbo) {
        destFbo = args->_blitFramebuffer;
    }
    outputs = destFbo;

    // Check valid src and dest
    if (!srcFbo || !destFbo) {
        return;
    }
    
    // Check valid size for sr and dest
    auto frameSize(srcFbo->getSize());
    if (destFbo->getSize() != frameSize) {
        return;
    }

    gpu::Vec4i rectSrc;
    rectSrc.z = frameSize.x;
    rectSrc.w = frameSize.y;
    gpu::doInBatch("Resolve", args->_context, [&](gpu::Batch& batch) { 
        batch.blit(srcFbo, rectSrc, destFbo, rectSrc);
    });
}

 void ExtractFrustums::run(const render::RenderContextPointer& renderContext, const Inputs& inputs, Outputs& output) {
    assert(renderContext->args);
    assert(renderContext->args->_context);

    RenderArgs* args = renderContext->args;

    const auto& shadowFrame = inputs;

    // Return view frustum
    auto& viewFrustum = output[VIEW_FRUSTUM].edit<ViewFrustumPointer>();
    if (!viewFrustum) {
        viewFrustum = std::make_shared<ViewFrustum>(args->getViewFrustum());
    } else {
        *viewFrustum = args->getViewFrustum();
    }

    // Return shadow frustum
    LightStage::ShadowPointer globalShadow;
    if (shadowFrame && !shadowFrame->_objects.empty() && shadowFrame->_objects[0]) {
        globalShadow = shadowFrame->_objects[0];
    }
    for (auto i = 0; i < SHADOW_CASCADE_FRUSTUM_COUNT; i++) {
        auto& shadowFrustum = output[SHADOW_CASCADE0_FRUSTUM+i].edit<ViewFrustumPointer>();
        if (globalShadow && i<(int)globalShadow->getCascadeCount()) {
            auto& cascade = globalShadow->getCascade(i);
            shadowFrustum = cascade.getFrustum();
        } else {
            shadowFrustum.reset();
        }
    }
}

class SetupMirrorTask {
public:
    using Input = RenderMirrorTask::Inputs;
    using Outputs = render::VaryingSet3<render::ItemBound, gpu::FramebufferPointer, RenderArgsPointer>;
    using JobModel = render::Job::ModelIO<SetupMirrorTask, Input, Outputs>;

    SetupMirrorTask(size_t mirrorIndex, size_t depth) : _mirrorIndex(mirrorIndex), _depth(depth) {}

    void run(const render::RenderContextPointer& renderContext, const Input& inputs, Outputs& outputs) {
        auto args = renderContext->args;
        auto items = inputs.get0();

        if (items.empty() || _mirrorIndex > items.size() - 1) {
            renderContext->taskFlow.abortTask();
            return;
        }

        auto inputFramebuffer = inputs.get1();
        if (!_mirrorFramebuffer || _mirrorFramebuffer->getWidth() != inputFramebuffer->getWidth() || _mirrorFramebuffer->getHeight() != inputFramebuffer->getHeight()) {
            _mirrorFramebuffer.reset(gpu::Framebuffer::create("mirror" + _mirrorIndex, gpu::Element::COLOR_SRGBA_32, inputFramebuffer->getWidth(), inputFramebuffer->getHeight()));
        }

        render::ItemBound mirror = items[_mirrorIndex];

        _cachedArgsPointer->_renderMode = args->_renderMode;
        _cachedArgsPointer->_blitFramebuffer = args->_blitFramebuffer;
        _cachedArgsPointer->_ignoreItem = args->_ignoreItem;
        _cachedArgsPointer->_mirrorDepth = args->_mirrorDepth;
        _cachedArgsPointer->_numMirrorFlips = args->_numMirrorFlips;

        ViewFrustum srcViewFrustum = args->getViewFrustum();
        ItemID portalExitID = args->_scene->getItem(mirror.id).computeMirrorView(srcViewFrustum);

        args->_blitFramebuffer = _mirrorFramebuffer;
        args->_ignoreItem = portalExitID != Item::INVALID_ITEM_ID ? portalExitID : mirror.id;
        args->_mirrorDepth = _depth;
        args->_numMirrorFlips += portalExitID != Item::INVALID_ITEM_ID ? 0 : 1;

        gpu::doInBatch("SetupMirrorTask::run", args->_context, [&](gpu::Batch& batch) {
            bool shouldMirror = args->_numMirrorFlips % 2 == (args->_renderMode != RenderArgs::MIRROR_RENDER_MODE);
            batch.setContextMirrorViewCorrection(shouldMirror);
        });

        // Without calculating the bound planes, the mirror will use the same culling frustum as the main camera,
        // which is not what we want here.
        srcViewFrustum.calculate();
        args->pushViewFrustum(srcViewFrustum);

        outputs.edit0() = mirror;
        outputs.edit1() = inputFramebuffer;
        outputs.edit2() = _cachedArgsPointer;
    }

protected:
    gpu::FramebufferPointer _mirrorFramebuffer { nullptr };
    RenderArgsPointer _cachedArgsPointer { std::make_shared<RenderArgs>() };
    size_t _mirrorIndex;
    size_t _depth;

};

class DrawMirrorTask {
public:
    using Inputs = SetupMirrorTask::Outputs;
    using JobModel = render::Job::ModelI<DrawMirrorTask, Inputs>;

    DrawMirrorTask(uint transformSlot) : _transformSlot(transformSlot) {
        static std::once_flag once;
        std::call_once(once, [this] {
            auto state = std::make_shared<gpu::State>();
            state->setCullMode(gpu::State::CULL_BACK);
            state->setDepthTest(true, true, gpu::LESS_EQUAL);
            PrepareStencil::testMaskDrawShapeNoAA(*state);

            initMirrorPipelines(*_forwardPipelines, state, FadeEffect::getBatchSetter(), FadeEffect::getItemUniformSetter(), true);
            initMirrorPipelines(*_deferredPipelines, state, FadeEffect::getBatchSetter(), FadeEffect::getItemUniformSetter(), false);
        });
    }

    void run(const render::RenderContextPointer& renderContext, const Inputs& inputs) {
        auto args = renderContext->args;
        auto mirror = inputs.get0();
        auto framebuffer = inputs.get1();
        auto cachedArgs = inputs.get2();

        if (cachedArgs) {
            args->_renderMode = cachedArgs->_renderMode;
        }
        args->popViewFrustum();

        gpu::doInBatch("DrawMirrorTask::run", args->_context, [&](gpu::Batch& batch) {
            args->_batch = &batch;

            if (cachedArgs) {
                bool shouldMirror = cachedArgs->_numMirrorFlips % 2 == (args->_renderMode != RenderArgs::MIRROR_RENDER_MODE);
                batch.setContextMirrorViewCorrection(shouldMirror);
            }

            batch.setFramebuffer(framebuffer);
            batch.setViewportTransform(args->_viewport);
            batch.setStateScissorRect(args->_viewport);

            batch.setSavedViewProjectionTransform(_transformSlot);

            batch.setResourceTexture(gr::Texture::MaterialMirror, args->_blitFramebuffer->getRenderBuffer(0));

            renderShapes(renderContext, args->_renderMethod == render::Args::RenderMethod::FORWARD ? _forwardPipelines : _deferredPipelines, { mirror });

            args->_batch = nullptr;
        });

        if (cachedArgs) {
            // Restore the blit framebuffer after we've sampled from it
            args->_blitFramebuffer = cachedArgs->_blitFramebuffer;
            args->_ignoreItem = cachedArgs->_ignoreItem;
            args->_mirrorDepth = cachedArgs->_mirrorDepth;
            args->_numMirrorFlips = cachedArgs->_numMirrorFlips;
        }
    }

private:
    static ShapePlumberPointer _forwardPipelines;
    static ShapePlumberPointer _deferredPipelines;

    uint _transformSlot;
};

ShapePlumberPointer DrawMirrorTask::_forwardPipelines = std::make_shared<ShapePlumber>();
ShapePlumberPointer DrawMirrorTask::_deferredPipelines = std::make_shared<ShapePlumber>();

void RenderMirrorTask::build(JobModel& task, const render::Varying& inputs, render::Varying& output, size_t mirrorIndex, render::CullFunctor cullFunctor,
        uint transformOffset,size_t depth) {
    size_t nextDepth = depth + 1;
    const auto setupOutput = task.addJob<SetupMirrorTask>("SetupMirror" + std::to_string(mirrorIndex) + "Depth" + std::to_string(depth), inputs, mirrorIndex, nextDepth);

    // Our primary view starts at transformOffset 0, and the secondary camera starts at transformOffset 2
    // Our primary mirror views thus start after the secondary camera, at transformOffset 4, and the secondary
    // camera mirror views start after all of the primary camera mirror views, at 4 + NUM_MAIN_MIRROR_SLOTS
    static uint NUM_MAIN_MIRROR_SLOTS = 0;
    static std::once_flag once;
    std::call_once(once, [] {
        for (size_t mirrorDepth = 0; mirrorDepth < MAX_MIRROR_DEPTH; mirrorDepth++) {
            NUM_MAIN_MIRROR_SLOTS += pow(MAX_MIRRORS_PER_LEVEL, mirrorDepth + 1);
        }
        NUM_MAIN_MIRROR_SLOTS *= 2;
    });

    uint mirrorOffset;
    if (transformOffset == RenderViewTask::TransformOffset::MAIN_VIEW) {
        mirrorOffset = RenderViewTask::TransformOffset::FIRST_MIRROR_VIEW - 2;
    } else if (transformOffset == RenderViewTask::TransformOffset::SECONDARY_VIEW) {
        mirrorOffset = RenderViewTask::TransformOffset::FIRST_MIRROR_VIEW + NUM_MAIN_MIRROR_SLOTS - 2;
    } else {
        mirrorOffset = transformOffset;
    }

    // To calculate our transformSlot, we take the transformSlot of our parent and add numSubSlots (the number of slots
    // taken up by a sub-tree starting at this depth) per preceding mirrorIndex
    uint numSubSlots = 0;
    for (size_t mirrorDepth = depth; mirrorDepth < MAX_MIRROR_DEPTH; mirrorDepth++) {
        numSubSlots += pow(MAX_MIRRORS_PER_LEVEL, mirrorDepth + 1 - nextDepth);
    }
    numSubSlots *= 2;

    mirrorOffset += 2 + numSubSlots * (uint)mirrorIndex;

    task.addJob<RenderViewTask>("RenderMirrorView" + std::to_string(mirrorIndex) + "Depth" + std::to_string(depth), cullFunctor, render::ItemKey::TAG_BITS_1,
        render::ItemKey::TAG_BITS_1, (RenderViewTask::TransformOffset) mirrorOffset, nextDepth);

    task.addJob<DrawMirrorTask>("DrawMirrorTask" + std::to_string(mirrorIndex) + "Depth" + std::to_string(depth), setupOutput,
        render::RenderEngine::TS_MAIN_VIEW + transformOffset);
}

void RenderSimulateTask::run(const render::RenderContextPointer& renderContext, const Inputs& inputs) {
    auto args = renderContext->args;
    auto items = inputs.get0();
    auto framebuffer = inputs.get1();

    gpu::doInBatch("RenderSimulateTask::run", args->_context, [&](gpu::Batch& batch) {
        args->_batch = &batch;

        for (ItemBound& item : items) {
            args->_scene->simulate(item.id, args);
        }

        // Reset render state after each simulate
        batch.setFramebuffer(framebuffer);
        batch.setViewportTransform(args->_viewport);
        batch.setStateScissorRect(args->_viewport);

        args->_batch = nullptr;
    });
}