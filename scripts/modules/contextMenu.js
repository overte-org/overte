// SPDX-License-Identifier: Apache-2.0
"use strict";

const CLICK_FUNC_CHANNEL = "org.overte.context-menu.click";
const ACTIONS_CHANNEL = "org.overte.context-menu.actions";
const MAIN_CHANNEL = "org.overte.context-menu";

let registeredActionSets = {};
let registeredActionSetParents = {};
let registeredActionSetTitles = {};
let disabled = false;

function ContextMenu_registerActionSet(name, itemData, parent, title, desc) {
	registeredActionSets[name] = itemData;
	registeredActionSetParents[name] = parent;
	registeredActionSetTitles[name] = { title: title, description: desc };

	Messages.sendLocalMessage(ACTIONS_CHANNEL, JSON.stringify({
		func: "register",
		name: name,
		parent: parent,
		actionSet: itemData,
		title: title,
		description: desc,
	}));
}

function ContextMenu_unregisterActionSet(name) {
	delete registeredActionSets[name];
	delete registeredActionSetParents[name];
	Messages.sendLocalMessage(ACTIONS_CHANNEL, JSON.stringify({func: "unregister", name: name}));
}

function ContextMenu_messageReceived(channel, msg, senderID, _localOnly) {
	if (channel !== MAIN_CHANNEL) { return; }
	if (senderID !== MyAvatar.sessionUUID) { return; }

	const data = JSON.parse(msg);

	// re-register anything the context menu might have missed
	if (data.func === "startup") {
		for (const [name, set] of Object.entries(registeredActionSets)) {
			Messages.sendLocalMessage(ACTIONS_CHANNEL, JSON.stringify({
				func: "register",
				name: name,
				parent: registeredActionSetParents[name],
				actionSet: set,
				title: registeredActionSetTitles[name].title,
				description: registeredActionSetTitles[name].description,
			}));
		}
	}
}

function ContextMenu_editActionSet(name, itemData) {
	if (!(name in registeredActionSets)) {
		console.error(`ContextMenu_editActionSet: Attempted to edit unregistered action set "${name}"`);
		return;
	}

	Messages.sendLocalMessage(ACTIONS_CHANNEL, JSON.stringify({
		func: "edit",
		name: name,
		actionSet: itemData,
	}));
}

function ContextMenu_disable() {
	if (disabled) { return; }

	Messages.sendLocalMessage(MAIN_CHANNEL, JSON.stringify({ func: "disable" }));
	disabled = true;
}

function ContextMenu_enable() {
	if (!disabled) { return; }

	Messages.sendLocalMessage(MAIN_CHANNEL, JSON.stringify({ func: "enable" }));
	disabled = false;
}

Messages.messageReceived.connect(ContextMenu_messageReceived);

// This would be helpful for automatically unregistering action sets
// registered by a script that's about to close, but see
// https://github.com/overte-org/overte/issues/1767
Script.scriptEnding.connect(() => {
	Messages.messageReceived.disconnect(ContextMenu_messageReceived);

	if (disabled) {
		Messages.sendLocalMessage(MAIN_CHANNEL, JSON.stringify({ func: "enable" }));
	}
});

/**
 * @namespace ContextMenu
 *
 * See <code>scripts/tutorials/contextMenu</code> for examples.
 */

// FIXME: How do I express a userdata schema?
/**
 * @namespace userdata.contextMenu
 * @property {ContextMenu.Action[]|ContextMenu.Action} actions - Either a list of available actions, or one action that will be triggered without opening the context menu.
 * @property {string} [title] - The title of this action set. The title is above the action buttons and between the page buttons.
 * @property {string} [description] - The description text of this action set. The description is above the title and page buttons.
 * @property {boolean} [root=true] - If <code>true</code>, then when the context menu is opened targeting the parent entity it'll open this action set, rather than opening @link{ContextMenu.ROOT_SET}. If <code>false</code>, then this action set will be a child of @link{ContextMenu.OBJECT_SET}.
 */

