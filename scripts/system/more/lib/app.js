//
//  app.js
//
//  Created by Armored Dragon on May 5th, 2025.
//  Copyright 2025, Overte e.V.
//
//  ---
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

const appSettings = {
	name: "MORE",
	icon: Script.resolvePath("../img/icon_white.png"),
	activeIcon: Script.resolvePath("../img/icon_black.png"),
	url: Script.resolvePath("../qml/More.qml")
}

let app = {
	toolbarAppButton: null,
	tablet: null,
	active: false,
	add: () => {
		app.tablet = Tablet.getTablet("com.highfidelity.interface.tablet.system");

		addAppToToolbar();

		app.tablet.fromQml.connect(onMessageFromQML);
	},
	remove: () => {
		app.tablet.fromQml.disconnect(onMessageFromQML);
		removeAppFromToolbar();
	}
}

ScriptDiscoveryService.scriptCountChanged.connect(() => {
    refreshData();
});

function refreshData() {
    repos.fetchAllAppsFromSavedRepositories();
	ui.sendAppListToQML();
}

function addAppToToolbar() {
	// Check if app is on toolbar

	app.toolbarAppButton = app.tablet.addButton({
		icon: appSettings.icon,
		activeIcon: appSettings.activeIcon,
		text: appSettings.name,
		isActive: app.active,
	});

	app.toolbarAppButton.clicked.connect(toolbarButtonClicked);
}
function removeAppFromToolbar() {
	if (app.toolbarAppButton) {
		app.tablet.removeButton(app.toolbarAppButton);
	}
}

function toolbarButtonClicked() {
	if (app.active) {
		deactivateToolbarButton();
	}
	else {
		activateToolbarButton();
	}
}

function onTabletScreenChanged(type, newURL) {
	if (appSettings.url === newURL) {
		activateToolbarButton();
	}
	else {
		deactivateToolbarButton(false);
	}
}

function deactivateToolbarButton(goToHomeScreen = true) {
	if (app.active === false) return; // Already inactive, ignore.

	app.tablet.screenChanged.disconnect(onTabletScreenChanged);
	app.active = false;
	if (goToHomeScreen) app.tablet.gotoHomeScreen();
	app.toolbarAppButton.editProperties({ isActive: false });
}

function activateToolbarButton() {
	if (app.active) return; // Already active, ignore.

	app.tablet.loadQMLSource(appSettings.url);
	app.active = true;
	app.toolbarAppButton.editProperties({ isActive: true });
	app.tablet.screenChanged.connect(onTabletScreenChanged);

	repos.fetchAllAppsFromSavedRepositories();
	ui.sendRepositoryListToQML();
	ui.sendAppListToQML(repos.applications);
}
