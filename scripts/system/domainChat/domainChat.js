//
//  domainChat.js
//
//  Created by Armored Dragon, May 17th, 2024.
//  Copyright 2024-2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html

(() => {
    ("use strict");

    var appIsVisible = false;
    var settings = {
        externalWindow: false,
        maximumMessages: 200,
        joinNotification: true,
        useChatBubbles: true,
    };

    // Global vars
    var tablet;
    var chatOverlayWindow;
    var appButton;
    var quickMessage;
    const CHANNELS = ["domain", "local"];
    var messageHistory = Settings.getValue("DomainChat-Messages", []) || [];
    var maxLocalDistance = 20; // Maximum range for the local chat
    var palData = AvatarManager.getPalData().data;
    var isTyping = false;

    Controller.keyPressEvent.connect(keyPressEvent);
    Messages.subscribe("Chat"); // Floofchat
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
            sortOrder: 8,
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
            source: Script.resolvePath("./domainChatQuick.qml"),
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
            Script.resolvePath("./domainChat.qml"),
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

        if (message.action !== "sendChatMessage") return;

        // Get the message data
        const CURRENT_TIMESTAMP = _getTimestamp();
        const TIME_ARRAY = _formatTimestamp(CURRENT_TIMESTAMP);

        if (!message.channel) message.channel = "domain"; // We don't know where to put this message. Assume it is a domain wide message.
        if (message.forApp) return; // Floofchat

        // Floofchat compatibility hook
        message = floofChatCompatibilityConversion(message);
        message.channel = message.channel.toLowerCase();

        // Check the channel. If the channel is not one we have, do nothing.
        if (!CHANNELS.includes(message.channel)) return;

        // If message is local, and if player is too far away from location, do nothing.
        if (message.channel == "local" && isTooFar(message.position)) return;

        // Format the timestamp 
        message.timeString = TIME_ARRAY[0];
        message.dateString = TIME_ARRAY[1];

        // Update qml view of to new message
        _emitEvent({ type: "showMessage", ...message });

        // Show new message on screen
        if (message.channel !== "local" || !settings.useChatBubbles) {
            Messages.sendLocalMessage(
                "Floof-Notif",
                JSON.stringify({
                    sender: message.displayName,
                    text: message.message,
                })
            );
        }

        // Save message to history
        let savedMessage = message;

        // Remove unnecessary data.
        delete savedMessage.position;
        delete savedMessage.timeString;
        delete savedMessage.dateString;
        delete savedMessage.action;

        savedMessage.timestamp = CURRENT_TIMESTAMP;

        messageHistory.push(savedMessage);
        while (messageHistory.length > settings.maximumMessages) {
            messageHistory.shift();
        }
        Settings.setValue("DomainChat-Messages", messageHistory);

        // Check to see if the message is close enough to the user
        function isTooFar(messagePosition) {
            return Vec3.distance(MyAvatar.position, messagePosition) > maxLocalDistance;
        }
    }
    function fromQML(event) {
        switch (event.type) {
            case "sendMessage":
                _sendMessage(event.message, event.channel);
                break;
            case "settingChange":

                // Set the setting value, and save the config
                settings[event.setting] = event.value; // Update local settings
                _saveSettings(); // Save local settings

                // Extra actions to preform. 
                switch (event.setting) {
                    case "externalWindow":
                        chatOverlayWindow.presentationMode = event.value
                            ? Desktop.PresentationMode.NATIVE
                            : Desktop.PresentationMode.VIRTUAL;
                        break;
                    case "useChatBubbles":
                        Messages.sendLocalMessage(
                            "ChatBubbles-Config",
                            JSON.stringify({
                                enabled: event.value,
                            })
                        );
                        break;
                }

                break;
            case "action":
                switch (event.action) {
                    case "eraseHistory":
                        Settings.setValue("DomainChat-Messages", null);
                        messageHistory = [];
                        _emitEvent({
                            type: "clearMessages",
                        });
                        break;
                    case "start_typing":
                        if (!isTyping) {
                            Messages.sendMessage(
                                "Chat-Typing",
                                JSON.stringify({
                                    action: "typing_start",
                                    position: MyAvatar.position,
                                })
                            );
                        }
                        isTyping = true;
                        break;
                    case "end_typing":
                        if (isTyping) {
                            Messages.sendMessage(
                                "Chat-Typing",
                                JSON.stringify({
                                    action: "typing_stop"
                                })
                            );
                        }
                        isTyping = false;
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
                    type: "changeVisibility",
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
                action: "sendChatMessage",
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
            displayName = AvatarManager.getPalData([sessionId])?.data[0]?.sessionDisplayName || null;
            if (displayName == null) {
                for (let i = 0; i < palData.length; i++) {
                    if (palData[i].sessionUUID == sessionId) {
                        displayName = palData[i].sessionDisplayName;
                    }
                }
            }

            // Format the packet
            let message = {};
            const TIME_ARRAY = _formatTimestamp(_getTimestamp());
            message.timeString = TIME_ARRAY[0];
            message.dateString = TIME_ARRAY[1];
            message.message = `${displayName} ${type}`;

            // Show new message on screen
            if (settings.joinNotification) {
                Messages.sendLocalMessage(
                    "Floof-Notif",
                    JSON.stringify({
                        sender: displayName,
                        text: type,
                    })
                );
            }

            _emitEvent({ type: "notification", ...message });
        }, 1500);
    }
    function _loadSettings() {
        settings = Settings.getValue("DomainChat-Config", settings);

        if (messageHistory) {
            // Load message history
            messageHistory.forEach((message) => {
                const TIME_ARRAY = _formatTimestamp(_getTimestamp());
                message.timeString = TIME_ARRAY[0];
                message.dateString = TIME_ARRAY[1];
                _emitEvent({ type: "showMessage", ...message });
            });
        }

        // Send current settings to the app
        _emitEvent({ type: "initialSettings", settings: settings });
    }
    function _saveSettings() {
        console.log("Saving config");
        Settings.setValue("DomainChat-Config", settings);
    }
    function _getTimestamp() {
        return Date.now();
    }
    function _formatTimestamp(timestamp) {
        let timeArray = [];

        timeArray.push(new Date().toLocaleTimeString(undefined, {
            hour12: false,
        }));

        timeArray.push(new Date(timestamp).toLocaleDateString(undefined, {
            year: "numeric",
            month: "long",
            day: "numeric",
        }));

        return timeArray;
    }

    /**
     * Emit a packet to the HTML front end. Easy communication!
     * @param {Object} packet - The Object packet to emit to the HTML
     * @param {("showMessage"|"clearMessages"|"notification"|"initialSettings")} packet.type - The type of packet it is
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
