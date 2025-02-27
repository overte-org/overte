//
//  Created by Bradley Austin Davis on 2016/05/09
//  Copyright 2013 High Fidelity, Inc.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "RenderableShapeEntityItem.h"

#include <glm/gtx/quaternion.hpp>

#include <gpu/Batch.h>
#include <DependencyManager.h>
#include <GeometryCache.h>
#include <PerfStat.h>

#include "RenderPipelines.h"

using namespace render;
using namespace render::entities;

// Sphere entities should fit inside a cube entity of the same size, so a sphere that has dimensions 1x1x1 
// is a half unit sphere.  However, the geometry cache renders a UNIT sphere, so we need to scale down.
static const float SPHERE_ENTITY_SCALE = 0.5f;

ShapeEntityRenderer::ShapeEntityRenderer(const EntityItemPointer& entity) : Parent(entity) {
    addMaterial(graphics::MaterialLayer(_material, 0), "0");
}

bool ShapeEntityRenderer::needsRenderUpdate() const {
    return needsRenderUpdateFromMaterials() || Parent::needsRenderUpdate();
}

void ShapeEntityRenderer::doRenderUpdateSynchronousTyped(const ScenePointer& scene, Transaction& transaction, const TypedEntityPointer& entity) {
    void* key = (void*)this;
    AbstractViewStateInterface::instance()->pushPostUpdateLambda(key, [this, entity] {
        withWriteLock([&] {
            _shape = entity->getShape();
            _renderTransform = getModelTransform(); // contains parent scale, if this entity scales with its parent
            if (_shape == EntityShape::Sphere) {
                _renderTransform.postScale(SPHERE_ENTITY_SCALE);
            }

            _renderTransform.postScale(entity->getUnscaledDimensions());
        });
    });
}

void ShapeEntityRenderer::doRenderUpdateAsynchronousTyped(const TypedEntityPointer& entity) {
    _pulseProperties = entity->getPulseProperties();

    bool materialChanged = false;
    glm::vec3 color = toGlm(entity->getColor());
    if (_color != color) {
        _color = color;
        _material->setAlbedo(color);
        materialChanged = true;
    }

    float alpha = entity->getAlpha();
    if (_alpha != alpha) {
        _alpha = alpha;
        _material->setOpacity(alpha);
        materialChanged = true;
    }

    bool unlit = entity->getUnlit();
    if (_unlit != unlit) {
        _unlit = unlit;
        _material->setUnlit(unlit);
        materialChanged = true;
    }

    auto userData = entity->getUserData();
    if (_proceduralData != userData) {
        _proceduralData = userData;
        _material->setProceduralData(_proceduralData);
        materialChanged = true;
    }

    updateMaterials(materialChanged);
}

bool ShapeEntityRenderer::isTransparent() const {
    return _pulseProperties.getAlphaMode() != PulseMode::NONE || Parent::isTransparent() || materialsTransparent();
}

Item::Bound ShapeEntityRenderer::getBound(RenderArgs* args) {
    return Parent::getMaterialBound(args);
}

ShapeKey ShapeEntityRenderer::getShapeKey() {
    ShapeKey::Builder builder;
    updateShapeKeyBuilderFromMaterials(builder);
    return builder.build();
}

