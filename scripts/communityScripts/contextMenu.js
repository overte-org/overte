// SPDX-License-Identifier: Apache-2.0
"use strict";
const CONTEXT_MENU_SETTINGS = Settings.getValue("Context Menu", {
	actionsPerPage: 6,
	font: "Roboto",
	noSfx: false,
	parented: true,
	public: false,
});

const ACTIONS_PER_PAGE = CONTEXT_MENU_SETTINGS["actionsPerPage"] ?? 6;

const SOUND_OPEN = SoundCache.getSound(Script.resourcesPath() + "sounds/Expand.wav");
const SOUND_CLOSE = SoundCache.getSound(Script.resourcesPath() + "sounds/Collapse.wav");
const SOUND_CLICK = SoundCache.getSound(Script.resourcesPath() + "sounds/Button06.wav");
const SOUND_HOVER = SoundCache.getSound(Script.resourcesPath() + "sounds/Button04.wav");

// Roboto
// Inconsolata
// Courier
// Timeless
// https://*
const CONTEXT_MENU_FONT = CONTEXT_MENU_SETTINGS["font"] ?? "Roboto";
const MENU_TOGGLE_ACTION = [Controller.Standard.LeftPrimaryThumb, Controller.Standard.RightPrimaryThumb];
const MENU_TOGGLE_KEY = "b";
const TARGET_HOVER_ACTION = [Controller.Standard.LT, Controller.Standard.RT];

const CLICK_FUNC_CHANNEL = "org.overte.context-menu.click";
const ACTIONS_CHANNEL = "org.overte.context-menu.actions";
const MAIN_CHANNEL = "org.overte.context-menu";

const DEBOUNCE_THRESHOLD_MS = 250;

const SENSOR_TO_WORLD_MATRIX_INDEX = 65534;
const CAMERA_MATRIX_INDEX = 65529;

const EMPTY_FUNCTION = () => {};

const SELF_ACTIONS = [
	_target => {
		if (!HMD.active) { return; }

		return {
			text: "Recenter",
			textColor: [255, 192, 64],
			clickFunc: _target => {
				HMD.centerUI();
				MyAvatar.centerBody();
			},
		};
	},
	_target => {
		return {
			text: MyAvatar.getCollisionsEnabled() ? "[  ] Noclip" : "[X] Noclip",
			textColor: [127, 255, 255],
			clickFunc: _target => MyAvatar.setCollisionsEnabled(!MyAvatar.getCollisionsEnabled()),
		};
	},
	_target => {
		return {
			text: MyAvatar.getOtherAvatarsCollisionsEnabled() ? "[X] Avatar collisions" : "[  ] Avatar collisions",
			textColor: [127, 255, 255],
			clickFunc: _target => MyAvatar.setOtherAvatarsCollisionsEnabled(!MyAvatar.getOtherAvatarsCollisionsEnabled()),
		};
	},
];

const OBJECT_ACTIONS = [
	ent => {
		if (Entities.getNestableType(ent) !== "entity") { return; }
		const { cloneable } = Entities.getEntityProperties(ent, "cloneable");

		if (!cloneable) { return; }

		return {
			text: "Clone",
			textColor: [0, 255, 0],
			clickFunc: ent => {
				const newEnt = Entities.cloneEntity(ent);
				Entities.editEntity(newEnt, {
					position: Vec3.sum(MyAvatar.position, Quat.getFront(MyAvatar.orientation)),
					grab: {grabbable: true},
				});
			},
		};
	},
	ent => {
		if (Entities.getNestableType(ent) !== "entity") { return; }
		const { cloneOriginID } = Entities.getEntityProperties(ent, "cloneOriginID");

		if (Uuid.isNull(cloneOriginID)) { return; }

		return {
			text: "Delete",
			textColor: [255, 0, 0],
			clickFunc: ent => Entities.deleteEntity(ent),
		};
	},
];

