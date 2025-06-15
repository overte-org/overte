"use strict";

//  controllerDispatcherUtils.js
//
//  Copyright 2017-2020 High Fidelity, Inc.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

/* global module, HMD, MyAvatar, controllerDispatcherPlugins:true, Quat, Vec3, Overlays, Xform, Mat4,
   Selection, Uuid, Controller,
   MSECS_PER_SEC:true , LEFT_HAND:true, RIGHT_HAND:true, FORBIDDEN_GRAB_TYPES:true,
   HAPTIC_PULSE_STRENGTH:true, HAPTIC_PULSE_DURATION:true, ZERO_VEC:true, ONE_VEC:true,
   DEFAULT_REGISTRATION_POINT:true, INCHES_TO_METERS:true,
   TRIGGER_OFF_VALUE:true,
   TRIGGER_ON_VALUE:true,
   PICK_MAX_DISTANCE:true,
   DEFAULT_SEARCH_SPHERE_DISTANCE:true,
   NEAR_GRAB_PICK_RADIUS:true,
   COLORS_GRAB_SEARCHING_HALF_SQUEEZE:true,
   COLORS_GRAB_SEARCHING_FULL_SQUEEZE:true,
   COLORS_GRAB_DISTANCE_HOLD:true,
   NEAR_GRAB_RADIUS:true,
   DISPATCHER_PROPERTIES:true,
   HAPTIC_PULSE_STRENGTH:true,
   HAPTIC_PULSE_DURATION:true,
   DISPATCHER_HOVERING_LIST:true,
   DISPATCHER_HOVERING_STYLE:true,
   Entities,
   makeDispatcherModuleParameters:true,
   makeRunningValues:true,
   enableDispatcherModule:true,
   disableDispatcherModule:true,
   getEnabledModuleByName:true,
   getGrabbableData:true,
   isAnothersAvatarEntity:true,
   isAnothersChildEntity:true,
   entityIsEquippable:true,
   entityIsGrabbable:true,
   entityIsDistanceGrabbable:true,
   getControllerJointIndexCacheTime:true,
   getControllerJointIndexCache:true,
   getControllerJointIndex:true,
   propsArePhysical:true,
   controllerDispatcherPluginsNeedSort:true,
   projectOntoXYPlane:true,
   projectOntoEntityXYPlane:true,
   projectOntoOverlayXYPlane:true,
   makeLaserLockInfo:true,
   entityHasActions:true,
   ensureDynamic:true,
   findGrabbableGroupParent:true,
   BUMPER_ON_VALUE:true,
   getEntityParents:true,
   findHandChildEntities:true,
   findFarGrabJointChildEntities:true,
   makeLaserParams:true,
   TEAR_AWAY_DISTANCE:true,
   TEAR_AWAY_COUNT:true,
   TEAR_AWAY_CHECK_TIME:true,
   TELEPORT_DEADZONE: true,
   NEAR_GRAB_DISTANCE: true,
   distanceBetweenPointAndEntityBoundingBox:true,
   entityIsEquipped:true,
   highlightTargetEntity:true,
   clearHighlightedEntities:true,
   unhighlightTargetEntity:true,
   distanceBetweenEntityLocalPositionAndBoundingBox: true,
   worldPositionToRegistrationFrameMatrix: true,
   handsAreTracked: true
*/

var controllerStandard = Controller.Standard;

var MSECS_PER_SEC = 1000.0;
var INCHES_TO_METERS = 1.0 / 39.3701;

var HAPTIC_PULSE_STRENGTH = 1.0;
var HAPTIC_PULSE_DURATION = 13.0;

var ZERO_VEC = { x: 0, y: 0, z: 0 };
var ONE_VEC = { x: 1, y: 1, z: 1 };

var LEFT_HAND = 0;
var RIGHT_HAND = 1;

var FORBIDDEN_GRAB_TYPES = ["Unknown", "Light", "PolyLine", "Zone"];

var HAPTIC_PULSE_STRENGTH = 1.0;
var HAPTIC_PULSE_DURATION = 13.0;

