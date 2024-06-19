//
//  Overlays.cpp
//  interface/src/ui/overlays
//
//  Copyright 2014 High Fidelity, Inc.
//  Copyright 2022-2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "Overlays.h"

#include <limits>

#include <shared/QtHelpers.h>
#include <OffscreenUi.h>
#include <render/Scene.h>
#include <RegisteredMetaTypes.h>

#include "Application.h"
#include "InterfaceLogging.h"

#include "ImageOverlay.h"
#include "TextOverlay.h"
#include "RectangleOverlay.h"

#include <raypick/RayPick.h>
#include <PointerManager.h>
#include <raypick/MouseTransformNode.h>
#include <PickManager.h>
#include <ScriptEngine.h>
#include <ScriptEngineCast.h>

#include <RenderableWebEntityItem.h>
#include "VariantMapToScriptValue.h"

#include "ui/Keyboard.h"
#include <QtQuick/QQuickWindow>

Q_LOGGING_CATEGORY(trace_render_overlays, "trace.render.overlays")

std::unordered_map<QString, QString> Overlays::_entityToOverlayTypes;
std::unordered_map<QString, QString> Overlays::_overlayToEntityTypes;

STATIC_SCRIPT_TYPES_INITIALIZER((+[](ScriptManager* manager){
    auto scriptEngine = manager->engine().get();

    scriptRegisterMetaType<RayToOverlayIntersectionResult, RayToOverlayIntersectionResultToScriptValue,
                            RayToOverlayIntersectionResultFromScriptValue>(scriptEngine);
}));

Overlays::Overlays() {
    ADD_TYPE_MAP(Box, cube);
    ADD_TYPE_MAP(Sphere, sphere);
    _overlayToEntityTypes["rectangle3d"] = "Shape";
    ADD_TYPE_MAP(Shape, shape);
    ADD_TYPE_MAP(Model, model);
    ADD_TYPE_MAP(Text, text3d);
    ADD_TYPE_MAP(Image, image3d);
    _overlayToEntityTypes["billboard"] = "Image";
    ADD_TYPE_MAP(Web, web3d);
    ADD_TYPE_MAP(PolyLine, line3d);
    ADD_TYPE_MAP(Grid, grid);
    ADD_TYPE_MAP(Gizmo, circle3d);
}

void Overlays::cleanupAllOverlays() {
    _shuttingDown = true;
    QMap<QUuid, Overlay::Pointer> overlays;
    {
        QMutexLocker locker(&_mutex);
        overlays.swap(_overlays);
    }

    foreach(Overlay::Pointer overlay, overlays) {
        _overlaysToDelete.push_back(overlay);
    }
    cleanupOverlaysToDelete();
}

void Overlays::init() {
    auto entityScriptingInterface = DependencyManager::get<EntityScriptingInterface>().data();
    auto pointerManager = DependencyManager::get<PointerManager>().data();
    connect(pointerManager, &PointerManager::hoverBeginOverlay, entityScriptingInterface , &EntityScriptingInterface::hoverEnterEntity);
    connect(pointerManager, &PointerManager::hoverContinueOverlay, entityScriptingInterface, &EntityScriptingInterface::hoverOverEntity);
    connect(pointerManager, &PointerManager::hoverEndOverlay, entityScriptingInterface, &EntityScriptingInterface::hoverLeaveEntity);
    connect(pointerManager, &PointerManager::triggerBeginOverlay, entityScriptingInterface, &EntityScriptingInterface::mousePressOnEntity);
    connect(pointerManager, &PointerManager::triggerContinueOverlay, entityScriptingInterface, &EntityScriptingInterface::mouseMoveOnEntity);
    connect(pointerManager, &PointerManager::triggerEndOverlay, entityScriptingInterface, &EntityScriptingInterface::mouseReleaseOnEntity);

    connect(entityScriptingInterface, &EntityScriptingInterface::mousePressOnEntity, this, &Overlays::mousePressOnPointerEvent);
    connect(entityScriptingInterface, &EntityScriptingInterface::mousePressOffEntity, this, &Overlays::mousePressOffPointerEvent);
    connect(entityScriptingInterface, &EntityScriptingInterface::mouseDoublePressOnEntity, this, &Overlays::mouseDoublePressOnPointerEvent);
    connect(entityScriptingInterface, &EntityScriptingInterface::mouseDoublePressOffEntity, this, &Overlays::mouseDoublePressOffPointerEvent);
    connect(entityScriptingInterface, &EntityScriptingInterface::mouseReleaseOnEntity, this, &Overlays::mouseReleasePointerEvent);
    connect(entityScriptingInterface, &EntityScriptingInterface::mouseMoveOnEntity, this, &Overlays::mouseMovePointerEvent);
    connect(entityScriptingInterface, &EntityScriptingInterface::hoverEnterEntity , this, &Overlays::hoverEnterPointerEvent);
    connect(entityScriptingInterface, &EntityScriptingInterface::hoverOverEntity, this, &Overlays::hoverOverPointerEvent);
    connect(entityScriptingInterface, &EntityScriptingInterface::hoverLeaveEntity, this, &Overlays::hoverLeavePointerEvent);
}