const ROOT_ACTIONS = [
	target => {
		if (Entities.getNestableType(target) !== "entity") { return; }
		let { userData } = Entities.getEntityProperties(target, "userData");
		let submenu = "_OBJECT";

		if (userData) {
			try {
				userData = JSON.parse(userData);
				if (!(userData?.contextMenu?.root ?? true)) {
					submenu = "_TARGET";
				}
			} catch (e) {
				console.error(`ROOT_ACTIONS._OBJECT: ${e}`);
			}
		}

		return {
			text: "Object",
			textColor: [0, 255, 0],
			keepMenuOpen: true,
			submenu: submenu,
			priority: -101,
		};
	},
	target => {
		if (Entities.getNestableType(target) !== "avatar") { return; }

		const avatar = AvatarList.getAvatar(target);
		if (!avatar || Object.keys(avatar).length === 0) { return; }

		return {
			text: avatar.displayName || "unnamed",
			textColor: [255, 255, 0],
			keepMenuOpen: true,
			submenu: "_AVATAR",
			priority: -101,
		};
	},
	_target => ({
		text: "My Avatar",
		textColor: [126, 255, 255],
		keepMenuOpen: true,
		submenu: "_SELF",
		priority: -100,
	}),
];

let registeredActionSets = {
	"_ROOT": [...ROOT_ACTIONS],
	"_SELF": [...SELF_ACTIONS],
	"_OBJECT": [...OBJECT_ACTIONS],
	"_AVATAR": [],
	"_TARGET": [],
};
let registeredActionSetParents = {};
let registeredActionSetTitles = {};

const targetingPick = [
	Picks.createPick(PickType.Ray, {
		enabled: true,
		filter: Picks.PICK_DOMAIN_ENTITIES | Picks.PICK_AVATAR_ENTITIES | Picks.PICK_LOCAL_ENTITIES | Picks.PICK_INCLUDE_NONCOLLIDABLE | Picks.PICK_AVATARS,
		maxDistance: 20,
		joint: HMD.active ? "_CAMERA_RELATIVE_CONTROLLER_LEFTHAND" : "Mouse",
	}),
	Picks.createPick(PickType.Ray, {
		enabled: true,
		filter: Picks.PICK_DOMAIN_ENTITIES | Picks.PICK_AVATAR_ENTITIES | Picks.PICK_LOCAL_ENTITIES | Picks.PICK_INCLUDE_NONCOLLIDABLE | Picks.PICK_AVATARS,
		maxDistance: 20,
		joint: HMD.active ? "_CAMERA_RELATIVE_CONTROLLER_RIGHTHAND" : "Mouse",
	}),
];

let currentMenuOpen = false;

/**
 * List of all entities owned by the context menu,
 * as well as a map to a function for when they're clicked.
 * @type Map<Uuid, null | (target: Uuid) => void>
 */
let currentMenuEntities = new Map();
/**
 * Callback actions for each action button.
 * @type Array<(target: Uuid) => void>
 */
let currentMenuActionFuncs = [];
/**
 * The selected target for the context menu, set by
 * holding down click on something when opening the menu.
 * @type Uuid
 */
let currentMenuTarget = Uuid.NONE;
let currentMenuTargetIsAvatar = false;
let currentMenuInSubmenu = false;
let mouseWasCaptured = false;
let disableCounter = 0;
let prevClickTime = Date.now();
/**
 * Stack of submenu action sets to allow going to previously open ones
 * @type Array<[actionSetName: string, page: number]>
 */
let actionSetHistory = [];

function ContextMenu_DeleteMenu() {
	for (const [e, _] of currentMenuEntities) {
		Entities.deleteEntity(e);
	}
	currentMenuEntities.clear();
	currentMenuActionFuncs = [];
	currentMenuOpen = false;
	currentMenuInSubmenu = false;
	currentMenuTargetIsAvatar = false;
	currentMenuTarget = Uuid.NONE;
	actionSetHistory = [];
	Camera.captureMouse = mouseWasCaptured;
}

