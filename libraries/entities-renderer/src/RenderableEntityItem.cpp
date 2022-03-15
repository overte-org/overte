//
//  RenderableEntityItem.cpp
//  interface/src
//
//  Created by Brad Hefta-Gaub on 12/6/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//


#include "RenderableEntityItem.h"

#include <glm/gtx/transform.hpp>
#include <ObjectMotionState.h>

#include "RenderableShapeEntityItem.h"
#include "RenderableModelEntityItem.h"
#include "RenderableTextEntityItem.h"
#include "RenderableImageEntityItem.h"
#include "RenderableWebEntityItem.h"
#include "RenderableParticleEffectEntityItem.h"
#include "RenderableLineEntityItem.h"
#include "RenderablePolyLineEntityItem.h"
#include "RenderablePolyVoxEntityItem.h"
#include "RenderableGridEntityItem.h"
#include "RenderableGizmoEntityItem.h"
#include "RenderableLightEntityItem.h"
#include "RenderableZoneEntityItem.h"
#include "RenderableMaterialEntityItem.h"

#include "RenderPipelines.h"

using namespace render;
using namespace render::entities;

void EntityRenderer::initEntityRenderers() {
    REGISTER_ENTITY_TYPE_WITH_FACTORY(Model, RenderableModelEntityItem::factory)
    REGISTER_ENTITY_TYPE_WITH_FACTORY(PolyVox, RenderablePolyVoxEntityItem::factory)
}

const Transform& EntityRenderer::getModelTransform() const {
    return _modelTransform;
}

