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
#include "RenderPipelines.h"

using namespace render;
using namespace render::entities;

CanvasEntityRenderer::CanvasEntityRenderer(const EntityItemPointer& entity) : Parent(entity) {
    _geometryId = DependencyManager::get<GeometryCache>()->allocateID();
    _material->setCullFaceMode(graphics::MaterialKey::CullFaceMode::CULL_NONE);
    _material->setAlbedo(vec3(1.0f), true);
    addMaterial(graphics::MaterialLayer(_material, 0), "0");
}

CanvasEntityRenderer::~CanvasEntityRenderer() {
    auto geometryCache = DependencyManager::get<GeometryCache>();
    if (geometryCache) {
        geometryCache->releaseID(_geometryId);
    }
}

void CanvasEntityRenderer::doRenderUpdateAsynchronousTyped(const TypedEntityPointer& entity) {
    _transparent = entity->getTransparent();
    _pulseProperties = entity->getPulseProperties();

    bool materialChanged = false;
    auto unlit = entity->getUnlit();
    if (_unlit != unlit) {
        _unlit = unlit;
        _material->setUnlit(_unlit);
        materialChanged = true;
    }

    updateMaterials(materialChanged);

    if (entity->_imageDataDirty.load()) {
        const std::lock_guard<std::recursive_mutex> dataLock(entity->_imageDataMutex);
        const auto& data = entity->getImageData();
        auto width = entity->getImageWidth();
        auto height = entity->getImageHeight();

        // TODO: generic sampler properties
        auto texture = gpu::Texture::createStrict(gpu::Element::COLOR_SRGBA_32, width, height, 1);
        texture->setSource("CanvasEntityRenderer");
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

Item::Bound CanvasEntityRenderer::getBound(RenderArgs* args) {
    return Parent::getMaterialBound(args);
}

ShapeKey CanvasEntityRenderer::getShapeKey() {
    auto builder = render::ShapeKey::Builder().withDepthBias();
    updateShapeKeyBuilderFromMaterials(builder);
    return builder.build();
}

bool CanvasEntityRenderer::isTransparent() const {
    bool imageTransparent = _transparent || _pulseProperties.getAlphaMode() != PulseMode::NONE;
    return imageTransparent || Parent::isTransparent() || materialsTransparent();
}

void CanvasEntityRenderer::doRender(RenderArgs* args) {
    PerformanceTimer perfTimer("RenderableCanvasEntityItem::render");
    Q_ASSERT(args->_batch);

    graphics::MultiMaterial materials;
    {
        std::lock_guard<std::mutex> lock(_materialsLock);
        materials = _materials["0"];
    }

    glm::vec4 color = materials.getColor();
    color = EntityRenderer::calculatePulseColor(color, _pulseProperties, _created);

    if (color.a == 0.0f) { return; }

    gpu::Batch& batch = *args->_batch;

    Transform transform;
    bool transparent;
    withReadLock([&] {
        transform = _renderTransform;
        transparent = isTransparent();
    });

    bool usePrimaryFrustum = args->_renderMode == RenderArgs::RenderMode::SHADOW_RENDER_MODE || args->_mirrorDepth > 0;

    transform.setRotation(BillboardModeHelpers::getBillboardRotation(transform.getTranslation(), transform.getRotation(), _billboardMode,
        usePrimaryFrustum ? BillboardModeHelpers::getPrimaryViewFrustumPosition() : args->getViewFrustum().getPosition()));
    batch.setModelTransform(transform, _prevRenderTransform);

    if (args->_renderMode == Args::RenderMode::DEFAULT_RENDER_MODE || args->_renderMode == Args::RenderMode::MIRROR_RENDER_MODE) {
        _prevRenderTransform = transform;
    }

    Pipeline pipelineType = getPipelineType(materials);
    if (pipelineType == Pipeline::PROCEDURAL) {
        auto procedural = std::static_pointer_cast<graphics::ProceduralMaterial>(materials.top().material);
        transparent |= procedural->isFading();
        procedural->prepare(batch, transform.getTranslation(), transform.getScale(), transform.getRotation(), _created, ProceduralProgramKey(transparent));
    } else if (pipelineType == Pipeline::SIMPLE) {
        batch.setResourceTexture(0, _texture);
    } else if (pipelineType == Pipeline::MATERIAL) {
        if (RenderPipelines::bindMaterials(materials, batch, args->_renderMode, args->_enableTexturing)) {
            args->_details._materialSwitches++;
        }
    }

    DependencyManager::get<GeometryCache>()->renderQuad(
        batch, glm::vec2(-0.5f), glm::vec2(0.5f), glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 0.0f),
        color, _geometryId
    );

    if (pipelineType == Pipeline::SIMPLE) {
        // we have to reset this to white for other simple shapes
        batch.setResourceTexture(graphics::slot::texture::Texture::MaterialAlbedo, DependencyManager::get<TextureCache>()->getWhiteTexture());
    }
}