function ContextMenu_EntityClick(eid, event) {
	if (!event.isPrimaryButton) { return; }
	if (!currentMenuEntities.has(eid)) { return; }
	if (Date.now() < prevClickTime + DEBOUNCE_THRESHOLD_MS) { return; }

	prevClickTime = Date.now();
	currentMenuInSubmenu = false;

	try {
		const data = JSON.parse(Entities.getEntityProperties(eid, "userData").userData);
		if (data.nextPage !== undefined && data.actionSetName !== undefined) {
			if (data.type === "historyBack") {
				const history = actionSetHistory.pop();
				ContextMenu_OpenActions(history[0], history[1]);
			} else {
				ContextMenu_OpenActions(data.actionSetName, data.nextPage);
			}
		} else {
			const func = data.actionFunc;
			currentMenuActionFuncs[func][0](currentMenuTarget, eid);
			if (!currentMenuActionFuncs[func][1] && !currentMenuInSubmenu) {
				ContextMenu_DeleteMenu();
			}
		}
	} catch (e) {}

	if (!(CONTEXT_MENU_SETTINGS.noSfx ?? false)) { Audio.playSystemSound(SOUND_CLICK); }
}

function ContextMenu_EntityHoverEnter(eid, _event) {
	const entityAction = currentMenuEntities.get(eid);

	if (!entityAction) { return; }

	try {
		const data = JSON.parse(Entities.getEntityProperties(eid, "userData").userData);
		if (data.actionFunc !== undefined || (data.nextPage !== undefined && data.actionSetName !== undefined)) {
			if (!(CONTEXT_MENU_SETTINGS.noSfx ?? false)) { Audio.playSystemSound(SOUND_HOVER); }
		}
	} catch (e) {}

	// make the background a little lighter on hovered actions
	// NOTE: the spread operator is necessary here to get a copy,
	// otherwise we'll be modifying entityAction.backgroundColor itself
	let bgColor = [...entityAction.backgroundColor ?? [0, 0, 0]];
	bgColor[0] = Math.min(bgColor[0] + 48, 255);
	bgColor[1] = Math.min(bgColor[1] + 48, 255);
	bgColor[2] = Math.min(bgColor[2] + 48, 255);

	Entities.editEntity(eid, {
		backgroundColor: bgColor,
		textEffectColor: bgColor,
	});
}

function ContextMenu_EntityHoverLeave(eid, _event) {
	const entityAction = currentMenuEntities.get(eid);

	if (!entityAction) { return; }

	const bgColor = entityAction.backgroundColor ?? [0, 0, 0];

	Entities.editEntity(eid, {
		backgroundColor: bgColor,
		textEffectColor: bgColor
	});
}

function ContextMenu_FindTarget(hand = 1) {
	currentMenuTargetIsAvatar = false;

	const ray = Picks.getPrevPickResult(targetingPick[hand]);
	if (!ray.intersects) {
		currentMenuTarget = Uuid.NONE;
		return;
	}

	if (ray.type === 3) {
		currentMenuTarget = ray.objectID;
		currentMenuTargetIsAvatar = true;
	} else {
		currentMenuTarget = ray.objectID;
	}
}

