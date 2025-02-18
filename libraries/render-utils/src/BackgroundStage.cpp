//
//  BackgroundStage.cpp
//
//  Created by Sam Gateau on 5/9/2017.
//  Copyright 2015 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "BackgroundStage.h"

#include "DeferredLightingEffect.h"

#include <graphics/ShaderConstants.h>

template <>
std::string render::PointerStage<graphics::SunSkyStage, graphics::SunSkyStagePointer>::_name { "BACKGROUND_STAGE" };

void DrawBackgroundStage::run(const render::RenderContextPointer& renderContext, const Inputs& inputs) {
    const auto& lightingModel = inputs.get0();
    if (!lightingModel->isBackgroundEnabled()) {
        return;
    }

    // Background rendering decision
    const auto& backgroundStage = renderContext->_scene->getStage<BackgroundStage>();
    const auto& backgroundFrame = inputs.get1();
    graphics::SkyboxPointer skybox;
    if (backgroundStage && backgroundFrame->_elements.size()) {
        const auto& background = backgroundStage->getElement(backgroundFrame->_elements.front());
        if (background) {
            skybox = background->getSkybox();
        }   
    }

    const auto& hazeFrame = inputs.get2();

    if (skybox && !skybox->empty()) {
        PerformanceTimer perfTimer("skybox");
        auto args = renderContext->args;

        gpu::doInBatch("DrawBackgroundStage::run", args->_context, [&](gpu::Batch& batch) {
            PROFILE_RANGE_BATCH(batch, "Background");
            args->_batch = &batch;

            batch.enableSkybox(true);

            batch.setViewportTransform(args->_viewport);
            batch.setStateScissorRect(args->_viewport);

            bool forward = args->_renderMethod == render::Args::RenderMethod::FORWARD;
            batch.setProjectionJitterEnabled(!forward);

            // If we're using forward rendering, we need to calculate haze
            if (forward) {
                const auto& hazeStage = args->_scene->getStage<HazeStage>();
                if (hazeStage && hazeFrame->_elements.size() > 0) {
                    const auto& hazePointer = hazeStage->getElement(hazeFrame->_elements.front());
                    if (hazePointer) {
                        batch.setUniformBuffer(graphics::slot::buffer::Buffer::HazeParams, hazePointer->getHazeParametersBuffer());
                    }
                }
            }

            skybox->render(batch, args->getViewFrustum(), forward, _transformSlot);
        });
        args->_batch = nullptr;
    }
}