var DEFAULT_REGISTRATION_POINT = { x: 0.5, y: 0.5, z: 0.5 };

var TRIGGER_OFF_VALUE = 0.1;
var TRIGGER_ON_VALUE = TRIGGER_OFF_VALUE + 0.05; // Squeezed just enough to activate search or near grab
var BUMPER_ON_VALUE = 0.5;

var PICK_MAX_DISTANCE = 500; // max length of pick-ray
var DEFAULT_SEARCH_SPHERE_DISTANCE = 1000; // how far from camera to search intersection?
var NEAR_GRAB_PICK_RADIUS = 0.25; // radius used for search ray vs object for near grabbing.

var COLORS_GRAB_SEARCHING_HALF_SQUEEZE = { red: 10, green: 10, blue: 255 };
var COLORS_GRAB_SEARCHING_FULL_SQUEEZE = { red: 250, green: 10, blue: 10 };
var COLORS_GRAB_DISTANCE_HOLD = { red: 238, green: 75, blue: 214 };

var NEAR_GRAB_RADIUS = 1.0;

var TEAR_AWAY_DISTANCE = 0.15; // ungrab an entity if its bounding-box moves this far from the hand
var TEAR_AWAY_COUNT = 2; // multiply by TEAR_AWAY_CHECK_TIME to know how long the item must be away
var TEAR_AWAY_CHECK_TIME = 0.15; // seconds, duration between checks

var TELEPORT_DEADZONE = 0.15;

var NEAR_GRAB_DISTANCE = 0.14; // Grab an entity if its bounding box is within this distance.
// Smaller than TEAR_AWAY_DISTANCE for hysteresis.

var DISPATCHER_HOVERING_LIST = "dispatcherHoveringList";
var DISPATCHER_HOVERING_STYLE = {
    isOutlineSmooth: true,
    outlineWidth: 0,
    outlineUnoccludedColor: {red: 255, green: 128, blue: 128},
    outlineUnoccludedAlpha: 0.0,
    outlineOccludedColor: {red: 255, green: 128, blue: 128},
    outlineOccludedAlpha:0.0,
    fillUnoccludedColor: {red: 255, green: 255, blue: 255},
    fillUnoccludedAlpha: 0.12,
    fillOccludedColor: {red: 255, green: 255, blue: 255},
    fillOccludedAlpha: 0.0
};

var DISPATCHER_PROPERTIES = [
    "id",
    "position",
    "registrationPoint",
    "rotation",
    "gravity",
    "collidesWith",
    "dynamic",
    "collisionless",
    "locked",
    "name",
    "shapeType",
    "parentID",
    "parentJointIndex",
    "density",
    "dimensions",
    "type",
    "href",
    "cloneable",
    "cloneDynamic",
    "localPosition",
    "localRotation",
    "grab.grabbable",
    "grab.grabKinematic",
    "grab.grabFollowsController",
    "grab.triggerable",
    "grab.equippable",
    "grab.grabDelegateToParent",
    "grab.equippableLeftPosition",
    "grab.equippableLeftRotation",
    "grab.equippableRightPosition",
    "grab.equippableRightRotation",
    "grab.equippableIndicatorURL",
    "grab.equippableIndicatorScale",
    "grab.equippableIndicatorOffset",
    "userData",
    "avatarEntity",
    "owningAvatarID"
];

// priority -- a lower priority means the module will be asked sooner than one with a higher priority in a given update step
// activitySlots -- indicates which "slots" must not yet be in use for this module to start
// requiredDataForReady -- which "situation" parts this module looks at to decide if it will start
// sleepMSBetweenRuns -- how long to wait between calls to this module's "run" method
var makeDispatcherModuleParameters = function (priority, activitySlots, requiredDataForReady, sleepMSBetweenRuns, enableLaserForHand) {
    if (enableLaserForHand === undefined) {
        enableLaserForHand = -1;
    }

    return {
        priority: priority,
        activitySlots: activitySlots,
        requiredDataForReady: requiredDataForReady,
        sleepMSBetweenRuns: sleepMSBetweenRuns,
        handLaser: enableLaserForHand
    };
};

