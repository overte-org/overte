//
//  LoginStateManager.cpp
//  interface/src
//
//  Created by Wayne Chen on 11/5/18.
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "LoginStateManager.h"

#include <QtCore/QString>
#include <QtCore/QVariantMap>

#include <plugins/PluginUtils.h>
#include <RegisteredMetaTypes.h>

#include "controllers/StateController.h"
#include "controllers/UserInputMapper.h"
#include "raypick/PointerScriptingInterface.h"
#include "raypick/RayPickScriptingInterface.h"
#include "raypick/PickScriptingInterface.h"
#include "scripting/ControllerScriptingInterface.h"

static const float SEARCH_SPHERE_SIZE = 0.0132f;
/*static const QVariantMap SEARCH_SPHERE = {{"x", SEARCH_SPHERE_SIZE},
                                            {"y", SEARCH_SPHERE_SIZE},
                                            {"z", SEARCH_SPHERE_SIZE}};*/

static const glm::vec3 SEARCH_SPHERE(SEARCH_SPHERE_SIZE, SEARCH_SPHERE_SIZE, SEARCH_SPHERE_SIZE);

static const int DEFAULT_SEARCH_SPHERE_DISTANCE = 1000; // how far from camera to search intersection?

/*static const QVariantMap COLORS_GRAB_SEARCHING_HALF_SQUEEZE = {{"red", 10},
                                                                {"green", 10},
                                                                {"blue", 255}};

static const QVariantMap COLORS_GRAB_SEARCHING_FULL_SQUEEZE = {{"red", 250},
                                                                {"green", 10},
                                                                {"blue", 10}};

static const QVariantMap COLORS_GRAB_DISTANCE_HOLD = {{"red", 238},
                                                        {"green", 75},
                                                        {"blue", 214}};*/

static const glm::u8vec3 COLORS_GRAB_SEARCHING_HALF_SQUEEZE(10, 10, 255);

static const glm::u8vec3 COLORS_GRAB_SEARCHING_FULL_SQUEEZE(250, 10, 10);

static const glm::u8vec3 COLORS_GRAB_DISTANCE_HOLD(238, 75, 215);


void LoginStateManager::tearDown() {
    auto pointers = DependencyManager::get<PointerManager>().data();
    if (pointers) {
        if (_leftLoginPointerID > PointerEvent::INVALID_POINTER_ID) {
            pointers->removePointer(_leftLoginPointerID);
            _leftLoginPointerID = PointerEvent::INVALID_POINTER_ID;
        }
        if (_rightLoginPointerID > PointerEvent::INVALID_POINTER_ID) {
            pointers->removePointer(_rightLoginPointerID);
            _rightLoginPointerID = PointerEvent::INVALID_POINTER_ID;
        }
    }
}

