// userSignal.js
// Created by Ada <ada@thingvellir.net> on 2026-02-26
// Copyright 2026 Overte e.V.
// SPDX-License-Identifier: Apache-2.0
"use strict";

/**
 * Allows registering and unregistering callback functions
 * to call when {@link UserSignal.emit} is called.
 *
 * An analogue to the {@link Signal} type exposed
 * by the C++ parts of the engine.
 */
class UserSignal {
    /**
     * @callback signalCallback
     * @param {...*} args
     * @returns {void}
     */

    /** @type {Array<signalCallback>} */
    #connections = [];

    /**
     * Adds a callback to run when this signal is emitted.
     * @param {signalCallback} func - Callback function
     * @returns {signalCallback} Returns the `func` parameter,
     * which can be passed later to {@link UserSignal.disconnect}.
     */
    connect(func) {
        this.#connections.push(func);
        return func;
    }

    /**
     * Removes a callback so it won't be run when this signal is emitted.
     * @param {signalCallback} func - The callback function to disconnect
     * @returns {void}
     */
    disconnect(func) {
        const index = this.#connections.indexOf(func);

        if (index !== -1) {
            this.#connections.splice(index, 1);
        }
    }

    /**
     * Calls all registered callbacks.
     * @param {...*} args - The arguments to pass to the callbacks.
     * @returns {void}
     */
    emit(...args) {
        for (const func of this.#connections) {
            func(...args);
        }
    }
}

module.exports = { UserSignal };
