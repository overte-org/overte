const GESTURE = {
	ACTIONS: {
		closeAllNotifications: () => {
			util.debugLog("Closing all active notifications.");
			sendMessageToQML({ type: "closeAllNotifications" });
		}
	},
	dismiss: () => {
		const myLeftHand = Controller.getPoseValue(Controller.Standard.LeftHand);
		const myRightHand = Controller.getPoseValue(Controller.Standard.RightHand);
		const eyesPosition = MyAvatar.getEyePosition();
		const hipsPosition = MyAvatar.getJointPosition("Hips");
		const eyesRelativeHeight = eyesPosition.y - hipsPosition.y;

		if (myLeftHand.translation.y > eyesRelativeHeight || myRightHand.translation.y > eyesRelativeHeight) {
			if (gestureStatus.quickCloseVR === true) return;
			GESTURE.ACTIONS.closeAllNotifications();
			gestureStatus.quickCloseVR = true;
		} else {
			gestureStatus.quickCloseVR = false;
		}
	},
}

// Gesture anti-spam protection
let gestureStatus = {
	quickCloseVR: false
}