// dash_windows.js
// Created by Ada <ada@thingvellir.net> 2026-02-16
// SPDX-License-Identifier: Apache-2.0
"use strict";

// TODO: use the system utilMath once it's been merged
const {
    Vector3,
    Quaternion,
    vec3,
    quat,
    euler,
    color,
    clamp,
} = require("./utilMath.js");

const Defs = require("./consts.js");

class DashWindow {
    #entityID;
    #sourceURL;
    #title;
    #initialized = false;

    #pinnable = false;
    #pinned = false;
    #hidden = false;
    #hiddenOverride = false;
    #grabbed = false;

    /**
     * Whether this window is pinned. Pinned windows aren't
     * attached to the window rail, and don't hide when the
     * dash is closed.
     * @type {boolean}
     */
    get pinned() { return this.#pinned; }

    set pinned(pinned) {
        const old = this.#pinned;

        if (!this.#pinnable) {
            this.#pinned = false;
        } else {
            this.#pinned = pinned;
        }

        if (!pinned && pinned != old) {
            // hide the window again if it's been unpinned while the dash is hidden
            this.#hideImpl();
            this.#setReleaseState();
        }
    }

    /** @type {Vector3} */
    desiredPosition;
    /** @type {Quaternion} */
    desiredRotation;

    /** @type {number} */
    grabReleaseTime = Date.now();
    /** @type {Vector3} */
    grabReleasePosition;
    /** @type {Quaternion} */
    grabReleaseRotation;

    constructor({
        sourceURL,
        title,
        position,
        rotation,
        entityProperties = {},
        pinnable = false,
    }) {
        this.#sourceURL = sourceURL;
        this.#title = title;
        this.#pinnable = pinnable;

        this.desiredPosition = position;
        this.desiredRotation = rotation;
        this.grabReleasePosition = position;
        this.grabReleaseRotation = rotation;

        this.#entityID = Entities.addEntity({
            type: "Web",
            name: `DashWindow: ${title}`,
            grab: { grabbable: false },
            isVisibleInSecondaryCamera: false,
            canCastShadow: false,
            collisionless: true,
            fadeInMode: "disabled",
            fadeOutMode: "disabled",

            localPosition: position,
            localRotation: rotation,

            // FIXME: localDimensions aren't actually local,
            // for some reason they're actually post-SNScale
            localDimensions: vec3(Defs.windowDimensions).multiply(MyAvatar.sensorToWorldScale),

            sourceUrl: Defs.windowRootQmlURL,
            // FIXME: use a constant when dpi scales properly
            dpi: Defs.scaleHackInv(Defs.windowDPI),
            maxFPS: 90,
            useBackground: false,
            wantsKeyboardFocus: true,
            showKeyboardFocusHighlight: false,
            // FIXME: "touch" is still glitchy and doesn't
            // work properly, use that when it works again
            inputMode: "mouse",
            ...entityProperties
        }, "local");

