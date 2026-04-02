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
        this.window.eventReceived.connect(msg => this.fromQml(msg));
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
                    this.window?.sendEvent(JSON.stringify({
                        action: "system:auth_request",
                        data: {
                            status: xhr.status,
                            statusText: xhr.statusText,
                            responseText: xhr.responseText,
                            responseURL: data.url,
                            cookie: data.cookie,
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
            Window.displayAnnouncement(`Going to ${data.name}`);
        } break;
    }
}

const DEVTOOLS_BUTTON_PROPS = {
    text: "Dev Tools",
    system: true,
    icons: `${Script.resourcesPath()}qml/overte/icons/dev_tools.png`,
    order: -4,
};

const DEVTOOLS_VISIBLE = Settings.getValue("Settings/Developer Menu", false);

const SYSTEM_APPS = {
    dev_tools: {
        // might be set up later if the user changes the setting
        button: DEVTOOLS_VISIBLE ? new DashButton(DEVTOOLS_BUTTON_PROPS) : null,
        visible: DEVTOOLS_VISIBLE,

        window: null,
        windowProps: {
            title: "Developer Tools",
            sourceURL: `${Script.resourcesPath()}qml/overte/dash/DevTools.qml`,
        },
    },

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
            icons: `${Script.resourcesPath()}qml/overte/icons/contacts.png`,
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
            icons: `${Script.resourcesPath()}qml/overte/icons/places.png`,
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
            // TODO: new icon
            icons: {
                dark: {
                    idle: Script.resolvePath("../more/appicon_i.png"),
                    active: Script.resolvePath("../more/appicon_i.png"),
                },
                light: {
                    idle: Script.resolvePath("../more/appicon_a.png"),
                    active: Script.resolvePath("../more/appicon_i.png"),
                },
            },
            order: 1000,
        }),

        window: null,
        windowProps: {
            title: "More Apps",
            sourceURL: `${Script.resourcesPath()}qml/overte/more_apps/MoreApps.qml`,
        },
    },
};

let tmpWindows = new Set();

function setupButtons() {
    for (let app of Object.values(SYSTEM_APPS)) {
        if (!app.onClicked) { app.onClicked = defaultOnClicked; }
        if (!app.fromQml) { app.fromQml = defaultFromQml; }
        app.button?.clicked.connect(() => app.onClicked());
    }

    Script.scriptEnding.connect(() => {
        for (const app of Object.values(SYSTEM_APPS)) {
            app.window?.close();
            app.button?.dispose();
        }

        for (const window of tmpWindows.values()) {
            window.close();
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

Messages.messageReceived.connect((channel, rawMsg, _senderID, localOnly) => {
    if (channel !== "Dash DevTools" || !localOnly) { return; }

    const msg = JSON.parse(rawMsg);

    if (msg.button_visible !== undefined) {
        const show = msg.button_visible;

        if (show && !SYSTEM_APPS.dev_tools.visible) {
            let button = new DashButton(DEVTOOLS_BUTTON_PROPS);
            button.clicked.connect(() => SYSTEM_APPS.dev_tools.onClicked());

            SYSTEM_APPS.dev_tools.button = button;
            SYSTEM_APPS.dev_tools.visible = true;
        } else if (!show && SYSTEM_APPS.dev_tools.visible) {
            SYSTEM_APPS.dev_tools.button?.dispose();
            SYSTEM_APPS.dev_tools.visible = false;
        }
    }

    switch (msg.open_window) {
        case "running scripts": {
            let window = new DashWindow({
                title: "Running Scripts",
                sourceURL: `${Script.resourcesPath()}qml/overte/dialogs/RunningScriptsDialog.qml`,
            });
            window.closed.connect(() => tmpWindows.delete(window));
            tmpWindows.add(window);
        } break;

        case "asset server": {
            let window = new DashWindow({
                title: "Server Assets",
                sourceURL: `${Script.resourcesPath()}qml/overte/dialogs/AssetDialog.qml`,
            });
            window.closed.connect(() => tmpWindows.delete(window));
            tmpWindows.add(window);
        } break;
    }
});

loadInstalledMoreApps();
setupButtons();
