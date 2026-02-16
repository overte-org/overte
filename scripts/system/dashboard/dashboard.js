// dashboard.js
// Created by Ada <ada@thingvellir.net> on 2026-01-20
// SPDX-License-Identifier: Apache-2.0
"use strict";

// FIXME: Entity scaling is *very* broken in the engine
function scaleHack(n) { return n * MyAvatar.sensorToWorldScale; }
function scaleHackI(n) { return n / MyAvatar.sensorToWorldScale; }

const IPC_MSG_CHANNEL = "Overte-DashboardIPC";
const SENSOR_TO_WORLD_MATRIX_INDEX = 65534;

const QML_APPBAR = `${Script.resourcesPath()}qml/overte/dash/DashBar.qml`;
const QML_WINDOW = `${Script.resourcesPath()}qml/overte/dash/DashWindow.qml`;
const QML_SETTINGS = `${Script.resourcesPath()}qml/overte/settings/Settings.qml`;

const dashRoot = Entities.addEntity({
    type: "Empty",
    name: "Dashboard",
    localPosition: [0, 1.6, 0],
    localRotation: Quat.IDENTITY,
    parentID: MyAvatar.SELF_ID,
    parentJointIndex: SENSOR_TO_WORLD_MATRIX_INDEX,
    ignorePickIntersection: true,
}, "local");

const DEFAULT_PROPS = {
    parentID: dashRoot,
    grab: { grabbable: false },
};

const PANEL_DEFAULT_PROPS = {
    ...DEFAULT_PROPS,
    maxFPS: 90,
    wantsKeyboardFocus: false,
    showKeyboardFocusHighlight: false,
    useBackground: false,
    inputMode: "mouse",
};

const appbar = Entities.addEntity({
    type: "Web",
    localDimensions: [0.5, 0.1, 0],
    localPosition: [0, 0, -0.5],
    localRotation: Quat.fromPitchYawRollDegrees(-30, 0, 0),
    ...PANEL_DEFAULT_PROPS,
    sourceUrl: QML_APPBAR,
    dpi: scaleHackI(35),
}, "local");

let railPoints = [];

for (let i = 0; i < 32; i++) {
    const x = (((i / 16) - 1) * 0.4) + 0.5;
    const z = (i / 32);
    railPoints.push([Math.cos(x * Math.PI), 0, Math.sin(z * Math.PI) * -0.4]);
}

const railVisual = Entities.addEntity({
    type: "PolyLine",
    ...DEFAULT_PROPS,
    localPosition: [0, 0, -0.4],
    color: [240, 140, 255],
    glow: true,
    faceCamera: true,
    linePoints: railPoints,
    normals: Array(32).fill([0, 0, 1]),
    strokeWidths: Array(32).fill(scaleHack(0.1)),
}, "local");

const testWindows = new Set();
const windowData = new Map();
let focusedWindow = null;

Entities.keyboardFocusEntityChanged.connect(entity => {
    if (entity !== focusedWindow) {
        Entities.emitScriptEvent(focusedWindow, JSON.stringify({
            event: "unfocus",
        }));
        focusedWindow = null;
    }

    if (testWindows.has(entity)) {
        focusedWindow = entity;
        Entities.emitScriptEvent(focusedWindow, JSON.stringify({
            event: "focus",
        }));
    }
});

Entities.webEventReceived.connect((entity, rawMsg) => {
    console.debug(entity, rawMsg);
    const msg = JSON.parse(rawMsg);

    switch (msg.event) {
        case "spawn_window": {
            const entity = Entities.addEntity({
                type: "Web",
                localDimensions: [0.5, 0.7, 0],
                localPosition: [0, 0.4, -0.7],
                localRotation: Quat.fromPitchYawRollDegrees(0, 0, 0),
                ...PANEL_DEFAULT_PROPS,
                wantsKeyboardFocus: true,
                sourceUrl: QML_WINDOW,
                dpi: scaleHackI(24),
            }, "local");

            testWindows.add(entity);
            windowData.set(entity, {
                title: msg.title,
                qmlSource: msg.qmlSource,
            });

            Script.setTimeout(() => {
            const data = windowData.get(entity);
            Entities.emitScriptEvent(entity, JSON.stringify({
                event: "open",
                title: data.title,
                qmlSource: data.qmlSource,
            }));
            }, 200);
        } break;

        case "window_spawned": {
            const data = windowData.get(entity);
            Entities.emitScriptEvent(entity, JSON.stringify({
                event: "open",
                title: data.title,
                qmlSource: data.qmlSource,
            }));
        } break;

        case "finished_closing": {
            Entities.deleteEntity(entity);
            testWindows.delete(entity);
            windowData.delete(entity);
        } break;
    }
});

// FIXME: Entity scaling is *very* broken in the engine
MyAvatar.sensorToWorldScaleChanged.connect(() => {
    Entities.editEntity(appbar, { dpi: scaleHackI(35) });
    Entities.editEntity(railVisual, { strokeWidths: Array(32).fill(scaleHack(0.1)) });

    for (const window of testWindows) {
        Entities.editEntity(window, { dpi: scaleHackI(24) });
    }
});

Script.scriptEnding.connect(() => {
    Entities.deleteEntity(dashRoot);
    Entities.deleteEntity(appbar);
    Entities.deleteEntity(railVisual);

    for (const window of testWindows) {
        Entities.deleteEntity(window);
    }
});
