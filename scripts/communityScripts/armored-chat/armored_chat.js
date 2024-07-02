//
//  armored_chat.js
//
//  Created by Armored Dragon, 2024.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html

(() => {
    "use strict";

    var appIsVisible = false;
    var settings = {
        external_window: false,
        maximum_messages: 200,
    };

    // Global vars
    var tablet;
    var chatOverlayWindow;
    var appButton;
    var quickMessage;
    const channels = ["domain", "local"];
    var messageHistory = Settings.getValue("ArmoredChat-Messages", []) || [];
    var maxLocalDistance = 20; // Maximum range for the local chat
    var palData = AvatarManager.getPalData().data;

    Controller.keyPressEvent.connect(keyPressEvent);
    Messages.subscribe("chat");
    Messages.messageReceived.connect(receivedMessage);
    AvatarManager.avatarAddedEvent.connect((sessionId) => {
        _avatarAction("connected", sessionId);
    });
    AvatarManager.avatarRemovedEvent.connect((sessionId) => {
        _avatarAction("left", sessionId);
    });

    startup();

    function startup() {
        tablet = Tablet.getTablet("com.highfidelity.interface.tablet.system");

        appButton = tablet.addButton({
            icon: Script.resolvePath("./img/icon_white.png"),
            activeIcon: Script.resolvePath("./img/icon_black.png"),
            text: "CHAT",
            isActive: appIsVisible,
        });

        // When script ends, remove itself from tablet
        Script.scriptEnding.connect(function () {
            console.log("Shutting Down");
            tablet.removeButton(appButton);
            chatOverlayWindow.close();
        });

        // Overlay button toggle
        appButton.clicked.connect(toggleMainChatWindow);

        quickMessage = new OverlayWindow({
            source: Script.resolvePath("./armored_chat_quick_message.qml"),
        });

        _openWindow();
    }
    function toggleMainChatWindow() {
        appIsVisible = !appIsVisible;
        appButton.editProperties({ isActive: appIsVisible });
        chatOverlayWindow.visible = appIsVisible;

        // External window was closed; the window does not exist anymore
        if (chatOverlayWindow.title == "" && appIsVisible) {
            _openWindow();
        }
    }
    function _openWindow() {
        chatOverlayWindow = new Desktop.createWindow(
            Script.resolvePath("./armored_chat.qml"),
            {
                title: "Chat",
                size: { x: 550, y: 400 },
                additionalFlags: Desktop.ALWAYS_ON_TOP,
                visible: appIsVisible,
                presentationMode: Desktop.PresentationMode.VIRTUAL,
            }
        );

        chatOverlayWindow.closed.connect(toggleMainChatWindow);
        chatOverlayWindow.fromQml.connect(fromQML);
        quickMessage.fromQml.connect(fromQML);
    }
    function receivedMessage(channel, message) {
        // Is the message a chat message?
        channel = channel.toLowerCase();
        if (channel !== "chat") return;
        message = JSON.parse(message);

        // Floofchat compatibility hook
        message = floofChatCompatibilityConversion(message);

        message.channel = message.channel.toLowerCase(); // Make sure the "local", "domain", etc. is formatted consistently

        if (!channels.includes(message.channel)) return; // Check the channel
        if (
            message.channel == "local" &&
            Vec3.distance(MyAvatar.position, message.position) >
                maxLocalDistance
        )
            return; // If message is local, and if player is too far away from location, don't do anything

        // Update qml view of to new message
        _emitEvent({ type: "show_message", ...message });

        Messages.sendLocalMessage(
            "Floof-Notif",
            JSON.stringify({
                sender: message.displayName,
                text: message.message,
            })
        );

        // Save message to history
        let savedMessage = message;
        delete savedMessage.position;
        savedMessage.timeString = new Date().toLocaleTimeString(undefined, {
            hour12: false,
        });
        savedMessage.dateString = new Date().toLocaleDateString(undefined, {
            year: "numeric",
            month: "long",
            day: "numeric",
        });
        messageHistory.push(savedMessage);
        while (messageHistory.length > settings.maximum_messages) {
            messageHistory.shift();
        }
        Settings.setValue("ArmoredChat-Messages", messageHistory);
    }
    function fromQML(event) {
        switch (event.type) {
            case "send_message":
                _sendMessage(event.message, event.channel);
                break;
            case "setting_change":
                settings[event.setting] = event.value; // Update local settings
                _saveSettings(); // Save local settings

                switch (event.setting) {
                    case "external_window":
                        chatOverlayWindow.presentationMode = event.value
                            ? Desktop.PresentationMode.NATIVE
                            : Desktop.PresentationMode.VIRTUAL;
                        break;
                    case "maximum_messages":
                        // Do nothing
                        break;
                }

                break;
            case "action":
                switch (event.action) {
                    case "erase_history":
                        Settings.setValue("ArmoredChat-Messages", []);
                        _emitEvent({
                            type: "clear_messages",
                        });
                        break;
                }
                break;
            case "initialized":
                // https://github.com/overte-org/overte/issues/824
                chatOverlayWindow.visible = appIsVisible; // The "visible" field in the Desktop.createWindow does not seem to work. Force set it to the initial state (false)
                _loadSettings();
                break;
        }
    }
    function keyPressEvent(event) {
        switch (JSON.stringify(event.key)) {
            case "16777220": // Enter key
                if (HMD.active) return; // Don't allow in VR

                quickMessage.sendToQml({
                    type: "change_visibility",
                    value: true,
                });
        }
    }
    function _sendMessage(message, channel) {
        if (message.length == 0) return;

        Messages.sendMessage(
            "chat",
            JSON.stringify({
                position: MyAvatar.position,
                message: message,
                displayName: MyAvatar.sessionDisplayName,
                channel: channel,
                action: "send_chat_message",
            })
        );

        floofChatCompatibilitySendMessage(message, channel);
    }
    function _avatarAction(type, sessionId) {
        Script.setTimeout(() => {
            if (type == "connected") {
                palData = AvatarManager.getPalData().data;
            }

            // Get the display name of the user
            let displayName = "";
            displayName =
                AvatarManager.getPalData([sessionId])?.data[0]
                    ?.sessionDisplayName || null;
            if (displayName == null) {
                for (let i = 0; i < palData.length; i++) {
                    if (palData[i].sessionUUID == sessionId) {
                        displayName = palData[i].sessionDisplayName;
                    }
                }
            }

            // Format the packet
            let message = {};
            message.message = `${displayName} ${type}`;

            _emitEvent({ type: "notification", ...message });
        }, 1500);
    }
    function _loadSettings() {
        settings = Settings.getValue("ArmoredChat-Config", settings);

        if (messageHistory) {
            // Load message history
            messageHistory.forEach((message) => {
                delete message.action;
                _emitEvent({ type: "show_message", ...message });
            });
        }

        // Send current settings to the app
        _emitEvent({ type: "initial_settings", settings: settings });
    }
    function _saveSettings() {
        console.log("Saving config");
        Settings.setValue("ArmoredChat-Config", settings);
    }

    /**
     * Emit a packet to the HTML front end. Easy communication!
     * @param {Object} packet - The Object packet to emit to the HTML
     * @param {("show_message"|"clear_messages"|"notification"|"initial_settings")} packet.type - The type of packet it is
     */
    function _emitEvent(packet = { type: "" }) {
        chatOverlayWindow.sendToQml(packet);
    }

    //
    // Floofchat compatibility functions
    // Added to ease the transition between Floofchat to ArmoredChat
    // These functions can be safely removed at a much later date.
    function floofChatCompatibilityConversion(message) {
        if (message.type === "TransmitChatMessage" && !message.forApp) {
            return {
                position: message.position,
                message: message.message,
                displayName: message.displayName,
                channel: message.channel.toLowerCase(),
            };
        }
        return message;
    }

    function floofChatCompatibilitySendMessage(message, channel) {
        Messages.sendMessage(
            "Chat",
            JSON.stringify({
                position: MyAvatar.position,
                message: message,
                displayName: MyAvatar.sessionDisplayName,
                channel: channel.charAt(0).toUpperCase() + channel.slice(1),
                type: "TransmitChatMessage",
                forApp: "Floof",
            })
        );
    }
})();