void EntityRenderer::makeStatusGetters(const EntityItemPointer& entity, Item::Status::Getters& statusGetters) {
    auto nodeList = DependencyManager::get<NodeList>();
    // DANGER: nodeList->getSessionUUID() will return null id when not connected to domain.
    const QUuid& myNodeID = nodeList->getSessionUUID();

    statusGetters.push_back([entity]() -> render::Item::Status::Value {
        uint64_t delta = usecTimestampNow() - entity->getLastEditedFromRemote();
        const float WAIT_THRESHOLD_INV = 1.0f / (0.2f * USECS_PER_SECOND);
        float normalizedDelta = delta * WAIT_THRESHOLD_INV;
        // Status icon will scale from 1.0f down to 0.0f after WAIT_THRESHOLD
        // Color is red if last update is after WAIT_THRESHOLD, green otherwise (120 deg is green)
        return render::Item::Status::Value(1.0f - normalizedDelta, (normalizedDelta > 1.0f ?
            render::Item::Status::Value::GREEN :
            render::Item::Status::Value::RED),
            (unsigned char)render::Item::Status::Icon::PACKET_RECEIVED);
    });

    statusGetters.push_back([entity] () -> render::Item::Status::Value {
        uint64_t delta = usecTimestampNow() - entity->getLastBroadcast();
        const float WAIT_THRESHOLD_INV = 1.0f / (0.4f * USECS_PER_SECOND);
        float normalizedDelta = delta * WAIT_THRESHOLD_INV;
        // Status icon will scale from 1.0f down to 0.0f after WAIT_THRESHOLD
        // Color is Magenta if last update is after WAIT_THRESHOLD, cyan otherwise (180 deg is green)
        return render::Item::Status::Value(1.0f - normalizedDelta, (normalizedDelta > 1.0f ?
            render::Item::Status::Value::MAGENTA :
            render::Item::Status::Value::CYAN),
            (unsigned char)render::Item::Status::Icon::PACKET_SENT);
    });

    statusGetters.push_back([entity] () -> render::Item::Status::Value {
        ObjectMotionState* motionState = static_cast<ObjectMotionState*>(entity->getPhysicsInfo());
        if (motionState && motionState->isActive()) {
            return render::Item::Status::Value(1.0f, render::Item::Status::Value::BLUE,
                (unsigned char)render::Item::Status::Icon::ACTIVE_IN_BULLET);
        }
        return render::Item::Status::Value(0.0f, render::Item::Status::Value::BLUE,
            (unsigned char)render::Item::Status::Icon::ACTIVE_IN_BULLET);
    });

    statusGetters.push_back([entity, myNodeID] () -> render::Item::Status::Value {
        bool weOwnSimulation = entity->getSimulationOwner().matchesValidID(myNodeID);
        bool otherOwnSimulation = !weOwnSimulation && !entity->getSimulationOwner().isNull();

        if (weOwnSimulation) {
            return render::Item::Status::Value(1.0f, render::Item::Status::Value::BLUE,
                (unsigned char)render::Item::Status::Icon::SIMULATION_OWNER);
        } else if (otherOwnSimulation) {
            return render::Item::Status::Value(1.0f, render::Item::Status::Value::RED,
                (unsigned char)render::Item::Status::Icon::OTHER_SIMULATION_OWNER);
        }
        return render::Item::Status::Value(0.0f, render::Item::Status::Value::BLUE,
            (unsigned char)render::Item::Status::Icon::SIMULATION_OWNER);
    });

    statusGetters.push_back([entity] () -> render::Item::Status::Value {
        if (entity->hasActions()) {
            return render::Item::Status::Value(1.0f, render::Item::Status::Value::GREEN,
                (unsigned char)render::Item::Status::Icon::HAS_ACTIONS);
        }
        return render::Item::Status::Value(0.0f, render::Item::Status::Value::GREEN,
            (unsigned char)render::Item::Status::Icon::HAS_ACTIONS);
    });

    statusGetters.push_back([entity] () -> render::Item::Status::Value {
        if (entity->isAvatarEntity()) {
            if (entity->isMyAvatarEntity()) {
                return render::Item::Status::Value(1.0f, render::Item::Status::Value::GREEN,
                    (unsigned char)render::Item::Status::Icon::ENTITY_HOST_TYPE);
            } else {
                return render::Item::Status::Value(1.0f, render::Item::Status::Value::RED,
                    (unsigned char)render::Item::Status::Icon::ENTITY_HOST_TYPE);
            }
        } else if (entity->isLocalEntity()) {
            return render::Item::Status::Value(1.0f, render::Item::Status::Value::BLUE,
                (unsigned char)render::Item::Status::Icon::ENTITY_HOST_TYPE);
        }
        return render::Item::Status::Value(0.0f, render::Item::Status::Value::GREEN,
            (unsigned char)render::Item::Status::Icon::ENTITY_HOST_TYPE);
    });
}


template <typename T> 
std::shared_ptr<T> make_renderer(const EntityItemPointer& entity) {
    // We want to use deleteLater so that renderer destruction gets pushed to the main thread
    return std::shared_ptr<T>(new T(entity), [](T* ptr) { ptr->deleteLater(); });
}

EntityRenderer::EntityRenderer(const EntityItemPointer& entity) : _created(entity->getCreated()), _entity(entity), _entityID(entity->getID()) {}

EntityRenderer::~EntityRenderer() {}

//
// Smart payload proxy members, implementing the payload interface
//

Item::Bound EntityRenderer::getBound(RenderArgs* args) {
    auto bound = _bound;
    if (_billboardMode != BillboardMode::NONE) {
        glm::vec3 dimensions = bound.getScale();
        float max = glm::max(dimensions.x, glm::max(dimensions.y, dimensions.z));
        const float SQRT_2 = 1.41421356237f;
        bound.setScaleStayCentered(glm::vec3(SQRT_2 * max));
    }
    return bound;
}

ShapeKey EntityRenderer::getShapeKey() {
    ShapeKey::Builder builder = ShapeKey::Builder().withOwnPipeline();
    if (_primitiveMode == PrimitiveMode::LINES) {
        builder.withWireframe();
    }
    return builder.build();
}