void Overlays::update(float deltatime) {
    cleanupOverlaysToDelete();
}

void Overlays::cleanupOverlaysToDelete() {
    if (!_overlaysToDelete.isEmpty()) {
        render::ScenePointer scene = qApp->getMain3DScene();
        render::Transaction transaction;

        {
            do {
                Overlay::Pointer overlay = _overlaysToDelete.takeLast();

                auto itemID = overlay->getRenderItemID();
                if (render::Item::isValidID(itemID)) {
                    overlay->removeFromScene(overlay, scene, transaction);
                }
            } while (!_overlaysToDelete.isEmpty());
        }

        if (transaction.hasRemovedItems()) {
            scene->enqueueTransaction(transaction);
        }
    }
}

void Overlays::render(RenderArgs* renderArgs) {
    PROFILE_RANGE(render_overlays, __FUNCTION__);
    gpu::Batch& batch = *renderArgs->_batch;

    auto geometryCache = DependencyManager::get<GeometryCache>();
    auto textureCache = DependencyManager::get<TextureCache>();

    auto size = glm::uvec2(qApp->getUiSize());
    int width = size.x;
    int height = size.y;
    mat4 legacyProjection = glm::ortho<float>(0, width, height, 0, -1000, 1000);

    QMutexLocker locker(&_mutex);
    foreach(Overlay::Pointer thisOverlay, _overlays) {

        // Reset all batch pipeline settings between overlay
        geometryCache->useSimpleDrawPipeline(batch);
        batch.setResourceTexture(0, textureCache->getWhiteTexture()); // FIXME - do we really need to do this??
        batch.setProjectionTransform(legacyProjection);
        batch.setModelTransform(Transform());
        batch.resetViewTransform();

        thisOverlay->render(renderArgs);
    }
}

Overlay::Pointer Overlays::take2DOverlay(const QUuid& id) {
    if (_shuttingDown) {
        return nullptr;
    }

    QMutexLocker locker(&_mutex);
    auto overlayIter = _overlays.find(id);
    if (overlayIter != _overlays.end()) {
        return _overlays.take(id);
    }
    return nullptr;
}

Overlay::Pointer Overlays::get2DOverlay(const QUuid& id) const {
    if (_shuttingDown) {
        return nullptr;
    }

    QMutexLocker locker(&_mutex);
    auto overlayIter = _overlays.find(id);
    if (overlayIter != _overlays.end()) {
        return overlayIter.value();
    }
    return nullptr;
}

QString Overlays::entityToOverlayType(const QString& type) {
    auto iter = _entityToOverlayTypes.find(type);
    if (iter != _entityToOverlayTypes.end()) {
        return iter->second;
    }
    return "unknown";
}

static QHash<QUuid, std::pair<glm::quat, bool>> savedRotations = QHash<QUuid, std::pair<glm::quat, bool>>();

QUuid Overlays::addOverlay(const QString& type, const QVariant& properties) {
    if (_shuttingDown) {
        return UNKNOWN_ENTITY_ID;
    }

    if (type == ImageOverlay::TYPE || type == TextOverlay::TYPE || type == RectangleOverlay::TYPE) {
#if !defined(DISABLE_QML)
        if (QThread::currentThread() != thread()) {
            QUuid result;
            PROFILE_RANGE(script, __FUNCTION__);
            BLOCKING_INVOKE_METHOD(this, "addOverlay", Q_RETURN_ARG(QUuid, result), Q_ARG(const QString&, type), Q_ARG(const QVariant&, properties));
            return result;
        }

        Overlay::Pointer overlay;
        if (type == ImageOverlay::TYPE) {
            overlay = std::shared_ptr<ImageOverlay>(new ImageOverlay(), [](ImageOverlay* ptr) { ptr->deleteLater(); });
        } else if (type == TextOverlay::TYPE) {
            overlay = std::shared_ptr<TextOverlay>(new TextOverlay(), [](TextOverlay* ptr) { ptr->deleteLater(); });
        } else if (type == RectangleOverlay::TYPE) {
            overlay = std::shared_ptr<RectangleOverlay>(new RectangleOverlay(), [](RectangleOverlay* ptr) { ptr->deleteLater(); });
        }
        if (overlay) {
            overlay->setProperties(properties.toMap());
            return add2DOverlay(overlay);
        }
#endif
        return QUuid();
    }

    return UNKNOWN_ENTITY_ID;
}

