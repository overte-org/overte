//
//  RenderableLightEntityItem.cpp
//  interface/src
//
//  Created by Brad Hefta-Gaub on 8/6/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "RenderableLightEntityItem.h"

#include <GLMHelpers.h>
#include <PerfStat.h>

using namespace render;
using namespace render::entities;

void LightEntityRenderer::doRenderUpdateAsynchronousTyped(const TypedEntityPointer& entity) {
    auto& lightPayload = *_lightPayload;

    lightPayload.setVisible(_visible);

    auto light = lightPayload.editLight();
    light->setPosition(entity->getWorldPosition());
    light->setOrientation(entity->getWorldOrientation());

    bool success;
    lightPayload.editBound() = entity->getAABox(success);
    if (!success) {
        lightPayload.editBound() = render::Item::Bound();
    }

    glm::vec3 dimensions = entity->getScaledDimensions();
    float largestDiameter = glm::compMax(dimensions);
    light->setMaximumRadius(largestDiameter / 2.0f);

    light->setColor(toGlm(entity->getColor()));

    _intensity = entity->getIntensity();
    light->setIntensity(_intensity);

    light->setFalloffRadius(entity->getFalloffRadius());


    float exponent = entity->getExponent();
    float cutoff = glm::radians(entity->getCutoff());
    if (!entity->getIsSpotlight()) {
        light->setType(graphics::Light::POINT);
    } else {
        light->setType(graphics::Light::SPOT);

        light->setSpotAngle(cutoff);
        light->setSpotExponent(exponent);
    }
}

ItemKey LightEntityRenderer::getKey() {
    return payloadGetKey(_lightPayload);
}

Item::Bound LightEntityRenderer::getBound(RenderArgs* args) {
    return payloadGetBound(_lightPayload, args);
}

void LightEntityRenderer::doRender(RenderArgs* args) {
    bool fading = ShapeKey(args->_itemShapeKey).isFaded();
    if (fading) {
        float fadeRatio = 1.0f - std::clamp(getFadeParams(args->_scene).baseOffsetAndThreshold.w, 0.0f, 1.0f);
        auto light = _lightPayload->editLight();
        light->setIntensity(_intensity * fadeRatio);
    }
    _lightPayload->render(args);
}