render::hifi::Tag EntityRenderer::getTagMask() const {
    render::hifi::Tag mask = render::hifi::TAG_NONE;
    mask = (render::hifi::Tag)(mask | (!_cauterized * render::hifi::TAG_MAIN_VIEW));
    mask = (render::hifi::Tag)(mask | (_isVisibleInSecondaryCamera * render::hifi::TAG_SECONDARY_VIEW));
    return mask;
}

render::hifi::Layer EntityRenderer::getHifiRenderLayer() const {
    switch (_renderLayer) {
        case RenderLayer::WORLD:
            return render::hifi::LAYER_3D;
        case RenderLayer::FRONT:
            return render::hifi::LAYER_3D_FRONT;
        case RenderLayer::HUD:
            return render::hifi::LAYER_3D_HUD;
        default:
            return render::hifi::LAYER_3D;
    }
}

ItemKey EntityRenderer::getKey() {
    ItemKey::Builder builder =
        ItemKey::Builder().withTypeShape().withTypeMeta().withTagBits(getTagMask()).withLayer(getHifiRenderLayer());

    if (isTransparent()) {
        builder.withTransparent();
    } else if (_canCastShadow) {
        builder.withShadowCaster();
    }

    if (_cullWithParent) {
        builder.withSubMetaCulled();
    }

    if (_mirrorMode != MirrorMode::NONE) {
        builder.withMirror();
    }

    if (!_visible) {
        builder.withInvisible();
    }

    return builder;
}

uint32_t EntityRenderer::metaFetchMetaSubItems(ItemIDs& subItems) const {
    if (Item::isValidID(_renderItemID)) {
        subItems.emplace_back(_renderItemID);
        return 1;
    }
    return 0;
}

bool EntityRenderer::passesZoneOcclusionTest(const std::unordered_set<QUuid>& containingZones) const {
    if (!_renderWithZones.isEmpty()) {
        if (!containingZones.empty()) {
            for (auto renderWithZone : _renderWithZones) {
                if (containingZones.find(renderWithZone) != containingZones.end()) {
                    return true;
                }
            }
        }
        return false;
    }
    return true;
}

