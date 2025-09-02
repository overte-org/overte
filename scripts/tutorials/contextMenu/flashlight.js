// SPDX-License-Identifier: Apache-2.0
"use strict";
const ContextMenu = Script.require("contextMenu");

const LEFT_HAND = MyAvatar.getDominantHand() === "right" ? true : false;

let lightEntity;
let animationOverride;

function DesktopAnimationOverride(_oldProps) {
	const scale = MyAvatar.getAvatarScale();
	if (LEFT_HAND) {
		return {
			leftHandType: 0,
			leftHandRotation: Quat.fromPitchYawRollDegrees(93, 0, -70),
			leftHandPosition: Vec3.multiply([0.2, 0.2, 0.2], scale),
		};
	} else {
		return {
			rightHandType: 0,
			rightHandRotation: Quat.fromPitchYawRollDegrees(87, 0, 70),
			rightHandPosition: Vec3.multiply([-0.2, 0.2, 0.2], scale),
		};
	}
}

function ToggleFlashlight() {
	if (lightEntity) {
		MyAvatar.removeAnimationStateHandler(animationOverride);
		Entities.deleteEntity(lightEntity);
		lightEntity = undefined;
	} else {
		lightEntity = Entities.addEntity({
			type: "Light",
			parentID: MyAvatar.sessionUUID,
			parentJointIndex: MyAvatar.getJointIndex(LEFT_HAND ? "LeftHand" : "RightHand"),
			localDimensions: [50, 50, 50],
			localPosition: [0, 0, -0.1 * MyAvatar.getAvatarScale()],
			localRotation: Quat.fromPitchYawRollDegrees(90, 0, 0),
			isSpotlight: true,
			cutoff: 30,
			exponent: 1,
			falloffRadius: 1,
			intensity: 10,
		}, "avatar");

		if (!HMD.active) {
			animationOverride = MyAvatar.addAnimationStateHandler(DesktopAnimationOverride, null);
		}
	}
}

const actionSet = [
	{
		text: "Flashlight",
		textColor: [255, 200, 0],
		localClickFunc: "flashlight.toggle",
	},
];

ContextMenu.registerActionSet("flashlight", actionSet, ContextMenu.SELF_SET);

Messages.messageReceived.connect((channel, msg, senderID, _localOnly) => {
	if (channel !== ContextMenu.CLICK_FUNC_CHANNEL) { return; }
	if (senderID !== MyAvatar.sessionUUID) { return; }

	const data = JSON.parse(msg);

	if (data.func === "flashlight.toggle") {
		ToggleFlashlight();
		actionSet[0].textColor = lightEntity ? [0, 0, 0] : [255, 200, 0];
		actionSet[0].backgroundColor = lightEntity ? [255, 200, 0] : [0, 0, 0];
		ContextMenu.editActionSet("flashlight", actionSet);
	}
});

Script.scriptEnding.connect(() => {
	MyAvatar.removeAnimationStateHandler(animationOverride);
	Entities.deleteEntity(lightEntity);
	ContextMenu.unregisterActionSet("flashlight");
});
