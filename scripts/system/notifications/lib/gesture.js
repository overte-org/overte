const GESTURE = {
	dismiss: () => {
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
}