void EntityRenderer::computeMirrorView(ViewFrustum& viewFrustum) const {
    //glm::vec3 mirrorPosition;
    //glm::quat mirrorRotation;
    //withReadLock([&]{
    //    mirrorPosition = _entity->getWorldPosition();
    //    mirrorRotation = _entity->getWorldOrientation();
    //});

    //glm::mat4 mirrorToWorld = glm::translate(mirrorPosition) * glm::mat4_cast(mirrorRotation);
    //glm::mat4 worldToMirror = glm::inverse(mirrorToWorld);

    //// get mirror camera position by reflecting main camera position's z coordinate in mirror space
    //glm::vec3 cameraPosition = viewFrustum.getPosition();
    //glm::quat cameraRotation = viewFrustum.getOrientation();
    //glm::vec3 localCameraPosition = glm::vec3(worldToMirror * glm::vec4(cameraPosition, 1.0f));
    //localCameraPosition.z *= -1.0f;
    //glm::quat localCameraRotation = worldToMirror * glm::mat4_cast(cameraRotation);
    //glm::vec3 localCameraRotationAngles = glm::eulerAngles(localCameraRotation);
    //localCameraRotationAngles.y = M_PI - localCameraRotationAngles.y;

    //viewFrustum.setPosition(mirrorToWorld * glm::vec4(localCameraPosition, 1.0f));
    //viewFrustum.setOrientation(mirrorToWorld * glm::mat4_cast(glm::quat(localCameraRotationAngles)));

    glm::vec3 mirrorPropertiesPosition;
    glm::quat mirrorPropertiesRotation;
    glm::vec3 mirrorPropertiesDimensions;
    withReadLock([&]{
        mirrorPropertiesPosition = _entity->getWorldPosition();
        mirrorPropertiesRotation = _entity->getWorldOrientation();
        mirrorPropertiesDimensions = _entity->getScaledDimensions();
    });

    glm::vec3 halfMirrorPropertiesDimensions = 0.5f * mirrorPropertiesDimensions;

    glm::mat4 worldFromMirrorRotation = glm::mat4_cast(mirrorPropertiesRotation);
    glm::mat4 worldFromMirrorTranslation = glm::translate(mirrorPropertiesPosition);
    glm::mat4 worldFromMirror = worldFromMirrorTranslation * worldFromMirrorRotation;
    glm::mat4 mirrorFromWorld = glm::inverse(worldFromMirror);

    // get mirror camera position by reflecting main camera position's z coordinate in mirror space
    glm::vec3 mainCameraPositionWorld = viewFrustum.getPosition();
    glm::vec3 mainCameraPositionMirror = vec3(mirrorFromWorld * vec4(mainCameraPositionWorld, 1.0f));
    glm::vec3 mirrorCameraPositionMirror = vec3(mainCameraPositionMirror.x, mainCameraPositionMirror.y,
                                                -mainCameraPositionMirror.z);
    glm::vec3 mirrorCameraPositionWorld = vec3(worldFromMirror * vec4(mirrorCameraPositionMirror, 1.0f));

    // get mirror camera rotation by reflecting main camera rotation in mirror space
    // TODO: we are assuming here that UP is world y-axis
    glm::quat mainCameraRotationWorld = viewFrustum.getOrientation();
    glm::mat4 mainCameraRotationMirror = mirrorFromWorld * glm::mat4_cast(mainCameraRotationWorld);
    glm::mat4 mirrorCameraRotationMirror = mainCameraRotationMirror;// * glm::scale(vec3(-1.0f, 1.0f, -1.0f));
    glm::quat mirrorCameraRotationWorld = worldFromMirror * mirrorCameraRotationMirror;

    viewFrustum.setPosition(mirrorCameraPositionWorld);
    viewFrustum.setOrientation(mirrorCameraRotationWorld);

    // build frustum using mirror space translation of mirrored camera
    //float nearClip = mirrorCameraPositionMirror.z + mirrorPropertiesDimensions.z * 2.0f;
    //glm::vec3 upperRight = halfMirrorPropertiesDimensions - mirrorCameraPositionMirror;
    //glm::vec3 bottomLeft = -halfMirrorPropertiesDimensions - mirrorCameraPositionMirror;
    //glm::mat4 frustum = glm::frustum(bottomLeft.x, upperRight.x, bottomLeft.y, upperRight.y, nearClip, viewFrustum.getFarClip());
    //viewFrustum.setProjection(frustum);
}

void EntityRenderer::render(RenderArgs* args) {
    if (!isValidRenderItem()) {
        return;
    }

    if (_visible && (args->_renderMode != RenderArgs::RenderMode::DEFAULT_RENDER_MODE || !_cauterized)) {
        doRender(args);
    }
}

//
// Methods called by the EntityTreeRenderer
//

EntityRenderer::Pointer EntityRenderer::addToScene(EntityTreeRenderer& renderer, const EntityItemPointer& entity, const ScenePointer& scene, Transaction& transaction) {
    EntityRenderer::Pointer result;
    if (!entity) {
        return result;
    }

    using Type = EntityTypes::EntityType_t;
    auto type = entity->getType();
    switch (type) {

        case Type::Shape:
        case Type::Box:
        case Type::Sphere:
            result = make_renderer<ShapeEntityRenderer>(entity);
            break;

        case Type::Model:
            result = make_renderer<ModelEntityRenderer>(entity);
            break;

        case Type::Text:
            result = make_renderer<TextEntityRenderer>(entity);
            break;

        case Type::Image:
            result = make_renderer<ImageEntityRenderer>(entity);
            break;

        case Type::Web:
            if (!nsightActive()) {
                result = make_renderer<WebEntityRenderer>(entity);
            }
            break;

        case Type::ParticleEffect:
            result = make_renderer<ParticleEffectEntityRenderer>(entity);
            break;

        case Type::Line:
            result = make_renderer<LineEntityRenderer>(entity);
            break;

        case Type::PolyLine:
            result = make_renderer<PolyLineEntityRenderer>(entity);
            break;

        case Type::PolyVox:
            result = make_renderer<PolyVoxEntityRenderer>(entity);
            break;

        case Type::Grid:
            result = make_renderer<GridEntityRenderer>(entity);
            break;

        case Type::Gizmo:
            result = make_renderer<GizmoEntityRenderer>(entity);
            break;

        case Type::Light:
            result = make_renderer<LightEntityRenderer>(entity);
            break;

        case Type::Zone:
            result = make_renderer<ZoneEntityRenderer>(entity);
            break;

        case Type::Material:
            result = make_renderer<MaterialEntityRenderer>(entity);
            break;

        default:
            break;
    }

    if (result) {
        result->addToScene(scene, transaction);
    }

    return result;
}

