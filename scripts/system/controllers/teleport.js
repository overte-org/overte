"use strict";
//
//  Created by Ada <ada@thingvellir.net> on 2025-05-07
//  Copyright 2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//
const TARGET_MODEL_URL = Script.resolvePath("../assets/models/teleportationSpotBasev8.fbx");
const TARGET_MODEL_DIMENSIONS = [0.6552, 0.3063, 0.6552];

const TELEPORT_ACTION_THRESHOLD = 0.8;

const TELEPORT_POINTER_RENDER_STATES = [
    {
        name: "invalid",
        distance: 8.0,
        path: {
            color: { red: 255, green: 184, blue: 73 },
            alpha: 0.7,
            width: 0.025,
        },
        end: {
            // TODO: Use an Empty when they're merged
            type: "Shape",
            shape: "Box",
            alpha: 0.0,
            collisionless: true,
            ignorePickIntersection: true,
        },
    },
    {
        name: "teleport",
        distance: 8.0,
        path: {
            color: { red: 97, green: 247, blue: 255 },
            alpha: 0.7,
            width: 0.025,
        },
        end: {
            // TODO: Use an Empty when they're merged
            type: "Shape",
            shape: "Box",
            alpha: 0.0,
            collisionless: true,
            ignorePickIntersection: true,
        },
    },
];

let teleportPointer;
let teleportControllerMapping;
let capsulePick;
let teleportIndicator;
let debugIndicator;

let activateActionStrength = 0;
let confirmActionStrength = 0;
let targetPosition = undefined;
let justTeleported = false;

let forceDisabled = false;
let ignoredEntities = new Set();

