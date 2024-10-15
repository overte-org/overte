//
//  AmbientOcclusionStage.cpp
//
//  Created by HifiExperiments on 6/24/24
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "AmbientOcclusionStage.h"

#include <gpu/Context.h>

std::string AmbientOcclusionStage::_name { "AMBIENT_OCCLUSION_STAGE" };

void AmbientOcclusionStageSetup::run(const render::RenderContextPointer& renderContext) {
    auto stage = renderContext->_scene->getStage(AmbientOcclusionStage::getName());
    if (!stage) {
        renderContext->_scene->resetStage(AmbientOcclusionStage::getName(), std::make_shared<AmbientOcclusionStage>());
    }
}