bool EntityRenderer::addToScene(const ScenePointer& scene, Transaction& transaction) {
    _renderItemID = scene->allocateID();
    // Complicated series of trusses
    auto renderPayload = std::make_shared<PayloadProxyInterface::ProxyPayload>(shared_from_this());
    Item::Status::Getters statusGetters;
    makeStatusGetters(_entity, statusGetters);
    renderPayload->addStatusGetters(statusGetters);
    transaction.resetItem(_renderItemID, renderPayload);
    onAddToScene(_entity);
    updateInScene(scene, transaction);
    _entity->bumpAncestorChainRenderableVersion();
    return true;
}

void EntityRenderer::removeFromScene(const ScenePointer& scene, Transaction& transaction) {
    onRemoveFromScene(_entity);
    transaction.removeItem(_renderItemID);
    Item::clearID(_renderItemID);
    _entity->bumpAncestorChainRenderableVersion();
}

void EntityRenderer::updateInScene(const ScenePointer& scene, Transaction& transaction) {
    DETAILED_PROFILE_RANGE(simulation_physics, __FUNCTION__);
    if (!isValidRenderItem()) {
        return;
    }
    _updateTime = usecTimestampNow();

    doRenderUpdateSynchronous(scene, transaction, _entity);
    transaction.updateItem<PayloadProxyInterface>(_renderItemID, [this](PayloadProxyInterface& self) {
        if (!isValidRenderItem()) {
            return;
        }
        // Happens on the render thread.  Classes should use
        doRenderUpdateAsynchronous(_entity);
    });
}

//
// Internal methods
//

// Returns true if the item needs to have updateInscene called because of internal rendering 
// changes (animation, fading, etc)
bool EntityRenderer::needsRenderUpdate() const {
    if (isFading()) {
        return true;
    }

    if (_prevIsTransparent != isTransparent()) {
        return true;
    }

    return needsRenderUpdateFromEntity(_entity);
}

Transform EntityRenderer::getTransformToCenterWithMaybeOnlyLocalRotation(const EntityItemPointer& entity, bool& success) const {
    return entity->getBillboardMode() == BillboardMode::NONE ? entity->getTransformToCenter(success) :
        entity->getTransformToCenterWithOnlyLocalRotation(success);
}

// Returns true if the item in question needs to have updateInScene called because of changes in the entity
bool EntityRenderer::needsRenderUpdateFromEntity(const EntityItemPointer& entity) const {
    if (entity->needsRenderUpdate()) {
        return true;
    }

    if (!entity->isVisuallyReady()) {
        return true;
    }

    bool success = false;
    auto bound = entity->getAABox(success);
    if (success && _bound != bound) {
        return true;
    }

    auto newModelTransform = getTransformToCenterWithMaybeOnlyLocalRotation(entity, success);
    // FIXME can we use a stale model transform here?
    if (success && newModelTransform != _modelTransform) {
        return true;
    }

    if (_moving != entity->isMovingRelativeToParent()) {
        return true;
    }

    if (entity->needsZoneOcclusionUpdate()) {
        return true;
    }

    return false;
}

