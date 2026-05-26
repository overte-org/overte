// flycam.js
// Created by Ada <ada@thingvellir.net> on 2026-01-14
// Copyright Overte e.V. 2026
// SPDX-License-Identifier: Apache-2.0
"use strict";

// TODO: Script.renderUpdate or something that isn't
// paused during safe landing and ticks at the display fps
const UPDATE_FPS = Settings.getValue("flyCam/updateFPS", 60);

const CAM_FRICTION = Settings.getValue("flyCam/friction", 10);
const CAM_ACCELERATION = Settings.getValue("flyCam/acceleration", 50);

const CAM_SPEED_BASE = Settings.getValue("flyCam/baseSpeed", 1.2);
const CAM_SPEED_SPRINT = Settings.getValue("flyCam/sprintSpeedMultiplier", 4);
const CAM_SPEED_SNEAK = Settings.getValue("flyCam/sneakSpeedMultiplier", 0.25);

let camState = {
    enabled: false,
    relativeToAvatar: false,
    position: { x: 0, y: 0, z: 0 },
    velocity: { x: 0, y: 0, z: 0 },
    rotation: { x: 0, y: 0, z: 0 },
    wishdir: { x: 0, y: 0, z: 0 },
    previousMode: Camera.mode,
};

let mouseState = {
    x: 0,
    y: 0,
    previousX: 0,
    previousY: 0,
    dx: 0,
    dy: 0,
    skipFirst: true,
};

let keyState = {
    forward: false,
    backward: false,
    left: false,
    right: false,
    up: false,
    down: false,
    sprint: false,
    sneak: false,
};

function startCamera(relativeToAvatar = false) {
    if (HMD.active) {
        Window.displayAnnouncement("The fly cam isn't available in VR.");
        return;
    }

    // something else is already using the camera
    if (Camera.mode === "independent" || Camera.mode === "entity") {
        Window.displayAnnouncement("The camera is already being controlled by another script. The fly cam can't be enabled.");
        return;
    }

    camState.enabled = true;
    camState.relativeToAvatar = relativeToAvatar;
    camState.previousMode = Camera.mode;

    if (camState.relativeToAvatar) {
        camState.position = Vec3.subtract(Camera.position, MyAvatar.position);
    } else {
        camState.position = Camera.position;
    }
    camState.rotation = Quat.safeEulerAngles(Quat.cancelOutRoll(Camera.orientation));
    camState.velocity = { x: 0, y: 0, z: 0 };

    Camera.mode = "independent";
}

function stopCamera() {
    if (!camState.enabled) { return; }

    camState.enabled = false;
    camState.relativeToAvatar = false;
    camState.velocity = { x: 0, y: 0, z: 0 };
    Camera.mode = camState.previousMode;
}

function setWishdir() {
    camState.wishdir = { x: 0, y: 0, z: 0 };
    camState.wishdir.z += Number(keyState.forward);
    camState.wishdir.z -= Number(keyState.backward);
    camState.wishdir.x -= Number(keyState.left);
    camState.wishdir.x += Number(keyState.right);
    camState.wishdir.y += Number(keyState.up);
    camState.wishdir.y -= Number(keyState.down);

    let speed = CAM_SPEED_BASE;
    speed *= keyState.sprint ? CAM_SPEED_SPRINT : 1;
    speed *= keyState.sneak ? CAM_SPEED_SNEAK : 1;

    camState.wishdir = Vec3.multiply(camState.wishdir, speed);
}

