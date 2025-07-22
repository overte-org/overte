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

const TYPING_NOTIFICATION_CHANNEL = "Chat-Typing";
const CONFIG_UPDATE_CHANNEL = "ChatBubbles-Config";

const BUBBLE_LIFETIME_SECS = 10;
const BUBBLE_FADE_TIME = 1;
const BUBBLE_ANIM_FPS = 15;
const BUBBLE_LINE_HEIGHT = 0.07;
const BUBBLE_WIDTH = 1.3;
const BUBBLE_WIDTH_MAX_CHARS = 24; // roughly 18 ems per meter
const MAX_DISTANCE = 20;
const SELF_BUBBLES = false;

const NOTIFY_SOUND = SoundCache.getSound(Script.resolvePath("./assets/notify.wav"));

let settings = {
    enabled: true,
};

let currentBubbles = {};
let typingIndicators = {};

// NOTE: naive wrapping algorithm that doesn't account
// for languages with non-latin scripts, though our SDF
// fonts and text renderer don't support them anyway
function ChatBubbles_WrapText(text, maxChars = BUBBLE_WIDTH_MAX_CHARS) {
    // split on spaces, periods, commas, slashes, hyphens, colons, and semicolons,
    // collapsing whitespace down to one space
    let tokens = text.replace(/\s+/g, " ").split(/([ \.,\/\-:;])/);
    let lineWidth = 0;
    let lineChunk = [];
    let linesAccum = [];

    for (const token of tokens) {
        // the split regex sometimes produces empty space tokens too, so skip those
        if (token.length < 1) { continue; }

        // this token would go over the limit,
        // push the line we have and start a new one
        if (lineWidth + token.length > maxChars && lineWidth !== 0) {
            linesAccum.push(lineChunk.join(""));
            lineChunk = [];
            lineWidth = 0;
        }

        // it's *still* too long for an empty line,
        // so break it apart into smaller chunks
        if (lineWidth + token.length > maxChars) {
            // split by codepoints so we don't get orphaned UTF16 surrogates
            let chars = [...token];
            let i = 0;

            while (i < chars.length) {
                const token = chars.slice(i, i + maxChars).join("");

                i += maxChars;

                // this token would go over the limit,
                // push the line we have and start a new one
                if (lineWidth + token.length > maxChars && lineWidth !== 0) {
                    linesAccum.push(lineChunk.join(""));
                    lineChunk = [];
                    lineWidth = 0;
                    lineChunk.push(token);
                } else {
                    // this token will fit, so add it to the current line
                    lineChunk.push(token);
                    lineWidth += token.length;
                }
            }
        } else {
            // this token will fit, so add it to the current line
            lineChunk.push(token);
            lineWidth += token.length;
        }
    }

    // push the trailing line
    linesAccum.push(lineChunk.join(""));

    return [linesAccum.join("\n"), linesAccum.length];
}

