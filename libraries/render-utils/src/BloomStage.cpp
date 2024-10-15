//
//  BloomStage.cpp
//
//  Created by Sam Gondelman on 8/7/2018
//  Copyright 2018 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "BloomStage.h"

#include "DeferredLightingEffect.h"

#include <gpu/Context.h>

std::string BloomStage::_name { "BLOOM_STAGE" };

void BloomStageSetup::run(const render::RenderContextPointer& renderContext) {
    auto stage = renderContext->_scene->getStage(BloomStage::getName());
    if (!stage) {
        renderContext->_scene->resetStage(BloomStage::getName(), std::make_shared<BloomStage>());
    }
}
