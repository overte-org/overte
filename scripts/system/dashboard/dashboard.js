// dashboard.js
// Created by Ada <ada@thingvellir.net> on 2026-01-20
// SPDX-License-Identifier: Apache-2.0
"use strict";

// TODO: use the system utilMath once it's been merged
const { Vector3, Quaternion, vec3, quat, euler } = require("./utilMath.js");

const Defs = require("./consts.js");
const { WindowManager } = require("./windows.js");
const { NotifyPanel } = require("./notify_panel.js");

// Script.update only ticks when out of safe landing mode,
// which makes it unusable for a system UI
// https://github.com/overte-org/overte/issues/1532
// Script.update also ticks at 60fps, which doesn't feel smooth
// in headsets that often run at 90hz or higher
// TODO: find or create something for ticking
// animations at the right framerate
const UPDATE_FPS = 90;

class AppButtonIPC {
    /** @type {boolean} */
    system;
    /** @type {string} */
    ipcID;
    /** @type {?string} */
    appID;
    /** @type {string} */
    text;
    /** @type {boolean} */
    active;
    /** @type {object} */
    icons;
    /** @type {number} */
    order;

    constructor({
        system = false,
        ipcID,
        appID = null,
        text,
        icons,
        active = false,
        order = 0,
    }) {
        this.system = system;
        this.ipcID = ipcID;
        this.appID = appID;
        this.text = text;
        this.icons = icons;
        this.active = active;
        this.order = order;
    }

    sendIPC(msg) {
        Messages.sendLocalMessage(Defs.ipcChannel, JSON.stringify({
            ipc_id: this.ipcID,
            app_id: this.appID,
            ipc_source: "dashboard",
            ...msg
        }));
    }
}

class Dashboard {
    #pingInterval = null;

    #controllerMapping;
    #openButtonPressedTime;
    #openButtonReleasedTime;
    #openButtonReleaseHandled = false;

    #desktopToolbar = Toolbars.getToolbar("com.highfidelity.interface.toolbar.system");
    #desktopToolbarVisible = false;

