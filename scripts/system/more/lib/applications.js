//
//  applications.js
//
//  Created by Armored Dragon on June 10th, 2025.
//  Copyright 2025, Overte e.V.
//
//  ---
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

let apps = {
	installedApps: [],
	getInstalledApps: () => {
        apps.installedApps = Settings.getValue(settingsAppListName, []);
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

		apps.installedApps.push(url);
		Settings.setValue(settingsAppListName, apps.installedApps);

        if (!apps.isAppRunning(url)) {
            ScriptDiscoveryService.loadScript(url, true);
        } else {
            refreshData();
        }

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
	stop: (url, baseUrl) => {
		debugLog(`Stop ${url}.`);

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

		ScriptDiscoveryService.stopScript(url, false);

		return true;
	},
	reload: (url, baseUrl) => {
		debugLog(`Reload ${url}.`);

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

		if (!ScriptDiscoveryService.stopScript(url, true)) {
            ScriptDiscoveryService.loadOneScript(url)
        }
        refreshData();
        
		return true;
	},
	isAppAlreadyInstalled: (url) => {
		apps.getInstalledApps();
		return apps.installedApps.indexOf(url) > -1;
	},
	isAppRunning: (url) => {
        const runningScripts = JSON.stringify(ScriptDiscoveryService.getRunning());
		return runningScripts.indexOf(url) !== -1;
	}
}