function ContextMenu_OpenActions(actionSetName, page = 0) {
	currentMenuEntities.forEach((_, e) => Entities.deleteEntity(e));
	currentMenuActionFuncs = [];

	currentMenuInSubmenu = true;

	mouseWasCaptured = Camera.captureMouse;
	Camera.captureMouse = false;

	const scale = 1;
	// FIXME: lineHeight and margin properties sets don't take SNScale into account
	const lineScale = scale / MyAvatar.sensorToWorldScale;
	const myAvatar = MyAvatar.sessionUUID;

	const baseActions = registeredActionSets[actionSetName];

	let actionEnts = [];
	let origin;
	if (HMD.active) {
		origin = Vec3.sum(
			Camera.position,
			Vec3.multiplyQbyV(
				Camera.orientation,
				[0, -0.25 * scale, -1.0 * scale]
			)
		);
	} else {
		const remap = (low1, high1, low2, high2, value) => low2 + (value - low1) * (high2 - low2) / (high1 - low1);
		const spawnDist = remap(20, 130, 1.2, 0.25, Camera.frustum.fieldOfView);
		origin = Vec3.sum(
			Camera.position,
			Vec3.multiplyQbyV(
				Camera.orientation,
				[0, 0, -spawnDist * scale]
			)
		);
	}
	let angle = Quat.lookAtSimple(Camera.position, origin);

	let activeActions = [];

	let actions = [...Object.values(baseActions)];

	for (const [setName, parent] of Object.entries(registeredActionSetParents)) {
		if (parent === actionSetName) {
			actions.push(...Object.values(registeredActionSets[setName]));
		}
	}

	for (const action of actions) {
		let actionData = action(currentMenuTarget);

		if (!actionData || Object.keys(actionData).length === 0) { continue; }

		// the menu item is only valid for a target entity
		if (actionData.requiredTargets !== undefined && !actionData.requiredTargets.includes(currentMenuTarget)) {
			continue;
		}

		activeActions.push(actionData);
	}

	// FIXME: in some cases there's one too many pages?
	const hasPages = activeActions.length > ACTIONS_PER_PAGE;
	const maxPages = Math.floor(activeActions.length / ACTIONS_PER_PAGE);
	page = Math.max(0, Math.min(page, maxPages));
	activeActions.sort((a, b) => (a.priority ?? 0) - (b.priority ?? 0));
	activeActions = activeActions.slice(
		page * ACTIONS_PER_PAGE,
		(page * ACTIONS_PER_PAGE) + ACTIONS_PER_PAGE
	);

	let yPos = Math.max(1, activeActions.length) * 0.02 * scale;
	let bgHeight = (yPos / 2.0) - (0.03 * scale);

	let titleText;
	let descriptionText;
	if (currentMenuTargetIsAvatar) {
		const data = AvatarList.getAvatar(currentMenuTarget);
		titleText = data.displayName;
	} else if (currentMenuTarget) {
		const data = Entities.getEntityProperties(currentMenuTarget, ["type", "name", "description", "userData"]);
		const type = data.type ?? "UNKNOWN";

		if (data.name) {
			titleText = data.name;
		} else {
			titleText = `${type}`;
		}

		descriptionText = data.description;
	} else {
		titleText = "Self";
	}

	if (registeredActionSetTitles[actionSetName]?.title) {
		titleText = registeredActionSetTitles[actionSetName].title;
	}

	if (registeredActionSetTitles[actionSetName]?.description) {
		descriptionText = registeredActionSetTitles[actionSetName].description;
	}

	if (descriptionText) {
		actionEnts.push({
			grab: {grabbable: false},
			type: "Text",
			position: Vec3.sum(origin, Vec3.multiplyQbyV(angle, [0, yPos, 0])),
			rotation: angle,
			localDimensions: [0.3 * scale, 0.2 * scale, 0.01 * scale],
			text: descriptionText,
			textColor: [255, 255, 255],
			backgroundColor: [0, 0, 0],
			backgroundAlpha: 0.9,
			unlit: true,
			lineHeight: 0.016 * lineScale,
			verticalAlignment: "bottom",
			alignment: "center",
			triggerable: false,
			topMargin: 0.005 * lineScale,
			bottomMargin: 0.005 * lineScale,
			leftMargin: 0.005 * lineScale,
			rightMargin: 0.005 * lineScale,
			textEffect: "outline fill",
			textEffectColor: [0, 0, 0],
			textEffectThickness: 0.3,
		});

		yPos -= 0.122 * scale;
		bgHeight += 0.122;
	}

	actionEnts.push({
		grab: {grabbable: false},
		type: "Text",
		position: Vec3.sum(origin, Vec3.multiplyQbyV(angle, [0, yPos, 0])),
		rotation: angle,
		localDimensions: [0.215 * scale, 0.04 * scale, 0.01 * scale],
		text: titleText,
		textColor: [230, 230, 230],
		backgroundColor: [0, 0, 0],
		backgroundAlpha: 0.9,
		unlit: true,
		lineHeight: 0.018 * lineScale,
		verticalAlignment: "center",
		alignment: "center",
		triggerable: false,
		textEffect: "outline fill",
		textEffectColor: [0, 0, 0],
		textEffectThickness: 0.3,
	});

	let backHasPages = hasPages && page > 0;
	let backHasHistory = !backHasPages && actionSetHistory.length > 0;
	let backUserData;
	let backText = "";

	if (backHasHistory) {
		const history = actionSetHistory[actionSetHistory.length - 1];
		backUserData = { actionSetName: history[0], nextPage: history[1], type: "historyBack" };
		backText = "^";
	} else if (backHasPages) {
		backUserData = { nextPage: page - 1, actionSetName: actionSetName };
		backText = "<";
	}

	actionEnts.push({
		action: (backHasPages || backHasHistory) ? {} : undefined,
		grab: {grabbable: false},
		type: "Text",
		position: Vec3.sum(origin, Vec3.multiplyQbyV(angle, [-0.13 * scale, yPos, 0])),
		rotation: angle,
		localDimensions: [0.04 * scale, 0.04 * scale, 0.01 * scale],
		text: backText,
		textColor: [230, 230, 230],
		backgroundColor: [0, 0, 0],
		backgroundAlpha: 0.9,
		unlit: true,
		lineHeight: 0.05 * lineScale,
		verticalAlignment: "top",
		alignment: "center",
		triggerable: false,
		leftMargin: 0.003 * lineScale,
		topMargin: -0.008 * lineScale,
		userData: backUserData ? JSON.stringify(backUserData) : undefined,
		textEffect: "outline fill",
		textEffectColor: [0, 0, 0],
		textEffectThickness: 0.3,
	});

	actionEnts.push({
		action: hasPages && page < maxPages ? {} : undefined,
		grab: {grabbable: false},
		type: "Text",
		position: Vec3.sum(origin, Vec3.multiplyQbyV(angle, [0.13 * scale, yPos, 0])),
		rotation: angle,
		localDimensions: [0.04 * scale, 0.04 * scale, 0.01 * scale],
		text: hasPages && page < maxPages ? ">" : "",
		textColor: [230, 230, 230],
		backgroundColor: [0, 0, 0],
		backgroundAlpha: 0.9,
		unlit: true,
		lineHeight: 0.05 * lineScale,
		verticalAlignment: "top",
		alignment: "center",
		triggerable: false,
		leftMargin: -0.002 * lineScale,
		topMargin: -0.008 * lineScale,
		userData: hasPages && page < maxPages ? JSON.stringify({nextPage: page + 1, actionSetName: actionSetName}) : undefined,
		textEffect: "outline fill",
		textEffectColor: [0, 0, 0],
		textEffectThickness: 0.3,
	});

	yPos -= 0.047 * scale;
	bgHeight += 0.047;

	for (let i = 0; i < activeActions.length; i++) {
		let action = activeActions[i];

		let pos = Vec3.sum(origin, Vec3.multiplyQbyV(angle, [0, yPos, 0]));
		actionEnts.push({
			action,
			grab: {grabbable: false},
			type: "Text",
			position: pos,
			rotation: angle,
			localDimensions: [0.3 * scale, 0.05 * scale, 0.01 * scale],
			text: action.text,
			textColor: action.textColor ?? [255, 255, 255],
			backgroundColor: action.backgroundColor ?? [0, 0, 0],
			backgroundAlpha: action.backgroundAlpha ?? 0.8,
			unlit: true,
			lineHeight: 0.025 * lineScale,
			verticalAlignment: "center",
			alignment: "left",
			triggerable: true,
			leftMargin: 0.05 * lineScale,
			rightMargin: 0.05 * lineScale,
			userData: JSON.stringify({actionFunc: i}),
			textEffect: "outline fill",
			textEffectColor: action.backgroundColor ?? [0, 0, 0],
			textEffectThickness: 0.3,
		});
		if (action.iconImage) {
			let pos = Vec3.sum(origin, Vec3.multiplyQbyV(angle, [-0.125 * scale, yPos, 0.01 * scale]));
			actionEnts.push({
				grab: {grabbable: false},
				type: "Image",
				position: pos,
				rotation: angle,
				localDimensions: [0.03 * scale, 0.03 * scale, 0.01 * scale],
				imageURL: action.iconImage,
				emissive: true,
				userData: JSON.stringify({actionFunc: i}),
			});
		}
		let clickFunc = EMPTY_FUNCTION;
		if (action.submenu) {
			if (registeredActionSets[action.submenu]) {
				clickFunc = _target => {
					actionSetHistory.push([actionSetName, page]);
					ContextMenu_OpenActions(action.submenu);
				};
			} else {
				console.error(`Action "${action.text}" referencing unregistered submenu action set "${action.submenu}"`);
			}
		} else if (action.clickFunc) {
			clickFunc = action.clickFunc;
		} else if (action.remoteClickFunc) {
			clickFunc = target => {
				Messages.sendMessage(CLICK_FUNC_CHANNEL, JSON.stringify({
					func: action.remoteClickFunc,
					targetID: target,
				}));
			};
		} else if (action.localClickFunc) {
			clickFunc = target => {
				Messages.sendLocalMessage(CLICK_FUNC_CHANNEL, JSON.stringify({
					func: action.localClickFunc,
					targetID: target,
				}));
			};
		}
		currentMenuActionFuncs.push([clickFunc, action.keepMenuOpen ?? false]);
		yPos -= 0.0525 * scale;
		bgHeight += 0.0525;
	}

	if (activeActions.length === 0) {
		let pos = Vec3.sum(origin, Vec3.multiplyQbyV(angle, [0, yPos, 0]));
		actionEnts.push({
			grab: {grabbable: false},
			type: "Text",
			position: pos,
			rotation: angle,
			localDimensions: [0.3 * scale, 0.05 * scale, 0.01 * scale],
			text: "(No actions)",
			textColor: [230, 230, 230],
			backgroundColor: [64, 64, 64],
			backgroundAlpha: 0.8,
			unlit: true,
			lineHeight: 0.025 * lineScale,
			verticalAlignment: "center",
			alignment: "left",
			triggerable: true,
			leftMargin: 0.05 * lineScale,
			rightMargin: 0.05 * lineScale,
			textEffect: "outline fill",
			textEffectColor: [0, 0, 0],
			textEffectThickness: 0.3,
		});
		yPos -= 0.0525 * scale;
		bgHeight += 0.0525;
	}

	for (let a of actionEnts) {
		if (CONTEXT_MENU_SETTINGS.parented ?? true) {
			a.parentID = myAvatar;
			a.parentJointIndex = HMD.active ? SENSOR_TO_WORLD_MATRIX_INDEX : CAMERA_MATRIX_INDEX;
		}
		a.font = CONTEXT_MENU_FONT;
		a.canCastShadow = false;
		a.isVisibleInSecondaryCamera = false;
		a.fadeInMode = "disabled";
		a.fadeOutMode = "disabled";

		const e = Entities.addEntity(a, (CONTEXT_MENU_SETTINGS.public ?? false) ? "avatar" : "local");
		currentMenuEntities.set(e, a.action ?? null);
	}

	currentMenuOpen = true;
}

