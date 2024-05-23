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

    var app_is_visible = false;
    var settings = {
        external_window: false,
        maximum_messages: 200,
    };

    // Global vars
    var tablet;
    var chat_overlay_window;
    var app_button;
    var quick_message;
    const channels = ["domain", "local"];
    var message_history = Settings.getValue("ArmoredChat-Messages", []) || [];
    var max_local_distance = 20; // Maximum range for the local chat
    var pal_data = AvatarManager.getPalData().data;

    Controller.keyPressEvent.connect(keyPressEvent);
    Messages.subscribe("chat");
    Messages.messageReceived.connect(receivedMessage);
    AvatarManager.avatarAddedEvent.connect((session_id) => {
        _avatarAction("connected", session_id);
    });
    AvatarManager.avatarRemovedEvent.connect((session_id) => {
        _avatarAction("left", session_id);
    });

    startup();

    function startup() {
        tablet = Tablet.getTablet("com.highfidelity.interface.tablet.system");

        app_button = tablet.addButton({
            icon: Script.resolvePath("./img/icon_white.png"),
            activeIcon: Script.resolvePath("./img/icon_black.png"),
            text: "CHAT",
            isActive: app_is_visible,
        });

        // When script ends, remove itself from tablet
        Script.scriptEnding.connect(function () {
            console.log("Shutting Down");
            tablet.removeButton(app_button);
            chat_overlay_window.close();
        });

        // Overlay button toggle
        app_button.clicked.connect(toggleMainChatWindow);

        quick_message = new OverlayWindow({
            source: Script.resolvePath("./armored_chat_quick_message.qml"),
        });

        _openWindow();
    }
    function toggleMainChatWindow() {
        app_is_visible = !app_is_visible;
        console.log(`App is now ${app_is_visible ? "visible" : "hidden"}`);
        app_button.editProperties({ isActive: app_is_visible });
        chat_overlay_window.visible = app_is_visible;

        // External window was closed; the window does not exist anymore
        if (chat_overlay_window.title == "" && app_is_visible) {
            _openWindow();
        }
    }
    function _openWindow() {
        chat_overlay_window = new Desktop.createWindow(
            Script.resolvePath("./armored_chat.qml"),
            {
                title: "Chat",
                size: { x: 550, y: 400 },
                additionalFlags: Desktop.ALWAYS_ON_TOP,
                visible: app_is_visible,
                presentationMode: Desktop.PresentationMode.VIRTUAL,
            }
        );

        chat_overlay_window.closed.connect(toggleMainChatWindow);
        chat_overlay_window.fromQml.connect(fromQML);
        quick_message.fromQml.connect(fromQML);
    }
    function receivedMessage(channel, message) {
        // Is the message a chat message?
        channel = channel.toLowerCase();
        if (channel !== "chat") return;
        console.log(`Received message:\n${message}`);
        var message = JSON.parse(message);

        message.channel = message.channel.toLowerCase(); // Make sure the "local", "domain", etc. is formatted consistently

        if (!channels.includes(message.channel)) return; // Check the channel
        if (
            message.channel == "local" &&
            Vec3.distance(MyAvatar.position, message.position) >
                max_local_distance
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
        let saved_message = message;
        delete saved_message.position;
        saved_message.timeString = new Date().toLocaleTimeString(undefined, {
            hour12: false,
        });
        saved_message.dateString = new Date().toLocaleDateString(undefined, {
            year: "numeric",
            month: "long",
            day: "numeric",
        });
        message_history.push(saved_message);
        while (message_history.length > settings.maximum_messages) {
            message_history.shift();
        }
        Settings.setValue("ArmoredChat-Messages", message_history);
    }
    function fromQML(event) {
        console.log(`New QML event:\n${JSON.stringify(event)}`);

        switch (event.type) {
            case "send_message":
                _sendMessage(event.message, event.channel);
                break;
            case "setting_change":
                settings[event.setting] = event.value; // Update local settings
                _saveSettings(); // Save local settings

                switch (event.setting) {
                    case "external_window":
                        chat_overlay_window.presentationMode = event.value
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
                chat_overlay_window.visible = app_is_visible; // The "visible" field in the Desktop.createWindow does not seem to work. Force set it to the initial state (false)
                _loadSettings();
                break;
        }
    }
    function keyPressEvent(event) {
        switch (JSON.stringify(event.key)) {
            case "16777220": // Enter key
                quick_message.sendToQml({
                    type: "change_visibility",
                    value: true,
                });
        }
    }
    function _sendMessage(message, channel) {
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
    }
    function _avatarAction(type, session_id) {
        Script.setTimeout(() => {
            if (type == "connected") {
                pal_data = AvatarManager.getPalData().data;
            }

            // Get the display name of the user
            let display_name = "";
            display_name =
                AvatarManager.getPalData([session_id])?.data[0]
                    ?.sessionDisplayName || null;
            if (display_name == null) {
                for (let i = 0; i < pal_data.length; i++) {
                    if (pal_data[i].sessionUUID == session_id) {
                        display_name = pal_data[i].sessionDisplayName;
                    }
                }
            }

            // Format the packet
            let message = {};
            message.message = `${display_name} ${type}`;

            _emitEvent({ type: "avatar_connected", ...message });
        }, 1500);
    }
    function _loadSettings() {
        settings = Settings.getValue("ArmoredChat-Config", settings);

        if (message_history) {
            // Load message history
            message_history.forEach((message) => {
                delete message.action;
                _emitEvent({ type: "show_message", ...message });
            });
        }

        // Send current settings to the app
        _emitEvent({ type: "initial_settings", settings: settings });

        console.log(`\n\n\n` + JSON.stringify(settings));
    }
    function _saveSettings() {
        console.log("Saving config");
        Settings.setValue("ArmoredChat-Config", settings);
    }

    /**
     * Emit a packet to the HTML front end. Easy communication!
     * @param {Object} packet - The Object packet to emit to the HTML
     * @param {("setting_update"|"show_message")} packet.type - The type of packet it is
     */
    function _emitEvent(packet = { type: "" }) {
        chat_overlay_window.sendToQml(packet);
    }
})();
