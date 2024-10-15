//
//  HazeStage.cpp
//
//  Created by Nissim Hadar on 9/26/2017.
//  Copyright 2015 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "HazeStage.h"

#include "DeferredLightingEffect.h"

#include <gpu/Context.h>

std::string HazeStage::_name { "HAZE_STAGE" };

void HazeStageSetup::run(const render::RenderContextPointer& renderContext) {
    auto stage = renderContext->_scene->getStage(HazeStage::getName());
    if (!stage) {
        renderContext->_scene->resetStage(HazeStage::getName(), std::make_shared<HazeStage>());
    }
}