void LoginStateManager::setUp() {
    QList<EntityItemProperties> entityProperties;

    //V8TODO: are points and normals needed here
    EntityItemProperties fullPathRenderState;
    fullPathRenderState.setType(EntityTypes::PolyLine);
    fullPathRenderState.setColor(COLORS_GRAB_SEARCHING_FULL_SQUEEZE);
    fullPathRenderState.setGlow(true);
    fullPathRenderState.setIgnorePickIntersection(true); // always ignore this
    fullPathRenderState.setRenderLayer(RenderLayer::FRONT); // Even when buried inside of something, show it.
    fullPathRenderState.setFaceCamera(true);
    int fullPathRenderStateIndex = entityProperties.length();
    entityProperties.append(fullPathRenderState);

    EntityItemProperties fullEndRenderState;
    fullEndRenderState.setType(EntityTypes::Sphere);
    fullEndRenderState.setDimensions(SEARCH_SPHERE);
    fullEndRenderState.setColor(COLORS_GRAB_SEARCHING_FULL_SQUEEZE);
    fullEndRenderState.setAlpha(0.9f);
    fullEndRenderState.setIgnorePickIntersection(true); // always ignore this
    fullEndRenderState.setRenderLayer(RenderLayer::FRONT); // Even when buried inside of something, show it.
    int fullEndRenderStateIndex = entityProperties.length();
    entityProperties.append(fullEndRenderState);

    EntityItemProperties halfPathRenderState;
    halfPathRenderState.setType(EntityTypes::PolyLine);
    halfPathRenderState.setColor(COLORS_GRAB_SEARCHING_HALF_SQUEEZE);
    halfPathRenderState.setGlow(true);
    halfPathRenderState.setIgnorePickIntersection(true); // always ignore this
    halfPathRenderState.setRenderLayer(RenderLayer::FRONT); // Even when buried inside of something, show it.
    halfPathRenderState.setFaceCamera(true);
    int halfPathRenderStateIndex = entityProperties.length();
    entityProperties.append(halfPathRenderState);

    EntityItemProperties halfEndRenderState;
    halfEndRenderState.setType(EntityTypes::Sphere);
    halfEndRenderState.setDimensions(SEARCH_SPHERE);
    halfEndRenderState.setColor(COLORS_GRAB_SEARCHING_HALF_SQUEEZE);
    halfEndRenderState.setAlpha(0.9f);
    halfEndRenderState.setIgnorePickIntersection(true); // always ignore this
    halfEndRenderState.setRenderLayer(RenderLayer::FRONT); // Even when buried inside of something, show it.
    int halfEndRenderStateIndex = entityProperties.length();
    entityProperties.append(halfEndRenderState);

    EntityItemProperties holdPathRenderState;
    holdPathRenderState.setType(EntityTypes::PolyLine);
    holdPathRenderState.setColor(COLORS_GRAB_DISTANCE_HOLD);
    holdPathRenderState.setGlow(true);
    holdPathRenderState.setIgnorePickIntersection(true); // always ignore this
    holdPathRenderState.setRenderLayer(RenderLayer::FRONT); // Even when buried inside of something, show it.
    holdPathRenderState.setFaceCamera(true);
    int holdPathRenderStateIndex = entityProperties.length();
    entityProperties.append(holdPathRenderState);

    QVariantMap halfRenderStateIdentifier {
        {"name", "half"},
        {"pathPropertyIndex", halfPathRenderStateIndex},
        {"endPropertyIndex", halfEndRenderStateIndex}
    };
    QVariantMap fullRenderStateIdentifier {
        {"name", "full"},
        {"pathPropertyIndex", fullPathRenderStateIndex},
        {"endPropertyIndex", fullEndRenderStateIndex}
    };
    QVariantMap holdRenderStateIdentifier {
        {"name", "hold"},
        {"pathPropertyIndex", holdPathRenderStateIndex},
    };

    QVariantMap halfDefaultRenderStateIdentifier {
        {"name", "half"},
        {"distance", DEFAULT_SEARCH_SPHERE_DISTANCE},
        {"pathPropertyIndex", halfPathRenderStateIndex}
    };
    QVariantMap fullDefaultRenderStateIdentifier {
        {"name", "full"},
        {"distance", DEFAULT_SEARCH_SPHERE_DISTANCE},
        {"pathPropertyIndex", fullPathRenderStateIndex}
    };
    QVariantMap holdDefaultRenderStateIdentifier {
        {"name", "hold"},
        {"distance", DEFAULT_SEARCH_SPHERE_DISTANCE},
        {"pathPropertyIndex", holdPathRenderStateIndex}
    };

    _renderStates = QList<QVariant>({halfRenderStateIdentifier, fullRenderStateIdentifier, holdRenderStateIdentifier});
    _defaultRenderStates = QList<QVariant>({halfDefaultRenderStateIdentifier, fullDefaultRenderStateIdentifier, holdDefaultRenderStateIdentifier});

    auto pointers = DependencyManager::get<PointerScriptingInterface>();
    auto controller = DependencyManager::get<controller::ScriptingInterface>();

    const glm::vec3 grabPointSphereOffsetLeft { -0.04f, 0.13f, 0.039f };  // x = upward, y = forward, z = lateral
    const glm::vec3 grabPointSphereOffsetRight { 0.04f, 0.13f, 0.039f };  // x = upward, y = forward, z = lateral
    const glm::vec3 malletOffset {glm::vec3(0.0f, 0.18f - 0.050f, 0.0f)};

    QList<QVariant> leftPointerTriggerProperties;
    QVariantMap ltClick1 {
        { "action", controller->getStandard()["LTClick"] },
        { "button", "Focus" }
    };
    QVariantMap ltClick2 {
        { "action", controller->getStandard()["LTClick"] },
        { "button", "Primary" }
    };

    leftPointerTriggerProperties = QList<QVariant>({ltClick1, ltClick2});
    const unsigned int leftHand = 0;
    QVariantMap leftPointerPropertiesMap {
        { "joint", "_CAMERA_RELATIVE_CONTROLLER_LEFTHAND" },
        { "filter", PickScriptingInterface::getPickLocalEntities() },
        { "triggers", leftPointerTriggerProperties },
        { "posOffset", vec3toVariant(grabPointSphereOffsetLeft + malletOffset) },
        { "hover", true },
        { "scaleWithParent", true },
        { "distanceScaleEnd", true },
        { "hand", leftHand }
    };
    leftPointerPropertiesMap["renderStates"] = _renderStates;
    leftPointerPropertiesMap["defaultRenderStates"] = _defaultRenderStates;
    RayPointerProperties leftPointerProperties;
    leftPointerProperties.properties = leftPointerPropertiesMap;
    leftPointerProperties.entityProperties = entityProperties;
    _leftLoginPointerID = pointers->createRayPointer(leftPointerProperties);
    pointers->setRenderState(_leftLoginPointerID, "");
    pointers->enablePointer(_leftLoginPointerID);
    const unsigned int rightHand = 1;
    QList<QVariant> rightPointerTriggerProperties;

    QVariantMap rtClick1 {
        { "action", controller->getStandard()["RTClick"] },
        { "button", "Focus" }
    };
    QVariantMap rtClick2 {
        { "action", controller->getStandard()["RTClick"] },
        { "button", "Primary" }
    };
    rightPointerTriggerProperties = QList<QVariant>({rtClick1, rtClick2});
    QVariantMap rightPointerPropertiesMap{
        { "joint", "_CAMERA_RELATIVE_CONTROLLER_RIGHTHAND" },
        { "filter", PickScriptingInterface::getPickLocalEntities() },
        { "triggers", rightPointerTriggerProperties },
        { "posOffset", vec3toVariant(grabPointSphereOffsetRight + malletOffset) },
        { "hover", true },
        { "scaleWithParent", true },
        { "distanceScaleEnd", true },
        { "hand", rightHand }
    };
    rightPointerPropertiesMap["renderStates"] = _renderStates;
    rightPointerPropertiesMap["defaultRenderStates"] = _defaultRenderStates;
    RayPointerProperties rightPointerProperties;
    rightPointerProperties.properties = rightPointerPropertiesMap;
    rightPointerProperties.entityProperties = entityProperties;
    _rightLoginPointerID = pointers->createRayPointer(rightPointerProperties);
    pointers->setRenderState(_rightLoginPointerID, "");
    pointers->enablePointer(_rightLoginPointerID);
}