QUuid Overlays::add2DOverlay(const Overlay::Pointer& overlay) {
    if (_shuttingDown) {
        return UNKNOWN_ENTITY_ID;
    }

    QUuid thisID = QUuid::createUuid();
    overlay->setID(thisID);
    overlay->setStackOrder(_stackOrder++);
    {
        QMutexLocker locker(&_mutex);
        _overlays[thisID] = overlay;
    }

    return thisID;
}

QUuid Overlays::cloneOverlay(const QUuid& id) {
    if (_shuttingDown) {
        return UNKNOWN_ENTITY_ID;
    }

    Overlay::Pointer overlay = get2DOverlay(id);
    if (overlay) {
        if (QThread::currentThread() != thread()) {
            QUuid result;
            PROFILE_RANGE(script, __FUNCTION__);
            BLOCKING_INVOKE_METHOD(this, "cloneOverlay", Q_RETURN_ARG(QUuid, result), Q_ARG(const QUuid&, id));
            return result;
        }
        return add2DOverlay(Overlay::Pointer(overlay->createClone(), [](Overlay* ptr) { ptr->deleteLater(); }));
    }

    return DependencyManager::get<EntityScriptingInterface>()->cloneEntity(id);
}

bool Overlays::editOverlay(const QUuid& id, const QVariant& properties) {
    if (_shuttingDown) {
        return false;
    }

    auto overlay = get2DOverlay(id);
    if (overlay) {
        if (QThread::currentThread() != thread()) {
            // NOTE editOverlay can be called very frequently in scripts and can't afford to
            // block waiting on the main thread.  Additionally, no script actually
            // examines the return value and does something useful with it, so use a non-blocking
            // invoke and just always return true
            QMetaObject::invokeMethod(this, "editOverlay", Q_ARG(const QUuid&, id), Q_ARG(const QVariant&, properties));
            return true;
        }

        overlay->setProperties(properties.toMap());
        return true;
    }

    return false;
}

bool Overlays::editOverlays(const QVariant& propertiesById) {
    if (_shuttingDown) {
        return false;
    }

    bool deferOverlays = QThread::currentThread() != thread();

    QVariantMap deferred;
    const QVariantMap map = propertiesById.toMap();
    for (const auto& key : map.keys()) {
        QUuid id = QUuid(key);
        const QVariant& properties = map[key];

        Overlay::Pointer overlay = get2DOverlay(id);
        if (overlay) {
            if (deferOverlays) {
                deferred[key] = properties;
                continue;
            }
            overlay->setProperties(properties.toMap());
        } else {
            qDebug() << "Overlays::editOverlays doesn't support editing entities anymore";
        }
    }

    // For 2D/QML overlays, we need to perform the edit on the main thread
    if (!deferred.empty()) {
        // NOTE see comment on editOverlay for why this is not a blocking call
        QMetaObject::invokeMethod(this, "editOverlays", Q_ARG(const QVariant&, deferred));
    }

    return true;
}

void Overlays::deleteOverlay(const QUuid& id) {
    if (_shuttingDown) {
        return;
    }

    Overlay::Pointer overlay = take2DOverlay(id);
    if (overlay) {
        if (QThread::currentThread() != thread()) {
            QMetaObject::invokeMethod(this, "deleteOverlay", Q_ARG(const QUuid&, id));
            return;
        }

        _overlaysToDelete.push_back(overlay);
        emit overlayDeleted(id);
        return;
    }

    DependencyManager::get<EntityScriptingInterface>()->deleteEntity(id);
    emit overlayDeleted(id);
}

QString Overlays::getOverlayType(const QUuid& id) {
    if (_shuttingDown) {
        return "";
    }

    Overlay::Pointer overlay = get2DOverlay(id);
    if (overlay) {
        if (QThread::currentThread() != thread()) {
            QString result;
            PROFILE_RANGE(script, __FUNCTION__);
            BLOCKING_INVOKE_METHOD(this, "getOverlayType", Q_RETURN_ARG(QString, result), Q_ARG(const QUuid&, id));
            return result;
        }
        return overlay->getType();
    }

    return entityToOverlayType(DependencyManager::get<EntityScriptingInterface>()->getEntityType(id));
}

