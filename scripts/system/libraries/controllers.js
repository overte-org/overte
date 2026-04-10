//  handControllerGrab.js
//
//  Created by Seth Alves on 2016-9-7
//  Copyright 2016 High Fidelity, Inc.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

/* global MyAvatar, Vec3, HMD, Controller, Camera, Quat, Settings,
   getGrabPointSphereOffset:true,
   setGrabCommunications:true,
   getGrabCommunications:true,
   getControllerWorldLocation:true
 */

var controllerStandard = Controller.Standard;

const GRAB_COMMUNICATIONS_SETTING = "io.highfidelity.isFarGrabbing";
const setGrabCommunications = function setFarGrabCommunications(on) {
    Settings.setValue(GRAB_COMMUNICATIONS_SETTING, on ? "on" : "");
};
const getGrabCommunications = function getFarGrabCommunications() {
    return !!Settings.getValue(GRAB_COMMUNICATIONS_SETTING, "");
};

// this offset needs to match the one in libraries/display-plugins/src/display-plugins/hmd/HmdDisplayPlugin.cpp:378

const getGrabPointSphereOffset = function(handController, ignoreSensorToWorldScale) {
    var GRAB_POINT_SPHERE_OFFSET = { x: 0.04, y: 0.13, z: 0.039 };  // x = upward, y = forward, z = lateral
    var offset = GRAB_POINT_SPHERE_OFFSET;
    if (handController === controllerStandard.LeftHand) {
        offset = {
            x: -GRAB_POINT_SPHERE_OFFSET.x,
            y: GRAB_POINT_SPHERE_OFFSET.y,
            z: GRAB_POINT_SPHERE_OFFSET.z
        };
    }
    if (ignoreSensorToWorldScale) {
        return offset;
    } else {
        return Vec3.multiply(MyAvatar.sensorToWorldScale, offset);
    }
};

/*@jsdoc
 *
 * This is a javascript library and is not included by default. To use the method(s) below, you must first include the library.
 *
 * @example <caption>Include this library in your script.</caption>
 * Script.include("/~/system/libraries/controllers.js");
 * @namespace Controllers
 */

/*@jsdoc
 * @typedef {Object} Controllers.controllerWorldLocation
 * @property {Vec3} position - The position of the controller, relative to the world
 * @property {Vec3} translation - Deprecated: Use position instead.
 * @property {Quat} orientation - The orientation of the controller, relative to the world
 * @property {Quat} rotation - Deprecated: Use orientation instead.
 * @property {boolean} valid
 */

/*@jsdoc
 * controllerWorldLocation is where the controller would be, in-world, with an added offset
 *
 * @example <caption>Get the controllerWorldPosition and print it to log.</caption>
 *
 * Script.include("/~/system/libraries/controllers.js");
 *
 * const controllerWorldLocation = getControllerWorldLocation(Controller.Standard.LeftHand, true);
 *
 * print(controllerWorldLocation);
 *
 *
 * @name Controllers.getControllerWorldLocation
 * @function
 * @param {Controller.Standard} handController - Which hand controller? LeftHand or RightHand.
 * @param {boolean} doOffset - true if returned position should be offset, so the grab position is in front of hand
 * @returns {Controllers.controllerWorldLocation}
 */
const getControllerWorldLocation = function (handController, doOffset) {
    var orientation;
    var position;
    var valid = false;

    if (handController >= 0) {
        var pose = Controller.getPoseValue(handController);
        valid = pose.valid;
        var controllerJointIndex;
        if (pose.valid) {
            if (handController === controllerStandard.RightHand) {
                controllerJointIndex = MyAvatar.getJointIndex("_CAMERA_RELATIVE_CONTROLLER_RIGHTHAND");
            } else {
                controllerJointIndex = MyAvatar.getJointIndex("_CAMERA_RELATIVE_CONTROLLER_LEFTHAND");
            }
            orientation = Quat.multiply(MyAvatar.orientation, MyAvatar.getAbsoluteJointRotationInObjectFrame(controllerJointIndex));
            position = Vec3.sum(MyAvatar.position, Vec3.multiplyQbyV(MyAvatar.orientation, MyAvatar.getAbsoluteJointTranslationInObjectFrame(controllerJointIndex)));

            // add to the real position so the grab-point is out in front of the hand, a bit
            if (doOffset) {
                var offset = getGrabPointSphereOffset(handController);
                position = Vec3.sum(position, Vec3.multiplyQbyV(orientation, offset));
            }

        } else if (!HMD.isHandControllerAvailable()) {
            // NOTE: keep this offset in sync with scripts/system/controllers/handControllerPointer.js:493
            var VERTICAL_HEAD_LASER_OFFSET = 0.1 * MyAvatar.sensorToWorldScale;
            position = Vec3.sum(Camera.position, Vec3.multiplyQbyV(Camera.orientation, {x: 0, y: VERTICAL_HEAD_LASER_OFFSET, z: 0}));
            orientation = Quat.multiply(Camera.orientation, Quat.angleAxis(-90, { x: 1, y: 0, z: 0 }));
            valid = true;
        }
    }

    return {position: position,
            translation: position,
            orientation: orientation,
            rotation: orientation,
            valid: valid};
};