var makeLaserLockInfo = function(targetID, isOverlay, hand, offset) {
    return {
        targetID: targetID,
        isOverlay: isOverlay,
        hand: hand,
        offset: offset
    };
};

var makeLaserParams = function(hand, alwaysOn) {
    if (alwaysOn === undefined) {
        alwaysOn = false;
    }

    return {
        hand: hand,
        alwaysOn: alwaysOn
    };
};

var makeRunningValues = function (active, targets, requiredDataForRun, laserLockInfo) {
    return {
        active: active,
        targets: targets,
        requiredDataForRun: requiredDataForRun,
        laserLockInfo: laserLockInfo
    };
};

var enableDispatcherModule = function (moduleName, module, priority) {
    if (!controllerDispatcherPlugins) {
        controllerDispatcherPlugins = {};
    }
    controllerDispatcherPlugins[moduleName] = module;
    controllerDispatcherPluginsNeedSort = true;
};

var disableDispatcherModule = function (moduleName) {
    delete controllerDispatcherPlugins[moduleName];
    controllerDispatcherPluginsNeedSort = true;
};

var getEnabledModuleByName = function (moduleName) {
    if (controllerDispatcherPlugins.hasOwnProperty(moduleName)) {
        return controllerDispatcherPlugins[moduleName];
    }
    return null;
};

var getGrabbableData = function (ggdProps) {
    // look in userData for a "grabbable" key, return the value or some defaults
    var grabbableData = {};
    var userDataParsed = null;
    try {
        if (!ggdProps.userDataParsed) {
            ggdProps.userDataParsed = JSON.parse(ggdProps.userData);
        }
        userDataParsed = ggdProps.userDataParsed;
    } catch (err) {
        userDataParsed = {};
    }

    if (userDataParsed.grabbableKey) {
        grabbableData = userDataParsed.grabbableKey;
    } else {
        grabbableData = ggdProps.grab;
    }

    // extract grab-related properties, provide defaults if any are missing
    if (!grabbableData.hasOwnProperty("grabbable")) {
        grabbableData.grabbable = true;
    }
    // kinematic has been renamed to grabKinematic
    if (!grabbableData.hasOwnProperty("grabKinematic") &&
        !grabbableData.hasOwnProperty("kinematic")) {
        grabbableData.grabKinematic = true;
    }
    if (!grabbableData.hasOwnProperty("grabKinematic")) {
        grabbableData.grabKinematic = grabbableData.kinematic;
    }
    // ignoreIK has been renamed to grabFollowsController
    if (!grabbableData.hasOwnProperty("grabFollowsController") &&
        !grabbableData.hasOwnProperty("ignoreIK")) {
        grabbableData.grabFollowsController = true;
    }
    if (!grabbableData.hasOwnProperty("grabFollowsController")) {
        grabbableData.grabFollowsController = grabbableData.ignoreIK;
    }
    // wantsTrigger has been renamed to triggerable
    if (!grabbableData.hasOwnProperty("triggerable") &&
        !grabbableData.hasOwnProperty("wantsTrigger")) {
        grabbableData.triggerable = false;
    }
    if (!grabbableData.hasOwnProperty("triggerable")) {
        grabbableData.triggerable = grabbableData.wantsTrigger;
    }
    if (!grabbableData.hasOwnProperty("equippable")) {
        grabbableData.equippable = false;
    }
    if (!grabbableData.hasOwnProperty("equippableLeftPosition")) {
        grabbableData.equippableLeftPosition = { x: 0, y: 0, z: 0 };
    }
    if (!grabbableData.hasOwnProperty("equippableLeftRotation")) {
        grabbableData.equippableLeftPosition = { x: 0, y: 0, z: 0, w: 1 };
    }
    if (!grabbableData.hasOwnProperty("equippableRightPosition")) {
        grabbableData.equippableRightPosition = { x: 0, y: 0, z: 0 };
    }
    if (!grabbableData.hasOwnProperty("equippableRightRotation")) {
        grabbableData.equippableRightPosition = { x: 0, y: 0, z: 0, w: 1 };
    }
    return grabbableData;
};

