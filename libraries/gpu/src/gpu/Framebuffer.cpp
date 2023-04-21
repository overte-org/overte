//
//  Framebuffer.cpp
//  libraries/gpu/src/gpu
//
//  Created by Sam Gateau on 4/12/2015.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "Framebuffer.h"
#include <math.h>
#include <QDebug>

#include <shared/PlatformHacks.h>
#include <Transform.h>

using namespace gpu;

Framebuffer::~Framebuffer() {
}

Framebuffer* Framebuffer::create(const std::string& name) {
    auto framebuffer = new Framebuffer();
    framebuffer->setName(name);
    framebuffer->_renderBuffers.resize(MAX_NUM_RENDER_BUFFERS);
    framebuffer->_colorStamps.resize(MAX_NUM_RENDER_BUFFERS, 0);
    return framebuffer;
}


Framebuffer* Framebuffer::create(const std::string& name, const Format& colorBufferFormat, uint16 width, uint16 height) {
    auto framebuffer = Framebuffer::create(name);

    auto colorTexture = TexturePointer(Texture::createRenderBuffer(colorBufferFormat, width, height, Texture::SINGLE_MIP, Sampler(Sampler::FILTER_MIN_MAG_POINT)));
    colorTexture->setSource("Framebuffer::colorTexture");

    framebuffer->setRenderBuffer(0, colorTexture);

    return framebuffer;
}

Framebuffer* Framebuffer::create(const std::string& name, const Format& colorBufferFormat, const Format& depthStencilBufferFormat, uint16 width, uint16 height) {
    auto framebuffer = Framebuffer::create(name);

    auto colorTexture = TexturePointer(Texture::createRenderBuffer(colorBufferFormat, width, height, Texture::SINGLE_MIP, Sampler(Sampler::FILTER_MIN_MAG_POINT)));
    auto depthTexture = TexturePointer(Texture::createRenderBuffer(depthStencilBufferFormat, width, height, Texture::SINGLE_MIP, Sampler(Sampler::FILTER_MIN_MAG_POINT)));
    framebuffer->setRenderBuffer(0, colorTexture);
    framebuffer->setDepthStencilBuffer(depthTexture, depthStencilBufferFormat);

    return framebuffer;
}

Framebuffer* Framebuffer::createShadowmap(uint16 width) {
    auto framebuffer = Framebuffer::create("Shadowmap");

    auto depthFormat = Element(gpu::SCALAR, gpu::FLOAT, gpu::DEPTH); // Depth32 texel format
    auto depthTexture = TexturePointer(Texture::createRenderBuffer(depthFormat, width, width));
    Sampler::Desc samplerDesc;
    samplerDesc._borderColor = glm::vec4(1.0f);
    samplerDesc._wrapModeU = Sampler::WRAP_BORDER;
    samplerDesc._wrapModeV = Sampler::WRAP_BORDER;
    samplerDesc._filter = Sampler::FILTER_MIN_MAG_LINEAR;
    samplerDesc._comparisonFunc = LESS;

    depthTexture->setSampler(Sampler(samplerDesc));
    framebuffer->setDepthStencilBuffer(depthTexture, depthFormat);

    return framebuffer;
}

bool Framebuffer::isSwapchain() const {
    return _swapchain != 0;
}

uint32 Framebuffer::getFrameCount() const {
    if (_swapchain) {
        return _swapchain->getFrameCount();
    } else {
        return _frameCount;
    }
}

bool Framebuffer::validateTargetCompatibility(const Texture& texture, uint32 subresource) const {
    if (texture.getType() == Texture::TEX_1D) {
        return false;
    }

    if (isEmpty()) {
        return true;
    } else {
        if ((texture.getWidth() == getWidth()) && 
            (texture.getHeight() == getHeight()) && 
            (texture.getNumSamples() == getNumSamples())) {
            return true;
        } else {
            return false;
        }
    }
}