QObject* Overlays::getOverlayObject(const QUuid& id) {
    Overlay::Pointer overlay = get2DOverlay(id);
    if (overlay) {
        if (QThread::currentThread() != thread()) {
            QObject* result;
            PROFILE_RANGE(script, __FUNCTION__);
            BLOCKING_INVOKE_METHOD(this, "getOverlayObject", Q_RETURN_ARG(QObject*, result), Q_ARG(const QUuid&, id));
            return result;
        }
        return qobject_cast<QObject*>(&(*overlay));
    }

    return DependencyManager::get<EntityScriptingInterface>()->getEntityObject(id);
}

QUuid Overlays::getOverlayAtPoint(const glm::vec2& point) {
    if (_shuttingDown) {
        return UNKNOWN_ENTITY_ID;
    }

    if (QThread::currentThread() != thread()) {
        QUuid result;
        BLOCKING_INVOKE_METHOD(this, "getOverlayAtPoint", Q_RETURN_ARG(QUuid, result), Q_ARG(const glm::vec2&, point));
        return result;
    }

    QMutexLocker locker(&_mutex);
    QMapIterator<QUuid, Overlay::Pointer> i(_overlays);
    unsigned int bestStackOrder = 0;
    QUuid bestID = UNKNOWN_ENTITY_ID;
    while (i.hasNext()) {
        i.next();
        auto thisOverlay = std::dynamic_pointer_cast<Overlay2D>(i.value());
        if (thisOverlay && thisOverlay->getVisible() && thisOverlay->getBoundingRect().contains(point.x, point.y, false)) {
            if (thisOverlay->getStackOrder() > bestStackOrder) {
                bestID = i.key();
                bestStackOrder = thisOverlay->getStackOrder();
            }
        }
    }

    return bestID;
}

RayToOverlayIntersectionResult Overlays::findRayIntersection(const PickRay& ray, bool precisionPicking,
                                                             const ScriptValue& overlayIDsToInclude,
                                                             const ScriptValue& overlayIDsToDiscard,
                                                             bool visibleOnly, bool collidableOnly) {
    const QVector<EntityItemID> include = qVectorEntityItemIDFromScriptValue(overlayIDsToInclude);
    const QVector<EntityItemID> discard = qVectorEntityItemIDFromScriptValue(overlayIDsToDiscard);

    return findRayIntersectionVector(ray, precisionPicking, include, discard, visibleOnly, collidableOnly);
}

RayToOverlayIntersectionResult Overlays::findRayIntersectionVector(const PickRay& ray, bool precisionPicking,
                                                                   const QVector<EntityItemID>& include,
                                                                   const QVector<EntityItemID>& discard,
                                                                   bool visibleOnly, bool collidableOnly) {
    unsigned int searchFilter = PickFilter::getBitMask(PickFilter::FlagBit::LOCAL_ENTITIES);

    if (!precisionPicking) {
        searchFilter = searchFilter | PickFilter::getBitMask(PickFilter::FlagBit::COARSE);
    }

    if (visibleOnly) {
        searchFilter = searchFilter | PickFilter::getBitMask(PickFilter::FlagBit::VISIBLE);
    }

    if (collidableOnly) {
        searchFilter = searchFilter | PickFilter::getBitMask(PickFilter::FlagBit::COLLIDABLE);
    }
    auto result = DependencyManager::get<EntityScriptingInterface>()->evalRayIntersectionVector(ray, PickFilter(searchFilter), include, discard);

    RayToOverlayIntersectionResult overlayResult;
    overlayResult.overlayID = result.entityID;
    overlayResult.intersects = result.intersects;
    overlayResult.intersection = result.intersection;
    overlayResult.distance = result.distance;
    overlayResult.surfaceNormal = result.surfaceNormal;
    overlayResult.face = result.face;
    overlayResult.extraInfo = result.extraInfo;
    return overlayResult;
}

