"use strict";
//
//  notifications.js
//
//  Created by Adrian McCarlie on October 8th, 2014
//  Copyright 2014 High Fidelity, Inc.
//  Copyright 2022-2024 Overte e.V.
//
//  Display notifications to the user for some specific events.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

(function () {
    Script.include([
        "create/audioFeedback/audioFeedback.js"
    ]);

    var controllerStandard = Controller.Standard;

    var NOTIFICATIONS_MESSAGE_CHANNEL = "Hifi-Notifications";
    var SETTING_ACTIVATION_SNAPSHOT_NOTIFICATIONS = "snapshotNotifications";
    var NOTIFICATION_LIFE_DURATION = 10000; //10 seconds (in millisecond) before expiration.
    var FADE_OUT_DURATION = 1000; //1 seconds (in millisecond) to fade out.
    var NOTIFICATION_ALPHA = 0.9; // a value between: 0.0 (transparent) and 1.0 (fully opaque).
    var MAX_LINE_LENGTH = 42;
    var notifications = [];
    var newEventDetected = false;
    var isOnHMD = HMD.active;

    var textColor =  { "red": 228, "green": 228, "blue": 228};
    var backColor =  { "red": 2, "green": 2, "blue": 2}; 

    //DESKTOP OVERLAY PROPERTIES
    var overlayWidth = 340.0; //width in pixel of notification overlay in desktop
    var overlayLocationX = (Window.innerWidth - (overlayWidth + 20.0)); // positions window 20px from the right of the interface window
    var overlayLocationY = 20.0; // position down from top of interface window
    var overlayTopMargin = 13.0;
    var overlayLeftMargin = 10.0;
    var overlayFontSize = 12.0;
    var TEXT_OVERLAY_FONT_SIZE_IN_PIXELS = 18.0; // taken from TextOverlay::textSize
    var DESKTOP_INTER_SPACE_NOTIFICATION = 5; //5 px
    
    //HMD NOTIFICATION PANEL PROPERTIES
    var HMD_UI_SCALE_FACTOR = 1.0; //This define the size of all the notification system in HMD.
    var hmdPanelLocalPosition = {"x": 0.3, "y": 0.25, "z": -1.5};
    var hmdPanelLocalRotation = Quat.fromVec3Degrees({"x": 0, "y": -3, "z": 0});
    var mainHMDnotificationContainerID = Uuid.NONE;
    var CAMERA_MATRIX_INDEX = -7;
    
    //HMD LOCAL ENTITY PROPERTIES
    var entityWidth = 0.8; //in meter
    var HMD_LINE_HEIGHT = 0.03;
    var entityTopMargin = 0.02;
    var entityLeftMargin = 0.02;
    var HMD_INTER_SPACE_NOTIFICATION = 0.05;

    //ACTIONS
    // handles clicks on notifications overlays to delete notifications. (DESKTOP only)
    function mousePressEvent(event) {
        if (!isOnHMD) {
            var clickedOverlay = Overlays.getOverlayAtPoint({ x: event.x, y: event.y });
            for (var i = 0; i < notifications.length; i += 1) {
                if (clickedOverlay === notifications[i].overlayID || clickedOverlay === notifications[i].imageOverlayID) {
                    deleteSpecificNotification(i);
                    notifications.splice(i, 1);
                    newEventDetected = true;
                }
            }
        }
    }

    function checkHands() {
        var myLeftHand = Controller.getPoseValue(controllerStandard.LeftHand);
        var myRightHand = Controller.getPoseValue(controllerStandard.RightHand);
        var eyesPosition = MyAvatar.getEyePosition();
        var hipsPosition = MyAvatar.getJointPosition("Hips");
        var eyesRelativeHeight = eyesPosition.y - hipsPosition.y;
        if (myLeftHand.translation.y > eyesRelativeHeight || myRightHand.translation.y > eyesRelativeHeight) {
            audioFeedback.action();
            deleteAllExistingNotificationsDisplayed();
            notifications = [];
        }
    }

    //DISPLAY
    function renderNotifications(remainingTime) {
        overlayLocationX = (Window.innerWidth - (overlayWidth + 20.0)); 
        var alpha = NOTIFICATION_ALPHA;
        if (remainingTime < FADE_OUT_DURATION) {
            alpha = NOTIFICATION_ALPHA * (remainingTime/FADE_OUT_DURATION);
        }
        var properties, count, extraLine, breaks, height;
        var breakPoint = MAX_LINE_LENGTH + 1;
        var level = overlayLocationY;
        var entityLevel = 0;
        if (notifications.length > 0) {
            for (var i = 0; i < notifications.length; i++) {
                count = (notifications[i].dataText.match(/\n/g) || []).length,
                extraLine = 0;
                breaks = 0;
                if (notifications[i].dataText.length >= breakPoint) {
                    breaks = count;
                }                
                if (isOnHMD) {
                    //use HMD local entities
                    var sensorScaleFactor = MyAvatar.sensorToWorldScale * HMD_UI_SCALE_FACTOR;
                    var lineHeight = HMD_LINE_HEIGHT;
                    height = lineHeight + (2 * entityTopMargin);
                    extraLine = breaks * lineHeight;
                    height = (height + extraLine) * HMD_UI_SCALE_FACTOR;
                    entityLevel = entityLevel - (height/2);
                    properties = {
                        "type": "Text",
                        "parentID": mainHMDnotificationContainerID,
                        "localPosition": {"x": 0, "y": entityLevel, "z": 0},
                        "dimensions": {"x": (entityWidth * HMD_UI_SCALE_FACTOR), "y": height, "z": 0.01},
                        "isVisibleInSecondaryCamera": false,
                        "lineHeight": lineHeight * sensorScaleFactor,
                        "textColor": textColor,
                        "textAlpha": alpha,
                        "backgroundColor": backColor,
                        "backgroundAlpha": alpha,
                        "leftMargin": entityLeftMargin * sensorScaleFactor,
                        "topMargin": entityTopMargin * sensorScaleFactor,
                        "unlit": true,
                        "renderLayer": "hud"
                    };
                    if (notifications[i].entityID === Uuid.NONE){
                        properties.text =  notifications[i].dataText;
                        notifications[i].entityID = Entities.addEntity(properties, "local");
                    } else {
                        Entities.editEntity(notifications[i].entityID, properties);
                    }
                    if (notifications[i].dataImage !== null) {
                        entityLevel = entityLevel - (height/2);
                        height = (entityWidth / notifications[i].dataImage.aspectRatio) * HMD_UI_SCALE_FACTOR;
                        entityLevel = entityLevel - (height/2);
                        properties = {
                            "type": "Image",
                            "parentID": mainHMDnotificationContainerID,
                            "localPosition": {"x": 0, "y": entityLevel, "z": 0},
                            "dimensions":  {"x": (entityWidth * HMD_UI_SCALE_FACTOR), "y": height, "z": 0.01},
                            "isVisibleInSecondaryCamera": false,
                            "emissive": true,
                            "visible": true,
                            "alpha": alpha,
                            "renderLayer": "hud"
                        };                        
                        if (notifications[i].imageEntityID === Uuid.NONE){
                            properties.imageURL = notifications[i].dataImage.path;
                            notifications[i].imageEntityID = Entities.addEntity(properties, "local");
                        } else {
                            Entities.editEntity(notifications[i].imageEntityID, properties);
                        }
                    }
                    entityLevel = entityLevel - (height/2) - (HMD_INTER_SPACE_NOTIFICATION * HMD_UI_SCALE_FACTOR);
                } else {
                    //use Desktop overlays
                    height = 40.0;
                    extraLine = breaks * TEXT_OVERLAY_FONT_SIZE_IN_PIXELS;
                    height = height + extraLine;
                    properties = {
                        "x": overlayLocationX,
                        "y": level,
                        "width": overlayWidth,
                        "height": height,
                        "color": textColor,
                        "backgroundColor": backColor,
                        "alpha": alpha,
                        "topMargin": overlayTopMargin,
                        "leftMargin": overlayLeftMargin,
                        "font": {"size": overlayFontSize}
                    };
                    if (notifications[i].overlayID === Uuid.NONE){
                        properties.text =  notifications[i].dataText;
                        notifications[i].overlayID = Overlays.addOverlay("text", properties);
                    } else {
                        Overlays.editOverlay(notifications[i].overlayID, properties);
                    }
                    if (notifications[i].dataImage !== null) {
                        level = level + height;
                        height = overlayWidth / notifications[i].dataImage.aspectRatio;
                        properties = {
                            "x": overlayLocationX,
                            "y": level,
                            "width": overlayWidth,
                            "height": height,
                            "subImage": { "x": 0, "y": 0 },
                            "visible": true,
                            "alpha": alpha
                        };
                        if (notifications[i].imageOverlayID === Uuid.NONE){
                            properties.imageURL = notifications[i].dataImage.path;
                            notifications[i].imageOverlayID = Overlays.addOverlay("image", properties);
                        } else {
                            Overlays.editOverlay(notifications[i].imageOverlayID, properties);
                        }
                    }
                    level = level + height + DESKTOP_INTER_SPACE_NOTIFICATION;
                }
            }
        }
    }

    function deleteAllExistingNotificationsDisplayed() {
        if (notifications.length > 0) {
            for (var i = 0; i < notifications.length; i++) {
                deleteSpecificNotification(i);
            }
        }
    }
    
    function deleteSpecificNotification(indexNotification) {
        if (notifications[indexNotification].entityID !== Uuid.NONE){
            Entities.deleteEntity(notifications[indexNotification].entityID);
            notifications[indexNotification].entityID = Uuid.NONE;
        }
        if (notifications[indexNotification].overlayID !== Uuid.NONE){
            Overlays.deleteOverlay(notifications[indexNotification].overlayID);
            notifications[indexNotification].overlayID = Uuid.NONE;
        }
        if (notifications[indexNotification].imageEntityID !== Uuid.NONE){
            Entities.deleteEntity(notifications[indexNotification].imageEntityID);
            notifications[indexNotification].imageEntityID = Uuid.NONE;
        }
        if (notifications[indexNotification].imageOverlayID !== Uuid.NONE){
            Overlays.deleteOverlay(notifications[indexNotification].imageOverlayID);
            notifications[indexNotification].imageOverlayID = Uuid.NONE;
        }
    }
    
    function createMainHMDnotificationContainer() {
        if (mainHMDnotificationContainerID === Uuid.NONE) {
            var properties = {
                "type": "Shape",
                "shape": "Cube",
                "visible": false,
                "dimensions": {"x": 0.1, "y": 0.1, "z":0.1},
                "parentID": MyAvatar.sessionUUID,
                "parentJointIndex": CAMERA_MATRIX_INDEX,
                "localPosition": hmdPanelLocalPosition,
                "localRotation": hmdPanelLocalRotation
            };
            mainHMDnotificationContainerID = Entities.addEntity(properties, "local");
        }
    }
    
    function deleteMainHMDnotificationContainer() {
        if (mainHMDnotificationContainerID !== Uuid.NONE) {
            Entities.deleteEntity(mainHMDnotificationContainerID);
            mainHMDnotificationContainerID = Uuid.NONE;
        }
    }    
    //UTILITY FUNCTIONS

    // Trims extra whitespace and breaks into lines of length no more 
    // than MAX_LINE_LENGTH, breaking at spaces. Trims extra whitespace.

    function wordWrap(string) {
        var finishedLines = [], currentLine = '';
        string.split(/\s/).forEach(function (word) {
            var tail = currentLine ? ' ' + word : word;
            if ((currentLine.length + tail.length) <= MAX_LINE_LENGTH) {
                currentLine += tail;
            } else {
                finishedLines.push(currentLine);
                currentLine = word;
                if (currentLine.length > MAX_LINE_LENGTH) {
                    finishedLines.push(currentLine.substring(0,MAX_LINE_LENGTH));
                    currentLine = currentLine.substring(MAX_LINE_LENGTH, currentLine.length);
                }
            }
        });
        if (currentLine) {
            finishedLines.push(currentLine);
        }
        return finishedLines.join('\n');
    }

    //NOTIFICATION STACK MANAGEMENT
    
    function addNotification (dataText, dataImage) {
        var d = new Date();
        var notification = {
            "dataText": dataText,
            "dataImage": dataImage,
            "timestamp": d.getTime(),
            "entityID": Uuid.NONE,
            "imageEntityID": Uuid.NONE,
            "overlayID": Uuid.NONE,
            "imageOverlayID": Uuid.NONE
        };
        notifications.push(notification);
        newEventDetected = true;
        
        if (notifications.length === 1) {
            createMainHMDnotificationContainer();
            Script.update.connect(update);
            Controller.mousePressEvent.connect(mousePressEvent);
        }
        
    }

    function update(deltaTime) {
        if (notifications.length === 0 && !newEventDetected) {
            Script.update.disconnect(update);
            Controller.mousePressEvent.disconnect(mousePressEvent);
            deleteMainHMDnotificationContainer();
        } else {
            if (isOnHMD !== HMD.active) {
                deleteAllExistingNotificationsDisplayed();
                isOnHMD = HMD.active;
            }            
            var d = new Date();
            var immediatly = d.getTime();
            var mostRecentRemainingTime = NOTIFICATION_LIFE_DURATION;
            var expirationDetected = false;
            for (var i = 0; i < notifications.length; i++) {
                if ((immediatly - notifications[i].timestamp) > NOTIFICATION_LIFE_DURATION){
                    deleteSpecificNotification(i);
                    notifications.splice(i, 1);
                    expirationDetected = true;
                } else {
                    mostRecentRemainingTime = NOTIFICATION_LIFE_DURATION - (immediatly - notifications[i].timestamp);
                }
            }
            if (newEventDetected || expirationDetected || mostRecentRemainingTime < FADE_OUT_DURATION) {
                renderNotifications(mostRecentRemainingTime);
                newEventDetected = false;
            }
        }
        if (isOnHMD) {
            checkHands();
        }
    }

    //NOTIFICATION EVENTS FUNCTIONS
    function onDomainConnectionRefused(reason, reasonCode) {
        // the "login error" reason means that the DS couldn't decrypt the username signature
        // since this eventually resolves itself for good actors we don't need to show a notification for it
        var LoginErrorMetaverse_REASON_CODE = 2;
        if (reasonCode !== LoginErrorMetaverse_REASON_CODE) {
            addNotification("Connection refused: " + reason, null);
        }
    }

    function onEditError(msg) {
        addNotification(wordWrap(msg), null);
    }

    function onNotify(msg) {
         // Generic notification system for user feedback, thus using this
        addNotification(wordWrap(msg), null);
    }

    function onMessageReceived(channel, message) {
        if (channel === NOTIFICATIONS_MESSAGE_CHANNEL) {
            message = JSON.parse(message);
            addNotification(wordWrap(message.message), null);
        }
    }

    function onSnapshotTaken(pathStillSnapshot, notify) {
        if (Settings.getValue(SETTING_ACTIVATION_SNAPSHOT_NOTIFICATIONS, true)) {
            if (notify) {
                var imageProperties = {
                    "path": "file:///" + pathStillSnapshot,
                    "aspectRatio": Window.innerWidth / Window.innerHeight
                };
                addNotification(wordWrap("Snapshot saved to " + pathStillSnapshot), imageProperties);
            }
        }
    }

    function tabletNotification() {
        addNotification("Tablet needs your attention", null);
    }

    function processingGif() {
        if (Settings.getValue(SETTING_ACTIVATION_SNAPSHOT_NOTIFICATIONS, true)) {
            addNotification("Processing GIF snapshot...", null);
        }
    }

    function connectionAdded(connectionName) {
        addNotification(connectionName, null);
    }

    function connectionError(error) {
        addNotification(wordWrap("Error trying to make connection: " + error), null);
    }

    //STARTING AND ENDING
    
    function scriptEnding() {
        //cleanup
        deleteAllExistingNotificationsDisplayed();
        
        //disconnecting
        if (notifications.length > 0) {
            Script.update.disconnect(update);
            Controller.mousePressEvent.disconnect(mousePressEvent);
        }
        Script.scriptEnding.disconnect(scriptEnding);        
        Messages.unsubscribe(NOTIFICATIONS_MESSAGE_CHANNEL);
        Window.domainConnectionRefused.disconnect(onDomainConnectionRefused);
        Window.stillSnapshotTaken.disconnect(onSnapshotTaken);
        Window.snapshot360Taken.disconnect(onSnapshotTaken);
        Window.processingGifStarted.disconnect(processingGif);
        Window.connectionAdded.disconnect(connectionAdded);
        Window.connectionError.disconnect(connectionError);
        Window.announcement.disconnect(onNotify);
        Tablet.tabletNotification.disconnect(tabletNotification);
        Messages.messageReceived.disconnect(onMessageReceived);
    }

    Script.scriptEnding.connect(scriptEnding);
    
    //EVENTS TO NOTIFY
    Window.domainConnectionRefused.connect(onDomainConnectionRefused);
    Window.stillSnapshotTaken.connect(onSnapshotTaken);
    Window.snapshot360Taken.connect(onSnapshotTaken);
    Window.processingGifStarted.connect(processingGif);
    Window.connectionAdded.connect(connectionAdded);
    Window.connectionError.connect(connectionError);
    Window.announcement.connect(onNotify);
    Window.notifyEditError = onEditError;
    Window.notify = onNotify;
    Tablet.tabletNotification.connect(tabletNotification);
    Messages.subscribe(NOTIFICATIONS_MESSAGE_CHANNEL);
    Messages.messageReceived.connect(onMessageReceived);
}());
