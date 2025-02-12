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

std::string TonemappingStage::_stageName { "TONEMAPPING_STAGE" };
const TonemappingStage::Index TonemappingStage::INVALID_INDEX { render::indexed_container::INVALID_INDEX };

TonemappingStage::Index TonemappingStage::findTonemapping(const TonemappingPointer& tonemapping) const {
    auto found = _tonemappingMap.find(tonemapping);
    if (found != _tonemappingMap.end()) {
        return INVALID_INDEX;
    } else {
        return (*found).second;
    }
}

TonemappingStage::Index TonemappingStage::addTonemapping(const TonemappingPointer& tonemapping) {
    auto found = _tonemappingMap.find(tonemapping);
    if (found == _tonemappingMap.end()) {
        auto tonemappingId = _tonemappings.newElement(tonemapping);
        // Avoid failing to allocate a tonemapping, just pass
        if (tonemappingId != INVALID_INDEX) {
            // Insert the tonemapping and its index in the reverse map
            _tonemappingMap.insert(TonemappingMap::value_type(tonemapping, tonemappingId));
        }
        return tonemappingId;
    } else {
        return (*found).second;
    }
}

TonemappingStage::TonemappingPointer TonemappingStage::removeTonemapping(Index index) {
    TonemappingPointer removed = _tonemappings.freeElement(index);
    if (removed) {
        _tonemappingMap.erase(removed);
    }
    return removed;
}

TonemappingStageSetup::TonemappingStageSetup() {}

void TonemappingStageSetup::run(const render::RenderContextPointer& renderContext) {
    auto stage = renderContext->_scene->getStage(TonemappingStage::getName());
    if (!stage) {
        renderContext->_scene->resetStage(TonemappingStage::getName(), std::make_shared<TonemappingStage>());
    }
}