ParabolaToOverlayIntersectionResult Overlays::findParabolaIntersectionVector(const PickParabola& parabola, bool precisionPicking,
                                                                             const QVector<EntityItemID>& include,
                                                                             const QVector<EntityItemID>& discard,
                                                                             bool visibleOnly, bool collidableOnly) {
    unsigned int searchFilter = PickFilter::getBitMask(PickFilter::FlagBit::LOCAL_ENTITIES);

    if (!precisionPicking) {
        searchFilter = searchFilter | PickFilter::getBitMask(PickFilter::FlagBit::COARSE);
    }

    if (visibleOnly) {
        searchFilter = searchFilter | PickFilter::getBitMask(PickFilter::FlagBit::VISIBLE);
    }

    if (collidableOnly) {
        searchFilter = searchFilter | PickFilter::getBitMask(PickFilter::FlagBit::COLLIDABLE);
    }
    auto result = DependencyManager::get<EntityScriptingInterface>()->evalParabolaIntersectionVector(parabola, PickFilter(searchFilter), include, discard);

    ParabolaToOverlayIntersectionResult overlayResult;
    overlayResult.overlayID = result.entityID;
    overlayResult.intersects = result.intersects;
    overlayResult.intersection = result.intersection;
    overlayResult.parabolicDistance = result.parabolicDistance;
    overlayResult.surfaceNormal = result.surfaceNormal;
    overlayResult.face = result.face;
    overlayResult.extraInfo = result.extraInfo;
    return overlayResult;
}

ScriptValue RayToOverlayIntersectionResultToScriptValue(ScriptEngine* engine, const RayToOverlayIntersectionResult& value) {
    ScriptValue obj = engine->newObject();
    obj.setProperty("intersects", value.intersects);
    ScriptValue overlayIDValue = quuidToScriptValue(engine, value.overlayID);
    obj.setProperty("overlayID", overlayIDValue);
    obj.setProperty("distance", value.distance);
    obj.setProperty("face", boxFaceToString(value.face));

    ScriptValue intersection = vec3ToScriptValue(engine, value.intersection);
    obj.setProperty("intersection", intersection);
    ScriptValue surfaceNormal = vec3ToScriptValue(engine, value.surfaceNormal);
    obj.setProperty("surfaceNormal", surfaceNormal);
    obj.setProperty("extraInfo", engine->toScriptValue(value.extraInfo));
    return obj;
}

bool RayToOverlayIntersectionResultFromScriptValue(const ScriptValue& object, RayToOverlayIntersectionResult& value) {
    value.intersects = object.property("intersects").toVariant().toBool();
    ScriptValue overlayIDValue = object.property("overlayID");
    quuidFromScriptValue(overlayIDValue, value.overlayID);
    value.distance = object.property("distance").toVariant().toFloat();
    value.face = boxFaceFromString(object.property("face").toVariant().toString());

    ScriptValue intersection = object.property("intersection");
    if (intersection.isValid()) {
        vec3FromScriptValue(intersection, value.intersection);
    }
    ScriptValue surfaceNormal = object.property("surfaceNormal");
    if (surfaceNormal.isValid()) {
        vec3FromScriptValue(surfaceNormal, value.surfaceNormal);
    }
    value.extraInfo = object.property("extraInfo").toVariant().toMap();
    return true;
}

bool Overlays::isLoaded(const QUuid& id) {
    Overlay::Pointer overlay = get2DOverlay(id);
    if (overlay) {
        return true;
    }

    return DependencyManager::get<EntityScriptingInterface>()->isLoaded(id);
}

QSizeF Overlays::textSize(const QUuid& id, const QString& text) {
    Overlay::Pointer overlay = get2DOverlay(id);
    if (overlay) {
        if (QThread::currentThread() != thread()) {
            QSizeF result;
            PROFILE_RANGE(script, __FUNCTION__);
            BLOCKING_INVOKE_METHOD(this, "textSize", Q_RETURN_ARG(QSizeF, result), Q_ARG(const QUuid&, id), Q_ARG(QString, text));
            return result;
        }
        if (auto textOverlay = std::dynamic_pointer_cast<TextOverlay>(overlay)) {
            return textOverlay->textSize(text);
        }
        return QSizeF(0.0f, 0.0f);
    }

    return DependencyManager::get<EntityScriptingInterface>()->textSize(id, text);
}

bool Overlays::isAddedOverlay(const QUuid& id) {
    Overlay::Pointer overlay = get2DOverlay(id);
    if (overlay) {
        return true;
    }

    return DependencyManager::get<EntityScriptingInterface>()->isAddedEntity(id);
}

void Overlays::sendMousePressOnOverlay(const QUuid& id, const PointerEvent& event) {
    mousePressOnPointerEvent(id, event);
}

void Overlays::sendMouseReleaseOnOverlay(const QUuid& id, const PointerEvent& event) {
    mouseReleasePointerEvent(id, event);
}

void Overlays::sendMouseMoveOnOverlay(const QUuid& id, const PointerEvent& event) {
    mouseMovePointerEvent(id, event);
}

