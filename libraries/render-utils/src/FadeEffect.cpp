//
//  FadeEffect.cpp

//  Created by Olivier Prat on 17/07/2017.
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "FadeEffect.h"
#include "FadeEffectJobs.h"
#include "TextureCache.h"

#include "render/TransitionStage.h"

#include "FadeObjectParams.shared.slh"
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
    const auto editedFadeCategory = task.addJob<FadeJob>("Fade");
    task.addJob<FadeEditJob>("FadeEdit", editedFadeCategory);
}

render::ShapePipeline::BatchSetter FadeEffect::getBatchSetter() {
    return [](const render::ShapePipeline& shapePipeline, gpu::Batch& batch, render::Args*) {
        batch.setResourceTexture(render_utils::slot::texture::FadeMask, _maskMap);
        batch.setUniformBuffer(render_utils::slot::buffer::FadeParameters, FadeJob::getConfigurationBuffer());
    };
}

render::ShapePipeline::ItemSetter FadeEffect::getItemUniformSetter() {
    return [](const render::ShapePipeline& shapePipeline, render::Args* args, const render::Item& item) {
        if (!render::TransitionStage::isIndexInvalid(item.getTransitionId())) {
            const auto& scene = args->_scene;
            const auto& batch = args->_batch;
            auto transitionStage = scene->getStage<render::TransitionStage>();
            auto& transitionState = transitionStage->getTransition(item.getTransitionId());

            if (transitionState.paramsBuffer._size != sizeof(gpu::StructBuffer<FadeObjectParams>)) {
                static_assert(sizeof(transitionState.paramsBuffer) == sizeof(gpu::StructBuffer<FadeObjectParams>), "Assuming gpu::StructBuffer is a helper class for gpu::BufferView");
                transitionState.paramsBuffer = gpu::StructBuffer<FadeObjectParams>();
            }

            const auto fadeCategory = FadeJob::transitionToCategory[transitionState.eventType];
            auto& paramsConst = static_cast<gpu::StructBuffer<FadeObjectParams>&>(transitionState.paramsBuffer).get();

            if (paramsConst.category != fadeCategory
                || paramsConst.threshold != transitionState.threshold
                || glm::vec3(paramsConst.baseOffset) != transitionState.baseOffset
                || glm::vec3(paramsConst.noiseOffset) != transitionState.noiseOffset
                || glm::vec3(paramsConst.baseInvSize) != transitionState.baseInvSize) {
                auto& params = static_cast<gpu::StructBuffer<FadeObjectParams>&>(transitionState.paramsBuffer).edit();

                params.category = fadeCategory;
                params.threshold = transitionState.threshold;
                params.baseInvSize = glm::vec4(transitionState.baseInvSize, 0.0f);
                params.noiseOffset = glm::vec4(transitionState.noiseOffset, 0.0f);
                params.baseOffset = glm::vec4(transitionState.baseOffset, 0.0f);
            }
            batch->setUniformBuffer(render_utils::slot::buffer::FadeObjectParameters, transitionState.paramsBuffer);
        }
    };
}
