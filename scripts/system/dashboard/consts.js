// consts.js
// Created by Ada <ada@thingvellir.net> 2026-02-16
// SPDX-License-Identifier: Apache-2.0

const OVERTE_QML = `${Script.resourcesPath()}qml/overte`;

module.exports = {
    ipcChannel: "System Dashboard IPC",

    /**
     * The `dpi` app windows will have.
     * @readonly
     * @type {number}
     */
    windowDPI: 24,

    /**
     * The `localDimensions` app windows will have by default.
     * @readonly
     * @type {Vector3}
     */
    windowDimensions: { x: 0.53, y: 0.8, z: 0 },

    /**
     * A URL to DashWindow.qml.
     * @readonly
     * @type {string}
     */
    windowRootQmlURL: `${OVERTE_QML}/dash/DashWindow.qml`,

    /**
     * How far the window rail should curve,
     * in meters relative to the root.
     * @readonly
     * @type {number}
     */
    windowRailCurvature: 0.1,

    /**
     * How long the window rail should be,
     * in meters relative to the root.
     * @readonly
     * @type {number}
     */
    windowRailWidth: 1,

    /**
     * How many vertices the window rail will have.
     * @readonly
     * @type {number}
     */
    windowRailResolution: 32,

    /**
     * How far away the window rail will be from the dash root.
     * @readonly
     * @type {number}
     */
    windowRailDistance: 0.7,

    /**
     * The `dpi` the app bar will have.
     * @readonly
     * @type {number}
     */
    dashBarDPI: 20,

    /**
     * A URL to DashBar.qml.
     * @readonly
     * @type {string}
     */
    dashBarQmlURL: `${OVERTE_QML}/dash/DashBar.qml`,

    /**
     * The `dpi` the notifications panel will have.
     * @readonly
     * @type {number}
     */
    notifyPanelDPI: 30,

    /**
     * A URL to DashNotifyPanel.qml.
     * @readonly
     * @type {string}
     */
    notifyPanelQmlURL: `${OVERTE_QML}/dash/DashNotifyPanel.qml`,

    /**
     * How quickly the notify panel floats to follow
     * the player's view in VR. This is unused on desktop,
     * where the notify panel is directly attached to the camera.
     * @readonly
     * @type {number}
     */
    notifyPanelFloatSpeed: 3,

    /**
     * `n * MyAvatar.sensorToWorldScale`
     * For working around buggy entity scaling
     *
     * FIXME: Remove this when entity scaling works properly
     * @param {number} n
     * @returns {number}
     */
    scaleHack(n) { return n * MyAvatar.sensorToWorldScale; },

    /**
     * `n / MyAvatar.sensorToWorldScale`
     * For working around buggy entity scaling
     *
     * FIXME: Remove this when entity scaling works properly
     * @param {number} n
     * @returns {number}
     */
    scaleHackInv(n) { return n / MyAvatar.sensorToWorldScale; },

    /**
     * The magic joint on MyAvatar that parents entities to the
     * VR playspace origin, rather than the character controller.
     *
     * FIXME: Positions scale properly, but dimensions and Web `dpi` don't.
     * @readonly
     * @type {number}
     */
    sensorToWorldJoint: 65534,

    /**
     * The magic joint on MyAvatar that parents
     * entities to the view camera.
     *
     * FIXME: Things parented to the camera joint aren't scaled relative
     * to the playspace, so everything has to be scaled manually.
     * @readonly
     * @type {number}
     */
    cameraJoint: 65529,
};
