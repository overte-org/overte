//
//  domainChat.js
//
//  Created by Armored Dragon, 2024.
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html

// TODO: Message trimming

(() => {
    ("use strict");

    Script.include([
        "./formatting.js"
    ])

    var appIsVisible = false;
    var settings = {
        external_window: false,
        maximum_messages: 200,
        join_notification: true
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
    AvatarManager.avatarAddedEvent.connect((sessionId) => { _avatarAction("connected", sessionId); });
    AvatarManager.avatarRemovedEvent.connect((sessionId) => { _avatarAction("left", sessionId); });

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
    async function receivedMessage(channel, message) {
        // Is the message a chat message?
        channel = channel.toLowerCase();
        if (channel !== "chat") return;
        
        if ((message = formatting.toJSON(message)) == null) return;             // Make sure we are working with a JSON object we expect, otherwise kill
        message = formatting.addTimeAndDateStringToPacket(message);  
        
        if (!message.channel) message.channel = "domain";                       // We don't know where to put this message. Assume it is a domain wide message.
        message.channel = message.channel.toLowerCase();                        // Only recognize channel names as lower case. 
        
        if (!channels.includes(message.channel)) return;                        // Check the channel. If the channel is not one we have, do nothing.
        if (message.channel == "local" && isTooFar(message.position)) return;   // If message is local, and if player is too far away from location, do nothing.
        
        let formattedMessagePacket = { ...message };
        formattedMessagePacket.message = await _parseMessage(message.message)

        _emitEvent({ type: "show_message", ...formattedMessagePacket });        // Update qml view of to new message.
        _notificationCoreMessage(message.displayName, message.message)          // Show a new message on screen.

        // Create a new variable based on the message that will be saved.
        let trimmedPacket = formatting.trimPacketToSave(message);
        messageHistory.push(trimmedPacket);

        while (messageHistory.length > settings.maximum_messages) {
            messageHistory.shift();
        }
        Settings.setValue("ArmoredChat-Messages", messageHistory);

        function isTooFar(messagePosition) {
            // Check to see if the message is close enough to the user
            return Vec3.distance(MyAvatar.position, messagePosition) > maxLocalDistance;
        }
    }
    function fromQML(event) {
        switch (event.type) {
            case "send_message":
                _sendMessage(event.message, event.channel);
                break;
            case "setting_change":
                // Set the setting value, and save the config
                settings[event.setting] = event.value; // Update local settings
                _saveSettings(); // Save local settings

                // Extra actions to preform. 
                switch (event.setting) {
                    case "external_window":
                        chatOverlayWindow.presentationMode = event.value
                            ? Desktop.PresentationMode.NATIVE
                            : Desktop.PresentationMode.VIRTUAL;
                        break;
                }

                break;
            case "action":
                switch (event.action) {
                    case "erase_history":
                        Settings.setValue("ArmoredChat-Messages", null);
                        messageHistory = [];
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
    }
    function _avatarAction(type, sessionId) {
        Script.setTimeout(async () => {
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
            let message = addTimeAndDateStringToPacket({});
            message.message = `${displayName} ${type}`;

            // Show new message on screen
            if (settings.join_notification){
                _notificationCoreMessage(displayName, type)
            }

            // Format notification message
            let formattedMessagePacket = {...message};
            formattedMessagePacket.message = await _parseMessage(message.message);

            _emitEvent({ type: "notification", ...formattedMessagePacket });
        }, 1500);
    }
    async function _loadSettings() {
        settings = Settings.getValue("ArmoredChat-Config", settings);

        if (messageHistory) {
            // Load message history
            for (message of messageHistory) {
                messagePacket = { ...message };                                             // Create new variable
                messagePacket = formatting.addTimeAndDateStringToPacket(messagePacket);     // Add timestamp
                messagePacket.message = await _parseMessage(messagePacket.message);         // Parse the message for the UI

                _emitEvent({ type: "show_message", ...messagePacket });                     // Send message to UI
            }
        }

        _emitEvent({ type: "initial_settings", settings: settings });                       // Send current settings to the app
    }
    function _saveSettings() {
        console.log("Saving config");
        Settings.setValue("ArmoredChat-Config", settings);
    }
    function _notificationCoreMessage(displayName, message){
        Messages.sendLocalMessage(
            "Floof-Notif",
            JSON.stringify({ sender: displayName, text: message })
        );
    }
    async function _parseMessage(message){
        const urlRegex = /https?:\/\/(www\.)?[-a-zA-Z0-9@:%._\+~#=]{1,256}\.[a-zA-Z0-9()]{1,6}\b([-a-zA-Z0-9()@:%_\+.~#?&//=]*)/;
        const mentionRegex = /@(\w+)/; // FIXME: Remove - devcode
        const overteLocationRegex = /hifi:\/\/[a-zA-Z0-9_-]+\/[-+]?\d*\.?\d+,[+-]?\d*\.?\d+,[+-]?\d*\.?\d+\/[-+]?\d*\.?\d+,[+-]?\d*\.?\d+,[+-]?\d*\.?\d+,[+-]?\d*\.?\d+/;

        let runningMessage = message;
        let messageArray = [];

        const regexPatterns = [
            { type: "url", regex: urlRegex },
            { type: "mention", regex: mentionRegex }, // FIXME: Remove - devcode
            { type: "overteLocation", regex: overteLocationRegex }
        ]

        // Here is a link https://www.example.com, #hashtag, and @mention. Just for some spice here is another https://exampletwo.com

        while (true) {
            let firstMatch = _findFirstMatch();

            if (firstMatch == null) {
                // Format any remaining text as a basic 'text' type.
                messageArray.push({type: 'text', value: runningMessage});

                // Append a final 'fill width' to the message text.
                messageArray.push({type: 'messageEnd'});
                break;
            }

            _formatMessage(firstMatch);
        }

        for (dataChunk of messageArray){
            if (dataChunk.type == 'url'){
                let url = dataChunk.value;

                const res = await fetch(url, {method: 'GET'});      // TODO: Replace with 'HEAD' method. https://github.com/overte-org/overte/issues/1273
                const contentType = res.getResponseHeader("content-type");

                // TODO: Add support for other media types
                if (contentType.startsWith('image/')) {
                    messageArray.push({type: 'imageEmbed', value: url});
                }
            }
        }

        return messageArray;

        function _formatMessage(firstMatch){
            let indexOfFirstMatch = firstMatch[0];
            let regex = regexPatterns[firstMatch[1]].regex;

            let foundMatch = runningMessage.match(regex)[0];

            messageArray.push({type: 'text', value: runningMessage.substring(0, indexOfFirstMatch)});
            messageArray.push({type: regexPatterns[firstMatch[1]].type, value: runningMessage.substring(indexOfFirstMatch, indexOfFirstMatch + foundMatch.length)});
            
            runningMessage = runningMessage.substring(indexOfFirstMatch + foundMatch.length);   // Remove the part of the message we have worked with
        }

        function _findFirstMatch(){
            let indexOfFirstMatch = Infinity;
            let indexOfRegexPattern = Infinity;

            for (let i = 0; regexPatterns.length > i; i++){
                let indexOfMatch = runningMessage.search(regexPatterns[i].regex);

                if (indexOfMatch == -1) continue;                                              // No match found

                if (indexOfMatch < indexOfFirstMatch) {
                    indexOfFirstMatch = indexOfMatch;
                    indexOfRegexPattern = i;
                }
            }

            if (indexOfFirstMatch !== Infinity) return [indexOfFirstMatch, indexOfRegexPattern];    // If there was a found match
            return null;                                                                            // No found match
        }
    }

    /**
     * Emit a packet to the HTML front end. Easy communication!
     * @param {Object} packet - The Object packet to emit to the HTML
     * @param {("show_message"|"clear_messages"|"notification"|"initial_settings")} packet.type - The type of packet it is
     */
    function _emitEvent(packet = { type: "" }) {
        chatOverlayWindow.sendToQml(packet);
    }

    function fetch(url, options = {method: "GET"}) {
        return new Promise((resolve, reject) => {
            let req = new XMLHttpRequest();

            req.onreadystatechange = function () {

                if (req.readyState === req.DONE) {
                    if (req.status === 200) {
                        console.log("Content type:", req.getResponseHeader("content-type"));
                        resolve(req);
            
                    } else {
                        console.log("Error", req.status, req.statusText);
                        reject();
                    }
                }
            };

            req.open(options.method, url);
            req.send();
        });
    }

})();
