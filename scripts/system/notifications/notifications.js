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

Script.scriptEnding.connect(shutDown);
HMD.displayModeChanged.connect(changeOverlayBasedOnViewMode)

const CAMERA_MATRIX_INDEX = -7;
const hmdPanelLocalPosition = { "x": 0.3, "y": 0.25, "z": -1.5 };
const hmdPanelLocalRotation = Quat.fromVec3Degrees({ "x": 0, "y": -3, "z": 0 });

let app = {
	_config: {
		enabled: true,				// Global enable / disable
		maximumSavedSystemNotifications: 20,
		maximumSavedConnectionNotifications: 20,
	},
	_ui: {
		overlay: null,
		overlayVR: null,
		notificationPopout: null
	},
	_data: {
		systemNotifications: [],
		connectionNotifications: [],
	}
}

// This stores the state of active gestures. It is only used to make sure we don't spam actions.
let gestures = {
	quickCloseVR: false
}

io.getNotifications();
changeOverlayBasedOnViewMode();
Script.update.connect(update);

function changeOverlayBasedOnViewMode() {
	util.debugLog(`Deploying notification interface...`)
	closeAllWindows()

	if (util.userIsUsingVR()) {
		util.debugLog(`User is in VR, creating web entity`);
		// Create the notification web entity
		const properties = {
			type: "Web",
			sourceUrl: Script.resolvePath("./qml/NotificationsBaseVR.qml"),
			position: { x: 0, y: 1, z: 0 },
			dimensions: { "x": 0.4, "y": 0.37, "z": 0.1 },
			alpha: 0.9,
			dpi: 20,
			maxFPS: 60,
			useBackground: false,
			parentID: MyAvatar.sessionUUID,
			parentJointIndex: CAMERA_MATRIX_INDEX,
			localPosition: hmdPanelLocalPosition,
			localRotation: hmdPanelLocalRotation,
			wantsKeyboardFocus: false,
			showKeyboardFocusHighlight: false,
			localEntity: true,
			grab: {
				grabbable: false,
			}
		};

		app._ui.overlayVR = Entities.addEntity(properties, "local");
	}
	else {
		util.debugLog(`User is on Desktop, creating Overlay`);
		// Just rely in the overlay
		app._ui.overlay = new OverlayWindow({
			source: Script.resolvePath("./qml/NotificationsBase.qml"),
		});
		app._ui.overlay.fromQml.connect(onMessageFromQML);
	}


	sendMessageToQML({ type: "initialized", isVRMode: util.userIsUsingVR() })
}

const notification = {
	system: (message = "No title", details = "No further information.", sound = false) => {
		io.saveNotification({ message, details, type: `system`, id: Uuid.generate(), timestamp: Date.now() });

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
	if (app._ui.notificationPopout === null) return util.debugLog(`Notification window is not open. Not sending a message.`);

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
	if (util.userIsUsingVR()) {
		checkGestureDismissNotification();
	}
}

function checkGestureDismissNotification() {
	const myLeftHand = Controller.getPoseValue(Controller.Standard.LeftHand);
	const myRightHand = Controller.getPoseValue(Controller.Standard.RightHand);
	const eyesPosition = MyAvatar.getEyePosition();
	const hipsPosition = MyAvatar.getJointPosition("Hips");
	const eyesRelativeHeight = eyesPosition.y - hipsPosition.y;

	if (myLeftHand.translation.y > eyesRelativeHeight || myRightHand.translation.y > eyesRelativeHeight) {
		if (gestures.quickCloseVR === true) return;
		closeAllNotifications();
		gestures.quickCloseVR = true;
	} else {
		gestures.quickCloseVR = false;
	}
}

function closeAllNotifications() {
	util.debugLog("Closing all active notifications.");
	sendMessageToQML({ type: "closeAllNotifications" });
}

function sendMessageToQML(message) {
	util.debugLog(`Sending message to qml: ${JSON.stringify(message)}`);
	if (app._ui.overlay) app._ui.overlay.sendToQml(message);
	if (app._ui.overlayVR) Entities.emitScriptEvent(app._ui.overlayVR, message);
}

function shutDown() {
	closeAllWindows();
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

function closeAllWindows() {
	util.debugLog('Closing all notification windows.')
	if (app._ui.overlayVR) {
		Entities.deleteEntity(app._ui.overlayVR);
		app._ui.overlayVR = null;
	}
	if (app._ui.overlay) {
		app._ui.overlay.fromQml.disconnect(onMessageFromQML);
		app._ui.overlay.close();
		app._ui.overlay = null;
	}
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