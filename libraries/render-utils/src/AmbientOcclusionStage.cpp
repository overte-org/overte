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

std::string AmbientOcclusionStage::_stageName { "AMBIENT_OCCLUSION_STAGE" };
const AmbientOcclusionStage::Index AmbientOcclusionStage::INVALID_INDEX { render::indexed_container::INVALID_INDEX };

AmbientOcclusionStage::Index AmbientOcclusionStage::findAmbientOcclusion(const AmbientOcclusionPointer& ambientOcclusion) const {
    auto found = _ambientOcclusionMap.find(ambientOcclusion);
    if (found != _ambientOcclusionMap.end()) {
        return INVALID_INDEX;
    } else {
        return (*found).second;
    }
}

AmbientOcclusionStage::Index AmbientOcclusionStage::addAmbientOcclusion(const AmbientOcclusionPointer& ambientOcclusion) {
    auto found = _ambientOcclusionMap.find(ambientOcclusion);
    if (found == _ambientOcclusionMap.end()) {
        auto ambientOcclusionId = _ambientOcclusions.newElement(ambientOcclusion);
        // Avoid failing to allocate a ambientOcclusion, just pass
        if (ambientOcclusionId != INVALID_INDEX) {
            // Insert the ambientOcclusion and its index in the reverse map
            _ambientOcclusionMap.insert(AmbientOcclusionMap::value_type(ambientOcclusion, ambientOcclusionId));
        }
        return ambientOcclusionId;
    } else {
        return (*found).second;
    }
}

AmbientOcclusionStage::AmbientOcclusionPointer AmbientOcclusionStage::removeAmbientOcclusion(Index index) {
    AmbientOcclusionPointer removed = _ambientOcclusions.freeElement(index);
    if (removed) {
        _ambientOcclusionMap.erase(removed);
    }
    return removed;
}

AmbientOcclusionStageSetup::AmbientOcclusionStageSetup() {}

void AmbientOcclusionStageSetup::run(const render::RenderContextPointer& renderContext) {
    auto stage = renderContext->_scene->getStage(AmbientOcclusionStage::getName());
    if (!stage) {
        renderContext->_scene->resetStage(AmbientOcclusionStage::getName(), std::make_shared<AmbientOcclusionStage>());
    }
}
