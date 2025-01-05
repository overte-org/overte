//
//  AmbientOcclusionStage.h
//
//  Created by HifiExperiments on 6/24/24
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_render_utils_AmbientOcclusionStage_h
#define hifi_render_utils_AmbientOcclusionStage_h

#include <graphics/AmbientOcclusion.h>
#include <render/Stage.h>
#include <render/StageSetup.h>

// AmbientOcclusion stage to set up ambientOcclusion-related rendering tasks
class AmbientOcclusionStage : public render::PointerStage<graphics::AmbientOcclusion, graphics::AmbientOcclusionPointer> {};
using AmbientOcclusionStagePointer = std::shared_ptr<AmbientOcclusionStage>;

class AmbientOcclusionStageSetup : public render::StageSetup<AmbientOcclusionStage> {
public:
    using JobModel = render::Job::Model<AmbientOcclusionStageSetup>;
};

#endif