void Overlays::sendHoverEnterOverlay(const QUuid& id, const PointerEvent& event) {
    hoverEnterPointerEvent(id, event);
}

void Overlays::sendHoverOverOverlay(const QUuid& id, const PointerEvent& event) {
    hoverOverPointerEvent(id, event);
}

void Overlays::sendHoverLeaveOverlay(const QUuid& id, const PointerEvent& event) {
    hoverLeavePointerEvent(id, event);
}

float Overlays::width() {
    if (QThread::currentThread() != thread()) {
        float result;
        PROFILE_RANGE(script, __FUNCTION__);
        BLOCKING_INVOKE_METHOD(this, "width", Q_RETURN_ARG(float, result));
        return result;
    }

    auto offscreenUI = DependencyManager::get<OffscreenUi>();
    return offscreenUI ? offscreenUI->getWindow()->size().width() : -1.0f;
}

float Overlays::height() {
    if (QThread::currentThread() != thread()) {
        float result;
        PROFILE_RANGE(script, __FUNCTION__);
        BLOCKING_INVOKE_METHOD(this, "height", Q_RETURN_ARG(float, result));
        return result;
    }

    auto offscreenUI = DependencyManager::get<OffscreenUi>();
    return offscreenUI ? offscreenUI->getWindow()->size().height() : -1.0f;
}

void Overlays::mousePressOnPointerEvent(const QUuid& id, const PointerEvent& event) {
    auto keyboard = DependencyManager::get<Keyboard>();
    // Do not send keyboard key event to scripts to prevent malignant scripts from gathering what you typed
    if (!keyboard->getKeyIDs().contains(id)) {
        auto entity = DependencyManager::get<EntityTreeRenderer>()->getEntity(id);
        if (entity && entity->isLocalEntity()) {
            emit mousePressOnOverlay(id, event);
        }
    }
}

void Overlays::mousePressOffPointerEvent() {
    emit mousePressOffOverlay();
}

void Overlays::mouseDoublePressOnPointerEvent(const QUuid& id, const PointerEvent& event) {
    auto keyboard = DependencyManager::get<Keyboard>();
    // Do not send keyboard key event to scripts to prevent malignant scripts from gathering what you typed
    if (!keyboard->getKeyIDs().contains(id)) {
        auto entity = DependencyManager::get<EntityTreeRenderer>()->getEntity(id);
        if (entity && entity->isLocalEntity()) {
            emit mouseDoublePressOnOverlay(id, event);
        }
    }
}

void Overlays::mouseDoublePressOffPointerEvent() {
    emit mouseDoublePressOffOverlay();
}

void Overlays::mouseReleasePointerEvent(const QUuid& id, const PointerEvent& event) {
    auto keyboard = DependencyManager::get<Keyboard>();
    // Do not send keyboard key event to scripts to prevent malignant scripts from gathering what you typed
    if (!keyboard->getKeyIDs().contains(id)) {
        auto entity = DependencyManager::get<EntityTreeRenderer>()->getEntity(id);
        if (entity && entity->isLocalEntity()) {
            emit mouseReleaseOnOverlay(id, event);
        }
    }
}

void Overlays::mouseMovePointerEvent(const QUuid& id, const PointerEvent& event) {
    auto keyboard = DependencyManager::get<Keyboard>();
    // Do not send keyboard key event to scripts to prevent malignant scripts from gathering what you typed
    if (!keyboard->getKeyIDs().contains(id)) {
        auto entity = DependencyManager::get<EntityTreeRenderer>()->getEntity(id);
        if (entity && entity->isLocalEntity()) {
            emit mouseMoveOnOverlay(id, event);
        }
    }
}

void Overlays::hoverEnterPointerEvent(const QUuid& id, const PointerEvent& event) {
    auto keyboard = DependencyManager::get<Keyboard>();
    // Do not send keyboard key event to scripts to prevent malignant scripts from gathering what you typed
    if (!keyboard->getKeyIDs().contains(id)) {
        auto entity = DependencyManager::get<EntityTreeRenderer>()->getEntity(id);
        if (entity && entity->isLocalEntity()) {
            emit hoverEnterOverlay(id, event);
        }
    }
}

void Overlays::hoverOverPointerEvent(const QUuid& id, const PointerEvent& event) {
    auto keyboard = DependencyManager::get<Keyboard>();
    // Do not send keyboard key event to scripts to prevent malignant scripts from gathering what you typed
    if (!keyboard->getKeyIDs().contains(id)) {
        auto entity = DependencyManager::get<EntityTreeRenderer>()->getEntity(id);
        if (entity && entity->isLocalEntity()) {
            emit hoverOverOverlay(id, event);
        }
    }
}

