//
//  notifications-debug.js
//
//  Created by Armored Dragon on 9 June 2025.
//  Copyright 2025 Overte e.V contributors.
//
//  A small debug app to test notifications
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//


const appSettings = {
	name: "NOTIF",
	icon: Script.resolvePath("./img/icon_white.svg")
}

Script.scriptEnding.connect(appShuttingDown);

let app = {
	toolbarAppButton: null,
	tablet: null,
	active: false,
	add: () => {
		app.tablet = Tablet.getTablet("com.highfidelity.interface.tablet.system");

		addAppToToolbar();
		// app.tablet.fromQml.connect(onMessageFromQML);
	},
	remove: () => {
		// app.tablet.fromQml.disconnect(onMessageFromQML);
		removeAppFromToolbar();
	}
}

function addAppToToolbar() {
	// Check if app is on toolbar
	app.toolbarAppButton = app.tablet.addButton({
		icon: appSettings.icon,
		text: appSettings.name
	});

	app.toolbarAppButton.clicked.connect(toolbarButtonClicked);
}
function removeAppFromToolbar() {
	if (app.toolbarAppButton) {
		app.tablet.removeButton(app.toolbarAppButton);
	}
}

function toolbarButtonClicked() {
	Messages.sendLocalMessage("overte.notification", JSON.stringify({ type: "system", message: `Debug ${Uuid.generate()}`, details: `Comment ${Uuid.generate()}` }))
}

app.add();

function appShuttingDown() {
	app.remove();
}