//
//  LaserPointer.cpp
//  interface/src/raypick
//
//  Created by Sam Gondelman 7/11/2017
//  Copyright 2017 High Fidelity, Inc.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "LaserPointer.h"

#include "Application.h"
#include "avatar/AvatarManager.h"

#include <DependencyManager.h>
#include "PickManager.h"
#include "RayPick.h"

#include "PolyLineEntityItem.h"

LaserPointer::LaserPointer(const QVariant& rayProps, const RenderStateMap& renderStates, const DefaultRenderStateMap& defaultRenderStates, bool hover,
                           const PointerTriggers& triggers, bool faceAvatar, bool followNormal, float followNormalTime, bool centerEndY, bool lockEnd,
                           bool distanceScaleEnd, bool scaleWithParent, bool enabled) :
    PathPointer(PickQuery::Ray, rayProps, renderStates, defaultRenderStates, hover, triggers, faceAvatar, followNormal, followNormalTime,
                centerEndY, lockEnd, distanceScaleEnd, scaleWithParent, enabled)
{
}

PickQuery::PickType LaserPointer::getType() const {
    return PickQuery::PickType::Ray;
}

void LaserPointer::editRenderStatePath(const std::string& state, const QVariant& pathProps) {
    auto renderState = std::static_pointer_cast<RenderState>(_renderStates[state]);
    if (renderState) {
        updateRenderState(renderState->getPathID(), pathProps);
        QVariantMap pathPropsMap = pathProps.toMap();
        QVariant lineWidth = pathPropsMap["lineWidth"];
        if (lineWidth.isValid()) {
            renderState->setLineWidth(lineWidth.toFloat());
        }

        if (pathPropsMap.contains("linePoints")) {
            QVariantList linePoints = pathPropsMap["linePoints"].toList();
            renderState->setNumPoints(linePoints.length());
        }
    }
}

PickResultPointer LaserPointer::getPickResultCopy(const PickResultPointer& pickResult) const {
    auto rayPickResult = std::dynamic_pointer_cast<RayPickResult>(pickResult);
    if (!rayPickResult) {
        return std::make_shared<RayPickResult>();
    }
    return std::make_shared<RayPickResult>(*rayPickResult.get());
}

QVariantMap LaserPointer::toVariantMap() const {
    QVariantMap qVariantMap = Parent::toVariantMap();

    QVariantMap qRenderStates;
    for (auto iter = _renderStates.cbegin(); iter != _renderStates.cend(); iter++) {
        auto renderState = iter->second;
        QVariantMap qRenderState;
        qRenderState["start"] = renderState->getStartID();
        qRenderState["path"] = std::static_pointer_cast<RenderState>(renderState)->getPathID();
        qRenderState["end"] = renderState->getEndID();
        qRenderStates[iter->first.c_str()] = qRenderState;
    }
    qVariantMap["renderStates"] = qRenderStates;

    QVariantMap qDefaultRenderStates;
    for (auto iter = _defaultRenderStates.cbegin(); iter != _defaultRenderStates.cend(); iter++) {
        float distance = iter->second.first;
        auto defaultRenderState = iter->second.second;
        QVariantMap qDefaultRenderState;

        qDefaultRenderState["distance"] = distance;
        qDefaultRenderState["start"] = defaultRenderState->getStartID();
        qDefaultRenderState["path"] = std::static_pointer_cast<RenderState>(defaultRenderState)->getPathID();
        qDefaultRenderState["end"] = defaultRenderState->getEndID();
        qDefaultRenderStates[iter->first.c_str()] = qDefaultRenderState;
    }
    qVariantMap["defaultRenderStates"] = qDefaultRenderStates;

    return qVariantMap;
}

glm::vec3 LaserPointer::getPickOrigin(const PickResultPointer& pickResult) const {
    auto rayPickResult = std::static_pointer_cast<RayPickResult>(pickResult);
    return (rayPickResult ? vec3FromVariant(rayPickResult->pickVariant["origin"]) : glm::vec3(0.0f));
}

glm::vec3 LaserPointer::getPickEnd(const PickResultPointer& pickResult, float distance) const {
    auto rayPickResult = std::static_pointer_cast<RayPickResult>(pickResult);
    if (!rayPickResult) {
        return glm::vec3(0.0f);
    }
    if (distance > 0.0f) {
        PickRay pick = PickRay(rayPickResult->pickVariant);
        return pick.origin + distance * pick.direction;
    } else {
        return rayPickResult->intersection;
    }
}

glm::vec3 LaserPointer::getPickedObjectNormal(const PickResultPointer& pickResult) const {
    auto rayPickResult = std::static_pointer_cast<RayPickResult>(pickResult);
    return (rayPickResult ? rayPickResult->surfaceNormal : glm::vec3(0.0f));
}