let mouseButtonHeld = false;
function ContextMenu_MousePressEvent(event) {
	if (event.isLeftButton) { mouseButtonHeld = true; }
}
function ContextMenu_MouseReleaseEvent(event) {
	if (event.isLeftButton) { mouseButtonHeld = false; }
}

function ContextMenu_OpenRoot() {
	registeredActionSets["_TARGET"] = [];
	delete registeredActionSetTitles["_TARGET"];

	// check if the target is worth selecting, so we don't
	// start highlighting something that we can't do anything with
	if (currentMenuTarget && !currentMenuTargetIsAvatar) {
		const {
			userData,
			cloneable,
			cloneOriginID,
		} = Entities.getEntityProperties(currentMenuTarget, [
			"userData",
			"cloneable",
			"cloneOriginID",
		]);

		let data = {};
		try {
			data = JSON.parse(userData);
		} catch (e) {}

		if (
			data?.contextMenu === undefined &&
			!cloneable &&
			Uuid.isNull(cloneOriginID)
		) {
			// doesn't do anything interesting, don't bother highlighting
			currentMenuTarget = undefined;
		}

		// don't target UI elements
		if (
			Keyboard.containsID(currentMenuTarget) ||
			currentMenuTarget === HMD.tabletID ||
			currentMenuTarget === HMD.tabletScreenID ||
			currentMenuTarget === HMD.homeButtonID ||
			currentMenuTarget === HMD.homeButtonHighlightID ||
			currentMenuTarget === HMD.miniTabletID ||
			currentMenuTarget === HMD.miniTabletScreenID
		) {
			currentMenuTarget = undefined;
		}
	}

	if (currentMenuTarget && !currentMenuTargetIsAvatar) {
		try {
			const { userData } = Entities.getEntityProperties(currentMenuTarget, "userData");
			const data = userData ? JSON.parse(userData) : undefined;

			if (data?.contextMenu?.actions) {
				if (Array.isArray(data.contextMenu.actions)) {
					// objects with custom context menu actions
					for (const action of data.contextMenu.actions) {
						registeredActionSets["_TARGET"].push(_entity => action);
					}

					registeredActionSetTitles["_TARGET"] = {
						title: data.contextMenu.title,
						description: data.contextMenu.description,
					};

					if (data.contextMenu.root ?? true) {
						ContextMenu_OpenActions("_TARGET");
					} else {
						ContextMenu_OpenActions("_ROOT");
					}

					if (!(CONTEXT_MENU_SETTINGS.noSfx)) { Audio.playSystemSound(SOUND_OPEN); }
				} else {
					// objects with only one implicit action that's triggered by the context menu button
					const remoteFunc = data?.contextMenu?.actions?.remoteClickFunc;
					if (remoteFunc) {
						Messages.sendMessage(CLICK_FUNC_CHANNEL, JSON.stringify({
							func: remoteFunc,
							targetID: currentMenuTarget,
						}));
					}

					const localFunc = data?.contextMenu?.actions?.localClickFunc;
					if (localFunc) {
						Messages.sendLocalMessage(CLICK_FUNC_CHANNEL, JSON.stringify({
							func: localFunc,
							targetID: currentMenuTarget,
						}));

						currentMenuTarget = undefined;
					}
				}

				return;
			}
		} catch (e) {
			console.error(`ContextMenu_OpenRoot: ${e}`);
		}
	}

	ContextMenu_OpenActions("_ROOT");

	if (!(CONTEXT_MENU_SETTINGS.noSfx ?? false)) { Audio.playSystemSound(SOUND_OPEN); }
}

