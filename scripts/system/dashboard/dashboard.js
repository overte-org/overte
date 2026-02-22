// dashboard.js
// Created by Ada <ada@thingvellir.net> on 2026-01-20
// SPDX-License-Identifier: Apache-2.0
"use strict";

// TODO: use the system utilMath once it's been merged
const { Vector3, Quaternion, vec3, quat, euler } = require("./utilMath.js");

const Defs = require("./consts.js");
const { WindowManager } = require("./dash_windows.js");

class Dashboard {
    static instance = new Dashboard();

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

    /** @type {Uuid} */
    #appbarPanelID;

    /** @type {Uuid} */
    #notifPanelID;
    /** @type {Vector3} */
    #notifPos;
    /** @type {Quaternion} */
    #notifRot;

    #updateCallback;
    #scaleCallback;
    #announcementCallback;
    #snapshotTakenCallback;
    #snapshotAnimStartCallback;
    #snapshotAnimDoneCallback;
    #eventCallback;
    #keyPressCallback;

    #setupCallbacks() {
        // FIXME: Script.update only ticks when out of safe landing mode,
        // which makes it unusable for a system UI
        // https://github.com/overte-org/overte/issues/1532
        const UPDATE_FPS = 60;
        this.#updateCallback = Script.setInterval(
            () => this.update(1 / UPDATE_FPS),
            1000 / UPDATE_FPS
        );

        /*
        this.#updateCallback = deltaTime => this.update(deltaTime);
        Script.update.connect(this.#updateCallback);
        */

        this.#scaleCallback = () => {
            Entities.editEntity(this.#notifPanelID, { dpi: Defs.scaleHackInv(Defs.notifPanelDPI) });
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
        };
        Controller.keyPressEvent.connect(this.#keyPressCallback);
    }

    constructor() {
        this.rootID = Entities.addEntity({
            type: "Empty",
            name: "Dashboard",
            parentID: MyAvatar.SELF_ID,
            parentJointIndex: Defs.sensorToWorldJoint,
            localPosition: vec3(0, MyAvatar.userHeight * 0.75, 0),
            ignorePickIntersection: true,
            grab: { grabbable: false },
        }, "local");

        this.#notifPanelID = Entities.addEntity({
            type: "Web",
            name: "Notification Panel",
            // not attached to the dash root,
            // floats to follow view
            parentID: MyAvatar.SELF_ID,
            parentJointIndex: Defs.sensorToWorldJoint,
            ignorePickIntersection: true,
            grab: { grabbable: false },
            localDimensions: vec3(0.3, 0.4, 0),
            dpi: Defs.scaleHackInv(Defs.notifPanelDPI),
            sourceUrl: Defs.notifPanelQmlURL,
            maxFPS: 90,
            wantsKeyboardFocus: false,
            showKeyboardHighlight: false,
            useBackground: false,
            renderLayer: "front",
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

        this.#notifPos = Vector3.ZERO;
        this.#notifRot = Quaternion.IDENTITY;

        this.#setupCallbacks();
    }

    #updateNotifPanel(dt) {
        const cameraPos = vec3(Camera.position);
        const cameraRot = quat(Camera.orientation);

        const targetPos = (
            cameraRot
            .multiply(vec3(0, HMD.active ? 0 : -0.3, -0.7))
            .add(cameraPos)
        );

        const targetRot = cameraRot;

        this.#notifPos = this.#notifPos.lerpTo(targetPos, dt * 5);
        this.#notifRot = this.#notifRot.lerpTo(targetRot, dt * 5);

        Entities.editEntity(this.#notifPanelID, {
            // NOTE: we can't directly set position or rotation,
            // because for whatever reason the dimensions get
            // slightly touched and it triggers an offscreen UI
            // resize
            localPosition: Entities.worldToLocalPosition(
                this.#notifPos,
                MyAvatar.SELF_ID,
                Defs.sensorToWorldJoint,
                true
            ),
            localRotation: Entities.worldToLocalRotation(
                this.#notifRot,
                MyAvatar.SELF_ID,
                Defs.sensorToWorldJoint,
                true
            ),
        });
    }

    postNotification({ text, icon, image, lifetime = 5 }) {
        Entities.emitScriptEvent(this.#notifPanelID, JSON.stringify({
            notify: {
                text,
                icon,
                image,
                lifetime,
            }
        }));
    }

    update(deltaTime) {
        this.#updateNotifPanel(deltaTime);
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

        // FIXME: https://github.com/overte-org/overte/issues/1532
        //Script.update.connect(this.#updateCallback);
        Script.clearInterval(this.#updateCallback);

        Entities.deleteEntity(this.#notifPanelID);
        Entities.deleteEntity(this.#appbarPanelID);
        this.windowManager.dispose();
    }
}

Script.scriptEnding.connect(() => {
    Dashboard.instance.dispose();
});