void ShapeEntityRenderer::doRender(RenderArgs* args) {
    PerformanceTimer perfTimer("RenderableShapeEntityItem::render");
    Q_ASSERT(args->_batch);

    graphics::MultiMaterial materials;
    {
        std::lock_guard<std::mutex> lock(_materialsLock);
        materials = _materials["0"];
    }

    glm::vec4 outColor = materials.getColor();
    outColor = EntityRenderer::calculatePulseColor(outColor, _pulseProperties, _created);

    if (outColor.a == 0.0f) {
        return;
    }

    gpu::Batch& batch = *args->_batch;

    auto geometryCache = DependencyManager::get<GeometryCache>();
    GeometryCache::Shape geometryShape = geometryCache->getShapeForEntityShape((int)_shape);
    Transform transform;
    withReadLock([&] {
        transform = _renderTransform;
    });

    bool wireframe = render::ShapeKey(args->_globalShapeKey).isWireframe() || _primitiveMode == PrimitiveMode::LINES;

    bool usePrimaryFrustum = args->_renderMode == RenderArgs::RenderMode::SHADOW_RENDER_MODE || args->_mirrorDepth > 0;
    transform.setRotation(BillboardModeHelpers::getBillboardRotation(transform.getTranslation(), transform.getRotation(), _billboardMode,
        usePrimaryFrustum ? BillboardModeHelpers::getPrimaryViewFrustumPosition() : args->getViewFrustum().getPosition(),
        _shape < EntityShape::Cube || _shape > EntityShape::Icosahedron));
    batch.setModelTransform(transform, _prevRenderTransform);
    if (args->_renderMode == Args::RenderMode::DEFAULT_RENDER_MODE || args->_renderMode == Args::RenderMode::MIRROR_RENDER_MODE) {
        _prevRenderTransform = transform;
    }

    Pipeline pipelineType = getPipelineType(materials);
    if (pipelineType == Pipeline::PROCEDURAL) {
        auto procedural = std::static_pointer_cast<graphics::ProceduralMaterial>(materials.top().material);
        outColor = procedural->getColor(outColor);
        outColor.a *= procedural->isFading() ? Interpolate::calculateFadeRatio(procedural->getFadeStartTime()) : 1.0f;
        withReadLock([&] {
            procedural->prepare(batch, transform.getTranslation(), transform.getScale(), transform.getRotation(), _created, ProceduralProgramKey(outColor.a < 1.0f));
        });

        const uint32_t compactColor = GeometryCache::toCompactColor(glm::vec4(outColor));
        _colorBuffer->setData(sizeof(compactColor), (const gpu::Byte*) &compactColor);
        if (wireframe) {
            geometryCache->renderWireShape(batch, geometryShape, _colorBuffer);
        } else {
            geometryCache->renderShape(batch, geometryShape, _colorBuffer);
        }
    } else if (pipelineType == Pipeline::SIMPLE) {
        // FIXME, support instanced multi-shape rendering using multidraw indirect
        outColor.a *= _isFading ? Interpolate::calculateFadeRatio(_fadeStartTime) : 1.0f;
        bool forward = _renderLayer != RenderLayer::WORLD || args->_renderMethod == Args::RenderMethod::FORWARD;
        if (outColor.a >= 1.0f) {
            render::ShapePipelinePointer pipeline = geometryCache->getShapePipelinePointer(false, wireframe || materials.top().material->isUnlit(),
                forward, materials.top().material->getCullFaceMode());
            if (wireframe) {
                geometryCache->renderWireShapeInstance(args, batch, geometryShape, outColor, pipeline);
            } else {
                geometryCache->renderSolidShapeInstance(args, batch, geometryShape, outColor, pipeline);
            }
        } else {
            const uint32_t compactColor = GeometryCache::toCompactColor(glm::vec4(outColor));
            _colorBuffer->setData(sizeof(compactColor), (const gpu::Byte*) &compactColor);
            if (wireframe) {
                geometryCache->renderWireShape(batch, geometryShape, _colorBuffer);
            } else {
                geometryCache->renderShape(batch, geometryShape, _colorBuffer);
            }
        }
    } else {
        if (pipelineType == Pipeline::MATERIAL && RenderPipelines::bindMaterials(materials, batch, args->_renderMode, args->_enableTexturing)) {
            args->_details._materialSwitches++;
        }

        const uint32_t compactColor = GeometryCache::toCompactColor(glm::vec4(outColor));
        _colorBuffer->setData(sizeof(compactColor), (const gpu::Byte*) &compactColor);
        geometryCache->renderShape(batch, geometryShape, _colorBuffer);
    }

    const auto triCount = geometryCache->getShapeTriangleCount(geometryShape);
    args->_details._trianglesRendered += (int)triCount;
}

scriptable::ScriptableModelBase ShapeEntityRenderer::getScriptableModel()  {
    scriptable::ScriptableModelBase result;
    auto geometryCache = DependencyManager::get<GeometryCache>();
    auto geometryShape = geometryCache->getShapeForEntityShape((int)_shape);
    glm::vec3 vertexColor;
    {
        std::lock_guard<std::mutex> lock(_materialsLock);
        result.appendMaterials(_materials);
        auto materials = _materials.find("0");
        if (materials != _materials.end()) {
            vertexColor = materials->second.getColor();
        }
    }
    if (auto mesh = geometryCache->meshFromShape(geometryShape, vertexColor)) {
        result.objectID = getEntity()->getID();
        result.append(mesh);
    }
    return result;
}
