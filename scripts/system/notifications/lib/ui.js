const UI = {
	closeAll: () => {
		util.debugLog('Closing all notification windows.')
		if (app._ui.overlayVR) {
			Entities.deleteEntity(app._ui.overlayVR);
			Entities.webEventReceived.disconnect((entityID, message) => { if (entityID === app._ui.overlayVR) onMessageFromQML(JSON.parse(message)) })
			app._ui.overlayVR = null;
		}
		if (app._ui.overlay) {
			app._ui.overlay.fromQml.disconnect(onMessageFromQML);
			app._ui.overlay.close();
			app._ui.overlay = null;
		}
	},
	openVR: () => {
		const CAMERA_MATRIX_INDEX = -7;
		const HMD_PANEL_LOCAL_POSITION = { "x": 0.3, "y": 0.25, "z": -1.5 };
		const HMD_PANEL_LOCAL_ROTATION = Quat.fromVec3Degrees({ "x": 0, "y": -3, "z": 0 });

		// Create the notification web entity
		const properties = {
			type: "Web",
			sourceUrl: Script.resolvePath("../qml/NotificationsBaseVR.qml"),
			position: { x: 0, y: 1, z: 0 },
			dimensions: { "x": 0.4, "y": 0.0, "z": 0.1 },
			visible: false,
			alpha: 0.9,
			dpi: 20,
			maxFPS: 60,
			useBackground: false,
			parentID: MyAvatar.sessionUUID,
			parentJointIndex: CAMERA_MATRIX_INDEX,
			localPosition: HMD_PANEL_LOCAL_POSITION,
			localRotation: HMD_PANEL_LOCAL_ROTATION,
			wantsKeyboardFocus: false,
			showKeyboardFocusHighlight: false,
			grab: {
				grabbable: false,
			}
		};

		app._ui.overlayVR = Entities.addEntity(properties, "local");
		Entities.webEventReceived.connect((entityID, message) => { if (entityID === app._ui.overlayVR) onMessageFromQML(JSON.parse(message)) })
	},
	openDesktop: () => {
		app._ui.overlay = new OverlayWindow({
			source: Script.resolvePath("../qml/NotificationsBase.qml"),
		});
		app._ui.overlay.fromQml.connect(onMessageFromQML);
	},
	resizeVROverlayFromActiveCount: (activeNotifications = 0) => {
		if (!app._ui.overlayVR) return;

		const DIMENSION_Y_VALUE = Math.min(activeNotifications, 4) * 0.1; 	// Never show more than 4 bubbles at once
		const OVERLAY_IS_ACTIVE = activeNotifications > 0;					// Don't have the overlay active if it is not needed.

		let dimensions = (Entities.getEntityProperties(app._ui.overlayVR, "dimensions")).dimensions;
		dimensions.y = DIMENSION_Y_VALUE;
		Entities.editEntity(app._ui.overlayVR, { dimensions, visible: OVERLAY_IS_ACTIVE });
	},
	sendNotificationHistory: () => {
		sendMessageToQML({ type: "notificationList", messages: [...app._data.systemNotifications, ...app._data.connectionNotifications] });
	}
}