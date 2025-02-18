//
//  RenderViewTask.cpp
//  render-utils/src/
//
//  Created by Sam Gateau on 5/25/2017.
//  Copyright 2017 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "RenderViewTask.h"

#include "FadeEffect.h"
#include "RenderShadowTask.h"
#include "RenderCommonTask.h"
#include "RenderDeferredTask.h"
#include "RenderForwardTask.h"

void RenderShadowsAndDeferredTask::build(JobModel& task, const render::Varying& input, render::Varying& output, render::CullFunctor cullFunctor, uint8_t tagBits,
        uint8_t tagMask, uint8_t transformOffset, size_t depth) {
    task.addJob<SetRenderMethod>("SetRenderMethodTask", render::Args::DEFERRED);

    const auto items = input.getN<DeferredForwardSwitchJob::Input>(0);
    const auto lightingModel = input.getN<DeferredForwardSwitchJob::Input>(1);
    const auto lightingStageFramesAndZones = input.getN<DeferredForwardSwitchJob::Input>(2);

    // Warning : the cull functor passed to the shadow pass should only be testing for LOD culling. If frustum culling
    // is performed, then casters not in the view frustum will be removed, which is not what we wish.
    const auto shadowTaskIn = RenderShadowTask::Input(lightingStageFramesAndZones.get<AssembleLightingStageTask::Output>().get0()[0], lightingModel).asVarying();
    const auto shadowTaskOut = task.addJob<RenderShadowTask>("RenderShadowTask", shadowTaskIn, cullFunctor, tagBits, tagMask);

    const auto renderDeferredInput = RenderDeferredTask::Input(items, lightingModel, lightingStageFramesAndZones, shadowTaskOut).asVarying();
    task.addJob<RenderDeferredTask>("RenderDeferredTask", renderDeferredInput, cullFunctor, transformOffset, depth);
}

void DeferredForwardSwitchJob::build(JobModel& task, const render::Varying& input, render::Varying& output, render::CullFunctor cullFunctor, uint8_t tagBits,
        uint8_t tagMask, uint8_t transformOffset, size_t depth) {
    task.addBranch<RenderShadowsAndDeferredTask>("RenderShadowsAndDeferredTask", 0, input, cullFunctor, tagBits, tagMask, transformOffset, depth);

    task.addBranch<RenderForwardTask>("RenderForwardTask", 1, input, cullFunctor, transformOffset, depth);
}

void RenderViewTask::build(JobModel& task, const render::Varying& input, render::Varying& output, render::CullFunctor cullFunctor, uint8_t tagBits, uint8_t tagMask,
        TransformOffset transformOffset, size_t depth) {
    const auto items = task.addJob<RenderFetchCullSortTask>("FetchCullSort", cullFunctor, tagBits, tagMask);

    if (depth == 0 && tagBits == render::ItemKey::TAG_BITS_0) {
        // TODO: This doesn't actually do any rendering, it simply processes the fade transactions.  Even though forward rendering
        // doesn't support fading right now, we still need to do this once for both paths, otherwise we are left with orphaned objects
        // after they fade out.  In the future, we should refactor this to happen elsewhere.
        task.addJob<FadeEffect>("FadeEffect");
    }

    // Issue the lighting model, aka the big global settings for the view 
    const auto lightingModel = task.addJob<MakeLightingModel>("LightingModel");

    // Assemble the lighting stages current frames
    const auto lightingStageFramesAndZones = task.addJob<AssembleLightingStageTask>("AssembleStages", items);

#ifndef Q_OS_ANDROID
        const auto deferredForwardIn = DeferredForwardSwitchJob::Input(items, lightingModel, lightingStageFramesAndZones).asVarying();
        task.addJob<DeferredForwardSwitchJob>("DeferredForwardSwitch", deferredForwardIn, cullFunctor, tagBits, tagMask, transformOffset, depth);
#else
        const auto renderInput = RenderForwardTask::Input(items, lightingModel, lightingStageFramesAndZones).asVarying();
        task.addJob<RenderForwardTask>("RenderForwardTask", renderInput, cullFunctor, transformOffset, depth);
#endif
}