function update(dt) {
    if (!camState.enabled) { return; }
    setWishdir();

    // clamp to 100ms to prevent runaway at low fps
    dt = Math.min(dt, 0.1);

    const rotQuat = Quat.fromPitchYawRollDegrees(camState.rotation.x, camState.rotation.y, camState.rotation.z);

    camState.velocity = Vec3.multiply(camState.velocity, 1.0 - (CAM_FRICTION * dt));

    camState.velocity = Vec3.sum(camState.velocity,
        Vec3.multiply(Quat.getForward(rotQuat), camState.wishdir.z * dt * CAM_ACCELERATION));
    camState.velocity = Vec3.sum(camState.velocity,
        Vec3.multiply(Quat.getRight(rotQuat), camState.wishdir.x * dt * CAM_ACCELERATION));
    camState.velocity = Vec3.sum(camState.velocity,
        { x: 0, y: camState.wishdir.y * dt * CAM_ACCELERATION, z: 0 });

    camState.position = Vec3.sum(camState.position, Vec3.multiply(camState.velocity, dt));

    if (camState.relativeToAvatar) {
        Camera.position = Vec3.sum(camState.position, MyAvatar.position);
    } else {
        Camera.position = camState.position;
    }
    Camera.orientation = rotQuat;
}

Controller.keyPressEvent.connect(e => {
    if (e.isAutoRepeat) { return; }

    switch (e.key) {
        case /* Qt::Key_F4 */ 0x01000033: {
            if (!camState.enabled) {
                startCamera(e.isShifted);
            } else {
                stopCamera();
            }
        } break;

        case /* Qt::Key_W */ 0x57: keyState.forward = true; break;
        case /* Qt::Key_S */ 0x53: keyState.backward = true; break;
        case /* Qt::Key_A */ 0x41: keyState.left = true; break;
        case /* Qt::Key_D */ 0x44: keyState.right = true; break;
        case /* Qt::Key_Q */ 0x51: keyState.down = true; break;
        case /* Qt::Key_E */ 0x45: keyState.up = true; break;
        case /* Qt::Key_Shift */ 0x01000020: keyState.sprint = true; break;
        case /* Qt::Key_Control */ 0x01000021: keyState.sneak = true; break;
    }
});

Controller.keyReleaseEvent.connect(e => {
    if (e.isAutoRepeat) { return; }

    switch (e.key) {
        case /* Qt::Key_W */ 0x57: keyState.forward = false; break;
        case /* Qt::Key_S */ 0x53: keyState.backward = false; break;
        case /* Qt::Key_A */ 0x41: keyState.left = false; break;
        case /* Qt::Key_D */ 0x44: keyState.right = false; break;
        case /* Qt::Key_Q */ 0x51: keyState.down = false; break;
        case /* Qt::Key_E */ 0x45: keyState.up = false; break;
        case /* Qt::Key_Shift */ 0x01000020: keyState.sprint = false; break;
        case /* Qt::Key_Control */ 0x01000021: keyState.sneak = false; break;
    }
});

Controller.mouseMoveEvent.connect(e => {
    if (!camState.enabled) { return; }
    if (!e.isRightButton) { return; }

    // FIXME: The flycam currently doesn't work *at all*
    // with mouselook enabled. The cursor recentering
    // counts as a full mouse motion event, so the
    // delta calculation just ends up back at zero.
    mouseState.previousX = mouseState.x;
    mouseState.previousY = mouseState.y;
    mouseState.x = e.x;
    mouseState.y = e.y;
    mouseState.dx = mouseState.x - mouseState.previousX;
    mouseState.dy = mouseState.y - mouseState.previousY;

    // skip the first mouse event so we have a baseline for
    // delta calculation, otherwise the camera will turn towards
    // the mouse cursor immediately when pressing the right button
    if (mouseState.skipFirst) {
        mouseState.skipFirst = false;
        return;
    }

    camState.rotation.y += -mouseState.dx / 8;
    camState.rotation.x += -mouseState.dy / 8;

    // real modulus to wrap between [0, 360], % is remainder and isn't quite what we want here
    camState.rotation.y = ((camState.rotation.y % 360) + 360) % 360;
    camState.rotation.x = Math.min(90, Math.max(-90, camState.rotation.x));
});

Controller.mouseReleaseEvent.connect(e => {
    if (e.isRightButton) { mouseState.skipFirst = true; }
});

Script.setInterval(() => update(1 / UPDATE_FPS), 1000 / UPDATE_FPS);

Script.scriptEnding.connect(() => stopCamera());