void EntityRenderer::updateModelTransformAndBound(const EntityItemPointer& entity) {
    bool success = false;
    auto newModelTransform = getTransformToCenterWithMaybeOnlyLocalRotation(entity, success);
    if (success) {
        _modelTransform = newModelTransform;
    }

    success = false;
    auto bound = entity->getAABox(success);
    if (success) {
        _bound = bound;
    }
}

void EntityRenderer::doRenderUpdateSynchronous(const ScenePointer& scene, Transaction& transaction, const EntityItemPointer& entity) {
    DETAILED_PROFILE_RANGE(simulation_physics, __FUNCTION__);
    withWriteLock([&] {
        auto transparent = isTransparent();
        auto fading = isFading();
        if (fading || _prevIsTransparent != transparent || !entity->isVisuallyReady()) {
            emit requestRenderUpdate();
        }
        if (fading) {
            _isFading = Interpolate::calculateFadeRatio(_fadeStartTime) < 1.0f;
        }

        _prevIsTransparent = transparent;

        updateModelTransformAndBound(entity);

        _moving = entity->isMovingRelativeToParent();
        _visible = entity->getVisible();
        entity->setNeedsRenderUpdate(false);
    });
}

void EntityRenderer::doRenderUpdateAsynchronous(const EntityItemPointer& entity) {
    setIsVisibleInSecondaryCamera(entity->isVisibleInSecondaryCamera());
    setRenderLayer(entity->getRenderLayer());
    _billboardMode = entity->getBillboardMode();
    _primitiveMode = entity->getPrimitiveMode();
    _canCastShadow = entity->getCanCastShadow();
    setCullWithParent(entity->getCullWithParent());
    _cauterized = entity->getCauterized();
    setMirrorMode(entity->getMirrorMode());
    setPortalExitID(entity->getPortalExitID());
    if (entity->needsZoneOcclusionUpdate()) {
        entity->resetNeedsZoneOcclusionUpdate();
        _renderWithZones = entity->getRenderWithZones();
    }
}

void EntityRenderer::onAddToScene(const EntityItemPointer& entity) {
    QObject::connect(this, &EntityRenderer::requestRenderUpdate, this, [this] {
        auto renderer = DependencyManager::get<EntityTreeRenderer>();
        if (renderer) {
            renderer->onEntityChanged(_entity->getID());
        }
    }, Qt::QueuedConnection);
    _changeHandlerId = entity->registerChangeHandler([](const EntityItemID& changedEntity) {
        auto renderer = DependencyManager::get<EntityTreeRenderer>();
        if (renderer) {
            auto renderable = renderer->renderableForEntityId(changedEntity);
            if (renderable && renderable->needsRenderUpdate()) {
                renderer->onEntityChanged(changedEntity);
            }
        }
    });
}

void EntityRenderer::onRemoveFromScene(const EntityItemPointer& entity) { 
    entity->deregisterChangeHandler(_changeHandlerId);
    QObject::disconnect(this, &EntityRenderer::requestRenderUpdate, this, nullptr);
}

void EntityRenderer::addMaterial(graphics::MaterialLayer material, const std::string& parentMaterialName) {
    std::lock_guard<std::mutex> lock(_materialsLock);
    _materials[parentMaterialName].push(material);
    emit requestRenderUpdate();
}

void EntityRenderer::removeMaterial(graphics::MaterialPointer material, const std::string& parentMaterialName) {
    std::lock_guard<std::mutex> lock(_materialsLock);
    _materials[parentMaterialName].remove(material);
    emit requestRenderUpdate();
}

graphics::MaterialPointer EntityRenderer::getTopMaterial() {
    std::lock_guard<std::mutex> lock(_materialsLock);
    auto materials = _materials.find("0");
    if (materials != _materials.end()) {
        return materials->second.top().material;
    }
    return nullptr;
}

