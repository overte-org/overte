"use strict";
//
// Created by Ada <ada@thingvellir.net> on 2025-05-07
// Copyright 2025 Overte e.V.
//
// Distributed under the Apache License, Version 2.0.
// See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
// SPDX-License-Identifier: Apache-2.0
//
const TARGET_MODEL_URL = Script.resolvePath("../../assets/models/teleportationSpotBasev8.fbx");
const TARGET_MODEL_DIMENSIONS = { x: 0.6552, y: 0.3063, z: 0.6552 };

const TELEPORT_ACTION = Controller.Standard.RY;
const TELEPORT_CONFIRM_ACTION = Controller.Standard.LY;
const TELEPORT_ACTION_THRESHOLD = 0.8;

const TELEPORT_POINTER_RENDER_STATES = [
    {
        name: "invalid",
        distance: 8.0,
        path: { color: {red: 255, green: 184, blue: 73}, alpha: 0.7, width: 0.025 },
    },
    {
        name: "teleport",
        distance: 8.0,
        path: { color: {red: 97, green: 247, blue: 255}, alpha: 0.7, width: 0.025 },
        end: {
            type: "Model",
            modelURL: TARGET_MODEL_URL,
            dimensions: TARGET_MODEL_DIMENSIONS,
            ignorePickIntersection: true,
        }
    },
];

const teleportPointer = Pointers.createParabolaPointer({
    joint: "_CAMERA_RELATIVE_CONTROLLER_RIGHTHAND",
    dirOffset: { x: 0, y: 1, z: 0.1 },
    posOffset: { x: 0.03, y: 0.2, z: 0.02 },
    filter: Picks.PICK_ENTITIES | Picks.PICK_INCLUDE_INVISIBLE,
    faceAvatar: true,
    scaleWithParent: true,
    centerEndY: false,
    // same constants as the old teleport script
    speed: 9.3,
    accelerationAxis: {x: 0, y: -5, z: 0},
    rotateAccelerationWithAvatar: true,
    renderStates: TELEPORT_POINTER_RENDER_STATES,
    defaultRenderStates: TELEPORT_POINTER_RENDER_STATES,
    maxDistance: 8.0
});

let activateActionStrength = 0;
let confirmActionStrength = 0;
let targetPosition = undefined;
let justTeleported = false;

function onInputEvent(action, value) {
    if (!MyAvatar.allowTeleporting) { return; }

    if (action === TELEPORT_ACTION) {
        activateActionStrength = -value;
    } else if (action === TELEPORT_CONFIRM_ACTION) {
        confirmActionStrength = -value;
    }

    if (activateActionStrength >= TELEPORT_ACTION_THRESHOLD) {
        Pointers.enablePointer(teleportPointer);

        // disable avatar movement when the teleport is active
        MyAvatar.disableDriveKey(DriveKeys.TRANSLATE_X);
        MyAvatar.disableDriveKey(DriveKeys.TRANSLATE_Y);
        MyAvatar.disableDriveKey(DriveKeys.TRANSLATE_Z);
    } else {
        Pointers.disablePointer(teleportPointer);

        MyAvatar.enableDriveKey(DriveKeys.TRANSLATE_X);
        MyAvatar.enableDriveKey(DriveKeys.TRANSLATE_Y);
        MyAvatar.enableDriveKey(DriveKeys.TRANSLATE_Z);

        justTeleported = false;
        targetPosition = undefined;
    }

    if (
        confirmActionStrength >= TELEPORT_ACTION_THRESHOLD
        && targetPosition !== undefined
        && !justTeleported
    ) {
        MyAvatar.goToFeetLocation(targetPosition);

        HMD.centerUI();
        MyAvatar.centerBody();

        justTeleported = true;
        targetPosition = undefined;
    }

    if (confirmActionStrength <= 0) { justTeleported = false; }
}

function onUpdate(_delta) {
    if (!Pointers.isPointerEnabled(teleportPointer)) { return; }

    const hit = Pointers.getPrevPickResult(teleportPointer);

    if (!hit.intersects || Vec3.dot(hit.surfaceNormal, [0, 1, 0]) <= 0) {
        Pointers.setRenderState(teleportPointer, "invalid");
        targetPosition = undefined;
    } else {
        Pointers.setRenderState(teleportPointer, "teleport");
        targetPosition = hit.intersection;
    }
}

Controller.inputEvent.connect(onInputEvent);
Script.update.connect(onUpdate);

Script.scriptEnding.connect(() => {
    Controller.inputEvent.disconnect(onInputEvent);
    Pointers.removePointer(teleportPointer);
    Script.update.disconnect(onUpdate);

    MyAvatar.enableDriveKey(DriveKeys.TRANSLATE_X);
    MyAvatar.enableDriveKey(DriveKeys.TRANSLATE_Y);
    MyAvatar.enableDriveKey(DriveKeys.TRANSLATE_Z);
});
