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
     * @type {string}
     */
    get appID() { return this.#appID; }

    set appID(appID) {
        this.#appID = appID;
        this.#sendIPC({ event: "set_window_property", app_id: appID });
    }

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

    /** @type {?Uuid} */
    #windowID;

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

        // automatically dispose when the window is closed
        this.closed.connect(() => this.#dispose());
    }

    /**
     * Sends an event to the window.
     * @param {*} data
     * @returns {void}
     */
    sendEvent(data) {
        if (this.#disposed) {
            throw new Error(`${this} has already been disposed`);
        }

        this.#sendIPC({
            event: "body_event",
            body_event: typeof(data) === "string" ? data : JSON.stringify(data),
        });
    }

    close() {
        this.#dispose();
    }

    /**
     * Cleans up state when a window is no longer needed.
     * Automatically called when the window is closed.
     * @returns {void}
     */
    #dispose() {
        if (this.#disposed) {
            throw new Error(`${this} has already been disposed`);
        }

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
                this.#windowID = msg.window_id;
                this.ready.emit();
                break;

            case "closed":
                this.closed.emit();
                break;

            case "body_event":
                this.eventReceived.emit(
                    typeof(msg.body_event) === "string" ?
                        msg.body_event :
                        JSON.stringify(msg.body_event)
                );
                break;

            case "set_window_property":
                if (msg.window_properties.app_id !== undefined) {
                    this.#appID = msg.window_props.app_id;
                }

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
                console.warn(`${this} Unknown event type ${msg.event}`);
                break;
        }
    };
}

// TODO
class DashButton {
    /**
     * IPC identifier
     * @readonly
     * @type {Uuid}
     */
    id = Uuid.generate();

    dispose() {
    }
}

module.exports = {
    DashWindow,
    DashButton,
};