IntersectionType LaserPointer::getPickedObjectType(const PickResultPointer& pickResult) const {
    auto rayPickResult = std::static_pointer_cast<RayPickResult>(pickResult);
    return (rayPickResult ? rayPickResult->type : IntersectionType::NONE);
}

QUuid LaserPointer::getPickedObjectID(const PickResultPointer& pickResult) const {
    auto rayPickResult = std::static_pointer_cast<RayPickResult>(pickResult);
    return (rayPickResult ? rayPickResult->objectID : QUuid());
}

void LaserPointer::setVisualPickResultInternal(PickResultPointer pickResult, IntersectionType type, const QUuid& id,
                                               const glm::vec3& intersection, float distance, const glm::vec3& surfaceNormal) {
    auto rayPickResult = std::static_pointer_cast<RayPickResult>(pickResult);
    if (rayPickResult) {
        rayPickResult->type = type;
        rayPickResult->objectID = id;
        rayPickResult->intersection = intersection;
        rayPickResult->distance = distance;
        rayPickResult->surfaceNormal = surfaceNormal;
        rayPickResult->pickVariant["direction"] = vec3toVariant(-surfaceNormal);
    }
}

LaserPointer::RenderState::RenderState(const QUuid& startID, const QUuid& pathID, const QUuid& endID) :
    StartEndRenderState(startID, endID), _pathID(pathID)
{
    if (!getPathID().isNull()) {
        EntityPropertyFlags desiredProperties;
        desiredProperties += PROP_IGNORE_PICK_INTERSECTION;
        desiredProperties += PROP_LINE_POINTS;
        desiredProperties += PROP_STROKE_WIDTHS;
        auto properties = DependencyManager::get<EntityScriptingInterface>()->getEntityPropertiesInternal(getPathID(), desiredProperties, false);

        auto widths = properties.getStrokeWidths();
        _lineWidth = widths.length() == 0 ? PolyLineEntityItem::DEFAULT_LINE_WIDTH : widths[0];

        setNumPoints(properties.getLinePoints().length());

        _pathIgnorePicks = properties.getIgnorePickIntersection();
    }
}

void LaserPointer::RenderState::cleanup() {
    StartEndRenderState::cleanup();
    if (!getPathID().isNull()) {
        DependencyManager::get<EntityScriptingInterface>()->deleteEntity(getPathID());
    }
}

void LaserPointer::RenderState::disable() {
    StartEndRenderState::disable();
    if (!getPathID().isNull()) {
        EntityItemProperties properties;
        properties.setVisible(false);
        properties.setIgnorePickIntersection(true);
        DependencyManager::get<EntityScriptingInterface>()->editEntity(getPathID(), properties);
    }
}

void LaserPointer::RenderState::update(const glm::vec3& origin, const glm::vec3& end, const glm::vec3& surfaceNormal, float parentScale, bool distanceScaleEnd, bool centerEndY,
                                       bool faceAvatar, bool followNormal, float followNormalStrength, float distance, const PickResultPointer& pickResult) {
    StartEndRenderState::update(origin, end, surfaceNormal, parentScale, distanceScaleEnd, centerEndY, faceAvatar, followNormal, followNormalStrength, distance, pickResult);
    if (!getPathID().isNull()) {
        EntityItemProperties properties;
        QVector<glm::vec3> points;
        const size_t numPoints = getNumPoints();
        points.append(glm::vec3(0.0f));
        const glm::vec3 endPoint = end - origin;
        if (numPoints > 2) {
            EntityPropertyFlags desiredProperties;
            desiredProperties += PROP_VISIBLE;
            auto oldProperties = DependencyManager::get<EntityScriptingInterface>()->getEntityPropertiesInternal(getPathID(), desiredProperties, false);

            bool hasUnmodifiedEndPoint = false;
            glm::vec3 unmodifiedEndPoint;
            auto rayPickResult = std::static_pointer_cast<RayPickResult>(pickResult);
            if (rayPickResult && rayPickResult->pickVariant.contains("unmodifiedDirection")) {
                unmodifiedEndPoint = glm::length(endPoint) * vec3FromVariant(rayPickResult->pickVariant["unmodifiedDirection"]);
                hasUnmodifiedEndPoint = true;
            }

            // Segment points are evenly spaced between origin and end
            for (size_t i = 1; i < numPoints - 1; i++) {
                const float frac = ((float)i / (numPoints - 1));
                if (!oldProperties.getVisible() || !_hasSetLinePoints || !hasUnmodifiedEndPoint) {
                    points.append(frac * endPoint);
                } else {
                    points.append(frac * mix(unmodifiedEndPoint, endPoint, frac));
                }
            }
            _hasSetLinePoints = true;
        }
        points.append(endPoint);
        properties.setPosition(origin);
        properties.setRotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
        properties.setLinePoints(points);
        properties.setVisible(true);
        properties.setIgnorePickIntersection(doesPathIgnorePicks());
        QVector<glm::vec3> normals;
        normals.fill(glm::vec3(0.0f, 0.0f, 1.0f), (int)numPoints);
        properties.setNormals(normals);
        QVector<float> widths;
        float width = getLineWidth() * parentScale;
        widths.fill(width, (int)numPoints);
        properties.setStrokeWidths(widths);
        DependencyManager::get<EntityScriptingInterface>()->editEntity(getPathID(), properties);
    }
}