void Overlays::hoverLeavePointerEvent(const QUuid& id, const PointerEvent& event) {
    auto keyboard = DependencyManager::get<Keyboard>();
    // Do not send keyboard key event to scripts to prevent malignant scripts from gathering what you typed
    if (!keyboard->getKeyIDs().contains(id)) {
        auto entity = DependencyManager::get<EntityTreeRenderer>()->getEntity(id);
        if (entity && entity->isLocalEntity()) {
            emit hoverLeaveOverlay(id, event);
        }
    }
}

QVector<QUuid> Overlays::findOverlays(const glm::vec3& center, float radius) {
    PROFILE_RANGE(script_entities, __FUNCTION__);

    QVector<QUuid> result;
    auto entityTree = DependencyManager::get<EntityScriptingInterface>()->getEntityTree();
    if (entityTree) {
        unsigned int searchFilter = PickFilter::getBitMask(PickFilter::FlagBit::LOCAL_ENTITIES);
        // For legacy reasons, this only finds visible objects
        searchFilter = searchFilter | PickFilter::getBitMask(PickFilter::FlagBit::VISIBLE);
        entityTree->withReadLock([&] {
            entityTree->evalEntitiesInSphere(center, radius, PickFilter(searchFilter), result);
        });
    }
    return result;
}

/*@jsdoc
 * <p>An overlay may be one of the following types:</p>
 * <table>
 *   <thead>
 *     <tr><th>Value</th><th>2D/3D</th><th>Description</th><th>Properties</th></tr>
 *   </thead>
 *   <tbody>
 *     <tr><td><code>"rectangle"</code></td><td>2D</td>
 *       <td>A rectangle.</td>
 *       <td>{@link Overlays.OverlayProperties-Rectangle|OverlayProperties-Rectangle}</td></tr>
 *     <tr><td><code>"image"</code></td><td>2D</td>
 *       <td>An image.</td>
 *       <td>{@link Overlays.OverlayProperties-Image|OverlayProperties-Image}</td></tr>
 *     <tr><td><code>"text"</code></td><td>2D</td>
 *       <td>Some text.</td>
 *       <td>{@link Overlays.OverlayProperties-Text|OverlayProperties-Text}</td></tr>
 *     <tr><td><code>"cube"</code></td><td>3D</td>
 *       <td><p>A cube. A <code>"shape"</code> overlay can also be used to create a cube.</p>
 *   </tbody>
 * </table>
 * <p>2D overlays are rendered on the display surface in desktop mode and on the HUD surface in HMD mode. 3D overlays are
 * rendered at a position and orientation in-world.</p>
 * @typedef {string} Overlays.OverlayType
 */

/*@jsdoc
 * Different overlay types have different properties: some common to all overlays (listed in the table) and some specific to 
 * each {@link Overlays.OverlayType|OverlayType} (linked to below).
 * <p>3D overlays are local entities, internally, so they also support the relevant entity's properties.</p>
 * @typedef {object} Overlays.OverlayProperties
 * @property {Uuid} id - The ID of the overlay. <em>Read-only.</em>
 * @property {Overlays.OverlayType} type - The overlay's type. <em>Read-only.</em>
 * @property {boolean} visible=true - <code>true</code> if the overlay is rendered, <code>false</code> if it isn't.
 *
 * @see {@link Overlays.OverlayProperties-Rectangle|OverlayProperties-Rectangle}
 * @see {@link Overlays.OverlayProperties-Image|OverlayProperties-Image}
 * @see {@link Overlays.OverlayProperties-Text|OverlayProperties-Text}
 */

/*@jsdoc
 * The <code>"image"</code> {@link Overlays.OverlayType|OverlayType} is for 2D images.
 * It has properties in addition to the common {@link Overlays.OverlayProperties|OverlayProperties}.
 * @typedef {object} Overlays.OverlayProperties-Image
 * @property {Rect} bounds - The position and size of the image display area, in pixels. <em>Write-only.</em>
 * @property {number} x - Integer left, x-coordinate value of the image display area = <code>bounds.x</code>.
 *     <em>Write-only.</em>
 * @property {number} y - Integer top, y-coordinate value of the image display area = <code>bounds.y</code>.
 *     <em>Write-only.</em>
 * @property {number} width - Integer width of the image display area = <code>bounds.width</code>. <em>Write-only.</em>
 * @property {number} height - Integer height of the image display area = <code>bounds.height</code>. <em>Write-only.</em>
 * @property {string} imageURL - The URL of the image file to display. The image is scaled to fit to the <code>bounds</code>.
 *     <em>Write-only.</em>
 * @property {Rect} subImage - The portion of the image to use. If not specified, the whole image is used.
 *     <em>Write-only.</em>
 * @property {Color} color=0,0,0 - The color to apply over the top of the image to colorize it. <em>Write-only.</em>
 * @property {number} alpha=0.0 - The opacity of the color applied over the top of the image, <code>0.0</code> &ndash;
 *     <code>1.0</code>. <em>Write-only.</em>
 */

