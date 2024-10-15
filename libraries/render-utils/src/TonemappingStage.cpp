//
//  TonemappingStage.cpp
//
//  Created by HifiExperiments on 6/24/24
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "TonemappingStage.h"

#include <gpu/Context.h>

std::string TonemappingStage::_name { "TONEMAPPING_STAGE" };

void TonemappingStageSetup::run(const render::RenderContextPointer& renderContext) {
    auto stage = renderContext->_scene->getStage(TonemappingStage::getName());
    if (!stage) {
        renderContext->_scene->resetStage(TonemappingStage::getName(), std::make_shared<TonemappingStage>());
    }
}