    #openButtonReleased() {
        if (this.#openButtonReleaseHandled) { return; }

        if ((this.#openButtonReleasedTime - this.#openButtonPressedTime) > Defs.dashTabletDelay) {
            if (HMD.active) {
                if (HMD.showTablet) {
                    HMD.closeTablet();
                } else {
                    HMD.openTablet();
                }
            } else {
                this.#desktopToolbarVisible = !this.#desktopToolbarVisible;
                this.#desktopToolbar.writeProperty("visible", this.#desktopToolbarVisible);
            }
        } else {
            this.visible = !this.visible;
        }

        this.#openButtonReleaseHandled = true;
    }

    /** @type {boolean} */
    #visible = false;

    get visible() { return this.#visible; }

    set visible(visible) {
        // not initialized yet, wait for a bit before ack'ing
        if (this.#pingInterval === null) { return; }

        this.#visible = visible;
        this.windowManager.hidden = !visible;

        Entities.emitScriptEvent(this.#appbarPanelID, JSON.stringify({
            dash_window: { event: visible ? "unhide" : "hide" }
        }));

        this.sendIPC({
            event: "set_dash_property",
            visible: visible,
        });

        // setting it to invisible happens later after the animation is done,
        // it can be made non-interactive immediately no problem though
        if (visible) {
            this.#setDesiredPosition();

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

    /** @type {Map<Uuid,AppButtonIPC>} */
    #appButtons = new Map();

    /** @type {Vector3} */
    #desiredPosition;
    /** @type {Quaternion} */
    #desiredRotation;

    sendIPC(data) {
        Messages.sendLocalMessage(Defs.ipcChannel, JSON.stringify({
            ipc_source: "dashboard",
            ...data
        }));
    }

    #setDesiredPosition() {
        const heightOffset = HMD.active ? -0.6 : -0.3;
        const pos = Vec3.sum(Entities.worldToLocalPosition(Camera.position, MyAvatar.SELF_ID, Defs.sensorToWorldJoint), [0, heightOffset, 0]);
        const rot = Quat.cancelOutRollAndPitch(Entities.worldToLocalRotation(Camera.orientation, MyAvatar.SELF_ID, Defs.sensorToWorldJoint));

        this.#desiredPosition = vec3(pos);
        this.#desiredRotation = quat(rot);

        Entities.editEntity(this.rootID, {
            localPosition: this.#desiredPosition,
            localRotation: this.#desiredRotation,
        });
    }

    constructor() {
        this.#setDesiredPosition();

        this.rootID = Entities.addEntity({
            type: "Empty",
            name: "Dashboard",
            parentID: MyAvatar.SELF_ID,
            parentJointIndex: Defs.sensorToWorldJoint,
            localPosition: this.#desiredPosition,
            localRotation: this.#desiredRotation,
            ignorePickIntersection: true,
            grab: { grabbable: false },
        }, "local");

        this.#appbarPanelID = Entities.addEntity({
            type: "Web",
            name: "Dash Bar",
            parentID: this.rootID,
            grab: { grabbable: false },
            localRotation: euler(-20, 0, 0),
            localPosition: vec3(0, -0.16, -(Defs.windowRailDistance + Defs.windowRailCurvature) + 0.12),
            // FIXME: localDimensions aren't actually local,
            // for some reason they're actually post-SNScale
            localDimensions: vec3(Defs.dashBarDimensions).multiply(MyAvatar.sensorToWorldScale),
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

        // https://github.com/overte-org/overte/issues/1532
        /*this.#updateCallback = deltaTime => this.update(deltaTime);
        Script.update.connect(this.#updateCallback);*/

        MyAvatar.sensorToWorldScaleChanged.connect(this.#scaleCallback);
        Window.announcement.connect(this.#announcementCallback);
        Window.stillSnapshotTaken.connect(this.#snapshotTakenCallback);
        Window.snapshot360Taken.connect(this.#snapshotTakenCallback);
        Window.processingGifStarted.connect(this.#snapshotAnimStartCallback);
        Window.processingGifCompleted.connect(this.#snapshotAnimDoneCallback);
        Entities.webEventReceived.connect(this.#eventCallback);
        Controller.keyPressEvent.connect(this.#keyPressCallback);
        Controller.keyReleaseEvent.connect(this.#keyReleaseCallback);
        Window.themeChanged.connect(this.#themeChangeCallback);
        Messages.messageReceived.connect(this.#messageCallback);
        this.#desktopToolbar.writeProperty("visible", this.#desktopToolbarVisible);

        {
            const app = Controller.Hardware.Application;
            const std = Controller.Standard;

            const map = Controller.newMapping("Dashboard");

            const clickHandler = clicked => {
                if (clicked) {
                    this.#openButtonPressedTime = Date.now();
                    this.#openButtonReleaseHandled = false;
                } else {
                    this.#openButtonReleasedTime = Date.now();
                    this.#openButtonReleased();
                }
            };

            map.from(std.LeftSecondaryThumb).peek().when(app.RightHandDominant).to(clickHandler);
            map.from(std.RightSecondaryThumb).peek().when(app.LeftHandDominant).to(clickHandler);
            map.from(std.Start).peek().to(clickHandler);

            map.enable();
            this.#controllerMapping = map;
        }

        // FIXME: entities don't have any "i'm done creating" callback,
        // and entity add/edit/delete calls are tied to the framerate,
        // so if the window is minimized at startup then messages to
        // the app bar will be dropped
        Script.setTimeout(() => {
            // FIXME: is there a better solution to this problem?
            // HACK: apps using the IPC might be loaded either before or after dashboard.js,
            // pinging ensures dashIPC can know it's safe to send messages. there might be
            // a delay until a script using dashIPC knows the dash is ready, but won't have
            // any delay after sending their own IPC messages
            this.sendIPC({ event: "ping" });
            this.#pingInterval = Script.setInterval(() => this.sendIPC({ event: "ping" }), 300);
        }, 1000);
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
        // optional chain so if they fail to start
        // they won't spam the log
        this.windowManager?.update(deltaTime);
        this.notifyPanel?.update(deltaTime);

        const now = Date.now();
        if (
            now > (this.#openButtonPressedTime + Defs.dashTabletDelay) &&
            !this.#openButtonReleaseHandled
        ) {
            this.#openButtonReleasedTime = now;
            this.#openButtonReleased();
        }
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
        Window.themeChanged.disconnect(this.#themeChangeCallback);
        Messages.messageReceived.disconnect(this.#messageCallback);
        this.#controllerMapping.disable();

        // https://github.com/overte-org/overte/issues/1532
        //Script.update.connect(this.#updateCallback);
        Script.clearInterval(this.#updateCallback);
        Script.clearInterval(this.#pingInterval);

        Entities.deleteEntity(this.#appbarPanelID);
        this.notifyPanel.dispose();
        this.windowManager.dispose();
    }

    #updateCallback = Script.setInterval(
        () => this.update(1 / UPDATE_FPS),
        1000 / UPDATE_FPS
    );

    #scaleCallback = () => {
        Entities.editEntity(this.#appbarPanelID, { dpi: Defs.scaleHackInv(Defs.dashBarDPI) });
    };

    #announcementCallback = msg => this.postNotification({ text: msg });

    #snapshotTakenCallback = (path, notify) => {
        if (!notify) { return; }

        this.postNotification({
            // TODO: translation support
            text: "Snapshot taken",
            image: `file://${path}`,
        });
    };

    #snapshotAnimStartCallback = _path => {
        this.postNotification({
            // TODO: translation support
            text: "Capturing animated snapshot…",
            lifetime: 2,
        });
    };

    #snapshotAnimDoneCallback = path => {
        this.postNotification({
            // TODO: translation support
            text: "Animated snapshot taken",
            image: `file://${path}`,
            lifetime: Settings.getValue("snapshotAnimatedDuration", 3) + 0.5,
        });
    };

    #eventCallback = (entity, rawMsg) => {
        if (entity !== this.#appbarPanelID) { return; }

        let msg;
        try {
            msg = JSON.parse(rawMsg);
        } catch (_) {
            return;
        }

        if (msg.dash_window?.event === "spawn_window") {
            this.windowManager.windowEvent(null, msg.dash_window);
        } else if (msg.dash_window?.event === "finished_hiding") {
            Entities.editEntity(this.#appbarPanelID, {
                visible: false,
                ignorePickIntersection: true,
            });
        } else if (msg.app_button?.event === "clicked") {
            this.#appButtons.get(msg.app_button.ipc_id)?.sendIPC({
                ipc_source: "dashboard",
                ipc_id: msg.app_button.ipc_id,
                event: "button_clicked",
            });
        }
    };

    #keyPressCallback = event => {
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
            this.#openButtonPressedTime = Date.now();
            this.#openButtonReleaseHandled = false;
        }

        if (event.key === 0x4f /* Qt::Key_O */) {
            this.postNotification({
                text: "Test notification",
                icon: `${Script.resourcesPath()}qml/overte/icons/gold_star.svg`,
                image: `${Script.resourcesPath()}qml/overte/icons/home.svg`,
            });
        }
    };

    #keyReleaseCallback = event => {
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
            this.#openButtonReleasedTime = Date.now();
            this.#openButtonReleased();
        }
    };

    #themeChangeCallback = () => {
        this.sendIPC({ dashboard: { event: "theme_change" } });
        this.windowManager.reloadThemeSettings();
        this.notifyPanel.reloadThemeSettings();

        Entities.emitScriptEvent(this.#appbarPanelID, JSON.stringify({
            dashboard: { event: "theme_change" }
        }));
    };

    #messageCallback = (channel, rawMsg, _senderID, localOnly) => {
        if (channel !== Defs.ipcChannel || !localOnly) { return; }

        let msg;
        try {
            msg = JSON.parse(rawMsg);
        } catch (_e) {
            return;
        }

        if (msg.event === "create_dash_button") {
            let button = new AppButtonIPC({
                ipcID: msg.ipc_id,
                appID: msg.app_id,
                text: msg.text,
                icons: msg.icons,
                system: msg.system,
                order: msg.order,
            });

            this.#appButtons.set(button.ipcID, button);

            button.sendIPC({
                ipc_source: "dashboard",
                ipc_id: button.ipcID,
                event: "button_created",
            });

            Entities.emitScriptEvent(this.#appbarPanelID, JSON.stringify({
                dash_bar: {
                    event: "set_app_button",
                    ipc_id: button.ipcID,
                    data: button,
                },
            }));
        } else if (msg.event === "set_button_property" && msg.ipc_source === "dash_button") {
            let button = this.#appButtons.get(msg.ipc_id);

            if (msg.active !== undefined) { button.active = msg.active; }
            if (msg.text !== undefined) { button.text = msg.active; }

            Entities.emitScriptEvent(this.#appbarPanelID, JSON.stringify({
                dash_bar: {
                    event: "set_app_button",
                    ipc_id: button.ipcID,
                    data: button,
                },
            }));
        } else if (msg.event === "dispose" && msg.ipc_source === "dash_button") {
            this.#appButtons.delete(msg.ipc_id);
            Entities.emitScriptEvent(this.#appbarPanelID, JSON.stringify({
                dash_bar: {
                    event: "delete_app_button",
                    ipc_id: msg.ipc_id,
                },
            }));
        } else if (msg.event === "set_dash_property" && msg.ipc_source === "dashboard_ipc") {
            if (msg.visible !== undefined) { this.visible = msg.visible; }
        }
    };
}

const instance = new Dashboard();

Script.scriptEnding.connect(() => instance.dispose());
