//
//  Created by Ada <ada@thingvellir.net> on 2025-02-24
//  Copyright 2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "RenderableCanvasEntityItem.h"

#include <DependencyManager.h>
#include <GeometryCache.h>
#include <graphics/ShaderConstants.h>

using namespace render;
using namespace render::entities;

CanvasEntityRenderer::CanvasEntityRenderer(const EntityItemPointer& entity) : Parent(entity) {
    _geometryId = DependencyManager::get<GeometryCache>()->allocateID();
}

CanvasEntityRenderer::~CanvasEntityRenderer() {
    auto geometryCache = DependencyManager::get<GeometryCache>();
    if (geometryCache) {
        geometryCache->releaseID(_geometryId);
    }
}

void CanvasEntityRenderer::doRenderUpdateAsynchronousTyped(const TypedEntityPointer& entity) {
    _unlit = entity->getUnlit();

    if (entity->_imageDataDirty.load()) {
        auto texture = gpu::Texture::createStrict(gpu::Element::COLOR_SRGBA_32, entity->getWidth(), entity->getHeight(), 1, gpu::Sampler(gpu::Sampler::FILTER_MIN_MAG_LINEAR));
        texture->setSource("CanvasEntityRenderer");

        const std::lock_guard<std::recursive_mutex> dataLock(entity->_imageDataMutex);
        const auto& data = entity->getImageData();

        texture->assignStoredMip(0, data.length(), reinterpret_cast<const uint8_t*>(data.constData()));
        _texture = texture;

        entity->_imageDataDirty.store(false);
    }
}

void CanvasEntityRenderer::doRenderUpdateSynchronousTyped(const ScenePointer& scene, Transaction& transaction, const TypedEntityPointer& entity) {
    void* key = (void*)this;
    AbstractViewStateInterface::instance()->pushPostUpdateLambda(key, [this, entity] {
        withWriteLock([&] {
            _renderTransform = getModelTransform();
            _renderTransform.postScale(entity->getScaledDimensions());
        });
    });
}

void CanvasEntityRenderer::doRender(RenderArgs* args) {
    PerformanceTimer perfTimer("RenderableCanvasEntityItem::render");
    Q_ASSERT(args->_batch);

    if (!_texture) { return; }

    Transform transform;
    withReadLock([&] {
        transform = _renderTransform;
    });

    gpu::Batch& batch = *args->_batch;

    batch.setResourceTexture(0, _texture);

    bool usePrimaryFrustum = args->_renderMode == RenderArgs::RenderMode::SHADOW_RENDER_MODE || args->_mirrorDepth > 0;
    transform.setRotation(BillboardModeHelpers::getBillboardRotation(transform.getTranslation(), transform.getRotation(), _billboardMode,
        usePrimaryFrustum ? BillboardModeHelpers::getPrimaryViewFrustumPosition() : args->getViewFrustum().getPosition()));
    batch.setModelTransform(transform, _prevRenderTransform);
    if (args->_renderMode == Args::RenderMode::DEFAULT_RENDER_MODE || args->_renderMode == Args::RenderMode::MIRROR_RENDER_MODE) {
        _prevRenderTransform = transform;
    }

    DependencyManager::get<GeometryCache>()->bindSimpleProgram(batch, true, true, _unlit, false, false, true, graphics::MaterialKey::CullFaceMode::CULL_NONE);
    DependencyManager::get<GeometryCache>()->renderQuad(
        batch, glm::vec2(-0.5f), glm::vec2(0.5f), glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 0.0f),
        glm::vec4(1.0f), _geometryId
    );

    batch.setResourceTexture(0, nullptr);
}