var isAnothersAvatarEntity = function (iaaeProps) {
    if (!iaaeProps.avatarEntity) {
        return false;
    }
    if (iaaeProps.owningAvatarID === MyAvatar.sessionUUID) {
        return false;
    }
    if (iaaeProps.owningAvatarID === MyAvatar.SELF_ID) {
        return false;
    }
    return true;
};

var isAnothersChildEntity = function (iaceProps) {
    while (iaceProps.parentID && iaceProps.parentID !== Uuid.NONE) {
        if (Entities.getNestableType(iaceProps.parentID) == "avatar") {
            if (iaceProps.parentID == MyAvatar.SELF_ID || iaceProps.parentID == MyAvatar.sessionUUID) {
                return false; // not another's, it's mine.
            }
            return true;
        }
        // Entities.getNestableType(iaceProps.parentID) == "entity") {
        var parentProps = Entities.getEntityProperties(iaceProps.parentID, DISPATCHER_PROPERTIES);
        if (!parentProps) {
            break;
        }
        parentProps.id = iaceProps.parentID;
        iaceProps = parentProps;
    }
    return false;
};


var entityIsEquippable = function (eieProps) {
    var grabbable = getGrabbableData(eieProps).grabbable;
    if (!grabbable ||
        isAnothersAvatarEntity(eieProps) ||
        isAnothersChildEntity(eieProps) ||
        FORBIDDEN_GRAB_TYPES.indexOf(eieProps.type) >= 0) {
        return false;
    }
    return true;
};

var entityIsGrabbable = function (eigProps) {
    var grabbable = getGrabbableData(eigProps).grabbable;
    if (!grabbable ||
        eigProps.locked ||
        FORBIDDEN_GRAB_TYPES.indexOf(eigProps.type) >= 0) {
        return false;
    }
    return true;
};

var clearHighlightedEntities = function() {
    Selection.clearSelectedItemsList(DISPATCHER_HOVERING_LIST);
};

var highlightTargetEntity = function(entityID) {
    Selection.addToSelectedItemsList(DISPATCHER_HOVERING_LIST, "entity", entityID);
};

var unhighlightTargetEntity = function(entityID) {
    Selection.removeFromSelectedItemsList(DISPATCHER_HOVERING_LIST, "entity", entityID);
};

var entityIsDistanceGrabbable = function(eidgProps) {
    if (!entityIsGrabbable(eidgProps)) {
        return false;
    }

    // we can't distance-grab non-physical
    var isPhysical = propsArePhysical(eidgProps);
    if (!isPhysical) {
        return false;
    }

    return true;
};

var getControllerJointIndexCacheTime = [0, 0];
var getControllerJointIndexCache = [-1, -1];

var getControllerJointIndex = function (hand) {
    var GET_CONTROLLERJOINTINDEX_CACHE_REFRESH_TIME = 3000; // msecs

    var now = Date.now();
    if (now - getControllerJointIndexCacheTime[hand] > GET_CONTROLLERJOINTINDEX_CACHE_REFRESH_TIME) {
        if (HMD.isHandControllerAvailable()) {
            var controllerJointIndex = MyAvatar.getJointIndex(hand === RIGHT_HAND ?
                                                              "_CONTROLLER_RIGHTHAND" :
                                                              "_CONTROLLER_LEFTHAND");
            getControllerJointIndexCacheTime[hand] = now;
            getControllerJointIndexCache[hand] = controllerJointIndex;
            return controllerJointIndex;
        }
    } else {
        return getControllerJointIndexCache[hand];
    }

    return -1;
};

var propsArePhysical = function (papProps) {
    if (!papProps.dynamic) {
        return false;
    }
    var isPhysical = (papProps.shapeType && papProps.shapeType !== 'none');
    return isPhysical;
};