std::shared_ptr<StartEndRenderState> LaserPointer::buildRenderState(const QVariantMap& propMap, const QList<EntityItemProperties> &entityProperties) {
    QUuid startID;
    if (propMap["startPropertyIndex"].isValid()) {
        int startPropertyIndex = propMap["startPropertyIndex"].toInt();
        if (startPropertyIndex >= 0 && startPropertyIndex < entityProperties.length()) {
            EntityItemProperties startProperties(entityProperties[startPropertyIndex]);
            startProperties.getGrab().setGrabbable(false);
            startID = DependencyManager::get<EntityScriptingInterface>()->addEntityInternal(startProperties, entity::HostType::LOCAL);
        }
    }

    QUuid pathID;
    if (propMap["pathPropertyIndex"].isValid()) {
        int pathPropertyIndex = propMap["pathPropertyIndex"].toInt();
        if (pathPropertyIndex >= 0 && pathPropertyIndex < entityProperties.length()) {
            EntityItemProperties pathProperties(entityProperties[pathPropertyIndex]);
            // laser paths must be PolyLine
            pathProperties.setType(EntityTypes::EntityType::PolyLine);
            pathProperties.getGrab().setGrabbable(false);
            pathID = DependencyManager::get<EntityScriptingInterface>()->addEntityInternal(pathProperties, entity::HostType::LOCAL);
        }
    }

    QUuid endID;
    if (propMap["endPropertyIndex"].isValid()) {
        int endPropertyIndex = propMap["endPropertyIndex"].toInt();
        if (endPropertyIndex >= 0 && endPropertyIndex < entityProperties.length()) {
            EntityItemProperties endProperties(entityProperties[endPropertyIndex]);
            endProperties.getGrab().setGrabbable(false);
            endID = DependencyManager::get<EntityScriptingInterface>()->addEntityInternal(endProperties, entity::HostType::LOCAL);
        }
    }

    return std::make_shared<RenderState>(startID, pathID, endID);
}

PointerEvent LaserPointer::buildPointerEvent(const PickedObject& target, const PickResultPointer& pickResult, const std::string& button, bool hover) {
    QUuid pickedID;
    glm::vec3 intersection, surfaceNormal, direction, origin;
    auto rayPickResult = std::static_pointer_cast<RayPickResult>(pickResult);
    if (rayPickResult) {
        intersection = rayPickResult->intersection;
        surfaceNormal = rayPickResult->surfaceNormal;
        const QVariantMap& searchRay = rayPickResult->pickVariant;
        direction = vec3FromVariant(searchRay["direction"]);
        origin = vec3FromVariant(searchRay["origin"]);
        pickedID = rayPickResult->objectID;
    }

    if (pickedID != target.objectID) {
        intersection = findIntersection(target, origin, direction);
    }
    glm::vec2 pos2D = findPos2D(target, intersection);

    // If we just started triggering and we haven't moved too much, don't update intersection and pos2D
    TriggerState& state = hover ? _latestState : _states[button];
    auto avatar = DependencyManager::get<AvatarManager>()->getMyAvatar();
    if (avatar) {
        float sensorToWorldScale = avatar->getSensorToWorldScale();
        float deadspotSquared = TOUCH_PRESS_TO_MOVE_DEADSPOT_SQUARED * sensorToWorldScale * sensorToWorldScale;
        bool withinDeadspot = usecTimestampNow() - state.triggerStartTime < POINTER_MOVE_DELAY && glm::distance2(pos2D, state.triggerPos2D) < deadspotSquared;
        if ((state.triggering || state.wasTriggering) && !state.deadspotExpired && withinDeadspot) {
            pos2D = state.triggerPos2D;
            intersection = state.intersection;
            surfaceNormal = state.surfaceNormal;
        }
        if (!withinDeadspot) {
            state.deadspotExpired = true;
        }
    }

    return PointerEvent(pos2D, intersection, surfaceNormal, direction);
}

glm::vec3 LaserPointer::findIntersection(const PickedObject& pickedObject, const glm::vec3& origin, const glm::vec3& direction) {
    switch (pickedObject.type) {
        case ENTITY:
        case LOCAL_ENTITY:
            return RayPick::intersectRayWithEntityXYPlane(pickedObject.objectID, origin, direction);
        default:
            return glm::vec3(NAN);
    }
}
