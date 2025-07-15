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

Script.include('./lib/utility.js');
Script.include('./lib/io.js');
Script.include('./lib/sound.js');
Script.include('./lib/window.js');
Script.include('./lib/gesture.js');
Script.include('./lib/ui.js');

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
Script.scriptEnding.connect(shutDown);
Script.update.connect(update);
HMD.displayModeChanged.connect(changeOverlayBasedOnViewMode)

let app = {
	_config: {
		enabled: true,								// Global enable / disable
		maximumSavedSystemNotifications: 20,
		maximumSavedConnectionNotifications: 20,
		doNotDisturb: false
	},
	_ui: {
		overlay: null,
		overlayVR: null
	},
	_data: {
		systemNotifications: [],
		connectionNotifications: [],
	}
}

// io.getNotifications();
changeOverlayBasedOnViewMode();

function changeOverlayBasedOnViewMode() {
	util.debugLog(`Deploying notification interface...`);
	UI.closeAll();

	if (util.userIsUsingVR()) {
		util.debugLog(`User is in VR, creating web entity`);
		UI.openVR();
	}
	else {
		util.debugLog(`User is on Desktop, creating Overlay`);
		UI.openDesktop();
	}

	UI.sendNotificationHistory();
}

const notification = {
	system: (message = "No title", details = "", sound = false) => {
		// Save the notification
		// io.saveNotification({ message, details, type: `system`, id: Uuid.generate(), timestamp: Date.now() });

		// Tell QML to render the announcement
		sendMessageToQML({ type: "addSystemNotification", message, details });

		// Play a sound
		if (sound && !app._config.doNotDisturb) playSound.system();

	},
	// connection: () => { }
}

function onMessageFromQML(event) {
	if (event.type === "bubbleCount") return UI.resizeVROverlayFromActiveCount(event.count);
	if (event.type === "doNotDisturbState") return app._config.doNotDisturb = Boolean(event.state);
}

function notificationFormat(notificationObject) {
	// This formats a notification object to human readable data where needed.
	// This modified data should not be saved anywhere, and should only be sent to the UI.
	if (notificationObject.timestamp) {
		const date = new Date(notificationObject.timestamp);

		const year = date.getFullYear();
		const month = String(date.getMonth() + 1).padStart(2, '0');
		const day = String(date.getDate()).padStart(2, '0');
		const hours = String(date.getHours()).padStart(2, '0');
		const minutes = String(date.getMinutes()).padStart(2, '0');
		const seconds = String(date.getSeconds()).padStart(2, '0');

		notificationObject.timestamp = `${year}-${month}-${day} ${hours}:${minutes}:${seconds}`;
	}

	return notificationObject;
}

function update() {
	// The update function is currently only used to check the status of gestures.
	// So if we are not in VR, just do nothing.
	if (util.userIsUsingVR() === false) return;

	GESTURE.dismiss();
}

function sendMessageToQML(message) {
	if (app._ui.overlay) app._ui.overlay.sendToQml(message);
	if (app._ui.overlayVR) Entities.emitScriptEvent(app._ui.overlayVR, message);
}

function shutDown() {
	UI.closeAll();
	Script.update.disconnect(update);
	Window.domainConnectionRefused.disconnect(windowFunc.domainConnectionRefused);
	Window.stillSnapshotTaken.disconnect(windowFunc.stillSnapshotTaken);
	Window.snapshot360Taken.disconnect(windowFunc.stillSnapshotTaken); // Same as still snapshot
	Window.processingGifStarted.disconnect(windowFunc.processingGifStarted);
	Window.connectionAdded.disconnect(windowFunc.connectionAdded);
	Window.connectionError.disconnect(windowFunc.connectionError);
	Window.announcement.disconnect(windowFunc.announcement);
	Tablet.tabletNotification.disconnect(windowFunc.tabletNotification);
}