var projectOntoXYPlane = function (worldPos, position, rotation, dimensions, registrationPoint) {
    var invRot = Quat.inverse(rotation);
    var localPos = Vec3.multiplyQbyV(invRot, Vec3.subtract(worldPos, position));
    var invDimensions = {
        x: 1 / dimensions.x,
        y: 1 / dimensions.y,
        z: 1 / dimensions.z
    };

    var normalizedPos = Vec3.sum(Vec3.multiplyVbyV(localPos, invDimensions), registrationPoint);
    return {
        x: normalizedPos.x * dimensions.x,
        y: (1 - normalizedPos.y) * dimensions.y // flip y-axis
    };
};

var projectOntoEntityXYPlane = function (entityID, worldPos, popProps) {
    return projectOntoXYPlane(worldPos, popProps.position, popProps.rotation,
                              popProps.dimensions, popProps.registrationPoint);
};

var projectOntoOverlayXYPlane = function projectOntoOverlayXYPlane(overlayID, worldPos) {
    var position = Entities.getEntityProperties(overlayID, ["position"]).position;
    var rotation = Entities.getEntityProperties(overlayID, ["rotation"]).rotation;
    var dimensions = Entities.getEntityProperties(overlayID, ["dimensions"]).dimensions;
    dimensions.z = 0.01; // we are projecting onto the XY plane of the overlay, so ignore the z dimension

    return projectOntoXYPlane(worldPos, position, rotation, dimensions, DEFAULT_REGISTRATION_POINT);
};

var entityHasActions = function (entityID) {
    return Entities.getActionIDs(entityID).length > 0;
};

var ensureDynamic = function (entityID) {
    // if we distance hold something and keep it very still before releasing it, it ends up
    // non-dynamic in bullet.  If it's too still, give it a little bounce so it will fall.
    var edProps = Entities.getEntityProperties(entityID, ["velocity", "dynamic", "parentID"]);
    if (edProps.dynamic && edProps.parentID === Uuid.NONE) {
        var velocity = edProps.velocity;
        if (Vec3.length(velocity) < 0.05) { // see EntityMotionState.cpp DYNAMIC_LINEAR_VELOCITY_THRESHOLD
            velocity = { x: 0.0, y: 0.2, z: 0.0 };
            Entities.editEntity(entityID, { velocity: velocity });
        }
    }
};

var findGrabbableGroupParent = function (controllerData, targetProps) {
    while (targetProps.grab.grabDelegateToParent &&
           targetProps.parentID &&
           targetProps.parentID !== Uuid.NONE &&
           Entities.getNestableType(targetProps.parentID) == "entity") {
        var parentProps = Entities.getEntityProperties(targetProps.parentID, DISPATCHER_PROPERTIES);
        if (!parentProps) {
            break;
        }
        if (!entityIsGrabbable(parentProps)) {
            break;
        }
        parentProps.id = targetProps.parentID;
        targetProps = parentProps;
        controllerData.nearbyEntityPropertiesByID[targetProps.id] = targetProps;
    }

    return targetProps;
};

var getEntityParents = function(targetProps) {
    var parentProperties = [];
    while (targetProps.parentID &&
           targetProps.parentID !== Uuid.NONE &&
           Entities.getNestableType(targetProps.parentID) == "entity") {
        var parentProps = Entities.getEntityProperties(targetProps.parentID, DISPATCHER_PROPERTIES);
        if (!parentProps) {
            break;
        }
        parentProps.id = targetProps.parentID;
        targetProps = parentProps;
        parentProperties.push(parentProps);
    }

    return parentProperties;
};


