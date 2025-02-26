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

CanvasEntityRenderer::CanvasEntityRenderer(const EntityItemPointer& entity) : Parent(entity) {
    auto canvas = std::dynamic_pointer_cast<CanvasEntityItem>(entity);
    _width = canvas->getWidth();
    _width = canvas->getHeight();
}

CanvasEntityRenderer::~CanvasEntityRenderer() { }

void CanvasEntityRenderer::doRenderUpdateSynchronousTyped(const ScenePointer& scene, Transaction& transaction, const TypedEntityPointer& entity) {
    _width = entity->getWidth();
    _height = entity->getHeight();

    qDebug() << "width: " << _width << ", height: " << _height;

    if (entity->needsRenderUpdate()) {
        // misaligned size, can't safely copy
        if (entity->getImageData().length() != _width * _height * 4) {
            entity->setNeedsRenderUpdate(false);
            return;
        }

        auto texture = gpu::Texture::createStrict(gpu::Element::COLOR_SRGBA_32, _width, _height);
        texture->setStoredMipFormat(gpu::Element::COLOR_SRGBA_32);
        texture->setAutoGenerateMips(false);
        texture->assignStoredMip(0, _width * _height * 4, reinterpret_cast<const uint8_t*>(entity->getImageData().data()));
        texture->setSource("CanvasEntityRenderer");
        _texture = texture;

        entity->setNeedsRenderUpdate(false);
    }
}