EntityRenderer::Pipeline EntityRenderer::getPipelineType(const graphics::MultiMaterial& materials) {
    if (_mirrorMode != MirrorMode::NONE) {
        return Pipeline::MIRROR;
    }

    if (materials.top().material && materials.top().material->isProcedural() && materials.top().material->isReady()) {
        return Pipeline::PROCEDURAL;
    }

    graphics::MaterialKey drawMaterialKey = materials.getMaterialKey();
    if (drawMaterialKey.isEmissive() || drawMaterialKey.isMetallic() || drawMaterialKey.isScattering()) {
        return Pipeline::MATERIAL;
    }

    // If the material is using any map, we need to use a material ShapeKey
    for (int i = 0; i < graphics::Material::MapChannel::NUM_MAP_CHANNELS; i++) {
        if (drawMaterialKey.isMapChannel(graphics::Material::MapChannel(i))) {
            return Pipeline::MATERIAL;
        }
    }
    return Pipeline::SIMPLE;
}

bool EntityRenderer::needsRenderUpdateFromMaterials() const {
    MaterialMap::const_iterator materials;
    {
        std::lock_guard<std::mutex> lock(_materialsLock);
        materials = _materials.find("0");

        if (materials == _materials.cend()) {
            return false;
        }
    }

    if (materials->second.shouldUpdate()) {
        return true;
    }

    if (materials->second.top().material && materials->second.top().material->isProcedural() && materials->second.top().material->isReady()) {
        auto procedural = std::static_pointer_cast<graphics::ProceduralMaterial>(materials->second.top().material);
        if (procedural->isFading()) {
            return true;
        }
    }

    return false;
}

void EntityRenderer::updateMaterials(bool baseMaterialChanged) {
    MaterialMap::iterator materials;
    {
        std::lock_guard<std::mutex> lock(_materialsLock);
        materials = _materials.find("0");

        if (materials == _materials.end()) {
            return;
        }
    }

    if (baseMaterialChanged) {
        materials->second.setNeedsUpdate(true);
    }

    bool requestUpdate = false;
    if (materials->second.top().material && materials->second.top().material->isProcedural() && materials->second.top().material->isReady()) {
        auto procedural = std::static_pointer_cast<graphics::ProceduralMaterial>(materials->second.top().material);
        if (procedural->isFading()) {
            procedural->setIsFading(Interpolate::calculateFadeRatio(procedural->getFadeStartTime()) < 1.0f);
            requestUpdate = true;
        }
    }

    if (materials->second.shouldUpdate()) {
        RenderPipelines::updateMultiMaterial(materials->second);
        requestUpdate = true;
    }

    if (requestUpdate) {
        emit requestRenderUpdate();
    }
}

bool EntityRenderer::materialsTransparent() const {
    MaterialMap::const_iterator materials;
    {
        std::lock_guard<std::mutex> lock(_materialsLock);
        materials = _materials.find("0");

        if (materials == _materials.cend()) {
            return false;
        }
    }

    if (materials->second.top().material) {
        if (materials->second.top().material->isProcedural() && materials->second.top().material->isReady()) {
            auto procedural = std::static_pointer_cast<graphics::ProceduralMaterial>(materials->second.top().material);
            if (procedural->isFading()) {
                return true;
            }
        }

        if (materials->second.getMaterialKey().isTranslucent()) {
            return true;
        }
    }

    return false;
}

Item::Bound EntityRenderer::getMaterialBound(RenderArgs* args) {
    MaterialMap::iterator materials;
    {
        std::lock_guard<std::mutex> lock(_materialsLock);
        materials = _materials.find("0");

        if (materials == _materials.end()) {
            return EntityRenderer::getBound(args);
        }
    }

    if (materials->second.top().material && materials->second.top().material->isProcedural() && materials->second.top().material->isReady()) {
        auto procedural = std::static_pointer_cast<graphics::ProceduralMaterial>(materials->second.top().material);
        if (procedural->hasVertexShader() && procedural->hasBoundOperator()) {
            return procedural->getBound(args);
        }
    }

    return EntityRenderer::getBound(args);
}