void LoginStateManager::update(const QString& dominantHand, const QUuid& loginEntityID) {
    if (!isSetUp()) {
        return;
    }
    if (_dominantHand != dominantHand) {
        _dominantHand = dominantHand;
    }
    auto pointers = DependencyManager::get<PointerScriptingInterface>();
    auto raypicks = DependencyManager::get<RayPickScriptingInterface>();
    if (pointers && raypicks) {
        const auto rightObjectID = raypicks->getPrevRayPickResult(_rightLoginPointerID)["objectID"].toUuid();
        const auto leftObjectID = raypicks->getPrevRayPickResult(_leftLoginPointerID)["objectID"].toUuid();
        const QString leftMode = (leftObjectID.isNull() || leftObjectID != loginEntityID) ? "" : "full";
        const QString rightMode = (rightObjectID.isNull() || rightObjectID != loginEntityID) ? "" : "full";
        pointers->setRenderState(_leftLoginPointerID, leftMode);
        pointers->setRenderState(_rightLoginPointerID, rightMode);
        if (_dominantHand == "left" && !leftObjectID.isNull()) {
            // dominant is left.
            pointers->setRenderState(_rightLoginPointerID, "");
            pointers->setRenderState(_leftLoginPointerID, leftMode);
        } else if (_dominantHand == "right" && !rightObjectID.isNull()) {
            // dominant is right.
            pointers->setRenderState(_leftLoginPointerID, "");
            pointers->setRenderState(_rightLoginPointerID, rightMode);
        }
    }
}
