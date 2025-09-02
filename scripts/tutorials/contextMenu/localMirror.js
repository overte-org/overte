// SPDX-License-Identifier: Apache-2.0
"use strict";
const ContextMenu = Script.require("contextMenu");

let mirrorEntity;

function ToggleMirror() {
	if (mirrorEntity) {
		Entities.deleteEntity(mirrorEntity);
		mirrorEntity = undefined;
	} else {
		const scale = MyAvatar.sensorToWorldScale;
		let origin = Vec3.sum(
			MyAvatar.position,
			Vec3.multiplyQbyV(
				MyAvatar.orientation,
				[0, 0, -1.5 * scale]
			)
		);
		let angle = Quat.lookAtSimple(MyAvatar.position, origin);
		origin.y += 0.5 * scale;

		mirrorEntity = Entities.addEntity({
			type: "Box",
			dimensions: [3 * scale, 3 * scale, 0.01 * scale],
			position: origin,
			rotation: angle,
			mirrorMode: "mirror",
			canCastShadow: false,
			isVisibleInSecondaryCamera: false,
		}, "local");
	}
}

const actionSet = [
	{
		text: "Local Mirror",
		textColor: [192, 255, 255],
		localClickFunc: "localMirror.toggle",
		priority: -50,
	},
];
const actionFuncs = {
	"localMirror.toggle": () => ToggleMirror(),
};

ContextMenu.registerActionSet("localMirror", actionSet, ContextMenu.ROOT_SET);

Messages.messageReceived.connect((channel, msg, senderID, _localOnly) => {
	if (channel !== ContextMenu.CLICK_FUNC_CHANNEL) { return; }
	if (senderID !== MyAvatar.sessionUUID) { return; }

	const data = JSON.parse(msg);

	if (!(data.func in actionFuncs)) { return; }

	actionFuncs[data.func]();
});

Script.scriptEnding.connect(() => {
	Entities.deleteEntity(mirrorEntity);
	ContextMenu.unregisterActionSet("localMirror");
});
