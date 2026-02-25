// dash_windows.js
// Created by Ada <ada@thingvellir.net> 2026-02-16
// SPDX-License-Identifier: Apache-2.0
"use strict";

// TODO: use the system utilMath once it's been merged
const { Vector3, Quaternion, vec3, color } = require("./utilMath.js");

const Defs = require("./consts.js");

class DashWindow {
    #entityID;
    #sourceURL;
    #title;
    #initialized = false;

    #pinnable = false;
    #hidden = false;
    #hiddenOverride = false;

    /**
     * This window's horizontal position along the window rail.
     * Origin in the middle of the rail, and the registration
     * point at the bottom-center of the window.
     * @type {number}
     */
    x = 0;

    /**
     * Whether this window is pinned. Pinned windows aren't
     * attached to the window rail, and don't hide when the
     * dash is closed.
     * @type {boolean}
     */
    pinned = false;

    constructor({ sourceURL, title, entityProperties = {}, pinnable = false }) {
        this.#sourceURL = sourceURL;
        this.#title = title;
        this.#pinnable = pinnable;

        this.#entityID = Entities.addEntity({
            type: "Web",
            name: `DashWindow: ${title}`,
            grab: { grabbable: false },
            isVisibleInSecondaryCamera: false,
            canCastShadow: false,
            collisionless: true,
            fadeInMode: "disabled",
            fadeOutMode: "disabled",

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
        let hidden = this.#hidden || (this.#hiddenOverride && !this.pinned);

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

    /** @type {?DashWindow} */
    draggedWindow = null;

    #rootID;
    #railID;
    #focusCallback;
    #eventCallback;
    #scaleCallback;

    /** @type {boolean} */
    #hidden = true;

    #setupCallbacks() {
        this.#focusCallback = entity => {
            this.focusedWindow?.unfocus();

            if (this.children.has(entity)) {
                this.focusedWindow = this.children.get(entity);
                this.focusedWindow.focus();
            } else {
                this.focusedWindow = null;
            }
        };

        Entities.keyboardFocusEntityChanged.connect(this.#focusCallback);

        this.#eventCallback = (entity, rawMsg) => {
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

        Entities.webEventReceived.connect(this.#eventCallback);

        this.#scaleCallback = () => {
            for (const [_, window] of this.children) {
                Entities.editEntity(window.entityID, { dpi: Defs.scaleHackInv(Defs.windowDPI) });
            }
        };

        MyAvatar.sensorToWorldScaleChanged.connect(this.#scaleCallback);
    }

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
        this.#setupCallbacks();
        this.#setupRail();
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
                let newWindow = new DashWindow({
                    sourceURL: event.source_url,
                    title: event.title,
                    pinnable: event.pinnable ?? true,
                    entityProperties: {
                        parentID: this.#rootID,
                        localPosition: vec3(0, 0.41, -(Defs.windowRailCurvature + Defs.windowRailDistance)),
                    },
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

            case "begin_drag": {
                this.draggedWindow = window;
            } break;

            case "finish_drag": {
                this.draggedWindow = null;
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

    /**
     * @param {number} deltaTime
     */
    update(deltaTime) {
        this.#updateRail(deltaTime);
    }

    dispose() {
        Entities.keyboardFocusEntityChanged.disconnect(this.#focusCallback);
        Entities.webEventReceived.disconnect(this.#eventCallback);
        MyAvatar.sensorToWorldScaleChanged.disconnect(this.#scaleCallback);

        Entities.deleteEntity(this.#railID);

        for (const [_, window] of this.children) {
            window.dispose();
        }
    }
}

module.exports = {
    DashWindow,
    WindowManager,
};