function ContextMenu_KeyEvent(event) {
	if (event.text === MENU_TOGGLE_KEY && !event.isAutoRepeat) {
		if (currentMenuOpen) {
			ContextMenu_DeleteMenu();

			if (!(CONTEXT_MENU_SETTINGS.noSfx ?? false)) { Audio.playSystemSound(SOUND_CLOSE); }
		} else if (disableCounter === 0) {
			if (mouseButtonHeld) { ContextMenu_FindTarget(); }
			ContextMenu_OpenRoot();
		}
	}
}

let controllerHovering = [false, false];
function ContextMenu_ActionEvent(action, value) {
	if (TARGET_HOVER_ACTION[0] === action) {
		controllerHovering[0] = value > 0.5;
	} else if (TARGET_HOVER_ACTION[1] === action) {
		controllerHovering[1] = value > 0.5;
	} else if (MENU_TOGGLE_ACTION.includes(action) && value > 0.5) {
		if (currentMenuOpen) {
			ContextMenu_DeleteMenu();

			if (!(CONTEXT_MENU_SETTINGS.noSfx ?? false)) { Audio.playSystemSound(SOUND_CLOSE); }
		} else if (disableCounter === 0) {
			if (controllerHovering[1]) { ContextMenu_FindTarget(1); }
			else if (controllerHovering[0]) { ContextMenu_FindTarget(0); }
			ContextMenu_OpenRoot();
		}
	}
}