        // FIXME: figure out why the "window_spawned"
        // message isnt getting through and use that
        // instead of a hardcoded delay, which will
        // definitely break at low game fps
        Script.setTimeout(() => {
            this.pushWindowEvent({
                event: "set_props",
                title: this.#title,
                source_url: this.#sourceURL,
                pinnable: this.#pinnable,
            });
            this.markInitialized();
        }, 100);
    }

    /**
     * The backing Web entity ID.
     * @readonly
     * @type {Uuid}
     */
    get entityID() { return this.#entityID; }

    /**
     * A URL to the QML or HTML source
     * that defines the window body content.
     * @type {Uuid}
     */
    get sourceURL() { return this.#sourceURL; }

    set sourceURL(url) {
        if (typeof(url) !== "string") {
            throw new Error("Window.sourceURL must be string");
        }

        this.#sourceURL = url;
        this.pushWindowEvent({ event: "set_props", source_url: this.#sourceURL });
    }

    /**
     * The text shown in the titlebar.
     * @type {string}
     */
    get title() { return this.#title; }

    set title(text) {
        if (typeof(text) !== "string") {
            throw new Error("Window.title must be string");
        }

        this.#title = text;
        this.pushWindowEvent({ event: "set_props", title: this.#title });
    }

    /**
     * Whether this window's Web entity has
     * been spawned, set up, and is usable.
     * @readonly
     * @type {boolean}
     */
    get initialized() { return this.#initialized; }

    markInitialized() {
        if (this.#initialized) {
            throw new Error("Window has already been initialized");
        }

        this.#initialized = true;
    }

    /**
     * Whether this window is hidden. The window continues existing,
     * but is invisible and isn't interactive.
     * @type {boolean}
     */
    get hidden() { return this.#hidden; }

    set hidden(hide) {
        this.#hidden = hide;
        this.#hideImpl();
    }

    /**
     * Whether this window can be pinned. See {@link DashWindow.pinned}
     * @type {boolean}
     */
    get pinnable() { return this.#pinnable; }

    set pinnable(pinnable) {
        this.#pinnable = pinnable;
        this.pushWindowEvent({ event: "set_props", pinnable: this.#pinnable });
    }

    /** @type {boolean} */
    get grabbed() { return this.#grabbed; }

    set grabbed(grabbed) {
        this.#grabbed = grabbed;

        if (!grabbed) { this.#setReleaseState(); }

        // grabbing entities fiddles with its dimensions,
        // so reset them back to how they're supposed to be
        Entities.editEntity(this.entityID, {
            // FIXME: localDimensions aren't actually local,
            // for some reason they're actually post-SNScale
            localDimensions: vec3(Defs.windowDimensions).multiply(MyAvatar.sensorToWorldScale),
            // FIXME: use a constant when dpi scales properly
            dpi: Defs.scaleHackInv(Defs.windowDPI),
        });

        this.pushWindowEvent({ event: "set_props", grabbed: grabbed });
    }

    #setReleaseState() {
        this.grabReleaseTime = Date.now();

        const {
            localPosition,
            localRotation,
        } = Entities.getEntityProperties(this.entityID, ["localPosition", "localRotation"]);

        this.grabReleasePosition = vec3(localPosition);
        this.grabReleaseRotation = quat(localRotation);
    }

    focus() {
        this.pushWindowEvent({ event: "focus" });
    }

    unfocus() {
        this.pushWindowEvent({ event: "unfocus" });
    }

    pushWindowEvent(obj) {
        Entities.emitScriptEvent(this.#entityID, JSON.stringify({
            dash_window: obj,
        }));
    }

    #hideImpl() {
        let hidden = this.#hidden || (this.#hiddenOverride && !this.#pinned);

        if (hidden) {
            // the window manager handles the 'visible' property
            // when the fade-out animation is done
            Entities.editEntity(this.#entityID, { ignorePickIntersection: true });
            this.pushWindowEvent({ event: "hide" });
        } else {
            Entities.editEntity(this.#entityID, { visible: true, ignorePickIntersection: false });
            this.pushWindowEvent({ event: "unhide" });
        }
    }

    /**
     * Hides a window if it isn't already hiding itself.
     * Used to hide unpinned windows when closing the dash.
     * @type {boolean}
     */
    get hiddenOverride() { return this.#hiddenOverride; }

    set hiddenOverride(hide) {
        this.#hiddenOverride = hide;
        this.#hideImpl();
    }

    dispose() {
        Entities.deleteEntity(this.#entityID);
    }
}

class WindowManager {
    /** @type {Map<Uuid, DashWindow>} */
    children = new Map();

    /** @type {?DashWindow} */
    focusedWindow = null;

    #rootID;
    #railID;

    /** @type {boolean} */
    #hidden = true;

    #setupRail() {
        const vertCount = Defs.windowRailResolution;

        let linePoints = Array(vertCount);

        for (let i = 0; i < vertCount; i++) {
            const x = (2 * (i / vertCount)) - 1;
            const z = Math.cos(x * (Math.PI / 2));
            linePoints[i] = vec3(x * Defs.windowRailWidth, 0, z * -Defs.windowRailCurvature);
        }

        this.#railID = Entities.addEntity({
            type: "PolyLine",
            name: "Window Rail",
            parentID: this.#rootID,
            grab: { grabbable: false },
            isVisibleInSecondaryCamera: false,
            canCastShadow: false,
            collisionless: true,
            fadeInMode: "disabled",
            fadeOutMode: "disabled",

            localPosition: [0, 0, -Defs.windowRailDistance],
            faceCamera: true,
            linePoints,
            color: color(255, 192, 255),
            normals: Array(vertCount).fill([0, 0, -1]),
            strokeWidths: Array(vertCount).fill(0.003),

            // dash is hidden by default
            visible: false,
        }, "local");
    }

    constructor(rootID) {
        this.#rootID = rootID;
        this.#setupRail();

        Entities.keyboardFocusEntityChanged.connect(this.#focusCallback);
        Entities.webEventReceived.connect(this.#eventCallback);
        MyAvatar.sensorToWorldScaleChanged.connect(this.#scaleCallback);
        Messages.messageReceived.connect(this.#messageCallback);
    }

    windowEvent(window, event) {
        switch (event.event) {
            case "window_spawned": {
                window.pushWindowEvent({
                    event: "set_props",
                    title: window.title,
                    source_url: window.sourceURL,
                });
                window.markInitialized();
            } break;

            case "spawn_window": {
                const pos = vec3(
                    0,
                    (Defs.windowDimensions.y / 2) + 0.01,
                    -(Defs.windowRailCurvature + Defs.windowRailDistance)
                );
                const rot = euler(0, 0, 0);
                let newWindow = new DashWindow({
                    sourceURL: event.source_url,
                    title: event.title,
                    pinnable: event.pinnable ?? true,
                    position: pos,
                    rotation: rot,
                    entityProperties: { parentID: this.#rootID },
                });
                this.children.set(newWindow.entityID, newWindow);
            } break;

            case "finished_closing": {
                this.children.delete(window.entityID);
                window.dispose();
            } break;

            case "finished_hiding": {
                Entities.editEntity(window.entityID, { visible: false });
            } break;

            case "set_grabbable": {
                Entities.editEntity(window.entityID, { grab: { grabbable: event.grabbable } });
            } break;

            case "pin": { window.pinned = true; } break;
            case "unpin": { window.pinned = false; } break;

            default: {
                console.error(`Unknown dash_window event "${event.event}"! Ignoring.`);
            } break;
        }
    }

    /**
     * Whether the child windows and rail are hidden.
     * @type {boolean}
     */
    get hidden() { return this.#hidden; }

    set hidden(hide) {
        this.#hidden = hide;

        for (const [_, window] of this.children) {
            window.hiddenOverride = hide;
        }

        Entities.editEntity(this.#railID, { visible: !hide });

        this.#railAnimTime = 0;
    }

    #railAnimTime = 1;

    #updateRail(dt) {
        if (this.#railAnimTime >= 1.0) { return; }

        this.#railAnimTime += dt * 2;

        const vertCount = Defs.windowRailResolution;

        let linePoints = Array(vertCount);

        for (let i = 0; i < vertCount; i++) {
            let scale = this.#hidden ? (1.0 - this.#railAnimTime) : this.#railAnimTime;

            if (this.#hidden) {
                scale = Math.pow(scale, 3);
            } else {
                scale = 1.0 - Math.pow(1.0 - scale, 3);
            }

            const x = ((2 * (i / vertCount)) - 1) * scale;
            const z = Math.cos(x * (Math.PI / 2));

            linePoints[i] = vec3(
                x * Defs.windowRailWidth,
                0,
                z * -Defs.windowRailCurvature
            );
        }

        Entities.editEntity(this.#railID, {
            linePoints,
            visible: !this.#hidden || (this.#hidden && this.#railAnimTime < (1 - dt * 4)),
        });
    }

    #updateWindows(_dt) {
        for (const [id, window] of this.children) {
            // the window is pinned, so don't do anything to it
            if (window.pinned) { continue; }

            if (window.grabbed) {
                let { localPosition: pos } = Entities.getEntityProperties(id, "localPosition");
                pos = vec3(pos);

                // sit on top of the rail
                pos.y = (Defs.windowDimensions.y / 2) + 0.01;
                pos.x = clamp(pos.x / Defs.windowRailWidth, -1, 1);
                pos.z = (Math.cos(pos.x * (Math.PI / 2)) * -Defs.windowRailCurvature) - Defs.windowRailDistance;

                let yaw = -Math.sin(pos.x * (Math.PI / 2)) * (Math.PI / 2) * Defs.windowRailCurvature;

                let rot = Quaternion.fromPitchYawRollRadians(0, yaw, 0);

                window.desiredPosition = pos;
                window.desiredRotation = rot;
            } else {
                // animate to snapping onto the rail
                const lerpMs = 300;

                let alpha = Math.min(1, (Date.now() - window.grabReleaseTime) / lerpMs);

                // exponential ease out
                alpha = 1.0 - Math.pow(1.0 - alpha, 2);

                let pos = window.grabReleasePosition.lerpTo(window.desiredPosition, alpha);
                let rot = Quat.slerp(window.grabReleaseRotation, window.desiredRotation, alpha);

                Entities.editEntity(id, {
                    localPosition: pos,
                    localRotation: quat(rot).normalized(),
                });
            }
        }
    }

    /**
     * @param {number} deltaTime
     */
    update(deltaTime) {
        this.#updateRail(deltaTime);
        this.#updateWindows(deltaTime);
    }

    dispose() {
        Entities.keyboardFocusEntityChanged.disconnect(this.#focusCallback);
        Entities.webEventReceived.disconnect(this.#eventCallback);
        MyAvatar.sensorToWorldScaleChanged.disconnect(this.#scaleCallback);
        Messages.messageReceived.disconnect(this.#messageCallback);

        Entities.deleteEntity(this.#railID);

        for (const [_, window] of this.children) {
            window.dispose();
        }
    }

    #focusCallback = entity => {
        this.focusedWindow?.unfocus();

        if (this.children.has(entity)) {
            this.focusedWindow = this.children.get(entity);
            this.focusedWindow.focus();
        } else {
            this.focusedWindow = null;
        }
    };

    #eventCallback = (entity, rawMsg) => {
        if (!this.children.has(entity)) { return; }

        let msg;
        try {
            msg = JSON.parse(rawMsg);
        } catch (_) {
            return;
        }

        if (msg?.dash_window?.event === undefined) {
            return;
        }

        const window = this.children.get(entity);
        this.windowEvent(window, msg.dash_window);
    };

    #scaleCallback = () => {
        for (const [_, window] of this.children) {
            Entities.editEntity(window.entityID, { dpi: Defs.scaleHackInv(Defs.windowDPI) });
        }
    };

    #messageCallback = (channel, rawMsg, _senderID, localOnly) => {
        if (channel !== "Hifi-Object-Manipulation" && channel !== Defs.ipcChannel) {
            return
        }

        let msg;
        try {
            msg = JSON.parse(rawMsg);
        } catch (e) {
            return;
        }

        // this channel and three undocumented entity
        // methods called by the grab scripts are the
        // only way of getting grab/release events
        if (channel === "Hifi-Object-Manipulation") {
            const window = this.children.get(msg.grabbedEntity);

            // not one of our child windows, not interested
            if (!window) { return; }

            if (msg.action === "grab") {
                window.grabbed = true;
            } else if (msg.action === "release") {
                window.grabbed = false;
            }
        }
    };
}

module.exports = {
    DashWindow,
    WindowManager,
};