var findHandChildEntities = function(hand) {
    // find children of avatar's hand joint
    var handJointIndex = MyAvatar.getJointIndex(hand === RIGHT_HAND ? "RightHand" : "LeftHand");
    var children = Entities.getChildrenIDsOfJoint(MyAvatar.sessionUUID, handJointIndex);
    children = children.concat(Entities.getChildrenIDsOfJoint(MyAvatar.SELF_ID, handJointIndex));

    // find children of faux controller joint
    var controllerJointIndex = getControllerJointIndex(hand);
    children = children.concat(Entities.getChildrenIDsOfJoint(MyAvatar.sessionUUID, controllerJointIndex));
    children = children.concat(Entities.getChildrenIDsOfJoint(MyAvatar.SELF_ID, controllerJointIndex));

    // find children of faux camera-relative controller joint
    var controllerCRJointIndex = MyAvatar.getJointIndex(hand === RIGHT_HAND ?
                                                        "_CAMERA_RELATIVE_CONTROLLER_RIGHTHAND" :
                                                        "_CAMERA_RELATIVE_CONTROLLER_LEFTHAND");
    children = children.concat(Entities.getChildrenIDsOfJoint(MyAvatar.sessionUUID, controllerCRJointIndex));
    children = children.concat(Entities.getChildrenIDsOfJoint(MyAvatar.SELF_ID, controllerCRJointIndex));

    return children.filter(function (childID) {
        var childType = Entities.getNestableType(childID);
        return childType == "entity";
    });
};

var findFarGrabJointChildEntities = function(hand) {
    // find children of avatar's far-grab joint
    var farGrabJointIndex = MyAvatar.getJointIndex(hand === RIGHT_HAND ? "_FARGRAB_RIGHTHAND" : "_FARGRAB_LEFTHAND");
    var children = Entities.getChildrenIDsOfJoint(MyAvatar.sessionUUID, farGrabJointIndex);
    children = children.concat(Entities.getChildrenIDsOfJoint(MyAvatar.SELF_ID, farGrabJointIndex));

    return children.filter(function (childID) {
        var childType = Entities.getNestableType(childID);
        return childType == "entity";
    });
};

var distanceBetweenEntityLocalPositionAndBoundingBox = function(entityProps, jointGrabOffset) {
    var DEFAULT_REGISTRATION_POINT = { x: 0.5, y: 0.5, z: 0.5 };
    var rotInv = Quat.inverse(entityProps.localRotation);
    var localPosition = Vec3.sum(entityProps.localPosition, jointGrabOffset);
    var localPoint = Vec3.multiplyQbyV(rotInv, Vec3.multiply(localPosition, -1.0));

    var halfDims = Vec3.multiply(entityProps.dimensions, 0.5);
    var regRatio = Vec3.subtract(DEFAULT_REGISTRATION_POINT, entityProps.registrationPoint);
    var entityCenter = Vec3.multiplyVbyV(regRatio, entityProps.dimensions);
    var localMin = Vec3.subtract(entityCenter, halfDims);
    var localMax = Vec3.sum(entityCenter, halfDims);


    var v = {x: localPoint.x, y: localPoint.y, z: localPoint.z};
    v.x = Math.max(v.x, localMin.x);
    v.x = Math.min(v.x, localMax.x);
    v.y = Math.max(v.y, localMin.y);
    v.y = Math.min(v.y, localMax.y);
    v.z = Math.max(v.z, localMin.z);
    v.z = Math.min(v.z, localMax.z);

    return Vec3.distance(v, localPoint);
};

var distanceBetweenPointAndEntityBoundingBox = function(point, entityProps) {
    var entityXform = new Xform(entityProps.rotation, entityProps.position);
    var localPoint = entityXform.inv().xformPoint(point);
    var minOffset = Vec3.multiplyVbyV(entityProps.registrationPoint, entityProps.dimensions);
    var maxOffset = Vec3.multiplyVbyV(Vec3.subtract(ONE_VEC, entityProps.registrationPoint), entityProps.dimensions);
    var localMin = Vec3.subtract(entityXform.pos, minOffset);
    var localMax = Vec3.sum(entityXform.pos, maxOffset);
    // TODO: was originally this, but this causes an error on V8 branch and probably never worked on QtScript either
    //var localMin = Vec3.subtract(entityXform.trans, minOffset);
    //var localMax = Vec3.sum(entityXform.trans, maxOffset);

    var v = {x: localPoint.x, y: localPoint.y, z: localPoint.z};
    v.x = Math.max(v.x, localMin.x);
    v.x = Math.min(v.x, localMax.x);
    v.y = Math.max(v.y, localMin.y);
    v.y = Math.min(v.y, localMax.y);
    v.z = Math.max(v.z, localMin.z);
    v.z = Math.min(v.z, localMax.z);

    return Vec3.distance(v, localPoint);
};

