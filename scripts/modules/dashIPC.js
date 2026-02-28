// dashIPC.js
// Created by Ada <ada@thingvellir.net> on 2026-02-26
// Copyright 2026 Overte e.V.
// SPDX-License-Identifier: Apache-2.0
"use strict";

const { UserSignal } = require("userSignal");

const IPC_CHANNEL = "System Dashboard IPC";

class DashWindow {
    /**
     * IPC identifier
     * @readonly
     * @type {Uuid}
     */
    id = Uuid.generate();

    /**
     * Emitted when the window content pushes a script event,
     * typically as a JSON string.
     * @type {UserSignal}
     */
    eventReceived = new UserSignal();

    /**
     * Emitted when the window is set up and ready to use.
     * @readonly
     * @type {UserSignal}
     */
    ready = new UserSignal();

    /**
     * Emitted when the window has been closed.
     * @readonly
     * @type {UserSignal}
     */
    closed = new UserSignal();

    /** @type {boolean} */
    #disposed = false;

    /**
     * Whether this window has been disposed and is no longer usable.
     * @type {boolean}
     */
    get disposed() { return this.#disposed; }

    /** @type {string} */
    #appID;

    /**
     * The app ID this window belongs to. If a {@link DashButton}
     * has a matching app ID to this window, it will automatically
     * switch to an active state, and will switch to being inactive
     * if there's no matching windows.
     * @readonly
     * @type {string}
     */
    get appID() { return this.#appID; }

    /** @type {string} */
    #title;

    /**
     * The title text shown at the bottom of the window.
     * @type {string}
     */
    get title() { return this.#title; }

    set title(title) {
        this.#title = title;
        this.#sendIPC({ event: "set_window_property", title: title });
    }

    /** @type {boolean} */
    #visible = true;

    get visible() { return this.#visible; }

    set visible(visible) {
        this.#visible = visible;
        this.#sendIPC({ event: "set_window_property", visible: visible });
    }

    /** @type {string} */
    #sourceURL;

    get sourceURL() { return this.#sourceURL; }

    set sourceURL(url) {
        this.#sourceURL = url;
        this.#sendIPC({ event: "set_window_property", source_url: url });
    }

    /** @returns {string} */
    toString() { return `DashWindow(${this.id})`; }

    constructor({ title = "Unnamed", appID = null, sourceURL = "No source URL provided!" }) {
        this.#title = title;
        this.#appID = appID;
        this.#sourceURL = sourceURL;

        this.#sendIPC({
            event: "create_window",
            properties: {
                title: title,
                app_id: appID,
                source_url: sourceURL,
            },
        });

        Messages.messageReceived.connect(this.#messageCallback);
    }

    /**
     * Sends an event to the window.
     * @param {*} data
     * @returns {void}
     */
    sendEvent(data) {
        if (this.#disposed) { return; }

        this.#sendIPC({
            event: "body_event",
            body_event: typeof(data) === "string" ? data : JSON.stringify(data),
        });
    }

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
    }

    /**
     * @param {object} data
     * @returns {void}
     */
    #sendIPC(data) {
        if (this.#disposed) { return; }

        Messages.sendLocalMessage(IPC_CHANNEL, JSON.stringify({
            ipc_id: this.id,
            app_id: this.#appID,
            ipc_source: "window",
            ...data
        }));
    }

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
 * @typedef {object} ButtonIcon
 * @property {string} idle - An 'inactive' icon image URL
 * @property {string} [active=idle] - An 'active' icon image URL
 */
/**
 * @typedef {object} ButtonIconSet
 * @property {ButtonIcon|string} dark - An icon that works with a dark background.
 * @property {ButtonIcon|string} [light=dark] - An icon that works with a light background.
 * @property {ButtonIcon|string} [darkContrast=dark] - A high contrast icon that works with a dark background.
 * @property {ButtonIcon|string} [lightContrast=light] - A high contrast icon that works with a light background.
 */

class DashButton {
    /**
     * IPC identifier
     * @readonly
     * @type {Uuid}
     */
    id = Uuid.generate();

    /** @type {boolean} */
    #disposed = false;
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
     * @type {UserSignal}
     */
    ready = new UserSignal();

    /**
     * Emitted when the dash button is clicked
     * with the mouse cursor or VR laser.
     * @readonly
     * @type {UserSignal}
     */
    clicked = new UserSignal();

    /**
     * Emitted when the active state of the
     * dash button changes.
     * @readonly
     * @type {UserSignal}
     */
    activeChanged = new UserSignal();

    /**
     * Emitted when the label text changes.
     * @readonly
     * @type {UserSignal}
     */
    textChanged = new UserSignal();

    /** @type {boolean} */
    get disposed() { return this.#disposed; }

    /**
     * @readonly
     * @type {string}
     */
    get appID() { return this.#appID; }

    /**
     * The text shown on the dash button.
     * @type {string}
     */
    get text() { return this.#text; }

    set text(text) {
        if (text !== this.#text) {
            this.#text = text;
            this.#sendIPC({ event: "set_button_property", text: text });
            this.textChanged.emit();
        }
    }

    /**
     * The "active" state of the button.
     * @type {boolean}
     */
    get active() { return this.#active; }

    set active(active) {
        if (active !== this.#active) {
            this.#active = active;
            this.#sendIPC({ event: "set_button_property", active: active });
            this.activeChanged.emit();
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
     * @param {Object} arg
     * @param {string} arg.text - The button label text
     * @param {?string} arg.appID - The app ID this button is associated with
     * @param {ButtonIconSet|Icon|string} arg.icons - The button icon set or icon image URL
     */
    constructor({
        text,
        appID = null,
        icons,
    }) {
        this.#text = text;
        this.#appID = appID;

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
        });

        Messages.messageReceived.connect(this.#messageCallback);
    }

    /**
     * Cleans up state when a button is no longer needed.
     * @returns {void}
     */
    dispose() {
        this.#sendIPC({ event: "dispose" });
        this.#disposed = true;
        Messages.messageReceived.disconnect(this.#messageCallback);
    }

    /**
     * @param {object} data
     * @returns {void}
     */
    #sendIPC(data) {
        if (this.#disposed) { return; }

        Messages.sendLocalMessage(IPC_CHANNEL, JSON.stringify({
            ipc_id: this.id,
            app_id: this.#appID,
            ipc_source: "dash_button",
            ...data
        }));
    }

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
    DashWindow,
    DashButton,
};
