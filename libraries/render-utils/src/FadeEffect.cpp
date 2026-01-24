//
//  FadeEffect.cpp

//  Created by Olivier Prat on 17/07/2017.
//  Copyright 2017 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "FadeEffect.h"
#include "FadeEffectJobs.h"
#include "TextureCache.h"

#include "render/TransitionStage.h"

#include "render-utils/ShaderConstants.h"
#include <PathUtils.h>

gpu::TexturePointer FadeEffect::_maskMap;

FadeEffect::FadeEffect() {
    std::once_flag once;
    std::call_once(once, [] {
        auto texturePath = PathUtils::resourcesPath() + "images/fadeMask.png";
        _maskMap = DependencyManager::get<TextureCache>()->getImageTexture(texturePath, image::TextureUsage::STRICT_TEXTURE);
    });
}

void FadeEffect::build(JobModel& task, const render::Varying& inputs, render::Varying& outputs) {
    const auto editedItem = task.addJob<FadeEditJob>("FadeEdit");
    task.addJob<FadeJob>("Fade", editedItem);
}

render::ShapePipeline::BatchSetter FadeEffect::getBatchSetter() {
    return [](const render::ShapePipeline*, gpu::Batch& batch, render::Args*) {
        batch.setResourceTexture(render_utils::slot::texture::FadeMask, _maskMap);
    };
}

render::ShapePipeline::ItemSetter FadeEffect::getItemUniformSetter() {
    return [](const render::ShapePipeline*, render::Args* args, const render::Item& item) {
        if (!render::TransitionStage::isIndexInvalid(item.getTransitionId())) {
            const auto& scene = args->_scene;
            const auto& batch = args->_batch;
            auto transitionStage = scene->getStage<render::TransitionStage>();
            auto& transitionState = transitionStage->getElement(item.getTransitionId());
            batch->setUniformBuffer(render_utils::slot::buffer::FadeObjectParameters, transitionState.paramsBuffer);
        }
    };
}