void Framebuffer::updateSize(const TexturePointer& texture) {
    if (!isEmpty()) {
        return;
    }

    if (texture) {
        _width = texture->getWidth();
        _height = texture->getHeight();
        _numSamples = texture->getNumSamples();
    } else {
        _width = _height = _numSamples = 0;
    }
}

uint16 Framebuffer::getWidth() const {
    if (isSwapchain()) {
        return getSwapchain()->getWidth();
    } else {
        return _width;
    }
}

uint16 Framebuffer::getHeight() const {
    if (isSwapchain()) {
        return getSwapchain()->getHeight();
    } else {
        return _height;
    }
}

uint16 Framebuffer::getNumSamples() const {
    if (isSwapchain()) {
        return getSwapchain()->getNumSamples();
    } else {
        return _numSamples;
    }
}

// Render buffers
int Framebuffer::setRenderBuffer(uint32 slot, const TexturePointer& texture, uint32 subresource) {
    if (isSwapchain()) {
        return -1;
    }

    Q_ASSERT(!texture || TextureUsageType::RENDERBUFFER == texture->getUsageType());

    // Check for the slot
    if (slot >= getMaxNumRenderBuffers()) {
        return -1;
    }

    // Check for the compatibility of size
    if (texture) {
        if (!validateTargetCompatibility(*texture, subresource)) {
            return -1;
        }

        if (texture->source().empty()) {
            texture->setSource(_name + "::color::" + std::to_string(slot));
        }
    }

    ++_colorStamps[slot];

    updateSize(texture);

    // assign the new one
    _renderBuffers[slot] = TextureView(texture, subresource);

    // update the mask
    int mask = (1<<slot);
    _bufferMask = (_bufferMask & ~(mask));
    if (texture) {
        _bufferMask |= mask;
    }

    return slot;
}

void Framebuffer::removeRenderBuffers() {

    if (isSwapchain()) {
        return;
    }

    _bufferMask = _bufferMask & BUFFER_DEPTHSTENCIL;

    for (auto renderBuffer : _renderBuffers) {
        renderBuffer._texture.reset();
    }

    updateSize(TexturePointer(nullptr));
}


uint32 Framebuffer::getNumRenderBuffers() const {
    uint32 nb = 0;
    for (auto i : _renderBuffers) {
        nb += (!i);
    }
    return nb;
}

const TexturePointer& Framebuffer::getRenderBuffer(uint32 slot) const {
    static const TexturePointer EMPTY;
    if (!isSwapchain() && (slot < getMaxNumRenderBuffers())) {
        return _renderBuffers[slot]._texture;
    } else {
        return EMPTY;
    }

}

uint32 Framebuffer::getRenderBufferSubresource(uint32 slot) const {
    if (!isSwapchain() && (slot < getMaxNumRenderBuffers())) {
        return _renderBuffers[slot]._subresource;
    } else {
        return 0;
    }
}

bool Framebuffer::assignDepthStencilBuffer(const TexturePointer& texture, const Format& format, uint32 subresource) {
    if (isSwapchain()) {
        return false;
    }

    Q_ASSERT(!texture || TextureUsageType::RENDERBUFFER == texture->getUsageType());

    // Check for the compatibility of size
    if (texture) {
        if (!validateTargetCompatibility(*texture)) {
            return false;
        }

        if (texture->source().empty()) {
            texture->setSource(_name + "::depthStencil");
        }
    }

    ++_depthStamp;
    updateSize(texture);

    // assign the new one
    _depthStencilBuffer = TextureView(texture, subresource, format);

    return true;
}

bool Framebuffer::setDepthBuffer(const TexturePointer& texture, const Format& format, uint32 subresource) {
    if (assignDepthStencilBuffer(texture, format, subresource)) {
        _bufferMask = (_bufferMask & ~BUFFER_DEPTHSTENCIL);
        if (texture) {
            if (format.getSemantic() == gpu::DEPTH || format.getSemantic() == gpu::DEPTH_STENCIL) {
                _bufferMask |= BUFFER_DEPTH;
            } else {
                return false;
            }
        }

        return true;
    }
    return false;
}

