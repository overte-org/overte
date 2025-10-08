//
//  more.js
//
//  Created by Armored Dragon on May 5th, 2025.
//  Based on the original More App. prototype by Keb Helion (Alezia Kurdis), February 2020.
//  Copyright 2025, Overte e.V.
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

const DEBUG = false; //to switch on-off all the debug info

const DELAY_BEFORE_RETRY = 4000;

Script.scriptEnding.connect(appShuttingDown);

app.add();

function appShuttingDown() {
	console.log("Shutting down more.js application");
	app.remove();
    Script.scriptEnding.disconnect(appShuttingDown);
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
        case "stopApp":
            apps.stop(event.appUrl, event.baseUrl);
            break;
        case "reloadApp":
            apps.reload(event.appUrl, event.baseUrl);
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
    if (DEBUG) {
        if (typeof content === "object") content = JSON.stringify(content, null, 4);

        console.log(`[ Debug ] ${content}`);
    }
}

let ui = {
	sendAppListToQML: () => {
		let visibleApplications = repos.applications.filter((item) => item.appActive === true);
		sendMessageToQML({ type: "appList", appList: visibleApplications });
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

// Retry Mechanism
Script.setTimeout(function () {
    const installed = Settings.getValue(settingsAppListName, []);
    const runningScripts = JSON.stringify(ScriptDiscoveryService.getRunning());
    for (let i = 0; i < installed.length; i++) {
        if (runningScripts.indexOf(installed[i]) === -1){
            debugLog("Retry" + installed[i] + ".");
            ScriptDiscoveryService.loadOneScript(installed[i]);
        }
    }
}, DELAY_BEFORE_RETRY);
