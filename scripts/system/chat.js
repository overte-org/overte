//
//  chat.js
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

let settings = Settings.getValue("Chat", {
    joinNotifications: true,
    broadcastEnabled: false,
    chatBubbles: true,
    desktopWindow: false,
});

function updateSetting(name, value) {
    switch (name) {
        case "join_notify":
            settings.joinNotifications = value;
            break;

        case "broadcast":
            settings.broadcastEnabled = value;
            break;

        case "chat_bubbles":
            settings.chatBubbles = value;
            Messages.sendLocalMessage("ChatBubbles-Enabled", JSON.stringify(value));
            break;

        case "desktop_window":
            settings.desktopWindow = value;
            // FIXME: this sometimes leaves an empty ghost window on the overlay?
            appWindow.presentationMode = value ?
                Desktop.PresentationMode.NATIVE :
                Desktop.PresentationMode.VIRTUAL;
            break;
    }

    Settings.setValue("Chat", settings);
}

function sendInitialSettings() {
    const send = (name, value) => appWindow.sendToQml(JSON.stringify({
        event: "change_setting",
        name: name,
        value: value,
    }));

    send("join_notify", settings.joinNotifications);
    send("broadcast", settings.broadcastEnabled);
    send("chat_bubbles", settings.chatBubbles);
    send("desktop_window", settings.desktopWindow);
}

function appWindowFromQml(rawMsg) {
    const msg = JSON.parse(rawMsg);

    switch (msg.event) {
        case "change_setting":
            updateSetting(msg.setting, msg.value);
            break;

        case "send_message":
            Messages.sendMessage("chat", JSON.stringify({
                action: "send_chat_message",
                position: MyAvatar.position,
                displayName: MyAvatar.sessionDisplayName ? MyAvatar.sessionDisplayName : MyAvatar.displayName,
                message: msg.body,
                channel: settings.broadcastEnabled ? "domain" : "local",
                timestamp: Date.now(),
            }));
            break;

        case "start_typing":
            Messages.sendMessage("Chat-Typing", JSON.stringify({
                action: "typing_start",
                position: MyAvatar.position,
            }));
            break;

        case "end_typing":
            Messages.sendMessage("Chat-Typing", JSON.stringify({
                action: "typing_stop"
            }));
            break;
    }
}

function recreateAppWindow() {
    appWindow = Desktop.createWindow(
        `${Script.resourcesPath()}qml/overte/chat/Chat.qml`,
        {
            // TODO: translation support in JS
            title: "Chat",
            size: { x: 550, y: 400 },
            visible: appButton.buttonData.isActive,
            presentationMode: settings.desktopWindow ?
                Desktop.PresentationMode.NATIVE :
                Desktop.PresentationMode.VIRTUAL,
            additionalFlags: Desktop.ALWAYS_ON_TOP | Desktop.CLOSE_BUTTON_HIDES,
        }
    );

    // https://github.com/overte-org/overte/issues/824
    appWindow.visible = appButton.buttonData.isActive;

    // FIXME: CLOSE_BUTTON_HIDES doesn't work with desktop windows
    appWindow.closed.connect(() => {
        appButton.buttonData.isActive = false;
        appButton.button.editProperties({ isActive: false });
        appWindow = undefined;
    });

    appWindow.fromQml.connect(appWindowFromQml);

    sendInitialSettings();
}

const appButton = {
    buttonData: {
        isActive: false,
        sortOrder: 6,
        icon: "icons/tablet-icons/chat-i.svg",
        activeIcon: "icons/tablet-icons/chat-a.svg",

        // TODO: translation support in JS
        text: "CHAT",
    },

    button: null,

    onClicked() {
        this.buttonData.isActive = !this.buttonData.isActive;
        this.button.editProperties({ isActive: this.buttonData.isActive });

        if (!appWindow) { recreateAppWindow(); }

        appWindow.visible = this.buttonData.isActive;
    },
};

let appWindow;

// postpone the window creation until there's a chat event,
// if it's opened too quickly then it can spawn visible
// rather than hidden
//recreateAppWindow();

appButton.button = SystemTablet.addButton(appButton.buttonData);
appButton.button.clicked.connect(() => appButton.onClicked());

Messages.subscribe("chat");
Messages.subscribe("Chat-Typing");

Messages.messageReceived.connect((channel, rawMsg, senderID, _localOnly) => {
    if (channel === "chat") {
        if (!appWindow) { recreateAppWindow(); }

        const msg = JSON.parse(rawMsg);
        appWindow.sendToQml(JSON.stringify({
            event: "recv_message",
            name: msg.displayName,
            body: msg.message,
            timestamp: msg.timestamp,
        }));
    } else if (channel === "Chat-Typing") {
        if (!appWindow) { recreateAppWindow(); }

        const avatar = AvatarManager.getAvatar(senderID);
        const msg = JSON.parse(rawMsg);
        appWindow.sendToQml(JSON.stringify({
            event: msg.action === "typing_start" ? "start_typing" : "end_typing",
            name: avatar.sessionDisplayName ? avatar.sessionDisplayName : avatar.displayName,
            uuid: senderID,
        }));
    }
});

let palData = [];

AvatarManager.avatarAddedEvent.connect(uuid => {
    if (!appWindow) { recreateAppWindow(); }

    // pal data isn't immediately available when an avatar joins,
    // so we need a bit of a delay to wait for it to come through
    Script.setTimeout(() => {
        palData = AvatarManager.getPalData().data;

        const name = palData.find(e => e.sessionUUID === uuid)?.sessionDisplayName;

        // still couldn't get anything to show, don't bother
        if (!name) { return; }

        appWindow.sendToQml(JSON.stringify({
            event: "user_joined",
            name: name,
            timestamp: Date.now(),
        }));
    }, 1500);
});

AvatarManager.avatarRemovedEvent.connect(uuid => {
    if (!appWindow) { recreateAppWindow(); }

    const name = palData.find(e => e.sessionUUID === uuid)?.sessionDisplayName;

    appWindow.sendToQml(JSON.stringify({
        event: "user_left",
        name: name,
        timestamp: Date.now(),
    }));
});

Script.scriptEnding.connect(() => {
    SystemTablet.removeButton(appButton.button);
});
