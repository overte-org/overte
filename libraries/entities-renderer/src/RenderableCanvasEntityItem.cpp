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
    gpu::Byte pixels[256 * 256 * 4];

    // dummy placeholder texture to make sure the per-frame thing works
    for (int x = 0; x < 256; x++) {
        for (int y = 0; y < 256; y++) {
            pixels[(y * 256 * 4) + (x * 4) + 0] = 255;
            pixels[(y * 256 * 4) + (x * 4) + 1] = 0;
            pixels[(y * 256 * 4) + (x * 4) + 2] = 255;
            pixels[(y * 256 * 4) + (x * 4) + 3] = 255;
        }
    }

    _texture = gpu::Texture::create2D(gpu::Element::COLOR_SRGBA_32, 256, 256);
    _texture->setStoredMipFormat(gpu::Element::COLOR_SRGBA_32);
    _texture->assignStoredMip(0, 256 * 256 * 4, pixels);
    _texture->setSource(__FUNCTION__);

    _testTimer.setInterval(33);
    connect(&_testTimer, &QTimer::timeout, this, &CanvasEntityRenderer::onTimeout);
    _testTimer.start();
}

CanvasEntityRenderer::~CanvasEntityRenderer() { }

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

    _texture->assignStoredMip(0, 256 * 256 * 4, pixels);

    _ticks += 1;
    qDebug("onTimeout: _ticks = %d", _ticks);
}
