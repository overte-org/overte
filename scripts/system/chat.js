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

const { DashWindow, DashButton } = require("dashIPC");

const CHAT_QML_URL = `${Script.resourcesPath()}qml/overte/chat/Chat.qml`;

let settings = Settings.getValue("Chat", {
    joinNotifications: true,
    broadcastEnabled: false,
    chatBubbles: true,
    desktopWindow: false,
});

const EVENT_LOG_SETTING = "chat/eventsLog";
let eventLog = Settings.getValue(EVENT_LOG_SETTING, []);

function sendLoggedEventToQml(data) {
    // only send it the event if the window exists
    if (settings.desktopWindow) {
        desktopWindow?.sendToQml(JSON.stringify(data));
    } else {
        dashWindow?.sendEvent(JSON.stringify(data));
    }

    eventLog.push(data);
    Settings.setValue(EVENT_LOG_SETTING, eventLog);
}

const appButton = new DashButton({
    // TODO: translation support in JS
    text: "Chat",
    system: true,
    icons: `${Script.resourcesPath()}qml/overte/icons/chat.png`,
    order: 0,
});

let dashWindow = null;
let desktopWindow = null;

appButton.clicked.connect(() => {
    if (appButton.active) {
        dashWindow?.close();
        desktopWindow?.close();
        appButton.active = false;
    } else {
        recreateWindow();
        appButton.active = true;
    }
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
            if (settings.desktopWindow != value) {
                if (settings.desktopWindow) {
                    desktopWindow?.close();
                } else {
                    dashWindow?.close();
                }
                settings.desktopWindow = value;
                recreateWindow();
                appButton.active = true;
            }
            break;
    }

    Settings.setValue("Chat", settings);
}

function sendInitialSettings() {
    let send;

    if (settings.desktopWindow) {
        send = (name, value) => desktopWindow.sendToQml(JSON.stringify({
            event: "change_setting",
            name: name,
            value: value,
        }));
    } else {
        send = (name, value) => dashWindow.sendEvent(JSON.stringify({
            event: "change_setting",
            name: name,
            value: value,
        }));
    }

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

        case "clear_history":
            eventLog = [];
            Settings.setValue(EVENT_LOG_SETTING, eventLog);
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

function recreateWindow() {
    if (settings.desktopWindow) {
        desktopWindow = Desktop.createWindow(
            CHAT_QML_URL,
            {
                // TODO: translation support in JS
                title: "Chat",
                size: { x: 550, y: 400 },
                presentationMode: Desktop.PresentationMode.NATIVE,
                additionalFlags: Desktop.ALWAYS_ON_TOP,
            }
        );
        desktopWindow.fromQml.connect(appWindowFromQml);
        desktopWindow.closed.connect(() => { appButton.active = false; });
        sendInitialSettings();
    } else {
        dashWindow = new DashWindow({
            title: "Chat",
            sourceURL: CHAT_QML_URL,
            dimensions: { x: 0.45, y: 0.45, z: 0 },
        });

        dashWindow.eventReceived.connect(appWindowFromQml);
        dashWindow.closed.connect(() => { appButton.active = false; });
        sendInitialSettings();
    }
}

Messages.subscribe("chat");
Messages.subscribe("Chat-Typing");

Messages.messageReceived.connect((channel, rawMsg, senderID, _localOnly) => {
    if (channel === "chat") {
        const msg = JSON.parse(rawMsg);
        sendLoggedEventToQml({
            event: "recv_message",
            name: msg.displayName,
            body: msg.message,
            timestamp: msg.timestamp,
        });
    } else if (channel === "Chat-Typing") {
        const avatar = AvatarManager.getAvatar(senderID);
        const msg = JSON.parse(rawMsg);
        const event = {
            event: msg.action === "typing_start" ? "start_typing" : "end_typing",
            name: avatar.sessionDisplayName ? avatar.sessionDisplayName : avatar.displayName,
            uuid: senderID,
        };

        if (settings.desktopWindow) {
            desktopWindow?.sendToQml(JSON.stringify(event));
        } else {
            dashWindow?.sendEvent(JSON.stringify(event));
        }
    }
});

let palData = [];

AvatarManager.avatarAddedEvent.connect(uuid => {
    // pal data isn't immediately available when an avatar joins,
    // so we need a bit of a delay to wait for it to come through
    Script.setTimeout(() => {
        palData = AvatarManager.getPalData().data;

        const name = palData.find(e => e.sessionUUID === uuid)?.sessionDisplayName;

        // still couldn't get anything to show, don't bother
        if (!name) { return; }

        sendLoggedEventToQml({
            event: "user_joined",
            name: name,
            timestamp: Date.now(),
        });
    }, 1500);
});

AvatarManager.avatarRemovedEvent.connect(uuid => {
    const name = palData.find(e => e.sessionUUID === uuid)?.sessionDisplayName;

    sendLoggedEventToQml({
        event: "user_left",
        name: name,
        timestamp: Date.now(),
    });
});

Script.scriptEnding.connect(() => {
    appButton.dispose();
});
