//
//  chatBubbles.js
//
//  Created by Ada <ada@thingvellir.net> on 2025-04-19
//  Copyright 2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
"use strict";

const CHAT_CHANNEL = "chat";

// can't reuse the chat channel because ArmoredChat passes
// anything on "chat" into FloofChat-Notif and throws an error
const TYPING_NOTIFICATION_CHANNEL = "ChatBubbles-Typing";
const CONFIG_UPDATE_CHANNEL = "ChatBubbles-Config";

const BUBBLE_LIFETIME_SECS = 10;
const MAX_DISTANCE = 20;
const SELF_BUBBLES = true;

let settings = {
	enabled: true,
};

let currentBubbles = {};
let typingIndicators = {};

function ChatBubbles_SpawnBubble(data, senderID) {
	if (currentBubbles[senderID]) {
		Entities.deleteEntity(currentBubbles[senderID].entity);
		Script.clearTimeout(currentBubbles[senderID].timeout);
		delete currentBubbles[senderID];
	}

	// TODO: handle avatar scale
	const bubbleEntity = Entities.addEntity({
		type: "Text",
		parentID: senderID,
		text: data.message,
		unlit: true,
		lineHeight: 0.07,
		dimensions: [1.3, 4, 0.01],
		localPosition: [0, 3.1, 0],
		backgroundAlpha: 0,
		textEffect: "outline fill",
		textEffectColor: "#000",
		textEffectThickness: 0.5,
		canCastShadow: false,
		billboardMode: "yaw",
		alignment: "center",
		verticalAlignment: "bottom",
	}, "local");

	currentBubbles[senderID] = {
		entity: bubbleEntity,
		timeout: Script.setTimeout(() => {
			Entities.deleteEntity(bubbleEntity);
			delete currentBubbles[senderID];
		}, BUBBLE_LIFETIME_SECS * 1000),
	};
}

function ChatBubbles_IndicatorTick(senderID) {
	const data = typingIndicators[senderID];

	const lowColor = [128, 192, 192];
	const hiColor = [255, 255, 255];

	let colorFade = 0.5 + (Math.cos(data.age / 5) * 0.5);

	Entities.editEntity(data.entity, {textColor: Vec3.mix(lowColor, hiColor, colorFade)});

	data.age += 1;
}

function ChatBubbles_ShowTypingIndicator(senderID) {
	if (typingIndicators[senderID]) { return; }

	// TODO: handle avatar scale
	const indicatorEntity = Entities.addEntity({
		type: "Text",
		parentID: senderID,
		text: "•••",
		unlit: true,
		lineHeight: 0.2,
		dimensions: [0.22, 0.1, 0.01],
		localPosition: [0, 1, 0],
		backgroundAlpha: 0.8,
		canCastShadow: false,
		billboardMode: "full",
		alignment: "center",
		verticalAlignment: "center",
		topMargin: -0.08,
	}, "local");

	const indicatorInterval = Script.setInterval(() => ChatBubbles_IndicatorTick(senderID), 50);

	typingIndicators[senderID] = {
		entity: indicatorEntity,
		interval: indicatorInterval,
		age: 0,
	};
}

function ChatBubbles_HideTypingIndicator(senderID) {
	const data = typingIndicators[senderID];

	if (!data) { return; }

	Entities.deleteEntity(data.entity);
	Script.clearInterval(data.interval);
	delete typingIndicators[senderID];
}

function ChatBubbles_RecvMsg(channel, msg, senderID, localOnly) {
    // IPC between ArmoredChat's config window and this script
    if (channel === CONFIG_UPDATE_CHANNEL && localOnly) {
        let data;
        try {
            data = JSON.parse(msg);
        } catch (e) {
            console.error(e);
            return;
        }

        for (const [key, value] of Object.entries(data)) {
            settings[key] = value;
        }
        return;
    }

    // not any other message we're interested in
    if (channel !== CHAT_CHANNEL && channel !== TYPING_NOTIFICATION_CHANNEL) { return; }

    // don't spawn bubbles for MyAvatar if the setting is disabled
    //if (!SELF_BUBBLES && (senderID === MyAvatar.sessionID || !MyAvatar.sessionID)) { return; }

	let data;
	try {
		data = JSON.parse(msg);
	} catch (e) {
		console.error(e);
		return;
	}

	if (channel === TYPING_NOTIFICATION_CHANNEL) {
		if (data.action === "typing_start") {
            // don't spawn a bubble if they're too far away
            if (Vec3.distance(MyAvatar.position, data.position) > MAX_DISTANCE) { return; }
			ChatBubbles_ShowTypingIndicator(senderID);
		} else if (data.action === "typing_stop") {
			ChatBubbles_HideTypingIndicator(senderID);
		}
	} else if (data.action === "send_chat_message" && settings.enabled) {
        // don't spawn a bubble if they're too far away
        if (data.channel !== "local") { return; }
        if (Vec3.distance(MyAvatar.position, data.position) > MAX_DISTANCE) { return; }
		ChatBubbles_SpawnBubble(data, senderID);
    }
}

function ChatBubbles_DeleteAll() {
	for (const [_, bubble] of Object.entries(currentBubbles)) {
		Entities.deleteEntity(bubble.entity);
		Script.clearTimeout(bubble.timeout);
	}

	for (const [_, indicator] of Object.entries(typingIndicators)) {
		Entities.deleteEntity(indicator.entity);
		Script.clearInterval(indicator.interval);
	}

    currentBubbles = {};
    typingIndicators = {};
}

function ChatBubbles_Delete(sessionID) {
    const bubble = currentBubbles[sessionID];
    const indicator = typingIndicators[sessionID];

    if (bubble) {
        Entities.deleteEntity(bubble.entity);
        Script.clearTimeout(bubble.timeout);
        delete currentBubbles[sessionID];
    }

    if (indicator) {
        Entities.deleteEntity(indicator.entity);
        Script.clearInterval(indicator.interval);
        delete typingIndicators[sessionID];
    }
}

// delete any chat bubbles or typing indicators if we get disconnected
Window.domainChanged.connect(_domainURL => ChatBubbles_DeleteAll());
Window.domainConnectionRefused.connect((_msg, _code, _info) => ChatBubbles_DeleteAll());

// delete the chat bubbles and typing indicators of someone who disconnects
AvatarList.avatarRemovedEvent.connect(sessionID => ChatBubbles_Delete(sessionID));
AvatarList.avatarSessionChangedEvent.connect((_, oldSessionID) => ChatBubbles_Delete(oldSessionID));

settings = Settings.getValue("ChatBubbles-Config", settings);
Messages.messageReceived.connect(ChatBubbles_RecvMsg);
Messages.subscribe(TYPING_NOTIFICATION_CHANNEL);
Messages.subscribe("chat");

Script.scriptEnding.connect(() => {
	Settings.setValue("ChatBubbles-Config", settings);
	Messages.messageReceived.disconnect(ChatBubbles_RecvMsg);
    Messages.unsubscribe(TYPING_NOTIFICATION_CHANNEL);
    Messages.unsubscribe("chat");
    ChatBubbles_DeleteAll();
});
