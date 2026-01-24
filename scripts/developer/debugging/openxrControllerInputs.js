// extra-inputs.js
// Created by Ada <ada@thingvellir.net> on 2025-12-16
// SPDX-License-Identifier: Apache-2.0
"use strict";

if (!Controller.Hardware.OpenXR) { throw new Error("Not in OpenXR mode!"); }

const CAMERA_RELATIVE_CONTROLLER_RIGHTHAND_INDEX = -5;
const CAMERA_RELATIVE_CONTROLLER_LEFTHAND_INDEX = -6;

const XR = Controller.Hardware.OpenXR;

// TODO: Expose *all* inputs later?
const inputHandles = {
	left: {
		triggerTouch: XR.LTTouch,
		triggerClick: XR.LTClick,
		thumbstickTouch: XR.LSTouch,
		primaryTouch: XR.LeftPrimaryTouch,
		secondaryTouch: XR.LeftSecondaryTouch,

		trackpadX: XR.LeftTrackpadX,
		trackpadY: XR.LeftTrackpadY,
		trackpadTouch: XR.LeftTrackpadTouch,
		trackpadClick: XR.LeftTrackpadClick,
	},
	right: {
		triggerTouch: XR.RTTouch,
		triggerClick: XR.RTClick,
		thumbstickTouch: XR.RSTouch,
		primaryTouch: XR.RightPrimaryTouch,
		secondaryTouch: XR.RightSecondaryTouch,

		trackpadX: XR.RightTrackpadX,
		trackpadY: XR.RightTrackpadY,
		trackpadTouch: XR.RightTrackpadTouch,
		trackpadClick: XR.RightTrackpadClick,
	},
};

function fetchInputs() {
	let tmp = {
		left: {},
		right: {},
	};

	for (const [key, path] of Object.entries(inputHandles.left)) {
		tmp.left[key] = Controller.getValue(path);
	}

	for (const [key, path] of Object.entries(inputHandles.right)) {
		tmp.right[key] = Controller.getValue(path);
	}

	return tmp;
}

const defaultProps = {
	parentID: MyAvatar.sessionUUID,
	type: "Text",
	lineHeight: 0.015,
	dimensions: [0.2, 0.2, 0.1],
	localPosition: [0.0, 0.3, 0.0],
	backgroundAlpha: 0.8,
	unlit: true,
	ignorePickIntersection: true,
	renderLayer: "front",
};

const leftEntity = Entities.addEntity({
	parentJointIndex: CAMERA_RELATIVE_CONTROLLER_LEFTHAND_INDEX,
	localRotation: Quat.fromPitchYawRollDegrees(45, -90, 0),
	text: "Left display",
	...defaultProps
}, "local");

const rightEntity= Entities.addEntity({
	parentJointIndex: CAMERA_RELATIVE_CONTROLLER_RIGHTHAND_INDEX,
	localRotation: Quat.fromPitchYawRollDegrees(45, 90, 0),
	text: "Right display",
	...defaultProps
}, "local");

Script.update.connect(() => {
	const inputs = fetchInputs();

	let leftText = [], rightText = [];

	for (const [key, value] of Object.entries(inputs.left)) {
		leftText.push(`${key}: ${value.toFixed(2)}`);
	}

	for (const [key, value] of Object.entries(inputs.right)) {
		rightText.push(`${key}: ${value.toFixed(2)}`);
	}

	Entities.editEntity(leftEntity, { text: leftText.join("\n") });
	Entities.editEntity(rightEntity, { text: rightText.join("\n") });
});

Script.scriptEnding.connect(() => {
	Entities.deleteEntity(leftEntity);
	Entities.deleteEntity(rightEntity);
});
