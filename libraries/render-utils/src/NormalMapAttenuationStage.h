//
//  NormalMapAttenuationStage.h
//
//  Created by HifiExperiments on 7/3/25
//  Copyright 2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_render_utils_NormalMapAttenuationStage_h
#define hifi_render_utils_NormalMapAttenuationStage_h

#include <graphics/NormalMapAttenuation.h>
#include <render/Engine.h>
#include <render/Stage.h>
#include <render/StageSetup.h>

#include "LightingModel.h"

// NormalMapAttenuation stage to set up normal-map-attenuation-related rendering tasks
class NormalMapAttenuationStage : public render::PointerStage<graphics::NormalMapAttenuation, graphics::NormalMapAttenuationPointer> {};
using NormalMapAttenuationStagePointer = std::shared_ptr<NormalMapAttenuationStage>;

class NormalMapAttenuationStageSetup : public render::StageSetup<NormalMapAttenuationStage> {
public:
    using JobModel = render::Job::Model<NormalMapAttenuationStageSetup>;
};

class SetNormalMapAttenuation {
public:
    // Inputs: lightingModel, normalMapAttenuationFrame
    using Inputs = render::VaryingSet2<LightingModelPointer, NormalMapAttenuationStage::FramePointer>;
    using JobModel = render::Job::ModelI<SetNormalMapAttenuation, Inputs>;

    void run(const render::RenderContextPointer& renderContext, const Inputs& input);
};

#endif
