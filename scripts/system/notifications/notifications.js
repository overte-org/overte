//
//  notifications.js
//
//  Created by Armored Dragon on 3 June 2025.
//  Copyright 2025 Overte e.V contributors.
//
//  This interface script provides a basic interface for displaying notifications 
// 	and in a neat manner to present them to the user.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

// TODO: Chat notifications?

Script.include('./lib/utility.js');
Script.include('./lib/io.js');
Script.include('./lib/sound.js');
Script.include('./lib/window.js');

let app = {
	_config: {
		enabled: true,				// Global enable / disable
		maximumSavedSystemNotifications: 20,
		maximumSavedConnectionNotifications: 20,
	},
	_ui: {
		overlay: null,
		notificationPopout: null
	},
	_data: {
		systemNotifications: [],
		connectionNotifications: [],
	}
}
addNotificationUIToInterface();
subscribeToMessages();
io.getNotifications();

function addNotificationUIToInterface() {
	// Generates the QML element(s) required to present the notifications to the screen
	app._ui.overlay = new OverlayWindow({
		source: Script.resolvePath("./qml/Notifications.qml"),
	});

	app._ui.overlay.fromQml.connect(onMessageFromQML);
}

function subscribeToMessages() {
	Messages.subscribe("overte.notification");
	Messages.subscribe("Floof-Notif");
	Messages.subscribe("Hifi-Notifications");
}

Messages.messageReceived.connect(receivedMessage);


const notification = {
	system: (message = "No title", details = "No further information.", sound = false) => {
		// Tell QML to render the announcement
		sendMessageToQML({ type: "addSystemNotification", message, details });

		// If the notification window is open, add that notification to the list.
		sendNotificationListToNotificationPopout();

		// Play a sound
		if (sound) playSound.system();

	},
	connection: (text = "") => {
		// Once we have connections implemented, finish this.
		// Tell QML to render the announcement
		// TODO

		// Play a sound
		// TODO: 
	}
}

function receivedMessage(channel, message, senderID, localOnly) {
	if (localOnly === false) return;
	if (channel !== "overte.notification") return;

	message = util.toJSON(message);
	if (!message) return debugLog(`Failed to parse message to JSON.`);

	message.id = Uuid.generate();
	message.timestamp = Date.now();
	io.saveNotification(message);

	if (message.type === "system") {
		notification.system(message.message, message.details);
		return;
	}

	if (message.type === "connection") {
		// TODO
		return;
	}
}

function onMessageFromQML(event) {
	switch (event.type) {
		case "openNotificationFromOverlay":
			if (app._ui.notificationPopout === null) {
				app._ui.notificationPopout = new OverlayWindow({ source: Script.resolvePath("./qml/PopoutWindow.qml"), title: "Notifications", width: 400, height: 600 });
				app._ui.notificationPopout.closed.connect(() => { app._ui.notificationPopout = null });
				sendNotificationListToNotificationPopout();
			}
			break;
	}
}

function sendNotificationListToNotificationPopout() {
	if (app._ui.notificationPopout === null) return debugLog(`Notification window is not open. Not sending a message.`);

	let connectionNotificationsReversed = [...app._data.connectionNotifications];
	connectionNotificationsReversed.reverse();
	connectionNotificationsReversed.forEach((obj, index) => {
		connectionNotificationsReversed[index] = notificationFormat(obj);
	})

	let systemNotificationsReversed = [...app._data.systemNotifications];
	systemNotificationsReversed.reverse();
	systemNotificationsReversed.forEach((obj, index) => {
		systemNotificationsReversed[index] = notificationFormat(obj);
	})

	app._ui.notificationPopout.sendToQml({ type: "notificationList", messages: [...connectionNotificationsReversed, ...systemNotificationsReversed] });
}

function notificationFormat(notificationObject) {
	// This formats a notification object to human readable data where needed.
	// This modified data should not be saved anywhere, and should only be sent to the UI.
	if (notificationObject.timestamp) {
		notificationObject.timestamp = new Date(notificationObject.timestamp).toLocaleString();
	}

	return notificationObject;
}

function sendMessageToQML(message) {
	app._ui.overlay.sendToQml(message);
}

function debugLog(content) {
	if (typeof content === "object") content = JSON.stringify(content, null, 4);

	console.log(`[ Debug ] ${content}`);
}

Window.domainConnectionRefused.connect(windowFunc.domainConnectionRefused);
Window.stillSnapshotTaken.connect(windowFunc.stillSnapshotTaken);
Window.snapshot360Taken.connect(windowFunc.stillSnapshotTaken); // Same as still snapshot
Window.processingGifStarted.connect(windowFunc.processingGifStarted);
Window.connectionAdded.connect(windowFunc.connectionAdded);
Window.connectionError.connect(windowFunc.connectionError);
Window.announcement.connect(windowFunc.announcement);
Window.notifyEditError = windowFunc.notifyEditError;
Window.notify = windowFunc.notify;
Tablet.tabletNotification.connect(windowFunc.tabletNotification);