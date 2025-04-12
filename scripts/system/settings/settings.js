//
//  settings.js
//
//  App to configure Overte
//
//  Created by Armored Dragon, 2024.
//  Copyright 2024-2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html

/* global Script Tablet */

// TODO: Fullscreen display?
// TODO: Replace bool setting with switch?
// FIXME: Setting slider handle
// TODO: Setting slider incorrect radius around filled background?
// FIXME: Combobox overflows text onto arrow visual
// TODO: Advanced Settings widget

(() => {
	"use strict";
	var tablet;
	var appButton;
	var active = false;
	const url = Script.resolvePath("./Settings.qml")

	tablet = Tablet.getTablet("com.highfidelity.interface.tablet.system");
	appButton = tablet.addButton({
		icon: Script.resolvePath("./img/icon_white.png"),
		activeIcon: Script.resolvePath("./img/icon_black.png"),
		text: "SETTINGS",
		isActive: active,
	});

	// When script ends, remove itself from tablet
	Script.scriptEnding.connect(function () {
		console.log("Shutting down Settings.js application");
		tablet.removeButton(appButton);
		Menu.removeMenuItem("Settings", "Graphics...");
	});

	// Event listeners
	appButton.clicked.connect(toolbarButtonClicked);
	tablet.fromQml.connect(fromQML);
	tablet.screenChanged.connect(onTabletScreenChanged);
	Menu.menuItemEvent.connect(onMenuItemEvent);

	// Menu button
	Menu.addMenuItem({
		menuName: "Settings",
		menuItemName: "Graphics...",
		afterItem: "Audio...",
	});

	function onMenuItemEvent(menuItem) {
		if (menuItem === 'Graphics...') {
			toolbarButtonClicked();
			toQML({ type: 'loadPage', page: 'Graphics' })
		}
	}

	function toolbarButtonClicked() {
		if (active) tablet.gotoHomeScreen();
		else tablet.loadQMLSource(url);

		active = !active;
		appButton.editProperties({
			isActive: active,
		});
	}

	function onTabletScreenChanged(type, new_url) {
		if (url == new_url) active = true;
		else active = false;

		appButton.editProperties({
			isActive: active,
		});
	}

	// Communication
	function fromQML(event) {
		console.log(`New QML event:\n${JSON.stringify(event)}`);

		if (event.type === "switch_app") {
			if (event.app_url == "hifi/dialogs/GeneralPreferencesDialog.qml") {
				// This page needs to be opened like this just because.
				Desktop.show("hifi/dialogs/GeneralPreferencesDialog.qml", "GeneralPreferencesDialog");
				return;
			}
			tablet.loadQMLSource(event.app_url);
		}
	}
	/**
	 * Emit a packet to the HTML front end. Easy communication!
	 * @param {Object} packet - The Object packet to emit to the HTML
	 * @param {("loadPage"|)} packet.type - The type of packet it is
	 */
	function toQML(packet = { type: "" }) {
		tablet.sendToQml(packet);
	}
})();