/**
 * @typedef {Object} Action
 * @property {string} text
 * @property {Color} [textColor=[255, 255, 255]]
 * @property {Color} [backgroundColor=[0, 0, 0]]
 * @property {number} [backgroundAlpha=0.8]
 * @property {string} [iconImage] - URL to an image that will be used as an icon, to the left of the action text.
 * @property {string} [remoteClickFunc] - A string that will be sent with @link{Messages.sendMessage} when clicked. See @link{ContextMenu.CLICK_FUNC_CHANNEL}.
 * @property {string} [localClickFunc] - A string that will be sent with @link{Messages.sendLocalMessage} when clicked. See @link{ContextMenu.CLICK_FUNC_CHANNEL}.
 * @property {boolean} [keepMenuOpen=false] - If <code>true</code>, then the context menu will stay open when this action is clicked. Otherwise, the context menu will close when the action is clicked.
 */
module.exports = {
	/**
	 * The message channel that @link{ContextMenu.Action.remoteClickFunc} and @link{ContextMenu.Action.localClickFunc} use.
	 * @readonly
	 */
	CLICK_FUNC_CHANNEL: CLICK_FUNC_CHANNEL,

	/**
	 * The top-level set first opened if not overridden by the target.
	 * @readonly
	 */
	ROOT_SET: "_ROOT",

	/**
	 * The "My Avatar" submenu in the context menu root.
	 * @readonly
	 */
	SELF_SET: "_SELF",

	/**
	 * The "Object" submenu in the context menu root when an object is targeted.
	 * @readonly
	 */
	OBJECT_SET: "_OBJECT",

	/**
	 * The "Avatar" submenu in the context menu root when an avatar is targeted.
	 * @readonly
	 */
	AVATAR_SET: "_AVATAR",

	/**
	 * Registers a new action set for the context menu.
	 * @param {string} name - The name of the action set
	 * @param {(Object.<string, ContextMenu.Action>|ContextMenu.Action[])} actions - The action information
	 * @param {string} [parent] - The parent of the action set. If no parent is set, the action set will only be accessible through another action's "submenu" property. See @link{ROOT_SET}, @link{SELF_SET}, @link{OBJECT_SET}, and @link{AVATAR_SET}.
	 * @param {string} [title] - The name of this action set, if used as a submenu.
	 * @param {string} [desc] - The description of this action set, if used as a submenu.
	 */
	registerActionSet: ContextMenu_registerActionSet,

	/**
	 * Unregisters an action set from the context menu.
	 * @param {string} name - The name of the action set to unregister
	 */
	unregisterActionSet: ContextMenu_unregisterActionSet,

	/**
	 * Edits a previously registered context menu action set. An action set can only be edited by the script that registered it.
	 * @param {string} name - The name of the action set
	 * @param {(Object.<string, ContextMenu.Action>|ContextMenu.Action[])} actions - The action information, present object keys or array indices will replace the registered ones
	 */
	editActionSet: ContextMenu_editActionSet,

	/*
	 * Disables the context menu open action so the bound buttons can be reused.
	 *
	 * The context menu will stay disabled if another script
	 * has disabled it and hasn't re-enabled it yet.
	 *
	 * NOTE: If your script uses @link{ContextMenu.disable}, you should
	 * call @link{ContextMenu.enable} in @link{Script.scriptEnding}.
	 */
	disable: ContextMenu_disable,

	/**
	 * Re-enables the context menu open action, after @link{ContextMenu.disable} has been called.
	 *
	 * The context menu will stay disabled if another script
	 * has disabled it and hasn't re-enabled it yet.
	 *
	 * NOTE: If your script uses @link{ContextMenu.disable}, you should
	 * call @link{ContextMenu.enable} in @link{Script.scriptEnding}.
	 */
	enable: ContextMenu_enable,
};
