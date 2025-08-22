//
//  NormalMapAttenuationStage.cpp
//
//  Created by HifiExperiments on 7/3/25
//  Copyright 2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "NormalMapAttenuationStage.h"

template <>
std::string render::PointerStage<graphics::NormalMapAttenuation, graphics::NormalMapAttenuationPointer>::_name { "NORMAL_MAP_ATTENUATION_STAGE" };

void SetNormalMapAttenuation::run(const render::RenderContextPointer& renderContext, const Inputs& input) {
    auto lightingModel = input.get0();
    const auto normalMapAttenuationFrame = input.get1();

    const auto& normalMapAttenuationStage = renderContext->_scene->getStage<NormalMapAttenuationStage>();
    graphics::NormalMapAttenuation normalMapAttenuation;
    if (normalMapAttenuationStage && normalMapAttenuationFrame->_elements.size()) {
        auto normalMapAttenuationPointer = normalMapAttenuationStage->getElement(normalMapAttenuationFrame->_elements.front());
        if (normalMapAttenuationPointer) {
            normalMapAttenuation.setMin(normalMapAttenuationPointer->getMin());
            normalMapAttenuation.setMax(normalMapAttenuationPointer->getMax());
        } else {
            normalMapAttenuation.setMin(16000.0f);
            normalMapAttenuation.setMax(16000.0f);
        }
    }

    lightingModel->setNormalMapAttenuation(normalMapAttenuation.getMin(), normalMapAttenuation.getMax());
}