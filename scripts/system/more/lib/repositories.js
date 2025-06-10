//
//  repos.js
//
//  Created by Armored Dragon on 5 May 2025.
//  Copyright 2025 Overte e.V contributors.
//
//  ---
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

let repos = {
	maxVersion: 2,
	repositories: [],
	applications: [],

	fetchAllAppsFromSavedRepositories: async () => {
		debugLog(`Fetching all saved repositories.`);
		let isLegacyInstalled = false;
		repos.loadRepositoriesFromStorage();
		repos.applications = [];

		const indexOfLegacyRepository = repos.repositories.indexOf(legacy.COMMUNITY_APPS_URL)
		if (indexOfLegacyRepository > -1) {
			// We remove the legacy url here to prevent any further action being taken using this.
			// This will be inserted back into the array just before we send the list of repositories to the UI
			isLegacyInstalled = true;
			await legacy.requestCommunityApps();
			repos.repositories.splice(indexOfLegacyRepository, 1);
		}

		for (let i = 0; repos.repositories.length > i && 99999 > i; i++) {
			// For each repository we have saved...
			let repositoryMetadata = await repos.fetchRepositoryContent(repos.repositories[i]);

			for (let i = 0; repositoryMetadata.applicationList.length > i && 9999999 > i; i++) {
				// For each app in the repository...
				let app = repositoryMetadata.applicationList[i];

				app = repos._embedRepositoryConstants(app, repositoryMetadata);
				app = repos._formatAppUrls(app);
				app = repos._checkIfInstalled(app);

				repos.applications.push(app);
			}
			debugLog(`Finished formatting repository "${repositoryMetadata.title}".`);
		}
		debugLog(`Finished fetching all repositories.`);

		if (isLegacyInstalled) {
			repos.repositories.push(legacy.COMMUNITY_APPS_URL);
		}

		ui.sendAppListToQML();
		ui.sendRepositoryListToQML();

		// debugLog(repos.applications);

		debugLog(`Finished sending repositories to UI.`);
		return true;
	},
	fetchRepositoryContent: async (url) => {
		let repositoryContent = await util.request(url);
		repositoryContent = util.toJSON(repositoryContent);

		// TODO: Versioning

		return repositoryContent;
	},
	installRepository: async (url) => {
		if (!url) {
			debugLog(`No URL provided! Nothing to do.`);
			return;
		}
		url = url.trim();
		url = legacy.formatCommunityAppsUrl(url);

		if (repos.doWeHaveThisRepositorySaved(url)) {
			debugLog(`Repository is already saved.`);
			return null;
		}

		debugLog(`Installing repository: ${url}`);

		if (url === legacy.COMMUNITY_APPS_URL) {
			// Trying to install the legacy metadata.js repository
			repos.repositories.push(legacy.COMMUNITY_APPS_URL);
			repos._saveRepositoriesToSettings();
			ui.sendAppListToQML();
			ui.sendRepositoryListToQML();
			repos.fetchAllAppsFromSavedRepositories();
			return;
		}

		url = util.extractUrlFromString(url);
		if (url === null) {
			debugLog(`Failed to extract url from string.`);
			return;
		}

		let repositoryContent = await repos.fetchRepositoryContent(url);

		if (!repositoryContent) {
			debugLog(`Repository does not contain valid JSON.`);
			return null;
		}

		if (repos.isRepositoryValid(repositoryContent) === false) {
			debugLog(`Repository is not valid.`);
			return null;
		}

		repos.repositories.push(url);

		repos._saveRepositoriesToSettings();
		repositoryContent.applicationList.forEach((entry) => repos.applications.push(entry));

		ui.sendAppListToQML();
		ui.sendRepositoryListToQML();
		repos.fetchAllAppsFromSavedRepositories();

	},
	removeRepository: (url) => {
		if (repos.doWeHaveThisRepositorySaved(url) === false) {
			debugLog(`"${url}" is not saved in our settings. Doing nothing.`);
			return;
		}

		const indexOfRepositoryInSettings = repos.repositories.indexOf(url);

		repos.repositories.splice(indexOfRepositoryInSettings, 1);
		repos._saveRepositoriesToSettings();
		repos.fetchAllAppsFromSavedRepositories();

		ui.sendRepositoryListToQML();
	},
	isRepositoryValid: (repositoryObject) => {
		if (!repositoryObject.version || repositoryObject.version > repos.maxVersion) return false;
		if (!repositoryObject.title) return false;
		if (!repositoryObject.baseApiUrl) return false;
		if (!repositoryObject.applicationList) return false;

		return true;
	},
	doWeHaveThisRepositorySaved: (url) => {
		return repos.repositories.indexOf(url) > -1;
	},
	loadRepositoriesFromStorage: () => {
		repos.repositories = Settings.getValue(settingsRepositoryListName, []);
	},
	_embedRepositoryConstants: (app, repositoryMetadata) => {
		app.repository = {};

		app.repository.baseApiUrl = repositoryMetadata.baseApiUrl;
		app.repository.baseRepositoryUrl = repositoryMetadata.baseRepositoryUrl; // TODO: Change this back to repository homepage
		app.repository.title = repositoryMetadata.title;

		return app;
	},
	_formatAppUrls: (app) => {
		if (util.isValidUrl(app.appIcon) === false) {
			// Application Icon
			debugLog(`"${app.appName}" icon is relative.`);
			app.appIcon = `${app.repository.baseApiUrl}/${app.appBaseDirectory}/${app.appIcon}`;
		}

		Object.keys(app.appScriptVersions).forEach((appVersion) => {
			// Application versions
			let appVersionUrl = app.appScriptVersions[appVersion];
			if (util.isValidUrl(appVersionUrl) === false) {
				debugLog(`"${appVersionUrl}" is relative.`);
				app.appScriptVersions[appVersion] = `${app.repository.baseApiUrl}/${app.appBaseDirectory}/${appVersionUrl}`;
			}
		});

		return app;
	},
	_saveRepositoriesToSettings: () => {
		debugLog(`Saving repositories list to settings.`)
		Settings.setValue(settingsRepositoryListName, repos.repositories);
	},
	_checkIfInstalled: (app) => {
		debugLog(`Checking if ${app.appName} is installed.`)
		app.installedUrl = null;
		app.isInstalled = false;  // Assume the app is not installed.

		const runningScripts = ScriptDiscoveryService.getRunning();
		for (let i = 0; Object.keys(app.appScriptVersions).length > i; i++) {
			// For each of the app versions...
			const appVersionUrl = app.appScriptVersions[Object.keys(app.appScriptVersions)[i]];
			for (let k = 0; runningScripts.length > k; k++) {
				if (appVersionUrl === runningScripts[k].url) {
					app.installedUrl = appVersionUrl;
					app.isInstalled = true;
					break;
				}
			}
			if (app.isInstalled) break;
		}

		return app;
	},
	updateIfAppIsInstalled: () => {
		// Parse through the existing applications array and just checks to see if an app is installed
		const runningScripts = ScriptDiscoveryService.getRunning().map((item) => item.url);
		debugLog(runningScripts)

		for (let i = 0; repos.applications.length > i; i++) {
			// For each application in the array...
			let app = repos.applications[i];
			debugLog(`Checking if ${app.appName} is installed...`);

			app.installedUrl = null;
			app.isInstalled = false;  // Assume the app is not installed.

			for (let k = 0; Object.keys(app.appScriptVersions).length > k; k++) {
				// For each of the app versions...
				const appVersionUrl = app.appScriptVersions[Object.keys(app.appScriptVersions)[k]];
				if (runningScripts.indexOf(appVersionUrl) > -1) {
					app.installedUrl = appVersionUrl;
					app.isInstalled = true;
					break;
				}
			}
			if (app.isInstalled) break;
		}

		return true;
	}
}