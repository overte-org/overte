// dashIPC.js
// Created by Ada <ada@thingvellir.net> on 2026-02-26
// Copyright 2026 Overte e.V.
// SPDX-License-Identifier: Apache-2.0
"use strict";

if (Script.context !== "client") {
    throw new Error("dashIPC only works on interface scripts");
}

const { UserSignal } = require("userSignal");

const IPC_CHANNEL = "System Dashboard IPC";

let _DASH_READY = false;

/**
 * Allows you to interact with the dashboard UI state.
 */
class Dashboard {
    static #DASH_READY = _DASH_READY;
    static #ipcID = Uuid.generate();
    static #ipcBuffer = [];
    static #visible = false;

    static #sendIPC(data) {
        const event = {
            ipc_id: this.#ipcID,
            ipc_source: "dashboard_ipc",
            ...data
        };

        if (this.#DASH_READY) {
            Messages.sendLocalMessage(IPC_CHANNEL, JSON.stringify(event));
        } else {
            this.#ipcBuffer.push(event);
        }
    }

    static #messageCallback = (channel, rawMsg, _senderID, localOnly) => {
        if (channel !== IPC_CHANNEL || !localOnly) { return; }

        let msg;
        try {
            msg = JSON.parse(rawMsg);
        } catch (e) {
            return;
        }

        // the dash has started talking, we're good to send ipc messages
        if (!this.#DASH_READY) {
            _DASH_READY = true;
            this.#DASH_READY = true;

            for (const msg of this.#ipcBuffer) { this.#sendIPC(msg); }
            this.#ipcBuffer = [];
        }

        // don't respond to our own messages
        if (msg.ipc_id === this.#ipcID && msg.ipc_source === "dashboard_ipc") {
            return;
        }

        if (msg.ipc_source === "dashboard" && msg.event === "set_dash_property") {
            if (msg.visible !== undefined) {
                this.#visible = msg.visible;
                this.visibleChanged.emit(this.#visible);
            }
        }
    };

    /**
     * Triggered when the dashboard UI is opened or closed.
     * @type {UserSignal<(visible: boolean) => void>}
     */
    static visibleChanged = new UserSignal();

    /**
     * Whether the dashboard UI is open.
     * @type {boolean}
     */
    static get visible() { return this.#visible; }

    static set visible(visible) {
        this.#visible = visible;
        this.#sendIPC({ event: "set_dash_property", visible: visible });
    }

    static {
        Messages.messageReceived.connect(this.#messageCallback);
    }
}

/**
 * An interactive UI window with QML or HTML body content.
 *
 * *Note: DashWindows are currently limited to the 3D dashboard.
 * Access to native desktop windows may be added in a future version.*
 *
 * *Note: HTML body content is currently read-only and cannot be interacted with
 * through {@link DashWindow#eventReceived} or {@link DashWindow#sendEvent}.*
 */
class DashWindow {
    /** @type {boolean} */
    #DASH_READY = _DASH_READY;

    /**
     * A temporary buffer for IPC events while we wait for dashboard.js to start.
     * @type {Array<object>}
     */
    #ipcBuffer = [];

    /**
     * IPC identifier
     * @readonly
     * @type {Uuid}
     */
    id = Uuid.generate();

    /**
     * Emitted when the window content pushes a script event,
     * typically as a JSON string.
     * @type {UserSignal<(event: string) => void>}
     */
    eventReceived = new UserSignal();

    /**
     * Emitted when the window is set up and ready to use.
     * @readonly
     * @type {UserSignal<() => void>}
     */
    ready = new UserSignal();

    /**
     * Emitted when the window has been closed.
     * @readonly
     * @type {UserSignal<() => void>}
     */
    closed = new UserSignal();

    /** @type {boolean} */
    #disposed = false;

    /**
     * Whether this window has been disposed and is no longer usable.
     * @type {boolean}
     */
    get disposed() { return this.#disposed; }

    /** @type {?string} */
    #appID;

    /**
     * The app ID this window belongs to. Currently unused.
     * @readonly
     * @type {?string}
     */
    get appID() { return this.#appID; }

    /** @type {string} */
    #title;

    /**
     * The window's title text.
     * @type {string}
     */
    get title() { return this.#title; }

    set title(title) {
        this.#title = title;
        this.#sendIPC({ event: "set_window_property", title: title });
    }

    /** @type {boolean} */
    #visible = true;

    /**
     * Whether the window is visible. This allows
     * you to hide a window without closing it.
     * @type {boolean}
     */
    get visible() { return this.#visible; }

    set visible(visible) {
        this.#visible = visible;
        this.#sendIPC({ event: "set_window_property", visible: visible });
    }

    /** @type {string} */
    #sourceURL;

    /**
     * A URL pointing to the QML or HTML body source.
     * @type {string}
     */
    get sourceURL() { return this.#sourceURL; }

    set sourceURL(url) {
        this.#sourceURL = url;
        this.#sendIPC({ event: "set_window_property", source_url: url });
    }

    /** @returns {string} */
    toString() { return `DashWindow(${this.id})`; }

    /**
     * @param {Object} args
     * @param {string} args.title - The window's titlebar text.
     * @param {string?} [args.appID=null] - The window's associated app ID.
     * @param {string} args.sourceURL - A URL pointing to the QML or HTML window body source.
     * @param {Vector3} [args.dimensions={ x: 0.53, y: 0.8, z: 0 }] - The window dimensions. The z coordinate is unused.
     */
    constructor({
        title = "Unnamed",
        appID = null,
        sourceURL,
        dimensions = { x: 0.53, y: 0.8, z: 0 },
    }) {
        if (!sourceURL) {
            throw new Error("sourceURL cannot be null");
        }

        this.#title = title;
        this.#appID = appID;
        this.#sourceURL = sourceURL;

        this.#sendIPC({
            event: "create_window",
            properties: {
                title: title,
                app_id: appID,
                source_url: sourceURL,
                dimensions,
            },
        });

        Messages.messageReceived.connect(this.#messageCallback);
        Script.scriptEnding.connect(this.#scriptEnding);
    }

    /**
     * Sends data to the window's body content.
     * @param {string|object} data
     * @returns {void}
     */
    sendEvent(data) {
        if (this.#disposed) { return; }

        this.#sendIPC({
            event: "body_event",
            body_event: typeof(data) === "string" ? data : JSON.stringify(data),
        });
    }

    /**
     * Closes the window. Once a window is closed, it cannot be reused.
     * @returns {void}
     */
    close() {
        if (this.#disposed) { return; }

        // NOTE: don't actually dispose until we get the message saying the window was closed
        this.#sendIPC({ event: "dispose" });
    }

    /**
     * Cleans up state when a window is no longer needed.
     * Automatically called when the window is closed.
     * @returns {void}
     */
    #dispose() {
        if (this.#disposed) { return; }

        this.#disposed = true;
        Messages.messageReceived.disconnect(this.#messageCallback);
        Script.scriptEnding.disconnect(this.#scriptEnding);
    }

    /**
     * @param {object} data
     * @returns {void}
     */
    #sendIPC(data) {
        if (this.#disposed) { return; }

        const event = {
            ipc_id: this.id,
            app_id: this.#appID,
            ipc_source: "window",
            ...data
        };

        if (this.#DASH_READY) {
            Messages.sendLocalMessage(IPC_CHANNEL, JSON.stringify(event));
        } else {
            this.#ipcBuffer.push(event);
        }
    }

    #scriptEnding = () => this.#dispose();

    #messageCallback = (channel, rawMsg, _senderID, localOnly) => {
        // dash windows only exist on the local client,
        // so ignore any ipc messages someone might be
        // broadcasting over the message mixer
        if (channel !== IPC_CHANNEL || !localOnly) { return; }

        let msg;
        try {
            msg = JSON.parse(rawMsg);
        } catch (e) {
            return;
        }

        // the dash has started talking, we're good to send ipc messages
        if (!this.#DASH_READY) {
            _DASH_READY = true;
            this.#DASH_READY = true;

            for (const msg of this.#ipcBuffer) { this.#sendIPC(msg); }
            this.#ipcBuffer = [];
        }

        // not targeted at us, ignore
        if (msg.ipc_id !== this.id) { return; }

        // don't respond to our own messages
        if (msg.ipc_id === this.id && msg.ipc_source === "window") {
            return;
        }

        switch (msg.event) {
            case "window_created":
                this.ready.emit();
                break;

            case "window_closed":
                this.closed.emit();
                this.#dispose();
                break;

            case "body_event":
                this.eventReceived.emit(
                    typeof(msg.body_event) === "string" ?
                        msg.body_event :
                        JSON.stringify(msg.body_event)
                );
                break;

            case "set_window_property":
                if (msg.window_properties.visible !== undefined) {
                    this.#visible = msg.window_props.visible;
                }

                if (msg.window_properties.title !== undefined) {
                    this.#title = msg.window_properties.title;
                }

                if (msg.window_properties.source_url !== undefined) {
                    this.#sourceURL = msg.window_properties.source_url;
                }
                break;

            default:
                throw new Error(`${this} Unknown event type ${msg.event}`);
        }
    };
}

/**
 * @typedef {object} DashButton.Icon
 * @property {string} idle - An 'inactive' icon image URL
 * @property {string} [active=idle] - An 'active' icon image URL
 */
/**
 * @typedef {object} DashButton.IconSet
 * @property {DashButton.Icon|string} dark - An icon that works with a dark background.
 * @property {DashButton.Icon|string} [light=dark] - An icon that works with a light background.
 * @property {DashButton.Icon|string} [darkContrast=dark] - A high contrast icon that works with a dark background.
 * @property {DashButton.Icon|string} [lightContrast=light] - A high contrast icon that works with a light background.
 */

/**
 * A script-driven button on the dashboard.
 */
class DashButton {
    /** @type {boolean} */
    #DASH_READY = _DASH_READY;

    /**
     * A temporary buffer for IPC events while we wait for dashboard.js to start.
     * @type {Array<object>}
     */
    #ipcBuffer = [];

    /**
     * IPC identifier
     * @readonly
     * @type {Uuid}
     */
    id = Uuid.generate();

    /** @type {boolean} */
    #disposed = false;
    /** @type {boolean} */
    #system = false;
    /** @type {number} */
    #order = 0;
    /** @type {string} */
    #text;
    /** @type {?string} */
    #appID;
    /** @type {ButtonIconSet} */
    #icons;
    /** @type {boolean} */
    #active;

    /**
     * Emitted when the dash button is set up and ready to use.
     * @readonly
     * @type {UserSignal<() => void>}
     */
    ready = new UserSignal();

    /**
     * Emitted when the dash button is clicked
     * with the mouse cursor or VR laser.
     * @readonly
     * @type {UserSignal<() => void>}
     */
    clicked = new UserSignal();

    /**
     * Emitted when the active state of the
     * dash button changes.
     * @readonly
     * @type {UserSignal<(active: boolean) => void>}
     */
    activeChanged = new UserSignal();

    /**
     * Emitted when the label text changes.
     * @readonly
     * @type {UserSignal<(text: string) => void>}
     */
    textChanged = new UserSignal();

    /**
     * Whether this DashButton has been disposed.
     * Disposed DashButtons cannot be reused.
     * @type {boolean}
     */
    get disposed() { return this.#disposed; }

    /**
     * This DashButton's associated app ID.
     * Currently unused.
     * @readonly
     * @type {string}
     */
    get appID() { return this.#appID; }

    /**
     * The DashButton's label.
     * @type {string}
     */
    get text() { return this.#text; }

    set text(text) {
        if (text !== this.#text) {
            this.#text = text;
            this.#sendIPC({ event: "set_button_property", text: text });
            this.textChanged.emit(this.#text);
        }
    }

    /**
     * The "active" or "toggled" state of the DashButton.
     * @type {boolean}
     */
    get active() { return this.#active; }

    set active(active) {
        if (active !== this.#active) {
            this.#active = active;
            this.#sendIPC({ event: "set_button_property", active: active });
            this.activeChanged.emit(this.#active);
        }
    }

    /** @returns {string} */
    toString() { return `DashButton(${this.id})`; }

    static #readIconSet(icon) {
        if (typeof(icon) === "string") {
            return { idle: icon, active: icon };
        } else if (typeof(icon.idle) === "string") {
            return { idle: icon.idle, active: icon.active ?? icon.idle };
        } else {
            throw new Error("ButtonIcon must be either a string or { idle: string, active: ?string }");
        }
    }

    /**
     * @param {Object} args
     * @param {string} args.text - The button label text.
     * @param {?string} args.appID - The button's associated app ID.
     * @param {DashButton.IconSet|DashButton.Icon|string} arg.icons - The button icon set or icon image URL.
     * @param {number} [arg.order=0] - The sorting order for this app button.
     * @param {boolean} [arg.system=false] - Whether this button is for a system app, which will be in the top part of the dash bar. When false, the button will be in the dash bar's apps drawer. There's limited space on the system app bar, so if there's too many buttons there the dashboard might break.
     */
    constructor({
        text,
        appID = null,
        icons,
        order = 0,
        system = false,
    }) {
        this.#text = text;
        this.#appID = appID;
        this.#order = order;
        this.#system = system;

        let iconSet;

        if (typeof(icons) === "string") {
            // "icons" is a single lone image
            iconSet = {
                dark: { idle: icons, active: icons },
                light: { idle: icons, active: icons },
                darkContrast: { idle: icons, active: icons },
                lightContrast: { idle: icons, active: icons },
            };
        } else if (typeof(icons.idle) === "string") {
            // "icons" is just idle/active
            const idle = icons.idle;
            const active = icons.active ?? idle;

            iconSet = {
                dark: { idle, active },
                light: { idle, active },
                darkContrast: { idle, active },
                lightContrast: { idle, active },
            };
        } else if (icons.dark !== undefined) {
            // "icons" is (potentially) a full set
            iconSet = {
                dark: DashButton.#readIconSet(icons.dark),
                light: DashButton.#readIconSet(icons.light ?? icons.dark),
                darkContrast: DashButton.#readIconSet(icons.darkContrast ?? icons.dark),
                lightContrast: DashButton.#readIconSet(icons.lightContrast ?? icons.light ?? icons.dark),
            };
        }

        this.#icons = iconSet;

        this.#sendIPC({
            event: "create_dash_button",
            text: this.#text,
            app_id: this.#appID,
            icons: this.#icons,
            system: this.#system,
            order: this.#order,
        });

        Messages.messageReceived.connect(this.#messageCallback);
        Script.scriptEnding.connect(this.#scriptEnding);
    }

    /**
     * Removes the button from the dashboard.
     * Once a DashButton has been disposed,
     * it cannot be reused.
     * @returns {void}
     */
    dispose() {
        if (this.#disposed) { return; }

        this.#sendIPC({ event: "dispose" });
        this.#disposed = true;
        Messages.messageReceived.disconnect(this.#messageCallback);
        Script.scriptEnding.disconnect(this.#scriptEnding);
    }

    /**
     * @param {object} data
     * @returns {void}
     */
    #sendIPC(data) {
        if (this.#disposed) { return; }

        const event = {
            ipc_id: this.id,
            app_id: this.#appID,
            ipc_source: "dash_button",
            ...data
        };

        if (this.#DASH_READY) {
            Messages.sendLocalMessage(IPC_CHANNEL, JSON.stringify(event));
        } else {
            this.#ipcBuffer.push(event);
        }
    }

    #scriptEnding = () => this.dispose();

    #messageCallback = (channel, rawMsg, _senderID, localOnly) => {
        // dash buttons only exist on the local client,
        // so ignore any ipc messages someone might be
        // broadcasting over the message mixer
        if (channel !== IPC_CHANNEL || !localOnly) { return; }

        let msg;
        try {
            msg = JSON.parse(rawMsg);
        } catch (e) {
            return;
        }

        // the dash has started talking, we're good to send ipc messages
        if (!this.#DASH_READY) {
            _DASH_READY = true;
            this.#DASH_READY = true;

            for (const msg of this.#ipcBuffer) { this.#sendIPC(msg); }
            this.#ipcBuffer = [];
        }

        // not targeted at us, ignore
        if (msg.ipc_id !== this.id) { return; }

        // don't respond to our own messages
        if (msg.ipc_id === this.id && msg.ipc_source === "dash_button") {
            return;
        }

        switch (msg.event) {
            case "button_created":
                this.ready.emit();
                break;

            case "button_clicked":
                this.clicked.emit();
                break;

            case "set_button_property":
                if (msg.active !== undefined && msg.active !== this.#active) {
                    this.#active = msg.active;
                    this.activeChanged.emit(this.#active);
                }

                if (msg.text !== undefined && msg.text !== this.#text) {
                    this.#text = msg.text;
                    this.textChanged.emit(this.#text);
                }
                break;

            default:
                console.warn(`${this} Unknown event type ${msg.event}`);
                break;
        }
    };
}

module.exports = {
    Dashboard,
    DashWindow,
    DashButton,
};