function ChatBubbles_SpawnBubble(data, senderID) {
    // this user doesn't have a bubble stack, so add one
    if (!currentBubbles[senderID]) {
        currentBubbles[senderID] = {};
    }

    const scale = AvatarList.getAvatar(senderID).scale;

    let link;
    let linkIsImage = false;

    // only handles cases where the whole message is just a URL,
    // text with a URL in the middle is ignored
    const maybeURL = data.message.trim();

    if (
        (maybeURL.startsWith("https://") || maybeURL.startsWith("http://")) &&
            !/\s+/g.test(maybeURL) &&
            /[A-Za-z0-9-._~:/?#\[\]@!$&'()*+,;%=]+/g.test(maybeURL)
    ) {
        link = maybeURL;

        const chunkBeforeQuery = maybeURL.split("?", 2)[0];

        if (
            chunkBeforeQuery.endsWith(".jpg") ||
                chunkBeforeQuery.endsWith(".png") ||
                chunkBeforeQuery.endsWith(".gif") ||
                chunkBeforeQuery.endsWith(".svg") ||
                chunkBeforeQuery.endsWith(".webp")
        ) {
            linkIsImage = true;
        }
    }

    const [text, lineCount] = ChatBubbles_WrapText(data.message);
    let height = lineCount * BUBBLE_LINE_HEIGHT;

    let bubbleEntity;
    if (link !== undefined && linkIsImage) {
        height = BUBBLE_WIDTH / 3;
        bubbleEntity = Entities.addEntity({
            type: "Image",
            parentID: senderID,
            imageURL: link,
            emissive: true,
            keepAspectRatio: true,
            ignorePickIntersection: true,
            dimensions: [BUBBLE_WIDTH, height, 0.01],
            localPosition: [0, scale + (height / 2) + 0.1, 0],
            canCastShadow: false,
            billboardMode: "yaw",
            grab: {grabbable: false},
        }, "local");
    } else {
        bubbleEntity = Entities.addEntity({
            type: "Text",
            parentID: senderID,
            text: text,
            unlit: true,
            ignorePickIntersection: (link === undefined),
            lineHeight: BUBBLE_LINE_HEIGHT,
            dimensions: [BUBBLE_WIDTH, height + 0.04, 0.01],
            localPosition: [0, scale + (height / 2) + 0.1, 0],
            backgroundAlpha: 0.5,
            textColor: (link === undefined) ? [255, 255, 255] : [128, 240, 255],
            textEffect: "outline fill",
            textEffectColor: "#000",
            textEffectThickness: 0.4,
            canCastShadow: false,
            billboardMode: "yaw",
            alignment: "center",
            verticalAlignment: "center",
            grab: {grabbable: false},
            script: (link === undefined && !linkIsImage) ? undefined :
`(function() {
    this.mousePressOnEntity = function(entity, event) {
        if (event.isPrimaryButton) {
            const url = ${JSON.stringify(link)};
            Window.openWebBrowser(url);
        }
    };
})`
        }, "local");
    }

    for (const bubble of Object.values(currentBubbles[senderID])) {
        let { localPosition } = Entities.getEntityProperties(bubble.entity, "localPosition");
        localPosition = Vec3.sum(localPosition, [0, height + 0.05, 0]);
        Entities.editEntity(bubble.entity, { localPosition: localPosition });
    }

    let bubbleIndex = Uuid.generate();

    let bubble = {
        entity: bubbleEntity,
        timeout: Script.setTimeout(() => {
            let fade = 1.0;

            const fadeInterval = Script.setInterval(() => {
                if (linkIsImage) {
                    Entities.editEntity(bubble.entity, { alpha: fade });
                } else {
                    Entities.editEntity(bubble.entity, { textAlpha: fade, backgroundAlpha: fade * 0.5 });
                }
                fade -= (1 / BUBBLE_ANIM_FPS) / BUBBLE_FADE_TIME;
            }, 1000 / BUBBLE_ANIM_FPS);

            bubble.timeout = Script.setTimeout(() => {
                Script.clearInterval(fadeInterval);
                Entities.deleteEntity(bubble.entity);
                delete currentBubbles[senderID][bubbleIndex];
            }, BUBBLE_FADE_TIME * 1000);
        }, BUBBLE_LIFETIME_SECS * 1000),
    };

    currentBubbles[senderID][bubbleIndex] = bubble;

    Audio.playSound(NOTIFY_SOUND, {
        position: data.position,
        volume: 0.25,
        localOnly: true,
    });
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

    const scale = AvatarList.getAvatar(senderID).scale;

    const indicatorEntity = Entities.addEntity({
        type: "Text",
        parentID: senderID,
        text: "•••",
        unlit: true,
        lineHeight: 0.15,
        dimensions: [0.18, 0.08, 0.01],
        localPosition: [0, scale, 0],
        backgroundAlpha: 0.8,
        canCastShadow: false,
        billboardMode: "full",
        alignment: "center",
        verticalAlignment: "center",
        textEffect: "outline fill",
        textEffectColor: "#000",
        textEffectThickness: 0.3,
        topMargin: -0.06,
        grab: {grabbable: false},
    }, "local");

    const indicatorInterval = Script.setInterval(() => ChatBubbles_IndicatorTick(senderID), 1000 / BUBBLE_ANIM_FPS);

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

        Settings.setValue("ChatBubbles-Config", settings);
        return;
    }

    // not any other message we're interested in
    if (channel !== CHAT_CHANNEL && channel !== TYPING_NOTIFICATION_CHANNEL) { return; }

    // don't spawn bubbles for MyAvatar if the setting is disabled
    if (!SELF_BUBBLES && (senderID === MyAvatar.sessionUUID || !MyAvatar.sessionUUID)) { return; }

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
    for (const [_, bubbleList] of Object.entries(currentBubbles)) {
        for (const [id, bubble] of Object.entries(bubbleList)) {
            Entities.deleteEntity(bubble.entity);
            Script.clearTimeout(bubble.timeout);
            delete bubbleList[id];
        }
    }

    for (const [_, indicator] of Object.entries(typingIndicators)) {
        Entities.deleteEntity(indicator.entity);
        Script.clearInterval(indicator.interval);
    }

    currentBubbles = {};
    typingIndicators = {};
}

function ChatBubbles_Delete(sessionID) {
    const bubbleList = currentBubbles[sessionID];
    const indicator = typingIndicators[sessionID];

    if (bubbleList) {
        for (const [_, bubble] of Object.entries(bubbleList)) {
            Entities.deleteEntity(bubble.entity);
            Script.clearTimeout(bubble.timeout);
        }
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

Script.scriptEnding.connect(() => {
    Settings.setValue("ChatBubbles-Config", settings);
    Messages.messageReceived.disconnect(ChatBubbles_RecvMsg);
    Messages.unsubscribe(TYPING_NOTIFICATION_CHANNEL);
    ChatBubbles_DeleteAll();
});
