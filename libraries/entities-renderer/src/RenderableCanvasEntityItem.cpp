//
//  Created by Ada <ada@thingvellir.net> on 2025-02-24
//  Copyright 2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "RenderableCanvasEntityItem.h"

using namespace render;
using namespace render::entities;

CanvasEntityRenderer::CanvasEntityRenderer(const EntityItemPointer& entity) : Parent(entity) { }

CanvasEntityRenderer::~CanvasEntityRenderer() { }

void CanvasEntityRenderer::doRenderUpdateAsynchronousTyped(const TypedEntityPointer& entity) {
    _texture = entity->getTexture();
}