void EntityRenderer::updateShapeKeyBuilderFromMaterials(ShapeKey::Builder& builder) {
    MaterialMap::iterator materials;
    {
        std::lock_guard<std::mutex> lock(_materialsLock);
        materials = _materials.find("0");

        if (materials != _materials.end()) {
            if (materials->second.shouldUpdate()) {
                RenderPipelines::updateMultiMaterial(materials->second);
            }
        } else {
            return;
        }
    }

    if (isTransparent()) {
        builder.withTranslucent();
    }

    if (_primitiveMode == PrimitiveMode::LINES) {
        builder.withWireframe();
    }

    builder.withCullFaceMode(materials->second.getCullFaceMode());

    graphics::MaterialKey drawMaterialKey = materials->second.getMaterialKey();
    if (drawMaterialKey.isUnlit()) {
        builder.withUnlit();
    }

    auto pipelineType = getPipelineType(materials->second);
    if (pipelineType == Pipeline::MATERIAL) {
        builder.withMaterial();
        if (drawMaterialKey.isNormalMap()) {
            builder.withTangents();
        }
        if (drawMaterialKey.isLightMap()) {
            builder.withLightMap();
        }
    } else if (pipelineType == Pipeline::PROCEDURAL) {
        builder.withOwnPipeline();
    }
}

glm::vec4 EntityRenderer::calculatePulseColor(const glm::vec4& color, const PulsePropertyGroup& pulseProperties, quint64 start) {
    if (pulseProperties.getPeriod() == 0.0f || (pulseProperties.getColorMode() == PulseMode::NONE && pulseProperties.getAlphaMode() == PulseMode::NONE)) {
        return color;
    }

    float t = ((float)(usecTimestampNow() - start)) / ((float)USECS_PER_SECOND);
    float pulse = 0.5f * (cosf(t * (2.0f * (float)M_PI) / pulseProperties.getPeriod()) + 1.0f) * (pulseProperties.getMax() - pulseProperties.getMin()) + pulseProperties.getMin();
    float outPulse = (1.0f - pulse);

    glm::vec4 result = color;
    if (pulseProperties.getColorMode() == PulseMode::IN_PHASE) {
        result.r *= pulse;
        result.g *= pulse;
        result.b *= pulse;
    } else if (pulseProperties.getColorMode() == PulseMode::OUT_PHASE) {
        result.r *= outPulse;
        result.g *= outPulse;
        result.b *= outPulse;
    }

    if (pulseProperties.getAlphaMode() == PulseMode::IN_PHASE) {
        result.a *= pulse;
    } else if (pulseProperties.getAlphaMode() == PulseMode::OUT_PHASE) {
        result.a *= outPulse;
    }

    return result;
}

glm::vec3 EntityRenderer::calculatePulseColor(const glm::vec3& color, const PulsePropertyGroup& pulseProperties, quint64 start) {
    if (pulseProperties.getPeriod() == 0.0f || (pulseProperties.getColorMode() == PulseMode::NONE && pulseProperties.getAlphaMode() == PulseMode::NONE)) {
        return color;
    }

    float t = ((float)(usecTimestampNow() - start)) / ((float)USECS_PER_SECOND);
    float pulse = 0.5f * (cosf(t * (2.0f * (float)M_PI) / pulseProperties.getPeriod()) + 1.0f) * (pulseProperties.getMax() - pulseProperties.getMin()) + pulseProperties.getMin();
    float outPulse = (1.0f - pulse);

    glm::vec3 result = color;
    if (pulseProperties.getColorMode() == PulseMode::IN_PHASE) {
        result.r *= pulse;
        result.g *= pulse;
        result.b *= pulse;
    } else if (pulseProperties.getColorMode() == PulseMode::OUT_PHASE) {
        result.r *= outPulse;
        result.g *= outPulse;
        result.b *= outPulse;
    }

    return result;
}