var entityIsEquipped = function(entityID) {
    var rightEquipEntity = getEnabledModuleByName("RightEquipEntity");
    var leftEquipEntity = getEnabledModuleByName("LeftEquipEntity");
    var equippedInRightHand = rightEquipEntity ? rightEquipEntity.targetEntityID === entityID : false;
    var equippedInLeftHand = leftEquipEntity ? leftEquipEntity.targetEntityID === entityID : false;
    return equippedInRightHand || equippedInLeftHand;
};

var worldPositionToRegistrationFrameMatrix = function(wptrProps, pos) {
    // get world matrix for intersection point
    var intersectionMat = new Xform({ x: 0, y: 0, z:0, w: 1 }, pos);

    // calculate world matrix for registrationPoint adjusted entity
    var DEFAULT_REGISTRATION_POINT = { x: 0.5, y: 0.5, z: 0.5 };
    var regRatio = Vec3.subtract(DEFAULT_REGISTRATION_POINT, wptrProps.registrationPoint);
    var regOffset = Vec3.multiplyVbyV(regRatio, wptrProps.dimensions);
    var regOffsetRot = Vec3.multiplyQbyV(wptrProps.rotation, regOffset);
    var modelMat = new Xform(wptrProps.rotation, Vec3.sum(wptrProps.position, regOffsetRot));

    // get inverse of model matrix
    var modelMatInv = modelMat.inv();

    // transform world intersection point into object's registrationPoint frame
    var xformMat = Xform.mul(modelMatInv, intersectionMat);

    // convert to Mat4
    var offsetMat = Mat4.createFromRotAndTrans(xformMat.rot, xformMat.pos);
    return offsetMat;
};

var handsAreTracked = function () {
    // FIXME: This currently breaks interaction lasers on controllers
    // that have both hand tracking and physical buttons
    /*return Controller.getPoseValue(controllerStandard.LeftHandIndex3).valid ||
        Controller.getPoseValue(controllerStandard.RightHandIndex3).valid;*/
    return false;
};

if (typeof module !== 'undefined') {
    module.exports = {
        makeDispatcherModuleParameters: makeDispatcherModuleParameters,
        enableDispatcherModule: enableDispatcherModule,
        disableDispatcherModule: disableDispatcherModule,
        highlightTargetEntity: highlightTargetEntity,
        unhighlightTargetEntity: unhighlightTargetEntity,
        clearHighlightedEntities: clearHighlightedEntities,
        makeRunningValues: makeRunningValues,
        findGrabbableGroupParent: findGrabbableGroupParent,
        LEFT_HAND: LEFT_HAND,
        RIGHT_HAND: RIGHT_HAND,
        BUMPER_ON_VALUE: BUMPER_ON_VALUE,
        TEAR_AWAY_DISTANCE: TEAR_AWAY_DISTANCE,
        propsArePhysical: propsArePhysical,
        entityIsEquippable: entityIsEquippable,
        entityIsGrabbable: entityIsGrabbable,
        NEAR_GRAB_RADIUS: NEAR_GRAB_RADIUS,
        projectOntoOverlayXYPlane: projectOntoOverlayXYPlane,
        projectOntoEntityXYPlane: projectOntoEntityXYPlane,
        TRIGGER_OFF_VALUE: TRIGGER_OFF_VALUE,
        TRIGGER_ON_VALUE: TRIGGER_ON_VALUE,
        DISPATCHER_HOVERING_LIST: DISPATCHER_HOVERING_LIST,
        worldPositionToRegistrationFrameMatrix: worldPositionToRegistrationFrameMatrix,
        handsAreTracked: handsAreTracked
    };
}
