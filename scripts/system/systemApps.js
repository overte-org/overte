//
//  systemApps.js
//
//  Created by Ada <ada@thingvellir.net> on 2025-10-26
//  Copyright 2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
//  SPDX-License-Identifier: Apache-2.0
"use strict";

const { Dashboard, DashButton, DashWindow } = require("dashIPC");

function defaultOnClicked() {
    this.button.active = !this.button.active;

    if (this.button.active) {
        this.window = new DashWindow(this.windowProps);
        this.window.eventReceived.connect(this.fromQml);
        this.window.closed.connect(() => { this.button.active = false; });
    } else {
        this.window.close();
    }
}

// has absolutely nothing to do with HTTP cookies,
// just here to keep track of what requests are being handled
let waitingRequestCookies = new Set();

function defaultFromQml(rawMsg) {
    const { action, data } = JSON.parse(rawMsg);

    switch (action) {
        // QML's XMLHttpRequest doesn't have the authentication header that's
        // needed for interacting with the directory server. This passes a
        // request through the JS XMLHttpRequest so it can use the account auth.
        case "system:auth_request": {
            // sometimes requests get sent twice? don't let that happen
            if (waitingRequestCookies.has(data.cookie)) { return; }

            waitingRequestCookies.add(data.cookie);

            let xhr = new XMLHttpRequest();

            xhr.onreadystatechange = () => {
                if (xhr.readyState === 4 /* DONE */) {
                    this.window.sendEvent(JSON.stringify({
                        action: "system:auth_request",
                        data: {
                            status: xhr.status,
                            statusText: xhr.statusText,
                            responseText: xhr.responseText,
                            responseURL: data.data.url,
                            cookie: data.data.cookie,
                        },
                    }));

                    waitingRequestCookies.delete(data.cookie);
                }
            };

            xhr.open(data.method, data.url);

            if (data.method === "POST") {
                xhr.setRequestHeader("Content-Type", "application/json;charset=utf-8");
            }

            xhr.send(data.body);
        } break;

        case "system:location_go_back": location.goBack(); break;
        case "system:location_go_forward": location.goForward(); break;
        case "system:location_go_to": {
            SYSTEM_APPS.places.window?.close();
            Dashboard.visible = false;
            location.handleLookupString(data.path);
        } break;
    }
}

const SYSTEM_APPS = {
    avatar: {
        button: new DashButton({
            // TODO: translation support in JS
            text: "Avatar",
            system: true,
            icons: `${Script.resourcesPath()}qml/overte/icons/avatars.png`,
            order: -3,
        }),

        window: null,
        windowProps: {
            title: "Avatar",
            sourceURL: `${Script.resourcesPath()}qml/overte/avatar_picker/AvatarPicker.qml`,
        },
    },

    contacts: {
        button: new DashButton({
            // TODO: translation support in JS
            text: "Contacts",
            system: true,
            // TODO: better icon
            icons: `${Script.resourcesPath()}qml/overte/icons/users.svg`,
            order: -2,
        }),

        window: null,
        windowProps: {
            title: "Contacts",
            sourceURL: `${Script.resourcesPath()}qml/overte/contacts/ContactsList.qml`,
        },
    },

    places: {
        button: new DashButton({
            // TODO: translation support in JS
            text: "Places",
            system: true,
            // TODO: better icon
            icons: `${Script.resourcesPath()}qml/overte/icons/home.svg`,
            order: -1,
        }),

        window: null,
        windowProps: {
            title: "Places",
            sourceURL: `${Script.resourcesPath()}qml/overte/place_picker/PlacePicker.qml`,
        },
    },

    more: {
        button: new DashButton({
            // TODO: translation support in JS
            text: "More Apps",
            // the more app lives in the user app drawer
            system: false,
            // TODO: better icon
            icons: `${Script.resourcesPath()}qml/overte/icons/delete.svg`,
            order: 1000,
        }),

        window: null,
        windowProps: {
            title: "More Apps",
            sourceURL: `${Script.resourcesPath()}qml/overte/more_apps/MoreApps.qml`,
        },
    },
};

function setupButtons() {
    for (let app of Object.values(SYSTEM_APPS)) {
        if (!app.onClicked) { app.onClicked = defaultOnClicked; }
        if (!app.fromQml) { app.fromQml = defaultFromQml; }
        app.button.clicked.connect(() => app.onClicked());
    }

    Script.scriptEnding.connect(() => {
        for (const app of Object.values(SYSTEM_APPS)) {
            app.window?.close();
            app.button.dispose();
        }
    });
}

function loadInstalledMoreApps() {
    const RETRY_DELAY_SECS = 5;
    const installedScripts = Settings.getValue("moreApp/installedScripts", []);

    for (const scriptUrl of installedScripts) {
        ScriptDiscoveryService.loadScript(scriptUrl, false);
    }

    // Check that all of the installed scripts are running.
    // If there's some that aren't, then give them one more try.
    Script.setTimeout(() => {
        const running = ScriptDiscoveryService.getRunning();

        for (const scriptUrl of installedScripts) {
            if (!running.includes(scriptUrl)) {
                ScriptDiscoveryService.loadScript(scriptUrl, false);
            }
        }
    }, RETRY_DELAY_SECS * 1000);
}

loadInstalledMoreApps();
setupButtons();
