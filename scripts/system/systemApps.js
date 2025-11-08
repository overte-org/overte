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

const SystemTablet = Tablet.getTablet("com.highfidelity.interface.tablet.system");

function defaultOnClicked() {
    this.appButtonData.isActive = !this.appButtonData.isActive;
    this.appButton.editProperties({ isActive: this.appButtonData.isActive });

    if (this.appButtonData.isActive) {
        SystemTablet.loadQMLSource(this.qmlSource, false);
    } else {
        SystemTablet.gotoHomeScreen();
    }
}

function defaultOnScreenChanged(type, url) {
    if (type != "QML" || url != this.qmlSource) {
        this.appButtonData.isActive = false;
        this.appButton.editProperties({ isActive: this.appButtonData.isActive });
    }
}

// has absolutely nothing to do with HTTP cookies,
// just here to keep track of what requests are being handled
let waitingRequestCookies = new Set();

function defaultFromQml(message) {
    const data = JSON.parse(message);

    switch (data.action) {
        // QML's XMLHttpRequest doesn't have the authentication header that's
        // needed for interacting with the directory server. This passes a
        // request through the JS XMLHttpRequest so it can use the account auth.
        case "system:auth_request": {
            // sometimes requests get sent twice? don't let that happen
            if (waitingRequestCookies.has(data.data.cookie)) {
                return;
            } else {
                waitingRequestCookies.add(data.data.cookie);
            }

            let xhr = new XMLHttpRequest();

            xhr.onreadystatechange = () => {
                if (xhr.readyState === 4 /* DONE */) {
                    SystemTablet.sendToQml(JSON.stringify({
                        action: "system:auth_request",
                        data: {
                            status: xhr.status,
                            statusText: xhr.statusText,
                            responseText: xhr.responseText,
                            responseURL: data.data.url,
                            cookie: data.data.cookie,
                        },
                    }));

                    waitingRequestCookies.delete(data.data.cookie);
                }
            };

            xhr.open(data.data.method, data.data.url);

            if (data.data.method === "POST") {
                xhr.setRequestHeader("Content-Type", "application/json;charset=utf-8");
            }

            xhr.send(data.data.body);
        } break;

        case "system:location_go_back": location.goBack(); break;
        case "system:location_go_forward": location.goForward(); break;
        case "system:location_go_to": {
            location.handleLookupString(data.data.path);

            // hide the tablet after travelling
            SystemTablet.gotoHomeScreen();
            SystemTablet.tabletShown = false;
        } break;
    }
}

const SYSTEM_APPS = {
    settings: {
        appButtonData: {
            sortOrder: 3,
            isActive: false,
            icon: Script.resolvePath("./settings/img/icon_white.png"),
            activeIcon: Script.resolvePath("./settings/img/icon_black.png"),

            // TODO: translation support in JS
            text: "SETTINGS",
        },

        qmlSource: "overte/settings/Settings.qml",
        appButton: null,
    },

    avatar: {
        appButtonData: {
            sortOrder: 4,
            isActive: false,
            icon: "icons/tablet-icons/avatar-i.svg",
            activeIcon: "icons/tablet-icons/avatar-a.svg",

            // TODO: translation support in JS
            text: "AVATAR",
        },

        qmlSource: "overte/avatar_picker/AvatarPicker.qml",
        appButton: null,
    },

    contacts: {
        appButtonData: {
            sortOrder: 5,
            isActive: false,
            icon: "icons/tablet-icons/people-i.svg",
            activeIcon: "icons/tablet-icons/people-a.svg",

            // TODO: translation support in JS
            text: "CONTACTS",
        },

        qmlSource: "overte/contacts/ContactsList.qml",
        appButton: null,
    },

    // TODO: to be picked up again in a later PR
    /*places: {
        appButtonData: {
            sortOrder: 6,
            isActive: false,
            // TODO: put these somewhere more global
            icon: Script.resolvePath("./places/icons/appicon_i.png"),
            activeIcon: Script.resolvePath("./places/icons/appicon_a.png"),

            // TODO: translation support in JS
            text: "PLACES",
        },

        qmlSource: "overte/place_picker/PlacePicker.qml",
        appButton: null,
    },*/

    more: {
        appButtonData: {
            isActive: false,
            // TODO: put these somewhere more global
            icon: Script.resolvePath("./more/appicon_i.png"),
            activeIcon: Script.resolvePath("./more/appicon_a.png"),

            // TODO: translation support in JS
            text: "MORE",
        },

        qmlSource: "overte/more_apps/MoreApps.qml",
        appButton: null,
    },
};

function setupButtons() {
    for (let app of Object.values(SYSTEM_APPS)) {
        if (!app.onClicked) { app.onClicked = defaultOnClicked; }
        if (!app.onScreenChanged) { app.onScreenChanged = defaultOnScreenChanged; }
        if (!app.fromQml) { app.fromQml = defaultFromQml; }

        let button = SystemTablet.addButton(app.appButtonData);
        button.clicked.connect(() => app.onClicked());
        SystemTablet.screenChanged.connect((type, url) => app.onScreenChanged(type, url));
        SystemTablet.fromQml.connect(message => app.fromQml(message));
        app.appButton = button;
    }

    Script.scriptEnding.connect(() => {
        for (const app of Object.values(SYSTEM_APPS)) {
            if (app.appButton) {
                SystemTablet.removeButton(app.appButton);
            }
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
