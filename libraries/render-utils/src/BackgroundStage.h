//
//  BackgroundStage.h
//
//  Created by Sam Gateau on 5/9/2017.
//  Copyright 2015 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_render_utils_BackgroundStage_h
#define hifi_render_utils_BackgroundStage_h

#include <graphics/Stage.h>
#include <render/Stage.h>
#include <render/StageSetup.h>

#include "HazeStage.h"
#include "LightingModel.h"

// Background stage to set up background-related rendering tasks
class BackgroundStage : public render::PointerStage<graphics::SunSkyStage, graphics::SunSkyStagePointer> {};
using BackgroundStagePointer = std::shared_ptr<BackgroundStage>;

class BackgroundStageSetup : public render::StageSetup<BackgroundStage> {
public:
    using JobModel = render::Job::Model<BackgroundStageSetup>;
};

class DrawBackgroundStage {
public:
    using Inputs = render::VaryingSet3<LightingModelPointer, BackgroundStage::FramePointer, HazeStage::FramePointer>;
    using JobModel = render::Job::ModelI<DrawBackgroundStage, Inputs>;

    DrawBackgroundStage(uint transformSlot) : _transformSlot(transformSlot) {}

    void run(const render::RenderContextPointer& renderContext, const Inputs& inputs);

private:
    uint _transformSlot;
};

#endif
