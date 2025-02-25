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
    _testTimer.setInterval(16);
    connect(&_testTimer, &QTimer::timeout, this, &CanvasEntityRenderer::onTimeout);
    _testTimer.start();
}

CanvasEntityRenderer::~CanvasEntityRenderer() { }

void CanvasEntityRenderer::doRenderUpdateAsynchronousTyped(const TypedEntityPointer &entity) {
    _width = entity->getWidth();
    _height = entity->getHeight();
}

void CanvasEntityRenderer::onTimeout() {
    gpu::Byte pixels[256 * 256 * 4];

    // XOR placeholder texture
    for (int x = 0; x < 256; x++) {
        for (int y = 0; y < 256; y++) {
            pixels[(y * 256 * 4) + (x * 4) + 0] = ((x + _ticks) ^ (y + _ticks));
            pixels[(y * 256 * 4) + (x * 4) + 1] = ((x + _ticks) ^ (y + _ticks));
            pixels[(y * 256 * 4) + (x * 4) + 2] = ((x + _ticks) ^ (y + _ticks));
            pixels[(y * 256 * 4) + (x * 4) + 3] = 255;
        }
    }

    auto texture = gpu::Texture::createStrict(gpu::Element::COLOR_SRGBA_32, 256, 256);
    texture->setStoredMipFormat(gpu::Element::COLOR_SRGBA_32);
    texture->setAutoGenerateMips(false);
    texture->assignStoredMip(0, 256 * 256 * 4, pixels);
    texture->setSource("CanvasEntityRenderer");
    _texture = texture;

    _ticks += 1;
}