function createPointer(hand) {
    let leftHanded = hand === "left";
    let teleportAction = leftHanded ? Controller.Standard.LY : Controller.Standard.RY;
    let teleportConfirmAction = leftHanded ? Controller.Standard.RY : Controller.Standard.LY;

    if (teleportPointer) { Pointers.removePointer(teleportPointer); }
    if (teleportControllerMapping) { teleportControllerMapping.disable(); }
    if (capsulePick) { Picks.removePick(capsulePick); }
    if (teleportIndicator) { Entities.deleteEntity(teleportIndicator); }
    if (debugIndicator) { Entities.deleteEntity(debugIndicator); }

    teleportPointer = Pointers.createParabolaPointer({
        joint: leftHanded ?
            "_CAMERA_RELATIVE_CONTROLLER_LEFTHAND" :
            "_CAMERA_RELATIVE_CONTROLLER_RIGHTHAND",
        dirOffset: { x: 0, y: 1, z: 0.1 },
        posOffset: { x: leftHanded ? -0.03 : 0.03, y: 0.2, z: 0.02 },
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

    // FIXME: The height capsule's position is only updated
    // when this renderstate is active, so we can't use the
    // "invalid" state for now :<
    Pointers.setRenderState(teleportPointer, "teleport");

    const teleportPointerProps = Pointers.getPointerProperties(teleportPointer);
    const endEntity = teleportPointerProps.renderStates.teleport.end;
    const avatarHeight = MyAvatar.getHeight() / MyAvatar.scale;

    teleportIndicator = Entities.addEntity({
        parentID: endEntity,
        type: "Model",
        modelURL: TARGET_MODEL_URL,
        dimensions: TARGET_MODEL_DIMENSIONS,
        localPosition: [0, 0.1, 0],
    });

    // NOTE: this is needed as a parent transform offset for
    // capsulePick, its own position offset doesn't work
    debugIndicator = Entities.addEntity({
        parentID: endEntity,
        type: "Shape",
        shape: "Cylinder",
        ignorePickIntersection: true,
        collisionless: true,
        dimensions: [0.5, avatarHeight / 2, 0.5],
        localPosition: [0, avatarHeight / 2, 0],
        alpha: 0.0,
    }, "local");

    capsulePick = Picks.createPick(PickType.Collision, {
        filter: Picks.PICK_ENTITIES | Picks.PICK_AVATARS | Picks.PICK_INCLUDE_VISIBLE,
        parentID: debugIndicator,
        scaleWithParent: true,
        shape: {
            shapeType: "capsule-y",
            dimensions: { x: 0.5, y: avatarHeight / 2, z: 0.5 }
        },
        // check roughly between head and hips, a whole capsule
        // including the feet would collide with stairs and ramps
        // and mark the teleport target as invalid
        // FIXME: the docs say this is relative to the parent,
        // though it doesn't seem to do anything
        // position: { x: 0.0, y: avatarHeight / 2, z: 0.0 },
    });

    teleportControllerMapping = Controller.newMapping(`overte.teleport.${hand}`);
    teleportControllerMapping.from(teleportAction).peek().to(x => {
        activateActionStrength = -x;
        onInputEvent();
    });
    teleportControllerMapping.from(teleportConfirmAction).peek().to(x => {
        confirmActionStrength = -x;
        onInputEvent();
    });
    teleportControllerMapping.enable();
}

function onInputEvent() {
    if (!MyAvatar.allowTeleporting || forceDisabled) {
        activateActionStrength = 0;
        confirmActionStrength = 0;
    }

    if (activateActionStrength >= TELEPORT_ACTION_THRESHOLD) {
        Pointers.enablePointer(teleportPointer);
        Picks.enablePick(capsulePick);
        Entities.editEntity(teleportIndicator, { visible: true });

        // disable avatar movement when the teleport is active so there's no slipping
        MyAvatar.disableDriveKey(DriveKeys.TRANSLATE_X);
        MyAvatar.disableDriveKey(DriveKeys.TRANSLATE_Y);
        MyAvatar.disableDriveKey(DriveKeys.TRANSLATE_Z);
    } else {
        Pointers.disablePointer(teleportPointer);
        Picks.disablePick(capsulePick);
        Entities.editEntity(teleportIndicator, { visible: false });

        MyAvatar.enableDriveKey(DriveKeys.TRANSLATE_X);
        MyAvatar.enableDriveKey(DriveKeys.TRANSLATE_Y);
        MyAvatar.enableDriveKey(DriveKeys.TRANSLATE_Z);

        justTeleported = false;
        targetPosition = undefined;
        lastCapsuleHit = undefined;
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

    if (confirmActionStrength < TELEPORT_ACTION_THRESHOLD) { justTeleported = false; }
}

let lastCapsuleHit;

function onUpdate(_delta) {
    if (
        !capsulePick ||
        !teleportPointer ||
        !Picks.isPickEnabled(capsulePick) ||
        !Pointers.isPointerEnabled(teleportPointer)
    ) {
        return;
    }

    const hit = Pointers.getPrevPickResult(teleportPointer);
    const lastCapsuleHit = Picks.getPrevPickResult(capsulePick);

    if (
        (lastCapsuleHit && lastCapsuleHit.intersects) ||
        !hit.intersects ||
        Vec3.dot(hit.surfaceNormal, [0, 1, 0]) <= 0.5
    ) {
        targetPosition = undefined;
        Entities.editEntity(teleportIndicator, { dimensions: [0, 0, 0] });
    } else {
        targetPosition = hit.intersection;
        Entities.editEntity(teleportIndicator, { dimensions: TARGET_MODEL_DIMENSIONS });
    }
}

function onMessage(channel, message, senderID, _localOnly) {
    if (senderID !== MyAvatar.sessionUUID) { return; }

    switch (channel) {
        case "Hifi-Teleport-Disabler": {
            if (message === "both" || message === MyAvatar.getDominantHand()) {
                forceDisabled = true;
            } else {
                forceDisabled = false;
            }
        } break;

        case "Hifi-Telport-Ignore-Add": {
            if (Uuid.isNull(message)) { return; }

            ignoredEntities.add(message);

            let tmp = [...ignoredEntities];
            Pointers.setIgnoreItems(teleportPointer, tmp);
            Picks.setIgnoreItems(capsulePick, tmp);
        } break;

        case "Hifi-Telport-Ignore-Remove": {
            if (Uuid.isNull(message)) { return; }

            ignoredEntities.delete(message);

            let tmp = [...ignoredEntities];
            Pointers.setIgnoreItems(teleportPointer, tmp);
            Picks.setIgnoreItems(capsulePick, tmp);
        } break;
    }
}

createPointer(MyAvatar.getDominantHand());
Script.update.connect(onUpdate);
MyAvatar.dominantHandChanged.connect(createPointer);

// legacy compatibility
Messages.subscribe("Hifi-Teleport-Disabler");
Messages.subscribe("Hifi-Teleport-Ignore-Add");
Messages.subscribe("Hifi-Teleport-Ignore-Remove");
Messages.messageReceived.connect(onMessage);

Script.scriptEnding.connect(() => {
    Script.update.disconnect(onUpdate);
    MyAvatar.dominantHandChanged.disconnect(createPointer);
    Messages.messageReceived.disconnect(onMessage);

    if (teleportPointer) { Pointers.removePointer(teleportPointer); }
    if (teleportControllerMapping) { teleportControllerMapping.disable(); }
    if (capsulePick) { Picks.removePick(capsulePick); }
    if (teleportIndicator) { Entities.deleteEntity(teleportIndicator); }
    if (debugIndicator) { Entities.deleteEntity(debugIndicator); }

    MyAvatar.enableDriveKey(DriveKeys.TRANSLATE_X);
    MyAvatar.enableDriveKey(DriveKeys.TRANSLATE_Y);
    MyAvatar.enableDriveKey(DriveKeys.TRANSLATE_Z);
});
