//
//  more.js
//
//  Created by Armored Dragon on 5 May 2025.
//  Copyright 2025 Overte e.V contributors.
//
//  This interface application allows users to manage and install apps provided by third party repositories easily.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

Script.include("./lib/app.js");
Script.include("./lib/format.js");
Script.include("./lib/repositories.js");
Script.include("./lib/applications.js");
Script.include("./lib/util.js");

const settingsRepositoryListName = "overte.more.repositories";
const settingsAppListName = "overte.more.app";
const settingsIsFirstRun = "overte.more.isFirstRun";

Script.scriptEnding.connect(appShuttingDown);

app.add();

function appShuttingDown() {
	console.log("Shutting down more.js application");
	app.remove();
}

function onMessageFromQML(event) {
	debugLog(event);
	switch (event.type) {
		case "addNewRepositoryButtonClicked":
			let newRepositoryUrl = Window.prompt("Enter the URL of the repository metadata.json file.", "");
			repos.installRepository(newRepositoryUrl);
			break;
		case "installApp":
			apps.install(event.appUrl, event.baseUrl);
			break;
		case "uninstallApp":
			apps.remove(event.appUrl, event.baseUrl);
			break;
		case "openAppRepository":
			Window.openUrl(event.repositoryUrl);
			break;
		case "removeRepository":
			repos.removeRepository(event.entryUrl);
			break;
	}
}

function sendMessageToQML(message) {
	app.tablet.sendToQml(message);
}

function debugLog(content) {
	if (typeof content === "object") content = JSON.stringify(content, null, 4);

	console.log(`[ Debug ] ${content}`);
}

let ui = {
	sendAppListToQML: () => {
		sendMessageToQML({ type: "appList", appList: repos.applications });
		return;
	},
	sendRepositoryListToQML: () => {
		let formattedListOfRepositories = repos.repositories.map((entry) => { return { entryText: entry } });

		sendMessageToQML({
			type: "repositoryList", repositoryList: formattedListOfRepositories
		});
		return;
	}
}

repos.fetchAllAppsFromSavedRepositories();

if (Settings.getValue(settingsIsFirstRun, true) === true) {
	// First run. Install the overte repository.
	// NOTE: The url provided is parsed and handled differently using the legacy.requestCommunityApps() function.
	repos.installRepository(legacy.COMMUNITY_APPS_URL);
	Settings.setValue(settingsIsFirstRun, false);
}