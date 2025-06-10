//
//  applications.js
//
//  Created by Armored Dragon on 10 June 2025.
//  Copyright 2025 Overte e.V contributors.
//
//  ---
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

let apps = {
	installedApps: [],
	getInstalledApps: () => {
		const runningScripts = ScriptDiscoveryService.getRunning().map((item) => item.url);

		// Get the list of apps we are supposed to have installed 
		apps.installedApps = runningScripts;

		return apps.installedApps;
	},
	install: (app, version) => {
		let url = app.appScriptVersions[version];

		debugLog(`"${version}".`);
		debugLog(`Installing "${url}".`);

		if (util.isValidUrl(url) === false) {
			// Not a url, handle this as a relative path.
			debugLog(`Handling link as a relative url.`);
			url = `${app.appRepositoryUrl}/${app.appBaseDirectory}/${url}`;
			debugLog(`Url was changed to "${url}`);
		}

		url = util.extractUrlFromString(url);
		if (!url) {
			debugLog(`Provided url was invalid.`);
			return null
		}

		if (apps.isAppAlreadyInstalled(url)) {
			debugLog(`App is already installed.`);
			return null;
		}

		// TODO: Check if app is loaded

		ScriptDiscoveryService.loadScript(url, true);
		apps.installedApps.push(url);
		Settings.setValue(settingsAppListName, apps.installedApps);

		return true;
	},
	remove: (url, baseUrl) => {
		debugLog(`Removing ${url}.`);

		if (util.isValidUrl(url) === false) {
			// Not a url, handle this as a relative path.
			debugLog(`Handling link as a relative url.`);
			url = `${baseUrl}/${url}`;
			debugLog(`Url was changed to "${url}`);
		}

		url = util.extractUrlFromString(url);
		if (!url) {
			debugLog(`Provided url was invalid.`);
			return null
		}

		if (apps.isAppAlreadyInstalled(url) === false) {
			debugLog(`"${url}" is not installed.`);
			return null;
		}

		ScriptDiscoveryService.stopScript(url, false);
		const indexOfApp = apps.installedApps.indexOf(url);
		apps.installedApps.splice(indexOfApp, 1);
		Settings.setValue(settingsAppListName, apps.installedApps);

		return true;
	},
	isAppAlreadyInstalled: (url) => {
		apps.getInstalledApps();
		return apps.installedApps.indexOf(url) > -1;
	}
}