function ContextMenu_Message(channel, msg, _senderID, _localOnly) {
	if (channel === MAIN_CHANNEL) {
		let data; try { data = JSON.parse(msg); } catch (e) {}

		if (data?.func === "disable") {
			disableCounter += 1;
		} else if (data?.func === "enable" && disableCounter > 0) {
			disableCounter -= 1;
		}

		return;
	}

	if (channel !== ACTIONS_CHANNEL) { return; }

	let data; try { data = JSON.parse(msg); } catch (e) {}

	if (data?.func === "register") {
		let tmp = {};
		for (const [k, v] of Object.entries(data.actionSet)) {
			tmp[k] = _entity => v;
		}
		registeredActionSets[data.name] = tmp;
		if (data.parent) {
			registeredActionSetParents[data.name] = data.parent;
		}
		registeredActionSetTitles[data.name] = {title: data?.title, description: data?.description};
	} else if (data?.func === "unregister") {
		delete registeredActionSets[data.name];
		delete registeredActionSetParents[data.name];
		delete registeredActionSetTitles[data.name];
	} else if (data?.func === "edit") {
		if (!(data.name in registeredActionSets)) {
			console.error(`ContextMenu_Message: tried to edit unregistered action set "${data.name}"`);
			return;
		}

		for (const [k, v] of Object.entries(data.actionSet)) {
			registeredActionSets[data.name][k] = _entity => v;
		}
	}
}