bool Framebuffer::setStencilBuffer(const TexturePointer& texture, const Format& format, uint32 subresource) {
    if (assignDepthStencilBuffer(texture, format, subresource)) {
        _bufferMask = (_bufferMask & ~BUFFER_DEPTHSTENCIL);
        if (texture) {
            if (format.getSemantic() == gpu::STENCIL || format.getSemantic() == gpu::DEPTH_STENCIL) {
                _bufferMask |= BUFFER_STENCIL;
            } else  {
                return false;
            }
        }

        return true;
    }
    return false;
}

bool Framebuffer::setDepthStencilBuffer(const TexturePointer& texture, const Format& format, uint32 subresource) {
    if (assignDepthStencilBuffer(texture, format, subresource)) {
        _bufferMask = (_bufferMask & ~BUFFER_DEPTHSTENCIL);
        if (texture) {
            if (format.getSemantic() == gpu::DEPTH) {
                _bufferMask |= BUFFER_DEPTH;
            } else if (format.getSemantic() == gpu::STENCIL) {
                _bufferMask |= BUFFER_STENCIL;
            } else if (format.getSemantic() == gpu::DEPTH_STENCIL) {
                _bufferMask |= BUFFER_DEPTHSTENCIL;
            }
        }

        return true;
    }
    return false;
}

const TexturePointer& Framebuffer::getDepthStencilBuffer() const {
    static const TexturePointer EMPTY;
    if (isSwapchain()) {
        return EMPTY;
    } else {
        return _depthStencilBuffer._texture;
    }
}

uint32 Framebuffer::getDepthStencilBufferSubresource() const {
    if (isSwapchain()) {
        return 0;
    } else {
        return _depthStencilBuffer._subresource;
    }
}

Format Framebuffer::getDepthStencilBufferFormat() const {
    if (isSwapchain()) {
      //  return getSwapchain()->getDepthStencilBufferFormat();
        return _depthStencilBuffer._element;
    } else {
        return _depthStencilBuffer._element;
    }
}
glm::vec4 Framebuffer::evalSubregionTexcoordTransformCoefficients(const glm::ivec2& sourceSurface, const glm::ivec2& destRegionSize, const glm::ivec2& destRegionOffset) {
    float sMin = destRegionOffset.x / (float)sourceSurface.x;
    float sWidth = destRegionSize.x / (float)sourceSurface.x;
    float tMin = destRegionOffset.y / (float)sourceSurface.y;
    float tHeight = destRegionSize.y / (float)sourceSurface.y;
    return glm::vec4(sMin, tMin, sWidth, tHeight);
}

glm::vec4 Framebuffer::evalSubregionTexcoordTransformCoefficients(const glm::ivec2& sourceSurface, const glm::ivec4& destViewport) {
    return evalSubregionTexcoordTransformCoefficients(sourceSurface, glm::ivec2(destViewport.z, destViewport.w), glm::ivec2(destViewport.x, destViewport.y));
}

Transform Framebuffer::evalSubregionTexcoordTransform(const glm::ivec2& sourceSurface, const glm::ivec2& destRegionSize, const glm::ivec2& destRegionOffset) {
    float sMin = destRegionOffset.x / (float)sourceSurface.x;
    float sWidth = destRegionSize.x / (float)sourceSurface.x;
    float tMin = destRegionOffset.y / (float)sourceSurface.y;
    float tHeight = destRegionSize.y / (float)sourceSurface.y;
    Transform model;
    model.setTranslation(glm::vec3(sMin, tMin, 0.0));
    model.setScale(glm::vec3(sWidth, tHeight, 1.0));
    return model;
}
Transform Framebuffer::evalSubregionTexcoordTransform(const glm::ivec2& sourceSurface, const glm::ivec4& destViewport) {
    return evalSubregionTexcoordTransform(sourceSurface, glm::ivec2(destViewport.z, destViewport.w), glm::ivec2(destViewport.x, destViewport.y));
}
