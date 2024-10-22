//
//  TonemappingStage.h
// 
//  Created by HifiExperiments on 6/24/24
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_render_utils_TonemappingStage_h
#define hifi_render_utils_TonemappingStage_h

#include <graphics/Tonemapping.h>
#include <render/Stage.h>
#include <render/StageSetup.h>

// Tonemapping stage to set up tonemapping-related rendering tasks
class TonemappingStage : public render::PointerStage<graphics::Tonemapping, graphics::TonemappingPointer> {};
using TonemappingStagePointer = std::shared_ptr<TonemappingStage>;

class TonemappingStageSetup : public render::StageSetup<TonemappingStage> {
public:
    using JobModel = render::Job::Model<TonemappingStageSetup>;
};

#endif
