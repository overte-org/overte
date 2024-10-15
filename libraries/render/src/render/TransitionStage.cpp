//
//  TransitionStage.cpp
//
//  Created by Olivier Prat on 07/07/2017.
//  Copyright 2017 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "TransitionStage.h"

#include <algorithm>

using namespace render;

std::string TransitionStage::_name { "TRANSITION_STAGE" };

TransitionStage::Index TransitionStage::addTransition(ItemID itemId, Transition::Type type, ItemID boundId) {
    Transition transition;
    transition.eventType = type;
    transition.itemId = itemId;
    transition.boundItemId = boundId;
    return addElement(transition);
}

void TransitionStageSetup::run(const RenderContextPointer& renderContext) {
    auto stage = renderContext->_scene->getStage(TransitionStage::getName());
    if (!stage) {
        renderContext->_scene->resetStage(TransitionStage::getName(), std::make_shared<TransitionStage>());
    }
}
