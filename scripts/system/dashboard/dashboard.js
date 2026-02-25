// dashboard.js
// Created by Ada <ada@thingvellir.net> on 2026-01-20
// SPDX-License-Identifier: Apache-2.0
"use strict";

// TODO: use the system utilMath once it's been merged
const { Vector3, Quaternion, vec3, quat, euler } = require("./utilMath.js");

const Defs = require("./consts.js");
const { WindowManager } = require("./dash_windows.js");
const { NotifyPanel } = require("./notify_panel.js");

class Dashboard {
    /** @type {boolean} */
    #visible = false;

    get visible() { return this.#visible; }
    set visible(visible) {
        this.#visible = visible;
        this.windowManager.hidden = !visible;

        Entities.emitScriptEvent(this.#appbarPanelID, JSON.stringify({
            dash_window: { event: visible ? "unhide" : "hide" }
        }));

        // setting it to invisible happens later after the animation is done,
        // it can be made non-interactive immediately no problem though
        if (visible) {
            Entities.editEntity(this.#appbarPanelID, {
                visible: true,
                ignorePickIntersection: false,
            });
        } else {
            Entities.editEntity(this.#appbarPanelID, {
                ignorePickIntersection: true,
            });
        }
    }

    /** @type {Uuid} */
    rootID;

    /** @type {WindowManager} */
    windowManager;

    /** @type {NotifyPanel} */
    notifyPanel;

    /** @type {Uuid} */
    #appbarPanelID;

    #updateCallback;
    #scaleCallback;
    #announcementCallback;
    #snapshotTakenCallback;
    #snapshotAnimStartCallback;
    #snapshotAnimDoneCallback;
    #eventCallback;
    #keyPressCallback;
    #themeChangeCallback;
    #hmdActiveCallback;
    #messageCallback;

    sendIPCMessage(data) {
        Messages.sendLocalMessage(Defs.ipcChannel, JSON.stringify(data));
    }

    #setupCallbacks() {
        // FIXME: Script.update only ticks when out of safe landing mode,
        // which makes it unusable for a system UI
        // https://github.com/overte-org/overte/issues/1532
        const UPDATE_FPS = 90;
        this.#updateCallback = Script.setInterval(
            () => this.update(1 / UPDATE_FPS),
            1000 / UPDATE_FPS
        );

        /*
        this.#updateCallback = deltaTime => this.update(deltaTime);
        Script.update.connect(this.#updateCallback);
        */

        this.#scaleCallback = () => {
            Entities.editEntity(this.#appbarPanelID, { dpi: Defs.scaleHackInv(Defs.dashBarDPI) });
        };
        MyAvatar.sensorToWorldScaleChanged.connect(this.#scaleCallback);

        this.#announcementCallback = msg => this.postNotification({ text: msg });
        Window.announcement.connect(this.#announcementCallback);

        this.#snapshotTakenCallback = (path, notify) => {
            if (!notify) { return; }

            this.postNotification({
                // TODO: translation support
                text: "Snapshot taken",
                image: `file://${path}`,
            });
        };
        Window.stillSnapshotTaken.connect(this.#snapshotTakenCallback);
        Window.snapshot360Taken.connect(this.#snapshotTakenCallback);

        this.#snapshotAnimStartCallback = _path => {
            this.postNotification({
                // TODO: translation support
                text: "Capturing animated snapshot…",
                lifetime: 2,
            });
        };
        this.#snapshotAnimDoneCallback = path => {
            this.postNotification({
                // TODO: translation support
                text: "Animated snapshot taken",
                image: `file://${path}`,
                lifetime: Settings.getValue("snapshotAnimatedDuration", 3) + 0.5,
            });
        };
        Window.processingGifStarted.connect(this.#snapshotAnimStartCallback);
        Window.processingGifCompleted.connect(this.#snapshotAnimDoneCallback);

        this.#eventCallback = (entity, rawMsg) => {
            if (entity !== this.#appbarPanelID) { return; }

            let msg;
            try {
                msg = JSON.parse(rawMsg);
            } catch (_) {
                return;
            }

            if (msg?.dash_window?.event === "spawn_window") {
                this.windowManager.windowEvent(null, msg.dash_window);
            } else if (msg?.dash_window?.event === "finished_hiding") {
                Entities.editEntity(this.#appbarPanelID, {
                    visible: false,
                    ignorePickIntersection: true,
                });
            }
        };
        Entities.webEventReceived.connect(this.#eventCallback);

        this.#keyPressCallback = event => {
            if (
                event.isShifted ||
                event.isMeta ||
                event.isControl ||
                event.isAlt ||
                event.isKeypad ||
                event.isAutoRepeat
            ) {
                return;
            }

            if (event.key === 0x01000000 /* Qt::Key_Escape */) {
                this.visible = !this.visible;
            }

            if (event.key === 0x4f /* Qt::Key_O */) {
                this.postNotification({
                    text: "Test notification",
                    icon: `${Script.resourcesPath()}qml/overte/icons/gold_star.svg`,
                    image: `${Script.resourcesPath()}qml/overte/icons/home.svg`,
                });
            }
        };
        Controller.keyPressEvent.connect(this.#keyPressCallback);

        /* FIXME: waiting on #2092
        this.#themeChangeCallback = () => sendIPCMessage({ dashboard: { event: "theme_change" } });
        Window.themeChanged.connect(this.#themeChangeCallback);*/

        this.#hmdActiveCallback = hmdActive => {};
        HMD.displayModeChanged.connect(this.#hmdActiveCallback);

        this.#messageCallback = (channel, rawMsg, senderID, localOnly) => {};
        Messages.messageReceived.connect(this.#messageCallback);
    }

    constructor() {
        this.rootID = Entities.addEntity({
            type: "Empty",
            name: "Dashboard",
            parentID: MyAvatar.SELF_ID,
            parentJointIndex: Defs.sensorToWorldJoint,
            localPosition: vec3(0, MyAvatar.userHeight * 0.62, 0),
            ignorePickIntersection: true,
            grab: { grabbable: false },
        }, "local");

        this.#appbarPanelID = Entities.addEntity({
            type: "Web",
            name: "Dash Bar",
            parentID: this.rootID,
            grab: { grabbable: false },
            localRotation: euler(-20, 0, 0),
            localPosition: vec3(0, -0.15, -(Defs.windowRailDistance + Defs.windowRailCurvature) + 0.1),
            // FIXME: localDimensions aren't actually local,
            // for some reason they're actually post-SNScale
            localDimensions: vec3(1, 0.3, 0).multiply(MyAvatar.sensorToWorldScale),
            dpi: Defs.scaleHackInv(Defs.dashBarDPI),
            sourceUrl: Defs.dashBarQmlURL,
            maxFPS: 90,
            wantsKeyboardFocus: false,
            showKeyboardHighlight: false,
            useBackground: false,
            // FIXME: "touch" is still glitchy and doesn't
            // work properly, use that when it works again
            inputMode: "mouse",

            // invisible by default
            ignorePickIntersection: true,
            visible: false,
        }, "local");

        this.windowManager = new WindowManager(this.rootID);
        this.notifyPanel = new NotifyPanel();

        this.#setupCallbacks();
    }

    /**
     * @param {object} args
     * @param {string} args.text - Notification text
     * @param {string} [args.icon] - URL to notification icon image
     * @param {string} [args.image] - URL to notification body image
     * @param {number} [args.lifetime=5]
     */
    postNotification({ text, icon, image, lifetime = 5 }) {
        this.notifyPanel.postNotification({ text, icon, image, lifetime });
    }

    /**
     * @param {number} deltaTime
     */
    update(deltaTime) {
        this.windowManager.update(deltaTime);
        this.notifyPanel.update(deltaTime);
    }

    dispose() {
        MyAvatar.sensorToWorldScaleChanged.disconnect(this.#scaleCallback);
        Window.announcement.disconnect(this.#announcementCallback);
        Window.stillSnapshotTaken.disconnect(this.#snapshotTakenCallback);
        Window.snapshot360Taken.disconnect(this.#snapshotTakenCallback);
        Window.processingGifStarted.disconnect(this.#snapshotAnimStartCallback);
        Window.processingGifCompleted.disconnect(this.#snapshotAnimDoneCallback);
        Entities.webEventReceived.disconnect(this.#eventCallback);
        Controller.keyPressEvent.disconnect(this.#keyPressCallback);
        //Window.themeChanged.disconnect(this.#themeChangeCallback);
        HMD.displayModeChanged.disconnect(this.#hmdActiveCallback);
        Messages.messageReceived.disconnect(this.#messageCallback);

        // FIXME: https://github.com/overte-org/overte/issues/1532
        //Script.update.connect(this.#updateCallback);
        Script.clearInterval(this.#updateCallback);

        Entities.deleteEntity(this.#appbarPanelID);
        this.notifyPanel.dispose();
        this.windowManager.dispose();
    }
}

const instance = new Dashboard();

Script.scriptEnding.connect(() => instance.dispose());
