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
};

for (let app of Object.values(SYSTEM_APPS)) {
    if (!app.onClicked) { app.onClicked = defaultOnClicked; }
    if (!app.onScreenChanged) { app.onScreenChanged = defaultOnScreenChanged; }

    let button = SystemTablet.addButton(app.appButtonData);
    button.clicked.connect(() => app.onClicked());
    SystemTablet.screenChanged.connect((type, url) => app.onScreenChanged(type, url));
    app.appButton = button;
}

Script.scriptEnding.connect(() => {
    for (const app of Object.values(SYSTEM_APPS)) {
        if (app.appButton) {
            SystemTablet.removeButton(app.appButton);
        }
    }
});