Messages.messageReceived.connect(ContextMenu_Message);
Controller.keyPressEvent.connect(ContextMenu_KeyEvent);
Controller.inputEvent.connect(ContextMenu_ActionEvent);
Controller.mousePressEvent.connect(ContextMenu_MousePressEvent);
Controller.mouseReleaseEvent.connect(ContextMenu_MouseReleaseEvent);
Entities.mousePressOnEntity.connect(ContextMenu_EntityClick);
Entities.hoverEnterEntity.connect(ContextMenu_EntityHoverEnter);
Entities.hoverLeaveEntity.connect(ContextMenu_EntityHoverLeave);

for (const pick of targetingPick) {
	Picks.setIgnoreItems(pick, [MyAvatar.sessionUUID]);
}

MyAvatar.sessionUUIDChanged.connect(() => {
	for (const pick of targetingPick) {
		Picks.setIgnoreItems(pick, [MyAvatar.sessionUUID]);
	}
});

Messages.sendLocalMessage(MAIN_CHANNEL, JSON.stringify({func: "startup"}));

Script.scriptEnding.connect(() => {
	Messages.messageReceived.disconnect(ContextMenu_Message);
	Controller.keyPressEvent.disconnect(ContextMenu_KeyEvent);
	Controller.inputEvent.disconnect(ContextMenu_ActionEvent);
	Controller.mousePressEvent.disconnect(ContextMenu_MousePressEvent);
	Controller.mouseReleaseEvent.disconnect(ContextMenu_MouseReleaseEvent);
	Entities.mousePressOnEntity.disconnect(ContextMenu_EntityClick);
	Entities.hoverEnterEntity.disconnect(ContextMenu_EntityHoverEnter);
	Entities.hoverLeaveEntity.disconnect(ContextMenu_EntityHoverLeave);
	Picks.removePick(targetingPick[0]);
	Picks.removePick(targetingPick[1]);
	ContextMenu_DeleteMenu();

	Messages.sendLocalMessage(MAIN_CHANNEL, JSON.stringify({func: "shutdown"}));
});