/*@jsdoc
 * The <code>"text"</code> {@link Overlays.OverlayType|OverlayType} is for 2D text.
 * It has properties in addition to the common {@link Overlays.OverlayProperties|OverlayProperties}.
 * @typedef {object} Overlays.OverlayProperties-Text
 * @property {Rect} bounds - The position and size of the rectangle, in pixels. <em>Write-only.</em>
 * @property {number} x - Integer left, x-coordinate value = <code>bounds.x</code>. <em>Write-only.</em>
 * @property {number} y - Integer top, y-coordinate value = <code>bounds.y</code>. <em>Write-only.</em>
 * @property {number} width - Integer width of the rectangle = <code>bounds.width</code>. <em>Write-only.</em>
 * @property {number} height - Integer height of the rectangle = <code>bounds.height</code>. <em>Write-only.</em>
 * @property {number} margin=0 - The <code>leftMargin</code> and <code>topMargin</code> values, in pixels.
 *     <em>Write-only.</em>
 * @property {number} leftMargin=0 - The left margin's size, in pixels. This value is also used for the right margin.
 *     <em>Write-only.</em>
 * @property {number} topMargin=0 - The top margin's size, in pixels. This value is also used for the bottom margin.
 *     <em>Write-only.</em>
 * @property {string} text="" - The text to display. Text does not automatically wrap; use <code>"\n"</code> for a line break. 
 *     Text is clipped to the <code>bounds</code>. <em>Write-only.</em>
 * @property {number} font.size=18 - The size of the text, in pixels. <em>Write-only.</em>
 * @property {number} lineHeight=18 - The height of a line of text, in pixels. <em>Write-only.</em>
 * @property {Color} color=255,255,255 - The color of the text. Synonym: <code>textColor</code>. <em>Write-only.</em>
 * @property {number} alpha=1.0 - The opacity of the overlay, <code>0.0</code> &ndash; <code>1.0</code>. <em>Write-only.</em>
 * @property {Color} backgroundColor=0,0,0 - The color of the background rectangle. <em>Write-only.</em>
 * @property {number} backgroundAlpha=0.7 - The opacity of the background rectangle, <code>0.0</code> &ndash; <code>1.0</code>. 
 *     <em>Write-only.</em>
 */

/*@jsdoc
 * The <code>"rectangle"</code> {@link Overlays.OverlayType|OverlayType} is for 2D rectangles.
 * It has properties in addition to the common {@link Overlays.OverlayProperties|OverlayProperties}.
 * @typedef {object} Overlays.OverlayProperties-Rectangle
 * @property {Rect} bounds - The position and size of the rectangle, in pixels. <em>Write-only.</em>
 * @property {number} x - Integer left, x-coordinate value = <code>bounds.x</code>. <em>Write-only.</em>
 * @property {number} y - Integer top, y-coordinate value = <code>bounds.y</code>. <em>Write-only.</em>
 * @property {number} width - Integer width of the rectangle = <code>bounds.width</code>. <em>Write-only.</em>
 * @property {number} height - Integer height of the rectangle = <code>bounds.height</code>. <em>Write-only.</em>
 * @property {number} radius=0 - Integer corner radius, in pixels. <em>Write-only.</em>
 * @property {Color} color=0,0,0 - The color of the overlay. <em>Write-only.</em>
 * @property {number} alpha=1.0 - The opacity of the overlay, <code>0.0</code> &ndash; <code>1.0</code>. <em>Write-only.</em>
 * @property {number} borderWidth=1 - Integer width of the border, in pixels. The border is drawn within the rectangle's bounds.
 *     It is not drawn unless either <code>borderColor</code> or <code>borderAlpha</code> are specified. <em>Write-only.</em>
 * @property {Color} borderColor=0,0,0 - The color of the border. <em>Write-only.</em>
 * @property {number} borderAlpha=1.0 - The opacity of the border, <code>0.0</code> &ndash; <code>1.0</code>.
 *     <em>Write-only.</em>
 */
